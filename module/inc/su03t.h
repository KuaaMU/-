#ifndef __SU_03T_H
#define __SU_03T_H


#define SU03T_TX_RCU            RCU_GPIOC               // 串口TX的端口时钟
#define SU03T_TX_PORT           GPIOC                   // 串口TX的端口
#define SU03T_TX_PIN            GPIO_PIN_12             // 串口TX的引脚

#define SU03T_RX_RCU            RCU_GPIOD               // 串口RX的端口时钟
#define SU03T_RX_PORT           GPIOD                   // 串口RX的端口
#define SU03T_RX_PIN            GPIO_PIN_2              // 串口RX的引脚

#define SU03T_GPIO_AF           GPIO_AF_8               // 串口x的复用功能
#define SU03T_UART              UART4                   // 串口x
#define SU03T_UART_RCU          RCU_UART4               // 串口x的时钟
#define SU03T_UART_IRQ          UART4_IRQn              // 串口x中断
#define SU03T_UART_IRQHandler   UART4_IRQHandler        // 串口x中断服务函数

/* 串口缓冲区的数据长度 */
#define SU03T_RECEIVE_BUFFER_LENGTH      8

typedef void (* su03t_recv_callback)(uint8_t *bytes, uint8_t len);

void su03t_init(uint32_t band_rate, su03t_recv_callback callback);
void su03t_send_byte(uint8_t byte);
void su03t_send_bytes(uint8_t *bytes, uint8_t len);
void su03t_receive_clear(void);

#endif
