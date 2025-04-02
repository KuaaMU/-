#ifndef __FLAME_H
#define __FLAME_H

#include "gd32f4xx.h"
#include "systick.h"
#include "stdio.h"

#define FLAME_DI_RCU 									  RCU_GPIOE
#define FLAME_DI_PORT        					  GPIOE 
#define FLAME_DI_PIN									  GPIO_PIN_13

#define FLAME_EXIT_X									  EXTI_13 
#define FLAME_EXTI_IRQ								  EXTI10_15_IRQn
#define FLAME_EXTI_SOURCE_PORT    		  EXTI_SOURCE_GPIOG
#define FLAME_EXTI_SOURCE_PIN     		  EXTI_SOURCE_PIN6

#define FLAME_AI_RCU 									  RCU_GPIOB
#define FLAME_AI_PORT        					 	GPIOB
#define FLAME_AI_PIN									  GPIO_PIN_1

#define FLAME_ADC_RCU									  RCU_ADC1
#define FLAME_ADC											  ADC1
#define FLAME_ADC_CHANNEL							  ADC_CHANNEL_9

typedef void (* flame_exti_callback)(void);

void flame_init(flame_exti_callback callback);
uint16_t flame_get_channel_data(void);
float flame_get_voltage(void);



#endif
