#ifndef __ESP8266_H
#define __ESP8266_H

#include "gd32f4xx.h"
#include "systick.h"
#include "string.h"
#include "stdio.h"

#define ESP8266_RX_RCU 									RCU_GPIOA
#define ESP8266_RX_PORT        					GPIOA
#define ESP8266_RX_PIN									GPIO_PIN_10
#define ESP8266_RX_GPIO_AF							GPIO_AF_7

#define ESP8266_TX_RCU 									RCU_GPIOA
#define ESP8266_TX_PORT        					GPIOA
#define ESP8266_TX_PIN									GPIO_PIN_9
#define ESP8266_TX_GPIO_AF							GPIO_AF_7

#define ESP8266_UART_RCU								RCU_USART0
#define ESP8266_USART                   USART0
#define ESP8266_USART_BAUDRATE					115200
#define ESP8266_USART_IRQ           		USART0_IRQn

#define ESP8266_MAX_RECV_LEN						768
#define ESP8266_MAX_SEND_LEN						768

#define Success													1U
#define Failure													0U

struct esp8266_send_data{
	float temperature;								//温度
	float humidity;									//湿度
	int illumination;								//光照
	int pressure;									//气压
	int noise;										//噪声
	float concentration;							//烟雾浓度
	// char alarm[16];									//报警
	// char alarmType[32];								//报警类型

	char living_light[12];
    char living_home_light[12];
    char living_door[12];

	char fire_switch[12];
	char people_switch[12];
    char fire_alarm[12];
    char people_alarm[12];

    char kitchen_youyan[12];
    char kitchen_light[12];

    char bed_kongtiao[12];
    char bed_light[12];
    char bed_chuanglian[12];

    char bath_yuba[12];
    char bath_fan[12];
    char bath_reshui[12];
    char bath_light[12];
    char bath_chuanglian[12];
    char bath_window[12];

    char chuanglian_time[12];
};
struct esp8266_receive_data{
	char color[16];									//颜色
	char commandType[16];							//命令类型
	char device[32];								//设备
	char houseNumber[16];							//门牌号
	char mode[16];									//模式
	char keys[16];									//钥匙

    char living_light[12];
    char living_home_light[12];
    char living_door[12];

	char fire_switch[12];
	char people_switch[12];

    char kitchen_youyan[12];
    char kitchen_light[12];

    char bed_kongtiao[12];
    char bed_light[12];
    char bed_chuanglian[12];

    char bath_yuba[12];
    char bath_fan[12];
    char bath_reshui[12];
    char bath_light[12];
    char bath_chuanglian[12];
    char bath_window[12];

    char chuanglian_time[12];
};


void esp8266_peripheral_init(void);
uint8_t send_command(char *command,char *response,uint32_t time_out,uint8_t petry);
void topic_init(void);
void esp8266_publish_data_update(void);
uint8_t esp8266_init(void);
void esp8266_clr_txbuf(void);
void esp8266_uart_transmit(char *data,uint16_t size);
void esp8266_clr_rxbuf(void);


#endif




