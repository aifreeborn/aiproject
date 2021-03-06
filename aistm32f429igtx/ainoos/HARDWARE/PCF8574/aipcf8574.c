#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aii2c.h"
#include "aipcf8574.h"

/*
********************************************************************************
*                          Private macro
********************************************************************************
*/
// PCF8574地址，左移了一位，包含了R/W位
#define    AI_PCF8574_ADDR        0x40


/*
********************************************************************************
*    Function: ai_pcf8574_init
* Description: PCF8574初始化
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_pcf8574_init(void)
{
    int tmp = 0;
    
    RCC->AHB1ENR |= 0x1 << 1;
    ai_gpio_set(GPIOB, PIN12, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PU);
    ai_i2c_init();
    // 检查PCF8574是否在位
    ai_i2c_start();
    ai_i2c_send_byte(AI_PCF8574_ADDR);
    tmp = ai_i2c_wait_ack();
    ai_i2c_stop();
    if (tmp == 0)
        ai_pcf8574_write_byte(0xff);    // 默认情况下所有IO输出高电平
    return tmp;
}

/*
********************************************************************************
*    Function: ai_pcf8574_read_byte
* Description: 读取PCF8574的8位IO值
*       Input: void
*      Output: None
*      Return: A non-negative integer indicating the data actually read, 
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_pcf8574_read_byte(void)
{
    int tmp = 0;
    
    ai_i2c_start();
    ai_i2c_send_byte(AI_PCF8574_ADDR | 0x01);
    tmp = ai_i2c_wait_ack();
    if (tmp < 0) {
        ai_i2c_stop();
        return -1;
    }
    tmp = ai_i2c_read_byte(0);
    ai_i2c_stop();
    return tmp;
}

/*
********************************************************************************
*    Function: ai_pcf8574_write_byte
* Description: 向PCF8574写入8位IO值
*       Input: data - 要写入的数据
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_pcf8574_write_byte(u8 data)
{
    int ret = 0;
    
    ai_i2c_start();
    ai_i2c_send_byte(AI_PCF8574_ADDR | 0x00);
    ret = ai_i2c_wait_ack();
    if (ret < 0) {
        ai_i2c_stop();
        return -1;
    }
    ai_i2c_send_byte(data);
    ret = ai_i2c_wait_ack();
    ai_i2c_stop();
    ai_delay_ms(10);
    
    return ret;
}

/*
********************************************************************************
*    Function: ai_pcf8574_read_bit
* Description: 读取PCF8574的某个IO的值
*       Input: bitn - 要读取的IO编号,0~7
*      Output: None
*      Return: A non-negative integer indicating the data actually read.
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_pcf8574_read_bit(u8 bitn)
{
    int data = 0;
    
    data = ai_pcf8574_read_byte();
    if (data < 0)
        return -1;
    
    if (data & (0x1 << bitn))
        return 1;
    else
        return 0;
}

/*
********************************************************************************
*    Function: ai_pcf8574_write_bit
* Description: 设置PCF8574某个IO的高低电平
*       Input: bitn - 要设置的IO编号,0~7
*              stat - IO状态，取值0或者1
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_pcf8574_write_bit(u8 bitn, u8 stat)
{
    int data = 0;
    
    data = ai_pcf8574_read_byte();
    if (data < 0)
        return -1;
    if (stat)
        data |= 0x1 << bitn;
    else
        data &= ~(0x1 << bitn);
    return ai_pcf8574_write_byte(data);
}

