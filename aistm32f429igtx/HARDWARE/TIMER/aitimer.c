#include <aisys.h>
#include "aitimer.h"
#include "ailed.h"

/*
********************************************************************************
*    Function: ͨ�ö�ʱ��3�жϳ�ʼ��
* Description: ��Ϊʱ�ӳ�ʼ��ʱ��APB1�ķ�Ƶ����Ϊ4������TIM3ʱ�Ӿ��ǣ�
*              2 * APB1ʱ�� = 90M,ʹ��Ĭ�ϵ����������ڲ�ʱ��
               �ж�ʱ����㣺
                           Tout = ((arr + 1) * (prescaler + 1)) / Tclk
               ���У�
                   Tout - TIM3�����ʱ��
                   Tclk - TIM3������ʱ��Ƶ�ʣ�������Ϊ90MHz
*       Input: arr       - Ҫװ�ص�ʵ���Զ����ؼĴ�����ֵ
*              prescaler - Ԥ��Ƶ��ֵ��������ʱ��Ƶ�ʵ���
                           f_ck_psc / (PSC[15:0] + 1)
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
