#ifndef AI_AT24CXX_H_
#define AI_AT24CXX_H_


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
#define    AI_AT24C01        127
#define    AI_AT24C02        255
#define    AI_AT24C04        511
#define    AI_AT24C08        1023
#define    AI_AT24C16        2047
#define    AI_AT24C32        4095
#define    AI_AT24C64        8191
#define    AI_AT24C128       16383
#define    AI_AT24C256       32767
// 开发板使用的是24c02，所以定义EE_TYPE为AT24C02
#define    AI_EE_TYPE        AI_AT24C02

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_at24cxx_init(void);
int ai_at24cxx_check(void);
u8 ai_at24cxx_read_one_byte(u16 addr);
void ai_at24cxx_write_one_byte(u16 addr, u8 data);
void ai_at24cxx_read(u16 addr, u8 *pbuf, u16 len);
void ai_at24cxx_write(u16 addr, u8 *pbuf, u16 len);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_AT24CXX_H_ */
