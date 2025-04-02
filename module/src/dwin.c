/**
 * @file    dwin.c
 * @brief   迪文串口屏人机界面
 *
 * @version 2024-07-26, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "dwin.h"


/** @brief  从串口屏收到数据时的回调函数 */
dwin_recv_callback dwin_recv;

/** @brief  从串口屏接收数据时用的缓存 */
uint8_t   dwin_recv_buffer[DWIN_RECEIVE_BUFFER_LENGTH];

/** @brief  已从串口屏接收到的数据长度 */
uint16_t  dwin_recv_count = 0;


/**
 * @brief   迪文串口屏引脚配置和初始化
 *
 * @param   callback: 串口屏接收到数据后的回调函数
 */
void dwin_init(dwin_recv_callback callback)
{
    rcu_periph_clock_enable(DWIN_RX_RCU);

    gpio_af_set(DWIN_RX_PORT, DWIN_GPIO_AF, DWIN_RX_PIN);
    gpio_mode_set(DWIN_RX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, DWIN_RX_PIN);
    gpio_output_options_set(DWIN_RX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, DWIN_RX_PIN);

    rcu_periph_clock_enable(DWIN_TX_RCU);

    gpio_af_set(DWIN_TX_PORT, DWIN_GPIO_AF, DWIN_TX_PIN);
    gpio_mode_set(DWIN_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, DWIN_TX_PIN);
    gpio_output_options_set(DWIN_TX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, DWIN_TX_PIN);

    rcu_periph_clock_enable(DWIN_USART_RCU);

    usart_deinit(DWIN_USART);                                       // 复位
    usart_baudrate_set(DWIN_USART, DWIN_USART_BAND_RATE);           // 设置波特率 (以下无设置采用默认值：8位数据、1位停止、无校验位)

    usart_enable(DWIN_USART);
    usart_transmit_config(DWIN_USART, USART_TRANSMIT_ENABLE);
    usart_receive_config(DWIN_USART, USART_RECEIVE_ENABLE);

    if (callback) {
        dwin_recv = callback;

        nvic_irq_enable(DWIN_USART_IRQn, 8, 0U);                     // 配置中断优先级
        usart_interrupt_enable(DWIN_USART, USART_INT_RBNE);         // 读数据缓冲区非空中断和溢出错误中断
        usart_interrupt_enable(DWIN_USART, USART_INT_IDLE);         // 空闲检测中断
    }
}


/**
 * @brief   向串口屏发送一个字节数据
 *
 * @param   byte: 一个字节
 */
void dwin_usart_send_byte(uint8_t byte)
{
    usart_data_transmit(DWIN_USART, byte);

    while (RESET == usart_flag_get(DWIN_USART, USART_FLAG_TBE));
}


/**
 * @brief   向串口屏发送字节数组
 *
 * @param   bytes: 字节数组
 * @param     len: 数组长度
 */
void dwin_usart_send_bytes(uint8_t *bytes, uint8_t len)
{
    while (len--) {
        dwin_usart_send_byte(*bytes++);
    }
}


/**
 * @brief   向串口屏发送一个指令
 *
 * @param   address: 指令地址
 * @param   command: 指令内容
 */
void dwin_write(uint16_t address, uint16_t command)
{
    uint8_t buff[8] = { 0x5A, 0xA5, 0x05, 0x82 };
    buff[4] = address >> 8;
    buff[5] = address & 0xFF;
    buff[6] = command >> 8;
    buff[7] = command & 0xFF;

    dwin_usart_send_bytes(buff, 8);
}


/**
 * @brief   从串口屏接收到数据后的处理
 *
 * @param   bytes:
 * @param   len:
 */
void dwin_receive_handle(uint8_t *bytes, uint8_t len)
{
    // 0x82 是写指令应答，0x83 是读指令应答或触摸返回的数据
    if (bytes[3] == 0x83) {
        uint16_t addr = (bytes[4] << 8) | bytes[5];

        dwin_recv(addr, bytes[8]);
    } else {
        dwin_receive_buffer_clear();
    }
}


/**
 * @brief   重置接收缓存和接收计数
 *
 */
void dwin_receive_buffer_clear(void)
{
    for(uint16_t i = 0; i < dwin_recv_count; i++ ) {
        dwin_recv_buffer[i] = 0;
    }

    dwin_recv_count = 0;
}


/**
 * @brief   串口屏(USART5)接收中断，接收到的数据交给回调函数处理
 */
void DWIN_USART_IRQHandler(void)
{
   if (usart_interrupt_flag_get(DWIN_USART, USART_INT_FLAG_RBNE) == SET) {
       uint16_t abyte = usart_data_receive(DWIN_USART);

       usart_interrupt_flag_clear(DWIN_USART, USART_INT_FLAG_RBNE);

       if (dwin_recv_count < DWIN_RECEIVE_BUFFER_LENGTH) {
           dwin_recv_buffer[dwin_recv_count++] = abyte;
       }
   } else if (usart_interrupt_flag_get(DWIN_USART, USART_INT_FLAG_IDLE) == SET) {
       usart_data_receive(DWIN_USART);
       usart_interrupt_flag_clear(DWIN_USART, USART_INT_FLAG_IDLE);

       if (dwin_recv_count > 0) {
           dwin_receive_handle(dwin_recv_buffer, dwin_recv_count);
       }
   }
}
