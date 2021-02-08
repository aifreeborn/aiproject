#include <stdio.h>
#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aii2c.h"
#include "aimpu9250.h"

/*
********************************************************************************
*    Function: ai_mpu9250_init
* Description: ��ʼ��MPU9250
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, negative integer is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_init(void)
{
    u8 ret = 0;
    
    ai_i2c_init();
    // ��λMPU9250
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_PWR_MGMT1, 0x80);
    ai_delay_ms(100);
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_PWR_MGMT1, 0x00);
    
    ai_mpu9250_set_gyro_fsr(3);    // �����Ǵ�����,��2000dps
    ai_mpu9250_set_accel_fsr(0);   // ���ٶȴ�����,��2g
    ai_mpu9250_set_rate(50);       // ���ò�����50Hz
    // �ر������ж�
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_INT_EN, 0x00);
    // I2C��ģʽ�ر�
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_USER_CTRL, 0x00);
    // �ر�FIFO
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_FIFO_EN, 0x00);
    // INT���ŵ͵�ƽ��Ч������bypassģʽ������ֱ�Ӷ�ȡ������
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_INT_PIN_CFG, 0x82);
    ret = ai_mpu9250_read_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_WHO_AM_I);
    if (ret == AI_MPU9250_ID1 || ret == AI_MPU9250_ID2) {
        // ����CLKSEL,PLL X��Ϊ�ο�
        ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_PWR_MGMT1, 0x01);
        // ���ٶ��������Ƕ�����
        ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_PWR_MGMT2, 0x00);
        ai_mpu9250_set_rate(50);
    } else {
        return -1;
    }
    
    ret = ai_mpu9250_read_byte(AI_AK8963_ADDR, AI_MPU9250_REG_MAG_WIA);
    if (ret == AI_AK8963_ADDR) {
        // ��λAK8963
        ai_mpu9250_write_byte(AI_AK8963_ADDR, AI_MPU9250_REG_MAG_CNTL2, 0x01);
        ai_delay_ms(50);
        // ����AK8963Ϊ���β���
        ai_mpu9250_write_byte(AI_AK8963_ADDR, AI_MPU9250_REG_MAG_CNTL1, 0x011);
    } else {
        return -1;
    }
    
    return  0;
}

/*
********************************************************************************
*    Function: ai_mpu9250_write_byte
* Description: IICдһ���ֽ�
*       Input: addr - mpu9250��I2C��ַ
*               reg - mpu9250�ļĴ�����ַ
*              data - ��д������
*      Output: None
*      Return: On success, 0 is returned,
*              On error, negative integer is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_write_byte(u8 addr, u8 reg, u8 data)
{
    ai_i2c_start();
    ai_i2c_send_byte((addr << 1) | 0x0);    // ����������ַ+д����
    if (ai_i2c_wait_ack() != 0) {
        ai_i2c_stop();
        return -1;
    }
    ai_i2c_send_byte(reg);
    ai_i2c_wait_ack();
    ai_i2c_send_byte(data);
    if (ai_i2c_wait_ack() != 0) {
        ai_i2c_stop();
        return -1;
    }
    ai_i2c_stop();
    return 0;
}

/*
********************************************************************************
*    Function: ai_mpu9250_read_byte
* Description: IICдһ���ֽ�
*       Input: addr - mpu9250��I2C��ַ
*               reg - mpu9250�ļĴ�����ַ
*      Output: None
*      Return: ����������
*      Others: None
********************************************************************************
*/
u8 ai_mpu9250_read_byte(u8 addr, u8 reg)
{
    u8 ret = 0;
    
    ai_i2c_start();
    ai_i2c_send_byte((addr << 1) | 0x0);
    ai_i2c_wait_ack();
    ai_i2c_send_byte(reg);
    ai_i2c_wait_ack();
    ai_i2c_start();
    ai_i2c_send_byte((addr << 1) | 0x1);
    ai_i2c_wait_ack();
    ret = ai_i2c_read_byte(0);
    ai_i2c_stop();
    return ret;
}

