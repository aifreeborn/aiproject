/*
********************************************************************************
*                          SYS INCLUDES
********************************************************************************
*/
#include <aigpio.h>
#include <aisys.h>
#include <aidelay.h>

/*
********************************************************************************
*                        HARDWARE INCLUDES
********************************************************************************
*/
#include "ailed.h"
#include "aikey.h"
#include "aiusart.h"
#include "aiwm9825g6kh.h"
#include "ailtdc.h"
#include "ailcd.h"
#include "aiw25qxx.h"
#include "aitouch.h"
#include "airemote.h"

/*
********************************************************************************
*                        Private variables、define
********************************************************************************
*/
// 10个触控点的颜色(电容触摸屏用)
const u16 ai_point_color_tbl[10] = {
    AI_RED, AI_GREEN, AI_BLUE, AI_BROWN, AI_GRED,
    AI_BRED, AI_GBLUE, AI_LIGHTBLUE, AI_BRRED, AI_GRAY
};

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_load_drow_dialog(void);
void ai_gui_draw_hline(u16 x0, u16 y0, u16 len, u16 color);
void ai_gui_fill_circle(u16 x0, u16 y0, u16 r, u16 color);
u16 ai_abs(u16 a, u16 b);
void ai_lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2, u8 size, u16 color);
void ai_lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2, u8 size, u16 color);
void ai_ctp_test(void);

/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 key = 0;
    u8 time = 0;
    u8 *str;
    
    /* 设置时钟180MHz */
    ai_sys_clock_init(360, 25, 2, 8);
    ai_delay_init(180);
	
    /* 外设初始化 */
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    ai_wm9825g6kh_init();
    ai_lcd_init();
    ai_delay_ms(100);
    // ai_tp_dev.init();
    ai_remote_init();
     
    /* 设置外设的开始运行状态 */
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
	ai_brush_color = AI_RED; 
    ai_lcd_show_str(30, 40, 240, 16, 16, (u8 *)"Apollo STM32 F4");
    ai_lcd_show_str(30, 60, 240, 16, 16, (u8 *)"REMOTE TEST");
    ai_lcd_show_str(30, 80, 240, 16, 16, (u8 *)"ATOM@ALIENTEK");
    ai_lcd_show_str(30, 100, 240, 16, 16, (u8 *)"KEYVAL:");
    ai_lcd_show_str(30, 120, 240, 16, 16, (u8 *)"KEYCNT:");
    ai_lcd_show_str(30, 140, 240, 16, 16, (u8 *)"SYMBOL:");
    ai_brush_color = AI_BLUE;
    
    // ai_delay_ms(1500);
    // ai_load_drow_dialog();
    // ai_ctp_test();
    /* main loop */
    while (1) {
        key = ai_remote_scan();
        if (key) {
            ai_lcd_show_num(86, 100, key, 3, 16);
            ai_lcd_show_num(86, 120, ai_remote_cnt, 3, 16);
            switch (key) {
            case 0:
                str = "ERROR";
                break;
            case 162:
                str = "POWER";
                break;
            case 98:
                str = "UP";
                break;
            case 2:
                str = "PLAY";
                break;
            case 226:
                str = "ALIENTEK";
                break;
            case 194:
                str = "RIGHT";
                break;
            case 34:
                str = "LEFT";
                break;
            case 224:
                str = "VOL-";
                break;
            case 168:
                str = "DOWN";
                break;
            case 144:
                str = "VOL+";
                break;
            case 104:
                str = "1";
                break;
            case 152:
                str = "2";
                break;
            case 176:
                str = "3";
                break;
            case 48:
                str = "4";
                break;
            case 24:
                str = "5";
                break;
            case 122:
                str = "6";
                break;
            case 16:
                str = "7";
                break;
            case 56:
                str = "8";
                break;
            case 90:
                str = "9";
                break;
            case 66:
                str = "0";
                break;
            case 82:
                str = "DELETE";
                break;
            }
            ai_lcd_fill(86, 140, 116 + 8 * 8, 170 + 16, AI_WHITE);
            ai_lcd_show_str(86, 140, 240, 16, 16, str);
        }else
            ai_delay_ms(10);
        time++;
        if (time == 20) {
            time = 0;
            AI_DS0 = !AI_DS0;
        }
    }
}

