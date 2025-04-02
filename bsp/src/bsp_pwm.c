/**
 * @file    bsp_pwm.c
 * @brief   简单的 PWM 板级支持包
 *
 * @version 2024-08-07, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "bsp_pwm.h"


/** @brief  TIMER0 计数更新回调函数 */
timer0_update_callback timer0_update;

/** @brief  TIMER2 计数更新回调函数 */
timer2_update_callback timer2_update;


/**
 * @brief   TIMER0 PWM 配置及初始化
 *
 * @param       period_us: 设置自动重装载寄存器的值，每个计数表示一微秒
 * @param            duty: PWM 占空比
 * @param   timer_channel: 使用 TIMER0 的 GPIO 引脚对应的 TIMER 通道
 * @param        callback: 定时器计数更新回调函数
 */
void bsp_timer0_pwm_init(uint32_t period_us, uint32_t duty, uint16_t timer_channel, timer0_update_callback callback)
{
    rcu_periph_clock_enable(RCU_TIMER0);
    timer_deinit(TIMER0);

    timer_parameter_struct timer_initpara;

    timer_initpara.prescaler         = 240 - 1;                         // 时钟预分频数
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;              // 对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;                // 向上计数模式
    timer_initpara.period            = period_us - 1;                   // 自动重装载寄存器周期的值(计数值)
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;                // 采样分频
    timer_initpara.repetitioncounter = 0;

    timer_init(TIMER0, &timer_initpara);

    timer_oc_parameter_struct timer_oc_initpara;

    timer_oc_initpara.outputstate  = TIMER_CCX_ENABLE;                  // 通道输出状态
    timer_oc_initpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;            // 通道输出极性
    timer_oc_initpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;           // 通道处于空闲时的输出
    timer_oc_initpara.outputnstate = TIMER_CCXN_ENABLE;                 // 互补通道输出状态
    timer_oc_initpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;           // 互补通道输出极性
    timer_oc_initpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;          // 互补通道处于空闲时的输出
    timer_channel_output_config(TIMER0, timer_channel, &timer_oc_initpara);

    timer_channel_output_pulse_value_config(TIMER0, timer_channel, duty);
    timer_channel_output_mode_config(TIMER0, timer_channel, TIMER_OC_MODE_PWM0);
    // timer_channel_output_shadow_config(TIMER0, timer_channel, TIMER_OC_SHADOW_DISABLE);

    if (callback) {
        nvic_irq_enable(TIMER0_Channel_IRQn, 8, 0U);                     // TIMER 中断设置，抢占优先级，子优先级

        timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);
        timer_interrupt_enable(TIMER0, TIMER_INT_UP);                   // 使能更新中断

        timer0_update = callback;
    }

    timer_disable(TIMER0);
}


/**
 * @brief   使能 TIMER0
 *
 */
void bsp_timer0_enable(void)
{
    timer_enable(TIMER0);
    timer_primary_output_config(TIMER0, ENABLE);                        // 高级定时器要输出PWM，要额外配置此函数
}


/**
 * @brief   失能 TIMER0
 *
 */
void bsp_timer0_disable(void)
{
    timer_disable(TIMER0);
    timer_primary_output_config(TIMER0, DISABLE);                        // 高级定时器要输出PWM，要额外配置此函数
}


/**
 * @brief   设置 TIMER0 PWM 的占空比
 *
 * @param   timer_channel: 要改变占空比的 TIMER 通道
 * @param            duty: 占空比
 */
void bsp_timer0_set_pulse(uint16_t timer_channel, uint32_t duty)
{
    timer_channel_output_pulse_value_config(TIMER0, timer_channel, duty);
}


/**
 * @brief   TIMER2 PWM 配置及初始化
 *
 * @param       period_us: 设置自动重装载寄存器的值，每个计数表示一微秒
 * @param            duty: PWM 占空比
 * @param   timer_channel: 使用 TIMER2 的 GPIO 引脚对应的 TIMER 通道
 * @param        callback: 定时器计数更新回调函数
 */
void bsp_timer2_pwm_init(uint32_t period_us, uint32_t duty, uint16_t timer_channel, timer2_update_callback callback)
{
    rcu_periph_clock_enable(RCU_TIMER2);
    timer_deinit(TIMER2);

    timer_parameter_struct timer_initpara;

    timer_initpara.prescaler         = 12000 - 1;                       // 时钟预分频数
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;              // 对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;                // 向上计数模式
    timer_initpara.period            = period_us * 10 - 1;              // 自动重装载寄存器周期的值(计数值)
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;                // 采样分频
    timer_initpara.repetitioncounter = 0;

    timer_init(TIMER2, &timer_initpara);

    timer_oc_parameter_struct timer_oc_initpara;

    timer_oc_initpara.outputstate  = TIMER_CCX_ENABLE;                  // 通道输出状态
    timer_oc_initpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;            // 通道输出极性
    timer_oc_initpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;           // 通道处于空闲时的输出
    timer_oc_initpara.outputnstate = TIMER_CCXN_DISABLE;                // 互补通道输出状态
    timer_oc_initpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;           // 互补通道输出极性
    timer_oc_initpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;          // 互补通道处于空闲时的输出
    timer_channel_output_config(TIMER2, timer_channel, &timer_oc_initpara);

    timer_channel_output_pulse_value_config(TIMER2, timer_channel, duty * 10);
    timer_channel_output_mode_config(TIMER2, timer_channel, TIMER_OC_MODE_PWM0);
    // timer_channel_output_shadow_config(TIMER2, timer_channel, TIMER_OC_SHADOW_DISABLE);

    if (callback) {
        nvic_irq_enable(TIMER2_IRQn,8, 0U);                             // TIMER 中断设置，抢占优先级，子优先级

        timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);
        timer_interrupt_enable(TIMER2, TIMER_INT_UP);                   // 使能更新中断

        timer2_update = callback;
    }
}


/**
 * @brief   使能 TIMER2
 *
 */
void bsp_timer2_enable(void)
{
    timer_enable(TIMER2);
}


/**
 * @brief   失能 TIMER2
 *
 */
void bsp_timer2_disable(void)
{
    timer_disable(TIMER2);
}


/**
 * @brief   设置 TIMER2 PWM 的占空比
 *
 * @param   timer_channel: 要改变占空比的 TIMER 通道
 * @param            duty: 占空比
 */
void bsp_timer2_set_pulse(uint16_t timer_channel, uint32_t duty)
{
    timer_channel_output_pulse_value_config(TIMER2, timer_channel, duty);
}


/**
 * @brief   TIMER0 计数更新中断
 *
 */
void TIMER0_Channel_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_UP) == SET) {
        timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);

        timer0_update();
    }
}


/**
 * @brief   TIMER2 计数更新中断
 *
 */
void TIMER2_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER2, TIMER_INT_FLAG_UP) == SET) {
        timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);

        timer2_update();
    }
}
