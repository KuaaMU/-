/**
 * @file    relay.c
 * @brief   两路继电器模块
 *
 * @version 2024-08-07, V1.0, yanf
 */

#include "gd32f4xx.h"
#include "systick.h"
#include "relay.h"


/**
 * @brief   继电器 1 引脚配置及初始化
 *
 */
void relay1_init(void) {
    rcu_periph_clock_enable(RELAY1_RCU);

    gpio_mode_set(RELAY1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RELAY1_PIN);
    gpio_output_options_set(RELAY1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, RELAY1_PIN);
}


/**
 * @brief   继电器 2 引脚配置及初始化
 *
 */
void relay2_init(void) {
    rcu_periph_clock_enable(RELAY2_RCU);

    gpio_mode_set(RELAY2_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RELAY2_PIN);
    gpio_output_options_set(RELAY2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, RELAY2_PIN);
}


/**
 * @brief   连通继电器 1
 *
 */
void relay1_on(void)
{
    gpio_bit_set(RELAY1_PORT, RELAY1_PIN);
}


/**
 * @brief   断开继电器 1
 *
 */
void relay1_off(void)
{
    gpio_bit_reset(RELAY1_PORT, RELAY1_PIN);
}


/**
 * @brief   连通继电器 2
 *
 */
void relay2_on(void)
{
    gpio_bit_set(RELAY2_PORT, RELAY2_PIN);
}


/**
 * @brief   断开继电器 2
 *
 */
void relay2_off(void)
{
    gpio_bit_reset(RELAY2_PORT, RELAY2_PIN);
}
