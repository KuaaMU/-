/**
 * @file    ws2812.c
 * @brief   WS2812 灯珠控制
 * @details WS2812采用单总线通讯方式，每颗灯珠支持 24 bit 的颜色控制 (按 GRB 顺序各 8 位)。
 *          信号线通过 DIN 输入，经过一颗灯珠之后，信号线上前 24 bit 数据会被该灯珠锁存，
 *          之后将剩下的数据信号整形之后通过 DOUT 输出给下一颗灯珠。
 *
 * @version 2024-08-07, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "systick.h"
#include "ws2812.h"



/**
 * @brief   WS2812 模块用到的延时函数
 *
 * @param   xns: 延时数 (在GD32F407上，示波器测量延时时间约等于 xns * 20ns)
 */
void delay_20ns(uint32_t xns)
{
    while(xns--); // 一次循环约20ns
}


/**
 * @brief   WS2812 模块引脚配置及初始化
 *
 */
void ws2812_init(void)
{
    rcu_periph_clock_enable(ws2812_RCU);

    gpio_mode_set(ws2812_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ws2812_PIN);
    gpio_output_options_set(ws2812_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ws2812_PIN);

    gpio_bit_reset(ws2812_PORT, ws2812_PIN);
}


/**
 * @brief   设置引脚高电平
 *
 */
void ws2812_set_high(void)
{
    gpio_bit_set(ws2812_PORT, ws2812_PIN);
}


/**
 * @brief   设置引脚低电平
 *
 */
void ws2812_set_low(void)
{
    gpio_bit_reset(ws2812_PORT, ws2812_PIN);
}


/**
 * @brief   向 WS2812 发送一个 bit 0
 *
 */
void ws2812_send_0(void)
{
    ws2812_set_high();
    delay_20ns(15);

    ws2812_set_low();
    delay_20ns(30);
}


/**
 * @brief   向 WS2812 发送一个 bit 1
 *
 */
void ws2812_send_1(void)
{
    ws2812_set_high();
    delay_20ns(30);

    ws2812_set_low();
    delay_20ns(15);
}


/**
 * @brief   向 WS2812 发送 RESET 信号
 *
 * @remark  (实测中发现 RESET 无效，暂未发现原因)
 *
 */
void ws2812_send_reset(void)
{
    ws2812_set_low();
    delay_1us(30);

    ws2812_set_high();
    ws2812_set_low();
}


/**
 * @brief   点亮一个 WS2812 灯珠
 *
 * @param   grb: 24位数据，按 GRB 顺序每 8 位控制一种颜色
 */
void ws2812_send_one(uint32_t grb)
{
    uint8_t byte;

    for (int8_t i = 23; i >= 0; i--) {
        byte = (grb >> i) & 0x01;

        if (byte == 1) {
            ws2812_send_1();
        } else {
            ws2812_send_0();
        }
    }
}


/**
 * @brief   设置 WS2812 灯珠的颜色
 *
 * @param     red: 8 位红色值
 * @param   green: 8 位绿色值
 * @param    blue: 8 位蓝色值
 */
void ws2812_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t grb = (green << 16) | (red << 8) | blue;
    ws2812_send_one(grb);
}


void ws2812_set_light(uint8_t light)
{

}


void ws2812_demo(void)
{
    ws2812_init();

    ws2812_set_color(0xff, 0xff, 0);
}
