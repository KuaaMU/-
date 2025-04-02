#include "beep.h"
#include "bsp_pwm.h"

/**
  * @brief  蜂鸣器引脚初始化函数
  * @param  无
  * @retval 无
  */
void beep_gpio_init(void)
{
	rcu_periph_clock_enable(BEEP_RCU);
	gpio_af_set(BEEP_PORT,GPIO_AF_1,BEEP_PIN);
	gpio_mode_set(BEEP_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BEEP_PIN);
	gpio_output_options_set(BEEP_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,BEEP_PIN);
	
}

/**
  * @brief  蜂鸣器定时器初始化函数
  * @param  无
  * @retval 无
  */
void beep_timer_init(void)
{
	rcu_periph_clock_enable(BEEP_TIMER_RCU);
	
	timer_deinit(BEEP_TIMER);
	
	timer_parameter_struct timer_initpara;
	timer_initpara.prescaler         = 480 - 1;                       // 时钟预分频数
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;            // 对齐模式
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;              // 向上计数模式
	timer_initpara.period            = 100 - 1;              				  // 自动重装载寄存器周期的值(计数值)
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;              // 采样分频
	timer_initpara.repetitioncounter = 0;
	
	timer_init(BEEP_TIMER, &timer_initpara);
	
	timer_oc_parameter_struct timer_oc_initpara;
	timer_oc_initpara.outputstate  = TIMER_CCX_ENABLE;                   //通道输出状态
	timer_oc_initpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;             //通道输出极性
	timer_oc_initpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;    				 //通道处于空闲时的输出
	timer_oc_initpara.outputnstate = TIMER_CCXN_ENABLE;          				 //互补通道输出状态
  timer_oc_initpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;            // 互补通道输出极性
  timer_oc_initpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;           // 互补通道处于空闲时的输出
	
	timer_channel_output_config(BEEP_TIMER, BEEP_CH, &timer_oc_initpara);
	
	timer_channel_output_pulse_value_config(BEEP_TIMER,BEEP_CH,50);
	timer_channel_output_mode_config(BEEP_TIMER, BEEP_CH, TIMER_OC_MODE_PWM0);

//	timer_channel_output_shadow_config(BEEP_TIMER, BEEP_CH, TIMER_OC_SHADOW_DISABLE);
//	
//  timer_auto_reload_shadow_enable(BEEP_TIMER);

}

/**
  * @brief  蜂鸣器初始化函数
  * @param  无
  * @retval 无
  */
void beep_init(void)
{
	beep_gpio_init();
	beep_timer_init();
}

/**
  * @brief  蜂鸣器使能函数
  * @param  无
  * @retval 无
  */
void beep_enable(void)
{
	timer_enable(BEEP_TIMER);
	timer_primary_output_config(BEEP_TIMER, ENABLE);
}

/**
  * @brief  蜂鸣器失能函数
  * @param  无
  * @retval 无
  */
void beep_disable(void)
{
		timer_disable(BEEP_TIMER);
		timer_primary_output_config(BEEP_TIMER, DISABLE);
}

/**
  * @brief  蜂鸣器鸣叫函数
  * @param  num：蜂鸣器鸣叫次数
  * @param  enable_time：蜂鸣器鸣叫时间
  * @param  disable_time：蜂鸣器停止鸣叫时间
  * @retval 无
  */
void beep_dididi(uint16_t num,uint16_t enable_time,uint16_t disable_time)
{
	uint16_t i = 0;
	for(;i<num;i++){
		beep_enable();
		delay_1ms(enable_time);
		beep_disable();
		delay_1ms(disable_time);
	}
}


