#include <aisys.h>
#include "aitimer.h"
#include "ailed.h"

/*
********************************************************************************
*    Function: 通用定时器3中断初始化
* Description: 因为时钟初始化时把APB1的分频设置为4，所以TIM3时钟就是：
*              2 * APB1时钟 = 90M,使用默认递增计数、内部时钟
*              中断时间计算：
*                          Tout = ((arr + 1) * (prescaler + 1)) / Tclk
*              其中：
*                  Tout - TIM3的溢出时间
*                  Tclk - TIM3的输入时钟频率，在这里为90MHz
*       Input: arr       - 要装载到实际自动重载寄存器的值
*              prescaler - 预分频器值，计数器时钟频率等于
*                          f_ck_psc / (PSC[15:0] + 1)
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

/*
********************************************************************************
*    Function: TIM3 PWM输出初始化函数
* Description: 使用LED0 - DS0 - PB1 - TIM3_CH4通道来输出PWM波形
*       Input: arr       - 要装载到实际自动重载寄存器的值
*              prescaler - 预分频器值，计数器时钟频率等于
*                          f_ck_psc / (PSC[15:0] + 1)，f_ck_psc = 90MHz
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_timer3_pwm_init(u16 arr, u16 prescaler)
{
    RCC->APB1ENR |= 0x1 << 1;
    RCC->AHB1ENR |= 0x1 << 1;
    
    ai_gpio_set(GPIOB, PIN1, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set_af(GPIOB, 1, 2);  // 设置AF2 - TIM3功能
    TIM3->ARR = arr;
    TIM3->PSC = prescaler;
    TIM3->CCMR2 &= ~(0x7 << 12 | 0x1 << 11 | 0x3 << 8);
    TIM3->CCMR2 |= (0x6 << 12) | (0x1 << 11);   // PWM模式1，使能输出比较预装载
    TIM3->CCER &= ~(0x1 << 15);
    TIM3->CCER |= 0x1 << 13 | 0x1 << 12;        // 输出使能，低电平有效
    TIM3->CR1 |= 0x1 << 7 | 0x1 << 0;           // 使能ARPE和计数器TIM3
}

void ai_timer3_set_pwm(u32 val)
{
    TIM3->CCR4 = val;
}

/*
********************************************************************************
*    Function: TIM5 通道1输入捕获初始化函数
* Description: KEY_UP - WK_UP - PA0  : VCC3.3高电平表示按下按键
*       Input: arr       - 要装载到实际自动重载寄存器的值
*              prescaler - 预分频器值，计数器时钟频率等于
*                          f_ck_psc / (PSC[15:0] + 1)
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_timer5_ch1_capture_init(u32 arr, u16 prescaler)
{
    RCC->APB1ENR |= 0x1 << 3;
    RCC->AHB1ENR |= 0x1 << 0;
    ai_gpio_set(GPIOA, PIN0, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_PD);
    ai_gpio_set_af(GPIOA, 0, 2);
    
    TIM5->ARR = arr;
    TIM5->PSC = prescaler;
    TIM5->CCMR1 &= ~0x00ff;
    TIM5->CCMR1 |= 0x0 << 4 | 0x0 << 2 | 0x1 << 0;
    TIM5->CCER &= ~(0x1 << 3 | 0x1 << 1);
    TIM5->CCER |= 0x1;
    TIM5->EGR |= 0x1;     // 软件控制产生更新事件，是写入PSC的值立即生效，否则，
                          // 将要会等待定时器溢出才会生效
    TIM5->DIER |= 0x1 << 1 | 0x1;
    TIM5->CR1 |= 0x1;
    
    ai_nvic_init(2, 0, TIM5_IRQn, 2);
}

/*
********************************************************************************
* +----------------------------------------------------------------+
* |               i_timer5_ch1_capture_stat                        |
* +----------------------------------------------------------------+
* |     bit[7]   |     bit[6]       |          bit[5:0]            |
* +--------------+------------------+------------------------------+
* | 捕获完成标志  | 捕获到高电平标志   | 捕获高电平后定时器溢出的次数   |
* +----------------------------------------------------------------+
********************************************************************************
*/

u8 ai_timer5_ch1_capture_stat = 0;   // 输入捕获状态
u32 ai_timer5_ch1_capture_val = 0;

/*
********************************************************************************
*    Function: 
* Description: 
*       Input: 
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
void TIM5_IRQHandler(void)
{
    u16 status = TIM5->SR;
    
    if ((ai_timer5_ch1_capture_stat & 0x80) == 0) {
        if (status & 0x01) {
            if (ai_timer5_ch1_capture_stat & 0x40) {
                if ((ai_timer5_ch1_capture_stat & 0x3f) == 0x3f) {
                    ai_timer5_ch1_capture_stat |= 0x80;
                    ai_timer5_ch1_capture_val = 0xffffffff;
                } else {
                    ai_timer5_ch1_capture_stat++;
                }
            }
        }
        
        // 发生捕获事件
        if (status & 0x02) {
            // 捕获到一个下降沿
            if (ai_timer5_ch1_capture_stat & 0x40) {
                ai_timer5_ch1_capture_stat |= 0x80;
                ai_timer5_ch1_capture_val = TIM5->CCR1;
                TIM5->CCER &= ~(0x1 << 1);                  // 设置为捕获上升沿
            } else {
                ai_timer5_ch1_capture_stat = 0;
                ai_timer5_ch1_capture_val = 0;
                ai_timer5_ch1_capture_stat |= 0x40;
                TIM5->CR1 &= ~0x1;
                TIM5->CNT = 0;
                TIM5->CCER |= 0x1 << 1;   // 设置为下降沿捕获
                TIM5->CR1 |= 0x1;
            }
        }
    }
    
    TIM5->SR = 0;
}

/*
********************************************************************************
*    Function: ai_timer9_ch2_pwm_init
* Description: TIM9 CH2 PWM输出初始化
*       Input: arr       - 要装载到实际自动重载寄存器的值
*              prescaler - 预分频器值，计数器时钟频率等于
*      Output: None
*      Return: void
*      Others: 用于实现PWM ADC功能,使用PA3引脚输出模拟信号
********************************************************************************
*/
void ai_timer9_ch2_pwm_init(u16 arr, u16 prescaler)
{
    RCC->APB2ENR |= 0x1 << 16;        // TIM9 clock enabled
    RCC->AHB1ENR |= 0x1;
    ai_gpio_set(GPIOA, PIN3, GPIO_MODE_AF,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set_af(GPIOA, 3, 3);
    
    TIM9->ARR = arr;
    TIM9->PSC = prescaler;
    TIM9->CCMR1 &= ~((0x7 << 12) | (0x1 << 11));
    TIM9->CCMR1 |= 0x6 << 12 | 0x1 << 11;
    TIM9->CCER |= 0x1 << 4;                         // OC2输出使能
    TIM9->CR1 |= 0x1 << 7;                          // TIMx_ARR 寄存器进行缓冲
    TIM9->CR1 |= 0x1;
}

/*
********************************************************************************
*    Function: ai_timer9_pwm_dac_set
* Description: 设置输出电压
*       Input: vol - 0~330,代表0~3.3V
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_timer9_pwm_dac_set(u16 value)
{
    double tmp = value;
    
    tmp /= 100;
    tmp = tmp * 255 / 3.3;
    TIM9->CCR2 = tmp;
}

u16 ai_timer9_pwm_dac_get(void)
{
    return TIM9->CCR2;
}
