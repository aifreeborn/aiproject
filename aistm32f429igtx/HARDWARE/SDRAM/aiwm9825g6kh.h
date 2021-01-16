#ifndef AI_WM9825G6KH_H_
#define AI_WM9825G6KH_H_

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
*                                 MACRO
********************************************************************************
*/    
#define AI_BANK5_SDRAM_ADDR ((u32)0xc0000000)
    
/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
void ai_wm9825g6kh_init(void);
void ai_wm9825g6kh_write_buf(u8 *pbuf, u32 addr, u32 nbytes);
void ai_wm9825g6kh_read_buf(u8 *pbuf, u32 addr, u32 nbytes);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_WM9825G6KH_H_ */
