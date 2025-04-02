/**
 * @file    fpm383.c
 * @brief   FPM383C 电容式指纹传感器
 * @details FPM383C 通过 UART 接口通讯，默认波特率 57600。
 *          模组周边内置 RGB 三色 LED 灯。
 *          无需上位机参与，FPM383C 即可完成指纹录入，图像处理，指纹比对，指纹特征储存等功能。
 * @remark  本代码完成了协议数据包封装、协议数据包校验、串口通讯以及部分指令封装，但未完成全部指令封装，
 *          需要其它指令，可参考官方用户手册的指令代码，调用本代码的函数 fp_send_command() 进行通讯。
 *
 * @version 2024-08-06, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fpm383.h"
#include "systick.h"
#include "stdio.h"

/** @brief   FPM383C 通讯协议头 */
uint8_t fp_packet_header[8] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A};

/** @brief   FPM383C 通讯协议加密密码，4 字节 */
uint32_t fp_packet_pwd = 0;
/** @brief   用于缓存应答包 */
uint8_t fp_recv_buffer[0xFFFF] = {0};

/** @brief   已接收到的应答包长度 */
uint16_t fp_recv_count = 0;

/** @brief   用于标记缓存接收是否出错，比如在出错后就不再继续接收剩下的字节，等待下一次从头开始 */
uint8_t fp_recv_error = 0;

/** @brief   用于标记缓存接收是否完成 */
uint8_t fp_recv_complete = 0;

/** @brief   从应答包解析出的结构体 */
fp_ack_struct fp_ack;

/**
 * @brief  配置用于中断的引脚
 *
 */
void fp_gpio_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);

    // gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_3);

    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_3);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    nvic_irq_enable(EXTI3_IRQn, 7, 0);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN3);

    exti_init(EXTI_3, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_enable(EXTI_3);
    exti_interrupt_flag_clear(EXTI_3);
}

/**
 * @brief   配置用于通讯的串口引脚
 *
 */
void fp_usart_init(void)
{
    rcu_periph_clock_enable(RCU_USART1); // 开启USART1时钟
    rcu_periph_clock_enable(RCU_GPIOA);  // 开启GPIOA1时钟

    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_2);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_3);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    /* 配置串口的参数 */
    usart_deinit(USART1);                         // 复位串口
    usart_baudrate_set(USART1, 57600);            // 设置波特率
    usart_parity_config(USART1, USART_PM_NONE);   // 没有校验位
    usart_word_length_set(USART1, USART_WL_8BIT); // 8位数据位
    usart_stop_bit_set(USART1, USART_STB_1BIT);   // 1位停止位

    /* 使能串口 */
    usart_enable(USART1);                                 // 使能串口
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE); // 使能串口发送
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);   // 使能串口接收

    /* 中断配置 */
    nvic_irq_enable(USART1_IRQn, 7, 0U);            // 配置中断优先级
    usart_interrupt_enable(USART1, USART_INT_RBNE); // 读数据缓冲区非空中断和溢出错误中断
    usart_interrupt_enable(USART1, USART_INT_IDLE); // 空闲检测中断
}

/**
 * @brief   FPM383C 指纹模组引脚配置及初始化
 *
 */
void fingerprint_init(void)
{
    fp_gpio_init();
    fp_usart_init();
}

/**
 * @brief   FPM383C 指纹模组解除初始化及注销中断
 *
 */
void fingerprint_deinit(void)
{
    nvic_irq_disable(USART1_IRQn);
    usart_disable(USART1);
    usart_deinit(USART1);
    rcu_periph_clock_disable(RCU_GPIOA);
    rcu_periph_clock_disable(RCU_USART1);

    exti_interrupt_disable(EXTI_3);
    nvic_irq_disable(EXTI3_IRQn);
    rcu_periph_clock_disable(RCU_GPIOC);
}

/**
 * @brief   调用串口发送一个字节数据
 *
 * @param   byte: 要发送的一个字节
 */
