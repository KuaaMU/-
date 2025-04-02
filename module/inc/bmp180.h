#ifndef __BMP180_H
#define __BMP180_H


#define BMP180_SDA_RCU        RCU_GPIOB
#define BMP180_SDA_PORT       GPIOB
#define BMP180_SDA_PIN        GPIO_PIN_7

#define BMP180_SCL_RCU        RCU_GPIOB
#define BMP180_SCL_PORT       GPIOB
#define BMP180_SCL_PIN        GPIO_PIN_6

#define BMP180_SDA(x)         gpio_bit_write(BMP180_SDA_PORT, BMP180_SDA_PIN, (x ? SET : RESET))
#define BMP180_SCL(x)         gpio_bit_write(BMP180_SCL_PORT, BMP180_SCL_PIN, (x ? SET : RESET))
#define BMP180_SDA_OUT()	    gpio_mode_set(BMP180_SDA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, BMP180_SDA_PIN)
#define BMP180_SDA_IN()	      gpio_mode_set(BMP180_SDA_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, BMP180_SDA_PIN)
#define BMP180_SDA_GET()	    gpio_input_bit_get(BMP180_SDA_PORT, BMP180_SDA_PIN)


void bmp180_init(void);
float bmp180_get_temperature(void);
float bmp180_get_pressure(void);
float bmp180_get_altitude(float p);
void bmp180_calibrate(void);

#endif
