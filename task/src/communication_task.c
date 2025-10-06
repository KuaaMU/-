#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
#include "communication_task.h"
#include "execution_task.h"
#include "app_rc522.h"   // RFID RC522模块驱动
#include "app_fpm383.h"  // FPM383指纹识别模块驱动
#include "app_esp8266.h" // ESP8266 Wi-Fi模块驱动
#include "app_hx1838.h"  // HX1838红外接收器驱动

// 任务优先级定义
#define ESP8266_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define HX1838_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define FPM383_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define RC522_TASK_PRIORITY (configMAX_PRIORITIES - 2)

// 任务堆栈大小定义
#define FPM383_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define RC522_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define HX1838_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define ESP8266_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 4)

// 任务句柄extern
extern TaskHandle_t xTaskCommHandle;
// 任务句柄定义
TaskHandle_t xTaskRc522Handle = NULL;
TaskHandle_t xTaskFpm383Handle = NULL;
TaskHandle_t xTaskHx1838Handle = NULL;
TaskHandle_t xTaskEsp8266Handle = NULL;

TimeOut_t Xfp_timeout;

// 通信任务--初始化完成后创建四个子任务--自销毁
void CommunicationTask(void *argument)
{
    // 创建子任务
    taskENTER_CRITICAL(); // 进入临界
    xTaskCreate(Esp8266Task, "Esp8266Task", ESP8266_TASK_STACK_SIZE, NULL, ESP8266_TASK_PRIORITY, &xTaskEsp8266Handle);
    xTaskCreate(Fpm383Task, "Fpm383Task", FPM383_TASK_STACK_SIZE, NULL, FPM383_TASK_PRIORITY, &xTaskFpm383Handle);
    xTaskCreate(Hx1838Task, "Hx1838Task", HX1838_TASK_STACK_SIZE, NULL, HX1838_TASK_PRIORITY, &xTaskHx1838Handle);
    xTaskCreate(Rc522Task, "Rc522Task", RC522_TASK_STACK_SIZE, NULL, RC522_TASK_PRIORITY, &xTaskRc522Handle);

    taskEXIT_CRITICAL(); // 退出临界区
    vTaskDelete(NULL);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒更新一次
    }
}

// RFID RC522模块任务
void Rc522Task(void *argument)
{
    // 初始化RFID RC522模块
    RC522_Init();
    RC522_Reset();
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        app_rc522_proc();
    }
}

// 指纹识别任务
void Fpm383Task(void *argument)
{

    vTaskSetTimeOutState(&Xfp_timeout);
    // 初始化指纹识别模块
    fingerprint_init();
    fp_reset();
    vTaskDelay(pdMS_TO_TICKS(2000));
    fp_led_switch(0x01, 0x04);

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        app_fpm383_proc();
    }
}

// 红外接收任务
void Hx1838Task(void *argument)
{
    // 初始化红外接收器
    infrared_init(my_infrared_recv_callback);
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        app_hx1838_proc();
        // vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// esp8266通信任务
void Esp8266Task(void *argument)
{
    // 初始化通信模块
    esp8266_peripheral_init();

    while (1)
    {

        app_esp8266_proc();
    }
}
