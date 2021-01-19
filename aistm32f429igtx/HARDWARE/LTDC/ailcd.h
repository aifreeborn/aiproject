#ifndef AI_LCD_H_
#define AI_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "aigpio.h"
#include "aitypes.h"

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
#define    AI_LCD_TFT_MCU                   0x0000
    
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

/* 定义画笔颜色 */
#define    AI_WHITE        0xffff
#define    AI_BLACK        0x0000
#define    AI_BLUE         0x001f
#define    AI_RED          0xf800
#define    AI_GREEN        0x07e0 
#define    AI_BRED         0XF81F
#define    AI_GRED         0XFFE0
#define    AI_GBLUE        0X07FF
#define    AI_MAGENTA      0xF81F
#define    AI_CYAN         0x7FFF
#define    AI_YELLOW       0xFFE0
#define    AI_BROWN        0XBC40 //棕色
#define    AI_BRRED        0XFC07 //棕红色
#define    AI_GRAY         0X8430 //灰色
#define    AI_DARKBLUE     0X01CF	//深蓝色
#define    AI_LIGHTBLUE    0X7D7C	//浅蓝色  
#define    AI_GRAYBLUE     0X5458 //灰蓝色
#define    AI_LIGHTGREEN   0X841F //浅绿色
#define    AI_LGRAY        0XC618 //浅灰色(PANNEL),窗体背景色

#define    AI_LGRAYBLUE    0XA651 //浅灰蓝色(中间层颜色)
#define    AI_LBBLUE       0X2B12 //浅棕蓝色(选择条目的反色)

/*
********************************************************************************
*                           LCD基本寄存器、参数定义
********************************************************************************
*/ 
typedef struct {
    u16    width;     // LCD宽度
    u16    height;    // LCD高度
    u16    id;        // LCD ID
     u8    dir;       // 横竖屏 - 0，竖屏；1，横屏
    u16	   wr_gram;   // 开始写GRAM指令
    u16    coord_x;   // 设置x坐标
    u16    coord_y;   // 设置y坐标
} ai_lcd_dev_t;

/*
 * 使用NOR/SRAM的Bank1.sector1,地址位HADDR[27,26]=00 A18作为数据命令区分线 
 * 注意设置时STM32内部会右移一位对其!
 */
// #define    AI_LCD_BASE        ((u32)(0x60000000 | 0x0007FFFE))
// #define    AI_LCD             ((ai_lcd_t *) AI_LCD_BASE)

extern     ai_lcd_dev_t       ai_lcd_dev;
extern     u32                ai_brush_color;      // LCD的画笔颜色，默认红色
extern     u32                ai_bg_color;         // LCD的背景色，默认白色

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
// LCD背光灯控制
#define    ai_lcd_backlight_on()         (PB_OUT(5) = 1)
#define    ai_lcd_backlight_off()        (PB_OUT(5) = 0)


int ai_lcd_init(void);
void ai_lcd_set_disp_dir(u8 dir);
void ai_lcd_clear(u32 color);
u32 ai_lcd_read_point(u16 x, u16 y);
void ai_lcd_draw_point(u16 x, u16 y);
void ai_lcd_fast_draw_point(u16 x, u16 y, u32 color);
void ai_lcd_display_on(void);
void ai_lcd_display_off(void);
void ai_lcd_fill(u16 sx, u16 sy, u16 ex, u16 ey, u32 color);
void ai_lcd_color_fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color);

// LCD图形绘制
void ai_lcd_draw_line(u16 x1, u16 y1, u16 x2, u16 y2);
void ai_lcd_draw_rect(u16 x1, u16 y1, u16 x2, u16 y2);
void ai_lcd_draw_circle(u16 x0, u16 y0, u16 r);
void ai_lcd_show_char(u16 x, u16 y, u8 ch, u8 size, u8 mode);
void ai_lcd_show_num(u16 x, u16 y, u32 num, u8 len, u8 size);
void ai_lcd_show_full_num(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode);
void ai_lcd_show_str(u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_LCD_H_ */
