#ifndef __BSP_PWM_H
#define __BSP_PWM_H


typedef void (* timer0_update_callback)(void);
typedef void (* timer2_update_callback)(void);

void bsp_timer0_pwm_init(uint32_t ms, uint32_t duty, uint16_t timer_channel, timer0_update_callback callback);
void bsp_timer2_pwm_init(uint32_t ms, uint32_t duty, uint16_t timer_channel, timer2_update_callback callback);

void bsp_timer0_set_pulse(uint16_t timer_channel, uint32_t duty);
void bsp_timer2_set_pulse(uint16_t timer_channel, uint32_t duty);

void bsp_timer0_enable(void);
void bsp_timer2_enable(void);

void bsp_timer0_disable(void);
void bsp_timer2_disable(void);

#endif
