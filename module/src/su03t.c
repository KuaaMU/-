/**
 * @file    su03t.c
 * @brief   与离线智能语音模块 SU-03T 的串口通讯
 *
 * @version 2024-07-24, V1.0, yanf, 厦门芯力量
 */


#include "gd32f4xx.h"
#include "su03t.h"


/** @brief   收到数据后的回调函数 */
su03t_recv_callback su03t_recv_handler;

/** @brief  接收缓冲区 */
uint8_t     su03t_recv_buffer[SU03T_RECEIVE_BUFFER_LENGTH];

/** @brief  已接收到的数据长度 */
uint16_t    su03t_recv_count = 0;


/**
 * @brief   SU-03T 串口初始化配置

 * @param   band_rate: 串口通讯波特率
 * @param    callback: 收到数据时的回调函数
 */
void su03t_init(uint32_t band_rate, su03t_recv_callback callback)
{
    rcu_periph_clock_enable(SU03T_UART_RCU);
    rcu_periph_clock_enable(SU03T_TX_RCU);
    rcu_periph_clock_enable(SU03T_RX_RCU);

    gpio_af_set(SU03T_TX_PORT, SU03T_GPIO_AF, SU03T_TX_PIN);
    gpio_mode_set(SU03T_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SU03T_TX_PIN);
    gpio_output_options_set(SU03T_TX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SU03T_TX_PIN);

    gpio_af_set(SU03T_RX_PORT, SU03T_GPIO_AF, SU03T_RX_PIN);
    gpio_mode_set(SU03T_RX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SU03T_RX_PIN);
    gpio_output_options_set(SU03T_RX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SU03T_RX_PIN);

    usart_deinit(SU03T_UART);                                        // 复位
    usart_baudrate_set(SU03T_UART, band_rate);                       // 设置波特率 (以下无设置采用默认值：8位数据、1位停止、无校验位)

    usart_enable(SU03T_UART);
    usart_transmit_config(SU03T_UART, USART_TRANSMIT_ENABLE);
    usart_receive_config(SU03T_UART, USART_RECEIVE_ENABLE);

    if (callback) {
        su03t_recv_handler = callback;

        nvic_irq_enable(SU03T_UART_IRQ, 6, 0);                      // 配置中断优先级
        usart_interrupt_enable(SU03T_UART, USART_INT_RBNE);         // 读数据缓冲区非空中断和溢出错误中断
        usart_interrupt_enable(SU03T_UART, USART_INT_IDLE);         // 空闲检测中断
    }
}


/**
 * @brief   SU-03T 串口发送一个字节数据
 *
 * @param   byte: 一个字节
 */
void su03t_send_byte(uint8_t byte)
{
    usart_data_transmit(SU03T_UART, byte);

    while (RESET == usart_flag_get(SU03T_UART, USART_FLAG_TBE));
}


/**
 * @brief   SU-03T 串口发送字节数组
 *
 * @param   bytes: 字节数组
 * @param     len: 数组长度
 */
void su03t_send_bytes(uint8_t *bytes, uint8_t len)
{
    while (len--) {
        su03t_send_byte(*bytes++);
    }
}


/**
 * @brief   重置接收缓存和接收计数
 *
 */
void su03t_receive_clear(void)
{
    for(uint16_t i = 0; i < su03t_recv_count; i++ ) {
        su03t_recv_buffer[i] = 0;
    }

    su03t_recv_count = 0;
}


/**
 * @brief   SU-03T 串口(UART4)接收中断，接收到的数据交给回调函数处理
 */
void SU03T_UART_IRQHandler(void)
{
    if (usart_interrupt_flag_get(SU03T_UART, USART_INT_FLAG_RBNE) == SET) {
        uint16_t abyte = usart_data_receive(SU03T_UART);

        usart_interrupt_flag_clear(SU03T_UART, USART_INT_FLAG_RBNE);

        if (su03t_recv_count < SU03T_RECEIVE_BUFFER_LENGTH) {
            su03t_recv_buffer[su03t_recv_count++] = abyte;
        }
    } else if (usart_interrupt_flag_get(SU03T_UART, USART_INT_FLAG_IDLE) == SET) {
        // 读取后抛弃，参考: https://blog.csdn.net/origin333/article/details/49992383
        usart_data_receive(SU03T_UART);
        usart_interrupt_flag_clear(SU03T_UART, USART_INT_FLAG_IDLE);

        if (su03t_recv_count > 0) {
            su03t_recv_handler(su03t_recv_buffer, su03t_recv_count);
        }
    }
}
