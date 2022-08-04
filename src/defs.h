#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

/*
 * Differences between angles are 2 to able the possible extension toward 4 more directions :
 * NE, NW, SE, SW
 */
enum angle {
    E=10,
    N=12,
    W=14,
    S=16,
};
typedef enum angle angle_t;

enum step { STOP=0, FORWARD, BACKWARD, LEFT, RIGHT };
typedef enum step step_t;

struct cell {
    /*
     * the state variable store all the different states of the cell :
     * bit 0-1 : free = 00 / occuped red = 01 / occuped blue = 10
     * bit 2-3 : blank = 00 / open = 01 / closed = 10
     * bit 4   : unknown = 0 / known = 1
     */
    uint16_t state;
    /*
     * f_score = g_score + H_score,
     * g_score : distance travelled from the origin cell
     * h_score : estimated minimal distance to the destination cell
     */
    uint8_t f_score, g_score;
    struct cell* parent;
};
typedef struct cell cell_t;

struct coord {
    int8_t x,y;
    angle_t t;
};
typedef struct coord coord_t;


#define FIRST_CASE                  1
#define SECOND_CASE                 2
#define THIRD_CASE                  3
#define OUT_MAP                     0xFF

/* Those defines are used both to designate :
 *  -scanning color : RED, BLUE or NONE
 *  -winner : RED, BLUE, NONE or DRAW (match nul)
 */
#define RED                         0
#define BLUE                        1
#define NONE                        2
#define DRAW                        3

#define MAX_PATH_SIZE               30
#define SIDE_NCELL                  9
#define GAMEMAP_CENTER              ((SIDE_NCELL - 1) / 2)
#define CELL_WIDTH                  100 // in mm

#define NSTEP_ONE_TURN              1000 // number of full steps for 1 turn of the motor
#define PI                          3.14159
#define WHEEL_DIAMETER              41 // in mm
#define WHEEL_SPACE                 53 // in mm
#define STEPS_TURN_90               323
#define NSTEPS_HALF_CELL            (CELL_WIDTH / (WHEEL_DIAMETER * PI) * NSTEP_ONE_TURN / 2)
#define STEP_SPEED_STRAIGHT         400
#define STEP_SPEED_TURN             325

/*
 * The 3 following defines are commented to show the derivation of of NTURNS_TURN_360
 */
//#define SPACE_PERIMETER (WHEEL_SPACE * PI)
//#define WHEEL_PERIMETER (WHEEL_DIAMETER * PI)
//#define NTURNS_TURN_360 (SPACE_PERIMETER / WHEEL_PERIMETER)
//#define NTURNS_TURN_360     (WHEEL_SPACE / WHEEL_DIAMETER)
//#define STEPS_TURN_360      (NTURNS_TURN_360 * NSTEP_ONE_TURN)

#define OBSTRUCTION_BITS            ((uint16_t) (1 << 1) | (1 << 0))
#define CELL_FREE                   ((uint16_t) (0 << 1) | (0 << 0))
#define CELL_OCCUPED                ((uint16_t) (0 << 1) | (1 << 0)) // Deprecated
#define CELL_OCCUPED_RED            ((uint16_t) (1 << 1) | (0 << 0))
#define CELL_OCCUPED_BLUE           ((uint16_t) (1 << 1) | (1 << 0))
#define PATH_FIND_BITS              ((uint16_t) (1 << 3) | (1 << 2))
#define CELL_BLANK                  ((uint16_t) (0 << 3) | (0 << 2))
#define CELL_OPEN                   ((uint16_t) (0 << 3) | (1 << 2))
#define CELL_CLOSED                 ((uint16_t) (1 << 3) | (0 << 2))
#define CELL_UNKNOWN                ((uint16_t) (1 << 4))
#define CELL_KNOWN                  ((uint16_t) (0 << 4))
#define KNOWLEDGE_BITS              ((uint16_t) (1 << 4))

