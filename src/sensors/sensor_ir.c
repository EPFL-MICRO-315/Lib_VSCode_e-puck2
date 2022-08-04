#include "sensor_ir.h"
#include "../defs.h"
#include "../game.h"
#include <ch.h>
#include <hal.h>
#include <memory_protection.h>
#include <leds.h>
#include <sensors/proximity.h>
#include <spi_comm.h>

inline void sensor_ir_measure(uint8_t *ir_count_1, uint8_t *ir_count_2, uint8_t *ir_state_1, uint8_t *ir_state_2);
inline void check_at_start(uint8_t *ir_count_1, uint8_t *ir_count_2, uint8_t *ir_state_1, uint8_t *ir_state_2);
inline void check_ingame(uint8_t *ir_count_1, uint8_t *ir_count_2, uint8_t *ir_state_1, uint8_t *ir_state_2);

static THD_WORKING_AREA(waSensorIR, 256);
static THD_FUNCTION(SensorIR, arg) {
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    /*
     * the ir_states enables to differentiate short hold to long old
     * the ir_count tells for how long the corresponding was kept in hold
     */
    uint8_t ir_count_1=0;
    uint8_t ir_count_2=0;
    uint8_t ir_state_1=0;
    uint8_t ir_state_2=0;

    systime_t time;
    while(1) {
        uint8_t gamestates_tmp = get_gamestates();
        sensor_ir_measure(&ir_count_1, &ir_count_2, &ir_state_1, &ir_state_2);
        switch(gamestates_tmp & STATE_BITS) {
            case STATE_START:
                check_at_start(&ir_count_1, &ir_count_2, &ir_state_1, &ir_state_2); break;
            case STATE_WAITING_PLAYER:
                check_ingame(&ir_count_1, &ir_count_2, &ir_state_1, &ir_state_2); break;
            case STATE_PLAYING:
                check_ingame(&ir_count_1, &ir_count_2, &ir_state_1, &ir_state_2); break;
            default:
                break;
        }
        show_stateofgame();

        palTogglePad(GPIOD, GPIOD_LED_FRONT);
        time = chVTGetSystemTime();
        chThdSleepUntilWindowed(time, time + MS2ST(SENSOR_IR_REFRESH_T));
    }
}

void sensor_ir_init(void) {
    //chThdSleep(1000); // wait for the spi to start
    proximity_start();
    calibrate_ir();
	chThdCreateStatic(waSensorIR, sizeof(waSensorIR), NORMALPRIO+1, SensorIR, NULL);
}

void sensor_ir_measure(uint8_t *ir_count_1, uint8_t *ir_count_2, uint8_t *ir_state_1, uint8_t *ir_state_2) {
    if(*ir_state_1 == 1) {
        *ir_state_1 = 0;
        *ir_count_1 = 0;
    }
    if(get_calibrated_prox(3) > SENSOR_IR_THRESHOLD) {
        *ir_count_1 = (*ir_count_1 < 0xFF) ? (*ir_count_1 + 1) : *ir_count_1;
        *ir_state_1 = 0;
    }
    else {
        *ir_state_1 = 1;
    }
    if(*ir_state_2 == 1) {
        *ir_state_2 = 0;
        *ir_count_2 = 0;
    }
    if(get_calibrated_prox(6) > SENSOR_IR_THRESHOLD) {
        *ir_count_2 = (*ir_count_2 < 0xFF) ? (*ir_count_2 + 1) : *ir_count_2;
        *ir_state_2 = 0;
    }
    else {
        *ir_state_2 = 1;
    }
}

void check_at_start(uint8_t *ir_count_1, uint8_t *ir_count_2, uint8_t *ir_state_1, uint8_t *ir_state_2) {
    if((*ir_state_1 == 1) && (*ir_state_2 == 1)) {
        if     ((*ir_count_1 > (HOLD_LONG / SENSOR_IR_REFRESH_T))  && (*ir_count_2 < (HOLD_LONG / SENSOR_IR_REFRESH_T)))
            set_gamestates((get_gamestates() & ~DIFFICULTY_BITS) | DIFFICULTY_EASY);
        else if((*ir_count_1 > (HOLD_SHORT / SENSOR_IR_REFRESH_T)) && (*ir_count_2 < (HOLD_SHORT / SENSOR_IR_REFRESH_T)))
            set_gamestates((get_gamestates() & ~DIFFICULTY_BITS) | DIFFICULTY_HARD);
        else if((*ir_count_1 < (HOLD_LONG / SENSOR_IR_REFRESH_T))  && (*ir_count_2 > (HOLD_LONG / SENSOR_IR_REFRESH_T)))
            set_gamestates((get_gamestates() & ~STATE_BITS) | STATE_WAITING_PLAYER);
        else if((*ir_count_1 < (HOLD_SHORT / SENSOR_IR_REFRESH_T)) && (*ir_count_2 > (HOLD_SHORT / SENSOR_IR_REFRESH_T)))
            set_gamestates((get_gamestates() & ~STATE_BITS) | STATE_PLAYING);
    }
}

void check_ingame(uint8_t *ir_count_1, uint8_t *ir_count_2, uint8_t *ir_state_1, uint8_t *ir_state_2) {
    if((*ir_state_1 == 1) && (*ir_state_2 == 1)) {
        if((*ir_count_1 > (HOLD_LONG / SENSOR_IR_REFRESH_T)) && (*ir_count_2 > (HOLD_LONG / SENSOR_IR_REFRESH_T)))
            set_gamestates((get_gamestates() & ~STATE_BITS) | STATE_END);
        else if((*ir_count_1 > (HOLD_SHORT / SENSOR_IR_REFRESH_T)) && (*ir_count_2 < (HOLD_SHORT / SENSOR_IR_REFRESH_T)))
            if((get_gamestates() & STATE_BITS) | STATE_WAITING_PLAYER)
                set_gamestates((get_gamestates() & ~STATE_BITS) | STATE_PLAYING);
    }
}