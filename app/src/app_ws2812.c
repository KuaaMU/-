#include "app_ws2812.h"
#include "FreeRTOS.h"
#include "task.h"
#include "execution_task.h"
#include "stdio.h"

#define WS2812_NUM_LEDS 8 // 定义灯珠数量

extern TaskHandle_t xTaskLedHandle;
extern beepData beep;
ws2812Data ws2812={ .status=0x00,
                    .brightness = 50,
                    .red = 0xff,
                    .green = 0xff,
                    .blue = 0xf0};

uint32_t ws2812_colors[WS2812_NUM_LEDS]; // 存储所有灯珠的颜色数据

void app_ws2812Control(uint8_t mode, uint8_t adjust)
{
    switch (mode)
    {
    case 0x01:
        switch (adjust)
        {
        case 0x01:
            // 关灯
            ws2812.status = 0x00;
            break;

        case 0x02:
            // 开灯
            ws2812.status =  0x01;
            app_ws2812_Set_color(&ws2812, 0xff, 0xff, 0xf0);
            app_ws2812_Set_light(&ws2812, 75);
            break;
        case 0x03:
            // 开红灯
            ws2812.status =  0x01;
            app_ws2812_Set_light(&ws2812, 75);
            app_ws2812_Set_color(&ws2812, 0xff, 0x00, 0x00);
            break;
        case 0x04:
            // 开绿灯
            ws2812.status = 0x01;
            app_ws2812_Set_light(&ws2812, 75);
            app_ws2812_Set_color(&ws2812, 0x00, 0xff, 0x00);
            break;
        case 0x05:
            // 开蓝灯
            ws2812.status = 0x01;
            app_ws2812_Set_light(&ws2812, 75);
            app_ws2812_Set_color(&ws2812, 0x00, 0x00, 0xff);
            break;
        case 0x06:
            //开紫灯
            ws2812.status = 0x01;
            app_ws2812_Set_color(&ws2812, 0xff, 0x00, 0xff);
            app_ws2812_Set_light(&ws2812, 75);
            break;
        case 0x07:
            //开黄灯
            ws2812.status = 0x01;
            app_ws2812_Set_light(&ws2812, 75);
            app_ws2812_Set_color(&ws2812, 0xff, 0xff, 0x00);
            break;
        }
        break;

    case 0x02:
        switch (adjust)
        {
        case 0x01:
            // 调暗
            app_ws2812_Set_light(&ws2812, ws2812.brightness - 25);
            break;

        case 0x02:
            // 调亮
            app_ws2812_Set_light(&ws2812, ws2812.brightness + 25);
            break;

        case 0x03:
            // 红色调暗
            app_ws2812_Set_color(&ws2812, ws2812.red - 64, ws2812.green, ws2812.blue);
            break;
        case 0x04:
            // 红色调亮
            app_ws2812_Set_color(&ws2812, ws2812.red + 64, ws2812.green, ws2812.blue);
        case 0x05:
            // 绿色调暗
            app_ws2812_Set_color(&ws2812, ws2812.red, ws2812.green - 64, ws2812.blue);
            break;
        case 0x06:
            // 绿色调亮
            app_ws2812_Set_color(&ws2812, ws2812.red, ws2812.green + 64, ws2812.blue);
        case 0x07:
            // 蓝色调暗
            app_ws2812_Set_color(&ws2812, ws2812.red, ws2812.green, ws2812.blue - 64);
            break;
        case 0x08:
            // 蓝色调亮
            app_ws2812_Set_color(&ws2812, ws2812.red, ws2812.green, ws2812.blue + 64);
        }
        break;
    case 0x03:
        switch (adjust)
        {
        case 0x01:
            ws2812.status = 2;
            beep.status = 0x01;
        }
    }
    xTaskNotifyGive(xTaskLedHandle);
}

void app_ws2812_Set_light(ws2812Data *ws2812handle, int light)
{
    if (light >= 0 && light <= 100)
    {
        ws2812handle->brightness = light;
    }
}

void app_ws2812_Set_color(ws2812Data *ws2812handle, uint8_t red, uint8_t green, uint8_t blue)
{
    if (red <= 0xff)
        ws2812handle->red = red;
    if (green <= 0xff)
        ws2812handle->green = green;
    if (blue <= 0xff)
        ws2812handle->blue = blue;
}
void app_ws2812_set(ws2812Data *ws2812handle)
{
    ws2812Data ws2812_temp;
    ws2812_temp.red = ws2812handle->red;
    ws2812_temp.green = ws2812handle->green;
    ws2812_temp.blue = ws2812handle->blue;
    float percent = ws2812handle->brightness / 100.0;
    ws2812_set_color(ws2812_temp.red * percent, ws2812_temp.green * percent, ws2812_temp.blue * percent);
}

