#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aisys.h"
#include "aigpio.h"
#include "aiwkup.h"
#include "ailed.h"
#include "aikey.h"

/*
********************************************************************************
*    Function: ai_wkup_standby
* Description: ϵͳ�������ģʽ
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
static void ai_wkup_standby(void)
{
    u32 tmp = 0;
    
    RCC->AHB1RSTR |= 0x01fe;        // ��λ��GPIOA���������IO��
    while (AI_WK_UP)                // �ȴ�WK_UP�����ɿ�
        ;                           // ����RTC�ж�ʱ,�����WK_UP�ɿ��ٽ������
    RCC->AHB1RSTR |= 0x1;           // ��λGPIOA
    
    RCC->APB1ENR |= 0x1 << 28;
    PWR->CR |= 0x1 << 8;            // ���������ʹ��(RTC+SRAM) 
    RTC->WPR=0xCA;
    RTC->WPR=0x53; 
	tmp = RTC->CR & (0x0f << 12);   // ��¼ԭ����RTC�ж�����
    RTC->CR &= ~(0x0f << 12);       // �ر�RTC�����ж�
    RTC->ISR &= ~(0x3f << 8);       // �������RTC�жϱ�־.
    PWR->CR |= 0x1 << 2;            // ���Wake-up ��־
    RTC->CR |= tmp;                 // ��������RTC�ж�,��Ϊ��������Ҫ�жϻ���
    RTC->WPR = 0xff;                // ʹ��RTC�Ĵ���д����
    ai_sys_standby();               // �������ģʽ
}

/*
********************************************************************************
*    Function: ai_wkup_monitor
* Description: ���WK_UP���ŵ��ź�
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,��ʾ�������°�����3s���ϣ�
*              On error, -1 is returned.��ʾ����Ĵ���
*      Others: None
********************************************************************************
*/
static int ai_wkup_monitor(void)
{
    u8 t = 0;
    u8 tx = 0;                          // ��¼�����ɿ��Ĵ���

    AI_DS0 = AI_LED_ON;
    while (1) {
        if (AI_WK_UP) {                 // WK_UP����
            t++;
            tx = 0;
        } else {
            tx++;
            if (tx > 3) {               // ����90ms��û��WKUP�ź�
                AI_DS0 = AI_LED_OFF;
                return -1;              // ����İ���,���´�������
            }
        }
        
        ai_delay_ms(30);
        if (t >= 100) {
            AI_DS0 = AI_LED_ON;
            return 0;                   // ����3s������
        }
    }
}

void EXTI0_IRQHandler(void)
{
    EXTI->PR = 0x1;    // ���LINE10�ϵ��жϱ�־λ 
    if (ai_wkup_monitor() == 0) {
        ai_wkup_standby();
    }    
}

/*
********************************************************************************
*    Function: ai_wkup_init
* Description: PA0 WKUP���ѳ�ʼ��
*       Input: void
*      Output: None
*      Return: void
*      Others: ����������Ҫ���°���3s���ϣ������ʾ�쳣����
********************************************************************************
*/
void ai_wkup_init(void)
{
    RCC->AHB1ENR |= 0x1;        // ��ΪWK_UP����ʹ�õ���PA0,����ʱ�Ǹߵ�ƽ
    ai_gpio_set(GPIOA, PIN0, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PD);
    // ����Ƿ�����������
    if (ai_wkup_monitor() != 0) {
        ai_wkup_standby();
    }
    
    ai_nvic_exti_cfg(GPIO_A, 0, RTIR);        // PA0�����ش���
    ai_nvic_init(2, 2, EXTI0_IRQn, 2);
}
