#include "esp8266.h"
#include "FreeRTOS.h"
#include "task.h"
#include "systick.h"
#include "stdio.h"
#include "string.h"
#include "execution_task.h"
#include "communication_task.h"

extern TaskHandle_t xTaskEsp8266Handle;

struct esp8266_send_data esp8266_send={.temperature = 0.0,\
										.humidity=0,\
										.illumination=0,\
										.pressure=0,\
										.noise=0,\
										.concentration=0,\

										.living_light="",\
										.living_home_light="",\
										.living_door="",\

										.kitchen_youyan="",\
										.kitchen_light="",\

										.bed_chuanglian="",\
										.bed_kongtiao="",\
										.bed_light="",\

										.bath_yuba="",\
										.bath_reshui="",\
										.bath_fan="",\
										.bath_chuanglian="",\
										.bath_window="",\
										.bath_light="",\

										.fire_alarm="",\
										.people_alarm="",\
										.fire_switch="on",\
										.people_switch="on", \
										.chuanglian_time=""
										};



struct esp8266_receive_data esp8266_receive;

extern DeviceStatus GolDeviceStatus;

//
//接收到的数组长度
uint16_t rev_buffer1_len = 0;
uint16_t rev_buffer2_len = 0;
//为1时代表接收到新的数据
uint8_t rev_flag = 0;
//代表接收到新数据的标志位
char new_data_flag = 0;
char esp8266_tx_flag = 0;
//值为1时，接收发出指令的返回值，
//值为0时，接收MQTT的返回值
unsigned char rxbuffer_mod = 0;
//用于接收发出指令返回的数据
char esp8266_rx_buffer1[ESP8266_MAX_RECV_LEN/2];
//用于接收APP发出的数据
char esp8266_rx_buffer2[ESP8266_MAX_RECV_LEN];
char esp8266_tx_buffer[ESP8266_MAX_SEND_LEN];

//-------------------小屋配置参数---------------------
//小屋的编号
char *smart_number = "33";
//-------------------WiFi配置参数--------------------
//wifi的名称
char *ssid = "KuaaMU";
//wifi的密码
char *wifi_password = "zxc159357";
// //--------------------MQTT配置参数(用户)------------------
// //接入mqtt的用户ID
// char *user_id = "mqtt_xmxll";
// //接入mqtt的用户密码
// char *user_psw = "12345678";
//--------------------mqtt配置参数(服务器)-------------------
//mqtt的地址
char *mqtt_server = "47.99.144.16";
int mqtt_port = 1883;
//接入mqtt服务器的客户端ID
char *mqtt_client_id = "mqttx_ccc45d44";
//接入mqtt服务器的用户名称
char *mqtt_username = "mqtt:root";
//mqtt服务器的密码
char *mqtt_password = "xmxllAdmin2020.";
//--------------------指令集-------------------------------
//获取wifi的指令
char get_wifi_command[96];
//接入mqtt服务器的用户设置配置
char set_mqtt_server_command[96];
//接入mqtt的用户设置指令
char set_user_command[96];
//小屋发布，APP订阅
char publish_topic[96];					//KuaaMU_subscribe*
//小屋订阅，APP发布
char subscribe_topic[96];				//KuaaMU_publish*
//发布指令
char publish_topic_command[96];
//接收指令
char subscribe_topic_command[256];
//检查mqtt服务器状态指令
char check_mqtt_server_command[96];



/**
  * @brief  ESP8266使用的IO引脚初始化
	* @param  无
  * @retval 无
  */
void esp8266_gpio_init(void)
{
	rcu_periph_clock_enable(ESP8266_RX_RCU);

	gpio_mode_set(ESP8266_RX_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, ESP8266_RX_PIN);
	gpio_output_options_set(ESP8266_RX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,ESP8266_RX_PIN);
	gpio_af_set(ESP8266_RX_PORT, ESP8266_RX_GPIO_AF, ESP8266_RX_PIN);

	rcu_periph_clock_enable(ESP8266_TX_RCU);

	gpio_mode_set(ESP8266_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, ESP8266_TX_PIN);
	gpio_output_options_set(ESP8266_TX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,ESP8266_TX_PIN);
	gpio_af_set(ESP8266_TX_PORT, ESP8266_TX_GPIO_AF, ESP8266_TX_PIN);
}

