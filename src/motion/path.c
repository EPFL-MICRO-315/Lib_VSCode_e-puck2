#include <stdlib.h>
#include "path.h"

/*
 * those functions are inline because they are only used once by findPath()
 */
inline void pathFindingReset(cell_t gameMap[SIDE_NCELL][SIDE_NCELL]);
inline step_t* generatePathCode(cell_t gameMap[SIDE_NCELL][SIDE_NCELL], coord_t current, coord_t target, uint8_t backward);
void avoid_path_out_of_memory(step_t *path, uint16_t *path_used, uint16_t *path_allocated);
void backward_or_halfturn(step_t *path, uint16_t *path_used, uint8_t backward);

step_t* findPath(cell_t gameMap[SIDE_NCELL][SIDE_NCELL], coord_t current, const coord_t target, uint8_t backward) {
    pathFindingReset(gameMap);

    /*
     * the pahtfinding algorithm start from the target and search the path toward the current position
     * it may seems illogical but this is way, the path obtained through the pointers "parent" starting
     * from the current position is in the right order (current_pos -> intermediate pos -> ... -> target)
     * Otherwise the path's would be reversed (target -> intermiate pos -> ... -> current_pos)
     * this later form needs further process : inverse the list, or process it backward; not handy in any case
     */
    gameMap[current.x][current.y].parent = NULL; // stop code for the generatePathCode's loop
    gameMap[target.x][target.y].state |= CELL_OPEN;
    uint8_t f_min, x_min = 0, y_min = 0;
    //use of for(;;) instead of while(true) because more efficient : there are no check to perform between each steps
    for(;;) {
        /*
        * the default f_min value is 0xFF (0xFF)
        * considering the size of the gameMap a such value should never be obtained
        * TODO: test if uint8_t is enough for the f_score & g_score
        */
        f_min = 0xFF;
        for(uint8_t x = 0; x < SIDE_NCELL; x++)
            for(uint8_t y = 0; y < SIDE_NCELL; y++)
                if(((gameMap[x][y].state & PATH_FIND_BITS) == CELL_OPEN) && (gameMap[x][y].f_score <= f_min)) {
                    f_min = gameMap[x][y].f_score;
                    x_min = x;
                    y_min = y;
                }
        gameMap[x_min][y_min].state = (gameMap[x_min][y_min].state & ~PATH_FIND_BITS) | CELL_CLOSED;
        // The current position has been reached (starting from the target)
        if(((x_min == current.x) && (y_min == current.y)))
            break;

        for(uint8_t w = 0; w < 4; w++) {
            int8_t x = x_min + nearest[w].x;
            int8_t y = y_min + nearest[w].y;

            // Check boundaries
            if((x < 0) || (x >= SIDE_NCELL) || (y < 0) || (y >= SIDE_NCELL)) {}

            // Check for obstacles and already checked cells
            else if(((gameMap[x][y].state & OBSTRUCTION_BITS) == CELL_OCCUPED) ||
                    ((gameMap[x][y].state & OBSTRUCTION_BITS) == CELL_OCCUPED_BLUE)  ||
                    ((gameMap[x][y].state & OBSTRUCTION_BITS) == CELL_OCCUPED_RED)  ||
                    ((gameMap[x][y].state & KNOWLEDGE_BITS) == CELL_UNKNOWN)  ||
                    ((gameMap[x][y].state & PATH_FIND_BITS) == CELL_CLOSED)) {}

            // Update scores, parents & state if cell blank or a shorter path to the (x,y) cell was found
            else if((gameMap[x_min][y_min].g_score + PATH_UNIT_LENGTH < gameMap[x][y].g_score) ||
                   ((gameMap[x][y].state & PATH_FIND_BITS) == CELL_BLANK))
            {
                gameMap[x][y].g_score = gameMap[x_min][y_min].g_score + PATH_UNIT_LENGTH;
                gameMap[x][y].f_score = gameMap[x][y].g_score + getDistance((coord_t){.x=x, .y=y}, current);


                gameMap[x][y].parent = &gameMap[x_min][y_min];
                gameMap[x][y].state = (gameMap[x][y].state & ~PATH_FIND_BITS) | CELL_OPEN;
            }
        }
    }
    return generatePathCode(gameMap, current, target, backward);
}

