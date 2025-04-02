#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "event_groups.h"
#include "queue.h"
#include "sensor_task.h"
#include "display_task.h"
#include "communication_task.h"
#include "safety_task.h"
#include "media_task.h"
#include "execution_task.h"

// 任务优先级定义
#define TASK_DEFAULT_PRIORITY (configMAX_PRIORITIES-1)
#define TASK_SAFETY_PRIORITY (configMAX_PRIORITIES-1)               //安全任务优先级
#define TASK_SENSOR_PRIORITY (configMAX_PRIORITIES-1)               //传感器任务优先级
#define TASK_COMM_PRIORITY (configMAX_PRIORITIES -1)                 //通信任务优先级
#define TASK_EXECUTION_PRIORITY (configMAX_PRIORITIES -1)            //执行任务优先级
#define TASK_DISPLAY_PRIORITY (configMAX_PRIORITIES -1)              //显示任务优先级
#define TASK_MEDIA_PRIORITY (configMAX_PRIORITIES -1 )              //多媒体任务优先级

// 任务堆栈大小定义
#define TASK_DEFAULT_STACK_SIZE configMINIMAL_STACK_SIZE * 2
#define TASK_SAFETY_STACK_SIZE configMINIMAL_STACK_SIZE * 2         //安全任务堆栈大小
#define TASK_SENSOR_STACK_SIZE configMINIMAL_STACK_SIZE * 2         //传感器任务堆栈大小
#define TASK_COMM_STACK_SIZE configMINIMAL_STACK_SIZE * 8           //通信任务堆栈大小
#define TASK_EXECUTION_STACK_SIZE configMINIMAL_STACK_SIZE * 2      //执行任务堆栈大小
#define TASK_DISPLAY_STACK_SIZE configMINIMAL_STACK_SIZE * 10      //显示任务堆栈大小
#define TASK_MEDIA_STACK_SIZE configMINIMAL_STACK_SIZE * 2          //多媒体任务堆栈大小

// 任务句柄定义
TaskHandle_t TaskDefaultHandle = NULL;
TaskHandle_t xTaskSensorHandle = NULL;
TaskHandle_t xTaskExecutionHandle = NULL;
TaskHandle_t xTaskCommHandle = NULL;
TaskHandle_t xTaskSafetyHandle = NULL;
TaskHandle_t xTaskDisplayHandle = NULL;
TaskHandle_t xTaskMediaHandle = NULL;


void StartDefaultTask(void* argument);
//------------------------------------------------------------------------
/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    /* place for user code */
}
//------------------------------------------------------------------------

void FREERTOS_Init(void)
{
    xTaskCreate(StartDefaultTask,
                "StartDefault Task",
                TASK_DEFAULT_STACK_SIZE,
                NULL,
                TASK_DEFAULT_PRIORITY,
                &TaskDefaultHandle);
    printf("StartDefault Start\r\n");

}

void StartDefaultTask(void* argument)
{
    /* USER CODE BEGIN StartDefaultTask */
    printf("DefaultTask Start\r\n");
    taskENTER_CRITICAL(); // 进入临界
    //---------------------------------------------------------------
    // 创建传感器数据采集任务
    xTaskCreate(SensorTask,
                "Sensor Task",
                TASK_SENSOR_STACK_SIZE,
                NULL,
                TASK_SENSOR_PRIORITY,
                &xTaskSensorHandle);
    printf("SensorTask Start\r\n");

    // 创建执行控制任务
    xTaskCreate(ExecutionTask,
                "Execution Task",
                TASK_EXECUTION_STACK_SIZE,
                NULL,
                TASK_EXECUTION_PRIORITY,
                &xTaskExecutionHandle);
    printf("ExecutionTask Start\r\n");

    // 创建显示与交互任务
    xTaskCreate(DisplayTask,
                "Display Task",
                TASK_DISPLAY_STACK_SIZE,
                NULL,
                TASK_DISPLAY_PRIORITY,
                &xTaskDisplayHandle);
    printf("DisplayTask Start\r\n");

    //创建通信任务
    xTaskCreate(CommunicationTask,
                "Comm Task",
                TASK_COMM_STACK_SIZE,
                NULL,
                TASK_COMM_PRIORITY,
                &xTaskCommHandle);
    printf("CommTask Start\r\n");

    //创建安全与控制任务
    xTaskCreate(SafetyTask,
                "Safety Task",
                TASK_SAFETY_STACK_SIZE,
                NULL,
                TASK_SAFETY_PRIORITY,
                &xTaskSafetyHandle);
    printf("SafetyTask Start\r\n");

    // 创建媒体与音频任务
    xTaskCreate(MediaTask,
                "Media Task",
                TASK_MEDIA_STACK_SIZE,
                NULL,
                TASK_MEDIA_PRIORITY,
                &xTaskMediaHandle);
    printf("MediaTask Start\r\n");
    //---------------------------------------------------------------
    vTaskDelete(NULL);
    taskEXIT_CRITICAL(); // 退出临界区
    /* Infinite loop */
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    /* USER CODE END StartDefaultTask */
}
