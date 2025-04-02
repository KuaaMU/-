/**
 * @file    bsp_adc.c
 * @brief   简单的 ADC 板级支持包
 * @remark  主要用于快速测试和演示，模块中需要用到 ADC 时建议重写
 *
 * @version 2024-08-07, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "bsp_adc.h"
#include "systick.h"


/**
 * @brief   通过外设 PERIPH 获取外设时钟 RCU_PERIPH
 *
 */
rcu_periph_enum bsp_adc_get_rcu(uint32_t periph)
{
	switch (periph) {
		case GPIOA:
			return RCU_GPIOA;
		case GPIOB:
			return RCU_GPIOB;
		case GPIOC:
			return RCU_GPIOC;
		case GPIOF:
			return RCU_GPIOF;
		case ADC0:
			return RCU_ADC0;
		case ADC1:
			return RCU_ADC1;
		case ADC2:
			return RCU_ADC2;
		default:
			return (rcu_periph_enum)NULL;
	}
}


/**
 * @brief   配置 ADC 及初始化
 *
 * @param   init_struct: 用于配置 ADC 的结构体
 */
void bsp_adc_config(adc_init_struct_t init_struct)
{
	// adc_deinit();

	rcu_periph_clock_enable(bsp_adc_get_rcu(init_struct.gpio_periph));
	gpio_mode_set(init_struct.gpio_periph, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, init_struct.gpio_pin);

	uint32_t adc_x = init_struct.adc_periph;

	rcu_periph_clock_enable(bsp_adc_get_rcu(adc_x));
	adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
	adc_channel_length_config(adc_x, ADC_ROUTINE_CHANNEL, (uint8_t)init_struct.adc_channel);
	adc_routine_channel_config(adc_x, 0, init_struct.adc_channel, ADC_SAMPLETIME_144);

	adc_special_function_config(adc_x, ADC_CONTINUOUS_MODE, ENABLE);
	adc_data_alignment_config(adc_x, ADC_DATAALIGN_RIGHT);
	adc_resolution_config(adc_x, ADC_RESOLUTION_12B);

	adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
	adc_sync_delay_config(ADC_SYNC_DELAY_5CYCLE);

	adc_enable(adc_x);
	delay_1ms(1);
	adc_calibration_enable(adc_x);
}


/**
 * @brief   获取 ADC 通道的数据
 *
 * @param   adc_periph: 指定的 ADC 外设
 * @param      channel: 对应的 ADC 通道
 * @return uint16_t 返回获取到的通道数据
 */
uint16_t bsp_adc_get_channel_data(uint32_t adc_periph, uint8_t channel)
{
	uint16_t ad_val = 0;

	adc_routine_channel_config(adc_periph, 0, channel, ADC_SAMPLETIME_144);
	adc_software_trigger_enable(adc_periph, ADC_ROUTINE_CHANNEL);

	while (!adc_flag_get(adc_periph, ADC_FLAG_EOC));

	ad_val = adc_routine_data_read(adc_periph);

	adc_flag_clear(adc_periph, ADC_FLAG_EOC);

	return ad_val;
}


/**
 * @brief   获取 ADC 通道的数据，并转换成相应的电压值
 *
 * @param   adc_periph: 指定的 ADC 外设
 * @param      channel: 对应的 ADC 通道
 * @return uint16_t 返回换算后的电压值
 */
uint16_t bsp_adc_get_voltage(uint32_t adc_periph, uint8_t channel)
{
	uint16_t val = bsp_adc_get_channel_data(adc_periph, channel);

	return (val * ADC_BASE_VOLTAGE) / 4096.0;
}


/**
 * @brief   停用 ADC
 *
 * @param   adc_periph:
 */
void bsp_adc_disable(uint32_t adc_periph)
{
    adc_disable(adc_periph);
}
