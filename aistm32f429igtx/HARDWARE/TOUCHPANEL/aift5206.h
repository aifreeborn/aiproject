#ifndef AI_FT5206_H_
#define AI_FT5206_H_


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
#define    AI_FT5206_CMD_WR        0x70
#define    AI_FT5206_CMD_RD        0x71

// 部分寄存器定义
#define    AI_FT5206_REG_DEV_MODE        0x00        // FT5206模式控制寄存器
#define    AI_FT5206_REG_TD_STATUS       0x02        // 触摸状态寄存器
#define    AI_FT5206_REG_TP1             0x03        // 第一个触摸点数据地址
#define    AI_FT5206_REG_TP2             0x09        // 第二个触摸点数据地址
#define    AI_FT5206_REG_TP3             0x0f        // 第三个触摸点数据地址
#define    AI_FT5206_REG_TP4             0x15        // 第四个触摸点数据地址
#define    AI_FT5206_REG_TP5             0x1b        // 第五个触摸点数据地址
#define    AI_FT5206_REG_ID_G_LIB_VER    0xa1        // 版本
#define    AI_FT5206_REG_ID_G_MODE       0xa4        // FT5206中断模式控制寄存器
#define    AI_FT5206_REG_ID_G_THGROUP    0x80        // 触摸有效值设置寄存器
#define    AI_FT5206_REG_ID_G_PERIODACTIVE 0x88      // 激活状态周期设置寄存器

enum {
    AI_FT5206_NO_TOUCH = 0,
    AI_FT5206_TOUCH
};

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
int ai_ft5206_init(void);
int ai_ft5206_wr_reg(u16 reg, u8 *buf, u8 len);
void ai_ft5206_rd_reg(u16 reg, u8 *buf, u8 len);
u8 ai_ft5206_scan(u8 mode);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_FT5206_H_ */
