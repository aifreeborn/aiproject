#include <stm32f4xx.h>
#include <aitypes.h>
#include "ailed.h"

/*
********************************************************************************
* 一共有两颗LED，低电平点亮LED，高电平熄灭LED：
*     DS0 - LED0 - PB1
*     DS1 - LED1 - PB0
********************************************************************************
*/
void ai_led_init(void)
{
    RCC->AHB1ENR |= 0x1 << 1;     // 使能PORTB时钟
    ai_gpio_set(GPIOB, PIN0 | PIN1, 
                GPIO_MODE_OUT, GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    AI_LED0 = AI_LED_OFF;
    AI_LED1 = AI_LED_OFF;
}

void ai_led_on(u8 num)
{
    if (num == AI_LED_DS0) {
        AI_LED0 = AI_LED_ON;
    } else if (num == AI_LED_DS1){
        AI_LED1 = AI_LED_ON;
    }
}

void ai_led_off(u8 num)
{
    if (num == AI_LED_DS0) {
        AI_LED0 = AI_LED_OFF;
    } else if (num == AI_LED_DS1){
        AI_LED1 = AI_LED_OFF;
    }
}

