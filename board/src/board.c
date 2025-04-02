/**
 * @file    board.c
 * @brief   主板初始化
 *
 * @version 2024-07-23, V1.0, yanf, 厦门芯力量
 */


#include "gd32f4xx.h"
#include "systick.h"
#include "stdio.h"
#include "board.h"
#include "board_led.h"
#include "board_uart.h"


/**
 * @brief   板载串口(UART3)接收到数据时的回调函数
 *
 * @param   bytes:  接收到的字节数组
 * @param   len:    字节长度
 */
void uart_onboard_recv_handler(uint8_t *bytes, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        printf("%c", bytes[i]);
    }

    printf("\r\n");

    uart_onboard_receive_clear();
}


/**
 * @brief   主板初始化
 */
void board_init(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    systick_config();

    led_onboard_init();
    uart_onboard_init(115200, uart_onboard_recv_handler);
}


/**
 * @brief   printf 重定向到板载串口输出
 */
#if 1
    #if !defined(__MICROLIB)
        // 不使用 MicroLIB 微库的话就需要添加下面的函数
        #if (__ARMCLIB_VERSION <= 6000000)
            //如果编译器是 AC5，就定义下面这个结构体
            struct __FILE { int handle; };
        #endif

        FILE __stdout;

        // 定义 _sys_exit() 以避免使用半主机模式
        void _sys_exit(int x) { x = x; }

        int fputc(int ch, FILE *f) {
            usart_data_transmit(PRINTF_USART, (uint8_t)ch);

            while(RESET == usart_flag_get(PRINTF_USART, USART_FLAG_TBE));
            return ch;
        }
    #endif
#endif
