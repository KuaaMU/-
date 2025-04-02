#ifndef __APP_MOTOR_H
#define __APP_MOTOR_H

#include "gd32f4xx.h"
#include "motor.h"

/**
 * @brief 定义直流电机数据结构
 */
typedef struct
{
    uint8_t status;  // 直流电机状态：0表示关闭，1表示开启
    uint8_t percent; // 电机速度占空比，范围0-100
} DCmotorData;

void app_DCmotorControl(uint8_t mode, uint8_t adjust);

#endif
