#ifndef AI_W25QXX_H_
#define AI_W25QXX_H_


/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "aitypes.h"

/*
********************************************************************************
*                          EXPORTED DEFINE
********************************************************************************
*/
// 定义芯片类型
#define    AI_W25Q08         0XEF13
#define    AI_W25Q16         0XEF14
#define    AI_W25Q32         0XEF15
#define    AI_W25Q64         0XEF16
#define    AI_W25Q128        0XEF17
#define    AI_W25Q256        0XEF18

// 定义Flash操作指令
#define    AI_W25QXX_WR_EN                     0x06 
#define    AI_W25QXX_WR_DISABLE                0x04 
#define    AI_W25QXX_RD_SR1                    0x05 
#define    AI_W25QXX_RD_SR2                    0x35 
#define    AI_W25QXX_RD_SR3                    0x15 
#define    AI_W25QXX_WR_SR1                    0x01 
#define    AI_W25QXX_WR_SR2                    0x31 
#define    AI_W25QXX_WR_SR3                    0x11 
#define    AI_W25QXX_RD_DATA                   0x03
// FRD -> Fast read
#define    AI_W25QXX_FRD_DATA                  0x0B 
#define    AI_W25QXX_FRD_DUAL                  0x3B 
#define    AI_W25QXX_PAGE_PROGRAM              0x02 
#define    AI_W25QXX_BLOCK_ERASE               0xD8 
#define    AI_W25QXX_SECTOR_ERASE              0x20 
#define    AI_W25QXX_CHIP_ERASE                0xC7 
#define    AI_W25QXX_POWER_DOWN                0xB9 
#define    AI_W25QXX_RELEASE_POWER_DOWN        0xAB 
#define    AI_W25QXX_DEV_ID                    0xAB 
#define    AI_W25QXX_MANUFACT_DEV_ID           0x90 
#define    AI_W25QXX_JEDEC_DEV_ID              0x9F 
#define    AI_W25QXX_4BYTE_ADDR_EN             0xB7
#define    AI_W25QXX_EXIT_4BYTE_ADDR           0xE9

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_w25qxx_init(void);
u16 ai_w25qxx_read_id(void);
u8 ai_w25qxx_read_sr(u8 num);
void ai_w25qxx_write_sr(u8 num, u8 data);
void ai_w25qxx_read(u8 *pbuf, u32 addr, u16 len);
void ai_w25qxx_write_page(u8 *pbuf, u32 addr, u16 len);
void ai_w25qxx_wr_nocheck(u8 *pbuf, u32 addr, u16 len);
void ai_w25qxx_write(u8 *pbuf, u32 addr, u16 len);
void ai_w25qxx_erase_chip(void);
void ai_w25qxx_power_down(void);
void ai_w25qxx_wake_up(void);
void ai_w25qxx_write_enable(void);
void ai_w25qxx_write_disable(void);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_W25QXX_H_ */
