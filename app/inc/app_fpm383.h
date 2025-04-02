#ifndef __APP_FPM383_H
#define __APP_FPM383_H

#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fpm383.h"

void app_fpm383_proc(void);

uint16_t fp_enroll_fingerprint(uint16_t id);
uint16_t fp_verify_fingerprint(void);
uint8_t fp_delete_fingerprint(uint16_t id);
uint8_t app_checkFingerPresence(TimeOut_t* const pxTimeout,uint8_t new_state);

#endif
