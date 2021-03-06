#include "stm32f4xx.h"
#include "aitypes.h"
#include "aigpio.h"
#include "aidelay.h"
#include "aisys.h"
#include "ailtdc.h"
#include "ailcd.h"
#include "ailcdfont.h"

/*
********************************************************************************
*                       LCD(MCU) - FMC
* 硬件接口：
*                        RESET         -> NRST
*             LCD_CS     FMC_NE1       -> PD7
*             WR/CL      FMC_NWE       -> PD5
*                        LCD_BL        -> PB5
*                RD      FMC_NOE       -> PD4
*             LCD_RS     FMC_A18       -> PD13
*                        FMC_D15-0     -> D[15:13] : PD[10:8]
*                                         D[12:4]  : PE[15:7]
*                                         D[3:2]   : PD[1:0]
*                                         D[1:0]   : PD[15:14]
*                        T_MISO        -> PG3
*                        T_MOSI        -> PI3
*                        T_SCK         -> PH6
*                        T_PEN         -> PH7
*                        T_CS          -> PI8
********************************************************************************
*/

ai_lcd_dev_t       ai_lcd_dev;                       // 管理LCD的参数，默认为竖屏
u32                ai_brush_color = 0xff000000;      // LCD的画笔颜色，默认红色
u32                ai_bg_color = 0xffffffff;         // LCD的背景色，默认白色

/*
********************************************************************************
*    Function: ai_lcd_set_disp_dir
* Description: 设置LCD的显示方向
*       Input: dir - 0, 竖屏; 1, 横屏
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_set_disp_dir(u8 dir)
{
    ai_lcd_dev.dir = dir;
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        ai_ltdc_set_disp_dir(dir);
        ai_lcd_dev.width = ai_ltdc_dev.width;
        ai_lcd_dev.height = ai_ltdc_dev.height;
    } else {
        /* TFT—LCD操作 */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_clear
* Description: 清屏函数
*       Input: color - 清屏填充的颜色
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_clear(u32 color)
{
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        ai_ltdc_clear(color);
    } else {
        /* TFT—LCD操作 */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_init
* Description: 各个型号LCD屏幕初始化，当前只有RGB7"，所以只实现这一个
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_lcd_init(void)
{
    ai_lcd_dev.id = ai_ltdc_read_panel_id();
    
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        ai_ltdc_init();
    } else {
        /* 没有这个硬件屏幕设备，不实现 */
        return -1;
    }
    
    ai_lcd_set_disp_dir(0);
    ai_lcd_backlight_on();
    ai_lcd_clear(AI_WHITE);
    
    return 0;
}

/*
********************************************************************************
*    Function: ai_lcd_read_point
* Description: 读取LCD屏幕某个点的颜色值
*       Input: x - 点的横坐标
*              y - 点的纵坐标
*      Output: None
*      Return: 颜色值
*      Others: None
********************************************************************************
*/
u32 ai_lcd_read_point(u16 x, u16 y)
{
    u32 color = 0;
    
    if (x >= ai_lcd_dev.width || y >= ai_lcd_dev.height)
        return 0;
    
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        color = ai_ltdc_read_point(x, y); 
    } else {
        /* 没有这个硬件屏幕设备，不实现 */
        color = 0;
    }
    
    return color;
}

/*
********************************************************************************
*    Function: ai_lcd_draw_point
* Description: 画点函数
*       Input: x - 点的横坐标
*              y - 点的纵坐标
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_draw_point(u16 x, u16 y)
{
    if (x >= ai_lcd_dev.width || y >= ai_lcd_dev.height)
        return;
    
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        ai_ltdc_draw_point(x, y, ai_brush_color); 
    } else {
        /* 没有这个硬件屏幕设备，不实现 */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_fast_draw_point
