#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h" // 添加互斥锁头文件
#include <stdio.h>
#include <string.h>
#include "sensor_task.h"
#include "communication_task.h"
#include "app_esp8266.h"
#include "bmp180.h"   // BMP180气压传感器驱动
#include "dht11.h"    // DHT11温湿度传感器驱动
#include "mq2.h"      // MQ2气体传感器驱动
#include "temt6000.h" // TEMT6000光敏传感器驱动
#include "ky037.h"    // KY037声音传感器驱动
#include "su03t.h"

// 传感器警报阈值
#define CO_THRESHOLD 30
#define NOISE_THRESHOLD 50

// 各个任务间隔时间
#define TASK_BMP180_INTERVAL 10000  // BMP180气压传感器读取间隔
#define TASK_DHT11_INTERVAL 10000   // DHT11温湿度传感器读取间隔
#define TASK_MQ2_INTERVAL 8000      // MQ2气体传感器读取间隔
#define TASK_TEMT6000_INTERVAL 7000 // TEMT6000光敏传感器读取间隔
#define TASK_KY037_INTERVAL 8000    // KY037声音传感器读取间隔

// 任务优先级定义
#define TASK_BMP180_PRIORITY (configMAX_PRIORITIES - 2)
#define TASK_DHT11_PRIORITY (configMAX_PRIORITIES - 2)
#define TASK_MQ2_PRIORITY (configMAX_PRIORITIES - 3)
#define TASK_TEMT6000_PRIORITY (configMAX_PRIORITIES - 3)
#define TASK_KY037_PRIORITY (configMAX_PRIORITIES - 3)
// 任务堆栈大小定义
#define TASK_BMP180_STACK_SIZE configMINIMAL_STACK_SIZE * 3
#define TASK_DHT11_STACK_SIZE configMINIMAL_STACK_SIZE * 3
#define TASK_MQ2_STACK_SIZE configMINIMAL_STACK_SIZE * 3
#define TASK_TEMT6000_STACK_SIZE configMINIMAL_STACK_SIZE * 3
#define TASK_KY037_STACK_SIZE configMINIMAL_STACK_SIZE * 3

extern TaskHandle_t xTaskSensorHandle;
extern TaskHandle_t xTaskEsp8266Handle;
extern TaskHandle_t xTaskDisplayHandle;
// 任务句柄定义
TaskHandle_t xTaskBmp180Handle = NULL;
TaskHandle_t xTaskDht11Handle = NULL;
TaskHandle_t xTaskMq2Handle = NULL;
TaskHandle_t xTaskTemt6000Handle = NULL;
TaskHandle_t xTaskKy037Handle = NULL;

uint8_t su03t_Temp_Tx[9] = {0xAA, 0x55, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA};
uint8_t su03t_Humi_Tx[9] = {0xAA, 0x55, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA};
uint8_t su03t_Air_Tx[9] = {0xAA, 0x55, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA};
uint8_t AlarmCO[5] = {0xAA, 0x55, (uint8_t)22, 0x55, 0xAA}; // 烟雾浓度超过阈值
uint8_t AlarmDB[5] = {0xAA, 0x55, (uint8_t)23, 0x55, 0xAA}; // 噪声超过阈值

extern uint8_t app_esp8266_state;

SensorData sensor_data;
static SemaphoreHandle_t sensor_data_mutex;

void my_ky037_exti_callback(void)
{
    if (xTaskSensorHandle != NULL)
    {
        // 处理 KY037 中断
        // 例如：唤醒传感器任务
        vTaskResume(xTaskSensorHandle);
    }
}

void my_mq2_exti_callback(void)
{
    if (xTaskSensorHandle != NULL)
    {
        // 处理 MQ2 中断
        // 例如：唤醒传感器任务
        vTaskResume(xTaskSensorHandle);
    }
}

void SensorTask(void *argument)
{
    taskENTER_CRITICAL(); // 进入临界

    // 创建互斥锁
    sensor_data_mutex = xSemaphoreCreateMutex();
    if (sensor_data_mutex == NULL)
    {
        LOG("Failed to create sensor data mutex\r\n");
        taskEXIT_CRITICAL(); // 退出临界区
        vTaskDelete(NULL);
        return;
    }

    // 创建子任务
    if (xTaskCreate(Bmp180Task, "Bmp180Task", TASK_BMP180_STACK_SIZE, NULL, TASK_BMP180_PRIORITY, &xTaskBmp180Handle) != pdPASS)
    {
        LOG("Failed to create Bmp180Task\r\n");
        vSemaphoreDelete(sensor_data_mutex);
        taskEXIT_CRITICAL(); // 退出临界区
        vTaskDelete(NULL);
        return;
    }
    if (xTaskCreate(Dht11Task, "Dht11Task", TASK_DHT11_STACK_SIZE, NULL, TASK_DHT11_PRIORITY, &xTaskDht11Handle) != pdPASS)
    {
        LOG("Failed to create Dht11Task\r\n");
        vSemaphoreDelete(sensor_data_mutex);
        taskEXIT_CRITICAL(); // 退出临界区
        vTaskDelete(NULL);
        return;
    }
    if (xTaskCreate(Mq2Task, "Mq2Task", TASK_MQ2_STACK_SIZE, NULL, TASK_MQ2_PRIORITY, &xTaskMq2Handle) != pdPASS)
    {
        LOG("Failed to create Mq2Task\r\n");
        vSemaphoreDelete(sensor_data_mutex);
        taskEXIT_CRITICAL(); // 退出临界区
        vTaskDelete(NULL);
        return;
    }
    if (xTaskCreate(Temt6000Task, "Temt6000Task", TASK_TEMT6000_STACK_SIZE, NULL, TASK_TEMT6000_PRIORITY, &xTaskTemt6000Handle) != pdPASS)
    {
        LOG("Failed to create Temt6000Task\r\n");
        vSemaphoreDelete(sensor_data_mutex);
        taskEXIT_CRITICAL(); // 退出临界区
        vTaskDelete(NULL);
        return;
    }
    if (xTaskCreate(Ky037Task, "Ky037Task", TASK_KY037_STACK_SIZE, NULL, TASK_KY037_PRIORITY, &xTaskKy037Handle) != pdPASS)
    {
        LOG("Failed to create Ky037Task\r\n");
        vSemaphoreDelete(sensor_data_mutex);
        taskEXIT_CRITICAL(); // 退出临界区
        vTaskDelete(NULL);
        return;
    }

    taskEXIT_CRITICAL(); // 退出临界区
    vTaskDelete(NULL);
}

