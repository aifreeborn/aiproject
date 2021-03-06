#ifndef AI_TOUCH_H_
#define AI_TOUCH_H_


/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "aitypes.h"

/*
********************************************************************************
*                           EXPORTED MACRO/DEFINE
********************************************************************************
*/
// 电容屏支持的点数,固定为5点
#define    AI_TP_MAX             5

// 按键状态
#define    AI_TP_DOWN_PREV       0x80        // 触屏被按下
#define    AI_TP_DOWN            0x40        // 有按键按下了

// touch panel,触摸屏设备控制器
typedef struct {
    int (*init)(void);        // 初始化触摸屏控制器
    u8 (*scan)(u8);          // 扫描触摸屏:0,屏幕扫描;1,物理坐标
    void (*adjust)(void);    // 触摸屏校准
    u16 x[AI_TP_MAX];
    u16 y[AI_TP_MAX];        // 电容屏有最多5组坐标
    /* 
     * 笔的状态
     *     b7 -> 按下1/松开0
     *     b6 -> 0,没有按键按下;1,有按键按下
     *     b5 -> 保留
     *  b4~b0 -> 相应的位代表电容触摸屏按下的点数(0,表示未按下,1表示按下)
     */
    u8 stat;
    
    //  触摸屏校准参数(电容屏不需要校准)
    float xfac;
    float yfac;
    short xoff;
    short yoff;
    
    /*
     * 触摸屏类型，当触摸屏的左右上下完全颠倒时需要用到.
     *   b0 -> 0,竖屏(适合左右为X坐标,上下为Y坐标的TP)
     *         1,横屏(适合左右为Y坐标,上下为X坐标的TP)
     * b1~6 -> 保留.
     * b7   -> 0,电阻屏;1,电容屏
     */
    u8 type;
} ai_tp_dev_t;

// 触屏控制器在aitouch.c里面定义
extern    ai_tp_dev_t    ai_tp_dev;

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
int ai_tp_init(void);


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_TOUCH_H_ */
