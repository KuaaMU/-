/**
 * @file    bmp180.c
 * @brief
 *
 * @version 2024-08-10, V1.0, yanf, 厦门芯力量
 */

#include "gd32f4xx.h"
#include "stdio.h"
#include "math.h"
#include "systick.h"
#include "bmp180.h"

typedef struct _BMP180_STRUCT{
    short AC1;
    short AC2;
    short AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    short B1;
    short B2;
    short MB;
    short MC;
    short MD;
}_BMP180_PARAM_;

_BMP180_PARAM_ param={0};

long B5 = 0;


/**
 * @brief   BMP180 SCL、SDA 引脚配置及初始化
 *
 */
void bmp180_init(void)
{
    rcu_periph_clock_enable(BMP180_SCL_RCU);
    rcu_periph_clock_enable(BMP180_SDA_RCU);

    gpio_mode_set(BMP180_SCL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, BMP180_SCL_PIN);
    gpio_output_options_set(BMP180_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, BMP180_SCL_PIN);

    gpio_mode_set(BMP180_SDA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, BMP180_SDA_PIN);
    gpio_output_options_set(BMP180_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, BMP180_SDA_PIN);
}


/**
 * @brief   BMP180 IIC 发送开始信号
 *
 */
void bmp180_i2c_start(void)
{
    BMP180_SDA_OUT();

    BMP180_SDA(1);
    delay_1us(5);
    BMP180_SCL(1);
    delay_1us(5);

    BMP180_SDA(0);
    delay_1us(5);
    BMP180_SCL(0);
    delay_1us(5);

}


/**
 * @brief   BMP180 IIC 发送停止信号
 *
 */
void bmp180_i2c_stop(void)
{
    BMP180_SDA_OUT();
    BMP180_SCL(0);
    BMP180_SDA(0);

    BMP180_SCL(1);
    delay_1us(5);
    BMP180_SDA(1);
    delay_1us(5);

}


/**
 * @brief   主机发送应答或者非应答信号
 *
 * @param   ack: 0:发送应答, 1:发送非应答
 */
void bmp180_i2c_send_ack(uint8_t ack)
{
    BMP180_SDA_OUT();
    BMP180_SCL(0);
    BMP180_SDA(0);
    delay_1us(5);
    if(!ack) BMP180_SDA(0);
    else     BMP180_SDA(1);
    BMP180_SCL(1);
    delay_1us(5);
    BMP180_SCL(0);
    BMP180_SDA(1);
}


/**
 * @brief   等待从机应答
 *
 * @return uint8_t 0:有应答, 1:超时无应答
 */
uint8_t bmp180_i2c_wait_ack(void)
{

    char ack = 0;
    uint8_t ack_flag = 10;
    BMP180_SCL(0);
    BMP180_SDA(1);
    BMP180_SDA_IN();
    delay_1us(5);
    BMP180_SCL(1);
    delay_1us(5);

    while( (BMP180_SDA_GET()==1) && ( ack_flag ) )
    {
        ack_flag--;
        delay_1us(5);
    }

    if( ack_flag <= 0 )
    {
        bmp180_i2c_stop();
        return 1;
    }
    else
    {
        BMP180_SCL(0);
        BMP180_SDA_OUT();
    }
    return ack;
}


/**
 * @brief   写入一个字节
 *
 * @param   dat: 要写入的数据
 */
void bmp180_send_byte(uint8_t dat)
{
    int i = 0;
    BMP180_SDA_OUT();
    BMP180_SCL(0);//拉低时钟开始数据传输

    for( i = 0; i < 8; i++ )
    {
        BMP180_SDA( (dat & 0x80) >> 7 );
        __nop();
        BMP180_SCL(1);
        delay_1us(5);
        BMP180_SCL(0);
        delay_1us(5);
        dat<<=1;
    }
}


/**
 * @brief   读一个字节
 *
 * @return uint8_t 读到的数据
 */
uint8_t bmp180_read_byte(void)
{
    uint8_t i, receive=0;
    BMP180_SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
    {
        BMP180_SCL(0);
        delay_1us(5);
        BMP180_SCL(1);
        delay_1us(5);
        receive<<=1;
        if( BMP180_SDA_GET() )
        {
            receive|=1;
        }
        delay_1us(5);
    }
    BMP180_SCL(0);
  return receive;
}


/**
 * @brief   向 BMP180 写入一个字节数据
 *
 * @remark  regaddr=0xf4, cmd=0X2E
 * @param   regaddr: regaddr 寄存器地址
 * @param       cmd: 写入的数据
 */
void bmp180_write_cmd(uint8_t regaddr, uint8_t cmd)
{
    bmp180_i2c_start();//起始信号
    bmp180_send_byte(0XEE);//器件地址+写
    if( bmp180_i2c_wait_ack() == 1 ) printf("Write_Cmd NACK -1\r\n");

    bmp180_send_byte(regaddr);
    if( bmp180_i2c_wait_ack() == 1 ) printf("Write_Cmd NACK -2\r\n");

    bmp180_send_byte(cmd);
    if( bmp180_i2c_wait_ack() == 1 ) printf("Write_Cmd NACK -3\r\n");

    bmp180_i2c_stop();
}


