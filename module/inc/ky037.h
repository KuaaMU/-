#ifndef __KY037_H
#define __KY037_H

#include "gd32f4xx.h"
#include "systick.h"
#include "stdio.h"
#include "bsp_basic_timer.h"

#define KY037_DI_RCU 									RCU_GPIOG
#define KY037_DI_PORT        					GPIOG
#define KY037_DI_PIN									GPIO_PIN_6

#define KYO37_EXIT_X									EXTI_6
#define KYO37_EXTI_IRQ								EXTI5_9_IRQn
#define KYO37_EXTI_SOURCE_PORT    		EXTI_SOURCE_GPIOG
#define KYO37_EXTI_SOURCE_PIN     		EXTI_SOURCE_PIN6

#define KY037_AI_RCU 									RCU_GPIOF
#define KY037_AI_PORT        					GPIOF
#define KY037_AI_PIN									GPIO_PIN_9

#define KY037_ADC_RCU									RCU_ADC2
#define KY037_ADC											ADC2
#define KY037_ADC_CHANNEL							ADC_CHANNEL_7

typedef void (* key037_exti_callback)(void);
extern key037_exti_callback ky037_exti;

void ky037_init(key037_exti_callback callback);
uint16_t ky037_get_channel_data(void);
float ky037_get_voltage(void);
float adc_to_decibel(uint16_t adc_value);

#endif


