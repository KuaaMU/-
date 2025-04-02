#ifndef __APP_HX1838_H
#define __APP_HX1838_H

#include "gd32f4xx.h"
#include "hx1838.h"
#include "systick.h"

typedef struct {
    char key;
    char pre_key;
    char device_key;
    char adjus_key;
    char special_key;
} Hx1838KeyData;

void my_infrared_recv_callback(uint8_t code);
void app_hx1838_proc(void);
uint8_t app_hx1838_keySwitch(char newkey);
void app_hx1838_deviceSwitch(char newkey);
void app_hx1838_deviceAdjust(char newkey);
uint8_t charToHex(char c);



#endif




