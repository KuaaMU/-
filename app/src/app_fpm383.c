#include "app_fpm383.h"
#include "FreeRTOS.h"
#include "task.h"
#include "systick.h"
#include "stdio.h"
#include "fpm383.h"
#include "su03t.h"
#include "execution_task.h"
#include "communication_task.h"

#define FP_TIMEOUT 20000
#define OUTTIME_STATE 0x02

extern DeviceStatus GolDeviceStatus;
// fpm383模块数据----------------------------------------------------------------
extern uint8_t fp_packet_header[8]; // FPM383C 通讯协议头
extern uint8_t fp_packet_pwd[4];    // FPM383C 通讯协议加密密码
/** @brief   从应答包解析出的结构体 */
extern fp_ack_struct fp_ack;
/** @brief   用于缓存应答包 */
extern uint8_t fp_recv_buffer[0xFFFF];
/** @brief   已接收到的应答包长度 */
extern uint16_t fp_recv_count;
// 实时任务流程全局变量
uint8_t app_fpm383_state = 0; // 状态0:不操作     1:注册流程    2:验证流程     3:删除流程
uint8_t app_fpm383_status = 0xff;

extern uint8_t app_esp8266_state;

uint16_t current_id=0x0001;

extern TimeOut_t Xfp_timeout;

uint8_t FI_Success[5]=     {0xAA,0x55,(uint8_t)41,0x55,0xAA};     //指纹识别成功
uint8_t FI_Faile[5]=       {0xAA,0x55,(uint8_t)42,0x55,0xAA};     //指纹识别失败
uint8_t FII_Success[5]=    {0xAA,0x55,(uint8_t)43,0x55,0xAA};     //指纹录入成功
uint8_t FII_Faile[5]=      {0xAA,0x55,(uint8_t)44,0x55,0xAA};     //指纹录入失败
uint8_t FID_Success[5]=    {0xAA,0x55,(uint8_t)45,0x55,0xAA};     //指纹删除成功
uint8_t FID_Faile[5]=      {0xAA,0x55,(uint8_t)46,0x55,0xAA};     //指纹删除失败

uint8_t FI_ID_Tx[9]={0xAA,0x55,0x33,0x00,0x00,0x00,0x00,0x55,0xAA};     //发送指纹ID



void app_fpm383_proc(void)
{
    uint8_t app_fpm383_status=0xff;
    switch (app_fpm383_state)
    {
    case 0x01:
        LOG("指纹流程：验证\r\n");
        app_fpm383_status = fp_verify_fingerprint();
        if(app_fpm383_status==0xff)
        {
            LOG("验证失败，无指纹\r\n");
            fp_led_switch(0x01, 0x02);
            su03t_send_bytes(FI_Faile,5);
            break;
        }
        LOG("验证指纹ID:%04X\r\n",current_id);
        sprintf(GolDeviceStatus.living_door,"%s", "zhiwen");
        fp_led_switch(0x01, 0x01);
        su03t_send_bytes(FI_Success,5);
        app_esp8266_state=0x02;
        app_fpm383_state=0x00;

        handleCommon(0x00, 0x01, 0x07);
        sprintf(GolDeviceStatus.living_light, "%s", "on");
        handleBedroom(0x11, 0x01, 0x02);  // 假设0x22是卧室床帘地址
        handleBathroom(0x31, 0x01, 0x02); // 假设0x42是浴室风扇地址
        handleBathroom(0x21, 0x01, 0x02); // 假设0x43是浴室热水器地址
        handleBathroom(0x11, 0x01, 0x02); // 假设0x41是浴室浴霸地址
        handleBedroom(0x31, 0x01, 0x02);  // 假设0x23是卧室空调地址
        handleKitchen(0x11, 0x01, 0x02);  // 假设0x31是厨房油烟地址
        break;

    case 0x02:
        LOG("指纹流程：注册\r\n");
        app_fpm383_status =fp_enroll_fingerprint(current_id);
        if(app_fpm383_status == 0xff)
        {
            LOG("指纹注册失败\r\n");
            fp_led_switch(0x01, 0x02);
            su03t_send_bytes(FII_Faile,5);
            break;
        }
        LOG("注册指纹ID:%04X\r\n",current_id);
        fp_led_switch(0x01, 0x01);
        su03t_send_bytes(FII_Success,5);
        app_fpm383_state=0x00;
        break;

    case 0x03:
        LOG("指纹流程：删除\r\n");
        app_fpm383_status = fp_delete_fingerprint(current_id);
        if(app_fpm383_status == 0xff)
        {
            LOG("指纹删除失败\r\n");
            fp_led_switch(0x01, 0x02);
            su03t_send_bytes(FID_Faile,5);
            break;
        }
        LOG("成功删除指纹ID:%04X\r\n",current_id);
        fp_led_switch(0x01, 0x01);
        su03t_send_bytes(FID_Success,5);
        app_fpm383_state=0x00;
        break;
    default:
        break;
    }

    if(app_fpm383_status != 0xff)
    {
        FI_ID_Tx[4]= (uint8_t)(current_id>>8);
        FI_ID_Tx[3]= (uint8_t)current_id;
        su03t_send_bytes(FI_ID_Tx,9);
    }
}

