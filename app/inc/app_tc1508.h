#ifndef __APP_TC1508_H
#define __APP_TC1508_H

#include "gd32f4xx.h"
#include "systick.h"
#include "bsp_basic_timer.h"
#include "tc1508.h"

/**
 * @brief 定义TC1508编码器驱动的步进电机数据结构
 */
typedef struct
{
    uint8_t status;    // 电机状态：0表示关闭，1表示开启
    uint8_t direction; // 电机转动方向
    int step;     // 电机步进数
    int angle;    // 电机转动角度
    uint32_t delay;    // 电机转动延时
} tc1508Data_Stepper;

/**
 * @brief 定义TC1508编码器驱动的直流电机数据结构
 */
typedef struct
{
    uint8_t status;    // 电机状态：0表示关闭，1表示开启
    uint8_t direction; // 电机转动方向
    uint16_t speed;    // 电机转动速度
} tc1508Data_DC;

// 驱动步进电机
#if STEPPER_MOTOR

void app_tc1508Control(uint8_t mode, uint8_t adjust);

// 驱动直流电机
#else

#endif

#endif
