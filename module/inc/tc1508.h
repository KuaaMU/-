#ifndef __TC1508_H
#define __TC1508_H

#include "gd32f4xx.h"
#include "systick.h"
#include "bsp_basic_timer.h"

/*
 *用于驱动直流电机还是用于驱动2相4线电机
 *值为1时用于驱动直流电机
 *值为0时用于驱动2相4线电机
*/
#define STEPPER_MOTOR			1

//驱动步进电机
#if STEPPER_MOTOR

#define TC1508_RCU          RCU_GPIOC
#define TC1508_PORT         GPIOC
#define TC1508_PIN_1        GPIO_PIN_7
#define TC1508_PIN_2        GPIO_PIN_6
#define TC1508_PIN_3        GPIO_PIN_9
#define TC1508_PIN_4        GPIO_PIN_8

void tc1508_action(uint8_t dir, uint8_t step);
void tc1508_angle(uint8_t direction, uint32_t angle);
void tc1508_timer5_update(void);
void tc1508_stepper_init(void);
void tc1508_stop(void);

//驱动直流电机
#else

#define TC1508_RCU                        RCU_GPIOC
#define TC1508_PORT                       GPIOC
#define TC1508_PIN_1                      GPIO_PIN_7
#define TC1508_PIN_2                      GPIO_PIN_6

#define TC1508_DC_MOTOR_TIMER_RCU		      RCU_TIMER2
#define TC1508_DC_MOTOR_TIMER				      TIMER2
#define TC1508_DC_MOTOR_PIN1_CH			      TIMER_CH_1
#define TC1508_DC_MOTOR_PIN2_CH			      TIMER_CH_0

void tc1508_dc_motor_init(void);
void tc1508_dc_motor_control(uint8_t dir,uint16_t speed);
void tc1508_dc_motor_stop(void);

#endif



#endif


