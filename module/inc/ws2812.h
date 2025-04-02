#ifndef __WS2812_H
#define __WS2812_H

#include "gd32f4xx.h"

#define ws2812_RCU		    RCU_GPIOE
#define ws2812_PORT			GPIOE
#define ws2812_PIN			GPIO_PIN_14

void ws2812_init(void);
void ws2812_demo(void);
void ws2812_send_reset(void);
void ws2812_send_one(uint32_t grb);
void ws2812_set_color(uint8_t red, uint8_t green, uint8_t blue);
void ws2812_set_light(uint8_t light);

#endif
