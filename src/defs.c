#include "defs.h"
#include <math.h>

uint8_t getDistance(coord_t A, coord_t B) {
    int8_t delta_x = A.x - B.x;
    int8_t delta_y = A.y - B.y;
    return (uint8_t)(PATH_UNIT_LENGTH * sqrt(delta_x*delta_x+delta_y*delta_y));
}