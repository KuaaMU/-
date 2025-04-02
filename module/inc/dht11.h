#ifndef __DHT11_H
#define __DHT11_H

#define DHT11_RCU           RCU_GPIOB
#define DHT11_PORT          GPIOB
#define DHT11_PIN           GPIO_PIN_8

#define DHT11_OUTPUT()      gpio_mode_set(DHT11_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, DHT11_PIN)
#define DHT11_INPUT()       gpio_mode_set(DHT11_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, DHT11_PIN)
#define DHT11_GPIO(x)       gpio_bit_write(DHT11_PORT, DHT11_PIN, (x ? SET : RESET))
#define DHT11_GPIO_GET()    gpio_input_bit_get(DHT11_PORT, DHT11_PIN)

void dht11_init(void);
void dht11_read_data(float *temperature, float *humidity);

#endif
