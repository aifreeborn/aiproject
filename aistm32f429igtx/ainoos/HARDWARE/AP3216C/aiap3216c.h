#ifndef AI_AP3216C_H_
#define AI_AP3216C_H_


/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "aitypes.h"

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
int ai_ap3216c_init(void);
int ai_ap3216c_read_byte(u8 reg, u8 *data);
int ai_ap3216c_write_byte(u8 reg, u8 data);
void ai_ap3216c_read_data(u16 *ir, u16 *ps, u16 *als);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_AP3216C_H_ */
