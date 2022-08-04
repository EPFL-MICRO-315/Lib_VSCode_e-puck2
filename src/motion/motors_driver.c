#include <ch.h>
#include <hal.h>
#include "motors_driver.h"
#include <chconf.h>
#define MOTOR_SPEED_LIMIT 1100 // [step/s]
#define MOTOR_TIMER_FREQ 100000 // [Hz]
#define THRESV 650 // This is the speed under which the power save feature is active.

static const uint8_t step_halt[4] = {0, 0, 0, 0};
//table of the differents steps of to control the motors
//it corresponds to microsteps.
//in the reality two microsteps correspond to 1 step of the motor.
static const uint8_t step_table[8][4] = {
    {1, 0, 1, 0},
	{0, 0, 1, 0},
    {0, 1, 1, 0},
	{0, 1, 0, 0},
    {0, 1, 0, 1},
	{0, 0, 0, 1},
    {1, 0, 0, 1},
	{1, 0, 0, 0},
};

struct stepper_motor_s {
    uint8_t step_index; //vary form 0 to 7
    int32_t steps;  //number of steps to be executed, in microsteps
    void (*update)(const uint8_t *out);
    void (*enable_power_save)(void);
    void (*disable_power_save)(void);
    PWMDriver *timer;
    uint32_t speed;
};

struct stepper_motor_s right_motor;
struct stepper_motor_s left_motor;

static void motor_set_speed(struct stepper_motor_s *m, uint32_t speed);
static void right_motor_update(const uint8_t *out);
static void right_motor_timer_callback(PWMDriver *gptp);
static void right_motor_pwm_ch1_cb(PWMDriver *pwmp);
static void left_motor_update(const uint8_t *out);
static void left_motor_timer_callback(PWMDriver *gptp);
static void left_motor_pwm_ch1_cb(PWMDriver *pwmp);