#define PATH_UNIT_LENGTH 10
/*
 * LookUpTable listing nearest cell (delta_x, delta_y)
 * some A star implementation allow diagonal crossing of cells
 * for sake of simplicity only horizontal and vertical crossing is possible
 */

#define DIFFICULTY_BITS             ((uint8_t) (1 << 0))
#define DIFFICULTY_EASY             ((uint8_t) (0 << 0))
#define DIFFICULTY_HARD             ((uint8_t) (1 << 0))
#define STATE_BITS                  ((uint8_t) (1 << 2) | (1 << 1))
#define STATE_START                 ((uint8_t) (0 << 2) | (0 << 1))
#define STATE_PLAYING               ((uint8_t) (0 << 2) | (1 << 1))
#define STATE_WAITING_PLAYER        ((uint8_t) (1 << 2) | (0 << 1))
#define STATE_END                   ((uint8_t) (1 << 2) | (1 << 1))
#define SCANNING_BITS               ((uint8_t) (1 << 4) | (1 << 3))
#define SCANNING_NOT_SCANNING       ((uint8_t) (0 << 4) | (0 << 3))
#define SCANNING_RED                ((uint8_t) (0 << 4) | (1 << 3))
#define SCANNING_BLUE               ((uint8_t) (1 << 4) | (0 << 3))
#define SCANNING_NONE               ((uint8_t) (1 << 4) | (1 << 3))
#define WINNER_BITS                 ((uint8_t) (1 << 6) | (1 << 5))
#define WINNER_NONE                 ((uint8_t) (0 << 6) | (0 << 5))
#define WINNER_RED                  ((uint8_t) (0 << 6) | (1 << 5))
#define WINNER_BLUE                 ((uint8_t) (1 << 6) | (0 << 5))
#define WINNER_DRAW                 ((uint8_t) (1 << 6) | (1 << 5))
#define LED_ON                      1
#define LED_OFF                     0

// USER_INTERACTION (~Infrared sensors) related #defines
#define SENSOR_IR_THRESHOLD         10
#define IR_1                        0
#define IR_2                        1
#define IR_3                        6
#define IR_4                        3
#define IR_4                        3
#define HOLD_LONG                   5000 // in ms
#define HOLD_SHORT                  2000 // in ms

#define SENSOR_IR_REFRESH_T         100 // in ms
#define SENSOR_IR_PRIORITY          (NORMALPRIO + 1)
#define SENSOR_DISTANCE_REFRESH_T   100 // in ms
#define SENSOR_DISTANCE_PRIORITY    (NORMALPRIO + 1)

#define IMAGE_BUFFER_SIZE		    640

/*
 * Used in path.c and game.c
 * those variables are static const and are used in multiple files,
 * -> it's ok for them to be global
 */
static const coord_t nearest[4] = {
    {-1,  0, E},
    {+1,  0, W},
    { 0, -1, S},
    { 0, +1, N}
};

// Is used in movements.c and game.c
static const coord_t storage[12] = {
    { GAMEMAP_CENTER - 3, GAMEMAP_CENTER - 1, E},
    { GAMEMAP_CENTER - 3, GAMEMAP_CENTER + 1, E},
    { GAMEMAP_CENTER - 3, GAMEMAP_CENTER    , E},

    { GAMEMAP_CENTER + 3, GAMEMAP_CENTER - 1, W},
    { GAMEMAP_CENTER + 3, GAMEMAP_CENTER    , W},
    { GAMEMAP_CENTER + 3, GAMEMAP_CENTER + 1, W},

    { GAMEMAP_CENTER - 1, GAMEMAP_CENTER - 3, S},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER - 3, S},
    { GAMEMAP_CENTER + 1, GAMEMAP_CENTER - 3, S},

    { GAMEMAP_CENTER - 1, GAMEMAP_CENTER + 3, N},
    { GAMEMAP_CENTER    , GAMEMAP_CENTER + 3, N},
    { GAMEMAP_CENTER + 1, GAMEMAP_CENTER + 3, N}
};

uint8_t getDistance(coord_t A, coord_t B);

#endif /*DEFS_H*/