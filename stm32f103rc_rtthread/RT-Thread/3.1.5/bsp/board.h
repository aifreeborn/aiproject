#ifndef BOARD_H_
#define BOARD_H_

#include "stm32f1xx.h"
#include "bsp_led.h"



void rt_hw_board_init(void);
void SysTick_Handler(void);
void SystemClock_Config(void);

#endif