/**
 * @brief   执行指纹注册流程
 * @return  uint8_t 返回注册结果，0为成功，0xFFFF为错误码
 */
uint16_t fp_enroll_fingerprint(uint16_t id)
{
    uint8_t reg_idx = 0x01; // 从第一次注册开始
    uint8_t progress = 0;
    LOG("请放上手指\r\n");
    // 等待手指在位
    if (app_checkFingerPresence(&Xfp_timeout, 0x01) == OUTTIME_STATE)
    {
        LOG("等待手指在位超时\r\n");
        return 0xFFFF;
    }

    // 判断指纹id是否已存在
    if (fp_check_feature_exist(id) == 0x01)
    {
        LOG("指纹ID(%04X)已存在\r\n", id);
        return 0xFFFF;
    }
    uint8_t error=0;
    start:
    // 发送指纹注册命令
    LOG("开始注册指纹\r\n");
    while (progress < 100 && reg_idx <= 0x06)
    {
        fp_enroll(reg_idx);
        vTaskDelay(pdMS_TO_TICKS(200));
        // 查询注册结果
        if (fp_ack.errcode == 0x00000000)
        {
            progress = fp_enroll_check();
            LOG("[%02X]rate:%02X\r\n", reg_idx, progress);
            if (progress < 100 && reg_idx < 0x06)
            {

                switch (fp_ack.errcode)
                {
                case 0x00000000:
                    reg_idx++;
                    break;
                case 0x00000005:
                    reg_idx=0x01;
                    break;
                case 0x00000015:
                    error++;
                    if(error>20)
                    {
                        return 0xffff;
                    }
                    goto start;
                }

                // 手指已离开，让用户重新放置手指
                LOG("请再次放置手指.\r\n");
                vTaskDelay(pdMS_TO_TICKS(2000));
                if (app_checkFingerPresence(&Xfp_timeout, 0x01) == OUTTIME_STATE)
                {
                    LOG("等待手指在位超时\r\n");
                    return 0xFFFF;
                }

            }
            else
            {
                // 进度达到100，保存指纹
                fp_enroll_save(id);
                delay_1ms(10);

                if (fp_ack.errcode == 0x00000000)
                {
                    LOG("指纹注册成功.\r\n");
                    fp_led_switch(0x01, 0x01);
                    return id; // 成功完成注册
                }
                else
                {
                    return fp_ack.errcode; // 保存失败，返回错误码
                }
            }
        }
        else
        {
            LOG("指纹注册失败.\r\n");
            fp_led_switch(0x01, 0x02);
            return fp_ack.errcode; // 注册失败，返回错误码
        }
    }

    return 0; // 成功完成注册
}

/**
 * @brief   执行指纹验证流程
 * @return  uint8_t 返回匹配的id 或 0xFFFF
 */
uint16_t fp_verify_fingerprint(void)
{
    LOG("开始指纹验证\r\n");

    // 等待手指在位
    if (app_checkFingerPresence(&Xfp_timeout, 0x01) == OUTTIME_STATE)
    {
        LOG("等待手指在位超时\r\n");
        return 0xFFFF;
    }

    // 发送指纹匹配命令
    return fp_match_REid();
}

/**
 * @brief   执行指纹删除流程
 * @param   id 要删除的指纹ID
 * @return  uint8_t 返回删除结果，0为成功，非0为错误码
 */
uint8_t fp_delete_fingerprint(uint16_t id)
{

    LOG("开始删除指纹\r\n");
    LOG("指纹ID: %04X\r\n", id);
    LOG("请放上手指\r\n");
    // 等待手指在位
    if (app_checkFingerPresence(&Xfp_timeout, 0x01) == OUTTIME_STATE)
    {
        LOG("等待手指在位超时\r\n");
        return 1;
    }

    // 判断指纹id是否已存在
    if (fp_check_feature_exist(id) == 0x01)
    {
        LOG("指纹ID(%04X)已存在\r\n", id);
    }
    else
    {
        LOG("指纹ID(%04X)不存在\r\n", id);
        return 1;
    }

    // 发送指纹特征清除命令
    fp_clear_feature_one(id);

    if (fp_ack.errcode == 0x00000000)
    {
        LOG("指纹删除成功.\r\n");\
        fp_led_switch(0x01, 0x01);
        return 0; // 成功删除
    }
    LOG("指纹删除失败.\r\n");
    fp_led_switch(0x01, 0x02);
    return fp_ack.errcode; // 返回错误码
}

uint8_t app_checkFingerPresence(TimeOut_t *const pxTimeout, uint8_t new_state)
{
    uint8_t finger_state;
    uint8_t outtime_state = 0;
    TickType_t timeoutTicks = pdMS_TO_TICKS(FP_TIMEOUT);
    new_state == 0x01 ? fp_led_switch(0x01, 0x06) : fp_led_switch(0x01, 0x05);
    do
    {
        finger_state = fp_finger_state();

        delay_1ms(200);
        outtime_state = xTaskCheckForTimeOut(pxTimeout, &timeoutTicks);
    } while (finger_state != new_state && outtime_state == pdFALSE);

    finger_state == 0x01 ? fp_led_switch(0x01, 0x05) : fp_led_switch(0x01, 0x06);

    return (finger_state == new_state) ? new_state : OUTTIME_STATE;
}
