#include <aisys.h>
#include "aitimer.h"
#include "ailed.h"

/*
********************************************************************************
*    Function: ͨ�ö�ʱ��3�жϳ�ʼ��
* Description: ��Ϊʱ�ӳ�ʼ��ʱ��APB1�ķ�Ƶ����Ϊ4������TIM3ʱ�Ӿ��ǣ�
*              2 * APB1ʱ�� = 90M,ʹ��Ĭ�ϵ����������ڲ�ʱ��
*              �ж�ʱ����㣺
*                          Tout = ((arr + 1) * (prescaler + 1)) / Tclk
*              ���У�
*                  Tout - TIM3�����ʱ��
*                  Tclk - TIM3������ʱ��Ƶ�ʣ�������Ϊ90MHz
*       Input: arr       - Ҫװ�ص�ʵ���Զ����ؼĴ�����ֵ
*              prescaler - Ԥ��Ƶ��ֵ��������ʱ��Ƶ�ʵ���
*                          f_ck_psc / (PSC[15:0] + 1)
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_timer3_interrupt_init(u16 arr, u16 prescaler)
{
    RCC->APB1ENR |= 0x1 << 1;
    
    TIM3->ARR = arr;
    TIM3->PSC = prescaler;
    TIM3->DIER |= 0x1 << 0;   // Update interrupt enable
    TIM3->CR1 |= 0x1 << 0;    // counter enable
    
    // ��ռ���ȼ�1�������ȼ�2�� ��2
    ai_nvic_init(1, 3, TIM3_IRQn, 2);
}

/*
********************************************************************************
*    Function: ��ʱ��3�жϷ������
* Description: ʹ�ö�ʱ������жϿ���DS1 LED������
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & 0x0001) {
        AI_DS1 = !AI_DS1;
    }
    
    TIM3->SR &= ~(0x1 << 0);
}

/*
********************************************************************************
*    Function: TIM3 PWM�����ʼ������
* Description: ʹ��LED0 - DS0 - PB1 - TIM3_CH4ͨ�������PWM����
*       Input: arr       - Ҫװ�ص�ʵ���Զ����ؼĴ�����ֵ
*              prescaler - Ԥ��Ƶ��ֵ��������ʱ��Ƶ�ʵ���
*                          f_ck_psc / (PSC[15:0] + 1)��f_ck_psc = 90MHz
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_timer3_pwm_init(u16 arr, u16 prescaler)
{
    RCC->APB1ENR |= 0x1 << 1;
    RCC->AHB1ENR |= 0x1 << 1;
    
    ai_gpio_set(GPIOB, PIN1, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set_af(GPIOB, 1, 2);  // ����AF2 - TIM3����
    TIM3->ARR = arr;
    TIM3->PSC = prescaler;
    TIM3->CCMR2 &= ~(0x7 << 12 | 0x1 << 11 | 0x3 << 8);
    TIM3->CCMR2 |= (0x6 << 12) | (0x1 << 11);   // PWMģʽ1��ʹ������Ƚ�Ԥװ��
    TIM3->CCER &= ~(0x1 << 15);
    TIM3->CCER |= 0x1 << 13 | 0x1 << 12;        // ���ʹ�ܣ��͵�ƽ��Ч
    TIM3->CR1 |= 0x1 << 7 | 0x1 << 0;           // ʹ��ARPE�ͼ�����TIM3
}

void ai_timer3_set_pwm(u32 val)
{
    TIM3->CCR4 = val;
}

/*
********************************************************************************
*    Function: TIM5 ͨ��1���벶���ʼ������
* Description: KEY_UP - WK_UP - PA0  : VCC3.3�ߵ�ƽ��ʾ���°���
*       Input: arr       - Ҫװ�ص�ʵ���Զ����ؼĴ�����ֵ
*              prescaler - Ԥ��Ƶ��ֵ��������ʱ��Ƶ�ʵ���
*                          f_ck_psc / (PSC[15:0] + 1)
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_timer5_ch1_capture_init(u32 arr, u16 prescaler)
{
    RCC->APB1ENR |= 0x1 << 3;
    RCC->AHB1ENR |= 0x1 << 0;
    ai_gpio_set(GPIOA, PIN0, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_PD);
    ai_gpio_set_af(GPIOA, 0, 2);
    
    TIM5->ARR = arr;
    TIM5->PSC = prescaler;
    TIM5->CCMR1 &= ~0x00ff;
    TIM5->CCMR1 |= 0x0 << 4 | 0x0 << 2 | 0x1 << 0;
    TIM5->CCER &= ~(0x1 << 3 | 0x1 << 1);
    TIM5->CCER |= 0x1;
    TIM5->EGR |= 0x1;     // ������Ʋ��������¼�����д��PSC��ֵ������Ч������
                          // ��Ҫ��ȴ���ʱ������Ż���Ч
    TIM5->DIER |= 0x1 << 1 | 0x1;
    TIM5->CR1 |= 0x1;
    
    ai_nvic_init(2, 0, TIM5_IRQn, 2);
}

/*
********************************************************************************
* +----------------------------------------------------------------+
* |               i_timer5_ch1_capture_stat                        |
* +----------------------------------------------------------------+
* |     bit[7]   |     bit[6]       |          bit[5:0]            |
* +--------------+------------------+------------------------------+
* | ������ɱ�־  | ���񵽸ߵ�ƽ��־   | ����ߵ�ƽ��ʱ������Ĵ���   |
* +----------------------------------------------------------------+
********************************************************************************
*/

