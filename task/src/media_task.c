#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "media_task.h"
#include <string.h>
#include <stdio.h>
#include "app_su03t.h" // SU03T智能语音模块驱动

// 任务优先级定义
#define TASK_SU03T_PRIORITY (configMAX_PRIORITIES - 2)

// 任务堆栈大小定义
#define TASK_SU03T_STACK_SIZE configMINIMAL_STACK_SIZE * 10

// 外部任务句柄声明
extern TaskHandle_t xTaskMediaHandle;

// SU03T任务句柄声明
TaskHandle_t xTaskSu03tHandle;

// 用于从ISR向任务传递数据的队列声明
QueueHandle_t xQueueSu03t;

// 外部声明的SU03T数据结构
extern Su03tData su03t_rxDATA;
extern Su03tData su03t_rxDATA_buffer;

/**
 * @brief 创建并初始化SU03T任务
 *
 * @param argument 任务参数，本任务中未使用
 */
void MediaTask(void *argument)
{
    // 创建用于从ISR向任务传递数据的队列
    xQueueSu03t = xQueueCreate(3, sizeof(su03t_rxDATA.rxDATA));

    // 进入临界区，防止多任务环境下创建任务的过程被中断
    taskENTER_CRITICAL();
    // 创建SU03T接收任务
    xTaskCreate(
        Su03tTask,
        "su03t_receive_task",
        configMINIMAL_STACK_SIZE,
        NULL,
        TASK_SU03T_PRIORITY,
        &xTaskSu03tHandle);

    // 退出临界区
    taskEXIT_CRITICAL();

    // 删除当前任务，因为初始化完成后不再需要
    vTaskDelete(NULL);
}

/**
 * @brief SU03T任务主循环
 *
 * @param argument 任务参数，本任务中未使用
 */
void Su03tTask(void *argument)
{
    // 初始化媒体设备
    su03t_init(115200, my_su03t_recv_callback);

    while (1)
    {
        // 等待通知或超时
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 从队列中读取数据
        if (xQueueReceive(xQueueSu03t, &su03t_rxDATA_buffer.rxDATA, portMAX_DELAY) == pdPASS)
        {
            // 处理接收到的数据
            app_su03t_proc(&su03t_rxDATA_buffer);
        }
    }
}