/**
 * @brief   读取 BMP180 数据
 *
 * @param   regaddr: 读取的地址
 * @param       len: 读取的长度
 * @return uint16_t 读取到的数据
 */
uint16_t bmp180_read16(uint16_t regaddr, uint8_t len)
{
    int timeout = 0;
    uint16_t dat[3] = {0};
    int i =0;
    for( i = 0; i < len; i++ )
    {
        bmp180_i2c_start();//起始信号
        bmp180_send_byte(0XEE);//器件地址+写
        if( bmp180_i2c_wait_ack() == 1 ) printf("Read_Reg NACK -1\r\n");
        bmp180_send_byte(regaddr+i);
        if( bmp180_i2c_wait_ack() == 1 ) printf("Read_Reg NACK -2\r\n");

        do{
            timeout++;
            delay_1ms(1);
            bmp180_i2c_start();//起始信号
            bmp180_send_byte(0XEF);//器件地址+读
        }while(bmp180_i2c_wait_ack() == 1 && (timeout < 5) );

        dat[i] = bmp180_read_byte();
        bmp180_i2c_send_ack(1);
        bmp180_i2c_stop();
        delay_1ms(1);
    }
    if( len == 2 ) return ( (dat[0]<<8) | dat[1] );
    if( len == 3 ) return (( (dat[0]<<16) | (dat[1]<<8) | (dat[2]) ) >> 8);
    return 0;
}


/**
 * @brief   读取温度单位℃
 *
 * @return float 温度
 */
float bmp180_get_temperature(void)
{
    long UT = 0;
    long X1 = 0, X2 = 0;

    bmp180_write_cmd(0XF4, 0X2E);
    delay_1ms(6);
    UT = bmp180_read16(0xf6, 2);

    X1 = ((long)UT - param.AC6) * param.AC5 / 32768.0;
    X2 = ((long)param.MC * 2048.0) / ( X1 + param.MD );
    B5 = X1 + X2;
    return ((B5+8)/16.0)*0.1f;
}


/**
 * @brief   读取气压, 单位Pa
 *
 * @return float 当前气压，单位Pa
 */
float bmp180_get_pressure(void)
{
    long UP = 0;
    uint8_t oss = 0;
    long X1 = 0, X2 = 0;

    bmp180_get_temperature();

    bmp180_write_cmd(0XF4, (0X34+(oss<<6)));
    delay_1ms(10);
    UP = bmp180_read16(0xf6, 3);


    int32_t B6 = B5 - 4000;

    X1 = (B6 * B6 >> 12) * param.B2 >> 11;

    X2 = param.AC2 * B6 >> 11;

    int32_t X3 = X1 + X2;

    int32_t B3 = (((param.AC1 << 2) + X3) + 2) >> 2;

    X1 = param.AC3 * B6 >> 13;

    X2 = (B6 * B6 >> 12) * param.B1 >> 16;

    X3 = (X1 + X2 + 2) >> 2;

    uint32_t B4 = param.AC4 * (uint32_t)(X3 + 32768) >> 15;

    uint32_t B7 = ((uint32_t)UP - B3) * 50000;

    int32_t p;
    if(B7 < 0x80000000){
            p = (B7 << 1) / B4;
    }else{
            p = B7/B4 << 1;
    }

    X1 = (p >> 8) * (p >> 8);

    X1 = (X1 * 3038) >> 16;

    X2 = (-7375 * p) >> 16;

    p = p + ((X1 + X2 + 3791) >> 4);
     return p;

}


/**
 * @brief   计算海拔高度
 *
 * @param   p: 当前气压
 * @return float 海拔高度
 */
float bmp180_get_altitude(float p)
{
//#define PRESSURE_OF_SEA    101325.0f // 参考海平面压强
    float altitude = 0;
    altitude = 44330*(1 - pow((p)/ 101325.0f, 1.0f / 5.255f));
//    printf("altitude = %.2f\r\n", altitude);
    return altitude;
}


/**
 * @brief   获取出厂校准值
 *
 */
void bmp180_calibrate(void)
{
    param.AC1 = bmp180_read16(0xaa, 2);
    param.AC2 = bmp180_read16(0xac, 2);
    param.AC3 = bmp180_read16(0xae, 2);
    param.AC4 = bmp180_read16(0xb0, 2);
    param.AC5 = bmp180_read16(0xb2, 2);
    param.AC6 = bmp180_read16(0xb4, 2);
    param.B1  = bmp180_read16(0xb6, 2);
    param.B2  = bmp180_read16(0xb8, 2);
    param.MB  = bmp180_read16(0xba, 2);
    param.MC  = bmp180_read16(0xbc, 2);
    param.MD  = bmp180_read16(0xbe, 2);
}
