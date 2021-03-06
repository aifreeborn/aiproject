#ifndef AI_RNG_H_
#define AI_RNG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
#include "aitypes.h"

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
int ai_rng_init(void);
u32 ai_rng_get_random_num(void);
u32 ai_rng_get_random_range(u32 min, u32 max);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_RNG_H_ */
