#ifndef MAIN_H_
#define MAIN_H_

/*
********************************************************************************
*                          SYS INCLUDES
********************************************************************************
*/
#include <stdio.h>
#include <aigpio.h>
#include <aisys.h>
#include <aidelay.h>
#include "aitypes.h"
#include "includes.h"

/*
********************************************************************************
*                        HARDWARE INCLUDES
********************************************************************************
*/
#include "ailed.h"
#include "aikey.h"
#include "aiusart.h"

/*
********************************************************************************
*                             DEFINES
********************************************************************************
*/
/*
********************************************************************************
* UCOSIII中以下优先级用户程序不能使用，ALIENTEK
* 将这些优先级分配给了UCOSIII的5个系统内部任务
* 优先级0：中断服务服务管理任务 OS_IntQTask()
* 优先级1：时钟节拍任务 OS_TickTask()
* 优先级2：定时任务 OS_TmrTask()
* 优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
* 优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
********************************************************************************
*/
#define    AI_START_TASK_PRIO        3          // 任务优先级
#define    AI_START_STK_SIZE         512        // 任务堆栈大小

#define    AI_LED0_TASK_PRIO         4
#define    AI_LED0_STK_SIZE          128

#define    AI_LED1_TASK_PRIO         5
#define    AI_LED1_STK_SIZE          128

#define    AI_FLOAT_TASK_PRIO        6
#define    AI_FLOAT_STK_SIZE         256

/*
********************************************************************************
*                           GLOBAL VARIABLES
********************************************************************************
*/
OS_TCB ai_start_task_tcb;                       // 任务控制块
CPU_STK ai_start_task_stk[AI_START_STK_SIZE];   // 任务堆栈

// led0任务相关
OS_TCB ai_led0_task_tcb;
CPU_STK ai_led0_task_stk[AI_LED0_STK_SIZE];

OS_TCB ai_led1_task_tcb;
CPU_STK ai_led1_task_stk[AI_LED1_STK_SIZE];

OS_TCB ai_float_task_tcb;
__align(8) CPU_STK ai_float_task_stk[AI_FLOAT_STK_SIZE];

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_start_task(void *parg);
void ai_led0_task(void *parg);
void ai_led1_task(void *parg);
void ai_float_task(void *parg);


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MAIN_H_ */
