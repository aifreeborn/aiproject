#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aidac.h"

/*
********************************************************************************
*    Function: ai_dac_init
* Description: DAC通道1输出初始化
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_dac_init(void)
{
    RCC->APB1ENR |= 0x1 << 29;
    RCC->AHB1ENR |= 0x1;
    ai_gpio_set(GPIOA, PIN4, GPIO_MODE_AIN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PU);
    
    DAC->CR |= 0x1;
    DAC->CR &= ~(0x1 << 13);
    DAC->CR &= ~(0x1 << 12);
    DAC->CR &= ~(0xf << 8);
    DAC->CR &= ~(0x3 << 6);
    DAC->CR &= ~(0x7 << 3);        // 定时器 6 TRGO 事件, 需要TEN1=1
    DAC->CR &= ~(0x1 << 2);
    DAC->CR |= 0x1 << 1;
    DAC->DHR12R1 = 0;
}

/*
********************************************************************************
*    Function: ai_dac_set_vol
* Description: 设置通道1输出电压
*       Input: vol - 0~3300,代表0~3.3V
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_dac_set_vol(u16 vol)
{
    double tmp = vol;
    
    tmp /= 1000;
    tmp = tmp * 4095 / 3.3;
    DAC->DHR12R1 = tmp;
}

u16 ai_dac_get_vol(void)
{
    return DAC->DHR12R1;
}
