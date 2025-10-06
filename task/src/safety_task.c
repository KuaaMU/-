#include "FreeRTOS.h"
#include "task.h"
#include "safety_task.h"
#include "sr602.h" // SR602人体感应模块驱动
#include "flame.h" // 火焰传感器驱动
#include "su03t.h"
#include "app_esp8266.h"
#include "app_fpm383.h"
#include "execution_task.h"
#include "communication_task.h"
// 各个任务间隔时间
#define TASK_SR602_INTERVAL 3000
#define TASK_FLAME_INTERVAL 2000

// 任务优先级定义
#define TASK_FLAME_PRIORITY (configMAX_PRIORITIES - 3)
#define TASK_SR602_PRIORITY (configMAX_PRIORITIES - 3)

// 任务堆栈大小定义
#define TASK_SR602_STACK_SIZE configMINIMAL_STACK_SIZE * 1
#define TASK_FLAME_STACK_SIZE configMINIMAL_STACK_SIZE * 1

uint8_t AlarmHB[5] = {0xAA, 0x55, (uint8_t)21, 0x55, 0xAA};   // 检测到人体
uint8_t AlarmFire[5] = {0xAA, 0x55, (uint8_t)31, 0x55, 0xAA}; // 检测到火

extern int app_esp8266_state;
extern int app_fpm383_state;
extern DeviceStatus GolDeviceStatus;
// 任务句柄引用
extern TaskHandle_t xTaskDisplayHandle;
extern TaskHandle_t xTaskSafetyHandle;
extern TaskHandle_t xTaskFpm383Handle;
extern TaskHandle_t xTaskRc522Handle;
// 任务句柄定义

TaskHandle_t xTaskSr602Handle = NULL;
TaskHandle_t xTaskFlameHandle = NULL;

SafetyData safety_data = {.flame_switch = 1, .flame_flag = 0, .sr602_switch = 1, .sr602_flag = 0};

// SR602人体感应模块中断回调函数
void my_sr602_exit_callback(void)
{
    // 检测到人体
    if (xTaskSr602Handle != NULL)
    {
        vTaskNotifyGiveFromISR(xTaskSr602Handle, pdFALSE);
    }
}

// 火焰传感器中断回调函数
void my_flame_exit_callback(void)
{
    // 火焰检测
    if (xTaskFlameHandle != NULL)
    {
        vTaskNotifyGiveFromISR(xTaskFlameHandle, pdFALSE);
    }
}

// 安全任务--初始化完成后创建两个子任务--自销毁
void SafetyTask(void *argument)
{

    taskENTER_CRITICAL(); // 进入临界
    xTaskCreate(Sr602Task, "Sr602Task", TASK_SR602_STACK_SIZE, NULL, TASK_SR602_PRIORITY, &xTaskSr602Handle);
    xTaskCreate(FlameTask, "FlameTask", TASK_FLAME_STACK_SIZE, NULL, TASK_FLAME_PRIORITY, &xTaskFlameHandle);

    taskEXIT_CRITICAL(); // 退出临界区
    vTaskDelete(NULL);
}

// 人体感应模块任务
void Sr602Task(void *argument)
{

    sr602_init(my_sr602_exit_callback);

    while (1)
    {
        // vTaskDelay(TASK_SR602_INTERVAL);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // 等待通知
        if (safety_data.sr602_switch)
        {
            safety_data.sr602_flag = 1;
        }
        else
        {
            safety_data.sr602_flag = 0;
        }
        xTaskNotifyGive(xTaskDisplayHandle);
        sprintf(GolDeviceStatus.people_alarm, "%s", "on");
        LOG("\r\n检测到人体\r\n");
        su03t_send_bytes(AlarmHB, 5);
        app_fpm383_state=0x01;
        xTaskNotifyGive(xTaskFpm383Handle);
        app_esp8266_state = 0x02;


    }
}

// 火焰传感器任务
void FlameTask(void *argument)
{

    flame_init(my_flame_exit_callback);

    while (1)
    {
        // vTaskDelay(TASK_FLAME_INTERVAL);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // 等待通知
        if (safety_data.flame_switch)
        {
            safety_data.flame_flag = 1;
        }
        else
        {
            safety_data.flame_flag = 0;
        }

        sprintf(GolDeviceStatus.people_alarm, "%s", "on");
        LOG("\r\n火焰检测:%.2f\r\n", (float)flame_get_channel_data());
        su03t_send_bytes(AlarmFire, 5);
        xTaskNotifyGive(xTaskDisplayHandle);
        xTaskNotifyGive(xTaskRc522Handle);
        app_esp8266_state = 0x02;
    }
}
