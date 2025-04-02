#include "app_dwin.h"
#include "FreeRTOS.h"
#include "task.h"
#include "display_task.h"
#include "sensor_task.h"
#include "safety_task.h"
#include "execution_task.h"
#include "communication_task.h"
#include "media_task.h"
#include "string.h"
#include "stdio.h"
#include "dwin.h" // DWIN串口屏驱动

extern TaskHandle_t xTaskRc522Handle;
extern TaskHandle_t xTaskFpm383Handle;
extern TaskHandle_t xTaskHx1838Handle;
extern TaskHandle_t xTaskEsp8266Handle;

extern uint8_t app_fpm383_state;

extern TaskHandle_t xTaskDisplayHandle;
extern SafetyData safety_data;
extern SensorData sensor_data;
extern DeviceStatus GolDeviceStatus;

uint8_t dwin_state = 0;
dwinData dwin_Data;

#define LUX_ADDR 0x3920
#define NOISE_ADDR 0x3930
#define TEMP_ADDR 0x3940
#define CO_ADDR 0x3950
#define HUM_ADDR 0x3960
#define PRE_ADDR 0x3970

#define RUHU_ZHIWEN_ADDR 0x1000
#define RUHU_NFC_ADDR 0x1010
#define RUHU_ZHIWEN_EN_ADDR 0x1030
#define RUHU_NFC_EN_ADDR 0x1040

#define LIVI_LIGHT_ADDR 0x3000
#define LIVI_LIGHT_STRIP_ADDR 0x3010
#define LIVI_IN_MODE_ADDR 0x3020
#define LIVI_OUT_MODE_ADDR 0x3030

#define BED_LIGHT_ADDR 0x4000
#define BED_CHUANGLIAN_ADDR 0x4010
#define BED_KONGTIAO_ADDR 0x4020

#define KITCH_YOUYAN_ADDR 0x5000
#define KITCH_LIGHT_ADDR 0x5010
#define KITCH_PEOPLE_ALARM_ADDR 0x5900
#define KITCH_FIRE_ALARM_ADDR 0x5910

#define BATH_YOUBA_ADDR 0x6000
#define BATH_FAN_ADDR 0x6010
#define BATH_RESHUI_ADDR 0x6020
#define BATH_CHUANGLIAN_ADDR 0x6030
#define BATH_CHUANGHU_ADDR 0x6040
#define BATH_LIGHT_ADDR 0x6050

void my_dwin_recv_callback(uint16_t address, uint8_t code)
{
    dwin_Data.address = address;
    dwin_Data.code = code;

    dwin_state = 1;
    dwin_receive_buffer_clear();
    vTaskNotifyGiveFromISR(xTaskDisplayHandle, NULL);
}

void app_dwin_proc(void)
{
    if (dwin_state == 0)
    {
        app_dwin_senserData_update();
    }
    else if (dwin_state == 1)
    {
        app_dwin_get_proc();
        dwin_state = 0;
    }
}

