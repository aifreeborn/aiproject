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
#include "aipcf8574.h"
#include "aimpu9250.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

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

// MPU9250测试
void ai_usart1_niming_report(u8 fun, u8 *data, u8 len);
void ai_mpu9250_send_data(short ax, short ay, short az,
                          short gx, short gy, short gz);
void ai_usart1_report_imu(short roll, short pitch,
                          short yaw, short csb, int prs);

/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 time = 0;
    u8 report = 1;             // 默认开启上报
    u8 key = AI_KEY_ALL_UP;
    float pitch, roll, yaw;    // 欧拉角
    short ax, ay, az;          // 加速度传感器原始数据
    short gx, gy, gz;          // 陀螺仪原始数据
    float temp;                // 温度
    short tmp = 0;
    
    /* 设置时钟180MHz */
    ai_sys_clock_init(360, 25, 2, 8);
    ai_delay_init(180);
	
    /* 外设初始化 */
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    ai_wm9825g6kh_init();
    ai_lcd_init();
    ai_mpu9250_init();
    ai_delay_ms(100);
    ai_pcf8574_init();
    ai_pcf8574_read_bit(AI_PCF8574_INT);
     
    /* 设置外设的开始运行状态 */
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
	ai_brush_color = AI_RED; 
    ai_lcd_show_str(30, 50, 200, 16, 16, (u8 *)"Apollo STM32 F4");
    ai_lcd_show_str(30, 70, 200, 16, 16, (u8 *)"MPU9250 TEST");
    ai_lcd_show_str(30, 90, 200, 16, 16, (u8 *)"ATOM@ALIENTEK");
    ai_lcd_show_str(30, 110, 200, 16, 16, (u8 *)"2021-02-08");
    
    while (ai_mpu_dmp_init()) {
        ai_lcd_show_str(30, 130, 200, 16, 16, "MPU9250 Error");
        ai_delay_ms(200);
        ai_lcd_fill(30, 130, 239, 130 + 16, AI_WHITE);
        ai_delay_ms(200);
        AI_DS0 = !AI_DS0;
    }
    ai_lcd_show_str(30, 130, 200, 16, 16, "MPU9250 OK");
    ai_lcd_show_str(30, 150, 200, 16, 16, "KEY0:UPLOAD ON/OFF");
    ai_brush_color = AI_BLUE;
    ai_lcd_show_str(30, 170, 200, 16, 16, "UPLOAD ON ");
    ai_lcd_show_str(30, 200, 200, 16, 16, " Temp:    . C");
    ai_lcd_show_str(30, 220, 200, 16, 16, "Pitch:    . C");
    ai_lcd_show_str(30, 240, 200, 16, 16, " Roll:    . C");
    ai_lcd_show_str(30, 260, 200, 16, 16, " Yaw :    . C");
    
    /* main loop */
    while (1) {
        key = ai_key_scan(0);
        if (key == AI_KEY0_DOWN) {
            report = !report;
            if (report)
                ai_lcd_show_str(30, 170, 200, 16, 16, "UPLOAD ON ");
            else
                ai_lcd_show_str(30, 170, 200, 16, 16, "UPLOAD OFF");
        }
        
        if (ai_mpu_mpl_get_data(&pitch, &roll, &yaw) == 0) {
            temp = ai_mpu9250_get_temperature();
            ai_mpu9250_get_accelerometer(&ax, &ay, &az);
            ai_mpu9250_get_gyroscope(&gx, &gy, &gz);
            if (report) {
                ai_mpu9250_send_data(ax, ay, az, gx, gy, gz);
                ai_usart1_report_imu((int)(roll * 100),
                                     (int)(pitch * 100),
                                     (int)(yaw * 100),
                                      0, 0);
            }
            
            if ((time % 10) == 0) {
                if (temp < 0) {
                    ai_lcd_show_char(30 + 48, 200, '-', 16, 0);
                    temp = -temp;
                } else {
                    ai_lcd_show_char(30 + 48, 200, ' ', 16, 0);
                }
                tmp = temp * 10;
                ai_lcd_show_num(30 + 48 + 8, 200, tmp / 10, 3, 16);
                ai_lcd_show_num(30 + 48 + 40, 200, tmp % 10, 1, 16);
                
                tmp = pitch * 10;
                if (tmp < 0) {
                    ai_lcd_show_char(30 + 48, 220, '-', 16, 0);
                    tmp = -tmp;
                } else {
                    ai_lcd_show_char(30 + 48, 220, ' ', 16, 0);
                }
                ai_lcd_show_num(30 + 48 + 8, 220, tmp / 10, 3, 16);
                ai_lcd_show_num(30 + 48 + 40, 220, tmp % 10, 1, 16);
                
                tmp = roll * 10;
                if (tmp < 0) {
                    ai_lcd_show_char(30 + 48, 240, '-', 16, 0);
                    tmp = -tmp;
                } else {
                    ai_lcd_show_char(30 + 48, 240, ' ', 16, 0);
                }
                ai_lcd_show_num(30 + 48 + 8, 240, tmp / 10, 3, 16);
                ai_lcd_show_num(30 + 48 + 40, 240, tmp % 10, 1, 16);
                
                tmp = yaw * 10;
                if (tmp < 0) {
                    ai_lcd_show_char(30 + 48, 260, '-', 16, 0);
                    tmp = -tmp;
                } else {
                    ai_lcd_show_char(30 + 48, 260, ' ', 16, 0);
                }
                ai_lcd_show_num(30 + 48 + 8, 260, tmp / 10, 3, 16);
                ai_lcd_show_num(30 + 48 + 40, 260, tmp % 10, 1, 16);
                
                time = 0;
                AI_DS0 = !AI_DS0;
            }
        }
        time++;
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

/*
********************************************************************************
*    Function: ai_usart1_niming_report
* Description: 传送数据给匿名四轴地面站(V4版本)
*       Input: fun -> 功能字. 0X01~0X1C
*             data -> 数据缓存区,最多28字节!!
*              len -> data区有效数据个数
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_usart1_niming_report(u8 fun, u8 *data, u8 len)
{
    u8 buf[32];
    u8 i;
    
    if (len > 28)
        return;
    buf[len + 3] = 0;
    buf[0] = 0xaa;
    buf[1] = 0xaa;
    buf[2] = fun;
    buf[3] = len;
    for (i = 0; i < len; i++) {         // 复制数据
        buf[4 + i] = data[i];
    }
    for (i = 0; i < len + 4; i++) {     // 计算校验和
        buf[4 + len] += data[i];
    }
    for (i = 0; i < len + 5; i++) {     // 发送数据到串口1 
        ai_usart_putc(buf[i]);
    }
}

/*
********************************************************************************
*    Function: ai_mpu_send_data
* Description: 发送加速度传感器数据+陀螺仪数据(传感器帧)
*       Input: ax,ay,az -> x,y,z三个方向上面的加速度值
*              gx,gy,gz -> x,y,z三个方向上面的陀螺仪值
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_mpu9250_send_data(short ax, short ay, short az,
                          short gx, short gy, short gz)
{
    u8 tbuf[18] = {0};
    
    tbuf[0] = (ax >> 8) & 0xff;
    tbuf[1] = ax & 0xff;
    tbuf[2] = (ay >> 8) & 0xff;
    tbuf[3] = ay & 0xff;
    tbuf[4] = (az >> 8) & 0xff;
    tbuf[5] = az & 0xff;
    
    tbuf[6] =(gx >> 8) & 0xff;
    tbuf[7] = gx & 0xff;
    tbuf[8] =(gy >> 8) & 0xff;
    tbuf[9] = gy & 0xff;
    tbuf[10] =(gz >> 8) & 0xff;
    tbuf[11] = gz & 0xff;
    // 因为开启MPL后,无法直接读取磁力计数据,所以这里直接屏蔽掉.用0替代.
    tbuf[12] = 0;
    tbuf[13] = 0;
    tbuf[14] = 0;
    tbuf[15] = 0;
    tbuf[16] = 0;
    tbuf[17] = 0;
	ai_usart1_niming_report(0x02, tbuf, 18);    // 传感器帧,0x02
}

/*
********************************************************************************
*    Function: ai_usart1_report_imu
* Description: 通过串口1上报结算后的姿态数据给电脑(状态帧)
*       Input: roll -> 横滚角.单位0.01度 [-18000,18000]对应[-180.00,180.00]度
*             pitch -> 俯仰角.单位 0.01度。-9000 - 9000 对应 -90.00 -> 90.00 度
*               yaw -> 航向角.单位为0.1度 0 -> 3600  对应 0 -> 360.0度
*               csb -> 超声波高度,单位:cm
*               prs -> 气压计高度,单位:mm
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_usart1_report_imu(short roll, short pitch,
                          short yaw, short csb, int prs)
{
    u8 tbuf[12] = {0};
    
    tbuf[0] = (roll >> 8) & 0xff;
    tbuf[1] = roll & 0xff;
    tbuf[2] = (pitch >> 8) & 0xff;
    tbuf[3] = pitch & 0xff;
    tbuf[4] = (yaw >> 8) & 0xff;
    tbuf[5] = yaw & 0xff;
    tbuf[6] = (csb >> 8) & 0xff;
    tbuf[7] = csb & 0xff;
    tbuf[8] = (prs >> 24) & 0xff;
    tbuf[9] = (prs >> 16) & 0xff;
    tbuf[10] = (prs >> 8) & 0xff;
    tbuf[11] = prs & 0xff;
    ai_usart1_niming_report(0x01, tbuf, 12);    // 状态帧,0X01
}
