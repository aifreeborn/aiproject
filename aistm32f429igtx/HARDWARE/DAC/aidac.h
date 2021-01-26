#ifndef AI_DAC_H_
#define AI_DAC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "aitypes.h"

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_dac_init(void);
void ai_dac_set_vol(u16 vol);
u16 ai_dac_get_vol(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_DAC_H_ */