// 定义各个传感器的读取任务
void Bmp180Task(void *argument)
{
    // 初始化传感器
    bmp180_init();
    sensor_data.temperature_bmp180=26.2;
    while (1)
    {
        if (xSemaphoreTake(sensor_data_mutex, portMAX_DELAY) == pdTRUE)
        {
            sensor_data.temperature_bmp180 = bmp180_get_temperature();
            sensor_data.pressure = bmp180_get_pressure();
            sensor_data.altitude = bmp180_get_altitude(sensor_data.pressure);
            // LOG("BMP180: Temperature = %.2f, Pressure = %.2f, Altitude = %.2f\r\n", sensor_data.temperature_bmp180, sensor_data.pressure, sensor_data.altitude);

            xTaskNotifyGive(xTaskDisplayHandle);
            app_esp8266_state = 0x02;
            xSemaphoreGive(sensor_data_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_BMP180_INTERVAL));
    }
}

void Dht11Task(void *argument)
{
    dht11_init();
    sensor_data.temperature_dht11=26.2;
    sensor_data.humidity=58.3;
    while (1)
    {
        if (xSemaphoreTake(sensor_data_mutex, portMAX_DELAY) == pdTRUE)
        {
            dht11_read_data(&sensor_data.temperature_dht11, &sensor_data.humidity);
            // LOG("DHT11: Temperature = %.2f, Humidity = %.2f\r\n", sensor_data.temperature_dht11, sensor_data.humidity);
            su03t_Humi_Tx[3] = (uint8_t)((int)sensor_data.humidity);
            su03t_send_bytes(su03t_Humi_Tx, sizeof(su03t_Humi_Tx));
            su03t_Temp_Tx[3] = (uint8_t)((int)sensor_data.temperature_dht11);
            su03t_send_bytes(su03t_Temp_Tx, sizeof(su03t_Temp_Tx));

            xTaskNotifyGive(xTaskDisplayHandle);
            app_esp8266_state = 0x02;
            xSemaphoreGive(sensor_data_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_DHT11_INTERVAL));
    }
}

void Mq2Task(void *argument)
{
    mq2_init(my_mq2_exti_callback);

    while (1)
    {
        if (xSemaphoreTake(sensor_data_mutex, portMAX_DELAY) == pdTRUE)
        {
            sensor_data.ppm = mq2_get_ppm();
            // LOG("MQ2: PPM = %.2f\r\n", sensor_data.ppm);
            // LOG("MQ2:ADC: %.2f\r\n",mq2_get_voltage());
            if (sensor_data.ppm > CO_THRESHOLD)
            {
                su03t_send_bytes(AlarmCO, sizeof(AlarmCO));
            }

            xTaskNotifyGive(xTaskDisplayHandle);
            app_esp8266_state = 0x02;
            su03t_Air_Tx[3] = (uint8_t)((int)sensor_data.ppm);
            su03t_send_bytes(su03t_Air_Tx, sizeof(su03t_Air_Tx));
            xSemaphoreGive(sensor_data_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_MQ2_INTERVAL));
    }
}

void Temt6000Task(void *argument)
{
    temt6000_init();

    while (1)
    {
        if (xSemaphoreTake(sensor_data_mutex, portMAX_DELAY) == pdTRUE)
        {
            sensor_data.lux = temt6000_get_lux();
            // LOG("TEMT6000: Lux = %.2f\r\n", sensor_data.lux);

            xTaskNotifyGive(xTaskDisplayHandle);
            app_esp8266_state = 0x02;
            xSemaphoreGive(sensor_data_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_TEMT6000_INTERVAL));
    }
}

void Ky037Task(void *argument)
{
    ky037_init(my_ky037_exti_callback);

    while (1)
    {
        if (xSemaphoreTake(sensor_data_mutex, portMAX_DELAY) == pdTRUE)
        {
            sensor_data.noise_adc = ky037_get_channel_data();
            sensor_data.noise_dB = adc_to_decibel(sensor_data.noise_adc);
            // LOG("KY037: Noise = %.2f dB\r\n", sensor_data.noise_dB);
            if (sensor_data.noise_dB > NOISE_THRESHOLD)
            {
                su03t_send_bytes(AlarmDB, sizeof(AlarmDB));
            }

            xTaskNotifyGive(xTaskDisplayHandle);
            app_esp8266_state = 0x02;
            xSemaphoreGive(sensor_data_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_KY037_INTERVAL));
    }
}
