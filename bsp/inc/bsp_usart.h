#ifndef __BSP_USART_H
#define __BSP_USART_H


#define BSP_USART_TX_RCU        RCU_GPIOB                   // 串口TX的端口时钟
#define BSP_USART_TX_PORT       GPIOB                       // 串口TX的端口
#define BSP_USART_TX_PIN        GPIO_PIN_10                 // 串口TX的引脚
#define BSP_USART_RX_RCU        RCU_GPIOB                   // 串口RX的端口时钟
#define BSP_USART_RX_PORT       GPIOB                       // 串口RX的端口
#define BSP_USART_RX_PIN        GPIO_PIN_11                 // 串口RX的引脚

#define BSP_USART               USART2                      // 串口x
#define BSP_USART_RCU           RCU_USART2                  // 串口x的时钟
#define BSP_USART_AF            GPIO_AF_7                   // 串口x的复用功能
#define BSP_USART_IRQ           USART2_IRQn                 // 串口x中断
#define BSP_USART_IRQHandler    USART2_IRQHandler           // 串口x中断服务函数

/* 串口缓冲区的数据长度 */
#define BSP_USART_RECEIVE_LENGTH    1024

void bsp_usart_init(uint32_t band_rate);                    // 配置串口
void bsp_usart_send_byte(uint8_t byte);                     // 发送一个字符
void bsp_usart_send_bytes(uint8_t *bytes, uint8_t len);     // 发送一个字符串
void bsp_usart_receive_clear(void);

#endif
