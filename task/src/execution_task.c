#include "FreeRTOS.h"
#include "task.h"
#include "execution_task.h"
#include "communication_task.h"
#include "display_task.h"
#include "media_task.h"
#include "beep.h"       // 控制蜂鸣器的函数
#include "app_motor.h"  // 直流电机控制驱动
#include "app_relay.h"  // 继电器控制驱动
#include "app_ws2812.h" // WS2812 LED灯珠控制驱动
#include "app_tc1508.h" /// 步进电机控制驱动
#include "app_fpm383.h"
#include "app_esp8266.h"

// 任务优先级定义
#define TASK_BEEP_PRIORITY (configMAX_PRIORITIES - 1)
#define TASK_RELAY_PRIORITY (configMAX_PRIORITIES - 1)
#define TASK_MOTOR_PRIORITY (configMAX_PRIORITIES - 1)
#define TASK_LED_PRIORITY (configMAX_PRIORITIES - 1)
#define TASK_STEPPER_PRIORITY (configMAX_PRIORITIES - 1)

// 任务堆栈大小定义
#define TASK_BEEP_STACK_SIZE configMINIMAL_STACK_SIZE * 1
#define TASK_RELAY_STACK_SIZE configMINIMAL_STACK_SIZE * 1
#define TASK_MOTOR_STACK_SIZE configMINIMAL_STACK_SIZE * 1
#define TASK_LED_STACK_SIZE configMINIMAL_STACK_SIZE * 1
#define TASK_STEPPER_STACK_SIZE configMINIMAL_STACK_SIZE * 1

beepData beep = {0x00};
extern relayData relay1;
extern relayData relay2;
extern DCmotorData dcMotor;
extern ws2812Data ws2812;
extern tc1508Data_DC tc1508_dc;
extern tc1508Data_Stepper tc1508_stepper;

extern uint8_t app_fpm383_state;
extern uint8_t app_esp8266_state;
// 任务句柄定义
extern TaskHandle_t xTaskExecutionHandle;
extern TaskHandle_t xTaskFpm383Handle;
extern TaskHandle_t xTaskEsp8266Handle;

TaskHandle_t xTaskBeepHandle;
TaskHandle_t xTaskMotorHandle;
TaskHandle_t xTaskLedHandle;
TaskHandle_t xTaskStepperHandle;
TaskHandle_t xTaskRelayHandle;
extern TaskHandle_t xtaskdisplay;

DeviceStatus GolDeviceStatus = {
    .living_light = "",
    .living_home_light = "",
    .living_door = "",

    .fire_alarm = "",
    .people_alarm = "",
    .fire_switch = "",
    .people_switch = "",

    .kitchen_youyan = "",
    .kitchen_light = "",

    .bed_kongtiao = "",
    .bed_light = "",
    .bed_chuanglian = "",

    .bath_yuba = "",
    .bath_fan = "",
    .bath_reshui = "",
    .bath_light = "",
    .bath_chuanglian = "",
    .bath_window = "",

    .chuanglian_time = "",
};

void ExecutionTask(void *argument)
{
    taskENTER_CRITICAL(); // 进入临界
    // 创建子任务
    xTaskCreate(BeepTask, "BeepTask", TASK_BEEP_STACK_SIZE, NULL, TASK_BEEP_PRIORITY, &xTaskBeepHandle);
    xTaskCreate(RelayTask, "RelayTask", TASK_RELAY_STACK_SIZE, NULL, TASK_RELAY_PRIORITY, &xTaskRelayHandle);
    xTaskCreate(MotorTask, "MotorTask", TASK_MOTOR_STACK_SIZE, NULL, TASK_MOTOR_PRIORITY, &xTaskMotorHandle);
    xTaskCreate(LedTask, "LedTask", TASK_LED_STACK_SIZE, NULL, TASK_LED_PRIORITY, &xTaskLedHandle);
    xTaskCreate(StepperMotorTask, "StepperMotorTask", TASK_STEPPER_STACK_SIZE, NULL, TASK_STEPPER_PRIORITY, &xTaskStepperHandle);

    taskEXIT_CRITICAL(); // 退出临界区
    vTaskDelete(NULL);
}

void BeepTask(void *argument)
{
    beep_init();
    while (1)
    {
        // 等待通知或执行其他逻辑
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (beep.status == 0x01)
        {
            beep_dididi(2, 200, 200);
        }
        else if (beep.status == 0x00)
        {
            beep_disable();
        }
        app_esp8266_state = 0x02;
    }
}