void fp_usart_send_data(uint8_t byte)
{
    usart_data_transmit(USART1, byte);

    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE))
        ;
}

/**
 * @brief   调用串口发送字节数组
 *
 * @param   bytes: 要发送的字节数组
 * @param     len: 要发送的字节数组长度
 */
void fp_usart_send_array(uint8_t *bytes, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        fp_usart_send_data(bytes[i]);
    }
}

/**
 * @brief   根据 FPM383 的通讯协议定义，生成数据校验和
 *
 * @remark  一个协议包中包含两次校验和，一次是帧头校验和，一次是数据校验和
 * @param    data: 要校验的数据
 * @param   start: 要参与校验的数据起始位置
 * @param     len: 要参与校验的数据的长度
 * @return uint8_t 返回生成的校验和
 */
uint8_t fp_protocol_get_checksum(uint8_t data[], uint16_t start, uint32_t len)
{
    uint8_t sum = 0;

    for (uint32_t i = start; i < start + len; i++)
    {
        sum += data[i];
    }

    return (uint8_t)((~sum) + 1);
}

/**
 * @brief   根据 FPM383 的通讯协议定义，封装数据包
 *
 * @remark
 * ＋－－－－＋－－－－－－－－－－－－－－－－－－－－－－＋－－－－－－－－－－－－－－－－－－－－－－－－＋
 * ｜　　　　｜　　　　　　　　　　９位　　　　　　　　　　｜　　　　　应　用　层　数　据　（７＋Ｎ）　　　　｜
 * ＋－－－－＋－－－－＋－－－－－－－－－＋－－－－－－－＋－－－－－－＋－－－－＋－－－－－－＋－－－－－＋
 * ｜　格式　｜　帧头　｜　应用层数据长度　｜　帧头校验和　｜　校验密码　｜　命令　｜　数据内容　｜　校验和　｜
 * ＋－－－－＋－－－－＋－－－－－－－－－＋－－－－－－－＋－－－－－－＋－－－－＋－－－－－－＋－－－－－＋
 * ｜　长度　｜　８　　｜　　　　２　　　　｜　　　１　　　｜　　４　　　｜　２　　｜　　Ｎ　　　｜　　１　　｜
 * ＋－－－－＋－－－－＋－－－－－－－－－＋－－－－－－－＋－－－－－－＋－－－－＋－－－－－－＋－－－－－＋
 *
 * 　　　　　帧头：固定 8 位 0xF1 1F E2 2E B6 6B A8 8A
 * 应用层数据长度：描述应用层实际数据的长度，此数据长度不包含帧头、应用层数据长度、帧头校验和 3 部分；
 * 　　帧头校验和：为帧头+应用层数据长度的校验和，用来检查数据长度是否有误。
 *
 * 　　　官方资料：https://h.hlktech.com/Mobile/download/fdetail/177.html
 *
 *
 * @param[in]      cmd: 2 位命令，命令集参考官方文档
 * @param[in]     data: 应用层数据内容
 * @param[in] data_len: 应用层数据长度
 * @param[out]  packet: 输出封装好的数据包
 * @retval
 */
void fp_protocol_gen_packet(uint16_t cmd, uint8_t *data, uint8_t data_len, uint8_t *packet)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        packet[i] = fp_packet_header[i];
    }

    // 应用层数据长度
    uint16_t app_data_len = 7 + data_len;

    packet[8] = (app_data_len >> 8) & 0xFF;
    packet[9] = app_data_len & 0xFF;
    packet[10] = fp_protocol_get_checksum(packet, 0, 10);

    packet[11] = (fp_packet_pwd >> 24);
    packet[12] = (fp_packet_pwd >> 16) & 0xFF;
    packet[13] = (fp_packet_pwd >> 8) & 0xFF;
    packet[14] = fp_packet_pwd & 0xFF;

    packet[15] = (cmd >> 8);
    packet[16] = cmd & 0xFF;

    for (uint8_t i = 0; i < data_len; i++)
    {
        packet[17 + i] = data[i];
    }

    packet[17 + data_len] = fp_protocol_get_checksum(packet, 11, 6 + data_len);
}

