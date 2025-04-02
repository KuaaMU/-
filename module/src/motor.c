/**
 * @file    motor.c
 * @brief   普通直流电机 PWM 驱动
 *
 * @version 2024-08-07, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "bsp_pwm.h"
#include "motor.h"


/**
 * @brief   电机引脚配置及初始化
 *
 */
void motor_init(void)
{
    rcu_periph_clock_enable(MOTOR_RCU);

	gpio_mode_set(MOTOR_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MOTOR_PIN);
	gpio_output_options_set(MOTOR_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, MOTOR_PIN);
	gpio_af_set(MOTOR_PORT, GPIO_AF_1, MOTOR_PIN);

    gpio_bit_reset(MOTOR_PORT, MOTOR_PIN);

    bsp_timer0_pwm_init(1000, 0, MOTOR_TIMER_CH, NULL);
}


/**
 * @brief   通过设置 PWM 的占空比来调整电机转速
 *
 * @param   percent: 百分比值
 */
void motor_set_speed(uint8_t percent)
{
    bsp_timer0_set_pulse(MOTOR_TIMER_CH, percent * 1000 / 100);
}


/**
 * @brief   通过启动 PWM 的定时器来启动电机
 *
 */
void motor_start(void)
{
    bsp_timer0_enable();
}


/**
 * @brief   通过停止 PWM 的定时器来停止电机
 *
 */
void motor_stop(void)
{
    bsp_timer0_disable();
}
