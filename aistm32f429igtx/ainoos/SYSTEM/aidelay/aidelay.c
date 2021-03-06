#include <stm32f4xx.h> 
#include "aidelay.h"

static u8  factor_10us = 0;                // us��ʱ������			   
static u16   factor_ms = 0;                // ms��ʱ������
			   
/*
********************************************************************************
* ��ʼ���ӳٺ���
* SYSTICK��ʱ�ӹ̶�ΪAHBʱ�ӵ�1/8, 24-bit system timer
* sysclk:ϵͳʱ��Ƶ��,��λΪMHz
********************************************************************************
*/
void ai_delay_init(u8 sysclk)
{
 	SysTick->CTRL &= ~(0x1 << 2);            // ����SysTickʹ���ⲿʱ��Դ	 
	
    factor_10us = (sysclk * 10) / 8;
	factor_ms = ((u32)sysclk * 1000) / 8;    // ÿ��ms��Ҫ��systickʱ����   
}								    

/*
********************************************************************************
* ��ʱnus
* nusΪҪ��ʱ��us��.
* ע��:nus��ֵ,��Ҫ����745654us
********************************************************************************
*/
void ai_delay_us(u32 nus)
{
    u32 temp;
    
    SysTick->LOAD = (nus * factor_10us) / 10; 				// ʱ�����	  		 
	SysTick->VAL  = 0x00;        				            // ��ռ�����
	SysTick->CTRL = 0x01 ;      				            // ��ʼ���� 	 
	do {
        temp = SysTick->CTRL;
	} while((temp & 0x01) && !(temp & (1 << 16)));	        // �ȴ�ʱ�䵽��   
	SysTick->CTRL = 0x00;      	 			                // �رռ�����
	SysTick->VAL  = 0X00;       				            // ��ռ����� 
}

/*
********************************************************************************
* ��ʱnms
* SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:745ms 
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
    // ������540�ǿ��ǵ����ܳ�Ƶʹ��
    // ���糬Ƶ��248M��ʱ��,delay_xms���ֻ����ʱ541ms������
	u8  repeat = nms / 540;
    u16 remain = nms % 540;
    
	while (repeat) {
		delay_xms(540);
		repeat--;
	}
	if (remain) delay_xms(remain);
}
			 
