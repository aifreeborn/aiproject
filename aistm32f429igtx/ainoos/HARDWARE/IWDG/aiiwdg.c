#include <stm32f4xx.h>
#include "aitypes.h"
#include "aiiwdg.h"

/*
********************************************************************************
*    Function: ��ʼ���������Ź�
* Description: ι��ʱ����t = IWDG_RLR * ((4 * 2^IWDG_PR) / 32) (ms)
*       Input: prescaler - Ԥ��Ƶ��ֵ����ЧλPR[2:0]�����Ϊ256��Ƶ
*              reload    - ���Ź���������ֵ����ЧλRL[11:0]
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
