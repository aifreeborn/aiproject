#include <stm32f4xx.h> 
#include "aidelay.h"
#include "aisys.h"

#if AI_SYS_SUPPORT_UCOS
#include "includes.h"
#endif

static u8  factor_10us = 0;                // us延时倍乘数			   
static u16   factor_ms = 0;                // ms延时倍乘数

/*
********************************************************************************
* 当ai_delay_us/ai_delay_ms需要支持OS的时候需要三个与OS相关的宏定义和函数来支持
* 首先是3个宏定义:
*         ai_delay_osrunning: 用于表示OS当前是否正在运行,以决定是否可以使用相关函数
*     ai_delay_ostickspersec: 用于表示OS设定的时钟节拍,ai_delay_init将根据这个参数
*                             来初始化systick
*      ai_delay_osintnesting: 用于表示OS中断嵌套级别,因为中断里面不可以调度,
*                             ai_delay_ms使用该参数来决定如何运行
* 然后是3个函数:
*       ai_delay_osschedlock: 用于锁定OS任务调度,禁止调度
*     ai_delay_osschedunlock: 用于解锁OS任务调度,重新开启调度
*         ai_delay_ostimedly: 用于OS延时,可以引起任务调度.
********************************************************************************
*/
#ifdef AI_SYS_SUPPORT_UCOS
//支持UCOSII
#ifdef 	OS_CRITICAL_METHOD
// Flag indicating that kernel is running,0,不运行;1,在运行
#define    ai_delay_osrunning            OSRunning
// OS时钟节拍,即每秒调度次数
#define    ai_delay_ostickspersec        OS_TICKS_PER_SEC
// 中断嵌套级别,即中断嵌套次数
#define    ai_delay_osintnesting         OSIntNesting
#endif /* ifdef OS_CRITICAL_METHOD */

//支持UCOSIII
#ifdef 	CPU_CFG_CRITICAL_METHOD
// Flag indicating that kernel is running,0,不运行;1,在运行
#define    ai_delay_osrunning            OSRunning
// OS时钟节拍,即每秒调度次数
#define    ai_delay_ostickspersec        OSCfg_TickRate_Hz
// 中断嵌套级别,即中断嵌套次数
#define    ai_delay_osintnesting         OSIntNestingCtr
#endif /* ifdef CPU_CFG_CRITICAL_METHOD */

// us级延时时,关闭任务调度(防止打断us级延迟)
void ai_delay_osschedlock(void)
{
#ifdef 	CPU_CFG_CRITICAL_METHOD    // 使用UCOSIII
    OS_ERR err;
    OSSchedLock(&err);             // UCOSIII的方式,禁止调度，防止打断us延时
#else
    OSSchedLock();
#endif
}

void ai_delay_osschedunlock(void)
{
#ifdef 	CPU_CFG_CRITICAL_METHOD    // 使用UCOSIII
    OS_ERR err;
    OSSchedUnlock(&err);             // UCOSIII的方式,恢复调度
#else
    OSSchedUnlock();
#endif
}

/*
********************************************************************************
*    Function: ai_delay_ostimedly
* Description: 调用OS自带的延时函数延时
*       Input: ticks - 延时的节拍数
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_delay_ostimedly(u32 ticks)
{
#ifdef 	CPU_CFG_CRITICAL_METHOD    // 使用UCOSIII
    OS_ERR err;
    OSTimeDly(ticks, OS_OPT_TIME_PERIODIC, &err);    // UCOSIII延时采用周期模式
#else
    OSTimeDly(ticks);
#endif    
}

// systick中断服务函数,使用OS时用到
void SysTick_Handler(void)
{
    if (ai_delay_osrunning == 1) {        // OS开始跑了,才执行正常的调度处理
        OSIntEnter();                     // 进入中断
        OSTimeTick();                     // 调用ucos的时钟服务程序
        OSIntExit();                      // 触发任务切换软中断
    }
}
#endif /* ifdef AI_SYS_SUPPORT_UCOS */

/*
********************************************************************************
* 初始化延迟函数
* SYSTICK的时钟固定为AHB时钟的1/8, 24-bit system timer
* sysclk:系统时钟频率,单位为MHz
********************************************************************************
*/
void ai_delay_init(u8 sysclk)
{
#ifdef AI_SYS_SUPPORT_UCOS
    u32 reload = 0;
#endif
    
 	SysTick->CTRL &= ~(0x1 << 2);            // 设置SysTick使用外部时钟源	 
	
    factor_10us = (sysclk * 10) / 8;
#ifdef AI_SYS_SUPPORT_UCOS
    reload = (sysclk * 10) / 8;
    // 根据delay_ostickspersec设定溢出时间, reload为24位寄存器,最大值:16777216,
    // 在180M下,约合0.699s左右
    reload *= (1000000 / ai_delay_ostickspersec);
    factor_ms = 1000 / ai_delay_ostickspersec;    // 代表OS可以延时的最少单位
    SysTick->CTRL |= 0x1 << 1;                    // 开启SYSTICK中断
    SysTick->LOAD = reload / 10;            // 每1/delay_ostickspersec秒中断一次
    SysTick->CTRL |= 0x1;                   // 开启SYSTICK
#else
	factor_ms = ((u32)sysclk * 1000) / 8;    // 每个ms需要的systick时钟数 
#endif    
}								    

#ifdef AI_SYS_SUPPORT_UCOS
/*
********************************************************************************
* 延时nus
* nus为要延时的us数.
* 注意:nus的值,不要大于745654us
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
            // SYSTICK是一个递减的计数器
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
* 延时nms
* SysTick->LOAD为24位寄存器,所以,最大延时为:745ms 
********************************************************************************
*/
void ai_delay_ms(u16 nms)
{
    // 如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度)
    if (ai_delay_osrunning && ai_delay_osintnesting == 0) {
        if (nms >= factor_ms) {
            ai_delay_ostimedly(nms / factor_ms);
        }
        nms %= factor_ms;
    }
    // OS已经无法提供这么小的延时了,采用普通方式延时
    ai_delay_us((u32)(nms * 1000));
}
#else
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
#endif /* #ifdef AI_SYS_SUPPORT_UCOS */			 
