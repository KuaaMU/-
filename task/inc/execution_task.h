#ifndef EXECUTION_TASK_H
#define EXECUTION_TASK_H


typedef struct
{
    char living_light[12];
    char living_home_light[12];
    char living_door[12];

	char fire_switch[12];
	char people_switch[12];
    char fire_alarm[12];
    char people_alarm[12];

    char kitchen_youyan[12];
    char kitchen_light[12];

    char bed_kongtiao[12];
    char bed_light[12];
    char bed_chuanglian[12];

    char bath_yuba[12];
    char bath_fan[12];
    char bath_reshui[12];
    char bath_light[12];
    char bath_chuanglian[12];
    char bath_window[12];

    char chuanglian_time[12];
} DeviceStatus;
/**
 * @brief 定义蜂鸣器数据结构
 */
typedef struct
{
    uint8_t status;    // 蜂鸣器状态：0表示关闭，1表示开启
    uint32_t duration; // 蜂鸣器响铃时长，0表示1秒，1表示2秒，以此类推
} beepData;

/**
 * @brief 执行任务函数，用于处理各种设备的操作
 * @param argument 传入的任务参数
 */
void ExecutionTask(void *argument);

/**
 * @brief 蜂鸣器任务函数，用于控制蜂鸣器的响铃
 * @param argument 传入的任务参数
 */
void BeepTask(void *argument);

/**
 * @brief 继电器任务函数，用于控制继电器的开关
 * @param argument 传入的任务参数
 */
void RelayTask(void *argument);

/**
 * @brief 直流电机任务函数，用于控制直流电机的转动
 * @param argument 传入的任务参数
 */
void MotorTask(void *argument);

/**
 * @brief 步进电机任务函数，用于控制步进电机的转动
 * @param argument 传入的任务参数
 */
void StepperMotorTask(void *argument);

/**
 * @brief LED灯任务函数，用于控制LED灯的颜色和亮度
 * @param argument 传入的任务参数
 */
void LedTask(void *argument);

void handleCommon(uint8_t device, uint8_t mode, uint8_t adjust);

void handleLivingRoom(uint8_t device, uint8_t mode, uint8_t adjust);

void handleBedroom(uint8_t device, uint8_t mode, uint8_t adjust);

void handleKitchen(uint8_t device, uint8_t mode, uint8_t adjust);

void handleBathroom(uint8_t device, uint8_t mode, uint8_t adjust);

void app_ws2812Control(uint8_t mode, uint8_t adjust);

void app_relayControl(uint8_t device, uint8_t mode, uint8_t adjust);

void app_DCmotorControl(uint8_t mode, uint8_t adjust);

void app_tc1508Control(uint8_t mode, uint8_t adjust);

#endif /* MOTOR_TASK_H */
