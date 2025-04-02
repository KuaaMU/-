#ifndef __APP_DWIN_H
#define __APP_DWIN_H

#include "gd32f4xx.h"
#include "systick.h"
#include "string.h"
#include "stdio.h"

void my_dwin_recv_callback(uint16_t address, uint8_t code);
void app_su03tControl( uint8_t mode, uint8_t adjust);
void app_dwin_senserData_update(void);
void app_dwin_get_proc(void);
void app_dwin_proc(void);

#endif
