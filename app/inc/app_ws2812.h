#ifndef __APP_WS2812_H__
#define __APP_WS2812_H__

#include "gd32f4xx.h"
#include "ws2812.h"


/**
 * @brief 定义WS2812灯数据结构
 */
typedef struct {
    uint8_t status;         // 灯的状态：0表示关闭，1表示开启
    uint8_t red;            // 红色亮度值
    uint8_t green;          // 绿色亮度值
    uint8_t blue;           // 蓝色亮度值
    int brightness;     // 灯的整体亮度值
    uint32_t grb;            // GRB模式，即绿红蓝排列
}ws2812Data;

void app_ws2812Control(uint8_t mode,uint8_t adjust);

void app_ws2812_Set_light(ws2812Data* ws2812handle,int light);

void app_ws2812_Set_color(ws2812Data* ws2812handle,uint8_t red, uint8_t green, uint8_t blue);

void app_ws2812_set(ws2812Data* ws2812handle);

void app_ws2812_set_all(ws2812Data* ws2812handle);

void app_ws2812_update_all(void);

void app_ws2812_set_all_color(uint8_t red, uint8_t green, uint8_t blue);

uint32_t app_ws2812_hsv_to_rgb(uint8_t hue, uint8_t sat, uint8_t val);

void app_ws2812_breath(uint8_t red, uint8_t green, uint8_t blue, int duration, int interval);

void app_ws2812_rainbow_cycle( int duration, int interval);

void app_ws2812_flash(uint8_t red, uint8_t green, uint8_t blue, int duration, int interval);

#endif