void motors_init(void)
{
    /* motor struct init */
    right_motor.step_index = 0;
    right_motor.steps = 0;
    right_motor.update = right_motor_update;
    right_motor.enable_power_save = right_motor_enable_power_save;
    right_motor.disable_power_save = right_motor_disable_power_save;
    right_motor.timer = &PWMD3;

    left_motor.step_index = 0;
    left_motor.steps = 0;
    left_motor.update = left_motor_update;
    left_motor.enable_power_save = left_motor_enable_power_save;
    left_motor.disable_power_save = left_motor_disable_power_save;
    left_motor.timer = &PWMD4;

    /* motor init halted*/
    right_motor_update(step_halt);
    left_motor_update(step_halt);

    /* timer init */
    static const PWMConfig pwmcfg_right_motor = {
        .frequency = MOTOR_TIMER_FREQ, // frequency of the pwm clock
        .period = 0xFFFF, //number of ticks of the pwm clock to trigger an interruption
        .cr2 = 0,
        .callback = right_motor_timer_callback,
        .channels = {
            // Channel 1 is used to handling motor power save feature.
            {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = right_motor_pwm_ch1_cb },
            {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
            {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
            {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
        },
    };
    pwmStart(&PWMD3, &pwmcfg_right_motor);
    pwmEnablePeriodicNotification(&PWMD3); // PWM general interrupt at the beginning of the period to handle motor steps.

    static const PWMConfig pwmcfg_left_motor = {
        .frequency = MOTOR_TIMER_FREQ,
        .period = 0xFFFF,
        .cr2 = 0,
        .callback = left_motor_timer_callback,
        .channels = {
            // Channel 1 is used to handling motor power save feature.
            {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = left_motor_pwm_ch1_cb},
            {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
            {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
            {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
        },
    };
    pwmStart(&PWMD4, &pwmcfg_left_motor);
    pwmEnablePeriodicNotification(&PWMD4); // PWM general interrupt at the beginning of the period to handle motor steps.
}

void right_motor_move(int32_t steps)
{
    right_motor.steps = steps * 2;
}

void right_motor_stop(void)
{
    right_motor.steps = 0;
}

void right_motor_set_speed(uint32_t speed)
{
    motor_set_speed(&right_motor, speed);
}

void right_motor_enable_power_save(void)
{
    //Enable channel 1 to set duty cycle for power save.
    pwmEnableChannel(&PWMD3, 0, (pwmcnt_t) (MOTOR_TIMER_FREQ/THRESV)); 
    //Channel 1 interrupt enable to handle motor shutdown.
	pwmEnableChannelNotification(&PWMD3, 0); 
}

void right_motor_disable_power_save(void)
{
	pwmDisableChannel(&PWMD3, 0);
}


void left_motor_move(int32_t steps)
{
    left_motor.steps = steps * 2;
}

void left_motor_stop(void)
{
    left_motor.steps = 0;
}

void left_motor_set_speed(uint32_t speed)
{
    motor_set_speed(&left_motor, speed);
}

void left_motor_enable_power_save(void)
{
    //Enable channel 1 to set duty cycle for power save.
    pwmEnableChannel(&PWMD4, 0, (pwmcnt_t) (MOTOR_TIMER_FREQ/THRESV)); 
    //Channel 1 interrupt enable to handle motor shutdown.
	pwmEnableChannelNotification(&PWMD4, 0); 
}

void left_motor_disable_power_save(void)
{
	pwmDisableChannel(&PWMD4, 0);
}

static void motor_set_speed(struct stepper_motor_s *m, uint32_t speed)
{
    /* limit motor speed */
    if (speed > MOTOR_SPEED_LIMIT)
        speed = MOTOR_SPEED_LIMIT;
    m->speed = speed;
    //twice the speed because we are doing microsteps,
    //which doubles the steps necessary to do one real step of the motor
    speed *=2;

    uint16_t interval;
    if (speed < THRESV) {
        interval = 1000;
    //    m->enable_power_save();
    }
    else {
        interval = MOTOR_TIMER_FREQ / speed;
    //    m->disable_power_save();
    }

    /* change motor step interval */
    pwmChangePeriod(m->timer, interval);
}

static void right_motor_update(const uint8_t *out)
{
    out[0] ? palSetPad(GPIOE, GPIOE_MOT_R_IN1) : palClearPad(GPIOE, GPIOE_MOT_R_IN1);
    out[1] ? palSetPad(GPIOE, GPIOE_MOT_R_IN2) : palClearPad(GPIOE, GPIOE_MOT_R_IN2);
    out[2] ? palSetPad(GPIOE, GPIOE_MOT_R_IN3) : palClearPad(GPIOE, GPIOE_MOT_R_IN3);
    out[3] ? palSetPad(GPIOE, GPIOE_MOT_R_IN4) : palClearPad(GPIOE, GPIOE_MOT_R_IN4);
}

static void right_motor_timer_callback(PWMDriver *gptp)
{
    (void) gptp;
    uint8_t i;
    if (right_motor.steps > 0) {
        i = (right_motor.step_index - 1) & 7;
        right_motor.update(step_table[i]);
        right_motor.steps--;
        right_motor.step_index = i;
    } else if (right_motor.steps < 0) {
        i = (right_motor.step_index + 1) & 7;
        right_motor.update(step_table[i]);
        right_motor.steps++;
        right_motor.step_index = i;
    } else {
        right_motor.update(step_halt);
    }    
}

static void right_motor_pwm_ch1_cb(PWMDriver *pwmp)
{
 	(void)pwmp;
    palClearPad(GPIOE, GPIOE_MOT_R_IN1);
    palClearPad(GPIOE, GPIOE_MOT_R_IN2);
    palClearPad(GPIOE, GPIOE_MOT_R_IN3);
    palClearPad(GPIOE, GPIOE_MOT_R_IN4);
}


static void left_motor_update(const uint8_t *out)
{
    out[0] ? palSetPad(GPIOE, GPIOE_MOT_L_IN1) : palClearPad(GPIOE, GPIOE_MOT_L_IN1);
    out[1] ? palSetPad(GPIOE, GPIOE_MOT_L_IN2) : palClearPad(GPIOE, GPIOE_MOT_L_IN2);
    out[2] ? palSetPad(GPIOE, GPIOE_MOT_L_IN3) : palClearPad(GPIOE, GPIOE_MOT_L_IN3);
    out[3] ? palSetPad(GPIOE, GPIOE_MOT_L_IN4) : palClearPad(GPIOE, GPIOE_MOT_L_IN4);
}

static void left_motor_timer_callback(PWMDriver *gptp)
{
    (void) gptp;
    uint8_t i;
    if (left_motor.steps > 0) {
        i = (left_motor.step_index + 1) & 7;
        left_motor.update(step_table[i]);
        left_motor.steps--;
        left_motor.step_index = i;
    } else if (left_motor.steps < 0) {
        i = (left_motor.step_index - 1) & 7;
        left_motor.update(step_table[i]);
        left_motor.steps++;
        left_motor.step_index = i;
    } else {
        left_motor.update(step_halt);
    }    
}

static void left_motor_pwm_ch1_cb(PWMDriver *pwmp)
{
 	(void)pwmp;
    palClearPad(GPIOE, GPIOE_MOT_L_IN1);
    palClearPad(GPIOE, GPIOE_MOT_L_IN2);
    palClearPad(GPIOE, GPIOE_MOT_L_IN3);
    palClearPad(GPIOE, GPIOE_MOT_L_IN4);
}

int32_t left_motor_steps_left(void)
{
    return left_motor.steps;
}

int32_t right_motor_steps_left(void)
{
    return right_motor.steps;
}
