#ifndef SCAN_H
#define SCAN_H

#include "defs.h"

/*
 * Called each time the human player has finished his move such that the robot
 * knows what was the human' last move.
 */
void updateMap(cell_t gameMap[SIDE_NCELL][SIDE_NCELL], coord_t *position);

#endif /*SCAN_H*/