/**
 * @file    dht11.c
 * @brief   DHT11 数字温湿度复合传感器
 * @details 测量范围: 温度: 0 - 50 °c , 湿度: 20 - 90 %RH
 *          测量精度: 温度:    ± 2 °c , 湿度:     ± 5 %RH
 *
 * @version 2024-07-24, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "dht11.h"
#include "systick.h"


/**
 * @brief   DHT11 引脚配置及初始化
 *
 */
void dht11_init(void)
{
    rcu_periph_clock_enable(DHT11_RCU);

    gpio_mode_set(DHT11_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, DHT11_PIN);
    gpio_output_options_set(DHT11_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, DHT11_PIN);
}


/**
 * @brief   发送开始信号
 * @remark  总线空闲状态为高电平, 主机把总线拉低等待 DHT11 响应, 主机把总线拉低必须大于 18 毫秒, 保证 DHT11 能检测到起始信号。
 *          DHT11 接收到主机的开始信号后, 等待主机开始信号结束, 然后发送 80us 低电平响应信号。
 *          主机发送开始信号结束, 延时等待 20-40us 后, 读取 DHT11 的响应信号, 主机发送开始信号后, 可以切换到输入模式,
 *          或者输出高电平均可, 总线由上拉电阻拉高。
 */
void dht11_start(void)
{
    DHT11_OUTPUT();                                         // 转为输出模式

    DHT11_GPIO(RESET);                                      // 将数据线拉低至少18ms
    delay_1ms(19);

    DHT11_GPIO(SET);                                        // 释放数据线，等待DHT11响应
    delay_1us(20);                                          // 等待 DHT11 拉低数据线作为响应
}


/**
 * @brief   在开始信号发送完后，等待和检测 DHT11 传感器的响应信号
 *
 * @return uint8_t 收到正确响应后返回 1，超时返回 0
 */
uint8_t dht11_check_response(void)
{
    DHT11_INPUT();                                          // 转为输入模式

    uint32_t timeout = 5000;

    while (DHT11_GPIO_GET() == RESET && --timeout > 0);     // 等待高电平的到来
    delay_1us(40);                                          // 等待高电平结束

    while (DHT11_GPIO_GET() == SET && --timeout > 0);

    return timeout > 0;                                     // 返回是否收到正确响应
}


/**
 * @brief   从 DHT11 读取一个字节数据
 *
 * @return uint8_t 返回读取到的一个字节数据
 */
uint8_t dht11_read_byte(void)
{
    uint8_t data = 0;

    // 每一bit数据都以50us低电平时隙开始，跳转高电平，高电平的长短决定了数据位是0还是1
    // 26~28us表示0，70us表示1
    for (uint8_t i = 0; i < 8; i++) {
        while (DHT11_GPIO_GET() == RESET);                  // 等待低电平时隙过去

        delay_1us(30);

        if (DHT11_GPIO_GET()) {                             // 等待了30us后如果还没跳转低电平，则收到的数据位为1
            data |= 1 << (7 - i);
        }

        while (DHT11_GPIO_GET() == SET);                    // 等待高电平过去，开始下一bit
    }

    return data;
}


/**
 * @brief   读取 DHT11 传感器检测到的温、湿度值
 *
 * @param   temperature: 环境温度
 * @param      humidity: 相对湿度
 */
void dht11_read_data(float *temperature, float *humidity)
{
    dht11_start();                                          // 发送开始信号

    if (dht11_check_response()) {                           // 检查 DHT11 响应
        uint8_t data[5];

        for(uint8_t i = 0; i < 5; i++) {                    // 读取40位数据（5字节）
            data[i] = dht11_read_byte();
        }

        // 数据传送正确时校验和数据(第5个字节)等于 "8bit 湿度整数数据 + 8bit 湿度小数数据
        // + 8bit 温度整数数据 + 8bit 温度小数数据" 所得结果的末 8 位。
        if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
            // 处理校验失败
        }

        *humidity = data[0] + data[1] * .1f;
        *temperature = data[2] + data[3] * .1f;

    } else {
        // 错误处理
    }
}
