#include "app_hx1838.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
#include "execution_task.h"
#include "communication_task.h"
#include "beep.h"       // 控制蜂鸣器的函数
#include "app_motor.h"  // 直流电机控制驱动
#include "app_relay.h"  // 继电器控制驱动
#include "app_ws2812.h" // WS2812 LED灯珠控制驱动
#include "app_tc1508.h" /// 步进电机控制驱动

// 任务句柄定义
extern TaskHandle_t xTaskRc522Handle ;
extern TaskHandle_t xTaskFpm383Handle ;
extern TaskHandle_t xTaskHx1838Handle ;
extern TaskHandle_t xTaskEsp8266Handle ;

extern TaskHandle_t xTaskHx1838Handle;
extern TaskHandle_t xTaskBeepHandle;
extern TaskHandle_t xTaskMotorHandle;
extern TaskHandle_t xTaskLedHandle;
extern TaskHandle_t xTaskStepperHandle;
extern TaskHandle_t xTaskRelayHandle;

extern beepData beep;
extern relayData relay1;
extern relayData relay2;
extern DCmotorData dcMotor;
extern ws2812Data ws2812;
extern tc1508Data_DC tc1508_dc;
extern tc1508Data_Stepper tc1508_stepper;

extern uint8_t app_fpm383_state ; // 状态0:不操作     1:注册流程    2:验证流程     3:删除流程
// HX1838模块数据----------------------------------------------------------------
Hx1838KeyData HX1838_keyData;
// HX1838红外接收模块------------------------------------------------------------
void my_infrared_recv_callback(uint8_t code)
{
    HX1838_keyData.key = ir_convert_code(code);
    vTaskNotifyGiveFromISR(xTaskHx1838Handle, pdFALSE);
}

void app_hx1838_proc(void)
{
    printf("Received Key: %c\r\n", HX1838_keyData.key);
    uint8_t newKey_status = app_hx1838_keySwitch(HX1838_keyData.key);
    if(newKey_status == 0x01)
    {
        app_hx1838_deviceSwitch(HX1838_keyData.device_key);
    }else if(newKey_status == 0x03 )
    {
        app_hx1838_deviceAdjust(HX1838_keyData.adjus_key);
    }
    newKey_status = 0x00;
    HX1838_keyData.adjus_key = 0;
    HX1838_keyData.pre_key = HX1838_keyData.key;
}

uint8_t app_hx1838_keySwitch(char newkey)
{
    printf("Switch to %c\r\n", newkey);
    switch (newkey)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '0':
        HX1838_keyData.device_key = newkey;
        return 0x01;

    case '*':
    case '#':
    case 'O':
        HX1838_keyData.special_key = newkey;
        return 0x02;

    case 'U':
    case 'D':
    case 'L':
    case 'R':
        HX1838_keyData.adjus_key = newkey;
        return 0x03;

    default:
        printf("Error Key: %c\n", newkey);
        return 0x00;
    }
}

void app_hx1838_deviceSwitch(char newkey)
{
    printf("Device: %c\r\n",newkey );
    switch (newkey)
    {
    case '1'://控制灯珠
        ws2812.status = ws2812.status == 0x01 ? 0x00 : 0x01;
        if(ws2812.status == 0x01)
        {
            app_ws2812_Set_light(&ws2812, 75);
        }else
        {
            app_ws2812_Set_light(&ws2812, 0);
        }
        xTaskNotifyGive(xTaskLedHandle);
        break;

    case '2'://控制风扇(直流电机)
        dcMotor.status = dcMotor.status == 0x01 ? 0x00 : 0x01;
        xTaskNotifyGive(xTaskMotorHandle);
        break;

    case '3'://控制继电器1
        relay1.status = relay1.status == 0x01 ? 0x00 : 0x01;
        xTaskNotifyGive(xTaskRelayHandle);
        break;

    case '4'://控制继电器2
        relay2.status = relay2.status == 0x01 ? 0x00 : 0x01;
        xTaskNotifyGive(xTaskRelayHandle);

        break;
    case '5'://控制步进电机
        tc1508_stepper.status = tc1508_stepper.status == 0x01 ? 0x00 : 0x01;
        if(tc1508_stepper.status == 0x01)
        {
            tc1508_stepper.direction = 1;
            tc1508_stepper.angle = 5400;
        }else{
            tc1508_stepper.direction = 0;
            tc1508_stepper.angle = 5400;
        }
        xTaskNotifyGive(xTaskStepperHandle);
        break;

    case '6'://验证RFID卡
        xTaskNotifyGive(xTaskRc522Handle);
        break;

    case '7'://验证指纹
        app_fpm383_state = 0x01;
        xTaskNotifyGive(xTaskFpm383Handle);
        break;

    case '8':

        break;

    case '9':

        break;

    case '0':

        break;

    default:
        break;
    }

}

void app_hx1838_deviceAdjust(char newkey)
{
    printf("Device: %c\r\n",HX1838_keyData.device_key );
    switch (HX1838_keyData.adjus_key)
    {
    case 'U':
        switch (HX1838_keyData.device_key)
        {
        case '1'://控制灯珠
            app_ws2812Control(0x02,0x02);
            xTaskNotifyGive(xTaskLedHandle);
            break;

        case '2'://控制风扇(直流电机)
            app_DCmotorControl(0x02,0x02);
            xTaskNotifyGive(xTaskMotorHandle);
            break;

        // case '3'://控制继电器1
        //     app_relayControl(1,0x02,0x02);
        //     xTaskNotifyGive(xTaskRelayHandle);
        //     break;

        // case '4'://控制继电器2
        //     app_relayControl(2,0x02,0x02);
        //     xTaskNotifyGive(xTaskRelayHandle);
            // break;
        case '5'://控制步进电机
            app_tc1508Control(0x02,0x02);
            tc1508_stepper.direction = 1;
            tc1508_stepper.angle = 1800;
            xTaskNotifyGive(xTaskStepperHandle);
            break;
        }
        break;
    case 'D':
        switch (HX1838_keyData.device_key)
        {
        case '1'://控制灯珠
            app_ws2812Control(0x02,0x01);
            xTaskNotifyGive(xTaskLedHandle);
            break;

        case '2'://控制风扇(直流电机)
            app_DCmotorControl(0x02,0x01);
            xTaskNotifyGive(xTaskMotorHandle);
            break;

        // case '3'://控制继电器1
        //     app_relayControl(1,0x02,0x02);
        //     xTaskNotifyGive(xTaskRelayHandle);
        //     break;

        // case '4'://控制继电器2
        //     app_relayControl(2,0x02,0x02);
        //     xTaskNotifyGive(xTaskRelayHandle);
            // break;
        case '5'://控制步进电机
            app_tc1508Control(0x02,0x01);
            tc1508_stepper.direction = 0;
            tc1508_stepper.angle = 1800;
            xTaskNotifyGive(xTaskStepperHandle);
            break;
        }
        break;
    case 'L':
        break;
    case 'R':
        break;
    default:
        break;
    }
}

uint8_t charToHex(char c)
{
    unsigned char hexValue = (unsigned char)c;
    hexValue -= '0';
    return hexValue;
}
