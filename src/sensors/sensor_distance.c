#include "sensor_distance.h"
#include <sensors/VL53L0X/VL53L0X.h>
#include <ch.h>

static semaphore_t dist_sem;
uint16_t dist_mm = 0;

uint8_t sensor_distance_norm(void) {
    uint8_t dist;
    chSemWait(&dist_sem);
    if((dist_mm < 1 * CELL_WIDTH))
        dist = FIRST_CASE;
    else if((dist_mm > 1 * CELL_WIDTH) && (dist_mm < 2 * CELL_WIDTH))
        dist = SECOND_CASE;
    else if((dist_mm > 2 * CELL_WIDTH) && (dist_mm < 3 * CELL_WIDTH))
        dist = THIRD_CASE;
    else //((dist_mm > 3 * CELL_WIDTH))
        dist = OUT_MAP;
    chSemSignal(&dist_sem);
    return dist;
}

static THD_WORKING_AREA(waSensorDistance, 256);
static THD_FUNCTION(SensorDistance, arg) {
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    for(;;) {
        chSemWait(&dist_sem);
        dist_mm = VL53L0X_get_dist_mm();
        chSemSignal(&dist_sem);
        chThdSleepMilliseconds(SENSOR_DISTANCE_REFRESH_T);
    }
}

void sensor_distance_init(void) {
    chSemObjectInit(&dist_sem, 1);
    chThdCreateStatic(waSensorDistance, sizeof(waSensorDistance), NORMALPRIO, SensorDistance, NULL);
    VL53L0X_start();
}