/**
 * @brief   根据 FPM383 的通讯协议定义，解析应答包
 *
 * @remark  参考封装数据包的格式，应答包在 [应用层数据] 的 [命令] 和 [数据内容] 之间，多了 4 位 [错误码]
 *
 * @param       packet: 接收到的应答包
 * @param   ack_struct: 存放解析结果的结构体
 * @return uint8_t  如果应答包错误或解析异常，返回 0，否则返回 1
 */
uint8_t fp_protocol_parse_ack(uint8_t *packet, fp_ack_struct *ack_struct)
{
    // 应用层数据长度
    uint16_t app_data_len = ((packet[8] << 8) | packet[9]);
    uint8_t checksum = packet[11 + app_data_len - 1];

    if (checksum != fp_protocol_get_checksum(packet, 11, app_data_len - 1))
    {
        LOG("checksum error\r\n");
        return 0;
    }

    ack_struct->password = ((packet[11] << 24) | (packet[12] << 16) | (packet[13] << 8) | packet[14]);
    ack_struct->cmd1 = packet[15];
    ack_struct->cmd2 = packet[16];
    ack_struct->errcode = ((packet[17] << 24) | (packet[18] << 16) | (packet[19] << 8) | packet[20]);

    uint16_t data_len = app_data_len - 11;
    ack_struct->data_len = data_len;

    if (data_len > 0)
    {
        ack_struct->data = malloc(data_len);

        if (!ack_struct->data)
        {
            return 0; // 内存分配失败
        }

        for (uint8_t i = 0; i < data_len; i++)
        {
            ack_struct->data[i] = packet[21 + i];
        }
    }

    return 1;
}

/**
 * @brief   向指纹模组发送指令
 *
 * @param      cmd: 指纹协议定义的指令码，由两个 8 位指令组成
 * @param     data: 要发送的数据内容
 * @param data_len: 要发送的数据长度
 */
void fp_send_command(uint16_t cmd, uint8_t *data, uint8_t data_len)
{

    fp_receive_clear();
    uint8_t packet_len = 18 + data_len;
    uint8_t packet[packet_len];

    fp_protocol_gen_packet(cmd, data, data_len, packet);
    fp_usart_send_array(packet, packet_len);
}
fp_ack_struct fp_get_ack(void)
{
    free(fp_ack.data);  // 释放 malloc 内存
    fp_ack.data = NULL; // 避免悬挂指针
    fp_ack.data_len = 0;

    while (!fp_recv_complete);

    fp_protocol_parse_ack(fp_recv_buffer, &fp_ack);

    return fp_ack;
}

/**
 * @brief   开启/关闭 LED 灯
 *
 * @param on_off: 1:开启 0:关闭
 * @param  color: 0:无颜色控制 1:绿色 2:红色 3:红绿 4:蓝色 5:红蓝 6:绿蓝
 */
void fp_led_switch(uint8_t on_off, uint8_t color)
{
    // LOG("fp_led_switch: %d, %d\r\n", on_off, color);
    uint8_t data[5] = {0};

    if (on_off)
    {
        data[0] = 0x01;
        data[1] = color;
    }

    fp_send_command(0x020F, data, 5);
    delay_1ms(20);
}

/**
 * @brief   闪烁 LED 灯
 * @param[in]    color: 0:无颜色控制 1:绿色 2:红色 3:红绿 4:蓝色 5:红蓝 6:绿蓝
 * @param[in]  on_10ms: LED 点亮时长 (单位: 10ms)
 * @param[in] off_10ms: LED 熄灭时长 (单位: 10ms)
 * @param[in]   period: 闪烁周期次数
 */
void fp_led_blink(uint8_t color, uint8_t on_10ms, uint8_t off_10ms, uint8_t period)
{
    // LOG("fp_led_blink: %d, %d, %d, %d\r\n", color, on_10ms, off_10ms, period);
    uint8_t data[5];

    data[0] = 0x04;
    data[1] = color;
    data[2] = on_10ms;
    data[3] = off_10ms;
    data[4] = period;

    fp_send_command(0x020F, data, 5);
    delay_1ms(20);
}

