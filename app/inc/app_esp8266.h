#ifndef __APP_ESP8266_H
#define __APP_ESP8266_H

#include "gd32f4xx.h"
#include "esp8266.h"
#include "systick.h"
#include "string.h"
#include "stdio.h"


extern struct esp8266_send_data esp8266_send;
extern struct esp8266_receive_data esp8266_receive;
extern char esp8266_rx_buffer1[ESP8266_MAX_RECV_LEN/4];
extern char esp8266_rx_buffer2[ESP8266_MAX_RECV_LEN];
extern char esp8266_tx_buffer[ESP8266_MAX_SEND_LEN];
extern uint16_t rev_buffer1_len;
extern uint16_t rev_buffer2_len;
extern uint8_t rev_flag;
extern unsigned char rxbuffer_mod;
//代表接收到新数据的标志位
extern char new_data_flag ;
//-------------------小屋配置参数---------------------
//小屋的编号
extern char *smart_number;
//-------------------WiFi配置参数--------------------
//wifi的名称
extern char *ssid;
//wifi的密码
extern char *wifi_password;
//--------------------MQTT配置参数(用户)------------------
//接入mqtt的用户ID
extern char *user_id;
//接入mqtt的用户密码
extern char *user_psw;
//--------------------mqtt配置参数(服务器)-------------------
//mqtt的地址
extern char *mqtt_server;
extern int mqtt_port;
//接入mqtt服务器的客户端ID
extern char *mqtt_client_id ;
//接入mqtt服务器的用户名称
extern char *mqtt_username;
//mqtt服务器的密码
extern char *mqtt_password;
//--------------------指令集-------------------------------
//获取wifi的指令
extern char get_wifi_command[96];
//接入mqtt服务器的用户设置配置
extern char set_mqtt_server_command[96];
//接入mqtt的用户设置指令
extern char set_user_command[96];
//小屋发布，APP订阅
extern char publish_topic[96];
//小屋订阅，APP发布
extern char subscribe_topic[96];
//发布指令
extern char publish_topic_command[96];
//接收指令
extern char subscribe_topic_command[256];

void app_esp8266_proc(void);
void app_esp8266_connect(void);
void app_sendMqttPubCommand(const char* topic, const char* data, int qos, int retain);
void app_sendMqttPubRawCommand(const char* topic,char* data, int qos, int retain);
void app_esp8266_clear_old_data(void);
void app_esp8266_senddata_update(void);
void app_mqtt_clean(void);
uint8_t app_esp8266_get_new_data(void);
void app_esp8266_GloData_update(void);
void app_esp8266_GloData_execu(void);


#endif




