#ifndef AI_DHT11_H_
#define AI_DHT11_H_


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
int ai_dht11_init(void);
int ai_dht11_read_data(u16 *temperature, u16 *humidity);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_DHT11_H_ */
