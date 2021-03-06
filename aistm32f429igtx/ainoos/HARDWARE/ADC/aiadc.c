#include "stm32f4xx.h"
#include "aitypes.h"
#include "aigpio.h"
#include "aidelay.h"
#include "aiadc.h"

/*
********************************************************************************
*    Function: ai_adc_init
* Description: 初始化ADC,以规则通道为例,默认仅开启ADC1_CH5
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_adc_init(void)
{
    RCC->APB2ENR |= 0x1 << 8;        // 使能ADC1时钟
    RCC->AHB1ENR |= 0x1;
    ai_gpio_set(GPIOA, PIN5, GPIO_MODE_AIN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PU);
    
    RCC->APB2RSTR |= 0x1 << 8;       // ADC复位
    RCC->APB2RSTR &= ~(0x1 << 8);
    ADC->CCR = 0x1 << 16;            // PCLK2 divided by 4,PCLK2/4=90/4=22.5Mhz
                                     // ADC时钟最好不要超过36Mhz
    ADC->CCR |= 0x1 << 23;           // 使能内部温度传感器
    ADC1->CR1 = 0;                   // CR1设置清零
    ADC1->CR2 = 0;
    ADC1->CR1 |= 0x0 << 24;          // 12-bit (15 ADCCLK cycles)
    ADC1->CR1 |= 0x0 << 8;           // Scan mode disabled
     
    ADC1->CR2 &= ~(0x1 << 11);       // 数据右对齐
    ADC1->CR2 |= 0x0 << 28;          // 软件触发
    ADC1->CR2 &= ~(0x1 << 1);        // 单次转换
    
    ADC1->SQR1 &= ~(0xf << 20);
    ADC1->SQR1 |= (0x0 << 20);
    
    // 设置通道5的采样时间
    ADC1->SMPR2 &= ~(0x7 << (3 * 5));
    ADC1->SMPR2 |= 0x7 << (3 * 5);   // 480个周期,提高采样时间可以提高精确度
    ADC1->SMPR1 &= ~(0x7 << 24);     // 清除通道18原来的设置
    ADC1->SMPR1 |= 0x7 << 24;        // 480个周期
    ADC1->CR2 |= 0x1;
}

/*
********************************************************************************
*    Function: ai_adc_get_data
* Description: 获取指定通道的ADC数据值
*       Input: ch - 通道号， 0~18
*      Output: None
*      Return: 转换的结果
*      Others: None
********************************************************************************
*/
u16 ai_adc_get_data(u8 ch)
{
    ADC1->SQR3 &= 0XFFFFFFE0;
    ADC1->SQR3 |= (u32)ch;
    ADC1->CR2 |= 0x1 << 30;                  // 开始转换规则通道
    while ((ADC1->SR & (0x1 << 1)) == 0)
        ; /* Empty */
    
    return ADC1->DR;
}

/*
********************************************************************************
*    Function: ai_adc_get_average_val
* Description: 获取指定通道的ADC数据值,取times次,然后平均 
*       Input:    ch - 通道号， 0~18
*              times - 获取的次数
*      Output: None
*      Return: 通道ch的times次转换结果平均值
*      Others: None
********************************************************************************
*/
u16 ai_adc_get_average_val(u8 ch, u8 times)
{
    u8 i = 0;
    u32 tmp = 0;
    
    if (times == 0)
        return 0;
    
    for (i = 0; i < times; i++) {
        tmp += ai_adc_get_data(ch);
        ai_delay_ms(5);
    }
    
    return tmp / times;
}

/*
********************************************************************************
*    Function: ai_adc_get_temperature
* Description: 获取内部温度传感器的数值 
*       Input: void
*      Output: None
*      Return: 温度值(扩大了100倍,单位:℃.)
*      Others: None
********************************************************************************
*/
int ai_adc_get_temperature(void)
{
    u32 ret = 0;
    double temperature = 0.0;
    
    ret = ai_adc_get_average_val(18, 20);
    temperature = ((double)ret * 3.3) / 4096.0;
    temperature = (temperature - 0.76) / 0.0025 + 25;
    temperature *= 100;
    
    return (int)temperature;
    
}
