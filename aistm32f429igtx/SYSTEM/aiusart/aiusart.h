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
/* ����ú�֧��printf����,������Ҫ������Target��ѡ��use MicroLIB */
#define    AI_PRINTF_SUPPORTED     1
#define    AI_EN_USART1_RX         1		     // ʹ�ܣ�1��/��ֹ��0������1����

#define    AI_USART_REC_LEN        200           // �����������ֽ��� 200
	  	
extern u8  AI_USART_RX_BUF[AI_USART_REC_LEN];    // ���ջ���,ĩ�ֽ�Ϊ���з� 
extern u16 AI_USART_RX_STA;         		     // ����״̬���	

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
















