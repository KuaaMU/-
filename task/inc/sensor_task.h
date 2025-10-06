#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "sensor_task.h"

typedef struct {
    float temperature_bmp180;   // 温度
    float pressure;             // 气压
    float altitude;             // 高度
    float temperature_dht11;    // 温度
    float humidity;             // 湿度
    float ppm;                  // 气体浓度
    float lux;                  // 光照强度
    float noise_adc;            // 噪声adc
    float noise_dB;            // 噪声分贝
} SensorData;



void SensorTask(void* argument);
void Bmp180Task(void *argument);
void Dht11Task(void *argument);
void Mq2Task(void *argument);
void Temt6000Task(void *argument);
void Ky037Task(void *argument);

#endif /* SENSOR_TASK_H */
