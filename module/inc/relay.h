#ifndef __DELAY_H
#define __DELAY_H

#define RELAY1_RCU          RCU_GPIOD
#define RELAY1_PORT         GPIOD
#define RELAY1_PIN          GPIO_PIN_6

#define RELAY2_RCU          RCU_GPIOD
#define RELAY2_PORT         GPIOD
#define RELAY2_PIN          GPIO_PIN_7


void relay1_init(void);
void relay2_init(void);
void relay1_on(void);
void relay1_off(void);
void relay2_on(void);
void relay2_off(void);


#endif
