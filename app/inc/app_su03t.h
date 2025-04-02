#ifndef __APP_SU_03T_H
#define __APP_SU_03T_H

#include "gd32f4xx.h"
#include "su03t.h"

#define SU03T_RX_BUFFER_LENGTH 8

typedef struct
{
    uint8_t rx_len;
    uint8_t rxDATA[SU03T_RX_BUFFER_LENGTH];
} Su03tData;

void my_su03t_recv_callback(uint8_t *data, uint8_t len);
void app_su03t_proc(Su03tData *data);
void app_su03tControl(uint8_t mode, uint8_t adjust);
#endif
