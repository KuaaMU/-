/**
 * @file    hx1838.c
 * @brief   HX1838是一种高灵敏度的红外接收模块，支持NEC编码格式。可用于接收来自遥控器的红外信号，并将其转换为数字信号输出。
 *
 * @version 2024-08-07, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "systick.h"
#include "stdio.h"
#include "hx1838.h"


/** @brief   接收到红外数据后的回调函数 */
infrared_recv_callback ir_recv_handler;


/**
 * @brief   HX1838 引脚配置及初始化
 *
 */
void ir_gpio_init()
{
    rcu_periph_clock_enable(IR_RCU);

    gpio_af_set(IR_PORT, GPIO_AF_9, IR_PIN);
    gpio_mode_set(IR_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, IR_PIN);
}


/**
 * @brief   配置定时器的输入捕获通道，用于红外信号的 NEC 解码
 *
 */
void ir_timer_init()
{
    rcu_periph_clock_enable(RCU_TIMER11);

    timer_parameter_struct timer_initpara;

    timer_initpara.prescaler         = 120 - 1;                 // 时钟预分频数
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;      // 对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;        // 向上计数模式
    timer_initpara.period            = 65535;                   // 自动重装载寄存器周期的值(计数值)
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;        // 采样分频

    timer_init(TIMER11, &timer_initpara);

    timer_ic_parameter_struct timer_ic_initpara;

    timer_ic_initpara.icpolarity    = TIMER_IC_POLARITY_FALLING;
    timer_ic_initpara.icselection   = TIMER_IC_SELECTION_DIRECTTI;
    timer_ic_initpara.icprescaler   = TIMER_IC_PSC_DIV1;
    timer_ic_initpara.icfilter      = 0x0;

    timer_input_capture_config(TIMER11, TIMER_CH_0, &timer_ic_initpara);

    timer_auto_reload_shadow_enable(TIMER11);
    timer_interrupt_flag_clear(TIMER11, TIMER_INT_FLAG_CH0);
    timer_interrupt_enable(TIMER11, TIMER_INT_CH0);

    nvic_irq_enable(TIMER7_BRK_TIMER11_IRQn, 7, 0U);

    timer_enable(TIMER11);
}


/**
 * @brief   HX1838 红外接收模块引脚配置及初始化
 *
 * @param   callback:
 */
void infrared_init(infrared_recv_callback callback)
{
    ir_gpio_init();
    ir_timer_init();

    ir_recv_handler = callback;
}



uint8_t ir_recv_buff[4];        // 4 字节红外命令帧接收缓存，地址码 + 地址码反码 + 命令码 + 命令码反码
uint8_t ir_recv_count = 0;      // 按位计算次数，标记已接收了多少 bit 缓存
uint8_t ir_has_leader = 0;      // 是否已经收到引导码
uint16_t ir_last_count = 0;     // 在定时器输入捕获中，用于临时存储最后一次记录的时间值


/**
 * @brief   清除红外接收的缓存
 *
 */
void ir_recv_clear(void)
{
    ir_recv_buff[0] = 0;
    ir_recv_buff[1] = 0;
    ir_recv_buff[2] = 0;
    ir_recv_buff[3] = 0;

    ir_recv_count = 0;
    ir_has_leader = 0;
    ir_last_count = 0;
}


/**
 * @brief   NEC 解码数据校验
 *
 * @param   data: 接收到的红外数据，4字节，地址码 + 地址码反码 + 命令码 + 命令码反码
 */
void ir_check_data(uint8_t *data)
{
    if (data[0] != (uint8_t)(~data[1])) { return; }

    if (data[2] != (uint8_t)(~data[3])) { return; }

    // printf("%x %x %x %x\r\n", data[0], data[1], data[2], data[3]);

    if (ir_recv_handler) {
        ir_recv_handler(data[2]);
    }

    ir_recv_clear();
}


/**
 * @brief   将接收到的命令码转换成遥控器按键值
 *
 * @param   code: NEC 解码后得到的命令码
 * @return char 返回对应的遥控器按键
 */
char ir_convert_code(uint8_t code)
{
    switch (code) {
        case 0xA2:
            return '1';
        case 0x62:
            return '2';
        case 0xE2:
            return '3';
        case 0x22:
            return '4';
        case 0x02:
            return '5';
        case 0xC2:
            return '6';
        case 0xE0:
            return '7';
        case 0xA8:
            return '8';
        case 0x90:
            return '9';
        case 0x98:
            return '0';
        case 0x68:
            return '*';
        case 0xB0:
            return '#';
        case 0x38:
            return 'O';     // OK
        case 0x18:
            return 'U';     // UP
        case 0x4A:
            return 'D';     // DOWN
        case 0x10:
            return 'L';     // LEFT
        case 0x5A:
            return 'R';     // RIGHT
        default:
            return 'N';     // NULL
    }
}


/**
 * @brief   定时器输入捕获通道中断，计算时长转换成 NEC 的高低电平
 *
 */
void TIMER7_BRK_TIMER11_IRQHandler(void)
{
    if (SET == timer_interrupt_flag_get(TIMER11, TIMER_INT_FLAG_CH0)) {
        timer_interrupt_flag_clear(TIMER11, TIMER_INT_FLAG_CH0);

        uint16_t now = timer_channel_capture_value_register_read(TIMER11, TIMER_CH_0);
        uint16_t count = (now > ir_last_count) ? (now - ir_last_count) : (65535 + now - ir_last_count);
        ir_last_count = now;

        /**
         * 引导码：9.0 ms 低电平 + 4.5 ms 高电平
         * 1 码：0.56 ms 低电平 + 0.56 ms 高电平
         * 0 码：0.56 ms 低电平 + 1.68 ms 高电平
         * 定时器响应低电平，两个跳变低电平之间的时长来判断 0 / 1 码和引导码
         */
        if (ir_has_leader == 1) {
            if (count > 2000 && count < 2500) {
                ir_recv_buff[ir_recv_count / 8] |= (1 << (7 - ir_recv_count % 8));
            } else if (count > 1000 && count < 1300) {

            } else {
                ir_recv_clear();
            }

            if (++ir_recv_count == 32) {
                ir_check_data(ir_recv_buff);
            }
        } else if (count > 12000 && count < 15000) {
            ir_has_leader = 1;
            ir_recv_count = 0;
        }
    }
}
