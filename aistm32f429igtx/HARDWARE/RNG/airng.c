#include <stdio.h>
#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "airng.h"

/*
********************************************************************************
*    Function: ai_rng_init
* Description: ��ʼ�������������(RNG)
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
* Description: ��ȡ�����
*       Input: void
*      Output: None
*      Return: ��ȡ���������ֵ
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
* Description: ��ȡĳ����Χ�ڵ������
*       Input: min - ��Сֵ
*              max - ���ֵ
*              ȡֵ��ΧΪ[min, max]
*      Output: None
*      Return: �õ��������(rval),����:min<=rval<=max
*      Others: None
********************************************************************************
*/
u32 ai_rng_get_random_range(u32 min, u32 max)
{
    return ai_rng_get_random_num() % (max - min + 1) + min;
}
