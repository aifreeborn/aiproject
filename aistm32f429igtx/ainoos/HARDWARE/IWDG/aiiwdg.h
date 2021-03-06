#ifndef AI_IWDG_H_
#define AI_IWDG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
********************************************************************************
*                               INCLUDE FILES
********************************************************************************
*/    
#include "aitypes.h"

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
int ai_iwdg_init(u8 prescaler, u16 reload);
void ai_iwdg_feed(void);

#ifdef __cpluscplus
}
#endif /* __cplusplus */

#endif /* AI_IWDG_H_ */
