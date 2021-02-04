#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aidht11.h"

/*
********************************************************************************
*                          PRIVATE DEFINE/FUNCTIONS
********************************************************************************
*/
// 定义数据I/O线
#define    AI_DHT11_DOUT        PB_OUT(12)
#define    AI_DHT11_DIN         PB_IN(12)

#define    ai_dht11_set_in() \
    do { \
        GPIOB->MODER &= ~(0x3 << (2 * 12)); \
        GPIOB->MODER |= 0x0 << (2 * 12); \
    } while (0)
    
#define    ai_dht11_set_out() \
    do { \
        GPIOB->MODER &= ~(0x3 << (2 * 12)); \
        GPIOB->MODER |= 0x1 << (2 * 12); \
    } while (0)

/*
********************************************************************************
*    Function: ai_dht11_reset
* Description: 复位DHT11
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
static void ai_dht11_reset(void)
{
    ai_dht11_set_out();
    AI_DHT11_DOUT = 0;
    ai_delay_ms(20);      // 拉低至少18ms
    AI_DHT11_DOUT = 1;    // 主机拉高
    ai_delay_us(30);      // 主机拉高20~40us
}

/*
********************************************************************************
*    Function: ai_dht11_check
* Description: 等待DHT11的回应
*       Input: void
*      Output: None
*      Return: -1 -> 未检测到DHT11的存在
*               0 -> 检测到DHT11的存在
*      Others: None
********************************************************************************
*/
static int ai_dht11_check(void)
{
    u8 timeout = 0;
    
    ai_dht11_set_in();
    while (AI_DHT11_DIN && (timeout < 100)) {    // DHT11会拉低40~50us
        timeout++;
        ai_delay_us(1);
    }
    if (timeout >= 100)
        return -1;
    else
        timeout = 0;
    
    while (!AI_DHT11_DIN && timeout < 100) {    // DHT11拉低后会再次拉高40~50us
        timeout++;
        ai_delay_us(1);
    }
    if (timeout >= 100)
        return -1;
    return 0;
}

/*
********************************************************************************
*    Function: ai_dht11_init
* Description: 初始化DHT11的IO口,同时检测DHT11的存在
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: DHT11和 MCU的一次通信最大为 3ms 左右，
*              建议主机连续读取时间间隔不要小于 100ms
*              读取DHT11的时候,需要先对 PCF8574T 进行一次读取操作，
*               以释放 IIC_INT 引脚
********************************************************************************
*/
int ai_dht11_init(void)
{
    RCC->AHB1ENR |= 0x1 << 1;
    ai_gpio_set(GPIOB, PIN12, GPIO_MODE_OUT,
                GPIO_OTYPE_PP, GPIO_SPEED_50M, GPIO_PUPD_PU);
    ai_dht11_reset();
    return ai_dht11_check();
}

/*
********************************************************************************
*    Function: ai_dht11_read_bit
* Description: 从DHT11读取一个位
*       Input: void
*      Output: None
*      Return: 返回读取的电平值1/0
*      Others: None
********************************************************************************
*/
static u8 ai_dht11_read_bit(void)
{
    u8 timeout = 0;
    
    while (AI_DHT11_DIN && timeout < 120) {     // 等待变为低电平
        timeout++;
        ai_delay_us(1);
    }
    timeout = 0;
    while (!AI_DHT11_DIN && timeout < 120) {    // 等待变高电平
        timeout++;
        ai_delay_us(1);
    }
    ai_delay_us(30);
    if (AI_DHT11_DIN)
        return 1;
    else
        return 0;
}

/*
********************************************************************************
*    Function: ai_dht11_read_byte
* Description: 从DHT11读取一个字节
*       Input: void
*      Output: None
*      Return: 返回读取的字节数据
*      Others: DHT11的数据是高位先出
********************************************************************************
*/
static u8 ai_dht11_read_byte(void)
{
    u8 i, data = 0;
    
    for (i = 0; i < 8; i++) {
        data <<= 1;
        data |= ai_dht11_read_bit();
    }
    
    return data;
}

/*
********************************************************************************
*    Function: ai_dht11_read_data
* Description: 从DHT11读取一次数据
*       Input: None
*      Output: temperature - 温度值
*                 humidity - 湿度值
*              高八位是整数部分，低八位是小数部分
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: DHT11的数据是高位先出
********************************************************************************
*/
int ai_dht11_read_data(u16 *temperature, u16 *humidity)
{
    u8 buf[5] = {0};
    u8 i;
    
    ai_dht11_reset();
    if (ai_dht11_check() == 0) {
        for (i = 0; i < 5; i++) {
            buf[i] = ai_dht11_read_byte();
        }
        
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
            *temperature = ((u16)buf[2] << 8) | buf[3];
            *humidity = ((u16)buf[0] << 8) | buf[1];
        }
    } else {
        return -1;
    }
    
    return 0;
}