/**
  * @brief  ESP8266使用的串口初始化
	* @param  无
  * @retval 无
  */
void esp8266_uart_init(void)
{
	rcu_periph_clock_enable(ESP8266_UART_RCU);

	usart_deinit(ESP8266_USART);
	usart_baudrate_set(ESP8266_USART, ESP8266_USART_BAUDRATE);
	usart_parity_config(ESP8266_USART, USART_PM_NONE);
	usart_word_length_set(ESP8266_USART, USART_WL_8BIT);
	usart_stop_bit_set(ESP8266_USART, USART_STB_1BIT);


	usart_enable(ESP8266_USART);
	usart_transmit_config(ESP8266_USART, USART_TRANSMIT_ENABLE);
	usart_receive_config(ESP8266_USART, USART_RECEIVE_ENABLE);

	nvic_irq_enable(USART0_IRQn, 6, 0U);
	usart_interrupt_enable(ESP8266_USART, USART_INT_RBNE);

}

/**
  * @brief  ESP8266使用的外设初始化
	* @param  无
  * @retval 无
  */
void esp8266_peripheral_init(void)
{
	esp8266_gpio_init();
	esp8266_uart_init();
}

/**
  * @brief  使用串口发送一个byte到esp8266
	* @param  无
  * @retval 无
  */
void esp8266_uart_send_byte(uint8_t ch)
{
	usart_data_transmit( ESP8266_USART ,(uint8_t)ch);
	while(RESET == usart_flag_get(ESP8266_USART, USART_FLAG_TBE));
}

/**
  * @brief  使用串口发送一组数据到esp8266
	* @param  data:数组的地址
	* @param  size:数组的大小
  * @retval 无
  */
void esp8266_uart_transmit(char *data,uint16_t size)
{
	esp8266_tx_flag=1;
	do{
		esp8266_uart_send_byte(*data++);
		size--;
	}while(size != 0);
	esp8266_tx_flag=0;
}

/**
	* @brief  清空接收数据缓存区1
	* @param  无
  * @retval 无
  */
void esp8266_clr_rxbuf(void)
{
	memset(esp8266_rx_buffer1,0,sizeof(esp8266_rx_buffer1));
	rev_buffer1_len = 0;
}

/**
	* @brief  清空发送数据缓存区
	* @param  无
  * @retval 无
  */
void esp8266_clr_txbuf(void)
{
	memset(esp8266_tx_buffer,0,ESP8266_MAX_SEND_LEN);
}

//command:ָ发送的指令   response:接收数据中要有什么ֵ  time_out:超时时间ms   petry:尝试次数
/**
	* @brief  向ESP8266发送指令
	* @param  command：指令的首地址
	* @param  response：ESP8266返回的数据中要出现的字符串
	* @param  time_out:等待超时的时间，单位为1000ms
	* @param  petry:重试次数
  * @retval 无
  */
uint8_t send_command(char *command,char *response,uint32_t time_out,uint8_t petry)
{
	uint8_t n;
	uint32_t time_cont=0;
	rxbuffer_mod = 1;
	esp8266_clr_rxbuf();
	for(n=0;n<petry;n++){
		esp8266_uart_transmit((char *)command,strlen(command));
		// LOG("\r\n***************send****************\r\n");
		// LOG("%s\r\n",command);

		while(time_cont<time_out){
			delay_1ms(100);
			time_cont+=100;
			if(strstr((char *)esp8266_rx_buffer1,(char *)response) != NULL ){
				// LOG("\r\n***************receive****************\r\n");
				// LOG("%s\r\n",esp8266_rx_buffer1);
				rxbuffer_mod = 0;
				esp8266_clr_rxbuf();
				return Success;
			}
		}
	}
	LOG("\r\n***************receive err****************\r\n");
	LOG("%s\r\n",esp8266_rx_buffer1);
	esp8266_clr_rxbuf();
	rxbuffer_mod = 0;
	return Failure;
}

