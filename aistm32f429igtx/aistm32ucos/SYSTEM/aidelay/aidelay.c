#include <stm32f4xx.h> 
#include "aidelay.h"
#include "aisys.h"

#if AI_SYS_SUPPORT_UCOS
#include "includes.h"
#endif

static u8  factor_10us = 0;                // us��ʱ������			   
static u16   factor_ms = 0;                // ms��ʱ������

/*
********************************************************************************
* ��ai_delay_us/ai_delay_ms��Ҫ֧��OS��ʱ����Ҫ������OS��صĺ궨��ͺ�����֧��
* ������3���궨��:
*         ai_delay_osrunning: ���ڱ�ʾOS��ǰ�Ƿ���������,�Ծ����Ƿ����ʹ����غ���
*     ai_delay_ostickspersec: ���ڱ�ʾOS�趨��ʱ�ӽ���,ai_delay_init�������������
*                             ����ʼ��systick
*      ai_delay_osintnesting: ���ڱ�ʾOS�ж�Ƕ�׼���,��Ϊ�ж����治���Ե���,
*                             ai_delay_msʹ�øò����������������
* Ȼ����3������:
*       ai_delay_osschedlock: ��������OS�������,��ֹ����
*     ai_delay_osschedunlock: ���ڽ���OS�������,���¿�������
*         ai_delay_ostimedly: ����OS��ʱ,���������������.
********************************************************************************
*/
#ifdef AI_SYS_SUPPORT_UCOS
//֧��UCOSII
#ifdef 	OS_CRITICAL_METHOD
// Flag indicating that kernel is running,0,������;1,������
#define    ai_delay_osrunning            OSRunning
// OSʱ�ӽ���,��ÿ����ȴ���
#define    ai_delay_ostickspersec        OS_TICKS_PER_SEC
// �ж�Ƕ�׼���,���ж�Ƕ�״���
#define    ai_delay_osintnesting         OSIntNesting
#endif /* ifdef OS_CRITICAL_METHOD */

//֧��UCOSIII
#ifdef 	CPU_CFG_CRITICAL_METHOD
// Flag indicating that kernel is running,0,������;1,������
#define    ai_delay_osrunning            OSRunning
// OSʱ�ӽ���,��ÿ����ȴ���
#define    ai_delay_ostickspersec        OSCfg_TickRate_Hz
// �ж�Ƕ�׼���,���ж�Ƕ�״���
#define    ai_delay_osintnesting         OSIntNestingCtr
#endif /* ifdef CPU_CFG_CRITICAL_METHOD */

// us����ʱʱ,�ر��������(��ֹ���us���ӳ�)
void ai_delay_osschedlock(void)
{
#ifdef 	CPU_CFG_CRITICAL_METHOD    // ʹ��UCOSIII
    OS_ERR err;
    OSSchedLock(&err);             // UCOSIII�ķ�ʽ,��ֹ���ȣ���ֹ���us��ʱ
#else
    OSSchedLock();
#endif
}

void ai_delay_osschedunlock(void)
{
#ifdef 	CPU_CFG_CRITICAL_METHOD    // ʹ��UCOSIII
    OS_ERR err;
    OSSchedUnlock(&err);             // UCOSIII�ķ�ʽ,�ָ�����
#else
    OSSchedUnlock();
#endif
}

/*
********************************************************************************
*    Function: ai_delay_ostimedly
* Description: ����OS�Դ�����ʱ������ʱ
*       Input: ticks - ��ʱ�Ľ�����
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_delay_ostimedly(u32 ticks)
{
#ifdef 	CPU_CFG_CRITICAL_METHOD    // ʹ��UCOSIII
    OS_ERR err;
    OSTimeDly(ticks, OS_OPT_TIME_PERIODIC, &err);    // UCOSIII��ʱ��������ģʽ
#else
    OSTimeDly(ticks);
#endif    
}

// systick�жϷ�����,ʹ��OSʱ�õ�
void SysTick_Handler(void)
{
    if (ai_delay_osrunning == 1) {        // OS��ʼ����,��ִ�������ĵ��ȴ���
        OSIntEnter();                     // �����ж�
        OSTimeTick();                     // ����ucos��ʱ�ӷ������
        OSIntExit();                      // ���������л����ж�
    }
}
#endif /* ifdef AI_SYS_SUPPORT_UCOS */

/*
********************************************************************************
* ��ʼ���ӳٺ���
* SYSTICK��ʱ�ӹ̶�ΪAHBʱ�ӵ�1/8, 24-bit system timer
* sysclk:ϵͳʱ��Ƶ��,��λΪMHz
********************************************************************************
*/
void ai_delay_init(u8 sysclk)
{
#ifdef AI_SYS_SUPPORT_UCOS
    u32 reload = 0;
#endif
    
 	SysTick->CTRL &= ~(0x1 << 2);            // ����SysTickʹ���ⲿʱ��Դ	 
	
    factor_10us = (sysclk * 10) / 8;
#ifdef AI_SYS_SUPPORT_UCOS
    reload = (sysclk * 10) / 8;
    // ����delay_ostickspersec�趨���ʱ��, reloadΪ24λ�Ĵ���,���ֵ:16777216,
    // ��180M��,Լ��0.699s����
    reload *= (1000000 / ai_delay_ostickspersec);
    factor_ms = 1000 / ai_delay_ostickspersec;    // ����OS������ʱ�����ٵ�λ
    SysTick->CTRL |= 0x1 << 1;                    // ����SYSTICK�ж�
    SysTick->LOAD = reload / 10;            // ÿ1/delay_ostickspersec���ж�һ��
    SysTick->CTRL |= 0x1;                   // ����SYSTICK
#else
	factor_ms = ((u32)sysclk * 1000) / 8;    // ÿ��ms��Ҫ��systickʱ���� 
#endif    
}								    

#ifdef AI_SYS_SUPPORT_UCOS
/*
********************************************************************************
* ��ʱnus
* nusΪҪ��ʱ��us��.
* ע��:nus��ֵ,��Ҫ����745654us
********************************************************************************
*/
void ai_delay_us(u32 nus)
{
    u32 ticks;
    u32 told, tnow, tcnt = 0;
    u32 reload = SysTick->LOAD;
    
    ticks = nus * factor_10us / 10;
    ai_delay_osschedlock();
    told = SysTick->VAL;
    while (1) {
        tnow = SysTick->VAL;
        if (told != tnow) {
            // SYSTICK��һ���ݼ��ļ�����
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += reload - tnow + told;
            told = tnow;
            if (tcnt >= ticks)
                break;
        }
    }
    ai_delay_osschedunlock();
}

/*
********************************************************************************
* ��ʱnms
* SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:745ms 
********************************************************************************
*/
void ai_delay_ms(u16 nms)
{
    // ���OS�Ѿ�������,���Ҳ������ж�����(�ж����治���������)
    if (ai_delay_osrunning && ai_delay_osintnesting == 0) {
        if (nms >= factor_ms) {
            ai_delay_ostimedly(nms / factor_ms);
        }
        nms %= factor_ms;
    }
    // OS�Ѿ��޷��ṩ��ôС����ʱ��,������ͨ��ʽ��ʱ
    ai_delay_us((u32)(nms * 1000));
}
#else
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
#endif /* #ifdef AI_SYS_SUPPORT_UCOS */			 
