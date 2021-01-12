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
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_timer3_interrupt_init(u16 arr, u16 prescaler);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_TIMER_H_ */
