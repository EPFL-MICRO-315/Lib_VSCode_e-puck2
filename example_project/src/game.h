#ifndef GAME_H
#define GAME_H

#include "defs.h"
#include <ch.h>

void game_init(cell_t gameMap[SIDE_NCELL][SIDE_NCELL]);
uint8_t get_gamestates(void);
void set_gamestates(uint8_t new_gamestates);

/*
 * 2 strategy of the Epuck to place a sign in a case. Here in easy mode.
 * The Epuck plays randomly, should be easy to beat it.
 * The Epcuck plays randomly but it emphasis on taking the most intesting cells.
 */
coord_t place_easy(cell_t gameMap[SIDE_NCELL][SIDE_NCELL]);
coord_t place_hard(cell_t gameMap[SIDE_NCELL][SIDE_NCELL]);

uint8_t search_winner(cell_t gameMap[SIDE_NCELL][SIDE_NCELL]);
void check_end_game(cell_t gameMap[SIDE_NCELL][SIDE_NCELL]);

void show_stateofgame(void);

#endif /*GAME_H*/