void RelayTask(void *argument)
{
    relay1_init();
    relay2_init();
    while (1)
    {
        // 等待通知或执行其他逻辑
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (relay1.status == 0x01)
        {
            relay1_on();
        }
        else if (relay1.status == 0x00)
        {
            relay1_off();
        }
        // 重复上述逻辑以控制其他继电器
        if (relay2.status == 0x01)
        {
            relay2_on();
        }
        else if (relay2.status == 0x00)
        {
            relay2_off();
        }
        app_esp8266_state = 0x02;
    }
}

void MotorTask(void *argument)
{
    motor_init();
    while (1)
    {
        // 等待通知或执行其他逻辑
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (dcMotor.status == 0x01)
        {
            motor_set_speed(dcMotor.percent);
            motor_start();
        }
        else if (dcMotor.status == 0x00)
        {
            motor_stop();
        }
        app_esp8266_state = 0x02;
    }
}

void StepperMotorTask(void *argument)
{
#if STEPPER_MOTOR
    tc1508_stepper_init();
    tc1508_stepper.step = 0;
    while (1)
    {
        // 等待通知或执行其他逻辑
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 检查angle值，并确保step在0到5400的范围内
        if (tc1508_stepper.angle < 0)
        {
            tc1508_stepper.angle = 0;
        }
        else if (tc1508_stepper.angle > 5400)
        {
            tc1508_stepper.angle = 5400;
        }

        // 检查step值，并确保step在0到5400的范围内
        if (tc1508_stepper.step < 0)
        {
            tc1508_stepper.step = 0;
        }
        else if (tc1508_stepper.step > 5400)
        {
            tc1508_stepper.step = 5400;
        }

        // 根据angle值和方向计算step值
        if (tc1508_stepper.direction == 0x01)
        {
            // 如果当前step值加上angle不会超出范围，则直接设置step
            if ((tc1508_stepper.step + tc1508_stepper.angle) <= 5400)
            {
                tc1508_stepper.step += tc1508_stepper.angle;
            }
            else
            {
                // 如果会超出范围，则设置step为5400，并调整angle
                tc1508_stepper.step = 5400;
                tc1508_stepper.angle = 5400 - tc1508_stepper.step;
            }
            tc1508_angle(tc1508_stepper.direction, tc1508_stepper.angle);
        }
        else if (tc1508_stepper.direction == 0x00)
        {
            // 如果当前step值减去angle不会低于0，则直接设置step
            if ((tc1508_stepper.step - tc1508_stepper.angle) >= 0)
            {
                tc1508_stepper.step -= tc1508_stepper.angle;
            }
            else
            {
                // 如果会低于0，则设置step为0，并调整angle
                tc1508_stepper.step = 0;
                tc1508_stepper.angle = tc1508_stepper.step - tc1508_stepper.angle;
            }
            tc1508_angle(tc1508_stepper.direction, tc1508_stepper.angle);
        }

        LOG("step = %d,dir:%d,angle:%d\r\n", tc1508_stepper.step, tc1508_stepper.direction, tc1508_stepper.angle);
        tc1508_timer5_update();
        app_esp8266_state = 0x02;
    }
#else
    tc1508_dc_motor_init();
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (tc1508_dc.status == 0x01)
        {
            tc1508_dc_motor_control(tc1508_dc.direction, tc1508_dc.speed);
        }
        if (tc1508_dc.status == 0x00)
        {
            tc1508_dc_motor_stop();
        }
    }
#endif
}

void LedTask(void *argument)
{
    ws2812_init();
    // app_ws2812_Set_light(&ws2812, 0);
    while (1)
    {
        // 等待通知或执行其他逻辑
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (ws2812.status == 0x00)
        {
            app_ws2812_Set_light(&ws2812, 0);
            app_ws2812_set_all(&ws2812);
        }
        else if (ws2812.status == 0x01)
        {
            app_ws2812_Set_light(&ws2812, ws2812.brightness);
            app_ws2812_set_all(&ws2812);
        }
        if (ws2812.status == 2)
        {
            app_ws2812_rainbow_cycle(2, 150);
        }
        app_esp8266_state = 0x02;
    }
}
//---------------------------------针对空间执行设备------------------------------------------
void handleCommon(uint8_t device, uint8_t mode, uint8_t adjust)
{
    switch (device)
    {
    case 0x00: // 灯
        app_ws2812Control(mode, adjust);
        break;
    case 0x10:
        app_su03tControl(mode, adjust);
        break;
    case 0x20:
        app_DCmotorControl(mode, adjust);
        break;
    case 0x30:
        app_tc1508Control(mode, adjust);
        break;
    }
}

