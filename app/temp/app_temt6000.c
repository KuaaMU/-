/**
 * @file    temt6000.c
 * @brief   TEMT6000 是一种光敏电阻器，主要用于测量环境光强度
 *
 * @version 2024-08-07, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "temt6000.h"
#include "systick.h"


/**
 * @brief   TEMT6000 光敏传感器引脚配置及初始化
 *
 */
void temt6000_init(void)
{
    rcu_periph_clock_enable(TEMT_RCU);
	gpio_mode_set(TEMT_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, TEMT_PIN);

	rcu_periph_clock_enable(TEMT_ADC_RCU);
	adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
	adc_channel_length_config(TEMT_ADC, ADC_ROUTINE_CHANNEL, (uint8_t)TEMT_ADC_CH);
	adc_routine_channel_config(TEMT_ADC, 0, TEMT_ADC_CH, ADC_SAMPLETIME_144);

	adc_special_function_config(TEMT_ADC, ADC_CONTINUOUS_MODE, ENABLE);
	adc_data_alignment_config(TEMT_ADC, ADC_DATAALIGN_RIGHT);
	adc_resolution_config(TEMT_ADC, ADC_RESOLUTION_12B);

	adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
	adc_sync_delay_config(ADC_SYNC_DELAY_5CYCLE);

	adc_enable(TEMT_ADC);
	delay_1ms(1);
	adc_calibration_enable(TEMT_ADC);
}


/**
 * @brief   读取 ADC 通道值
 *
 * @return uint16_t 返回读取到的 ADC 通道值
 */
uint16_t temt6000_get_channel_data(void)
{
    adc_routine_channel_config(TEMT_ADC, 0, TEMT_ADC_CH, ADC_SAMPLETIME_144);
    adc_software_trigger_enable(TEMT_ADC, ADC_ROUTINE_CHANNEL);

    while (RESET == adc_flag_get(TEMT_ADC, ADC_FLAG_EOC));

    uint16_t val = adc_routine_data_read(TEMT_ADC);

    adc_flag_clear(TEMT_ADC, ADC_FLAG_EOC);

    return val;
}


/**
 * @brief   将 ADC 通道值转换成电压值
 *
 * @return float 返回电压值
 */
float temt6000_get_voltage(void)
{
    uint16_t val = temt6000_get_channel_data();

    return (val / 4096.0) * 3.3F;
}


/**
 * @brief   获取 TEMT6000 光敏传感器的 LUX 值
 *
 * @return float
 */
float temt6000_get_lux(void)
{
    uint16_t val = temt6000_get_channel_data();

    /********************************************
     * 以下 LUX 的换算并不准确，暂未查询到正确公式
     ********************************************/
    return (val / 4096.0) * 1000;
}
