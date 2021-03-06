/*
********************************************************************************
* һ�����ĸ��������ֱ��ǣ�
*     KEY_UP - WK_UP - PA0  : VCC3.3�ߵ�ƽ��ʾ���°���
*     KEY0   - KEY0  - PH3  : �͵�ƽ��ʾ���°���
*     KEY1   - KEY1  - PH2  : �͵�ƽ��ʾ���°���
*     KEY2   - KEY2  - PC13 : �͵�ƽ��ʾ���°���
* ע��
*     ���а�����û�������ϡ��������裬��Ҫ�ڲ������ϡ��µ���
********************************************************************************
*/
#include <aidelay.h>
#include "aikey.h"

/*
********************************************************************************
* ��ʼ��
********************************************************************************
*/ 
void ai_key_init(void)
{
    RCC->AHB1ENR |= 0x1 << 0;        // ʹ�� IO �˿� A ʱ��
    RCC->AHB1ENR |= 0x1 << 2;        // ʹ�� IO �˿� C ʱ��
    RCC->AHB1ENR |= 0x1 << 7;        // ʹ�� IO �˿� H ʱ��
    
    ai_gpio_set(GPIOA, PIN0, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PD);    // ��������
    ai_gpio_set(GPIOC, PIN13, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PU);    // ��������
    ai_gpio_set(GPIOH, PIN2 | PIN3, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PU);    // ��������
}

/*
********************************************************************************
* ����������
* ����mode:
*     0 - ��֧����������
*     1 - ֧����������
* ����ֵ��
*     ai_key_pressed_t�ж���ĺ�ֱ��ʾ����
* ����ɨ�����ȼ���
*     KEY0 > KEY1 > KEY2 > KEY_UP
********************************************************************************
*/ 
u8 ai_key_scan(u8 mode)
{
    static u8 key_up = 1;        // �������¡��ɿ���־
    
    if (mode)
        key_up = 1;
    if (key_up && (AI_KEY0 == AI_LOW_LEVEL || AI_KEY1 == AI_LOW_LEVEL 
                   || AI_KEY2 == AI_LOW_LEVEL || AI_WK_UP == AI_HIGH_LEVEL)) {
        ai_delay_ms(10);         // ȥ����
        key_up = 0;
        if (AI_KEY0 == AI_LOW_LEVEL) {
            return AI_KEY0_DOWN;
        } else if (AI_KEY1 == AI_LOW_LEVEL) {
            return AI_KEY1_DOWN;
        } else if (AI_KEY2 == AI_LOW_LEVEL) {
            return AI_KEY2_DOWN;
        } else if (AI_WK_UP == AI_HIGH_LEVEL) {
            return AI_KEY_WK_DOWN;
        }
    } else if (AI_KEY0 == AI_HIGH_LEVEL && AI_KEY1 == AI_HIGH_LEVEL
               && AI_KEY2 == AI_HIGH_LEVEL && AI_WK_UP == AI_LOW_LEVEL) {
        key_up = 1;
    }
    return AI_KEY_ALL_UP;
}

