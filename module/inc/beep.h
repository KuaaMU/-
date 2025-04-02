#ifndef __BEEP_H
#define __BEEP_H

#include "gd32f4xx.h"
#include "systick.h"
#include "stdio.h"

#define BEEP_RCU       		RCU_GPIOB
#define BEEP_PORT      		GPIOB
#define BEEP_PIN       	  GPIO_PIN_13

#define BEEP_TIMER_RCU		RCU_TIMER0
#define BEEP_TIMER				TIMER0
#define BEEP_CH			      TIMER_CH_0

void beep_init(void);
void beep_enable(void);
void beep_disable(void);
void beep_dididi(uint16_t num,uint16_t enable_time,uint16_t disable_time);

#endif


