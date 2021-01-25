#include "stm32f4xx.h"
#include "aitypes.h"
#include "aigpio.h"
#include "aidelay.h"
#include "aiadc.h"

/*
********************************************************************************
*    Function: ai_adc_init
* Description: ��ʼ��ADC,�Թ���ͨ��Ϊ��,Ĭ�Ͻ�����ADC1_CH5
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_adc_init(void)
{
    RCC->APB2ENR |= 0x1 << 8;        // ʹ��ADC1ʱ��
    RCC->AHB1ENR |= 0x1;
    ai_gpio_set(GPIOA, PIN5, GPIO_MODE_AIN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PU);
    
    RCC->APB2RSTR |= 0x1 << 8;       // ADC��λ
    RCC->APB2RSTR &= ~(0x1 << 8);
    ADC->CCR = 0x1 << 16;            // PCLK2 divided by 4,PCLK2/4=90/4=22.5Mhz
                                     // ADCʱ����ò�Ҫ����36Mhz
    ADC->CCR |= 0x1 << 23;           // ʹ���ڲ��¶ȴ�����
    ADC1->CR1 = 0;                   // CR1��������
    ADC1->CR2 = 0;
    ADC1->CR1 |= 0x0 << 24;          // 12-bit (15 ADCCLK cycles)
    ADC1->CR1 |= 0x0 << 8;           // Scan mode disabled
     
    ADC1->CR2 &= ~(0x1 << 11);       // �����Ҷ���
    ADC1->CR2 |= 0x0 << 28;          // �������
    ADC1->CR2 &= ~(0x1 << 1);        // ����ת��
    
    ADC1->SQR1 &= ~(0xf << 20);
    ADC1->SQR1 |= (0x0 << 20);
    
    // ����ͨ��5�Ĳ���ʱ��
    ADC1->SMPR2 &= ~(0x7 << (3 * 5));
    ADC1->SMPR2 |= 0x7 << (3 * 5);   // 480������,��߲���ʱ�������߾�ȷ��
    ADC1->SMPR1 &= ~(0x7 << 24);     // ���ͨ��18ԭ��������
    ADC1->SMPR1 |= 0x7 << 24;        // 480������
    ADC1->CR2 |= 0x1;
}

/*
********************************************************************************
*    Function: ai_adc_get_data
* Description: ��ȡָ��ͨ����ADC����ֵ
*       Input: ch - ͨ���ţ� 0~18
*      Output: None
*      Return: ת���Ľ��
*      Others: None
********************************************************************************
*/
u16 ai_adc_get_data(u8 ch)
{
    ADC1->SQR3 &= 0XFFFFFFE0;
    ADC1->SQR3 |= (u32)ch;
    ADC1->CR2 |= 0x1 << 30;                  // ��ʼת������ͨ��
    while ((ADC1->SR & (0x1 << 1)) == 0)
        ; /* Empty */
    
    return ADC1->DR;
}

/*
********************************************************************************
*    Function: ai_adc_get_average_val
* Description: ��ȡָ��ͨ����ADC����ֵ,ȡtimes��,Ȼ��ƽ�� 
*       Input:    ch - ͨ���ţ� 0~18
*              times - ��ȡ�Ĵ���
*      Output: None
*      Return: ͨ��ch��times��ת�����ƽ��ֵ
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
* Description: ��ȡ�ڲ��¶ȴ���������ֵ 
*       Input: void
*      Output: None
*      Return: �¶�ֵ(������100��,��λ:��.)
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
