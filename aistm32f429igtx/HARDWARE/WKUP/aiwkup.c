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
* Description: 系统进入待机模式
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
static void ai_wkup_standby(void)
{
    u32 tmp = 0;
    
    RCC->AHB1RSTR |= 0x01fe;        // 复位除GPIOA以外的所有IO口
    while (AI_WK_UP)                // 等待WK_UP按键松开
        ;                           // 在有RTC中断时,必须等WK_UP松开再进入待机
    RCC->AHB1RSTR |= 0x1;           // 复位GPIOA
    
    RCC->APB1ENR |= 0x1 << 28;
    PWR->CR |= 0x1 << 8;            // 后备区域访问使能(RTC+SRAM) 
    RTC->WPR=0xCA;
    RTC->WPR=0x53; 
	tmp = RTC->CR & (0x0f << 12);   // 记录原来的RTC中断设置
    RTC->CR &= ~(0x0f << 12);       // 关闭RTC所有中断
    RTC->ISR &= ~(0x3f << 8);       // 清除所有RTC中断标志.
    PWR->CR |= 0x1 << 2;            // 清除Wake-up 标志
    RTC->CR |= tmp;                 // 重新设置RTC中断,因为待机后还需要中断唤醒
    RTC->WPR = 0xff;                // 使能RTC寄存器写保护
    ai_sys_standby();               // 进入待机模式
}

/*
********************************************************************************
*    Function: ai_wkup_monitor
* Description: 检测WK_UP引脚的信号
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,表示连续按下按键达3s以上；
*              On error, -1 is returned.表示错误的触发
*      Others: None
********************************************************************************
*/
static int ai_wkup_monitor(void)
{
    u8 t = 0;
    u8 tx = 0;                          // 记录按键松开的次数

    AI_DS0 = AI_LED_ON;
    while (1) {
        if (AI_WK_UP) {                 // WK_UP按下
            t++;
            tx = 0;
        } else {
            tx++;
            if (tx > 3) {               // 超过90ms内没有WKUP信号
                AI_DS0 = AI_LED_OFF;
                return -1;              // 错误的按键,按下次数不够
            }
        }
        
        ai_delay_ms(30);
        if (t >= 100) {
            AI_DS0 = AI_LED_ON;
            return 0;                   // 按下3s以上了
        }
    }
}

void EXTI0_IRQHandler(void)
{
    EXTI->PR = 0x1;    // 清除LINE10上的中断标志位 
    if (ai_wkup_monitor() == 0) {
        ai_wkup_standby();
    }    
}

/*
********************************************************************************
*    Function: ai_wkup_init
* Description: PA0 WKUP唤醒初始化
*       Input: void
*      Output: None
*      Return: void
*      Others: 正常开机需要按下按键3s以上，否则表示异常出发
********************************************************************************
*/
void ai_wkup_init(void)
{
    RCC->AHB1ENR |= 0x1;        // 因为WK_UP按键使用的是PA0,按下时是高电平
    ai_gpio_set(GPIOA, PIN0, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PD);
    // 检查是否是正常开机
    if (ai_wkup_monitor() != 0) {
        ai_wkup_standby();
    }
    
    ai_nvic_exti_cfg(GPIO_A, 0, RTIR);        // PA0上升沿触发
    ai_nvic_init(2, 2, EXTI0_IRQn, 2);
}
