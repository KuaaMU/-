#include "ky037.h"
#include "sr602.h"
#include "mq2.h"

key037_exti_callback ky037_exti;

/**
  * @brief  ky037模块中断初始化函数
  * @param  无
  * @retval 无
  */
void key037_interrupt_init(void)
{
	//开启引脚时钟
	rcu_periph_clock_enable(KY037_DI_RCU);

	//配置引脚为浮空输入模式
	gpio_mode_set(KY037_DI_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, KY037_DI_PIN);

	//开启系统配置时钟
	rcu_periph_clock_enable(RCU_SYSCFG);

	//使能NVIC中断 中断分组为2位抢占优先级，2位子优先级
	nvic_irq_enable(KYO37_EXTI_IRQ, 8, 0U;

	// 连接中断线到GPIO
	syscfg_exti_line_config(KYO37_EXTI_SOURCE_PORT,KYO37_EXTI_SOURCE_PIN);

	// 初始化中断线上降沿触发
	exti_init(KYO37_EXIT_X,EXTI_INTERRUPT,EXTI_TRIG_RISING);
	// 使能中断
	exti_interrupt_enable(KYO37_EXIT_X);
	// 清除中断标志位
	exti_interrupt_flag_clear(KYO37_EXIT_X);
}

/**
  * @brief  ky037模块模拟量输入初始化函数
  * @param  无
  * @retval 无
  */
void ky037_analog_init(void)
{
	/**********************GPIO config***********************/

	//开启引脚时钟
	rcu_periph_clock_enable(KY037_AI_RCU);
	//配置引脚为复用输入模式
	gpio_mode_set(KY037_AI_PORT, GPIO_MODE_ANALOG , GPIO_PUPD_NONE, KY037_AI_PIN);

	/***********************ADC config***********************/
	// 配置需要用到的ADC时钟
	rcu_periph_clock_enable(KY037_ADC_RCU);
	// 重置ADC
	adc_deinit();
	// 配置ADC主频
	adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
	// 设置ADC分辨率为12位
	adc_resolution_config(KY037_ADC, ADC_RESOLUTION_12B);
	// ADC数据右对齐
	adc_data_alignment_config(KY037_ADC, ADC_DATAALIGN_RIGHT);
	// 配置通道和通道数量
	adc_routine_channel_config(KY037_ADC, 0, KY037_ADC_CHANNEL, ADC_SAMPLETIME_15);
	adc_channel_length_config(KY037_ADC, ADC_ROUTINE_CHANNEL, 1);

	// 取消扫描模式
	adc_special_function_config(KY037_ADC, ADC_SCAN_MODE, DISABLE);
	//取消连续模式
	adc_special_function_config(KY037_ADC, ADC_CONTINUOUS_MODE, DISABLE);

	// 打开ADC
	adc_enable(KY037_ADC);

	delay_1ms(1);

	// 校准ADC
	adc_calibration_enable(KY037_ADC);
	delay_1ms(100);
}

/**
  * @brief  ky037模块初始化函数
  * @param  无
  * @retval 无
  */
void ky037_init(key037_exti_callback callback)
{
	ky037_analog_init();
	if(callback){
		key037_interrupt_init();
		ky037_exti = callback;
	}
}

/**
 * @brief   读取 ADC 通道值
 *
 * @return uint16_t 返回读取到的 ADC 通道值
 */
uint16_t ky037_get_channel_data(void)
{
	adc_routine_channel_config(KY037_ADC, 0, KY037_ADC_CHANNEL, ADC_SAMPLETIME_144);
	adc_software_trigger_enable(KY037_ADC, ADC_ROUTINE_CHANNEL);

	while (RESET == adc_flag_get(KY037_ADC, ADC_FLAG_EOC));
	uint16_t val = adc_routine_data_read(KY037_ADC);
	adc_flag_clear(KY037_ADC, ADC_FLAG_EOC);
	return val;
}

/**
 * @brief   将 ADC 通道值转换成电压值
 *
 * @return float 返回电压值
 */
float ky037_get_voltage(void)
{
    uint16_t val = ky037_get_channel_data();

    return (val / 4096.0) * 3.3F;
}

void EXTI5_9_IRQHandler(void)
{
	if(exti_interrupt_flag_get(KYO37_EXIT_X) == SET){   // 中断标志位为1
		exti_interrupt_flag_clear(KYO37_EXIT_X);            // 清中断标志位
		ky037_exti();
	}

	if(exti_interrupt_flag_get(MQ2_EXIT_X) == SET){   // 中断标志位为1
		exti_interrupt_flag_clear(MQ2_EXIT_X);            // 清中断标志位
		mq2_exti();
	}

	// SR602 模块的中断函数
    if (exti_interrupt_flag_get(SR602_EXTI) == SET) {
        exti_interrupt_flag_clear(SR602_EXTI);

        sr602_exti();
    }
}
