#ifndef __APP_DELAY_H
#define __APP_DELAY_H

#include "gd32f4xx.h"
#include "relay.h"

/**
 * @brief 定义继电器数据结构
 */
typedef struct
{
    uint8_t status; // 继电器状态：0表示关闭，1表示开启
} relayData;

void app_relayControl(uint8_t device, uint8_t mode, uint8_t adjust);
#endif
