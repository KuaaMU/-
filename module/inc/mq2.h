#ifndef __MQ2_H
#define __MQ2_H

#include "gd32f4xx.h"
#include "systick.h"
#include "stdio.h"

#define MQ2_DI_RCU 									RCU_GPIOF
#define MQ2_DI_PORT        					GPIOF 
#define MQ2_DI_PIN									GPIO_PIN_7

#define MQ2_EXIT_X									EXTI_7 
#define MQ2_EXTI_IRQ								EXTI5_9_IRQn
#define MQ2_EXTI_SOURCE_PORT    		EXTI_SOURCE_GPIOF
#define MQ2_EXTI_SOURCE_PIN     		EXTI_SOURCE_PIN7

#define MQ2_AI_RCU 									RCU_GPIOF
#define MQ2_AI_PORT        					GPIOF
#define MQ2_AI_PIN									GPIO_PIN_8

#define MQ2_ADC_RCU									RCU_ADC2
#define MQ2_ADC											ADC2
#define MQ2_ADC_CHANNEL							ADC_CHANNEL_6

typedef void (* mq2_exti_callback)(void);
extern mq2_exti_callback mq2_exti;

void mq2_init(mq2_exti_callback callback);
uint16_t mq2_get_channel_data(void);
float mq2_get_voltage(void);
float mq2_get_ppm(void);

#endif



