#ifndef SENSOR_DISTANCE_H
#define SENSOR_DISTANCE_H

#include "../defs.h"

void sensor_distance_init(void);
/*
 * Return the last measured distance with the TOF sensor
 * See defs.h to see to what corresponds those uint8_t numbers
 */
uint8_t sensor_distance_norm(void);

#endif /*SENSOR_DISTANCE_H*/