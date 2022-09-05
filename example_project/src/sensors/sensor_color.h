#ifndef SENSOR_COLOR_H
#define SENSOR_COLOR_H

#include "../defs.h"

void sensor_color_init(void);
/*
 * Return the last measured color with the camera
 * See defs.h to see to what corresponds those uint8_t numbers
 */
uint8_t sensor_measure_color(void);

#endif /*SENSOR_COLOR_H*/