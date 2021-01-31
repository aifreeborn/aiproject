#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aii2c.h"
#include "aiap3216c.h"

/*
********************************************************************************
*                          Private macro
********************************************************************************
*/
// AP3216C����IIC��ַ(������һλ)��������R/Wλ
#define    AI_AP3216C_ADDR        0x3C

/*
********************************************************************************
*    Function: ai_ap3216c_init
* Description: ��ʼ��AP3216C
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_ap3216c_init(void)
{
    u8 tmp = 0;
    
    ai_i2c_init();
    ai_ap3216c_write_byte(0x00, 0x04);    // ��λAP3216C
    ai_delay_ms(50);                      // AP33216C��λ����10ms
    ai_ap3216c_write_byte(0x00, 0x03);    // ����ALS��PS+IR
    ai_ap3216c_read_byte(0x00, &tmp);
    if (tmp == 0x03)
        return 0;
    return -1;
}

/*
********************************************************************************
*    Function: ai_ap3216c_read_byte
* Description: IIC��һ���ֽ�
*       Input:  reg - �Ĵ�����ַ
*      Output: data - ָ�����ͣ�ָ���ȡ��һ�ֽ�����
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_ap3216c_read_byte(u8 reg, u8 *data)
{
    int ret = 0;
    
    ai_i2c_start();
    ai_i2c_send_byte(AI_AP3216C_ADDR | 0x00);
    ret = ai_i2c_wait_ack();
    if (ret < 0) {
        ai_i2c_stop();
        return -1;
    }
    ai_i2c_send_byte(reg);
    ret = ai_i2c_wait_ack();
    if (ret < 0) {
        ai_i2c_stop();
        return -1;
    }
    ai_i2c_start();
    ai_i2c_send_byte(AI_AP3216C_ADDR | 0x01);
    ret = ai_i2c_wait_ack();
    if (ret < 0) {
        ai_i2c_stop();
        return -1;
    }
    
    *data = ai_i2c_read_byte(0);
    ai_i2c_stop();
    return 0;
}

/*
********************************************************************************
*    Function: ai_ap3216c_write_byte
* Description: IICдһ���ֽ�
*       Input:  reg - �Ĵ�����ַ
*              data - Ҫд�������
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_ap3216c_write_byte(u8 reg, u8 data)
{
    int ret = 0;
    
    ai_i2c_start();
    ai_i2c_send_byte(AI_AP3216C_ADDR | 0x00);
    ret = ai_i2c_wait_ack();
    if (ret < 0) {
        ai_i2c_stop();
        return -1;
    }
    ai_i2c_send_byte(reg);
    ret = ai_i2c_wait_ack();
    if (ret < 0) {
        ai_i2c_stop();
        return -1;
    }
    ai_i2c_send_byte(data);
    ret = ai_i2c_wait_ack();
    ai_i2c_stop();
    return ret;
}

/*
********************************************************************************
*    Function: ai_ap3216c_read_data
* Description: ��ȡAP3216C������,��ȡԭʼ���ݣ�����ALS,PS��IR
*       Input: ir, ps, als - ��ȡ���ݱ���ĵ�ַ
*      Output: ir, ps, als - ��ȡ�����ݱ���ĵ�ַ
*      Return: void
*      Others: ע�⣡���ͬʱ��ALS,IR+PS�Ļ��������ݶ�ȡ��ʱ����Ҫ����112.5ms
*              ALS:������ǿ��
*               PS:�ӽ�����
*               IR:������ǿ��
********************************************************************************
*/
void ai_ap3216c_read_data(u16 *ir, u16 *ps, u16 *als)
{
    u8 i = 0;
    u8 buf[6] = {0};
    
    // ѭ����ȡ���д���������
    for (i = 0; i < 6; i++) {
        ai_ap3216c_read_byte(0x0a + i, &buf[i]);
    }
    
    if (buf[0] & 0x80)
        *ir = 0;        // IR_OFλΪ1,��������Ч
    else
        *ir = ((u16)buf[1] << 2) | (buf[0] & 0x03);
    *als = ((u16)buf[3] << 8) | buf[2];
    if (buf[4] & 0x40)
        *ps = 0;
    else
        *ps = (((u16)buf[5] & 0x3f) << 4) | (buf[4] & 0x0f);
}
