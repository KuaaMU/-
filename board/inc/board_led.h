#ifndef __BOARD_LED_H
#define __BOARD_LED_H

#define LED_OB_RCU          RCU_GPIOF
#define LED_OB_PORT         GPIOF
#define LED_OB_PIN          GPIO_PIN_6

void led_onboard_init(void);
void led_onboard_on(void);
void led_onboard_off(void);
void led_onboard_toggle(void);

#endif
