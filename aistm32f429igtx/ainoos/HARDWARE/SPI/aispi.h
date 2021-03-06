#ifndef AI_SPI_H_
#define AI_SPI_H_


/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "aitypes.h"

/*
********************************************************************************
*                           PRIVATE DEFINE
********************************************************************************
*/
enum {
    AI_SPI_SPEED_2 = 0,
    AI_SPI_SPEED_4,
    AI_SPI_SPEED_8,
    AI_SPI_SPEED_16,
    AI_SPI_SPEED_32,
    AI_SPI_SPEED_64,
    AI_SPI_SPEED_128,
    AI_SPI_SPEED_256
};

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_spi5_init(void);
u8 ai_spi5_rdwr_byte(u8 data);
void ai_spi5_set_speed(u8 br);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_SPI_H_ */
