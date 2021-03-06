#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aii2c.h"
#include "aiat24cxx.h"

/*
********************************************************************************
*    Function: ai_at24cxx_init
* Description: at24cxx��ʼ��
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
* Description: ���AT24CXX�Ƿ�����,��������24XX�����һ����ַ(255)���洢��־��.
*              ���������24Cϵ��,�����ַҪ�޸�
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
    } else {     // �ų���һ�γ�ʼ�������
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
* Description: ��AT24CXXָ����ַ����һ������
*       Input: addr - ��ʼ�����ݵĵ�ַ
*      Output: None
*      Return: ����������
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
        ai_i2c_send_byte(addr >> 8);    // ���͸ߵ�ַ
    } else {
        ai_i2c_send_byte(0xa0 + ((addr / 256) << 1)); //����������ַ0XA0,д����
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
* Description: ��AT24CXXָ����ַд��һ������
*       Input: addr - д�����ݵ�Ŀ�ĵ�ַ
*              data - ��д������� 
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
        ai_i2c_send_byte(addr >> 8);    // ���͸ߵ�ַ
    } else {
        ai_i2c_send_byte(0xa0 + ((addr / 256) << 1)); //����������ַ0XA0,д����
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
* Description: ��AT24CXX�����ָ����ַ��ʼд�볤��Ϊn������
*       Input: addr - д�����ʼ��ַ
*              data - ��д������ݣ�ֻ����16bit����32bit 
*                 n - ��д������ݳ��ȣ�ȡֵ��2��4
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
* Description: ��AT24CXX�����ָ����ַ��ʼ��������Ϊn������
*       Input: addr - ��ʼ���ĵ�ַ
*                 n - ���������ݳ��ȣ�ȡֵ��2��4
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
* Description: ��AT24CXX�����ָ����ַ��ʼ����ָ������������
*       Input: addr - ��ʼ���ĵ�ַ,��24c02Ϊ0~255
*              pubf - ���������׵�ַ
*               len - Ҫ�������ݵĳ���
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
* Description: ��AT24CXX�����ָ����ַ��ʼд��ָ������������
*       Input: addr - ��ʼд�ĵ�ַ,��24c02Ϊ0~255
*              pubf - ���������׵�ַ
*               len - Ҫд������ݵĳ���
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
