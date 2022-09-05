/**
 * @file motors_driver.h
 * @author Antoine Martin (martantoine@tuta.io)
 * @brief Largely copied from the e-puck2-main-processor's motors library
 * modified to handle position control of the motor (rather than not speed control)
 * @version 0.1
 * @date 2022-05-08
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef MOTORS_DRIVER_H
#define MOTORS_DRIVER_H

#include <stdint.h>

void motors_init(void);

/**
 * @brief Set the number of steps to be executed
 *
 * @param steps 1000 steps : 1 motor turn, negative number for reverse
 */
void right_motor_move(int32_t steps);

int32_t right_motor_steps_left(void);

/**
 * @brief Set the number of steps to be executed to 0
 *
 */
void right_motor_stop(void);

/**
 * @brief Set the speed
 *
 * @param speed step/s, no negative number
 */
void right_motor_set_speed(uint32_t speed);

/**
 * @brief Re-enable all right motor's functionality
 *
 */
void right_motor_enable_power_save(void);

/**
 * @brief Disable right motor's PWM clock and stop sending current in its coils
 *
 */
void right_motor_disable_power_save(void);

/**
 * @brief Set the number of steps to be executed
 *
 * @param steps 1000 steps : 1 motor turn, negative number for reverse
 */
void left_motor_move(int32_t steps);

int32_t left_motor_steps_left(void);

/**
 * @brief Set the number of steps to be executed to 0
 *
 */
void left_motor_stop(void);

/**
 * @brief Set the speed
 *
 * @param speed step/s, no negative number
 */
void left_motor_set_speed(uint32_t speed);

/**
 * @brief Re-enable all left motor's functionality
 *
 */
void left_motor_enable_power_save(void);

/**
 * @brief Disable left motor's PWM clock and stop sending current in its coils
 *
 */
void left_motor_disable_power_save(void);

#endif /*MOTORS_DRIVER_H*/