void handleLivingRoom(uint8_t device, uint8_t mode, uint8_t adjust)
{
    // 处理客厅的设备
    switch (device)
    {
    case 0x11: // 房梁灯带
        app_ws2812Control(mode, (adjust == 0x02 || adjust == 0x04) ? 0x04 : 0x01);
        app_relayControl(1, mode, (adjust == 0x02 || adjust == 0x04) ? 0x02 : 0x01);
        sprintf(GolDeviceStatus.living_home_light, "%s", (adjust == 0x02 || adjust == 0x04) ? "on" : "off");
        break;
    case 0x21: // 灯
        app_ws2812Control(mode, (adjust == 0x02 || adjust == 0x07) ? 0x07 : 0x01);
        sprintf(GolDeviceStatus.living_light, "%s", (adjust == 0x02 || adjust == 0x07) ? "on" : "off");
        break;
        // 添加更多设备处理逻辑
    }
}

void handleBedroom(uint8_t device, uint8_t mode, uint8_t adjust)
{
    // 处理卧室的设备
    switch (device)
    {
    case 0x11: // 窗帘
        app_relayControl(2, mode, adjust);
        app_tc1508Control(mode, adjust);
        sprintf(GolDeviceStatus.bed_chuanglian, "%s", adjust == 0x02 ? "on" : "off");
        break;
    case 0x21: // 灯
        app_ws2812Control(mode, (adjust == 0x02 || adjust == 0x07) ? 0x07 : 0x01);
        sprintf(GolDeviceStatus.bed_light, "%s", (adjust == 0x02 || adjust == 0x07) ? "on" : "off");
        break;
    case 0x31: // 空调
        app_DCmotorControl(mode, adjust);
        app_relayControl(1, mode, adjust);
        sprintf(GolDeviceStatus.bed_kongtiao, "%s", adjust == 0x02 ? "on" : "off");
        break;
        // 添加更多设备处理逻辑
    }
}

void handleKitchen(uint8_t device, uint8_t mode, uint8_t adjust)
{
    // 处理厨房的设备
    switch (device)
    {
    case 0x11: // 抽油烟机
        app_relayControl(2, mode, adjust);
        app_DCmotorControl(mode, adjust);
        sprintf(GolDeviceStatus.kitchen_youyan, "%s", adjust == 0x02 ? "on" : "off");
        break;
    case 0x21: // 灯
        app_ws2812Control(mode, (adjust == 0x02 || adjust == 0x05) ? 0x05 : 0x01);
        sprintf(GolDeviceStatus.kitchen_light, "%s", (adjust == 0x02 || adjust == 0x05) ? "on" : "off");
        break;
        // 添加更多设备处理逻辑
    }
}

void handleBathroom(uint8_t device, uint8_t mode, uint8_t adjust)
{
    // 处理浴室的设备
    switch (device)
    {
    case 0x11: // 浴霸
        app_relayControl(1, mode, adjust);
        sprintf(GolDeviceStatus.bath_yuba, "%s", adjust == 0x02 ? "on" : "off");
        break;
    case 0x21: // 热水器
        app_relayControl(2, mode, adjust);
        sprintf(GolDeviceStatus.bath_reshui, "%s", adjust == 0x02 ? "on" : "off");
        break;
    case 0x31: // 排气扇
        app_DCmotorControl(mode, adjust);
        sprintf(GolDeviceStatus.bath_fan, "%s", adjust == 0x02 ? "on" : "off");
        break;
    case 0x41: // 窗帘
        app_tc1508Control(mode, adjust);
        sprintf(GolDeviceStatus.bath_chuanglian, "%s", adjust == 0x02 ? "on" : "off");
        break;
    case 0x51: // 窗户
        app_tc1508Control(mode, adjust);
        sprintf(GolDeviceStatus.bath_window, "%s", adjust == 0x02 ? "on" : "off");
        break;
        // 添加更多设备处理逻辑
    case 0x61: // 灯
        app_ws2812Control(mode, (adjust == 0x02 || adjust == 0x06) ? 0x06 : 0x01);
        sprintf(GolDeviceStatus.bath_light, "%s", (adjust == 0x02 || adjust == 0x06) ? "on" : "off");
        break;
    }
}

//---------------------------------------------------------------------------------
// void app_beepControl(uint8_t mode,uint8_t adjust) {
//     switch (mode)
//     {
//     case 0x01:
//         // 播放音乐
//         break;
//     case 0x02:
//         // 停止音乐
//         break;
//     default:
//         // 播放音乐
//         break;
//     }
// }
