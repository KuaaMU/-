#ifndef __DWIN_USART_HMI_H
#define __DWIN_USART_HMI_H


#define DWIN_RX_RCU                         RCU_GPIOA
#define DWIN_RX_PORT                        GPIOA
#define DWIN_RX_PIN                         GPIO_PIN_11

#define DWIN_TX_RCU                         RCU_GPIOA
#define DWIN_TX_PORT                        GPIOA
#define DWIN_TX_PIN                         GPIO_PIN_12

#define DWIN_GPIO_AF                        GPIO_AF_8

#define DWIN_USART_RCU                      RCU_USART5
#define DWIN_USART                          USART5
#define DWIN_USART_IRQn                     USART5_IRQn
#define DWIN_USART_IRQHandler               USART5_IRQHandler

#define DWIN_USART_BAND_RATE                115200
#define DWIN_RECEIVE_BUFFER_LENGTH          9

typedef struct
{
    uint16_t address;
    uint8_t code;
} dwinData;


typedef void (* dwin_recv_callback)(uint16_t address, uint8_t code);

void dwin_init(dwin_recv_callback callback);
void dwin_write(uint16_t address, uint16_t command);
void dwin_receive_buffer_clear(void);

#endif
