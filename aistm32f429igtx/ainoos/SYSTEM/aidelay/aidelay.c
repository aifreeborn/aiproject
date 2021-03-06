#include <stm32f4xx.h> 
#include "aidelay.h"

static u8  factor_10us = 0;                // us延时倍乘数			   
static u16   factor_ms = 0;                // ms延时倍乘数
			   
/*
********************************************************************************
* 初始化延迟函数
* SYSTICK的时钟固定为AHB时钟的1/8, 24-bit system timer
* sysclk:系统时钟频率,单位为MHz
********************************************************************************
*/
void ai_delay_init(u8 sysclk)
{
 	SysTick->CTRL &= ~(0x1 << 2);            // 设置SysTick使用外部时钟源	 
	
    factor_10us = (sysclk * 10) / 8;
	factor_ms = ((u32)sysclk * 1000) / 8;    // 每个ms需要的systick时钟数   
}								    

/*
********************************************************************************
* 延时nus
* nus为要延时的us数.
* 注意:nus的值,不要大于745654us
********************************************************************************
*/
void ai_delay_us(u32 nus)
{
    u32 temp;
    
    SysTick->LOAD = (nus * factor_10us) / 10; 				// 时间加载	  		 
	SysTick->VAL  = 0x00;        				            // 清空计数器
	SysTick->CTRL = 0x01 ;      				            // 开始倒数 	 
	do {
        temp = SysTick->CTRL;
	} while((temp & 0x01) && !(temp & (1 << 16)));	        // 等待时间到达   
	SysTick->CTRL = 0x00;      	 			                // 关闭计数器
	SysTick->VAL  = 0X00;       				            // 清空计数器 
}

/*
********************************************************************************
* 延时nms
* SysTick->LOAD为24位寄存器,所以,最大延时为:745ms 
********************************************************************************
*/
static void delay_xms(u16 nms)
{		  	  
	u32 temp;
    
    SysTick->LOAD = (u32)nms * factor_ms;
    SysTick->VAL  = 0x00;
    SysTick->CTRL = 0x01;
    do {
        temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp & (0x1 << 16)));
    SysTick->CTRL=0x00;
    SysTick->VAL =0X00;
} 

void ai_delay_ms(u16 nms)
{
    // 这里用540是考虑到可能超频使用
    // 比如超频到248M的时候,delay_xms最大只能延时541ms左右了
	u8  repeat = nms / 540;
    u16 remain = nms % 540;
    
	while (repeat) {
		delay_xms(540);
		repeat--;
	}
	if (remain) delay_xms(remain);
}
			 