u8 ai_timer5_ch1_capture_stat = 0;   // ���벶��״̬
u32 ai_timer5_ch1_capture_val = 0;

/*
********************************************************************************
*    Function: 
* Description: 
*       Input: 
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
void TIM5_IRQHandler(void)
{
    u16 status = TIM5->SR;
    
    if ((ai_timer5_ch1_capture_stat & 0x80) == 0) {
        if (status & 0x01) {
            if (ai_timer5_ch1_capture_stat & 0x40) {
                if ((ai_timer5_ch1_capture_stat & 0x3f) == 0x3f) {
                    ai_timer5_ch1_capture_stat |= 0x80;
                    ai_timer5_ch1_capture_val = 0xffffffff;
                } else {
                    ai_timer5_ch1_capture_stat++;
                }
            }
        }
        
        // ���������¼�
        if (status & 0x02) {
            // ����һ���½���
            if (ai_timer5_ch1_capture_stat & 0x40) {
                ai_timer5_ch1_capture_stat |= 0x80;
                ai_timer5_ch1_capture_val = TIM5->CCR1;
                TIM5->CCER &= ~(0x1 << 1);                  // ����Ϊ����������
            } else {
                ai_timer5_ch1_capture_stat = 0;
                ai_timer5_ch1_capture_val = 0;
                ai_timer5_ch1_capture_stat |= 0x40;
                TIM5->CR1 &= ~0x1;
                TIM5->CNT = 0;
                TIM5->CCER |= 0x1 << 1;   // ����Ϊ�½��ز���
                TIM5->CR1 |= 0x1;
            }
        }
    }
    
    TIM5->SR = 0;
}

/*
********************************************************************************
*    Function: ai_timer9_ch2_pwm_init
* Description: TIM9 CH2 PWM�����ʼ��
*       Input: arr       - Ҫװ�ص�ʵ���Զ����ؼĴ�����ֵ
*              prescaler - Ԥ��Ƶ��ֵ��������ʱ��Ƶ�ʵ���
*      Output: None
*      Return: void
*      Others: ����ʵ��PWM ADC����,ʹ��PA3�������ģ���ź�
********************************************************************************
*/
void ai_timer9_ch2_pwm_init(u16 arr, u16 prescaler)
{
    RCC->APB2ENR |= 0x1 << 16;        // TIM9 clock enabled
    RCC->AHB1ENR |= 0x1;
    ai_gpio_set(GPIOA, PIN3, GPIO_MODE_AF,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set_af(GPIOA, 3, 3);
    
    TIM9->ARR = arr;
    TIM9->PSC = prescaler;
    TIM9->CCMR1 &= ~((0x7 << 12) | (0x1 << 11));
    TIM9->CCMR1 |= 0x6 << 12 | 0x1 << 11;
    TIM9->CCER |= 0x1 << 4;                         // OC2���ʹ��
    TIM9->CR1 |= 0x1 << 7;                          // TIMx_ARR �Ĵ������л���
    TIM9->CR1 |= 0x1;
}

/*
********************************************************************************
*    Function: ai_timer9_pwm_dac_set
* Description: ���������ѹ
*       Input: vol - 0~330,����0~3.3V
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_timer9_pwm_dac_set(u16 value)
{
    double tmp = value;
    
    tmp /= 100;
    tmp = tmp * 255 / 3.3;
    TIM9->CCR2 = tmp;
}

u16 ai_timer9_pwm_dac_get(void)
{
    return TIM9->CCR2;
}
