/**
 * @file    bsp_usart.c
 * @brief   简单的 USART 板级支持包
 * @remark  (演示、参考)
 *
 * @version 2024-08-07, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "stdio.h"
#include "bsp_usart.h"


/** @brief  串口数据接收缓存 */
uint8_t  bsp_usart_recv_buffer[BSP_USART_RECEIVE_LENGTH];

/** @brief  串口已接收到的数据长度 */
uint16_t bsp_usart_recv_count = 0;


/**
 * @brief   串口引脚配置及初始化
 *
 * @param   band_rate: 串口通讯波特率
 */
void bsp_usart_init(uint32_t band_rate)
{
    /* 开启时钟 */
    rcu_periph_clock_enable(BSP_USART_TX_RCU);
    rcu_periph_clock_enable(BSP_USART_RX_RCU);
    rcu_periph_clock_enable(BSP_USART_RCU);

    /* 配置 GPIO 复用功能 */
    gpio_af_set(BSP_USART_TX_PORT, BSP_USART_AF, BSP_USART_TX_PIN);
    gpio_af_set(BSP_USART_RX_PORT, BSP_USART_AF, BSP_USART_RX_PIN);

    /* 配置 GPIO 的模式 */
    /* 配置 TX 为复用模式 上拉模式 */
    gpio_mode_set(BSP_USART_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_USART_TX_PIN);
    /* 配置 RX 为复用模式 上拉模式 */
    gpio_mode_set(BSP_USART_RX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_USART_RX_PIN);

    /* 配置 TX 为推挽输出 50MHZ */
    gpio_output_options_set(BSP_USART_TX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_USART_TX_PIN);
    /* 配置 RX 为推挽输出 50MHZ */
    gpio_output_options_set(BSP_USART_RX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_USART_RX_PIN);

    /* 配置串口的参数 */
    usart_deinit(BSP_USART);                                        // 复位串口
    usart_baudrate_set(BSP_USART, band_rate);                       // 设置波特率
    usart_parity_config(BSP_USART, USART_PM_NONE);                  // 没有校验位
    usart_word_length_set(BSP_USART, USART_WL_8BIT);                // 8 位数据位
    usart_stop_bit_set(BSP_USART, USART_STB_1BIT);                  // 1 位停止位

    /* 使能串口 */
    usart_enable(BSP_USART);                                        // 使能串口
    usart_transmit_config(BSP_USART, USART_TRANSMIT_ENABLE);        // 使能串口发送
    usart_receive_config(BSP_USART, USART_RECEIVE_ENABLE);          // 使能串口接收

    /* 中断配置 */
    nvic_irq_enable(BSP_USART_IRQ, 6, 0U);                           // 配置中断优先级
    usart_interrupt_enable(BSP_USART, USART_INT_RBNE);              // 读数据缓冲区非空中断和溢出错误中断
    usart_interrupt_enable(BSP_USART, USART_INT_IDLE);              // 空闲检测中断
}


/**
 * @brief   串口发送一个字节数据
 *
 * @param   byte: 一个字节
 */
void bsp_usart_send_byte(uint8_t byte)
{
    usart_data_transmit(BSP_USART, byte);                           // 发送数据

    while(RESET == usart_flag_get(BSP_USART, USART_FLAG_TBE));      // 等待发送数据缓冲区标志置位
}


/**
 * @brief   串口发送字节数组
 *
 * @param   bytes: 字节数组
 * @param     len: 数组长度
 */
void bsp_usart_send_bytes(uint8_t *bytes, uint8_t len)
{
    while (len--) {
        bsp_usart_send_byte(*bytes++);
    }
}


/**
 * @brief   重置接收缓存和接收计数
 *
 */
void bsp_usart_receive_clear(void)
{
    for(uint16_t i = 0; i < BSP_USART_RECEIVE_LENGTH; i++) {
        bsp_usart_recv_buffer[i] = 0;
    }

    bsp_usart_recv_count = 0;
}


/**
 * @brief   串口接收中断，接收到的数据交给回调函数处理
 */
void BSP_USART_IRQHandler(void)
{
    if(usart_interrupt_flag_get(BSP_USART, USART_INT_FLAG_RBNE) == SET) {       // 接收缓冲区不为空
        usart_interrupt_flag_clear(BSP_USART, USART_INT_FLAG_RBNE);             // 清除标志位

        uint16_t byte = usart_data_receive(BSP_USART);                          // 接收数据

        if (bsp_usart_recv_count < BSP_USART_RECEIVE_LENGTH) {                  // 缓存区未满则缓存接收到的数据
            bsp_usart_recv_buffer[bsp_usart_recv_count++] = usart_data_receive(BSP_USART);
        }
    }

    if(usart_interrupt_flag_get(BSP_USART, USART_INT_FLAG_IDLE) == SET) {       // 检测到帧中断
        usart_interrupt_flag_clear(BSP_USART, USART_INT_FLAG_IDLE);

        if (bsp_usart_recv_count > 0) {
           usart_data_receive(BSP_USART);                                       // 必须要读，读出来的值不能要
//            bsp_usart_recv_buffer[bsp_usart_recv_count] = '\0';               // 数据接收完毕，数组结束标志
        }
    }

    if (bsp_usart_recv_count > 0) {
        for (int i = 0; i < bsp_usart_recv_count; i++) {
            bsp_usart_send_byte(bsp_usart_recv_buffer[i]);
        }

        bsp_usart_receive_clear();
    }
}
