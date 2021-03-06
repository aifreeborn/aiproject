#include "main.h"

/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    OS_ERR err;
    CPU_SR_ALLOC();
    
    /* ����ʱ��180MHz */
    ai_sys_clock_init(360, 25, 2, 8);
    ai_delay_init(180);
	
    /* �����ʼ�� */
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    OSInit(&err);           // ��ʼ��UCOSIII
    OS_CRITICAL_ENTER();    // �����ٽ���
    
    OSTaskCreate((OS_TCB     *)&ai_start_task_tcb,
                 (CPU_CHAR   *)"start task",
                 (OS_TASK_PTR )ai_start_task,
                 (void       *)0,
                 (OS_PRIO     )AI_START_TASK_PRIO,
                 (CPU_STK    *)&ai_start_task_stk[0],
                 (CPU_STK_SIZE)AI_START_STK_SIZE / 10,
                 (CPU_STK_SIZE)AI_START_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR
                               | OS_OPT_TASK_SAVE_FP,
                 (OS_ERR     *)&err);
    OS_CRITICAL_EXIT();
    OSStart(&err);
    /* main loop */
    while (1) {
        /* Empty */
    }
}

void ai_start_task(void *parg)
{
    OS_ERR err;
    CPU_SR_ALLOC();
    
    parg = parg;
    CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);        // ͳ������                
#endif
    
#ifdef CPU_CFG_INT_DIS_MEAS_EN           // ���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();
#endif
    
#if OS_CFG_SCHED_ROUND_ROBIN_EN          // ��ʹ��ʱ��Ƭ��ת��ʱ��
    // ʹ��ʱ��Ƭ��ת���ȹ���,����Ĭ�ϵ�ʱ��Ƭ����s
    OSSchedRoundRobinCfg(DEF_ENABLED,10,&err);
#endif
    OS_CRITICAL_ENTER();
    // ����LED0����
    OSTaskCreate((OS_TCB     *)&ai_led0_task_tcb,
                 (CPU_CHAR   *)"led0 task",
                 (OS_TASK_PTR )ai_led0_task,
                 (void       *)0,
                 (OS_PRIO     )AI_LED0_TASK_PRIO,
                 (CPU_STK    *)&ai_led0_task_stk[0],
                 (CPU_STK_SIZE)AI_LED0_STK_SIZE / 10,
                 (CPU_STK_SIZE)AI_LED0_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR
                               | OS_OPT_TASK_SAVE_FP,
                 (OS_ERR     *)&err);
                 
    // ����LED1����
    OSTaskCreate((OS_TCB     *)&ai_led1_task_tcb,
                 (CPU_CHAR   *)"led1 task",
                 (OS_TASK_PTR )ai_led1_task,
                 (void       *)0,
                 (OS_PRIO     )AI_LED1_TASK_PRIO,
                 (CPU_STK    *)&ai_led1_task_stk[0],
                 (CPU_STK_SIZE)AI_LED1_STK_SIZE / 10,
                 (CPU_STK_SIZE)AI_LED1_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR
                               | OS_OPT_TASK_SAVE_FP,
                 (OS_ERR     *)&err);
                 
    // ���������������
                 OSTaskCreate((OS_TCB     *)&ai_float_task_tcb,
                 (CPU_CHAR   *)"float test task",
                 (OS_TASK_PTR )ai_float_task,
                 (void       *)0,
                 (OS_PRIO     )AI_FLOAT_TASK_PRIO,
                 (CPU_STK    *)&ai_float_task_stk[0],
                 (CPU_STK_SIZE)AI_FLOAT_STK_SIZE / 10,
                 (CPU_STK_SIZE)AI_FLOAT_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR
                               | OS_OPT_TASK_SAVE_FP,
                 (OS_ERR     *)&err);
    OS_CRITICAL_EXIT();
    OS_TaskSuspend((OS_TCB *)&ai_start_task_tcb, &err);
}

void ai_led0_task(void *parg)
{
    OS_ERR err;
    
    parg = parg;
    while (1) {
        AI_DS0 = 0;
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        AI_DS0 = 1;
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

void ai_led1_task(void *parg)
{
    parg = parg;
    
    while (1) {
        AI_DS1 = !AI_DS1;
        ai_delay_ms(500);
    }
}

void ai_float_task(void *parg)
{
    CPU_SR_ALLOC();
    static float num = 0.00;
    
    while (1) {
        num += 0.01f;
        OS_CRITICAL_ENTER();
        printf("float_num��ֵΪ: %.4f\r\n", num);
        OS_CRITICAL_EXIT();
        ai_delay_ms(1000);
    }
}
