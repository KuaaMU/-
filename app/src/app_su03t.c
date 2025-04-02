#include "app_su03t.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "media_task.h"
#include "execution_task.h"
#include "communication_task.h"
#include <string.h>
#include <stdio.h>
#include "su03t.h" // SU03T智能语音模块驱动
#include "app_fpm383.h"

extern uint8_t app_fpm383_state;

extern TaskHandle_t xTaskMediaHandle;
extern TaskHandle_t xTaskFpm383Handle;

extern TaskHandle_t xTaskSu03tHandle;

extern QueueHandle_t xQueueSu03t; // 用于从ISR向任务传递数据的队列

Su03tData su03t_rxDATA;
Su03tData su03t_rxDATA_buffer;

extern DeviceStatus GolDeviceStatus;

void my_su03t_recv_callback(uint8_t *data, uint8_t len)
{
    // 将接收到的数据长度存储在su03t_rxDATA中
    su03t_rxDATA.rx_len = len;
    memcpy(su03t_rxDATA.rxDATA, data, len);
    su03t_receive_clear();
    // 将数据发送到队列
    if (xQueueSendFromISR(xQueueSu03t, &su03t_rxDATA.rxDATA, pdFALSE) != pdPASS)
    {
        // 处理队列满的情况
        // 清除队列，重新发送
        xQueueReset(xQueueSu03t);
        xQueueSendFromISR(xQueueSu03t, &su03t_rxDATA.rxDATA, pdFALSE);
    }

    vTaskNotifyGiveFromISR(xTaskSu03tHandle, pdFALSE);
}

void app_su03t_proc(Su03tData *data)
{
    // 检查数据包的有效性
    if (data->rxDATA[0] == 0xAA &&
        data->rxDATA[1] == 0x55 &&
        data->rxDATA[6] == 0x55 &&
        data->rxDATA[7] == 0xAA)
    {

        uint8_t position = data->rxDATA[2];
        uint8_t device = data->rxDATA[3];
        uint8_t mode = data->rxDATA[4];
        uint8_t adjust = data->rxDATA[5];

        LOG("Position: %02X, Device: %02X, Mode: %02X, Adjust: %02X\r\n", position, device, mode, adjust);

        switch (position)
        {
        case 0x00: // 通用模式
            handleCommon(device, mode, adjust);
            break;
        case 0x01: // 客厅
            handleLivingRoom(device, mode, adjust);
            break;
        case 0x02: // 卧室
            handleBedroom(device, mode, adjust);
            break;
        case 0x03: // 厨房
            handleKitchen(device, mode, adjust);
            break;
        case 0x04: // 浴室
            handleBathroom(device, mode, adjust);
            break;
        default:

            break;
        }
    }
}

void app_su03tControl(uint8_t mode, uint8_t adjust)
{
    switch (mode)
    {
    case 0x01:
        switch (adjust)
        {
        case 0x01:
            app_fpm383_state = 0x01;
            break;

        case 0x02:
            app_fpm383_state = 0x02;
            break;

        case 0x03:
            app_fpm383_state = 0x03;
            break;
        }
        break;
    }
    xTaskNotifyGive(xTaskFpm383Handle);
}
