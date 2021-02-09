#ifndef AI_INFLASH_H_
#define AI_INFLASH_H_


/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "aitypes.h"

/*
********************************************************************************
*                          EXPORTED MACRO
********************************************************************************
*/
// 定义Flash状态
enum {
    AI_FLASH_FAILURE = -1,
    AI_FLASH_SUCCESS = 0,
    AI_FLASH_BSY,        // 繁忙
    AI_FLASH_PGSERR,     // 编程顺序错误
    AI_FLASH_PGPERR,     // 编程并行位数错误
    AI_FLASH_PGAERR,     // 编程对齐错误
    AI_FLASH_WRPERR,     // 写保护错误
    AI_FLASH_OPERR,      // 操作错误
    AI_FLASH_EOP         // 操作结束
};

// FLASH起始地址
#define    AI_INFLASH_BASE             0x08000000

// FLASH 扇区的起始地址，BANK1
#define    AI_INFLASH_ADDR_SECTOR0     ((u32)0x08000000)    // 扇区0，16KB
#define    AI_INFLASH_ADDR_SECTOR1     ((u32)0x08004000)    // 扇区1，16KB
#define    AI_INFLASH_ADDR_SECTOR2     ((u32)0x08008000)    // 扇区2，16KB
#define    AI_INFLASH_ADDR_SECTOR3     ((u32)0x0800C000)    // 扇区3，16KB
#define    AI_INFLASH_ADDR_SECTOR4     ((u32)0x08010000)    // 扇区4，64KB
#define    AI_INFLASH_ADDR_SECTOR5     ((u32)0x08020000)    // 扇区5，128KB
#define    AI_INFLASH_ADDR_SECTOR6     ((u32)0x08040000)    // 扇区6，128KB
#define    AI_INFLASH_ADDR_SECTOR7     ((u32)0x08060000)    // 扇区7，128KB
#define    AI_INFLASH_ADDR_SECTOR8     ((u32)0x08080000)    // 扇区8，128KB
#define    AI_INFLASH_ADDR_SECTOR9     ((u32)0x080A0000)    // 扇区9，128KB
#define    AI_INFLASH_ADDR_SECTOR10    ((u32)0x080C0000)    // 扇区10，128KB
#define    AI_INFLASH_ADDR_SECTOR11    ((u32)0x080E0000)    // 扇区11，128KB

// BANK2
#define    AI_INFLASH_ADDR_SECTOR12    ((u32)0x08100000)    // 扇区12，16KB
#define    AI_INFLASH_ADDR_SECTOR13    ((u32)0x08104000)    // 扇区13，16KB
#define    AI_INFLASH_ADDR_SECTOR14    ((u32)0x08108000)    // 扇区14，16KB
#define    AI_INFLASH_ADDR_SECTOR15    ((u32)0x0810C000)    // 扇区15，16KB
#define    AI_INFLASH_ADDR_SECTOR16    ((u32)0x08110000)    // 扇区16，64KB
#define    AI_INFLASH_ADDR_SECTOR17    ((u32)0x08120000)    // 扇区17，128KB
#define    AI_INFLASH_ADDR_SECTOR18    ((u32)0x08140000)    // 扇区18，128KB
#define    AI_INFLASH_ADDR_SECTOR19    ((u32)0x08160000)    // 扇区19，128KB
#define    AI_INFLASH_ADDR_SECTOR20    ((u32)0x08180000)    // 扇区20，128KB
#define    AI_INFLASH_ADDR_SECTOR21    ((u32)0x081A0000)    // 扇区21，128KB
#define    AI_INFLASH_ADDR_SECTOR22    ((u32)0x081C0000)    // 扇区22，128KB
#define    AI_INFLASH_ADDR_SECTOR23    ((u32)0x081E0000)    // 扇区23，128KB

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
int ai_inflash_erase_sector(u8 num);
int ai_inflash_wr_word(u32 addr, u32 data);
u32 ai_inflash_rd_word(u32 addr);
u8 ai_inflash_get_sector_num(u32 addr);
int ai_inflash_write(u32 addr, u32 *buf, u32 len);
void ai_inflash_read(u32 addr, u32 *buf, u32 len);


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_INFLASH_H_ */
