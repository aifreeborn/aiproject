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
* UCOSIII���������ȼ��û�������ʹ�ã�ALIENTEK
* ����Щ���ȼ��������UCOSIII��5��ϵͳ�ڲ�����
* ���ȼ�0���жϷ������������� OS_IntQTask()
* ���ȼ�1��ʱ�ӽ������� OS_TickTask()
* ���ȼ�2����ʱ���� OS_TmrTask()
* ���ȼ�OS_CFG_PRIO_MAX-2��ͳ������ OS_StatTask()
* ���ȼ�OS_CFG_PRIO_MAX-1���������� OS_IdleTask()
********************************************************************************
*/
#define    AI_START_TASK_PRIO        3          // �������ȼ�
#define    AI_START_STK_SIZE         512        // �����ջ��С

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
OS_TCB ai_start_task_tcb;                       // ������ƿ�
CPU_STK ai_start_task_stk[AI_START_STK_SIZE];   // �����ջ

// led0�������
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
