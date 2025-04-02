#ifndef __BSP_LED_H
#define __BSP_LED_H

typedef enum {
    LED1 = 0,
    LED2
} led_type_def;

#define LEDn                            2U

#define LED1_CLK                        RCU_GPIOE
#define LED1_PORT                       GPIOE
#define LED1_PIN                        GPIO_PIN_0

#define LED2_CLK                        RCU_GPIOA
#define LED2_PORT                       GPIOE
#define LED2_PIN                        GPIO_PIN_3


void bsp_led_init(void);
void bsp_led_on(led_type_def led);
void bsp_led_off(led_type_def led);
void bsp_led_toggle(led_type_def led);
uint8_t bsp_led_get_status(led_type_def led);

#endif
