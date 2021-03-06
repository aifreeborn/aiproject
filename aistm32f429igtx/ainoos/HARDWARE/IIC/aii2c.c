#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aii2c.h"

/*
********************************************************************************
*                          Private define
********************************************************************************
*/
// 定义I2C信号线，硬件使用PH4、PH5两个引脚来模拟I2C信号，并没有使用硬件I2C
#define    AI_I2C_SCL        PH_OUT(4)
#define    AI_I2C_SDA        PH_OUT(5)

// 设置SDA线的方向
#define    ai_i2c_set_sda_out() \
    do { \
        GPIOH->MODER &= ~(0x3 << (2 * 5)); \
        GPIOH->MODER |= 0x1 << (2 * 5); \
    } while (0)
    
#define    ai_i2c_set_sda_in() \
    do { \
        GPIOH->MODER &= ~(0x3 << (2 * 5)); \
        GPIOH->MODER |= 0x0 << (2 * 5); \
    } while (0)

#define    ai_i2c_read_sda()    PH_IN(5)
    
/*
********************************************************************************
*    Function: ai_i2c_init
* Description: 初始化I2C
*              PH4 -> IIC_SCL
*              PH5 -> IIC_SDA
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_i2c_init(void)
{
    RCC->AHB1ENR |= 0x1 << 7;
    ai_gpio_set(GPIOH, PIN4 | PIN5, GPIO_MODE_OUT,
                GPIO_OTYPE_PP, GPIO_SPEED_50M, GPIO_PUPD_PU);
    AI_I2C_SCL = 0;
    ai_delay_us(2);
    AI_I2C_SDA = 1;
    ai_delay_us(2);
    AI_I2C_SCL = 1;
}

/*
********************************************************************************
*    Function: ai_i2c_start
* Description: 生成I2C开始信号
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_i2c_start(void)
{
    ai_i2c_set_sda_out();
    AI_I2C_SDA = 1;
    AI_I2C_SCL = 1;
    ai_delay_us(4);
    AI_I2C_SDA = 0;
    ai_delay_us(4);
    AI_I2C_SCL = 0;
}

/*
********************************************************************************
*    Function: ai_i2c_stop
* Description: i2c停止信号
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_i2c_stop(void)
{
    ai_i2c_set_sda_out();
    AI_I2C_SCL = 0;
    AI_I2C_SDA = 0;
    ai_delay_us(4);
    AI_I2C_SCL = 1;
    ai_delay_us(4);
    AI_I2C_SDA = 1;
    ai_delay_us(4);
}

/*
********************************************************************************
*    Function: ai_i2c_wait_ack
* Description: 等待应答信号到来
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_i2c_wait_ack(void)
{
    u8 timeout = 0;
    
    ai_i2c_set_sda_in();
    AI_I2C_SCL = 0;
    AI_I2C_SDA = 1;
    ai_delay_us(1);
    AI_I2C_SCL = 1;
    ai_delay_us(1);
    while (ai_i2c_read_sda()) {
        timeout++;
        if (timeout > 250) {
            ai_i2c_stop();
            return -1;
        }
    }
    
    AI_I2C_SCL = 0;
    ai_delay_us(1);
    return 0;
}

/*
********************************************************************************
*    Function: ai_i2c_ack
* Description: 产生ACK应答
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_i2c_ack(void)
{
    AI_I2C_SCL = 0;
    ai_i2c_set_sda_out();
    
    AI_I2C_SDA = 0;
    ai_delay_us(2);
    AI_I2C_SCL = 1;
    ai_delay_us(2);
    AI_I2C_SCL = 0;
}

/*
********************************************************************************
*    Function: ai_i2c_no_ack
* Description: 不产生ACK应答
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_i2c_no_ack(void)
{
    AI_I2C_SCL = 0;
    ai_i2c_set_sda_out();
    
    AI_I2C_SDA = 1;
    ai_delay_us(2);
    AI_I2C_SCL = 1;
    ai_delay_us(2);
    AI_I2C_SCL = 0;
}

/*
********************************************************************************
*    Function: ai_i2c_send_byte
* Description: IIC发送一个字节
*       Input: data - 待发送的一字节数据
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_i2c_send_byte(u8 data)
{
    int i = 0;
    
    ai_i2c_set_sda_out();
    AI_I2C_SCL = 0;
    for (i = 0; i < 8; i++) {
        AI_I2C_SDA = (data & 0x80) >> 7;
        data <<= 1;
        ai_delay_us(2);    // 对TEA5767这三个延时都是必须的
        AI_I2C_SCL = 1;
        ai_delay_us(2);
        AI_I2C_SCL = 0;
        ai_delay_us(2);
    }
}

/*
********************************************************************************
*    Function: ai_i2c_read_byte
* Description: 读1个字节
*       Input: ack - ack=1时，发送ACK，ack=0，发送NO ACK
*      Output: None
*      Return: 读取的字节数据
*      Others: None
********************************************************************************
*/
u8 ai_i2c_read_byte(u8 ack)
{
    int i = 0;
    u8 recv = 0;
    
    ai_i2c_set_sda_in();
    for (i = 0; i < 8; i++) {
        AI_I2C_SCL = 0;
        ai_delay_us(2);
        AI_I2C_SCL = 1;
        recv <<= 1;
        if (ai_i2c_read_sda())
            recv++;
        ai_delay_us(1);
    }
    
    if (ack)
        ai_i2c_ack();
    else
        ai_i2c_no_ack();
    
    return recv;
}
