#include <stdio.h>
#include "stm32f4xx.h"
#include "aigpio.h"
#include "aisys.h"
#include "aidelay.h"
#include "aitouchpad.h"

/*
********************************************************************************
*                               Private define
********************************************************************************
*/
#define TOUCHPAD_ARR_VAL_MAX 0xffffffff

/*
********************************************************************************
*                               Private variables
********************************************************************************
*/
vu32 ai_touchpad_default_val = 0;

/*
********************************************************************************
*    Function: TIM2���벶���ʼ��
* Description: TOUCH_KEY -> STM_ADC -> PA5/TIM2_CH1/TIM2_ETR/TIM8_CH1N
*              ���ô�������ʵ�ֶ�DS1���������
*       Input: arr       - Ҫװ�ص�ʵ���Զ����ؼĴ�����ֵ
*              prescaler - Ԥ��Ƶ��ֵ��������ʱ��Ƶ�ʵ���
*                          f_ck_psc / (PSC[15:0] + 1) 
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
static void ai_tim2_ch1_capture_init(u32 arr, u16 prescaler)
{
    RCC->APB1ENR |= 0x1 << 0;    // TIM2EN
    RCC->AHB1ENR |= 0x1 << 0;    // PORTA ʹ��
    ai_gpio_set(GPIOA, PIN5, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_NONE);
    ai_gpio_set_af(GPIOA, 5, 1);
    
    TIM2->ARR = arr;
    TIM2->PSC = prescaler;
    TIM2->CCMR1 &= ~0x00ff;
    TIM2->CCMR1 |= 0x0 << 4 | 0x0 << 2 | 0x1 << 0;
    TIM2->CCER &= ~(0x1 << 3 | 0x1 << 1);
    TIM2->CCER |= 0x1;
    TIM2->EGR |= 0x1;     // ������Ʋ��������¼�����д��PSC��ֵ������Ч������
                          // ��Ҫ��ȴ���ʱ������Ż���Ч
    TIM2->DIER |= 0x1 << 1 | 0x1;
    TIM2->CR1 |= 0x1;
}

static void ai_touchpad_reset(void)
{
    ai_gpio_set(GPIOA, PIN5, GPIO_MODE_OUT, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_PD);
    PA_OUT(5) = 0;
    ai_delay_ms(5);
    TIM2->SR &= ~0x3;
    TIM2->CNT = 0;
    ai_gpio_set(GPIOA, PIN5, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_NONE);
}

static u32 ai_touchpad_get_val(void)
{
    ai_touchpad_reset();
    
    // �ȴ�CH1����������
    while ((TIM2->SR & 0x02) == 0) {
        if (TIM2->CNT > TOUCHPAD_ARR_VAL_MAX - 500)
            return TIM2->CNT;
    }
    
    return TIM2->CCR1;
}
    
/*
********************************************************************************
*    Function: ����������ʼ��
* Description: ȡ��û�а��°���ʱ����������ȡֵ
*       Input: prescaler - ��Ƶϵ����ȡֵԽС������Խ��
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_touchpad_init(u8 prescaler)
{
    u8 i, j;
    u32 buf[10];
    unsigned long long temp;
    
    // ����TIM2�ķ�Ƶϵ��
    ai_tim2_ch1_capture_init(TOUCHPAD_ARR_VAL_MAX, prescaler - 1);
    for (i = 0; i < 10; i++) {
        buf[i] = ai_touchpad_get_val();
        ai_delay_ms(10);
    }
    
    for (i = 0; i < 9; i++) {
        for (j = i + 1; j < 10; j++) {
            if (buf[i] > buf[j]) {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    temp = 0;
    for (i = 1; i < 9; i++)
        temp += buf[i];
    ai_touchpad_default_val = temp / 8;
    printf("ai_touchpad_default_val: %d\r\n", ai_touchpad_default_val);
    if (ai_touchpad_default_val > TOUCHPAD_ARR_VAL_MAX / 2)
        return -1;
    return 0;
}

static u32 ai_touchpad_get_max_val(u8 n)
{
    u32 temp;
    u32 ret = 0;
    u8 ok_cnt = 0;
    u8 valid_cnt = n * 2 / 3;
    
    while (n--) {
        temp = ai_touchpad_get_val();
        if (temp > (ai_touchpad_default_val * 5 / 4))
            ok_cnt++;
        if (temp > ret)
            ret = temp;
    }
    
    if (ok_cnt >= valid_cnt)
        return ret;
    return 0;
}

/*
********************************************************************************
*    Function: ��������ɨ�躯��
* Description: 
*       Input: mode - 0,��֧����������(����һ�α����ɿ����ܰ���һ��)
*                     1,��������(����һֱ����)
*      Output: None
*      Return: ����ʱ����0��û�а���ʱ����-1
*      Others: None
********************************************************************************
*/
int ai_touchpad_scan(u8 mode)
{
    static u8 enable = 0;    // 0 - ���Կ�ʼ��⣬��0 - ���ܿ�ʼ���
    
    int ret = -1;
    u8 sample = 3;
    u32 key_val = 0;
    
    if (mode) {
        sample = 6;
        enable = 0;    // ֧��������
    }
    key_val = ai_touchpad_get_max_val(sample);
    // ��������ֵ����1.3��С��10��ai_touchpad_default_valʱΪ��Чֵ
    if (key_val > (ai_touchpad_default_val * 4 / 3)
        && key_val < (10 * ai_touchpad_default_val)) {
        if (enable == 0)
            ret = 0;
        printf("%s: %d\r\n", __func__, key_val);
        enable = 3;    // ����Ҫ�ٹ�3���Ժ���ܰ�����Ч
    }
        
    if (enable)
        enable--;
    return ret;
}

