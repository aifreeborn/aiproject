#ifndef AI_I2C_H_
#define AI_I2C_H_


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
void ai_i2c_init(void);
void ai_i2c_start(void);
void ai_i2c_stop(void);
int ai_i2c_wait_ack(void);
void ai_i2c_ack(void);
void ai_i2c_no_ack(void);
void ai_i2c_send_byte(u8 data);
u8 ai_i2c_read_byte(u8 ack);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_I2C_H_ */