void app_dwin_senserData_update(void)
{
    // 传感器数据
    dwin_write(LUX_ADDR, (uint16_t)sensor_data.lux);
    dwin_write(NOISE_ADDR, (uint16_t)sensor_data.noise_dB);
    dwin_write(TEMP_ADDR, (uint16_t)sensor_data.temperature_dht11);
    dwin_write(CO_ADDR, (uint16_t)sensor_data.ppm);
    dwin_write(HUM_ADDR, (uint16_t)sensor_data.humidity);
    dwin_write(PRE_ADDR, (uint16_t)sensor_data.pressure);
    dwin_write(KITCH_FIRE_ALARM_ADDR, (uint16_t)safety_data.flame_flag);
    dwin_write(KITCH_PEOPLE_ALARM_ADDR, (uint16_t)safety_data.sr602_flag);

    // 入户门检测
    dwin_write(RUHU_ZHIWEN_ADDR, (uint16_t)strcmp(GolDeviceStatus.living_door, "zhiwen") == 0 ? 0x01 : 0x00);
    dwin_write(RUHU_NFC_ADDR, (uint16_t)strcmp(GolDeviceStatus.living_door, "NFC") == 0 ? 0x01 : 0x00);

    // 起居设备状态
    dwin_write(LIVI_LIGHT_ADDR, (uint16_t)strcmp(GolDeviceStatus.living_light, "on") == 0 ? 0x02 : 0x01);
    dwin_write(LIVI_LIGHT_STRIP_ADDR, (uint16_t)strcmp(GolDeviceStatus.living_home_light, "on") == 0 ? 0x02 : 0x01);

    // 卧室设备状态
    dwin_write(BED_LIGHT_ADDR, (uint16_t)strcmp(GolDeviceStatus.bed_light, "on") == 0 ? 0x02 : 0x01);
    dwin_write(BED_CHUANGLIAN_ADDR, (uint16_t)strcmp(GolDeviceStatus.bed_chuanglian, "on") == 0 ? 0x02 : 0x01);
    dwin_write(BED_KONGTIAO_ADDR, (uint16_t)strcmp(GolDeviceStatus.bed_kongtiao, "on") == 0 ? 0x02 : 0x01);

    // 厨房设备状态
    dwin_write(KITCH_YOUYAN_ADDR, (uint16_t)strcmp(GolDeviceStatus.kitchen_youyan, "on") == 0 ? 0x02 : 0x01);
    dwin_write(KITCH_LIGHT_ADDR, (uint16_t)strcmp(GolDeviceStatus.kitchen_light, "on") == 0 ? 0x02 : 0x01);

    // 浴室设备状态
    dwin_write(BATH_YOUBA_ADDR, (uint16_t)strcmp(GolDeviceStatus.bath_yuba, "on") == 0 ? 0x02 : 0x01);
    dwin_write(BATH_FAN_ADDR, (uint16_t)strcmp(GolDeviceStatus.bath_fan, "on") == 0 ? 0x02 : 0x01);
    dwin_write(BATH_RESHUI_ADDR, (uint16_t)strcmp(GolDeviceStatus.bath_reshui, "on") == 0 ? 0x02 : 0x01);
    dwin_write(BATH_CHUANGLIAN_ADDR, (uint16_t)strcmp(GolDeviceStatus.bath_chuanglian, "on") == 0 ? 0x02 : 0x01);
    dwin_write(BATH_CHUANGHU_ADDR, (uint16_t)strcmp(GolDeviceStatus.bath_window, "on") == 0 ? 0x02 : 0x01);
    dwin_write(BATH_LIGHT_ADDR, (uint16_t)strcmp(GolDeviceStatus.bath_light, "on") == 0 ? 0x02 : 0x01);
}

