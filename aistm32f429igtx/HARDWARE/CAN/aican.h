#ifndef AI_CAN_H_
#define AI_CAN_H_


/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "aitypes.h"

/*
********************************************************************************
*                          Exported macro
********************************************************************************
*/
#define    AI_CAN1_RX0_INT_EN        0


/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
int ai_can1_mode_init(u8 sjw, u8 ts1, u8 ts2, u16 brp, u8 mode);
void ai_can1_rx_msg(u8 fifox, u32 *id, u8 *ide, u8 *rtr, u8 *len, u8 *data);
int ai_can1_tx_msg(u32 id, u8 ide, u8 rtr, u8 len, u8 *data);
u8 ai_can1_get_tx_stat(u8 mbox);
u8 ai_can1_msg_pend(u8 fifo_num);
int ai_can1_send_msg(u8 *msg, u8 len);
u8 ai_can1_receive_msg(u8 *buf);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_CAN_H_ */
