#include "scan.h"

#include "defs.h"
#include "motion/path.h"
#include "motion/movements.h"
#include "sensors/sensor_color.h"
#include "sensors/sensor_distance.h"
#include "game.h"

/*
 * This array tells the order in which cell are scanned
 * Directions doesn't matter, it'll be computed later using KNOWLEDGE_BITS & scanning_position_order table
 */
static const coord_t cell_scanning_order[9] = {
    { GAMEMAP_CENTER - 1, GAMEMAP_CENTER - 1, E},
    { GAMEMAP_CENTER - 1, GAMEMAP_CENTER    , E},
    { GAMEMAP_CENTER - 1, GAMEMAP_CENTER + 1, E},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER + 1, E},
    { GAMEMAP_CENTER + 1, GAMEMAP_CENTER + 1, E},
    { GAMEMAP_CENTER + 1, GAMEMAP_CENTER    , E},
    { GAMEMAP_CENTER + 1, GAMEMAP_CENTER - 1, E},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER - 1, E},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER    , E}
};

/*
 * This table with cell_scanning_order are complementary
 * cell_scanning_order tells what cell and in what order it should scan
 * scanning_position_order tells in order where the robot should be to scan the correspinding cell_scanning_order cell
 */
static const coord_t scanning_position_order[9] = {
    { GAMEMAP_CENTER - 2, GAMEMAP_CENTER - 1, E},
    { GAMEMAP_CENTER - 2, GAMEMAP_CENTER    , E},
    { GAMEMAP_CENTER - 2, GAMEMAP_CENTER + 1, E},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER + 2, N},
    { GAMEMAP_CENTER + 2, GAMEMAP_CENTER + 1, W},
    { GAMEMAP_CENTER + 2, GAMEMAP_CENTER    , W},
    { GAMEMAP_CENTER + 2, GAMEMAP_CENTER - 1, W},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER - 2, S},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER    , S} //this direction doesn't matter, it'll be computed later
};

void updateMap(cell_t gameMap[SIDE_NCELL][SIDE_NCELL], coord_t *position) {
    coord_t tocheck;
    step_t *path;

    /*
     * Set cell's state which aren't occuped yet as UNKNOWN (they could have changed since th elast updateMap() call)
     * If they are already occuped, put them to KNOWN state => no need to scan this cell
     */
    for(int8_t i = -1 + GAMEMAP_CENTER; i <= 1 + GAMEMAP_CENTER; i++) {
        for(int8_t j = -1 + GAMEMAP_CENTER; j <= 1 + GAMEMAP_CENTER; j++) {
            if(((gameMap[i][j].state & OBSTRUCTION_BITS) == CELL_OCCUPED_RED) || ((gameMap[i][j].state & OBSTRUCTION_BITS) == CELL_OCCUPED_BLUE))
                gameMap[i][j].state = (gameMap[i][j].state & ~KNOWLEDGE_BITS) | CELL_KNOWN;
            else
                gameMap[i][j].state = (gameMap[i][j].state & ~KNOWLEDGE_BITS) | CELL_UNKNOWN;
        }
    }

    for(uint8_t i = 0; i < 9; i++) {
        if((gameMap[cell_scanning_order[i].x][cell_scanning_order[i].y].state & KNOWLEDGE_BITS) == CELL_UNKNOWN) {
            tocheck = scanning_position_order[i];
            if((cell_scanning_order[i].x == GAMEMAP_CENTER) && (cell_scanning_order[i].y == GAMEMAP_CENTER)) {
                /*
                 * It could looks like there is a case where it's impossible to scan the cell in the middle because it's surrounded on every sides
                 * However, the robot's playing strategy forbidden this kind of situation by always playing the middle case if this later one is free
                 * This strategy simplify the scanning and is moreover the winning strategy
                 */
                for(uint8_t w = 0; w < 4; w++) {
                    if(((gameMap[GAMEMAP_CENTER + nearest[w].x][GAMEMAP_CENTER + nearest[w].y].state & OBSTRUCTION_BITS) != CELL_OCCUPED_RED) &&
                       ((gameMap[GAMEMAP_CENTER + nearest[w].x][GAMEMAP_CENTER + nearest[w].y].state & OBSTRUCTION_BITS) != CELL_OCCUPED_BLUE) &&
                       ((gameMap[GAMEMAP_CENTER + nearest[w].x][GAMEMAP_CENTER + nearest[w].y].state & KNOWLEDGE_BITS) != CELL_UNKNOWN))
                       {
                        tocheck = (coord_t){.x=GAMEMAP_CENTER+nearest[w].x, .y=GAMEMAP_CENTER+nearest[w].y, .t=nearest[w].t};
                       }
                }
            }
            path = findPath(gameMap, *position, tocheck, 1);
            mvt_executePath(position, tocheck, path);
            uint8_t c_result = sensor_measure_color();
            uint8_t d_result = sensor_distance_norm();

            if((c_result == RED) && (d_result == FIRST_CASE))
                set_gamestates((get_gamestates() & ~SCANNING_BITS) | SCANNING_RED);
            else if((c_result == BLUE) && (d_result == FIRST_CASE))
                set_gamestates((get_gamestates() & ~SCANNING_BITS) | SCANNING_BLUE);
            else if((c_result == NONE) || (d_result != FIRST_CASE))
                set_gamestates((get_gamestates() & ~SCANNING_BITS) | SCANNING_NONE);

            chThdSleepMilliseconds(500); // to make leds' color visible at least 500 ms
            set_gamestates((get_gamestates() & ~SCANNING_BITS) | SCANNING_NOT_SCANNING);

            switch(position->t) {
                case E:
                    gameMap[position->x+1][position->y].state &= ~KNOWLEDGE_BITS;
                    break;
                case W:
                    gameMap[position->x-1][position->y].state &= ~KNOWLEDGE_BITS;
                    break;
                case N:
                    gameMap[position->x][position->y-1].state &= ~KNOWLEDGE_BITS;
                    break;
                case S:
                    gameMap[position->x][position->y+1].state &= ~KNOWLEDGE_BITS;
                    break;
            }

            int c = 0;
            if(c_result == BLUE)
                c = CELL_OCCUPED_BLUE;
            else if(c_result == RED)
                c = CELL_OCCUPED_RED;
            
            if(d_result == FIRST_CASE) {
                switch(position->t) {
                    case E:
                        gameMap[position->x+1][position->y].state |= c;
                        break;
                    case W:
                        gameMap[position->x-1][position->y].state |= c;
                        break;
                    case N:
                        gameMap[position->x][position->y-1].state |= c;
                        break;
                    case S:
                        gameMap[position->x][position->y+1].state |= c;
                        break;
                }
            }
        }
    }
}