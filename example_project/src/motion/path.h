#ifndef PATH_H
#define PATH_H

#include "../defs.h"

/*
 * Use the A* algorithm to mark the cells on the path
 * Then use the function generatePathCode() to generate a dynamically allocated list of steps
 */
step_t* findPath(cell_t gameMap[SIDE_NCELL][SIDE_NCELL], coord_t current, const coord_t target, uint8_t backward);

#endif /*PATH_H*/