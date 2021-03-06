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
// AP3216C器件IIC地址(左移了一位)，包含了R/W位
#define    AI_AP3216C_ADDR        0x3C

/*
********************************************************************************
*    Function: ai_ap3216c_init
* Description: 初始化AP3216C
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
    ai_ap3216c_write_byte(0x00, 0x04);    // 复位AP3216C
    ai_delay_ms(50);                      // AP33216C复位至少10ms
    ai_ap3216c_write_byte(0x00, 0x03);    // 开启ALS、PS+IR
    ai_ap3216c_read_byte(0x00, &tmp);
    if (tmp == 0x03)
        return 0;
    return -1;
}

/*
********************************************************************************
*    Function: ai_ap3216c_read_byte
* Description: IIC读一个字节
*       Input:  reg - 寄存器地址
*      Output: data - 指针类型，指向读取的一字节数据
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
* Description: IIC写一个字节
*       Input:  reg - 寄存器地址
*              data - 要写入的数据
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
* Description: 读取AP3216C的数据,读取原始数据，包括ALS,PS和IR
*       Input: ir, ps, als - 读取数据保存的地址
*      Output: ir, ps, als - 读取的数据保存的地址
*      Return: void
*      Others: 注意！如果同时打开ALS,IR+PS的话两次数据读取的时间间隔要大于112.5ms
*              ALS:环境光强度
*               PS:接近距离
*               IR:红外线强度
********************************************************************************
*/
void ai_ap3216c_read_data(u16 *ir, u16 *ps, u16 *als)
{
    u8 i = 0;
    u8 buf[6] = {0};
    
    // 循环读取所有传感器数据
    for (i = 0; i < 6; i++) {
        ai_ap3216c_read_byte(0x0a + i, &buf[i]);
    }
    
    if (buf[0] & 0x80)
        *ir = 0;        // IR_OF位为1,则数据无效
    else
        *ir = ((u16)buf[1] << 2) | (buf[0] & 0x03);
    *als = ((u16)buf[3] << 8) | buf[2];
    if (buf[4] & 0x40)
        *ps = 0;
    else
        *ps = (((u16)buf[5] & 0x3f) << 4) | (buf[4] & 0x0f);
}
