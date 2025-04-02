#include "flame.h"

flame_exti_callback flame_exti;

/**
  * @brief  flame模块中断初始化函数
  * @param  无
  * @retval 无
  */
void flame_interrupt_init(void)
{
	//开启引脚时钟
	rcu_periph_clock_enable(FLAME_DI_RCU);

	//配置引脚为浮空输入模式
	gpio_mode_set(FLAME_DI_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, FLAME_DI_PIN);

	//开启系统配置时钟
	rcu_periph_clock_enable(RCU_SYSCFG);

	//使能NVIC中断 中断分组为2位抢占优先级，2位子优先级
	nvic_irq_enable(FLAME_EXTI_IRQ, 8, 0U);

	// 连接中断线到GPIO
	syscfg_exti_line_config(FLAME_EXTI_SOURCE_PORT,FLAME_EXTI_SOURCE_PIN);

	// 初始化中断线上降沿触发
	exti_init(FLAME_EXIT_X,EXTI_INTERRUPT,EXTI_TRIG_RISING);
	// 使能中断
	exti_interrupt_enable(FLAME_EXIT_X);
	// 清除中断标志位
	exti_interrupt_flag_clear(FLAME_EXIT_X);
}

/**
  * @brief  flame模块ADC初始化函数
  * @param  value：设置的阈值大小
  * @retval 无
  */
void flame_analog_init(void)
{
	/**********************GPIO config***********************/
	//开启引脚时钟
	rcu_periph_clock_enable(FLAME_AI_RCU);
	//配置引脚为复用输入模式
	gpio_mode_set(FLAME_AI_PORT, GPIO_MODE_ANALOG , GPIO_PUPD_NONE, FLAME_AI_PIN);

	/***********************ADC config***********************/
	// 配置需要用到的ADC时钟
	rcu_periph_clock_enable(FLAME_ADC_RCU);

	// 重置ADC
	adc_deinit();

	// 配置ADC主频
	adc_clock_config(ADC_ADCCK_PCLK2_DIV4);

	// 设置ADC分辨率为12位
	adc_resolution_config(FLAME_ADC, ADC_RESOLUTION_12B);

	// ADC数据右对齐
	adc_data_alignment_config(FLAME_ADC, ADC_DATAALIGN_RIGHT);

	// 配置通道和通道数量
	adc_routine_channel_config(FLAME_ADC, 0, FLAME_ADC_CHANNEL, ADC_SAMPLETIME_15);
	adc_channel_length_config(FLAME_ADC, ADC_ROUTINE_CHANNEL, 1);

	// 取消扫描模式
	adc_special_function_config(FLAME_ADC, ADC_SCAN_MODE, DISABLE);
	//取消连续模式
	adc_special_function_config(FLAME_ADC, ADC_CONTINUOUS_MODE, DISABLE);

	// 打开ADC
	adc_enable(FLAME_ADC);

	delay_1ms(1);

	// 校准ADC
	// adc_calibration_enable(FLAME_ADC);

	delay_1ms(100);
}

/**
  * @brief  flame模块初始化函数
  * @param  无
  * @retval 无
  */
void flame_init(flame_exti_callback callback)
{
	flame_analog_init();
	if(callback){
		flame_interrupt_init();
		flame_exti = callback;
	}
}

/**
 * @brief   flame模块读取ADC 通道值
 *
 * @return uint16_t 返回读取到的 ADC 通道值
 */
uint16_t flame_get_channel_data(void)
{
	adc_routine_channel_config(FLAME_ADC, 0, FLAME_ADC_CHANNEL, ADC_SAMPLETIME_144);
	adc_software_trigger_enable(FLAME_ADC, ADC_ROUTINE_CHANNEL);

	while (RESET == adc_flag_get(FLAME_ADC, ADC_FLAG_EOC));
	uint16_t val = adc_routine_data_read(FLAME_ADC);
	adc_flag_clear(FLAME_ADC, ADC_FLAG_EOC);
	return val;
}

/**
 * @brief   将ADC通道值转换成电压值
 *
 * @return float 返回电压值
 */
float flame_get_voltage(void)
{
    uint16_t val = flame_get_channel_data();

    return (val / 4096.0) * 3.3F;
}

void EXTI10_15_IRQHandler(void)
{
	if(exti_interrupt_flag_get(FLAME_EXIT_X) == SET){   // 中断标志位为1
		exti_interrupt_flag_clear(FLAME_EXIT_X);            // 清中断标志位
		flame_exti();
	}
}
