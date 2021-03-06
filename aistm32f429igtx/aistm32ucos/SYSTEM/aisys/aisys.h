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

// ����ϵͳ�ļ����Ƿ�֧��OS
#define    AI_SYS_SUPPORT_UCOS        1
/*
********************************************************************************
*                        HSE/HSI Values adaptation
********************************************************************************
*/
// Value of the Internal oscillator in Hz
#define    AI_HSI_VALUE               ((uint32_t)16000000)
#define    AI_HSE_VALUE               ((uint32_t)25000000)

// RCC_PLL_Clock_Source PLL Clock Source
#define    AI_RCC_PLLSOURCE_HSI        RCC_PLLCFGR_PLLSRC_HSI
#define    AI_RCC_PLLSOURCE_HSE        RCC_PLLCFGR_PLLSRC_HSE

/*
********************************************************************************
* brief  Macro to get the oscillator used as PLL clock source.
* @retval The oscillator used as PLL clock source. The returned value can be one
*         of the following:
*              - RCC_PLLSOURCE_HSI: HSI oscillator is used as PLL clock source.
*              - RCC_PLLSOURCE_HSE: HSE oscillator is used as PLL clock source.
********************************************************************************
*/
#define    AI_RCC_GET_PLL_OSCSOURCE()        ((uint32_t)(RCC->PLLCFGR \
                                                         & RCC_PLLCFGR_PLLSRC))

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

uint32_t ai_sys_get_sysclk_freq(void);
uint32_t ai_sys_get_hclk_freq(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_SYS_H_ */