/**
 * @brief   PWM 控制 LED 灯 (呼吸灯)
 *
 * @param     color: 0:无颜色控制 1:绿色 2:红色 3:红绿 4:蓝色 5:红蓝 6:绿蓝
 * @param  max_duty: 最大占空比 (范围: 0 ~ 100)
 * @param  min_duty: 最小占空比 (范围: 0 ~ 100)
 * @param frequency: 占空比每秒变化速率 (范围: 0 ~ 255)
 */
void fp_led_breathe(uint8_t color, uint8_t max_duty, uint8_t min_duty, uint8_t frequency)
{
    // LOG("fp_led_breathe: %d, %d, %d, %d\r\n", color, max_duty, min_duty, frequency);
    uint8_t data[5];

    data[0] = 0x03;
    data[1] = color;
    data[2] = max_duty;
    data[3] = min_duty;
    data[4] = frequency;

    fp_send_command(0x020F, data, 5);
    delay_1ms(20);
}

/**
 * @brief   触摸时亮灯
 *
 * @remark  (此功能按官方文档的指令编写，但经测试无触摸亮灯的效果，暂未排查原因)
 *
 * @param   color: 触摸亮灯的颜色
 */
void fp_led_touch(uint8_t color)
{
    // LOG("fp_led_touch: %d\r\n", color);
    uint8_t data[5] = {0};

    data[0] = 0x02;
    data[1] = color;

    fp_send_command(0x020F, data, 5);
    delay_1ms(20);
}

/**
 * @brief   指纹注册
 *
 * @remark  FPM383C 指纹注册是指纹拼接功能，需要多次按压，直到进度值达到 100 时(通过指纹注册查询功能获得)，再进行指纹模板的保存
 *
 * @param   reg_idx: 表示一次指纹注册时按压的次数，从 0x01 开始
 */
void fp_enroll(uint8_t reg_idx)
{
    LOG("fp_enroll: %d\r\n", reg_idx);
    uint8_t data[1] = {reg_idx};

    fp_send_command(0x0111, data, 1);
    delay_1ms(50);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 0)
    {
        LOG("error:%02X\r\n", ack.errcode);
    }
}

/**
 *
 * @brief   注册指纹确认与查询注册指纹确认结果
 */
uint8_t fp_enroll_confirm(void)
{
    LOG("fp_enroll_confirm\r\n");
    uint8_t state = 0;
    // 0x41 命令(注册指纹确认)
    fp_send_command(0x0141, NULL, 0);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 0)
    {
        LOG("error:%02X\r\n", ack.errcode);
        state++;
    }

    // 0x42 命令（查询注册指纹确认结果）
    fp_send_command(0x0142, NULL, 0);
    delay_1ms(20);

    if (ack.data_len == 0)
    {
        LOG("error:%02X\r\n", ack.errcode);
        state++;
    }

    if (state == 2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   查询指纹注册结果
 *
 * @return  uint8_t 返回指纹注册进度值，达到 100 时，可进行指纹模板的保存
 */
uint8_t fp_enroll_check(void)
{
    LOG("fp_enroll_check\r\n");
    fp_send_command(0x0112, NULL, 0);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 3)
    {
        LOG("error:%02X,ID_H:%02X,ID_L:%02X,PROC:%02X\r\n", ack.errcode, ack.data[0], ack.data[1], ack.data[2]);
        return ack.data[2];
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   保存指纹模版
 * @param id   指纹模版 ID
 * @return  void
 */
void fp_enroll_save(uint16_t id)
{
    LOG("fp_enroll_save: %d\r\n", id);
    uint8_t data[2];
    data[0] = id >> 8;
    data[1] = id & 0xFF;
    fp_send_command(0x0113, data, 2);
    delay_1ms(200);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 0)
    {
        LOG("error:%02X\r\n", ack.errcode);
    }
}

/**
 * @brief   查询指纹保存结果
 *
 * @return  uint16_t 返回保存成功的指纹模版 ID，失败返回 0x000F
 */
uint16_t fp_enroll_save_check(void)
{
    LOG("fp_enroll_save_check\r\n");
    fp_send_command(0x0114, NULL, 0);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 2)
    {
        LOG("ID_H:%02X,ID_:%02X\r\n", ack.data[0], ack.data[1]);
        return (ack.data[0] << 8) | ack.data[1];
    }
    else
    {
        LOG("error:%02X\r\n", ack.errcode);
        return 0;
    }
}

