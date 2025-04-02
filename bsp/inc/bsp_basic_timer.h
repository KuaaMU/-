#ifndef __BSP_BASIC_TIMER_H
#define __BSP_BASIC_TIMER_H


typedef void (* timer5_update_callback)(void);
typedef void (* timer6_update_callback)(void);

void bsp_timer5_init_ms(uint32_t period_us, timer5_update_callback callback);
void bsp_timer5_init(uint32_t period_us, timer5_update_callback callback);
void bsp_timer5_enable(void);
void bsp_timer5_disable(void);

void bsp_timer6_init(uint32_t period_us, timer6_update_callback callback);
void bsp_timer6_enable(void);
void bsp_timer6_disable(void);

#endif
