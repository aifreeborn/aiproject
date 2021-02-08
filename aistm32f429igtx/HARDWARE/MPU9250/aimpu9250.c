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
* Description: 初始化MPU9250
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
    // 复位MPU9250
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_PWR_MGMT1, 0x80);
    ai_delay_ms(100);
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_PWR_MGMT1, 0x00);
    
    ai_mpu9250_set_gyro_fsr(3);    // 陀螺仪传感器,±2000dps
    ai_mpu9250_set_accel_fsr(0);   // 加速度传感器,±2g
    ai_mpu9250_set_rate(50);       // 设置采样率50Hz
    // 关闭所有中断
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_INT_EN, 0x00);
    // I2C主模式关闭
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_USER_CTRL, 0x00);
    // 关闭FIFO
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_FIFO_EN, 0x00);
    // INT引脚低电平有效，开启bypass模式，可以直接读取磁力计
    ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_INT_PIN_CFG, 0x82);
    ret = ai_mpu9250_read_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_WHO_AM_I);
    if (ret == AI_MPU9250_ID1 || ret == AI_MPU9250_ID2) {
        // 设置CLKSEL,PLL X轴为参考
        ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_PWR_MGMT1, 0x01);
        // 加速度与陀螺仪都工作
        ai_mpu9250_write_byte(AI_MPU9250_ADDR, AI_MPU9250_REG_PWR_MGMT2, 0x00);
        ai_mpu9250_set_rate(50);
    } else {
        return -1;
    }
    
    ret = ai_mpu9250_read_byte(AI_AK8963_ADDR, AI_MPU9250_REG_MAG_WIA);
    if (ret == AI_AK8963_ADDR) {
        // 复位AK8963
        ai_mpu9250_write_byte(AI_AK8963_ADDR, AI_MPU9250_REG_MAG_CNTL2, 0x01);
        ai_delay_ms(50);
        // 设置AK8963为单次测量
        ai_mpu9250_write_byte(AI_AK8963_ADDR, AI_MPU9250_REG_MAG_CNTL1, 0x011);
    } else {
        return -1;
    }
    
    return  0;
}

/*
********************************************************************************
*    Function: ai_mpu9250_write_byte
* Description: IIC写一个字节
*       Input: addr - mpu9250的I2C地址
*               reg - mpu9250的寄存器地址
*              data - 待写的数据
*      Output: None
*      Return: On success, 0 is returned,
*              On error, negative integer is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_write_byte(u8 addr, u8 reg, u8 data)
{
    ai_i2c_start();
    ai_i2c_send_byte((addr << 1) | 0x0);    // 发送器件地址+写命令
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
* Description: IIC写一个字节
*       Input: addr - mpu9250的I2C地址
*               reg - mpu9250的寄存器地址
*      Output: None
*      Return: 读到的数据
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
* Description: IIC连续写
*       Input: addr - mpu9250的I2C地址
*               reg - mpu9250的寄存器地址
*               buf - 待写的数据区
*               len - 写入的数据的长度
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
* Description: IIC连续读取
*       Input: addr - mpu9250的I2C地址
*               reg - mpu9250的寄存器地址
*               buf - 读取到的数据存储区
*               len - 要读取的数据长度
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
* Description: 设置MPU9250陀螺仪传感器满量程范围
*       Input: fsr -> 0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_set_gyro_fsr(u8 fsr)
{
    // 设置陀螺仪满量程范围
    return ai_mpu9250_write_byte(AI_MPU9250_ADDR,
                                 AI_MPU9250_REG_GRYO_CFG, (fsr << 3) & ~0x3);
}

/*
********************************************************************************
*    Function: ai_mpu9250_set_accel_fsr
* Description: 设置MPU9250加速度传感器满量程范围
*       Input: fsr -> 0,±2g;1,±4g;2,±8g;3,±16g
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_mpu9250_set_accel_fsr(u8 fsr)
{
    // 设置加速度传感器满量程范围
    return ai_mpu9250_write_byte(AI_MPU9250_ADDR,
                                 AI_MPU9250_REG_ACCEL_CFG, fsr << 3);
}

/*
********************************************************************************
*    Function: ai_mpu9250_set_lpf
* Description: 设置MPU9250的数字低通滤波器
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
* Description: 设置MPU9250的采样率(假定Fs=1KHz)
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
    // 自动设置LPF为采样率的一半
    return ai_mpu9250_set_lpf(rate / 2);
}

/*
********************************************************************************
*    Function: ai_mpu9250_get_temperature
* Description: 得到温度值
*       Input: 温度值(扩大了100倍)
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
    // TEMP_degC = ((TEMP_OUT – RoomTemp_Offset)/Temp_Sensitivity) + 21degC
    tmp = ((double)raw) / 333.87 + 21;
    return tmp;
}

/*
********************************************************************************
*    Function: ai_mpu9250_get_gyroscope
* Description: 得到陀螺仪的原始值
*       Input: void
*      Output: gx,gy,gz -> 指向陀螺仪x,y,z轴的原始读数(带符号)
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
* Description: 得到加速度的原始值
*       Input: void
*      Output: ax,ay,az -> 指向读取的陀螺仪加速度x,y,z轴的原始带符号数据
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
* Description: 得到磁力计的原始值
*       Input: void
*      Output: mx,my,mz -> 读取的磁力计x,y,z轴的带符号原始数据地址
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
    // AK8963每次读完以后都需要重新设置为单次测量模式
    ai_mpu9250_write_byte(AI_AK8963_ADDR, AI_MPU9250_REG_MAG_CNTL1, 0x11);
    return 0;
}
