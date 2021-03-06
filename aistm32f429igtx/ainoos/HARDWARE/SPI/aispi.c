#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aispi.h"

/*
********************************************************************************
*    Function: ai_spi_init
* Description: SPIģ��ĳ�ʼ��: ���ó�����ģ,SPI5�ĳ�ʼ��
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: ������ʹ��SPI5����FLASH - W25Q256
*              SPI5_SCK -> PF7    SPI5_MOSI -> PF9    SPI5_MISO -> PF8
*              F_CS -> PF6
********************************************************************************
*/
void ai_spi5_init(void)
{
    u16 reg_val = 0;
    
    RCC->AHB1ENR |= 0x1 << 5;
    RCC->APB2ENR |= 0x1 << 20;    // SPI5 clock enabled
    // ����GPIOF7��8��9Ϊ���ù���AF5 -> SPI5
    ai_gpio_set(GPIOF, PIN9 | PIN8 | PIN7, GPIO_MODE_AF,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set_af(GPIOF, 7, 5);
    ai_gpio_set_af(GPIOF, 8, 5);
    ai_gpio_set_af(GPIOF, 9, 5);
    
    RCC->APB2RSTR |= 0x1 << 20;       // SPI5��λ
    RCC->APB2RSTR &= ~(0x1 << 20);
    reg_val |= 0x0 << 11;             // 8bit����
    reg_val |= 0x0 << 10;
    reg_val |= 0x1 << 9;              // Software slave management
    reg_val |= 0x1 << 8;
    reg_val |= 0x0 << 7;              // MSB first
    reg_val |= 0x1 << 6;              // SPI enable
    // ��SPI5����APB2������.ʱ��Ƶ�����Ϊ96MhzƵ��.
    reg_val |= 0x7 << 3;              // Fsck = Fpclk / 256
    reg_val |= 0x1 << 2;              // SPI����
    reg_val |= 0x1 << 1;              // ����״̬ʱ�� SCK���ָߵ�ƽ,CPOL=1 
    reg_val |= 0x1;                   // �ӵڶ���ʱ�ӱ��ؿ�ʼ��������,CPHA=1
    SPI5->CR1 = reg_val;
    SPI5->I2SCFGR &= ~(0x1 << 11);    // ѡ��SPIģʽ
    // �������䣬���������þ���ά�� MOSI Ϊ�ߵ�ƽ��������仰Ҳ���Ǳ���ģ�
    // ����ȥ��
    ai_spi5_rdwr_byte(0xff);
}

/*
********************************************************************************
*    Function: ai_spi5_rdwr_byte
* Description: ��дһ���ֽ�
*       Input: data - Ҫд����ֽ�
*      Output: None
*      Return: ��ȡ�����ֽ�
*      Others: ��Ϊʹ�õ���ȫ˫��ģʽ�����͵�ͬʱ�����,8bit����
********************************************************************************
*/

u8 ai_spi5_rdwr_byte(u8 data)
{
    while ((SPI5->SR & (0x1 << 1)) == 0)
        ; /* �ȴ��������� */
    SPI5->DR = data;
    while ((SPI5->SR & (0x1 << 0)) == 0)
        ; /* �ȴ�������һ��byte */
    return SPI5->DR;
}

/*
********************************************************************************
*    Function: ai_spi5_set_speed
* Description: SPI5�ٶ����ú���
*       Input: br - ���ô����ٶȣ��������ʿ��ƣ�ȡֵ��Χ[0~7]
*      Output: None
*      Return: void
*      Others: SPI�ٶ� = fAPB2 / 2^(br + 1)
*              fAPB2ʱ��һ��Ϊ90Mhz
********************************************************************************
*/
void ai_spi5_set_speed(u8 br)
{
    br &= 0x7;
    SPI5->CR1 &= ~(0x1 << 6);
    SPI5->CR1 &= ~(0x7 << 3);
    SPI5->CR1 |= br << 3;
    SPI5->CR1 |= 0x1 << 6;
}