/**
	* @brief  订阅和发布的话题初始化
	* @param  无
  * @retval 无
  */
void topic_init(void)
{
	sprintf(publish_topic,"KuaaMU_subscribe*%s",smart_number);
	sprintf(subscribe_topic,"KuaaMU_publish*%s",smart_number);

	// sprintf(publish_topic_command,"AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n",publish_topic,esp8266_tx_buffer);
	sprintf(subscribe_topic_command,"AT+MQTTSUB=0,\"%s\",1\r\n",subscribe_topic);
}

/**
 * @brief  更新需要发布的数据
 * @param  无
 * @retval 无
 */
void esp8266_publish_data_update(void)
{
    if(esp8266_tx_flag == 1) return;
    LOG("esp8266_publish_data_update\r\n");
    memset(esp8266_tx_buffer, 0, ESP8266_MAX_SEND_LEN);

    // 构造要发布的JSON格式数据字符串
    sprintf(esp8266_tx_buffer,
        "{"
        "\"temperature\":\"%.2f\","  // 温度
        "\"humidity\":\"%.2f\","    // 湿度
        "\"illumination\":\"%d\","  // 光照
        "\"pressure\":\"%d\","      // 压力
        "\"CO\":\"%.2f\","         // 浓度
        "\"noise\":\"%d\","        // 噪声
        "\"living_light\":\"%s\","  // 客厅灯状态
        "\"living_home_light\":\"%s\","  // 客厅主灯状态
        "\"living_door\":\"%s\","  // 客厅门状态
        "\"fire_alarm\":\"%s\","    // 火警报警状态
        "\"people_alarm\":\"%s\","  // 人员报警状态
        "\"kitchen_youyan\":\"%s\","  // 厨房烟雾报警状态
        "\"kitchen_light\":\"%s\","  // 厨房灯状态
        "\"bed_kongtiao\":\"%s\","  // 卧室空调状态
        "\"bed_light\":\"%s\","     // 卧室灯状态
        "\"bed_chuanglian\":\"%s\","  // 卧室床帘状态
        "\"bath_yuba\":\"%s\","     // 浴室浴霸状态
        "\"bath_fan\":\"%s\","       // 浴室风扇状态
        "\"bath_reshui\":\"%s\","    // 浴室热水器状态
        "\"bath_light\":\"%s\","     // 浴室灯状态
        "\"bath_chuanglian\":\"%s\","  // 浴室窗帘状态
        "\"bath_window\":\"%s\","     // 浴室窗户状态
        "\"chuanglian_time\":\"%s\""   // 床帘时间状态
        "}",                       // JSON结束
        esp8266_send.temperature,  // 实际温度值
        esp8266_send.humidity,      // 实际湿度值
        esp8266_send.illumination,  // 实际光照值
        esp8266_send.pressure,     // 实际压力值
        esp8266_send.concentration,// 实际浓度值
        esp8266_send.noise,       // 实际噪声值
        GolDeviceStatus.living_light,  // 客厅灯状态
        GolDeviceStatus.living_home_light,  // 客厅主灯状态
        GolDeviceStatus.living_door,  // 客厅门状态
        GolDeviceStatus.fire_alarm,    // 火警报警状态
        GolDeviceStatus.people_alarm,  // 人员报警状态
        GolDeviceStatus.kitchen_youyan,  // 厨房烟雾报警状态
        GolDeviceStatus.kitchen_light,  // 厨房灯状态
        GolDeviceStatus.bed_kongtiao,  // 卧室空调状态
        GolDeviceStatus.bed_light,    // 卧室灯状态
        GolDeviceStatus.bed_chuanglian,  // 卧室床帘状态
        GolDeviceStatus.bath_yuba,    // 浴室浴霸状态
        GolDeviceStatus.bath_fan,      // 浴室风扇状态
        GolDeviceStatus.bath_reshui,   // 浴室热水器状态
        GolDeviceStatus.bath_light,    // 浴室灯状态
        GolDeviceStatus.bath_chuanglian, // 浴室窗帘状态
        GolDeviceStatus.bath_window,   // 浴室窗户状态
        GolDeviceStatus.chuanglian_time // 床帘时间状态
    );
}

