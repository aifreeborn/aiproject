#ifndef AI_TOUCHPAD_H_
#define AI_TOUCHPAD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/*
********************************************************************************
*                               INCLUDE FILES
********************************************************************************
*/    
#include "aitypes.h"	

extern vu32 ai_touchpad_default_val;
/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
int ai_touchpad_init(u8 prescaler);    
int ai_touchpad_scan(u8 mode);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_TOUCHPAD_H_ */
