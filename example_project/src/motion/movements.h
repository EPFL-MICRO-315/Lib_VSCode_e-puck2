#ifndef MOVEMENTS_H
#define MOVEMENTS_H

#include "../defs.h"

void mvt_init(void);
// Read the path array and execute it
void mvt_executePath(coord_t *position, const coord_t target, step_t *path);
// Used to place a piece at target coord
void mvt_place(cell_t gameMap[SIDE_NCELL][SIDE_NCELL], coord_t *position, const coord_t target);

// Basic and only movements the EMorpuck can execute
void mvt_forward(void);
void mvt_backward(void);
void mvt_left(void);
void mvt_right(void);

#endif /*MOVEMENTS_H*/
