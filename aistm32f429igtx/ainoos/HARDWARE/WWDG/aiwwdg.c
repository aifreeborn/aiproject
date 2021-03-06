#include <stm32f4xx.h>
#include <stdio.h>
#include "aitypes.h"
#include "aisys.h"
#include "aiwwdg.h"
#include "ailed.h"

/*
********************************************************************************
*    Function: ��ʼ�����ڿ��Ź�
* Description: ��ʱֵ�ļ���
*              t_wwdg = t_pclk1 * 4096 * 2^WDGTB[1:0] * (t[5:0] + 1) (ms)
*              ���У�
*                   t_wwdg - WWDG��ʱ��
*                   t_pclk1 - APB1ʱ�����ڣ���msΪ������λ
*       Input: counter       - T[6:0],7-bits counter��MSB to LSB��
*              window_val    - W[6:0],����������ݼ����������бȽϵĴ���ֵ��
*              timer_base    - WDGTB[1:0],�������ü�����ʱ�ӵķ�Ƶֵ��
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_wwdg_init(u8 counter, u8 window_val, u8 timer_base)
{
    if (counter > 0x7f || window_val > 0x7f || timer_base > 3)
        return -1;
    
    RCC->APB1ENR |= 0x1 << 11;
    
    WWDG->CR &= ~0x7f;
    WWDG->CR |= counter & 0x7f;
    WWDG->CFR &= ~0x1ff;
    WWDG->CFR |= timer_base << 7;
    WWDG->CFR |= window_val & 0x7f;
    WWDG->CR |= 0x1 << 7;
    
    // ��ռ2�������ȼ�3����2
    ai_nvic_init(2, 3, WWDG_IRQn, 2);
    WWDG->SR &= ~0x1;
    WWDG->CFR |= 0x1 << 9;
    
    
    return 0;
}

/*
********************************************************************************
*    Function: ���贰�ڼ�����ֵ
* Description: 
*       Input: cnt - 7bits��������ֵ
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_wwdg_set_counter(u8 cnt)
{
    WWDG->CR = (cnt & 0x7f) | (0x1 << 7);
}

/*
********************************************************************************
*    Function: ���ڿ��Ź��жϷ������
* Description: 
*       Input: None
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void WWDG_IRQHandler(void)
{
    ai_wwdg_set_counter(0x7f);
    WWDG->SR &= ~0x1;
    AI_DS1 = !AI_DS1;
    printf("WWDG_IRQHandler done.\r\n");
}
