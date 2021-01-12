#include <aisys.h>
#include "aitimer.h"
#include "ailed.h"

/*
********************************************************************************
*    Function: 通用定时器3中断初始化
* Description: 因为时钟初始化时把APB1的分频设置为4，所以TIM3时钟就是：
*              2 * APB1时钟 = 90M,使用默认递增计数、内部时钟
               中断时间计算：
                           Tout = ((arr + 1) * (prescaler + 1)) / Tclk
               其中：
                   Tout - TIM3的溢出时间
                   Tclk - TIM3的输入时钟频率，在这里为90MHz
*       Input: arr       - 要装载到实际自动重载寄存器的值
*              prescaler - 预分频器值，计数器时钟频率等于
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
    
    // 抢占优先级1，子优先级2， 组2
    ai_nvic_init(1, 3, TIM3_IRQn, 2);
}

/*
********************************************************************************
*    Function: 定时器3中断服务程序
* Description: 使用定时器溢出中断控制DS1 LED的闪亮
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
