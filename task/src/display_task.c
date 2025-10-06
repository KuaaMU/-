#include "FreeRTOS.h"
#include "task.h"
#include "display_task.h"
#include "app_dwin.h"
#include "media_task.h"
#include "string.h"
#include "stdio.h"
#include "dwin.h" // DWIN串口屏驱动

/**
 * 显示任务函数
 *
 * 该任务负责初始化显示设备，并进行持续的显示更新操作
 * 它使用DWIN串口屏驱动来完成显示相关的操作
 *
 * @param argument 任务参数，未使用
 */
void DisplayTask(void *argument)
{
    // 初始化显示设备
    // dwin
    dwin_init(my_dwin_recv_callback);
    dwin_write(0x0001, 0x0001);

    while (1)
    {
        // 等待通知以进行下一次更新
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // LOG("address: %04X, code: %02X\r\n", dwin_Data.address, dwin_Data.code);

        // 处理DWIN应用层协议
        app_dwin_proc();

        // vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒更新一次
    }
}
