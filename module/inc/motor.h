#ifndef __MOTOR_H
#define __MOTOR_H


#define MOTOR_RCU           RCU_GPIOE
#define MOTOR_PORT          GPIOE
#define MOTOR_PIN           GPIO_PIN_11

#define MOTOR_TIMER_CH      TIMER_CH_1


void motor_init(void);
void motor_start(void);
void motor_stop(void);
void motor_set_speed(uint8_t percent);

#endif
