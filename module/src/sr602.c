/**
 * @file    sr602.c
 * @brief   SR602 人体感应模块
 *
 * @version 2024-08-11, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "sr602.h"


sr602_exti_callback sr602_exti;

/**
 * @brief   SR602 人体感应模块引脚和中断配置及初始化
 *
 */
void sr602_init(sr602_exti_callback callback)
{
    rcu_periph_clock_enable(SR602_RCU);

    gpio_mode_set(SR602_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SR602_PIN);
    gpio_output_options_set(SR602_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SR602_PIN);

    if (callback) {
        rcu_periph_clock_enable(RCU_SYSCFG);

        nvic_irq_enable(SR602_EXTI_IRQn, 8, 0U);
        syscfg_exti_line_config(SR602_EXTI_SOURCE_PORT, SR602_EXTI_SOURCE_PIN);

        exti_init(SR602_EXTI, EXTI_INTERRUPT, EXTI_TRIG_RISING);
        exti_interrupt_flag_clear(SR602_EXTI);

        sr602_exti = callback;
    }
}


/**
 * @brief   中断函数
 *
 * @remark  因中断入口与 KY-037 模块相同引起冲突，中断函数已移植到 KY-037 模块当中
 *
 */
// void EXTI5_9_IRQHandler(void)
// {
//     if (exti_interrupt_flag_get(SR602_EXTI) == SET) {
//         exti_interrupt_flag_clear(SR602_EXTI);

//         sr602_exti();
//     }
// }
