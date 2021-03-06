#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "ailcd.h"
#include "aift5206.h"
#include "aitouch.h"

/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/
ai_tp_dev_t    ai_tp_dev = {
    ai_tp_init,
    ai_ft5206_scan,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

/*
********************************************************************************
*    Function: ai_tp_init
* Description: 触摸屏初始化
*       Input: void
*      Output: None
*      Return:  0 -> 进行过校准
*              -1 -> 没有进行过校准
*      Others: 本文件是触摸屏的顶层文件，目的是支持电阻屏和电容屏，但是手上只有
*              LCD-RGB 1024*600电容屏，所以只实现这个屏幕的，其它未实现.
********************************************************************************
*/
int ai_tp_init(void)
{
    if (ai_lcd_dev.id == AI_LCD_INCH7_1024_600) {
        ai_ft5206_init();
        ai_tp_dev.scan = ai_ft5206_scan;
        ai_tp_dev.type |= 0x80;             // 电容屏
        ai_tp_dev.type |= ai_lcd_dev.dir & 0x01;
    }
    // 可以添加else分支实现其它屏幕的支持，这里不实现
    
    return 0;
}