* Description: 快速画点
*       Input: x     - 点的横坐标
*              y     - 点的纵坐标
*              color - 颜色值
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_fast_draw_point(u16 x, u16 y, u32 color)
{
    if (x >= ai_lcd_dev.width || y >= ai_lcd_dev.height)
        return;
    
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        ai_ltdc_draw_point(x, y, color); 
    } else {
        /* 没有这个硬件屏幕设备，不实现 */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_display_on
* Description: 开启LCD显示
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_display_on(void)
{
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        ai_ltdc_switch(AI_LTDC_ON);
    } else {
        /* 没有这个硬件屏幕设备，不实现 */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_display_off
* Description: 关闭LCD显示
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_display_off(void)
{
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        ai_ltdc_switch(AI_LTDC_OFF);
    } else {
        /* 没有这个硬件屏幕设备，不实现 */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_fill
* Description: 在指定区域内填充单个颜色
*              (sx,sy),(ex,ey)构成填充矩形对角坐标，
*              区域大小为:(ex - sx + 1) * (ey - sy + 1)
*       Input: sx, sy - 要填充的矩形区域的左上角横、纵坐标
*              ex, ey - 要填充的矩形区域的右下角横、纵坐标
*              color  - 填充使用的颜色值
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_fill(u16 sx, u16 sy, u16 ex, u16 ey, u32 color)
{
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        ai_ltdc_fill(sx, sy, ex, ey, color);
    } else {
        /* 没有这个硬件屏幕设备，不实现 */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_color_fill
* Description: 在指定区域内填充指定颜色块
*              (sx,sy),(ex,ey)构成填充矩形对角坐标，
*              区域大小为:(ex - sx + 1) * (ey - sy + 1)   
*       Input: sx, sy - 要填充的矩形区域的左上角横、纵坐标
*              ex, ey - 要填充的矩形区域的右下角横、纵坐标
*              color  - 要填充的颜色数组首地址
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_color_fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color)
{
    if (ai_lcd_dev.id != AI_LCD_TFT_MCU) {
        ai_ltdc_color_fill(sx, sy, ex, ey, color);
    } else {
        /* 没有这个硬件屏幕设备，不实现 */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_draw_line
* Description: 画一条直线,使用Bresenham直线算法，算法原理：
*              计算Δx和Δy，取两者中的最大值设为distance，然后依据distance开始打点，
*              每次循环都进行递增，对于另外一个坐标，则选择递增或者不递增。
*              算法原理地址：
*              https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html
*       Input: (x1, y1) - 起始点坐标
*              (x2, y2) - 终点坐标
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_draw_line(u16 x1, u16 y1, u16 x2, u16 y2)
{
    int i;
    int delta_x, delta_y;
    int inc_x, inc_y;          // 单步的增量方向
    int distance;
    int tmp_x = 0, tmp_y = 0, x, y;
    
    delta_x = x2 - x1;         // 计算两点之间在x方向的间距，得到画笔在x方向的步进值
    if (delta_x > 0) {         // 斜线(从左到右)
        inc_x = 1;
    } else if (delta_x == 0) { // 垂直斜线(竖线)
        inc_x = 0;
    } else {                   // 斜线(从右到左)
        inc_x = -1;
        delta_x = -delta_x;
    }
    
    delta_y = y2 - y1;         // 计算两点之间在y方向的间距，得到画笔在y方向的步进值
    if (delta_y > 0) {
        inc_y = 1;
    } else if (delta_y == 0) {
        inc_y = 0;
    } else {
        inc_y = -1;
        delta_y = -delta_y;
    }
    
    /* 计算画笔打点距离(取两个间距中的最大值) */
    if (delta_x > delta_y)
        distance = delta_x;
    else
        distance = delta_y;
    
    /* 开始打点,第一个点无效，所以i的次数加一 */
    x = x1;
    y = y1;
    for (i = 0; i <= distance + 1; i++) {
        ai_lcd_draw_point(x, y);
        
        // 判断离实际值最近的像素点
        tmp_x += delta_x;
        if (tmp_x > distance) {
            tmp_x -= distance;        // x方向越界，减去距离值，为下一次检测做准备
            x += inc_x;               // 在x方向递增打点
        }
        
        tmp_y += delta_y;
        if (tmp_y > distance) {
            tmp_y -= distance;
            y += inc_y;
        }
    }
}

/*
********************************************************************************
*    Function: ai_lcd_draw_rect
* Description: 利用址线画法算法绘制矩形
*       Input: (x1, y1) - 矩形起始点坐标
*              (x2, y2) - 矩形终点坐标
*              两个点为矩形的对角点
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_draw_rect(u16 x1, u16 y1, u16 x2, u16 y2)
{
    ai_lcd_draw_line(x1, y1, x2, y1);
    ai_lcd_draw_line(x1, y1, x1, y2);
    ai_lcd_draw_line(x1, y2, x2, y2);
    ai_lcd_draw_line(x2, y1, x2, y2);
}

/*
********************************************************************************
*    Function: ai_lcd_draw_rect
* Description: 利用址线画法算法绘制矩形
*       Input: (x0, y0) - 圆点坐标
*              r        - 圆的半径
*              两个点为矩形的对角点
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_draw_circle(u16 x0, u16 y0, u16 r)
{
    int a = 0, b = r;
    int d = 3 - (r << 1);        // 算法决策参数,用于判断下个点位置的标志
    
    if ((x0 - r < 0) || (x0 + r) > ai_lcd_dev.width
        || (y0 - r) < 0 || (y0 + r) > ai_lcd_dev.height)
        return;
    
    while (a <= b) {
        ai_lcd_draw_point(x0 + a, y0 - b);
        ai_lcd_draw_point(x0 + b, y0 - a);
        ai_lcd_draw_point(x0 + b, y0 + a);
        ai_lcd_draw_point(x0 + a, y0 + b);
        ai_lcd_draw_point(x0 - a, y0 + b);
        ai_lcd_draw_point(x0 - b, y0 + a);
        ai_lcd_draw_point(x0 - a, y0 - b);
        ai_lcd_draw_point(x0 - b, y0 - a);
        a++;
        if (d < 0) {
            d += 4 * a + 6;
        } else {
            d += 10 + 4 * (a - b);
            b--;
        }
    }  
}

/*
********************************************************************************
*    Function: ai_lcd_show_char
* Description: 在指定位置显示一个字符
*       Input: (x, y) - 起始位置坐标
*                  ch - 要显示的字符
*                size - 字体大小；12/16/24/32
*                mode - 显示模式；0，非叠加模式；1，叠加模式
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_show_char(u16 x, u16 y, u8 ch, u8 size, u8 mode)
{
    u8 i, j, tmp;
    u16 y0 = y;
    // 得到字体一个字符对应点阵集所占的字节数
    u8 char_size = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    
    ch = ch - ' ';
    for (i = 0; i < char_size; i++) {
        if (size == 12)
            tmp = asc2_1206[ch][i];
        else if (size == 16)
            tmp = asc2_1608[ch][i];
        else if (size == 24)
            tmp = asc2_2412[ch][i];
        else if (size == 32)
            tmp = asc2_3216[ch][i];
        else 
            return;
        
        for (j = 0; j < 8;  j++) {
            if (tmp & 0x80)
                ai_lcd_fast_draw_point(x, y, ai_brush_color);
            else if (mode == 0)
                ai_lcd_fast_draw_point(x, y, ai_bg_color);
            tmp <<= 1;
            y++;
            if (y >= ai_lcd_dev.height)      // 超出屏幕区域
                return;
            if ((y - y0) == size) {
                y = y0;
                x++;
                if (x >= ai_lcd_dev.width)
                    return;
                break;
            }
        }
    }
}

// m^n函数
static u32 ai_lcd_pow(u8 m, u8 n)
{
    u32 result = 1;
    
    while (n--)
        result *= m;
    
    return result;
}
    
/*
********************************************************************************
*    Function: ai_lcd_show_num
* Description: 在指定位置显示数字,高位为0,则不显示
*       Input: (x, y) - 起始位置坐标
*                 num - 要显示的数字(0~4294967295)
*                 len - 数字的位数
*                size - 字体大小
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_show_num(u16 x, u16 y, u32 num, u8 len, u8 size)
{
    u8 i, tmp;
    u8 show_enable = 0;      // 控制去掉开头的零
    
    for (i = 0; i < len; i++) {
        tmp = (num / ai_lcd_pow(10, len - i - 1)) % 10;
        if (show_enable == 0 && i < (len - 1)) {
            if (tmp == 0) {
                ai_lcd_show_char(x + (size / 2) * i, y, ' ', size, 0);
                continue;
            } else {
                show_enable = 1;
            };
        }
        ai_lcd_show_char(x + (size / 2) * i, y, tmp + '0', size, 0);
    }
}

/*
********************************************************************************
*    Function: ai_lcd_show_full_num
* Description: 在指定位置显示数字,高位为0,仍然显示
*       Input: (x, y) - 起始位置坐标
*                 num - 要显示的数字(0~999999999)
*                 len - 数字的位数
*                size - 字体大小
*                mode - [7]: 0,不填充;1,填充0.
*                       [6:1]: 保留
*                       [0]: 0,非叠加显示;1,叠加显示.
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_show_full_num(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode)
{
    u8 i, tmp;
    u8 show_enable = 0;      // 控制去掉开头的零
    
    for (i = 0; i < len; i++) {
        tmp = (num / ai_lcd_pow(10, len - i - 1)) % 10;
        if (show_enable == 0 && i < (len - 1)) {
            if (tmp == 0) {
                if (mode & 0x80)
                    ai_lcd_show_char(x + (size / 2) * i, y,
                                     '0', size, mode & 0x01);
                else
                    ai_lcd_show_char(x + (size / 2) * i, y,
                                     ' ', size, mode & 0x01);
                continue;
            } else {
                show_enable = 1;
            };
        }
        ai_lcd_show_char(x + (size / 2) * i, y, tmp + '0', size, mode & 0x01);
    }
}

/*
********************************************************************************
*    Function: ai_lcd_show_str
* Description: 在指定位置字符串
*       Input:         (x, y) - 起始位置坐标
*               width, height - 指定所写区域的大小
*                        size - 字体大小
*                           p - 字符串起始地址
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_show_str(u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p)
{
    u8 x0 = x;
    
    width += x;
    height += y;
    while ((*p <= '~') && (*p >= ' ')) {
        if (x >= width) {
            x = x0;
            y += size;
        }
        if (y >= height)
            break;
        ai_lcd_show_char(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }
}