/**
 * @brief   取消指纹注册或匹配
 *
 * @return  void
 */
void fp_enroll_cancel(void)
{
    LOG("fp_enroll_cancel\r\n");
    fp_send_command(0x0115, NULL, 0);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 0)
    {
        LOG("error:%02X\r\n", ack.errcode);
    }
}

/**
 * @brief   指纹匹配与查询匹配结果
 *
 * @return  uint16_t 返回指纹id
 */
uint16_t fp_match_REid(void)
{
    LOG("fp_match\r\n");
    // 0x0121 命令(指纹匹配)
    fp_send_command(0x0121, NULL, 0);
    delay_1ms(2000);
    fp_ack_struct ack = fp_get_ack();
    // 0x0122 命令(查询匹配结果)
    fp_send_command(0x0122, NULL, 0);
    delay_1ms(100);

    ack = fp_get_ack();

    if (ack.data_len == 6)
    {
        LOG("fin:%04X,score:%04X,id:%04X\r\n", (ack.data[0] << 8) | ack.data[1], (ack.data[2] << 8) | ack.data[3], (ack.data[4] << 8) | ack.data[5]);
        fp_led_switch(1, 0x01);
        return (ack.data[4] << 8) | ack.data[5];
    }
    else
    {
        LOG("error:%02X\r\n", ack.errcode);
        fp_led_switch(1, 0x02);
        return 0xFFFF;
    }
}

/**
 * @brief   指纹匹配与查询匹配结果
 *
 * @return  uint8_t 返回是否成功，1为成功，0为失败
 */
uint8_t fp_match(void)
{
    LOG("fp_match\r\n");
    // 0x0121 命令(指纹匹配)
    fp_send_command(0x0121, NULL, 0);
    delay_1ms(20);

    // 0x0122 命令(查询匹配结果)
    fp_send_command(0x0122, NULL, 0);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 6)
    {
        LOG("fin:%04X,score:%04X,id:%04X\r\n", (ack.data[0] << 8) | ack.data[1], (ack.data[2] << 8) | ack.data[3], (ack.data[4] << 8) | ack.data[5]);
        return 1;
    }
    else
    {
        LOG("error:%02X\r\n", ack.errcode);
        return 0;
    }
}

/**
 * @brief   指纹特征清除(单条)
 * @param   cl_flag 清除模式标志：
 *                - 0x00: 清除单个指纹（需要提供ID）
 * @param   id 高位指纹ID
 * @return  uint8_t 返回操作结果，0为成功，非0为错误码
 */
