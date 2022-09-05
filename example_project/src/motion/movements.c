#include <ch.h>
#include <stdlib.h>
#include "movements.h"
#include "motors_driver.h"
#include "path.h"

static const coord_t pickup[12] = {
    { GAMEMAP_CENTER - 4, GAMEMAP_CENTER - 1, E},
    { GAMEMAP_CENTER - 4, GAMEMAP_CENTER + 1, E},
    { GAMEMAP_CENTER - 4, GAMEMAP_CENTER    , E},

    { GAMEMAP_CENTER + 4, GAMEMAP_CENTER - 1, W},
    { GAMEMAP_CENTER + 4, GAMEMAP_CENTER    , W},
    { GAMEMAP_CENTER + 4, GAMEMAP_CENTER + 1, W},

    { GAMEMAP_CENTER - 1, GAMEMAP_CENTER - 4, S},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER - 4, S},
    { GAMEMAP_CENTER + 1, GAMEMAP_CENTER - 4, S},

    { GAMEMAP_CENTER - 1, GAMEMAP_CENTER + 4, N},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER + 4, N},
    { GAMEMAP_CENTER + 1, GAMEMAP_CENTER + 4, N}
};

void mvt_init(void) {
    motors_init();
    right_motor_set_speed(STEP_SPEED_STRAIGHT);
    left_motor_set_speed(STEP_SPEED_STRAIGHT);
}

void mvt_executePath(coord_t *position, const coord_t target, step_t *path) {
    uint8_t i = 0;
    while(path[i] != STOP) {
        switch(path[i]) {
            case FORWARD:
                mvt_forward();
                break;
            case BACKWARD:
                mvt_backward();
                break;
            case LEFT:
                mvt_left();
                break;
            case RIGHT:
                mvt_right();
                break;
            default:
                break;
        }
        i++;
    }
    *position = target;
    free(path); //better not forget to free unused memory
}

void mvt_place(cell_t gameMap[SIDE_NCELL][SIDE_NCELL], coord_t *position, const coord_t target) {
    coord_t min, tmp;
    uint8_t min_dist = 0xFF, imin = 0;

    // find among all available pieces the cloest one to the target
    for(uint8_t i = 0; i < 12; i++) {
        tmp = storage[i];
        if((gameMap[tmp.x][tmp.y].state & OBSTRUCTION_BITS) == CELL_OCCUPED_RED) {
            uint8_t dist = getDistance((coord_t){.x=tmp.x, .y=tmp.y, .t=tmp.t}, target);
            if(dist <= min_dist) {
                min = pickup[i];
                min_dist = dist;
                imin = i;
            }
        }
    }
    // go behind the stored piece (in the pickup position)
    step_t *path = findPath(gameMap, *position, min, 1);
    mvt_executePath(position, min, path);
    // go forward 4*(1/2 cell) = 2 cells => is now between the storage ring and the 3x3 playing grid
    mvt_forward();
    mvt_forward();
    mvt_forward();
    mvt_forward();
    switch(position->t) {
        case E: position->x += 2; break;
        case W: position->x -= 2; break;
        case N: position->y -= 2; break;
        case S: position->y += 2; break;
    }
    gameMap[storage[imin].x][storage[imin].y].state &= ~OBSTRUCTION_BITS; // the robot now see the cell free and can go through it
    // search the place to place the piece
    path = findPath(gameMap, *position, target, 0);
    mvt_executePath(position, target, path);
    mvt_forward();
    mvt_backward();
    // mark the cell that received the red piece
    coord_t delta = { 0, 0, E};
    switch(position->t) {
        case E: delta.x += 1; break;
        case W: delta.x -= 1; break;
        case N: delta.y -= 1; break;
        case S: delta.y += 1; break;
    }
    gameMap[target.x+delta.x][target.y+delta.y].state = (gameMap[target.x+delta.x][target.y+delta.y].state & ~OBSTRUCTION_BITS) | CELL_OCCUPED_RED;

    // go back to idling positon
    path = findPath(gameMap, *position, (coord_t){1, 7, N}, 1);
    mvt_executePath(position, (coord_t){1, 7, N}, path);
}

#define WAIT_MOTOR_TARGET_REACHED() \
    while((left_motor_steps_left() != 0) || (right_motor_steps_left() != 0)) \
        chThdSleepMilliseconds(200)

void mvt_forward(void) {
    right_motor_set_speed(STEP_SPEED_STRAIGHT);
    left_motor_set_speed(STEP_SPEED_STRAIGHT);
    left_motor_move(NSTEPS_HALF_CELL);
    right_motor_move(NSTEPS_HALF_CELL);
    WAIT_MOTOR_TARGET_REACHED();
}

void mvt_backward(void) {
    right_motor_set_speed(STEP_SPEED_STRAIGHT);
    left_motor_set_speed(STEP_SPEED_STRAIGHT);
    left_motor_move(-NSTEPS_HALF_CELL);
    right_motor_move(-NSTEPS_HALF_CELL);
    WAIT_MOTOR_TARGET_REACHED();
}

void mvt_left(void) {
    right_motor_set_speed(STEP_SPEED_TURN);
    left_motor_set_speed(STEP_SPEED_TURN);
    left_motor_move(-STEPS_TURN_90);
    right_motor_move(STEPS_TURN_90);
    WAIT_MOTOR_TARGET_REACHED();
}

void mvt_right(void) {
    right_motor_set_speed(STEP_SPEED_TURN);
    left_motor_set_speed(STEP_SPEED_TURN);
    left_motor_move(STEPS_TURN_90);
    right_motor_move(-STEPS_TURN_90);
    WAIT_MOTOR_TARGET_REACHED();
}