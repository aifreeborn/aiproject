#include <stm32f4xx.h>
#include <stdio.h>
#include "aitypes.h"
#include "aisys.h"
#include "aiwwdg.h"
#include "ailed.h"

/*
********************************************************************************
*    Function: 初始化窗口看门狗
* Description: 超时值的计算
*              t_wwdg = t_pclk1 * 4096 * 2^WDGTB[1:0] * (t[5:0] + 1) (ms)
*              其中：
*                   t_wwdg - WWDG超时；
*                   t_pclk1 - APB1时钟周期，以ms为测量单位
*       Input: counter       - T[6:0],7-bits counter（MSB to LSB）
*              window_val    - W[6:0],包含用于与递减计数器进行比较的窗口值；
*              timer_base    - WDGTB[1:0],用于设置计数器时钟的分频值；
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
    
    // 抢占2，子优先级3，组2
    ai_nvic_init(2, 3, WWDG_IRQn, 2);
    WWDG->SR &= ~0x1;
    WWDG->CFR |= 0x1 << 9;
    
    
    return 0;
}

/*
********************************************************************************
*    Function: 重设窗口计数器值
* Description: 
*       Input: cnt - 7bits计数器数值
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
*    Function: 窗口看门狗中断服务程序
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
