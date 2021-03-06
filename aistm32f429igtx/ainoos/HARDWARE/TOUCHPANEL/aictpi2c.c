#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aictpi2c.h"

/*
********************************************************************************
*                          Private define
* ���ݴ�����ʹ�õĿ��ƽӿڣ�
*     T_PEN         -> PH7      CT_INT
*     T_SCK         -> PH6      CT_SCL
*     T_MISO        -> PG3      
*     T_MOSI        -> PI3      CT_SDA
*     T_CS          -> PI8      CT_RST
*
* CTP capacitive touch panel, ����ʽ������
********************************************************************************
*/
// ����I2C�ź��ߣ�Ӳ��ʹ��PH6��PI3��������
#define    AI_CTP_I2C_SCL        PH_OUT(6)
#define    AI_CTP_I2C_SDA        PI_OUT(3)

// ����SDA�ߵķ���
#define    ai_ctp_i2c_set_sda_out() \
    do { \
        GPIOI->MODER &= ~(0x3 << (2 * 3)); \
        GPIOI->MODER |= 0x1 << (2 * 3); \
    } while (0)
    
#define    ai_ctp_i2c_set_sda_in() \
    do { \
        GPIOI->MODER &= ~(0x3 << (2 * 3)); \
        GPIOI->MODER |= 0x0 << (2 * 3); \
    } while (0)

#define    ai_ctp_i2c_read_sda()    PI_IN(3)

/*
********************************************************************************
*    Function: ai_ctp_i2c_init
* Description: ��ʼ��I2C
*              PH4 -> IIC_SCL
*              PH5 -> IIC_SDA
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ctp_i2c_init(void)
{
    RCC->AHB1ENR |= 0x1 << 7;
    RCC->AHB2ENR |= 0x1 << 8;
    
    ai_gpio_set(GPIOH, PIN6, GPIO_MODE_OUT,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set(GPIOI, PIN3, GPIO_MODE_OUT,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    AI_CTP_I2C_SCL = 0;
    ai_delay_us(2);
    AI_CTP_I2C_SDA = 1;
    ai_delay_us(2);
    AI_CTP_I2C_SCL = 1;
}

/*
********************************************************************************
*    Function: ai_ctp_i2c_start
* Description: ����I2C��ʼ�ź�
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ctp_i2c_start(void)
{
    ai_ctp_i2c_set_sda_out();
    AI_CTP_I2C_SDA = 1;
    AI_CTP_I2C_SCL = 1;
    ai_delay_us(4);
    AI_CTP_I2C_SDA = 0;
    ai_delay_us(4);
    AI_CTP_I2C_SCL = 0;    // ǯסI2C���ߣ�׼�����ͻ�������� 
}

/*
********************************************************************************
*    Function: ai_ctp_i2c_stop
* Description: i2cֹͣ�ź�
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ctp_i2c_stop(void)
{
    ai_ctp_i2c_set_sda_out();
    AI_CTP_I2C_SCL = 0;
    AI_CTP_I2C_SDA = 0;
    ai_delay_us(4);
    AI_CTP_I2C_SCL = 1;
    ai_delay_us(4);
    AI_CTP_I2C_SDA = 1;
    ai_delay_us(4);
}

/*
********************************************************************************
*    Function: ai_ctp_i2c_wait_ack
* Description: �ȴ�Ӧ���źŵ���
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_ctp_i2c_wait_ack(void)
{
    u8 timeout = 0;
    
    ai_ctp_i2c_set_sda_in();
    AI_CTP_I2C_SCL = 0;
    AI_CTP_I2C_SDA = 1;
    ai_delay_us(1);
    AI_CTP_I2C_SCL = 1;
    ai_delay_us(1);
    while (ai_ctp_i2c_read_sda()) {
        timeout++;
        if (timeout > 250) {
            ai_ctp_i2c_stop();
            return -1;
        }
    }
    
    AI_CTP_I2C_SCL = 0;
    ai_delay_us(1);
    return 0;
}

/*
********************************************************************************
*    Function: ai_ctp_i2c_ack
* Description: ����ACKӦ��
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ctp_i2c_ack(void)
{
    AI_CTP_I2C_SCL = 0;
    ai_ctp_i2c_set_sda_out();
    
    AI_CTP_I2C_SDA = 0;
    ai_delay_us(2);
    AI_CTP_I2C_SCL = 1;
    ai_delay_us(2);
    AI_CTP_I2C_SCL = 0;
}

/*
********************************************************************************
*    Function: ai_ctp_i2c_no_ack
* Description: ������ACKӦ��
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ctp_i2c_no_ack(void)
{
    AI_CTP_I2C_SCL = 0;
    ai_ctp_i2c_set_sda_out();
    
    AI_CTP_I2C_SDA = 1;
    ai_delay_us(2);
    AI_CTP_I2C_SCL = 1;
    ai_delay_us(2);
    AI_CTP_I2C_SCL = 0;
}

/*
********************************************************************************
*    Function: ai_ctp_i2c_send_byte
* Description: IIC����һ���ֽ�
*       Input: data - �����͵�һ�ֽ�����
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ctp_i2c_send_byte(u8 data)
{
    int i = 0;
    
    ai_ctp_i2c_set_sda_out();
    AI_CTP_I2C_SCL = 0;
    for (i = 0; i < 8; i++) {
        AI_CTP_I2C_SDA = (data & 0x80) >> 7;
        data <<= 1;
        ai_delay_us(2);    // ��TEA5767��������ʱ���Ǳ����
        AI_CTP_I2C_SCL = 1;
        ai_delay_us(2);
        AI_CTP_I2C_SCL = 0;
        ai_delay_us(2);
    }
}

/*
********************************************************************************
*    Function: ai_ctp_i2c_read_byte
* Description: ��1���ֽ�
*       Input: ack - ack=1ʱ������ACK��ack=0������NO ACK
*      Output: None
*      Return: ��ȡ���ֽ�����
*      Others: None
********************************************************************************
*/
u8 ai_ctp_i2c_read_byte(u8 ack)
{
    int i = 0;
    u8 recv = 0;
    
    ai_ctp_i2c_set_sda_in();
    for (i = 0; i < 8; i++) {
        AI_CTP_I2C_SCL = 0;
        ai_delay_us(2);
        AI_CTP_I2C_SCL = 1;
        recv <<= 1;
        if (ai_ctp_i2c_read_sda())
            recv++;
        ai_delay_us(1);
    }
    
    if (ack)
        ai_ctp_i2c_ack();
    else
        ai_ctp_i2c_no_ack();
    
    return recv;
}
