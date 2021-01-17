#ifndef AI_LTDC_H_
#define AI_LTDC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "aitypes.h"

// 定义默认背景层颜色
#define    AI_LTDC_BACKLAYER_COLOR          0x00000000	

/*
********************************************************************************
*                            LTDC的开关宏定义
********************************************************************************
*/
enum _ai_ltdc_switch {
    AI_LTDC_OFF = 0,
    AI_LTDC_ON
};

enum _ai_ltdc_layer_switch {
    AI_LTDC_LAYER_OFF = 0,
    AI_LTDC_LAYER_ON
};

/*
********************************************************************************
*                            兼容不同RGB LCD的参数集合
********************************************************************************
*/
typedef struct {
    u32    panel_width;     // LCD面板的宽度,固定参数,不随显示方向改变,
                            // 如果为0,说明没有任何RGB屏接入;单位为像素
    u32    panel_height;    // LCD面板的高度,固定参数,不随显示方向改变；单位为像素
    u16    hsw;             // 水平同步宽度
    u16    vsw;             // 垂直同步宽度
    u16    hbp;             // 水平后廊
    u16    vbp;             // 垂直后廊
    u16    hfp;             // 水平前廊
    u16    vfp;             // 垂直前廊
    u8     active_layer;    // 当前层编号:0/1
    u8     dir;             // 0,竖屏;1,横屏;
    u16    width;           // LCD宽度
    u16    height;          // LCD高度
    u32    pixel_size;      // 每个像素所占字节数
} ai_ltdc_dev_t;


extern ai_ltdc_dev_t ai_ltdc_dev;

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
int ai_ltdc_init(void);
void ai_ltdc_switch(u8 action);
void ai_ltdc_draw_point(u16 x, u16 y, u32 color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_LTDC_H_ */