/*
********************************************************************************
*    Function: ai_mpu9250_write
* Description: IIC����д
*       Input: addr - mpu9250��I2C��ַ
*               reg - mpu9250�ļĴ�����ַ
*               buf - ��д��������
*               len - д������ݵĳ���
*      Output: None
*      Return: On success, 0 is returned,
*              On error, negative integer is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_write(u8 addr, u8 reg, u8 len, u8 *buf)
{
    u8 i = 0;
    
    if (buf == NULL)
        return -1;
    
    ai_i2c_start();
    ai_i2c_send_byte((addr << 1) | 0x0);
    if (ai_i2c_wait_ack() != 0) {
        ai_i2c_stop();
        return -1;
    }
    ai_i2c_send_byte(reg);
    ai_i2c_wait_ack();
    for (i = 0; i < len; i++) {
        ai_i2c_send_byte(buf[i]);
        if (ai_i2c_wait_ack() != 0) {
            ai_i2c_stop();
            return -1;
        }
    }
    ai_i2c_stop();
    return 0;
}

/*
********************************************************************************
*    Function: ai_mpu9250_read
* Description: IIC������ȡ
*       Input: addr - mpu9250��I2C��ַ
*               reg - mpu9250�ļĴ�����ַ
*               buf - ��ȡ�������ݴ洢��
*               len - Ҫ��ȡ�����ݳ���
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_read(u8 addr, u8 reg, u8 len, u8 *buf)
{
    ai_i2c_start();
    ai_i2c_send_byte((addr << 1) | 0x0);
    if (ai_i2c_wait_ack() != 0) {
        ai_i2c_stop();
        return -1;
    }
    ai_i2c_send_byte(reg);
    ai_i2c_wait_ack();
    ai_i2c_start();
    ai_i2c_send_byte((addr << 1) | 0x1);
    ai_i2c_wait_ack();
    while (len) {
        if (len == 1)
            *buf = ai_i2c_read_byte(0);
        else
            *buf = ai_i2c_read_byte(1);
        len--;
        buf++;
    }
    ai_i2c_stop();
    return 0;
}

/*
********************************************************************************
*    Function: ai_mpu9250_set_gyro_fsr
* Description: ����MPU9250�����Ǵ����������̷�Χ
*       Input: fsr -> 0,��250dps;1,��500dps;2,��1000dps;3,��2000dps
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_set_gyro_fsr(u8 fsr)
{
    // ���������������̷�Χ
    return ai_mpu9250_write_byte(AI_MPU9250_ADDR,
                                 AI_MPU9250_REG_GRYO_CFG, (fsr << 3) & ~0x3);
}

/*
********************************************************************************
*    Function: ai_mpu9250_set_accel_fsr
* Description: ����MPU9250���ٶȴ����������̷�Χ
*       Input: fsr -> 0,��2g;1,��4g;2,��8g;3,��16g
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_set_accel_fsr(u8 fsr)
{
    // ���ü��ٶȴ����������̷�Χ
    return ai_mpu9250_write_byte(AI_MPU9250_ADDR,
                                 AI_MPU9250_REG_ACCEL_CFG, fsr << 3);
}

/*
********************************************************************************
*    Function: ai_mpu9250_set_lpf
* Description: ����MPU9250�����ֵ�ͨ�˲���
*       Input: 
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_set_lpf(u16 lpf)
{
    u8 data = 0;
    
    if (lpf >= 188)
        data = 1;
    else if (lpf >= 98)
        data = 2;
    else if (lpf >= 42)
        data = 3;
    else if (lpf >= 20)
        data = 4;
    else if (lpf >= 10)
        data = 5;
    else
        data = 6;
    return ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_CFG, data);
}

/*
********************************************************************************
*    Function: ai_mpu9250_set_rate
* Description: ����MPU9250�Ĳ�����(�ٶ�Fs=1KHz)
*       Input: rate -> 4~1000(Hz)
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_set_rate(u16 rate)
{
    u8 data = 0;
    int ret = 0;
    
    if (rate > 1000)
        rate = 1000;
    if (rate < 4)
        rate = 4;
    data = 1000 / rate - 1;
    ret = ai_mpu9250_write_byte(AI_MPU9250_ADDR,
                                 AI_MPU9250_REG_SMPLRT_DIV, data);
    if (ret < 0)
        return -1;
    // �Զ�����LPFΪ�����ʵ�һ��
    return ai_mpu9250_set_lpf(rate / 2);
}

/*
********************************************************************************
*    Function: ai_mpu9250_get_temperature
* Description: �õ��¶�ֵ
*       Input: �¶�ֵ(������100��)
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
float ai_mpu9250_get_temperature(void)
{
    u8 buf[2] = {0};
    short raw = 0;
    float tmp = 0;
    
    ai_mpu9250_read(AI_MPU9250_ADDR, AI_MPU9250_REG_TEMP_OUT_H, 2, buf);
    raw = ((u16)buf[0] << 8) | buf[1];
    // 3.4.2 A.C. Electrical Characteristics
    // TEMP_degC = ((TEMP_OUT �C RoomTemp_Offset)/Temp_Sensitivity) + 21degC
    tmp = ((double)raw) / 333.87 + 21;
    return tmp;
}

/*
********************************************************************************
*    Function: ai_mpu9250_get_gyroscope
* Description: �õ������ǵ�ԭʼֵ
*       Input: void
*      Output: gx,gy,gz -> ָ��������x,y,z���ԭʼ����(������)
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_get_gyroscope(short *gx, short *gy, short *gz)
{
    u8 buf[6] = {0};
    int ret = 0;
    
    ret = ai_mpu9250_read(AI_MPU9250_ADDR, AI_MPU9250_REG_GYRO_XOUT_H, 6, buf);
    if (ret != 0)
        return -1;
    
    *gx = ((u16)buf[0] << 8) | buf[1];
    *gy = ((u16)buf[2] << 8) | buf[3];
    *gz = ((u16)buf[4] << 8) | buf[5];
    return 0;
}

/*
********************************************************************************
*    Function: ai_mpu9250_get_accelerometer
* Description: �õ����ٶȵ�ԭʼֵ
*       Input: void
*      Output: ax,ay,az -> ָ���ȡ�������Ǽ��ٶ�x,y,z���ԭʼ����������
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_get_accelerometer(short *ax, short *ay, short *az)
{
    u8 buf[6] = {0};
    int ret = 0;
    
    ret = ai_mpu9250_read(AI_MPU9250_ADDR, AI_MPU9250_REG_ACCEL_XOUT_H, 6, buf);
    if (ret != 0)
        return -1;
    
    *ax = ((u16)buf[0] << 8) | buf[1];
    *ay = ((u16)buf[2] << 8) | buf[3];
    *az = ((u16)buf[4] << 8) | buf[5];
    return 0;
}

/*
********************************************************************************
*    Function: ai_mpu9250_get_magnetometer
* Description: �õ������Ƶ�ԭʼֵ
*       Input: void
*      Output: mx,my,mz -> ��ȡ�Ĵ�����x,y,z��Ĵ�����ԭʼ���ݵ�ַ
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_get_magnetometer(short *mx, short *my, short *mz)
{
    u8 buf[6] = {0};
    int ret = 0;
    
    ret = ai_mpu9250_read(AI_AK8963_ADDR, AI_MPU9250_REG_MAG_HXL, 6, buf);
    if (ret == 0) {
        *mx = ((u16)buf[1] << 8) | buf[0];
        *my = ((u16)buf[3] << 8) | buf[2];
        *mz = ((u16)buf[5] << 8) | buf[4];
    }
    // AK8963ÿ�ζ����Ժ���Ҫ��������Ϊ���β���ģʽ
    ai_mpu9250_write_byte(AI_AK8963_ADDR, AI_MPU9250_REG_MAG_CNTL1, 0x11);
    return 0;
}
