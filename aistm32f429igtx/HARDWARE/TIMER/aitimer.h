#ifndef AI_TIMER_H_
#define AI_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  
/*
********************************************************************************
*                               INCLUDE FILES
********************************************************************************
*/  
#include <aitypes.h>
    
/*
********************************************************************************
*                           Variables
********************************************************************************
*/   
extern u8 ai_timer5_ch1_capture_stat;   //  ‰»Î≤∂ªÒ◊¥Ã¨
extern u32 ai_timer5_ch1_capture_val;
    
/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_timer3_interrupt_init(u16 arr, u16 prescaler);
void ai_timer3_pwm_init(u16 arr, u16 prescaler);
void ai_timer3_set_pwm(u32 val);
void ai_timer5_ch1_capture_init(u32 arr, u16 prescaler);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_TIMER_H_ */
