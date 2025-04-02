#ifndef __TEMT_6000_H
#define __TEMT_6000_H


#define TEMT_RCU        RCU_GPIOF
#define TEMT_PORT       GPIOF
#define TEMT_PIN        GPIO_PIN_10

#define TEMT_ADC_RCU    RCU_ADC2
#define TEMT_ADC        ADC2
#define TEMT_ADC_CH     ADC_CHANNEL_8


void temt6000_init(void);
float temt6000_get_lux(void);

#endif
