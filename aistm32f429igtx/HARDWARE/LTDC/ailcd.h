#ifndef AI_LCD_H_
#define AI_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "aigpio.h"

/*
********************************************************************************
*                            定义像素格式
********************************************************************************
*/
#define    AI_LCD_INCH43_480_272            0x4342
#define    AI_LCD_INCH7_800_480             0x7084
#define    AI_LCD_INCH7_1024_600            0x7016
#define    AI_LCD_INCH7_1280_800            0x7018
#define    AI_LCD_INCH8_1024_600            0x8016
    
#define    AI_LCD_PIXEL_FMT_ARGB8888        0x00
#define    AI_LCD_PIXEL_FMT_RGB888          0x01
#define    AI_LCD_PIXEL_FMT_RGB565          0x02
#define    AI_LCD_PIXEL_FMT_ARGB1555        0x03
#define    AI_LCD_PIXEL_FMT_ARGB4444        0x04
#define    AI_LCD_PIXEL_FMT_L8              0x05
#define    AI_LCD_PIXEL_FMT_AL44            0x06
#define    AI_LCD_PIXEL_FMT_AL88            0x07

// LCD帧缓冲区首地址,这里定义在SDRAM里面.
#define    AI_LCD_FRAMEBUF_ADDR             0XC0000000
// 定义颜色像素格式,一般用RGB565
#define    AI_LCD_PIXEL_FMT                 AI_LCD_PIXEL_FMT_RGB565	

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
// LCD背光灯控制
#define    ai_lcd_backlight_on()         (PB_OUT(5) = 1)
#define    ai_lcd_backlight_off()        (PB_OUT(5) = 0)
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_LCD_H_ */
