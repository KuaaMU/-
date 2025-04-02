#ifndef __SR602_H
#define __SR602_H

#define SR602_RCU                   RCU_GPIOA
#define SR602_PORT                  GPIOA
#define SR602_PIN                   GPIO_PIN_8

#define SR602_EXTI                  EXTI_8
#define SR602_EXTI_IRQn             EXTI5_9_IRQn
#define SR602_EXTI_SOURCE_PORT      EXTI_SOURCE_GPIOA
#define SR602_EXTI_SOURCE_PIN       EXTI_SOURCE_PIN8


typedef void (* sr602_exti_callback)(void);

extern sr602_exti_callback sr602_exti;

void sr602_init(sr602_exti_callback callback);

#endif
