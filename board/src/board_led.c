/**
 * @file    board_led.c
 * @brief   板载 LED 的驱动
 *
 * @version 2024-07-23, V1.0, yanf, 厦门芯力量
 */


#include "gd32f4xx.h"
#include "board_led.h"


/**
 * @brief   板载 LED GPIO 初始化
 *
 */
void led_onboard_init(void)
{
    rcu_periph_clock_enable(LED_OB_RCU);

    gpio_mode_set(LED_OB_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OB_PIN);
    gpio_output_options_set(LED_OB_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_OB_PIN);

    gpio_bit_reset(LED_OB_PORT, LED_OB_PIN);
}


/**
 * @brief   点亮 LED
 *
 */
void led_onboard_on(void)
{
    gpio_bit_set(LED_OB_PORT, LED_OB_PIN);
}


/**
 * @brief   熄灭 LED
 *
 */
void led_onboard_off(void)
{
    gpio_bit_reset(LED_OB_PORT, LED_OB_PIN);
}


/**
 * @brief   切换 LED 的亮灭
 *
 */
void led_onboard_toggle(void)
{
    gpio_bit_toggle(LED_OB_PORT, LED_OB_PIN);
}
