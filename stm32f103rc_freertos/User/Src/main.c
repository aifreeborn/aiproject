/**
  ******************************************************************************
  * @file    main.c 
  * @author  aitakeno
  * @version V1.0
  * @date    2025-07-12
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火STM32F103RC mini开发板
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"
#include "bsp_led.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle;
/* LED任务句柄 */
static TaskHandle_t LED_Task_Handle;    
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* AppTaskCreate任务任务堆栈 */
static StackType_t AppTaskCreate_Stack[128];
/* LED任务堆栈 */
static StackType_t LED_Task_Stack[128];

/* AppTaskCreate 任务控制块 */
static StaticTask_t AppTaskCreate_TCB;
/* AppTaskCreate 任务控制块 */
static StaticTask_t LED_Task_TCB;

/* 空闲任务任务堆栈 */
static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
/* 定时器任务堆栈 */
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];

/* 空闲任务控制块 */
static StaticTask_t Idle_Task_TCB;    
/* 定时器任务控制块 */
static StaticTask_t Timer_Task_TCB;

/* Private function prototypes -----------------------------------------------*/
static void AppTaskCreate(void);/* 用于创建任务 */

static void LED_Task(void* pvParameters);/* LED_Task任务实现 */

/* Private functions ---------------------------------------------------------*/
/**
    * 使用了静态分配内存，以下这两个函数是由用户实现，函数在task.c文件中有引用
    *    当且仅当 configSUPPORT_STATIC_ALLOCATION 这个宏定义为 1 的时候才有效
    */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

int main(void)
{
    BSP_Init();


    /* 创建 AppTaskCreate 任务 */
    AppTaskCreate_Handle = xTaskCreateStatic((TaskFunction_t)AppTaskCreate,           //任务函数
                                             (const char*)"AppTaskCreate",            //任务名称
                                             (uint32_t)128,                           //任务堆栈大小
                                             (void*)NULL,                             //传递给任务函数的参数
                                             (UBaseType_t)3,                          //任务优先级
                                             (StackType_t*)AppTaskCreate_Stack,       //任务堆栈
                                             (StaticTask_t*  )&AppTaskCreate_TCB);    //任务控制块   
                                                            
    if(NULL != AppTaskCreate_Handle)    /* 创建成功 */
        vTaskStartScheduler();          /* 启动任务，开启调度 */

    while(1);   /* 正常不会执行到这里 */   
}

/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
    printf("this is a static task\n");
    taskENTER_CRITICAL();               //进入临界区
    
    /* 创建LED_Task任务 */
    LED_Task_Handle = xTaskCreateStatic((TaskFunction_t    )LED_Task,            //任务函数
                                        (const char*     )"LED_Task",        //任务名称
                                        (uint32_t         )128,                //任务堆栈大小
                                        (void*               )NULL,                //传递给任务函数的参数
                                        (UBaseType_t     )4,                 //任务优先级
                                        (StackType_t*   )LED_Task_Stack,    //任务堆栈
                                        (StaticTask_t*  )&LED_Task_TCB);    //任务控制块   
    
    if(NULL != LED_Task_Handle)/* 创建成功 */
        printf("LED_Task create successful!\n");
    else
        printf("LED_Task create failed!\n");

    vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
    taskEXIT_CRITICAL();            //退出临界区
}



/**********************************************************************
  * @ 函数名  ： LED_Task
  * @ 功能说明： LED_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void LED_Task(void* parameter)
{    
    while (1)
    {
        printf("LED_Task Running,LED1_ON\r\n");
        LED1_ON;
        LED2_OFF;
        vTaskDelay(1000);   /* 延时1000个tick */

        printf("LED_Task Running,LED1_OFF\r\n");        
        LED1_OFF;
        LED2_ON;
        vTaskDelay(1000);   /* 延时1000个tick */                 
    }
}

/**
  **********************************************************************
  * @brief  获取空闲任务的任务堆栈和任务控制块内存
    *                    ppxTimerTaskTCBBuffer    :        任务控制块内存
    *                    ppxTimerTaskStackBuffer  :        任务堆栈内存
    *                    pulTimerTaskStackSize    :        任务堆栈大小
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  **********************************************************************
  */ 
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
                                   StackType_t **ppxIdleTaskStackBuffer, 
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* 任务控制块内存 */
    *ppxIdleTaskStackBuffer=Idle_Task_Stack;/* 任务堆栈内存 */
    *pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* 任务堆栈大小 */
}

/**
  *********************************************************************
  * @brief  获取定时器任务的任务堆栈和任务控制块内存
    *                    ppxTimerTaskTCBBuffer    :    任务控制块内存
    *                    ppxTimerTaskStackBuffer  :    任务堆栈内存
    *                    pulTimerTaskStackSize    :    任务堆栈大小
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  **********************************************************************
  */ 
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
                                    StackType_t **ppxTimerTaskStackBuffer, 
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* 任务控制块内存 */
    *ppxTimerTaskStackBuffer=Timer_Task_Stack;/* 任务堆栈内存 */
    *pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* 任务堆栈大小 */
}

/************************ END OF FILE *****************************************/
