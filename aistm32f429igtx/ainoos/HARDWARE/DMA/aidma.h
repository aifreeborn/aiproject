#ifndef AI_DMA_H_
#define AI_DMA_H_

/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "stm32f4xx.h"
#include "aitypes.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
void ai_dma_config(DMA_Stream_TypeDef *streamx, u8 channel,
                   u32 paddr, u32 maddr, u16 ndtr);
void ai_dma_enable(DMA_Stream_TypeDef *streamx, u16 ndtr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_DMA_H_ */
