#ifndef _BSP_ADC_H
#define _BSP_ADC_H


/**
 * @brief   用于初始化 ADC 的结构体，主要包含 GPIO 引脚及对应的 ADC 通道等
 *
 */
typedef struct {
    uint32_t gpio_periph;
    uint32_t gpio_pin;
    uint32_t adc_periph;
    uint32_t adc_channel;
} adc_init_struct_t;


#define ADC_BASE_VOLTAGE   		3300	// ADC基准电压

void bsp_adc_config(adc_init_struct_t init_struct);
uint16_t bsp_adc_get_channel_data(uint32_t adc_periph, uint8_t channel);
uint16_t bsp_adc_get_voltage(uint32_t adc_periph, uint8_t channel);
void bsp_adc_disable(uint32_t adc_periph);

#endif
