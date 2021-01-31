#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aispi.h"

/*
********************************************************************************
*    Function: ai_spi_init
* Description: SPI模块的初始化: 配置成主机模,SPI5的初始化
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: 板子上使用SPI5控制FLASH - W25Q256
*              SPI5_SCK -> PF7    SPI5_MOSI -> PF9    SPI5_MISO -> PF8
*              F_CS -> PF6
********************************************************************************
*/
void ai_spi5_init(void)
{
    u16 reg_val = 0;
    
    RCC->AHB1ENR |= 0x1 << 5;
    RCC->APB2ENR |= 0x1 << 20;    // SPI5 clock enabled
    // 设置GPIOF7、8、9为复用功能AF5 -> SPI5
    ai_gpio_set(GPIOF, PIN9 | PIN8 | PIN7, GPIO_MODE_AF,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set_af(GPIOF, 7, 5);
    ai_gpio_set_af(GPIOF, 8, 5);
    ai_gpio_set_af(GPIOF, 9, 5);
    
    RCC->APB2RSTR |= 0x1 << 20;       // SPI5复位
    RCC->APB2RSTR &= ~(0x1 << 20);
    reg_val |= 0x0 << 11;             // 8bit数据
    reg_val |= 0x0 << 10;
    reg_val |= 0x1 << 9;              // Software slave management
    reg_val |= 0x1 << 8;
    reg_val |= 0x0 << 7;              // MSB first
    reg_val |= 0x1 << 6;              // SPI enable
    // 对SPI5属于APB2的外设.时钟频率最大为96Mhz频率.
    reg_val |= 0x7 << 3;              // Fsck = Fpclk / 256
    reg_val |= 0x1 << 2;              // SPI主机
    reg_val |= 0x1 << 1;              // 空闲状态时， SCK保持高电平,CPOL=1 
    reg_val |= 0x1;                   // 从第二个时钟边沿开始采样数据,CPHA=1
    SPI5->CR1 = reg_val;
    SPI5->I2SCFGR &= ~(0x1 << 11);    // 选择SPI模式
    // 启动传输，它最大的作用就是维持 MOSI 为高电平，而且这句话也不是必须的，
    // 可以去掉
    ai_spi5_rdwr_byte(0xff);
}

/*
********************************************************************************
*    Function: ai_spi5_rdwr_byte
* Description: 读写一个字节
*       Input: data - 要写入的字节
*      Output: None
*      Return: 读取到的字节
*      Others: 因为使用的是全双工模式，发送的同时会接收,8bit数据
********************************************************************************
*/

u8 ai_spi5_rdwr_byte(u8 data)
{
    while ((SPI5->SR & (0x1 << 1)) == 0)
        ; /* 等待发送区空 */
    SPI5->DR = data;
    while ((SPI5->SR & (0x1 << 0)) == 0)
        ; /* 等待接收完一个byte */
    return SPI5->DR;
}

/*
********************************************************************************
*    Function: ai_spi5_set_speed
* Description: SPI5速度设置函数
*       Input: br - 设置传输速度，即波特率控制，取值范围[0~7]
*      Output: None
*      Return: void
*      Others: SPI速度 = fAPB2 / 2^(br + 1)
*              fAPB2时钟一般为90Mhz
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