step_t* generatePathCode(cell_t gameMap[SIDE_NCELL][SIDE_NCELL], coord_t current, coord_t target, uint8_t backward) {
    cell_t* icell = &gameMap[current.x][current.y];
    cell_t* icell_old = NULL;
    int8_t delta_x = 0, delta_y = 0;
    angle_t tmp_t = current.t;

    uint16_t path_allocated = MAX_PATH_SIZE;
    uint16_t path_used = 0;
    step_t* path = calloc(path_allocated, sizeof(step_t)); //use of calloc to default initiate to zero (=STOP)

    while(icell->parent != NULL) {
        icell_old = icell;
        icell = icell_old->parent;
        int32_t delta_addr = (icell - icell_old);

        switch(delta_addr) {
            case +SIDE_NCELL:
                delta_x = +1;
                delta_y = 0;
                break;
            case -SIDE_NCELL:
                delta_x = -1;
                delta_y = 0;
                break;
            case +1:
                delta_x = 0;
                delta_y = +1;
                break;
            case -1:
                delta_x = 0;
                delta_y = -1;
                break;
        }

        avoid_path_out_of_memory(path, &path_used, &path_allocated);

        switch(tmp_t) {
            case E:
                if(delta_x == 1) {
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                }
                else if(delta_x == -1)
                    backward_or_halfturn(path, &path_used, backward);
                else if(delta_y == 1) {
                    path[path_used++] = RIGHT;
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                    tmp_t = S;
                }
                else if(delta_y == -1) {
                    path[path_used++] = LEFT;
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                    tmp_t = N;
                }
                break;
            case N:
                if(delta_x == 1) {
                    path[path_used++] = RIGHT;
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                    tmp_t = E;
                }
                else if(delta_x == -1) {
                    path[path_used++] = LEFT;
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                    tmp_t = W;
                }
                else if(delta_y == 1)
                    backward_or_halfturn(path, &path_used, backward);
                else if(delta_y == -1) {
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                }
                break;
            case W:
                if(delta_x == 1)
                    backward_or_halfturn(path, &path_used, backward);
                else if(delta_x == -1) {
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                }
                else if(delta_y == 1) {
                    path[path_used++] = LEFT;
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                    tmp_t = S;
                }
                else if(delta_y == -1) {
                    path[path_used++] = RIGHT;
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                    tmp_t = N;
                }
                break;
            case S:
                if(delta_x == 1) {
                    path[path_used++] = LEFT;
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                    tmp_t = E;
                }
                else if(delta_x == -1) {
                    path[path_used++] = RIGHT;
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                    tmp_t = W;
                }
                else if(delta_y == 1) {
                    path[path_used++] = FORWARD;
                    path[path_used++] = FORWARD;
                }
                else if(delta_y == -1)
                    backward_or_halfturn(path, &path_used, backward);
                break;
        }
    }

    avoid_path_out_of_memory(path, &path_used, &path_allocated);

    //might want to use look up table instead, cleaner, hard to figure out
    switch(tmp_t) {
        case E:
            switch(target.t) {
                case E: break;
                case N: path[path_used++] = LEFT; break;
                case W: path[path_used++] = LEFT; path[path_used++] = LEFT; break;
                case S: path[path_used++] = RIGHT; break;
            }
            break;
        case N:
            switch(target.t) {
                case E: path[path_used++] = RIGHT; break;
                case N: break;
                case W: path[path_used++] = LEFT; break;
                case S: path[path_used++] = LEFT; path[path_used++] = LEFT; break;
            }
            break;
        case W:
            switch(target.t) {
                case E: path[path_used++] = LEFT; path[path_used++] = LEFT; break;
                case N: path[path_used++] = RIGHT; break;
                case W: break;
                case S: path[path_used++] = LEFT; break;
            }
            break;
        case S:
            switch(target.t) {
                case E: path[path_used++] = LEFT; break;
                case N: path[path_used++] = LEFT; path[path_used++] = LEFT; break;
                case W: path[path_used++] = RIGHT; break;
                case S: break;
            }
            break;
    }

    return path;
}

void pathFindingReset(cell_t gameMap[SIDE_NCELL][SIDE_NCELL]) {
    for(uint8_t x = 0; x < SIDE_NCELL; x++)
        for(uint8_t y = 0; y < SIDE_NCELL; y++) {
            //set to zero state's bits related to path finding
            gameMap[x][y].state &= ~PATH_FIND_BITS;
            gameMap[x][y].f_score = 0xFF;
            //reset of parent & g_score not really necessary, test before remove
            gameMap[x][y].g_score = 0;
            gameMap[x][y].parent = NULL;
        }
}

void avoid_path_out_of_memory(step_t *path, uint16_t *path_used, uint16_t *path_allocated) {
    /*
     * Each iteration in the while loop can add at max 2 steps
     * => the realloc must reserve space for at least 2 step_t
     * to limitate the number of call of realloc, realloc will only
     * happens when the current size is close to the current limit and
     * will reserve 10 more free spaces
     */
    if((*path_used) + 4 >= (*path_allocated)) {
        (*path_allocated) += 10;
        path = realloc(path, (*path_allocated)*sizeof(step_t));
        for(uint8_t i = (*path_allocated) - 10; i < (*path_allocated); i++)
            path[i] = STOP;
    }
}

void backward_or_halfturn(step_t *path, uint16_t *path_used, uint8_t backward) {
    if(backward == 1) {
        path[(*path_used)++] = BACKWARD;
        path[(*path_used)++] = BACKWARD;
    }
    else {
        path[(*path_used)++] = LEFT;
        path[(*path_used)++] = LEFT;
        path[(*path_used)++] = FORWARD;
        path[(*path_used)++] = FORWARD;
    }
}