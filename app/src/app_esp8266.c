#include "app_esp8266.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"
#include "cJSON.h"
#include "safety_task.h"
#include "sensor_task.h"
#include "execution_task.h"
#include "app_motor.h"  // 直流电机控制驱动
#include "app_relay.h"  // 继电器控制驱动
#include "app_ws2812.h" // WS2812 LED灯珠控制驱动
#include "app_tc1508.h" /// 步进电机控制驱动
#include "app_fpm383.h"
#include "app_esp8266.h"
#include "app_dwin.h"


extern TaskHandle_t xTaskDisplayHandle;

extern SensorData sensor_data;
extern SafetyData safety_data;
extern DeviceStatus GolDeviceStatus;

extern beepData beep;
extern relayData relay1;
extern relayData relay2;
extern DCmotorData dcMotor;
extern ws2812Data ws2812;
extern tc1508Data_DC tc1508_dc;
extern tc1508Data_Stepper tc1508_stepper;

uint8_t app_esp8266_state = 0x00;
void app_esp8266_proc(void)
{
  switch (app_esp8266_state)
  {
  case 0x00:
    app_esp8266_connect();
    break;

  case 0x01:
    if (app_esp8266_get_new_data())
    {
      // app_esp8266_GloData_update();
      app_esp8266_GloData_execu();
      xTaskNotifyGive(xTaskDisplayHandle);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    break;

  case 0x02:
    // 更新需要发布的数据
    app_esp8266_senddata_update();
    esp8266_publish_data_update();
    // 发布数据
    app_sendMqttPubRawCommand(publish_topic, esp8266_tx_buffer, 0, 0);

    sprintf(GolDeviceStatus.fire_alarm,"%s","");
    sprintf(GolDeviceStatus.people_alarm,"%s","");
    app_esp8266_state = 0x01;
    break;
  }
}

void app_esp8266_connect(void)
{
  uint8_t esp8266_status = Failure;
  do
  {
    app_mqtt_clean();
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp8266_status = esp8266_init();

    if (Success != esp8266_status)
    {
      LOG("esp8266_init fail\r\n");
    }
  } while (Success != esp8266_status);
  LOG("esp8266_init success\r\n");
  app_esp8266_state = 0x01;
}

/**
 * @brief  发送mqtt发布命令（大量数据）
 * @param  topic:主题
 * @param  data:数据
 * @param  qos:qos等级
 * @param  retain:保留等级
 * @retval 无
 */
void app_sendMqttPubRawCommand(const char *topic, char *data, int qos, int retain)
{
  sprintf(publish_topic_command, "AT+MQTTPUBRAW=0,\"%s\",%d,%d,%d\r\n", topic, strlen(data), qos, retain);
  if (send_command(publish_topic_command, "OK\r\n\r\n>", 1000, 1) != Success)
  {
    app_esp8266_state = 0x00;
  }
  else
  {
    esp8266_uart_transmit(data, strlen(data));
    LOG("\r\n***************send****************\r\n");
    // LOG("%s\r\n", data);
    // LOG("\r\n***************receive****************\r\n");
    // LOG("%s\r\n",esp8266_rx_buffer1);
    rxbuffer_mod = 0;
    esp8266_clr_rxbuf();
  }
}

/**
 * @brief  发送mqtt发布命令
 * @param  topic:主题
 * @param  data:数据
 * @param  qos:qos等级
 * @param  retain:保留等级
 * @retval 无
 */
void app_sendMqttPubCommand(const char *topic, const char *data, int qos, int retain)
{
  char commond[256];
  sprintf(commond, "AT+MQTTPUB=0,\"%s\",\\\"%s\\\",%d,%d\r\n", topic, data, qos, retain);
  send_command(commond, "OK", 1000, 1);
}

/**
 * @brief  清除旧的数据
 * @param  无
 * @retval 无
 */
void app_esp8266_clear_old_data(void)
{

  // 清空接收缓冲区
  memset(esp8266_rx_buffer2, 0, sizeof(esp8266_rx_buffer2));
  rev_buffer2_len = 0;
  new_data_flag = 0;
}

/**
 * @brief  更新需要发布的数据
 * @param  无
 * @retval 无
 */
void app_esp8266_senddata_update(void)
{
  esp8266_send.temperature = sensor_data.temperature_dht11;
  esp8266_send.humidity = sensor_data.humidity;
  esp8266_send.pressure = (int)sensor_data.pressure;
  esp8266_send.illumination = (int)sensor_data.lux;
  esp8266_send.noise = (int)sensor_data.noise_dB;
  esp8266_send.concentration = sensor_data.ppm;

  sprintf(esp8266_send.living_light, "%s", GolDeviceStatus.living_light);
  sprintf(esp8266_send.living_door, "%s", GolDeviceStatus.living_door);
  sprintf(esp8266_send.living_home_light, "%s", GolDeviceStatus.living_home_light);

  sprintf(esp8266_send.kitchen_light, "%s", GolDeviceStatus.kitchen_light);
  sprintf(esp8266_send.kitchen_youyan, "%s", GolDeviceStatus.kitchen_youyan);

  sprintf(esp8266_send.bed_chuanglian, "%s", GolDeviceStatus.bed_chuanglian);
  sprintf(esp8266_send.bed_light, "%s", GolDeviceStatus.bed_light);
  sprintf(esp8266_send.bed_kongtiao, "%s", GolDeviceStatus.bed_kongtiao);

  sprintf(esp8266_send.bath_chuanglian, "%s", GolDeviceStatus.bath_chuanglian);
  sprintf(esp8266_send.bath_fan, "%s", GolDeviceStatus.bath_fan);
  sprintf(esp8266_send.bath_light, "%s", GolDeviceStatus.bath_light);
  sprintf(esp8266_send.bath_reshui, "%s", GolDeviceStatus.bath_reshui);
  sprintf(esp8266_send.bath_window, "%s", GolDeviceStatus.bath_window);
  sprintf(esp8266_send.bath_yuba, "%s", GolDeviceStatus.bath_yuba);

  sprintf(esp8266_send.fire_alarm, "%s", GolDeviceStatus.fire_alarm);
  sprintf(esp8266_send.people_alarm, "%s", GolDeviceStatus.people_alarm);
  sprintf(esp8266_send.fire_switch, "%s", GolDeviceStatus.fire_switch);
  sprintf(esp8266_send.people_switch, "%s", GolDeviceStatus.people_switch);

  sprintf(esp8266_send.chuanglian_time, "%s", GolDeviceStatus.chuanglian_time);
}

/**
 * @brief  断开mqtt连接
 * @param  无
 * @retval 无
 */
void app_mqtt_clean(void)
{
  send_command("AT+MQTTCLEAN=0\r\n", "OK", 1000, 1);
}

/**
 * @brief  获取esp8266发送过来的数据
 * @param  无
 * @retval 无
 */
uint8_t app_esp8266_get_new_data(void)
{
  if (new_data_flag)
  {
    LOG("---------------------get new data-------------------------\r\n");
    const char *response = esp8266_rx_buffer2;
    // LOG("\r\nReceived response: %s\r\n", response);

    // 查找 JSON 数据的起始位置，即在 <data_length>, 之后
    const char *json_start = strchr(response, '{'); // 跳过 LinkID
    if (json_start != NULL)
    {
      // 使用 cJSON 解析 JSON 数据
      cJSON *json_obj = cJSON_Parse(json_start);
      if (json_obj == NULL)
      {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
          LOG("Error before: %s\r\n", error_ptr);
        }
        app_esp8266_clear_old_data();
        return Failure; // 添加返回值
      }
      else
      {
        cJSON *bath_chuanglian = cJSON_GetObjectItem(json_obj, "bath_chuanglian");
        if (cJSON_IsString(bath_chuanglian) && (bath_chuanglian->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.bath_chuanglian, bath_chuanglian->valuestring, sizeof(GolDeviceStatus.bath_chuanglian) - 1);
          GolDeviceStatus.bath_chuanglian[sizeof(GolDeviceStatus.bath_chuanglian) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("bath_chuanglian: %s\r\n", GolDeviceStatus.bath_chuanglian);
        }

        cJSON *bath_fan = cJSON_GetObjectItem(json_obj, "bath_fan");
        if (cJSON_IsString(bath_fan) && (bath_fan->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.bath_fan, bath_fan->valuestring, sizeof(GolDeviceStatus.bath_fan) - 1);
          GolDeviceStatus.bath_fan[sizeof(GolDeviceStatus.bath_fan) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("bath_fan: %s\r\n", GolDeviceStatus.bath_fan);
        }

        cJSON *bath_reshui = cJSON_GetObjectItem(json_obj, "bath_reshui");
        if (cJSON_IsString(bath_reshui) && (bath_reshui->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.bath_reshui, bath_reshui->valuestring, sizeof(GolDeviceStatus.bath_reshui) - 1);
          GolDeviceStatus.bath_reshui[sizeof(GolDeviceStatus.bath_reshui) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("bath_reshui: %s\r\n", GolDeviceStatus.bath_reshui);
        }

        cJSON *bath_yuba = cJSON_GetObjectItem(json_obj, "bath_yuba");
        if (cJSON_IsString(bath_yuba) && (bath_yuba->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.bath_yuba, bath_yuba->valuestring, sizeof(GolDeviceStatus.bath_yuba) - 1);
          GolDeviceStatus.bath_yuba[sizeof(GolDeviceStatus.bath_yuba) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("bath_yuba: %s\r\n", GolDeviceStatus.bath_yuba);
        }
//----------------------------------------------------------------------------------------------------------------

        cJSON *bed_kongtiao = cJSON_GetObjectItem(json_obj, "bed_kongtiao");
        if (cJSON_IsString(bed_kongtiao) && (bed_kongtiao->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.bed_kongtiao, bed_kongtiao->valuestring, sizeof(GolDeviceStatus.bed_kongtiao) - 1);
          GolDeviceStatus.bed_kongtiao[sizeof(GolDeviceStatus.bed_kongtiao) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("bed_kongtiao: %s\r\n", GolDeviceStatus.bed_kongtiao);
        }

        // cJSON *bed_chuanglian = cJSON_GetObjectItem(json_obj, "bed_chuanglian");
        // if (cJSON_IsString(bed_chuanglian) && (bed_chuanglian->valuestring != NULL))
        // {
        //   strncpy(GolDeviceStatus.bed_chuanglian, bed_chuanglian->valuestring, sizeof(GolDeviceStatus.bed_chuanglian) - 1);
        //   GolDeviceStatus.bed_chuanglian[sizeof(GolDeviceStatus.bed_chuanglian) - 1] = '\0'; // 确保字符串以 NULL 结尾
        //   LOG("bed_chuanglian: %s\r\n", GolDeviceStatus.bed_chuanglian);
        // }

        // cJSON *bed_light = cJSON_GetObjectItem(json_obj, "bed_light");
        // if (cJSON_IsString(bed_light) && (bed_light->valuestring != NULL))
        // {
        //   strncpy(GolDeviceStatus.bed_light, bed_light->valuestring, sizeof(GolDeviceStatus.bed_light) - 1);
        //   GolDeviceStatus.bed_light[sizeof(GolDeviceStatus.bed_light) - 1] = '\0'; // 确保字符串以 NULL 结尾
        //   LOG("bed_light: %s\r\n", GolDeviceStatus.bed_light);
        // }

//-------------------------------------------------------------------------------------------------------------------------

        cJSON *chuanglian_time = cJSON_GetObjectItem(json_obj, "chuanglian_time");
        if (cJSON_IsString(chuanglian_time) && (chuanglian_time->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.chuanglian_time, chuanglian_time->valuestring, sizeof(GolDeviceStatus.chuanglian_time) - 1);
          GolDeviceStatus.chuanglian_time[sizeof(GolDeviceStatus.chuanglian_time) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("chuanglian_time: %s\r\n", GolDeviceStatus.chuanglian_time);
        }

        cJSON *fire_alarm = cJSON_GetObjectItem(json_obj, "fire_alarm");
        if (cJSON_IsString(fire_alarm) && (fire_alarm->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.fire_alarm, fire_alarm->valuestring, sizeof(GolDeviceStatus.fire_alarm) - 1);
          GolDeviceStatus.fire_alarm[sizeof(GolDeviceStatus.fire_alarm) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("fire_alarm: %s\r\n", GolDeviceStatus.chuanglian_time);
        }

        // 提取字段并更新 GolDeviceStatus 结构体
        cJSON *living_light = cJSON_GetObjectItem(json_obj, "living_light");
        if (cJSON_IsString(living_light) && (living_light->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.living_light, living_light->valuestring, sizeof(GolDeviceStatus.living_light) - 1);
          GolDeviceStatus.living_light[sizeof(GolDeviceStatus.living_light) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("living_light: %s\r\n", GolDeviceStatus.living_light);
        }

        cJSON *living_door = cJSON_GetObjectItem(json_obj, "living_door");
        if (cJSON_IsString(living_door) && (living_door->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.living_door, living_door->valuestring, sizeof(GolDeviceStatus.living_door) - 1);
          GolDeviceStatus.living_door[sizeof(GolDeviceStatus.living_door) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("living_door: %s\r\n", GolDeviceStatus.living_door);
        }

        cJSON *living_home_light = cJSON_GetObjectItem(json_obj, "living_home_light");
        if (cJSON_IsString(living_home_light) && (living_home_light->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.living_home_light, living_home_light->valuestring, sizeof(GolDeviceStatus.living_home_light) - 1);
          GolDeviceStatus.living_home_light[sizeof(GolDeviceStatus.living_home_light) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("living_home_light: %s\r\n", GolDeviceStatus.living_home_light);
        }

        // cJSON *kitchen_light = cJSON_GetObjectItem(json_obj, "kitchen_light");
        // if (cJSON_IsString(kitchen_light) && (kitchen_light->valuestring != NULL))
        // {
        //   strncpy(GolDeviceStatus.kitchen_light, kitchen_light->valuestring, sizeof(GolDeviceStatus.kitchen_light) - 1);
        //   GolDeviceStatus.kitchen_light[sizeof(GolDeviceStatus.kitchen_light) - 1] = '\0'; // 确保字符串以 NULL 结尾
        //   LOG("kitchen_light: %s\r\n", GolDeviceStatus.kitchen_light);
        // }

        cJSON *kitchen_youyan = cJSON_GetObjectItem(json_obj, "kitchen_youyan");
        if (cJSON_IsString(kitchen_youyan) && (kitchen_youyan->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.kitchen_youyan, kitchen_youyan->valuestring, sizeof(GolDeviceStatus.kitchen_youyan) - 1);
          GolDeviceStatus.kitchen_youyan[sizeof(GolDeviceStatus.kitchen_youyan) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("kitchen_youyan: %s\r\n", GolDeviceStatus.kitchen_youyan);
        }









        // cJSON *bath_light = cJSON_GetObjectItem(json_obj, "bath_light");
        // if (cJSON_IsString(bath_light) && (bath_light->valuestring != NULL))
        // {
        //   strncpy(GolDeviceStatus.bath_light, bath_light->valuestring, sizeof(GolDeviceStatus.bath_light) - 1);
        //   GolDeviceStatus.bath_light[sizeof(GolDeviceStatus.bath_light) - 1] = '\0'; // 确保字符串以 NULL 结尾
        //   LOG("bath_light: %s\r\n", GolDeviceStatus.bath_light);
        // }


        cJSON *bath_window = cJSON_GetObjectItem(json_obj, "bath_window");
        if (cJSON_IsString(bath_window) && (bath_window->valuestring != NULL))
        {
          strncpy(GolDeviceStatus.bath_window, bath_window->valuestring, sizeof(GolDeviceStatus.bath_window) - 1);
          GolDeviceStatus.bath_window[sizeof(GolDeviceStatus.bath_window) - 1] = '\0'; // 确保字符串以 NULL 结尾
          LOG("bath_window: %s\r\n", GolDeviceStatus.bath_window);
        }







        cJSON_Delete(json_obj); // 清理 cJSON 对象
      }
    }
    else
    {
      LOG("Failed to find JSON data in response\r\n");
    }

    // 清空旧数据，准备接收新的数据
    app_esp8266_clear_old_data();
    return Success; // JSON 解析成功
  }
  else
  {
    return Failure; // 没有新数据
  }
}

// void app_esp8266_GloData_update(void)
// {
//   LOG("Starting to update global data...\r\n");

//   // 更新 GolDeviceStatus.living_light
//   LOG("Updating living_light: %s\r\n", esp8266_receive.living_light);
//   sprintf(GolDeviceStatus.living_light, "%s", esp8266_receive.living_light);

//   // 更新 GolDeviceStatus.living_door
//   LOG("Updating living_door: %s\r\n", esp8266_receive.living_door);
//   sprintf(GolDeviceStatus.living_door, "%s", esp8266_receive.living_door);

//   // 更新 GolDeviceStatus.living_home_light
//   LOG("Updating living_home_light: %s\r\n", esp8266_receive.living_home_light);
//   sprintf(GolDeviceStatus.living_home_light, "%s", esp8266_receive.living_home_light);

//   // 更新 GolDeviceStatus.kitchen_light
//   LOG("Updating kitchen_light: %s\r\n", esp8266_receive.kitchen_light);
//   sprintf(GolDeviceStatus.kitchen_light, "%s", esp8266_receive.kitchen_light);

//   // 更新 GolDeviceStatus.kitchen_youyan
//   LOG("Updating kitchen_youyan: %s\r\n", esp8266_receive.kitchen_youyan);
//   sprintf(GolDeviceStatus.kitchen_youyan, "%s", esp8266_receive.kitchen_youyan);

//   // 更新 GolDeviceStatus.bed_chuanglian
//   LOG("Updating bed_chuanglian: %s\r\n", esp8266_receive.bed_chuanglian);
//   sprintf(GolDeviceStatus.bed_chuanglian, "%s", esp8266_receive.bed_chuanglian);

//   // 更新 GolDeviceStatus.bed_light
//   LOG("Updating bed_light: %s\r\n", esp8266_receive.bed_light);
//   sprintf(GolDeviceStatus.bed_light, "%s", esp8266_receive.bed_light);

//   // 更新 GolDeviceStatus.bed_kongtiao
//   LOG("Updating bed_kongtiao: %s\r\n", esp8266_receive.bed_kongtiao);
//   sprintf(GolDeviceStatus.bed_kongtiao, "%s", esp8266_receive.bed_kongtiao);

//   // 更新 GolDeviceStatus.bath_chuanglian
//   LOG("Updating bath_chuanglian: %s\r\n", esp8266_receive.bath_chuanglian);
//   sprintf(GolDeviceStatus.bath_chuanglian, "%s", esp8266_receive.bath_chuanglian);

//   // 更新 GolDeviceStatus.bath_fan
//   LOG("Updating bath_fan: %s\r\n", esp8266_receive.bath_fan);
//   sprintf(GolDeviceStatus.bath_fan, "%s", esp8266_receive.bath_fan);

//   // 更新 GolDeviceStatus.bath_light
//   LOG("Updating bath_light: %s\r\n", esp8266_receive.bath_light);
//   sprintf(GolDeviceStatus.bath_light, "%s", esp8266_receive.bath_light);

//   // 更新 GolDeviceStatus.bath_reshui
//   LOG("Updating bath_reshui: %s\r\n", esp8266_receive.bath_reshui);
//   sprintf(GolDeviceStatus.bath_reshui, "%s", esp8266_receive.bath_reshui);

//   // 更新 GolDeviceStatus.bath_window
//   LOG("Updating bath_window: %s\r\n", esp8266_receive.bath_window);
//   sprintf(GolDeviceStatus.bath_window, "%s", esp8266_receive.bath_window);

//   // 更新 GolDeviceStatus.bath_yuba
//   LOG("Updating bath_yuba: %s\r\n", esp8266_receive.bath_yuba);
//   sprintf(GolDeviceStatus.bath_yuba, "%s", esp8266_receive.bath_yuba);

//   // 更新 GolDeviceStatus.fire_switch
//   LOG("Updating fire_switch: %s\r\n", esp8266_receive.fire_switch);
//   sprintf(GolDeviceStatus.fire_switch, "%s", esp8266_receive.fire_switch);

//   // 更新 GolDeviceStatus.people_switch
//   LOG("Updating people_switch: %s\r\n", esp8266_receive.people_switch);
//   sprintf(GolDeviceStatus.people_switch, "%s", esp8266_receive.people_switch);

//   // 更新 GolDeviceStatus.chuanglian_time
//   LOG("Updating chuanglian_time: %s\r\n", esp8266_receive.chuanglian_time);
//   sprintf(GolDeviceStatus.chuanglian_time, "%s", esp8266_receive.chuanglian_time);

//   LOG("Global data update complete.\r\n");
// }

void app_esp8266_GloData_execu(void)
{
  LOG("update execu\r\n");

  // 客厅设备处理
  // LOG("---Check living home light status: %s\r\n", GolDeviceStatus.living_home_light);
  if (strlen(GolDeviceStatus.living_home_light) > 0)
  {
    handleLivingRoom(0x11, 0x01, strcmp(GolDeviceStatus.living_home_light, "on") == 0 ? 0x02 : 0x01);
  }

  // LOG("---Check living light status: %s\r\n", GolDeviceStatus.living_light);
  if (strlen(GolDeviceStatus.living_light) > 0)
  {
    // 根据状态字符串决定模式和调整值
    if (strcmp(GolDeviceStatus.living_light, "off") == 0)
    {
      LOG("Living light set off\r\n");
      handleLivingRoom(0x21, 0x01, 0x01); // adjust = 0x01 (关灯)
    }

    sprintf(GolDeviceStatus.living_home_light,"%s","off");
    sprintf(GolDeviceStatus.bed_light,"%s","off");
    sprintf(GolDeviceStatus.bath_light,"%s","off");
    sprintf(GolDeviceStatus.kitchen_light,"%s","off");

    if (strcmp(GolDeviceStatus.living_light, "on") == 0)
    {
      LOG("Living light set on\r\n");
      handleLivingRoom(0x21, 0x01, 0x07); // adjust = 0x07 (黄灯)
    }
    else if (strcmp(GolDeviceStatus.living_light, "yellow") == 0)
    {
      LOG("Living light set yellow\r\n");
      handleBedroom(0x21, 0x01, 0x07); //adjust = 0x07 (黄灯)
    }
    else if (strcmp(GolDeviceStatus.living_light, "red") == 0)
    {
      LOG("Living light set red\r\n");
      handleCommon(0x00, 0x01, 0x03); // adjust = 0x03 (红灯)
    }
    else if (strcmp(GolDeviceStatus.living_light, "blue") == 0)
    {
      LOG("Living light set blue\r\n");
      handleKitchen(0x21, 0x01, 0x05); // adjust = 0x05 (蓝灯)
    }
    else if (strcmp(GolDeviceStatus.living_light, "purple") == 0)
    {
      LOG("Living light set purple\r\n");
      handleBathroom(0x61, 0x01, 0x06); // adjust = 0x06 (紫灯)
    }
    else if (strcmp(GolDeviceStatus.living_light,"green") == 0)
    {
      LOG("Living light set green\r\n");
      handleLivingRoom(0x11, 0x01, 0x04); // adjust = 0x04 (绿灯)
    }
  } // 如果有更多的颜色，可以继续添加更多的else if分支

  // LOG("---Check living door status: %s\r\n", GolDeviceStatus.living_door);
  if (strlen(GolDeviceStatus.living_door) > 0)
  {
    handleLivingRoom(0x31, 0x01, strcmp(GolDeviceStatus.living_door, "on") == 0 ? 0x02 : 0x01);
  }

  // 添加更多客厅设备处理逻辑

  // 卧室设备处理
  // LOG("---Check bed chuanglian status: %s\r\n", GolDeviceStatus.bed_chuanglian);
  if (strlen(GolDeviceStatus.bed_chuanglian) > 0)
  {
    handleBedroom(0x11, 0x01, strcmp(GolDeviceStatus.bed_chuanglian, "on") == 0 ? 0x02 : 0x01);
  }
  // LOG("---Check bed light status: %s\r\n", GolDeviceStatus.bed_light);
  if (strlen(GolDeviceStatus.bed_light) > 0)
  {
    handleBedroom(0x21, 0x01, strcmp(GolDeviceStatus.bed_light, "on") == 0 ? 0x02 : 0x01);
  }
  // LOG("---Check bed kongtiao status: %s\r\n", GolDeviceStatus.bed_kongtiao);
  if (strlen(GolDeviceStatus.bed_kongtiao) > 0)
  {
    handleBedroom(0x31, 0x01, strcmp(GolDeviceStatus.bed_kongtiao, "on") == 0 ? 0x02 : 0x01);
  }
  // 添加更多卧室设备处理逻辑

  // 厨房设备处理
  // LOG("---Check kitchen youyan status: %s\r\n", GolDeviceStatus.kitchen_youyan);
  if (strlen(GolDeviceStatus.kitchen_youyan) > 0)
  {
    handleKitchen(0x11, 0x01, strcmp(GolDeviceStatus.kitchen_youyan, "on") == 0 ? 0x02 : 0x01);
  }
  // LOG("---Check kitchen light status: %s\r\n", GolDeviceStatus.kitchen_light);
  if (strlen(GolDeviceStatus.kitchen_light) > 0)
  {
    handleKitchen(0x21, 0x01, strcmp(GolDeviceStatus.kitchen_light, "on") == 0 ? 0x02 : 0x01);
  }

  // 添加更多厨房设备处理逻辑

  // 浴室设备处理
  // LOG("---Check bath yuba status: %s\r\n", GolDeviceStatus.bath_yuba);
  if (strlen(GolDeviceStatus.bath_yuba) > 0)
  {
    handleBathroom(0x11, 0x01, strcmp(GolDeviceStatus.bath_yuba, "on") == 0 ? 0x02 : 0x01);
  }
  // LOG("---Check bath reshui status: %s\r\n", GolDeviceStatus.bath_reshui);
  if (strlen(GolDeviceStatus.bath_reshui) > 0)
  {
    handleBathroom(0x21, 0x01, strcmp(GolDeviceStatus.bath_reshui, "on") == 0 ? 0x02 : 0x01);
  }
  // LOG("---Check bath fan status: %s\r\n", GolDeviceStatus.bath_fan);
  if (strlen(GolDeviceStatus.bath_fan) > 0)
  {
    handleBathroom(0x31, 0x01, strcmp(GolDeviceStatus.bath_fan, "on") == 0 ? 0x02 : 0x01);
  }
  // LOG("---Check bath chuanglian status: %s\r\n", GolDeviceStatus.bath_chuanglian);
  if (strlen(GolDeviceStatus.bath_chuanglian) > 0)
  {
    handleBathroom(0x41, 0x01, strcmp(GolDeviceStatus.bath_chuanglian, "on") == 0 ? 0x02 : 0x01);
  }
  // LOG("---Check bath light status: %s\r\n", GolDeviceStatus.bath_light);
  if (strlen(GolDeviceStatus.bath_light) > 0)
  {
    handleBathroom(0x61, 0x01, strcmp(GolDeviceStatus.bath_light, "on") == 0 ? 0x02 : 0x01);
  }

  // LOG("---Check chuanglian_time status: %s\r\n", GolDeviceStatus.chuanglian_time);
  if (strlen(GolDeviceStatus.chuanglian_time) > 0)
  {
    if (strcmp(GolDeviceStatus.chuanglian_time, "1_off") == 0)
    {
      LOG("chuanglian_time set 1_off\r\n");
    }
    else if (strcmp(GolDeviceStatus.chuanglian_time, "1_on") == 0)
    {
      LOG("chuanglian_time set 1_on\r\n");
      handleBathroom(0x41, 0x03, 0x02);
    }
    else if (strcmp(GolDeviceStatus.chuanglian_time, "2_off") == 0)
    {
      LOG("chuanglian_time set 2_off\r\n");
    }
    else if (strcmp(GolDeviceStatus.chuanglian_time, "2_on") == 0)
    {
      LOG("chuanglian_time set 2_on\r\n");
      handleBathroom(0x41, 0x03, 0x01);
    }
  }
  // if (strlen(GolDeviceStatus.bath_window) > 0) {
  //     handleBathroom(0x41, 0x01, strcmp(GolDeviceStatus.bath_window, "open")? 0x02 : 0x01);
  // }
  // 添加更多浴室设备处理逻辑

}
