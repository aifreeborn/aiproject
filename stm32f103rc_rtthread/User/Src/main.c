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
#include "board.h"
#include "rtthread.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static struct rt_thread led1_thread;

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t rt_led1_thread_stack[1024];


/* Private function prototypes -----------------------------------------------*/
static void led1_thread_entry(void *param);


/* Private functions ---------------------------------------------------------*/


int main(void)
{
    rt_thread_init(&led1_thread,
                   "led1",
                   led1_thread_entry,
                   RT_NULL,
                   &rt_led1_thread_stack[0],
                   sizeof(rt_led1_thread_stack),
                   3,
                   20);
     rt_thread_startup(&led1_thread);
}

static void led1_thread_entry(void *param)
{
    while (1) {
        LED1_ON;
        rt_thread_delay(500);
        rt_kprintf("led1_thread running, LED1_ON\r\n");
        
        LED1_OFF;
        rt_thread_delay(500);
        rt_kprintf("led1_thread running, LED1_OFF\r\n");
    }
}

/************************ END OF FILE *****************************************/
