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
    
    /* 设置时钟180MHz */
    ai_sys_clock_init(360, 25, 2, 8);
    ai_delay_init(180);
	
    /* 外设初始化 */
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    OSInit(&err);           // 初始化UCOSIII
    OS_CRITICAL_ENTER();    // 进入临界区
    
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
    OSStatTaskCPUUsageInit(&err);        // 统计任务                
#endif
    
#ifdef CPU_CFG_INT_DIS_MEAS_EN           // 如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();
#endif
    
#if OS_CFG_SCHED_ROUND_ROBIN_EN          // 当使用时间片轮转的时候
    // 使能时间片轮转调度功能,设置默认的时间片长度s
    OSSchedRoundRobinCfg(DEF_ENABLED,10,&err);
#endif
    OS_CRITICAL_ENTER();
    // 创建LED0任务
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
                 
    // 创建LED1任务
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
                 
    // 创建浮点测试任务
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
        printf("float_num的值为: %.4f\r\n", num);
        OS_CRITICAL_EXIT();
        ai_delay_ms(1000);
    }
}