uint32_t fp_clear_feature_one(uint16_t id)
{
    LOG("fp_clear_feature_one: %d\r\n", id);
    uint8_t data[3];
    data[0] = 0x00;
    data[1] = id >> 8;
    data[2] = id & 0xFF;

    fp_send_command(0x0131, data, 3);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();
    if (ack.data_len == 0)
    {
        LOG("error:%02X\r\n", ack.errcode);
        return ack.errcode;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   指纹特征清除(块删除)
 * @param   cl_flag 清除模式标志：
 *                - 0x03: 传入要删除的首 ID 和尾 ID
 * @param   first_id 指纹ID
 * @param   last_id 指纹ID
 * @return  uint8_t 返回操作结果，0为成功，非0为错误码
 */
uint32_t fp_clear_feature_block(uint16_t first_id, uint16_t last_id)
{
    LOG("fp_clear_feature_block: %d,%d\r\n", first_id, last_id);
    uint8_t data[5];
    data[0] = 0x03;
    data[1] = first_id >> 8;
    data[2] = first_id & 0xFF;
    data[3] = last_id >> 8;
    data[4] = last_id & 0xFF;

    fp_send_command(0x0131, data, 5);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();
    if (ack.data_len == 0)
    {
        LOG("error:%02X\r\n", ack.errcode);
        return ack.errcode;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   指纹特征清除(全部)
 * @param   cl_flag 清除模式标志：
 *                - 0x01:
 * @param   id 指纹ID
 * @return  uint8_t 返回操作结果，0为成功，非0为错误码
 */
uint32_t fp_clear_feature_all(void)
{
    LOG("fp_clear_feature_all\r\n");
    uint8_t data[2];
    data[0] = 0x01;
    data[1] = 0x00;
    data[1] = 0x01;

    fp_send_command(0x0131, data, 3);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();
    if (ack.data_len == 0)
    {
        LOG("error:%02X\r\n", ack.errcode);
        return ack.errcode;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   查询指纹特征清除结果
 * @return  uint8_t 返回查询结果，0为成功，非0为错误码
 */
uint32_t fp_check_feature_clear(void)
{
    LOG("fp_check_feature_clear\r\n");
    uint8_t packet[7];
    fp_ack_struct ack;

    // 生成查询指纹特征清除结果的数据包
    fp_protocol_gen_packet(0x0132, NULL, 0, packet);

    // 发送数据包
    fp_usart_send_array(packet, 7);

    // 等待响应并处理
    ack = fp_get_ack();
    if (ack.data_len == 0)
    {
        LOG("error:%02X\r\n", ack.errcode);
        return ack.errcode; // 错误码
    }
    else
    {
        return 0; // 成功
    }
}

/**
 * @brief   查询指纹ID是否存在
 * @return  uint8_t 返回查询结果，0x01为存在，0x00为不存在
 */
uint8_t fp_check_feature_exist(uint16_t id)
{
    LOG("fp_check_feature_exist: %d\r\n", id);
    uint8_t data[3];
    data[0] = id >> 8;
    data[1] = id & 0xFF;

    fp_send_command(0x0133, data, 2);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();
    if (ack.data_len == 3)
    {
        LOG("state:%02X\r\n", ack.data[0]);
        return ack.data[0]; // 0x01
    }
    else
    {
        return 0; // 0x00
    }
}

/**
 * @brief   查询手指在位状态
 *
 * @return uint8_t 如果手指在位，则返回 state 为 1，否则 state 为 0
 */
uint8_t fp_finger_state(void)
{
    // LOG("fp_finger_state\r\n");
    fp_send_command(0x0135, NULL, 0);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 1)
    {
        LOG("state:%02X\r\n", ack.data[0]);
        return ack.data[0]; // 0x01
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   复位指纹模块
 * @return  void
 */
void fp_reset(void)
{
    LOG("fp_reset\r\n");
    fp_send_command(0x0202, NULL, 0);
    delay_1ms(20);
}

/**
 * @brief   获取指纹模板数量
 * @return  uint16_t 返回指纹模板的数量
 */
uint16_t fp_template_num(void)
{
    LOG("fp_template_num\r\n");
    fp_send_command(0x0203, NULL, 0);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 2)
    {
        LOG("num:%02X,%02X\r\n", ack.data[0], ack.data[1]);
        return (ack.data[0] << 8) | ack.data[1];
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   获取增益
 * @return  uint16_t 返回当前增益值
 */
uint16_t fp_get_gain(void)
{
    LOG("fp_get_gain\r\n");
    fp_send_command(0x0209, NULL, 0);
    delay_1ms(20);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 3)
    {
        LOG("shift:%02X,gain:%02X,pxl:%02X\r\n", ack.data[0], ack.data[1], ack.data[2]);
        return (ack.data[1] << 8) | ack.data[2];
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   设置休眠模式
 *
 * @param mode: 0:普通休眠 (按压指纹唤醒)  1:深度休眠 (外部输入复位信号唤醒)
 */
void fp_sleep(uint8_t mode)
{
    LOG("fp_sleep: %d\r\n", mode);
    uint8_t data[1];

    data[0] = mode ? 0x01 : 0x00;

    fp_send_command(0x020C, data, 1);
    delay_1ms(300);

    exti_interrupt_enable(EXTI_3);
    exti_interrupt_flag_clear(EXTI_3);
}

/**
 * @brief   设置指纹注册拼接次数
 * @param   count 拼接次数，范围为 1 到 6
 */
void fp_set_enroll_count(uint8_t count)
{
    LOG("fp_set_enroll_count: %d\r\n", count);
    uint8_t data[1];

    data[0] = count;

    fp_send_command(0x020E, data, 1);
    delay_1ms(300);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 0)
    {
        LOG("err:%02X\r\n", ack.errcode);
    }
}

/**
 * @brief   设置波特率
 * @param   baud
 */
void fp_set_baud(uint32_t baud)
{
    LOG("fp_set_baud: %d\r\n", baud);
    uint8_t data[4];

    data[0] = (baud >> 24) & 0xFF;
    data[1] = (baud >> 16) & 0xFF;
    data[2] = (baud >> 8) & 0xFF;
    data[3] = baud & 0xFF;

    fp_send_command(0x020E, data, 4);
    delay_1ms(300);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 0)
    {
        LOG("err:%02X\r\n", ack.errcode);
    }
}

/**
 * @brief   设置通信密码
 * @param   baud
 */
void fp_set_password(uint32_t password)
{
    LOG("fp_set_password: %d\r\n", password);
    uint8_t data[4];

    data[0] = (password >> 24) & 0xFF;
    data[1] = (password >> 16) & 0xFF;
    data[2] = (password >> 8) & 0xFF;
    data[3] = password & 0xFF;

    fp_send_command(0x0305, data, 4);
    delay_1ms(300);

    fp_ack_struct ack = fp_get_ack();

    if (ack.data_len == 0)
    {
        LOG("err:%02X\r\n", ack.errcode);
    }
}

/**
 * @brief   重置接收缓存和接收计数
 *
 */
void fp_receive_clear(void)
{
    // LOG("fp_receive_clear\r\n");
    for (uint16_t i = 0; i < fp_recv_count; i++)
    {
        fp_recv_buffer[i] = 0;
    }

    fp_recv_count = 0;
    fp_recv_error = 0;
    fp_recv_complete = 0;
}

// 打印缓存区
void fp_print_recv_buffer(void)
{
    if (fp_recv_complete == 1)
    {
        LOG("fp_print_recv_buffer\r\n");
        for (uint16_t i = 0; i < fp_recv_count; i++)
        {
            LOG("%02X ", fp_recv_buffer[i]);
        }
        LOG("\r\n");
    }
}

/**
 * @brief   非空中断、溢出中断、空闲中断
 *
 */
void USART1_IRQHandler(void)
{
    if (usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE) == SET)
    {
        // 清除中断标志位
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);
        if(fp_recv_complete == 0)
        {
            fp_recv_buffer[fp_recv_count++] = usart_data_receive(USART1);
        }
    }
    else if (usart_interrupt_flag_get(USART1, USART_INT_FLAG_IDLE) == SET)
    {
        usart_data_receive(USART1);
        fp_recv_complete = 1;
    }
}

/**
 * @brief   上升沿中断
 *
 */
void EXTI3_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_3) == SET)
    {
        if (gpio_input_bit_get(GPIOC, GPIO_PIN_3) == SET)
        {
            fp_led_blink(1, 10, 10, 3);
            exti_deinit();
            LOG("fp_sleep_wakeup\r\n");
        }

        exti_interrupt_flag_clear(EXTI_3);
    }
}
