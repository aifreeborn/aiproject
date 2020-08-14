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
// ����ƫ�Ƶ�ַ
void ai_nvic_set_vector_table(u32 base_addr, u32 offset);
// ����NVIC����
void ai_nvic_priority_group_cfg(u8 group);
// �����ж�
void ai_nvic_init(u8 preempt_priority, u8 sub_priority, u8 channel, u8 group);
// �ⲿ�ж����ú���(ֻ��GPIOA~I)
void ai_nvic_exti_cfg(u8 gpiox, u8 bitn, u8 trim_mode);


// �ر������ж�
void ai_interrupt_disable_all(void);
// ���������ж�
void ai_interrupt_enable_all(void);
//���ö�ջ��ַ
void ai_stack_msr_msp(u32 addr);

// ����ģʽ
void ai_sys_standby(void);
// ϵͳ��λ
void ai_sys_soft_reset(void);


// ʱ�ӳ�ʼ��
void ai_sys_clock_init(u32 plln, u32 pllm, u32 pllp, u32 pllq);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_SYS_H_ */











