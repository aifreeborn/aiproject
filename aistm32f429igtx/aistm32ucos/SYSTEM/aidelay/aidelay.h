#ifndef AI_DELAY_H_
#define AI_DELAY_H_	

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
void ai_delay_init(u8 sysclk);
void ai_delay_ms(u16 nms);
void ai_delay_us(u32 nus);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_DELAY_H_ */





























