#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aidht11.h"

/*
********************************************************************************
*                          PRIVATE DEFINE/FUNCTIONS
********************************************************************************
*/
// ��������I/O��
#define    AI_DHT11_DOUT        PB_OUT(12)
#define    AI_DHT11_DIN         PB_IN(12)

#define    ai_dht11_set_in() \
    do { \
        GPIOB->MODER &= ~(0x3 << (2 * 12)); \
        GPIOB->MODER |= 0x0 << (2 * 12); \
    } while (0)
    
#define    ai_dht11_set_out() \
    do { \
        GPIOB->MODER &= ~(0x3 << (2 * 12)); \
        GPIOB->MODER |= 0x1 << (2 * 12); \
    } while (0)

/*
********************************************************************************
*    Function: ai_dht11_reset
* Description: ��λDHT11
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
static void ai_dht11_reset(void)
{
    ai_dht11_set_out();
    AI_DHT11_DOUT = 0;
    ai_delay_ms(20);      // ��������18ms
    AI_DHT11_DOUT = 1;    // ��������
    ai_delay_us(30);      // ��������20~40us
}

/*
********************************************************************************
*    Function: ai_dht11_check
* Description: �ȴ�DHT11�Ļ�Ӧ
*       Input: void
*      Output: None
*      Return: -1 -> δ��⵽DHT11�Ĵ���
*               0 -> ��⵽DHT11�Ĵ���
*      Others: None
********************************************************************************
*/
static int ai_dht11_check(void)
{
    u8 timeout = 0;
    
    ai_dht11_set_in();
    while (AI_DHT11_DIN && (timeout < 100)) {    // DHT11������40~50us
        timeout++;
        ai_delay_us(1);
    }
    if (timeout >= 100)
        return -1;
    else
        timeout = 0;
    
    while (!AI_DHT11_DIN && timeout < 100) {    // DHT11���ͺ���ٴ�����40~50us
        timeout++;
        ai_delay_us(1);
    }
    if (timeout >= 100)
        return -1;
    return 0;
}

/*
********************************************************************************
*    Function: ai_dht11_init
* Description: ��ʼ��DHT11��IO��,ͬʱ���DHT11�Ĵ���
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: DHT11�� MCU��һ��ͨ�����Ϊ 3ms ���ң�
*              ��������������ȡʱ������ҪС�� 100ms
*              ��ȡDHT11��ʱ��,��Ҫ�ȶ� PCF8574T ����һ�ζ�ȡ������
*               ���ͷ� IIC_INT ����
********************************************************************************
*/
int ai_dht11_init(void)
{
    RCC->AHB1ENR |= 0x1 << 1;
    ai_gpio_set(GPIOB, PIN12, GPIO_MODE_OUT,
                GPIO_OTYPE_PP, GPIO_SPEED_50M, GPIO_PUPD_PU);
    ai_dht11_reset();
    return ai_dht11_check();
}

/*
********************************************************************************
*    Function: ai_dht11_read_bit
* Description: ��DHT11��ȡһ��λ
*       Input: void
*      Output: None
*      Return: ���ض�ȡ�ĵ�ƽֵ1/0
*      Others: None
********************************************************************************
*/
static u8 ai_dht11_read_bit(void)
{
    u8 timeout = 0;
    
    while (AI_DHT11_DIN && timeout < 120) {     // �ȴ���Ϊ�͵�ƽ
        timeout++;
        ai_delay_us(1);
    }
    timeout = 0;
    while (!AI_DHT11_DIN && timeout < 120) {    // �ȴ���ߵ�ƽ
        timeout++;
        ai_delay_us(1);
    }
    ai_delay_us(30);
    if (AI_DHT11_DIN)
        return 1;
    else
        return 0;
}

/*
********************************************************************************
*    Function: ai_dht11_read_byte
* Description: ��DHT11��ȡһ���ֽ�
*       Input: void
*      Output: None
*      Return: ���ض�ȡ���ֽ�����
*      Others: DHT11�������Ǹ�λ�ȳ�
********************************************************************************
*/
static u8 ai_dht11_read_byte(void)
{
    u8 i, data = 0;
    
    for (i = 0; i < 8; i++) {
        data <<= 1;
        data |= ai_dht11_read_bit();
    }
    
    return data;
}

/*
********************************************************************************
*    Function: ai_dht11_read_data
* Description: ��DHT11��ȡһ������
*       Input: None
*      Output: temperature - �¶�ֵ
*                 humidity - ʪ��ֵ
*              �߰�λ���������֣��Ͱ�λ��С������
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: DHT11�������Ǹ�λ�ȳ�
********************************************************************************
*/
int ai_dht11_read_data(u16 *temperature, u16 *humidity)
{
    u8 buf[5] = {0};
    u8 i;
    
    ai_dht11_reset();
    if (ai_dht11_check() == 0) {
        for (i = 0; i < 5; i++) {
            buf[i] = ai_dht11_read_byte();
        }
        
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
            *temperature = ((u16)buf[2] << 8) | buf[3];
            *humidity = ((u16)buf[0] << 8) | buf[1];
        }
    } else {
        return -1;
    }
    
    return 0;
}
