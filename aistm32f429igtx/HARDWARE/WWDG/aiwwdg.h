#ifndef AI_WWDG_H_
#define AI_WWDG_H_

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
int ai_wwdg_init(u8 counter, u8 window_val, u8 timer_base);
void ai_wwdg_set_counter(u8 cnt);

#ifdef __cpluscplus
}
#endif /* __cplusplus */

#endif /* AI_WWDG_H_ */
