/*
********************************************************************************
* 一共有四个按键，分别是：
*     KEY_UP - WK_UP - PA0  : VCC3.3高电平表示按下按键
*     KEY0   - KEY0  - PH3  : 低电平表示按下按键
*     KEY1   - KEY1  - PH2  : 低电平表示按下按键
*     KEY2   - KEY2  - PC13 : 低电平表示按下按键
* 注：
*     所有按键均没有外置上、下拉电阻，需要内部设置上、下电阻
********************************************************************************
*/
#include <aidelay.h>
#include "aikey.h"

/*
********************************************************************************
* 初始化
********************************************************************************
*/ 
void ai_key_init(void)
{
    RCC->AHB1ENR |= 0x1 << 0;        // 使能 IO 端口 A 时钟
    RCC->AHB1ENR |= 0x1 << 2;        // 使能 IO 端口 C 时钟
    RCC->AHB1ENR |= 0x1 << 7;        // 使能 IO 端口 H 时钟
    
    ai_gpio_set(GPIOA, PIN0, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PD);    // 下拉输入
    ai_gpio_set(GPIOC, PIN13, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PU);    // 上拉输入
    ai_gpio_set(GPIOH, PIN2 | PIN3, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PU);    // 上拉输入
}

/*
********************************************************************************
* 按键处理函数
* 参数mode:
*     0 - 不支持连续按键
*     1 - 支持连续按键
* 返回值：
*     ai_key_pressed_t中定义的宏分别表示按下
* 按键扫描优先级：
*     KEY0 > KEY1 > KEY2 > KEY_UP
********************************************************************************
*/ 
u8 ai_key_scan(u8 mode)
{
    static u8 key_up = 1;        // 按键按下、松开标志
    
    if (mode)
        key_up = 1;
    if (key_up && (AI_KEY0 == AI_LOW_LEVEL || AI_KEY1 == AI_LOW_LEVEL 
                   || AI_KEY2 == AI_LOW_LEVEL || AI_WK_UP == AI_HIGH_LEVEL)) {
        ai_delay_ms(10);         // 去抖动
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

