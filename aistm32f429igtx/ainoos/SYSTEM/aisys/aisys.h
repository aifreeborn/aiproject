#ifndef AI_SYS_H_
#define AI_SYS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
********************************************************************************
*                             INCLUDE FILES
********************************************************************************
*/ 
#include "stm32f4xx.h" 
#include "aitypes.h"
#include "aigpio.h"

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
// 设置偏移地址
void ai_nvic_set_vector_table(u32 base_addr, u32 offset);
// 设置NVIC分组
void ai_nvic_priority_group_cfg(u8 group);
// 设置中断
void ai_nvic_init(u8 preempt_priority, u8 sub_priority, u8 channel, u8 group);
// 外部中断配置函数(只对GPIOA~I)
void ai_nvic_exti_cfg(u8 gpiox, u8 bitn, u8 trim_mode);


// 关闭所有中断
void ai_interrupt_disable_all(void);
// 开启所有中断
void ai_interrupt_enable_all(void);
//设置堆栈地址
void ai_stack_msr_msp(u32 addr);

// 待机模式
void ai_sys_standby(void);
// 系统软复位
void ai_sys_soft_reset(void);


// 时钟初始化
void ai_sys_clock_init(u32 plln, u32 pllm, u32 pllp, u32 pllq);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_SYS_H_ */











