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
// PCF8574��ַ��������һλ��������R/Wλ
#define    AI_PCF8574_ADDR        0x40


/*
********************************************************************************
*    Function: ai_pcf8574_init
* Description: PCF8574��ʼ��
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
    // ���PCF8574�Ƿ���λ
    ai_i2c_start();
    ai_i2c_send_byte(AI_PCF8574_ADDR);
    tmp = ai_i2c_wait_ack();
    ai_i2c_stop();
    if (tmp == 0)
        ai_pcf8574_write_byte(0xff);    // Ĭ�����������IO����ߵ�ƽ
    return tmp;
}

/*
********************************************************************************
*    Function: ai_pcf8574_read_byte
* Description: ��ȡPCF8574��8λIOֵ
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
* Description: ��PCF8574д��8λIOֵ
*       Input: data - Ҫд�������
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
* Description: ��ȡPCF8574��ĳ��IO��ֵ
*       Input: bitn - Ҫ��ȡ��IO���,0~7
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
* Description: ����PCF8574ĳ��IO�ĸߵ͵�ƽ
*       Input: bitn - Ҫ���õ�IO���,0~7
*              stat - IO״̬��ȡֵ0����1
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

