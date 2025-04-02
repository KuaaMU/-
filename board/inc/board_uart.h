#ifndef __BOARD_UART_H
#define __BOARD_UART_H

#define UART_ONBOARD_RECEIVE_BUFFER_LENGTH      1024

typedef void (* uart_onboard_recv_callback)(uint8_t *bytes, uint16_t len);

void uart_onboard_init(uint32_t band_rate, uart_onboard_recv_callback callback);
void uart_onboard_send_byte(uint8_t byte);
void uart_onboard_send_bytes(uint8_t *bytes, uint8_t len);
void uart_onboard_receive_clear(void);

#endif