/*
********************************************************************************
*    Function: ai_load_drow_dialog
* Description: 清空屏幕并在右上角显示"RST"
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_load_drow_dialog(void)
{
    ai_lcd_clear(AI_WHITE);
    ai_brush_color = AI_BLUE;
    ai_lcd_show_str(ai_lcd_dev.width - 24, 0, 240, 16, 16, "RST");
    ai_brush_color = AI_RED;
}

/*
********************************************************************************
*    Function: ai_gui_draw_hline
* Description: 画水平线
*       Input: (x0, y0) - 坐标
*                   len - 线长度
*                 color - 颜色
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_gui_draw_hline(u16 x0, u16 y0, u16 len, u16 color)
{
    if (len == 0)
        return;
    if ((x0 + len - 1) >= ai_lcd_dev.width)
        x0 = ai_lcd_dev.width - len - 1;
    if (y0 >= ai_lcd_dev.height)
        y0 = ai_lcd_dev.height - 1;
    ai_lcd_fill(x0, y0, x0 + len - 1, y0, color);
}

/*
********************************************************************************
*    Function: ai_gui_fill_circle
* Description: 画实心圆
*       Input: (x0, y0) - 坐标
*                     r - 半径
*                 color - 颜色
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
void ai_gui_fill_circle(u16 x0, u16 y0, u16 r, u16 color)
{
    u32 i;
    u32 imax = ((u32)r * 707) / 1000 + 1;
    u32 sqmax = (u32)r * (u32)r + (u32)r / 2;
    u32 x = r;
    
    ai_gui_draw_hline(x0 - r, y0, 2 * r, color);
    for (i = 1; i <= imax; i++) {
        if ((i * i+ x * x) > sqmax) {
            if (x > imax) {
                ai_gui_draw_hline(x0 - i + 1, y0 + x, 2 * (i - 1), color);
                ai_gui_draw_hline(x0 - i + 1, y0 - x, 2 * (i - 1), color);
            }
            x--;
        }
        
        ai_gui_draw_hline(x0 - x, y0 + i, 2 * x, color);
        ai_gui_draw_hline(x0 - x, y0 - i, 2 * x, color);
    }
    
}

/*
********************************************************************************
*    Function: ai_abs
* Description: 两个数之差的绝对值
*       Input: a, b -> 需取差值的两个数
*      Output: None
*      Return: |a - b|
*      Others: None
********************************************************************************
*/
u16 ai_abs(u16 a, u16 b)
{
    if (a > b)
        return a - b;
    else
        return b - a;
}

/*
********************************************************************************
*    Function: ai_lcd_draw_bline
* Description: 画一条粗线
*       Input: (x1,y1),(x2,y2) -> 线条的起始坐标
*                         size -> 线条的粗细程度
*                        color -> 线条的颜色
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2, u8 size, u16 color)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, urow, ucol;
    
    if (x1 < size || x2 < size || y1 < size || y2 < size)
        return;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    urow = x1;
    ucol = y1;
    
    if (delta_x > 0) {
        incx = 1;                           // 设置单步方向
    } else if (delta_x == 0) {
        incx = 0;                           // 垂直线
    } else {
        incx = -1;
        delta_x = -delta_x;
    }
    
    if (delta_y > 0) {
        incy = 1;
    } else if (delta_y == 0) {
        incy = 0;                           // 水平线
    } else {
        incy = -1;
        delta_y = -delta_y;
    }
    
    if (delta_x > delta_y)
        distance = delta_x;                 // 选取基本增量坐标轴
    else
        distance = delta_y; 
	
    for (t = 0; t <= distance + 1; t++) {   // 画线输出
        ai_gui_fill_circle(urow, ucol, size, color);    // 画点
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            urow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            ucol += incy;
        }
    }
}

/*
********************************************************************************
*    Function: ai_ctp_test
* Description: 电容触摸屏测试函数
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ctp_test(void)
{
    u8 i = 0, t = 0;
    u16 last_pos[5][2] = {0};    // 最后一次的数据
    
    while (1) {
        ai_tp_dev.scan(0);
        for (i = 0; i < 5; i++) {
            if (ai_tp_dev.stat & (0x1 << i)) {
                if ((ai_tp_dev.x[i] < ai_lcd_dev.width)
                    && (ai_tp_dev.y[i] < ai_lcd_dev.height)) {
                    if (last_pos[i][0] == 0xffff) {
                        last_pos[i][0] = ai_tp_dev.x[i];
                        last_pos[i][1] = ai_tp_dev.y[i];
                    }
                    ai_lcd_draw_bline(last_pos[i][0], last_pos[i][1],
                                      ai_tp_dev.x[i], ai_tp_dev.y[i],
                                      2, ai_point_color_tbl[i]);
                    last_pos[i][0] = ai_tp_dev.x[i];
                    last_pos[i][1] = ai_tp_dev.y[i];
                    if ((ai_tp_dev.x[i] > (ai_lcd_dev.width - 24))
                        && (ai_tp_dev.y[i] < 20)) {
                        ai_load_drow_dialog();
                    }
                }
            } else {
                last_pos[i][0] = 0xffff;
            }
        }
        
        ai_delay_ms(5);
        t++;
        if (t % 20 == 0)
            AI_DS0 = !AI_DS0;
    }
}
