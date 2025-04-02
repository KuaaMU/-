/**
 * @file    bsp_led.c
 * @brief   LED 驱动板级支持包
 *
 * @version 2024-08-07, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "bsp_led.h"

static const rcu_periph_enum    LED_CLK[LEDn]   = { LED1_CLK, LED2_CLK };
static const        uint32_t    LED_PORT[LEDn]  = { LED1_PORT, LED2_PORT };
static const        uint32_t    LED_PIN[LEDn]   = { LED1_PIN, LED2_PIN };


/**
 * @brief   单个 LED 引脚配置及初始化
 *
 * @param   led: LED 编号
 */
static void bsp_led_gpio_init(led_type_def led)
{
    rcu_periph_clock_enable(LED_CLK[led]);

    gpio_mode_set(LED_PORT[led], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN[led]);
    gpio_output_options_set(LED_PORT[led], GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_PIN[led]);
}


/**
 * @brief   全部 LED 引脚配置及初始化
 *
 */
void bsp_led_init(void)
{
    for (led_type_def _led = LED1; _led < LEDn; _led++) {
        bsp_led_gpio_init(_led);
        bsp_led_off(_led);
    }
}


/**
 * @brief   点亮指定 LED
 *
 * @param   led: LED 编号
 */
void bsp_led_on(led_type_def led)
{
    gpio_bit_set(LED_PORT[led], LED_PIN[led]);
}


/**
 * @brief   熄灭指定 LED
 *
 * @param   led: LED 编号
 */
void bsp_led_off(led_type_def led)
{
    gpio_bit_reset(LED_PORT[led], LED_PIN[led]);
}


/**
 * @brief   翻转亮灭指定 LED
 *
 * @param   led: LED 编号
 */
void bsp_led_toggle(led_type_def led)
{
    gpio_bit_toggle(LED_PORT[led], LED_PIN[led]);
}


/**
 * @brief   获取指定 LED 状态
 *
 * @param   led: LED 编号
 * @return uint8_t 返回 LED 的亮灭状态 (1:亮 2:灭)
 */
uint8_t bsp_led_get_status(led_type_def led)
{
    return (RESET == gpio_input_bit_get(LED_PORT[led], LED_PIN[led])) ? 0 : 1;
}
