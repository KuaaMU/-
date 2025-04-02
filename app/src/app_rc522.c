#include "app_rc522.h"
#include "FreeRTOS.h"
#include "systick.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
#include "su03t.h"
#include "execution_task.h"
#include "communication_task.h"

extern uint8_t app_esp8266_state;
extern DeviceStatus GolDeviceStatus;
// RFID RC522模块数据-----------------------------------------------------------
extern uint8_t ucArray_ID[4];                               // 卡的ID存储，4字节
uint8_t read_write_data[16] = {0};                          // 读写数据缓存
uint8_t card_KEY[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // 默认密码
// 实时任务流程全局变量
uint8_t app_rc522_state = 0; // 状态0:不操作     1:注册流程    2:验证流程     3:删除流程
uint8_t app_rc522_status = 0xff;

uint8_t NFC_OK[5] = {0xAA, 0x55, (uint8_t)61, 0x55, 0xAA}; // NFC验证成功
uint8_t NFC_NO[5] = {0xAA, 0x55, (uint8_t)62, 0x55, 0xAA}; // NFC验证失败
void app_rc522_proc(void)
{
    uint8_t status;

    LOG("RC522 Start\r\n");
    // 寻卡（方式：范围内全部），第一次寻卡失败后再进行一次，寻卡成功时卡片序列传入数组ucArray_ID中
    if ((status = PcdRequest(PICC_REQALL, ucArray_ID)) != MI_OK)
    {
        status = PcdRequest(PICC_REQALL, ucArray_ID);
    }

    if (status == MI_OK)
    {
        LOG("RC522 IN\r\n");
        // 防冲突操作，被选中的卡片序列传入数组ucArray_ID中
        if (PcdAnticoll(ucArray_ID) == MI_OK)
        {
            // 输出卡ID
            LOG("卡号ID: %X %X %X %X\r\n", ucArray_ID[0], ucArray_ID[1], ucArray_ID[2], ucArray_ID[3]);

            // 选卡
            if (PcdSelect(ucArray_ID) != MI_OK)
            {
                LOG("PcdSelect failure\n");
                su03t_send_bytes(NFC_NO, 5);
            }

            // 校验卡片密码
            if (PcdAuthState(PICC_AUTHENT1A, 6, card_KEY, ucArray_ID) != MI_OK)
            {
                LOG("PcdAuthState failure\n");
                su03t_send_bytes(NFC_NO, 5);
            }

            su03t_send_bytes(NFC_OK, 5);
            // 将read_write_data的16位数据，填充为0（清除数据的意思）
            memset(read_write_data, 0, 16);
            delay_1us(8);

            // 读取数据块4的数据
            if (PcdRead(4, read_write_data) != MI_OK)
            {
                LOG("PcdRead failure\n");
                su03t_send_bytes(NFC_NO, 5);
            }
            sprintf(GolDeviceStatus.living_door, "%s", "NFC");
            app_esp8266_state = 0x02;
            handleCommon(0x00, 0x01, 0x07);
            sprintf(GolDeviceStatus.living_light, "%s", "on");
            handleBedroom(0x11, 0x01, 0x02);  // 假设0x22是卧室床帘地址
            handleBathroom(0x31, 0x01, 0x02); // 假设0x42是浴室风扇地址
            handleBathroom(0x21, 0x01, 0x02); // 假设0x43是浴室热水器地址
            handleBathroom(0x11, 0x01, 0x02); // 假设0x41是浴室浴霸地址
            handleBedroom(0x31, 0x01, 0x02);  // 假设0x23是卧室空调地址
            handleKitchen(0x11, 0x01, 0x02);  // 假设0x31是厨房油烟地址
            // 输出读出的数据
            for (int i = 0; i < 16; i++)
            {
                LOG("%x ", read_write_data[i]);
            }
            LOG("\n");
        }
    }
}