void app_ws2812_set_all(ws2812Data *ws2812handle)
{
    ws2812Data ws2812_temp;
    ws2812_temp.red = ws2812handle->red;
    ws2812_temp.green = ws2812handle->green;
    ws2812_temp.blue = ws2812handle->blue;
    float percent = ws2812handle->brightness / 100.0;
    app_ws2812_set_all_color(ws2812_temp.red * percent, ws2812_temp.green * percent, ws2812_temp.blue * percent);
}

/**
 * @brief   更新所有 WS2812 灯珠的颜色
 *
 */
void app_ws2812_update_all(void)
{
    // // 发送复位信号
    // ws2812_send_reset();

    // 发送所有灯珠的颜色数据
    for (uint8_t i = 0; i < WS2812_NUM_LEDS; i++)
    {
        ws2812_send_one(ws2812_colors[i]);
    }
}

/**
 * @brief   设置所有 WS2812 灯珠的颜色
 *
 * @param     red: 8 位红色值
 * @param   green: 8 位绿色值
 * @param    blue: 8 位蓝色值
 */
void app_ws2812_set_all_color(uint8_t red, uint8_t green, uint8_t blue)
{
    for (uint8_t i = 0; i < WS2812_NUM_LEDS; i++)
    {
        ws2812_colors[i] = (green << 16) | (red << 8) | blue;
    }
    app_ws2812_update_all();
}

/**
 * @brief   HSV到RGB颜色转换
 */
uint32_t app_ws2812_hsv_to_rgb(uint8_t hue, uint8_t sat, uint8_t val)
{
    uint8_t region, remainder, p, q, t;
    uint32_t rgb = 0;

    if (sat == 0)
    { // Acromatic (grey)
        rgb = (val << 16) | (val << 8) | val;
        return rgb;
    }

    region = hue / 43;
    remainder = (hue - (region * 43)) * 6;

    p = (val * (255 - sat)) >> 8;
    q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
    t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;

    if (region == 0)
    {
        rgb = (val << 16) | (t << 8) | p;
    }
    else if (region == 1)
    {
        rgb = (q << 16) | (val << 8) | p;
    }
    else if (region == 2)
    {
        rgb = (p << 16) | (val << 8) | t;
    }
    else if (region == 3)
    {
        rgb = (p << 16) | (q << 8) | val;
    }
    else if (region == 4)
    {
        rgb = (t << 16) | (p << 8) | val;
    }
    else
    {
        rgb = (val << 16) | (p << 8) | q;
    }

    return rgb;
}

//-------------------------------特殊效果-----------------------------------

/**
 * @brief   呼吸灯效果
 */
void app_ws2812_breath(uint8_t red, uint8_t green, uint8_t blue, int duration, int interval)
{
    for (int i = 0; i < duration; i++)
    {
        for (int brightness = 0; brightness <= 100; brightness++)
        {
            app_ws2812_set_all_color(ws2812.red, ws2812.green, ws2812.blue);
            vTaskDelay(pdMS_TO_TICKS(interval)); // 延时5ms
            app_ws2812_Set_light(&ws2812, brightness);
            app_ws2812_set_all(&ws2812);
        }
        for (int brightness = 100; brightness >= 0; brightness--)
        {
            app_ws2812_set_all_color(ws2812.red, ws2812.green, ws2812.blue);
            vTaskDelay(pdMS_TO_TICKS(interval)); // 延时5ms
            app_ws2812_Set_light(&ws2812, brightness);
            app_ws2812_set_all(&ws2812);
        }
    }
}

/**
 * @brief   彩虹渐变效果
 */
void app_ws2812_rainbow_cycle(int duration, int interval)
{
    uint8_t starthue = 0;
    for (int i = 0; i < duration; i++)
    {
        for (int j = 0; j < 256 * WS2812_NUM_LEDS; j++)
        { // 足够多的迭代次数以覆盖整个色相圈
            for (int i = 0; i < WS2812_NUM_LEDS; i++)
            {
                // hue increases by 'j' each time, modulo 256 to keep it within the range
                uint8_t hue = (starthue + j) % 256;
                ws2812_colors[i] = app_ws2812_hsv_to_rgb(hue, 255, 255);
            }
            app_ws2812_update_all();
            vTaskDelay(pdMS_TO_TICKS(interval)); // 延时10ms
            starthue++;
            starthue %= 256; // 防止starthue溢出
        }
    }
}

/**
 * @brief   闪烁效果
 */
void app_ws2812_flash(uint8_t red, uint8_t green, uint8_t blue, int duration, int interval)
{
    for (int i = 0; i < duration; i++)
    {
        app_ws2812_set_all_color(red, green, blue);
        vTaskDelay(pdMS_TO_TICKS(interval));

        app_ws2812_set_all_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(interval));
    }
}