/**
	* @brief  初始化wifi信息
	* @param  无
  * @retval 无
  */
void wifi_info_init(void)
{
	sprintf(get_wifi_command,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,wifi_password);
}

/**
	* @brief  初始化mqtt用户信息
	* @param  无
  * @retval 无
  */
void mqtt_user_init(void)
{
	sprintf(set_user_command,"AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",mqtt_client_id,mqtt_username,mqtt_password);
}

/**
	* @brief  配置mqtt服务器信息
	* @param  无
  * @retval 无
  */
void mqtt_server_init(void)
{
	sprintf(set_mqtt_server_command,"AT+MQTTCONN=0,\"%s\",%d,1\r\n",mqtt_server,mqtt_port);
}





//使用这个初始化，因为esp01s上电启动运行固件，所以调用前要注意esp01s上电时间到这个函数被调用的时间.
//如果在main函数始初初始化，最好延时几秒等esp01s模块上电初始化完成后再调用，测试的时候延时5s可以用。
/**
	* @brief  初始化esp8266连接MQTT服务器，并订阅话题
	* @param  无
  * @retval 初始化成功返回Success，失败返回Failure
  */
uint8_t esp8266_init(void)
{
	LOG("start esp8266 init\r\n");
	uint8_t err_flag = 0;
	wifi_info_init();
	mqtt_server_init();
	mqtt_user_init();
	topic_init();

	delay_1ms(2000);
	if(send_command("AT\r\n","OK\r\n",1000,2) == Success){
	}else{
		err_flag = 0;
		goto err;
	}
	if(send_command("ATE0\r\n","OK\r\n",1000,2) == Success){
	}else{
		err_flag = 1;
		goto err;
	}
	if(send_command("AT+CWMODE=1\r\n","OK\r\n",1000,2) == Success){
	}else{
		err_flag = 2;
		goto err;
	}
	if(send_command(get_wifi_command,"OK\r\n",10000,2) == Success){
	}else{
		err_flag = 3;
		goto err;
	}
	if(send_command(set_user_command,"OK\r\n",1000,2) == Success){
	}else{
		err_flag = 4;
		goto err;
	}
	delay_1ms(1000);
	if(send_command(set_mqtt_server_command,"OK\r\n",3000,2) == Success){
	}else{
		err_flag = 5;
		goto err;
	}
	delay_1ms(1000);
	if(send_command(subscribe_topic_command,"OK\r\n",1000,2) == Success){
	}else{
		err_flag = 6;
		goto err;
	}
	return Success;

err:

	LOG("esp8266 init err code:%d\r\n",err_flag);
	return Failure;

}

/**
	* @brief  串口0中断，用于接收数据
	* @param  无
  * @retval 无
  */
void USART0_IRQHandler(void)
{
	if(usart_interrupt_flag_get(ESP8266_USART, USART_INT_FLAG_RBNE) == SET){
		usart_interrupt_flag_clear(ESP8266_USART, USART_INT_FLAG_RBNE);
		if(rxbuffer_mod == 1){
			if (rev_buffer1_len < (ESP8266_MAX_RECV_LEN/4)){
				esp8266_rx_buffer1[rev_buffer1_len++] = usart_data_receive(ESP8266_USART);
			}
		}else if(rxbuffer_mod == 0){
			if (rev_buffer2_len < ESP8266_MAX_RECV_LEN){
				esp8266_rx_buffer2[rev_buffer2_len++] = usart_data_receive(ESP8266_USART);
				if(strstr(esp8266_rx_buffer2,"}\r\n") != NULL){
					//秉承中断快进快出原则，尽量不要在中断内处理大量数据
					new_data_flag = 1;
				}
			}
		}
	}
}