void app_dwin_get_proc(void)
{
    switch (dwin_Data.address)
    {
    case RUHU_ZHIWEN_EN_ADDR: // 入户指纹
        app_fpm383_state = 0x01;
        xTaskNotifyGive(xTaskFpm383Handle);
        break;
    case RUHU_NFC_EN_ADDR: // 入户NFC
        xTaskNotifyGive(xTaskRc522Handle);
        break;

    case LIVI_LIGHT_ADDR: // 起居灯光
        switch (dwin_Data.code)
        {
        case 0x01: // 关
            handleLivingRoom(0x21, 0x01, 0x01);
            break;
        case 0x02: // 黄色
            handleCommon(0x00, 0x01, 0x07);
            sprintf(GolDeviceStatus.living_light, "%s", "on");
            break;
        case 0x03: // 红
            handleCommon(0x00, 0x01, 0x03);
            sprintf(GolDeviceStatus.living_light, "%s", "on");
            break;
        case 0x04: // 蓝
            handleCommon(0x00, 0x01, 0x05);
            sprintf(GolDeviceStatus.living_light, "%s", "on");
            break;
        case 0x05: // 紫
            handleCommon(0x00, 0x01, 0x06);
            sprintf(GolDeviceStatus.living_light, "%s", "on");
            break;
        default:
            break;
        }
        break;
    case LIVI_LIGHT_STRIP_ADDR:                       // 起居灯带
        handleLivingRoom(0x11, 0x01, dwin_Data.code); // 假设0x12是起居灯带地址
        break;
    case LIVI_IN_MODE_ADDR: // 居家模式
        if (dwin_Data.code == 0x02)
        {
            handleCommon(0x00, 0x01, 0x07);
            sprintf(GolDeviceStatus.living_light, "%s", "on");
        }
        else
        {
            handleCommon(0x00, 0x01, 0x01);
            sprintf(GolDeviceStatus.living_light, "%s", "off");
        }
        handleBedroom(0x11, 0x01, dwin_Data.code);  // 假设0x22是卧室床帘地址
        handleBathroom(0x31, 0x01, dwin_Data.code); // 假设0x42是浴室风扇地址
        handleBathroom(0x21, 0x01, dwin_Data.code); // 假设0x43是浴室热水器地址
        handleBathroom(0x11, 0x01, dwin_Data.code); // 假设0x41是浴室浴霸地址
        handleBedroom(0x31, 0x01, dwin_Data.code);  // 假设0x23是卧室空调地址
        handleKitchen(0x11, 0x01, dwin_Data.code);  // 假设0x31是厨房油烟地址
        break;
    case LIVI_OUT_MODE_ADDR: // 离家模式
        if (dwin_Data.code == 0x02)
        {
            handleCommon(0x00, 0x01, 0x07);
            sprintf(GolDeviceStatus.living_light, "%s", "off");
            handleBedroom(0x11, 0x01, 0x01);  // 假设0x22是卧室床帘地址
            handleBathroom(0x31, 0x01, 0x01); // 假设0x42是浴室风扇地址
            handleBathroom(0x21, 0x01, 0x01); // 假设0x43是浴室热水器地址
            handleBathroom(0x11, 0x01, 0x01); // 假设0x41是浴室浴霸地址
            handleBedroom(0x31, 0x01, 0x01);  // 假设0x23是卧室空调地址
            handleKitchen(0x11, 0x01, 0x01);  // 假设0x31是厨房油烟地址
        }
        break;
    case BED_LIGHT_ADDR:                           // 卧室灯光
        handleBedroom(0x21, 0x01, dwin_Data.code); // 假设0x21是卧室灯光地址
        break;
    case BED_CHUANGLIAN_ADDR:                      // 卧室床帘
        handleBedroom(0x11, 0x01, dwin_Data.code); // 假设0x22是卧室床帘地址
        break;
    case BED_KONGTIAO_ADDR:                        // 卧室空调
        handleBedroom(0x31, 0x01, dwin_Data.code); // 假设0x23是卧室空调地址
        break;
    case KITCH_YOUYAN_ADDR:                        // 厨房油烟
        handleKitchen(0x11, 0x01, dwin_Data.code); // 假设0x31是厨房油烟地址
        break;
    case KITCH_LIGHT_ADDR:                         // 厨房灯光
        handleKitchen(0x21, 0x01, dwin_Data.code); // 假设0x32是厨房灯光地址
        break;
    case BATH_YOUBA_ADDR:                           // 浴室浴霸
        handleBathroom(0x11, 0x01, dwin_Data.code); // 假设0x41是浴室浴霸地址
        break;
    case BATH_FAN_ADDR:                             // 浴室排气扇
        handleBathroom(0x31, 0x01, dwin_Data.code); // 假设0x42是浴室风扇地址
        break;
    case BATH_RESHUI_ADDR:                          // 浴室热水器
        handleBathroom(0x21, 0x01, dwin_Data.code); // 假设0x43是浴室除湿地址
        break;
    case BATH_CHUANGLIAN_ADDR:                      // 浴室窗帘
        handleBathroom(0x41, 0x01, dwin_Data.code); // 假设0x44是浴室窗帘地址
        break;
    case BATH_CHUANGHU_ADDR:                        // 浴室窗户
        handleBathroom(0x51, 0x01, dwin_Data.code); // 假设0x45是浴室窗户地址
        break;
    case BATH_LIGHT_ADDR:                           // 浴室灯光
        handleBathroom(0x61, 0x01, dwin_Data.code); // 假设0x46是浴室灯光地址
        break;
    default:
        // 未知地址处理
        LOG("Unknown address received: %04X\r\n", dwin_Data.address);
        break;
    }
}
