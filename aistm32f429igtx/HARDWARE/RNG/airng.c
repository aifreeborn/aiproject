#include <stdio.h>
#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "airng.h"

/*
********************************************************************************
*    Function: ai_rng_init
* Description: 初始化随机数发生器(RNG)
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_rng_init(void)
{
    u16 timeout = 0;
    
    RCC->AHB2ENR |= 0x1 << 6;
    RNG->CR |= 0x1 << 2;
    while ((RNG->SR & 0x1) == 0 && timeout < 10000) {
        timeout++;
        ai_delay_us(100);
    }
    
    if (timeout >= 10000)
        return -1;
    
    return 0;
}

/*
********************************************************************************
*    Function: ai_rng_get_random_num
* Description: 获取随机数
*       Input: void
*      Output: None
*      Return: 获取的随机数数值
*      Others: None
********************************************************************************
*/
u32 ai_rng_get_random_num(void)
{
    while ((RNG->SR & 0x1) == 0)
        ; /* Empty */
    
    return RNG->DR;
}

/*
********************************************************************************
*    Function: ai_rng_get_random_range
* Description: 获取某个范围内的随机数
*       Input: min - 最小值
*              max - 最大值
*              取值范围为[min, max]
*      Output: None
*      Return: 得到的随机数(rval),满足:min<=rval<=max
*      Others: None
********************************************************************************
*/
u32 ai_rng_get_random_range(u32 min, u32 max)
{
    return ai_rng_get_random_num() % (max - min + 1) + min;
}
