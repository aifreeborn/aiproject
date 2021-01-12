#include <stm32f4xx.h>
#include "aitypes.h"
#include "aiiwdg.h"

/*
********************************************************************************
*    Function: 初始化独立看门狗
* Description: 喂狗时间间隔t = IWDG_RLR * ((4 * 2^IWDG_PR) / 32) (ms)
*       Input: prescaler - 预分频数值，有效位PR[2:0]，最大为256分频
*              reload    - 看门狗计数重载值，有效位RL[11:0]
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_iwdg_init(u8 prescaler, u16 reload)
{
    if (prescaler > 7 || reload > 0x0fff)
        return -1;
    
    IWDG->KR = 0x5555;
    IWDG->PR = prescaler;
    IWDG->RLR = reload;
    IWDG->KR = 0XAAAA;
    IWDG->KR = 0XCCCC;
    
    return 0;
}

void ai_iwdg_feed(void)
{
    IWDG->KR = 0XAAAA;
}
