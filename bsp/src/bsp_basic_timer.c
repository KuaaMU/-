/**
 * @file    bsp_basic_timer.c
 * @brief   基本定时器 TIMER5、TIMER6 的简单封装
 *
 * @version 2024-08-07, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "bsp_basic_timer.h"

/** @brief  TIMER5 更新回调函数 */
timer5_update_callback timer5_update;

/** @brief  TIMER6 更新回调函数 */
timer6_update_callback timer6_update;


/**
 * @brief   配置 TIMER5 定时器 (每微秒计数一次)
 *
 * @param   period_us: 设置自动重装载寄存器的值，每个计数表示一微秒
 * @param    callback: TIMER5 计数更新时的回调函数
 */
void bsp_timer5_init(uint32_t period_us, timer5_update_callback callback)
{
    rcu_periph_clock_enable(RCU_TIMER5);
    timer_deinit(TIMER5);

    timer_parameter_struct timer_initpara;

    timer_initpara.prescaler         = 120 - 1;                 // 时钟预分频数
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;      // 对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;        // 向上计数模式
    timer_initpara.period            = period_us - 1;           // 自动重装载寄存器周期的值(计数值)
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;        // 采样分频

    timer_init(TIMER5, &timer_initpara);

	nvic_irq_enable(TIMER5_DAC_IRQn, 8, 0U);                     // TIMER 中断设置，抢占优先级，子优先级

    timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER5, TIMER_INT_UP);               // 使能更新中断

    timer5_update = callback;

    timer_enable(TIMER5);
}


/**
 * @brief   配置 TIMER5 定时器 (每毫秒计数一次)
 *
 * @param   period_ms: 设置自动重装载寄存器的值，每个计数表示一毫秒
 * @param    callback: TIMER5 计数更新时的回调函数
 */
void bsp_timer5_init_ms(uint32_t period_ms, timer5_update_callback callback)
{
    rcu_periph_clock_enable(RCU_TIMER5);
    timer_deinit(TIMER5);

    timer_parameter_struct timer_initpara;

    timer_initpara.prescaler         = 12000 - 1;               // 时钟预分频数
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;      // 对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;        // 向上计数模式
    timer_initpara.period            = period_ms * 10 - 1;      // 自动重装载寄存器周期的值(计数值)
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;        // 采样分频

    timer_init(TIMER5, &timer_initpara);

	nvic_irq_enable(TIMER5_DAC_IRQn, 8, 0U);                     // TIMER 中断设置，抢占优先级，子优先级

    timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER5, TIMER_INT_UP);               // 使能更新中断

    timer5_update = callback;

    timer_enable(TIMER5);
}


/**
 * @brief   使能 TIMER5 定时器
 *
 */
void bsp_timer5_enable(void)
{
    timer_enable(TIMER5);
}


/**
 * @brief   失能 TIMER5 定时器
 *
 */
void bsp_timer5_disable(void)
{
    timer_disable(TIMER5);
}


/**
 * @brief   配置 TIMER6 定时器 (每微秒计数一次)
 *
 * @param   period_us: 设置自动重装载寄存器的值，每个计数表示一微秒
 * @param    callback: TIMER6 计数更新时的回调函数
 */
void bsp_timer6_init(uint32_t period_us, timer6_update_callback callback)
{
    rcu_periph_clock_enable(RCU_TIMER6);
    timer_deinit(TIMER6);

    timer_parameter_struct timer_initpara;

    timer_initpara.prescaler         = 120 - 1;                 // 时钟预分频数
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;      // 对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;        // 向上计数模式
    timer_initpara.period            = period_us - 1;           // 自动重装载寄存器周期的值(计数值)
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;        // 采样分频

    timer_init(TIMER6, &timer_initpara);

	nvic_irq_enable(TIMER6_IRQn, 8, 0U);                         // TIMER 中断设置，抢占优先级，子优先级

    timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER6, TIMER_INT_UP);               // 使能更新中断

    timer6_update = callback;

    timer_enable(TIMER6);
}


/**
 * @brief   使能 TIMER6 定时器
 *
 */
void bsp_timer6_enable(void)
{
    timer_enable(TIMER6);
}


/**
 * @brief   失能 TIMER6 定时器
 *
 */
void bsp_timer6_disable(void)
{
    timer_disable(TIMER6);
}


/**
 * @brief   TIMER5 更新中断
 *
 */
void TIMER5_DAC_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER5, TIMER_INT_FLAG_UP) == SET) {
        timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);

        timer5_update();
    }
}


/**
 * @brief   TIMER6 更新中断
 *
 */
void TIMER6_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER6, TIMER_INT_FLAG_UP) == SET) {
        timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);

        timer6_update();
    }
}
