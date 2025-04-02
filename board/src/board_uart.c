/**
 * @file    board_uart.c
 * @brief   板载串口 (UART3) 的驱动
 *
 * @version 2024-07-23, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "board_uart.h"
#include <stdio.h>


/** @brief   收到数据后的回调函数 */
uart_onboard_recv_callback ob_recv_handler;

/** @brief  串口数据接收缓存 (ob 前缀 = onboard) */
uint8_t     ob_recv_buffer[UART_ONBOARD_RECEIVE_BUFFER_LENGTH];

/** @brief  串口已接收到的数据长度 */
uint16_t    ob_recv_count = 0;


/**
 * @brief   板载串口初始化配置
 *
 * @param   band_rate: 串口通讯波特率
 * @param    callback: 收到数据时的回调函数
 */
void uart_onboard_init(uint32_t band_rate, uart_onboard_recv_callback callback)
{
    rcu_periph_clock_enable(RCU_UART3);
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_af_set(GPIOA, GPIO_AF_8, GPIO_PIN_0 | GPIO_PIN_1);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0 | GPIO_PIN_1);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);

    usart_deinit(UART3);                                        // 复位
    usart_baudrate_set(UART3, band_rate);                       // 设置波特率 (以下无设置采用默认值：8位数据、1位停止、无校验位)

    usart_enable(UART3);
    usart_transmit_config(UART3, USART_TRANSMIT_ENABLE);
    usart_receive_config(UART3, USART_RECEIVE_ENABLE);

    if (callback) {
        ob_recv_handler = callback;

        nvic_irq_enable(UART3_IRQn, 8, 0U);                      // 配置中断优先级
        usart_interrupt_enable(UART3, USART_INT_RBNE);          // 读数据缓冲区非空中断和溢出错误中断
        usart_interrupt_enable(UART3, USART_INT_IDLE);          // 空闲检测中断
    }
}


/**
 * @brief   板载串口发送一个字节数据
 *
 * @param   byte: 一个字节
 */
void uart_onboard_send_byte(uint8_t byte)
{
    usart_data_transmit(UART3, byte);

    while (RESET == usart_flag_get(UART3, USART_FLAG_TBE));
}


/**
 * @brief   板载串口发送字节数组
 *
 * @param   bytes: 字节数组
 * @param     len: 数组长度
 */
void uart_onboard_send_bytes(uint8_t *bytes, uint8_t len)
{
    while (len--) {
        uart_onboard_send_byte(*bytes++);
    }
}


/**
 * @brief   重置接收缓存和接收计数
 *
 */
void uart_onboard_receive_clear(void)
{
    for(uint16_t i = 0; i < ob_recv_count; i++) {
        ob_recv_buffer[i] = 0;
    }

    ob_recv_count = 0;
}


/**
 * @brief   板载串口(UART3)接收中断，接收到的数据交给回调函数处理
 */
void UART3_IRQHandler(void)
{
    if (usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE) == SET) {
        uint16_t abyte = usart_data_receive(UART3);

        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_RBNE);

        if (ob_recv_count < UART_ONBOARD_RECEIVE_BUFFER_LENGTH) {
            ob_recv_buffer[ob_recv_count++] = abyte;
        }
    } else if (usart_interrupt_flag_get(UART3, USART_INT_FLAG_IDLE) == SET) {
        // 读取后抛弃，参考: https://blog.csdn.net/origin333/article/details/49992383
        usart_data_receive(UART3);
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_IDLE);

        if (ob_recv_count > 0) {
            // ob_recv_buffer[ob_recv_count] = '\0';
            ob_recv_handler(ob_recv_buffer, ob_recv_count);
        }
    }
}
