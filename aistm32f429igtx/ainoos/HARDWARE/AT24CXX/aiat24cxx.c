#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aii2c.h"
#include "aiat24cxx.h"

/*
********************************************************************************
*    Function: ai_at24cxx_init
* Description: at24cxx初始化
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_at24cxx_init(void)
{
    ai_i2c_init();
}

/*
********************************************************************************
*    Function: ai_at24cxx_check
* Description: 检查AT24CXX是否正常,这里用了24XX的最后一个地址(255)来存储标志字.
*              如果用其他24C系列,这个地址要修改
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_at24cxx_check(void)
{
    u8 tmp = 0;
    
    tmp = ai_at24cxx_read_one_byte(0xff);
    if (tmp == 0x55) {
        return 0;
    } else {     // 排除第一次初始化的情况
        ai_at24cxx_write_one_byte(255, 0x55);
        tmp = ai_at24cxx_read_one_byte(255);
        if (tmp == 0x55)
            return 0;
    }
    return -1;
}

/*
********************************************************************************
*    Function: ai_at24cxx_read_one_byte
* Description: 在AT24CXX指定地址读出一个数据
*       Input: addr - 开始读数据的地址
*      Output: None
*      Return: 读到的数据
*      Others: None
********************************************************************************
*/
u8 ai_at24cxx_read_one_byte(u16 addr)
{
    u8 tmp = 0;
    
    ai_i2c_start();
    if (AI_EE_TYPE > AI_AT24C16) {
        ai_i2c_send_byte(0xa0);
        ai_i2c_wait_ack();
        ai_i2c_send_byte(addr >> 8);    // 发送高地址
    } else {
        ai_i2c_send_byte(0xa0 + ((addr / 256) << 1)); //发送器件地址0XA0,写数据
    }
    ai_i2c_wait_ack();
    ai_i2c_send_byte(addr & 0xff);
    ai_i2c_wait_ack();
    ai_i2c_start();
    ai_i2c_send_byte(0xa1);
    ai_i2c_wait_ack();
    tmp = ai_i2c_read_byte(0);
    ai_i2c_stop();
    
    return tmp;
}

/*
********************************************************************************
*    Function: ai_at24cxx_write_one_byte
* Description: 在AT24CXX指定地址写入一个数据
*       Input: addr - 写入数据的目的地址
*              data - 待写入的数据 
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_at24cxx_write_one_byte(u16 addr, u8 data)
{
    ai_i2c_start();
    if (AI_EE_TYPE > AI_AT24C16) {
        ai_i2c_send_byte(0xa0);
        ai_i2c_wait_ack();
        ai_i2c_send_byte(addr >> 8);    // 发送高地址
    } else {
        ai_i2c_send_byte(0xa0 + ((addr / 256) << 1)); //发送器件地址0XA0,写数据
    }
    ai_i2c_wait_ack();
    ai_i2c_send_byte(addr & 0xff);
    ai_i2c_wait_ack();
    ai_i2c_send_byte(data);
    ai_i2c_wait_ack();
    ai_i2c_stop();
    ai_delay_ms(10);
}

/*
********************************************************************************
*    Function: ai_at24cxx_write_one_byte
* Description: 在AT24CXX里面的指定地址开始写入长度为n的数据
*       Input: addr - 写入的起始地址
*              data - 待写入的数据，只能是16bit或者32bit 
*                 n - 待写入的数据长度，取值：2，4
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_at24cxx_write_nbytes(u16 addr, u32 data, u8 n)
{
    u8 i = 0;
    
    if (n != 2 || n != 4)
        return;
    
    for (i = 0; i < n; i++) {
        ai_at24cxx_write_one_byte(addr + i, (addr >> (8 * i)) & 0xff);
    }
}

/*
********************************************************************************
*    Function: ai_at24cxx_write_one_byte
* Description: 在AT24CXX里面的指定地址开始读出长度为n的数据
*       Input: addr - 开始读的地址
*                 n - 待读的数据长度，取值：2，4
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
u32 ai_at24cx_read_nbytes(u16 addr, u8 n)
{
    u8 i = 0;
    u32 tmp = 0;
    
    if (n != 2 || n != 4)
        return 0;
    
    for (i = 0; i < n; i++) {
        tmp <<= 8;
        tmp += ai_at24cxx_read_one_byte(addr + n - i - 1);
    }
    
    return tmp;
}

/*
********************************************************************************
*    Function: ai_at24cxx_read
* Description: 在AT24CXX里面的指定地址开始读出指定个数的数据
*       Input: addr - 开始读的地址,对24c02为0~255
*              pubf - 数据数组首地址
*               len - 要读出数据的长度
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_at24cxx_read(u16 addr, u8 *pbuf, u16 len)
{
    while (len--) {
        *pbuf++ = ai_at24cxx_read_one_byte(addr++);
    }
}

/*
********************************************************************************
*    Function: ai_at24cxx_write_one_byte
* Description: 在AT24CXX里面的指定地址开始写入指定个数的数据
*       Input: addr - 开始写的地址,对24c02为0~255
*              pubf - 数据数组首地址
*               len - 要写入的数据的长度
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_at24cxx_write(u16 addr, u8 *pbuf, u16 len)
{
    while (len--) {
        ai_at24cxx_write_one_byte(addr, *pbuf);
        addr++;
        pbuf++;
    }
}
