#ifndef AI_USART_H_
#define AI_USART_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
********************************************************************************
*                               INCLUDE FILES
********************************************************************************
*/     
#include <aitypes.h>
#include "stdio.h"	  

/*
********************************************************************************
*                               RECV BUFFER
********************************************************************************
*/
/* 定义该宏支持printf函数,而不需要在设置Target中选择use MicroLIB */
#define    AI_PRINTF_SUPPORTED     1
#define    AI_EN_USART1_RX         1		     // 使能（1）/禁止（0）串口1接收

#define    AI_USART_REC_LEN        200           // 定义最大接收字节数 200
	  	
extern u8  AI_USART_RX_BUF[AI_USART_REC_LEN];    // 接收缓冲,末字节为换行符 
extern u16 AI_USART_RX_STA;         		     // 接收状态标记	

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
void ai_uart_init(u32 pclk2, u32 baudrate);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_USART_H_ */	   
















