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
* Ӳ���ӿڣ�
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

ai_lcd_dev_t       ai_lcd_dev;                       // ����LCD�Ĳ�����Ĭ��Ϊ����
u32                ai_brush_color = 0xff000000;      // LCD�Ļ�����ɫ��Ĭ�Ϻ�ɫ
u32                ai_bg_color = 0xffffffff;         // LCD�ı���ɫ��Ĭ�ϰ�ɫ

/*
********************************************************************************
*    Function: ai_lcd_set_disp_dir
* Description: ����LCD����ʾ����
*       Input: dir - 0, ����; 1, ����
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
        /* TFT��LCD���� */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_clear
* Description: ��������
*       Input: color - ����������ɫ
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
        /* TFT��LCD���� */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_init
* Description: �����ͺ�LCD��Ļ��ʼ������ǰֻ��RGB7"������ֻʵ����һ��
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
        /* û�����Ӳ����Ļ�豸����ʵ�� */
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
* Description: ��ȡLCD��Ļĳ�������ɫֵ
*       Input: x - ��ĺ�����
*              y - ���������
*      Output: None
*      Return: ��ɫֵ
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
        /* û�����Ӳ����Ļ�豸����ʵ�� */
        color = 0;
    }
    
    return color;
}

/*
********************************************************************************
*    Function: ai_lcd_draw_point
* Description: ���㺯��
*       Input: x - ��ĺ�����
*              y - ���������
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
        /* û�����Ӳ����Ļ�豸����ʵ�� */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_fast_draw_point
* Description: ���ٻ���
*       Input: x     - ��ĺ�����
*              y     - ���������
*              color - ��ɫֵ
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
        /* û�����Ӳ����Ļ�豸����ʵ�� */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_display_on
* Description: ����LCD��ʾ
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
        /* û�����Ӳ����Ļ�豸����ʵ�� */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_display_off
* Description: �ر�LCD��ʾ
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
        /* û�����Ӳ����Ļ�豸����ʵ�� */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_fill
* Description: ��ָ����������䵥����ɫ
*              (sx,sy),(ex,ey)���������ζԽ����꣬
*              �����СΪ:(ex - sx + 1) * (ey - sy + 1)
*       Input: sx, sy - Ҫ���ľ�����������ϽǺᡢ������
*              ex, ey - Ҫ���ľ�����������½Ǻᡢ������
*              color  - ���ʹ�õ���ɫֵ
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
        /* û�����Ӳ����Ļ�豸����ʵ�� */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_color_fill
* Description: ��ָ�����������ָ����ɫ��
*              (sx,sy),(ex,ey)���������ζԽ����꣬
*              �����СΪ:(ex - sx + 1) * (ey - sy + 1)   
*       Input: sx, sy - Ҫ���ľ�����������ϽǺᡢ������
*              ex, ey - Ҫ���ľ�����������½Ǻᡢ������
*              color  - Ҫ������ɫ�����׵�ַ
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
        /* û�����Ӳ����Ļ�豸����ʵ�� */
    }
}

/*
********************************************************************************
*    Function: ai_lcd_draw_line
* Description: ��һ��ֱ��,ʹ��Bresenhamֱ���㷨���㷨ԭ��
*              ���㦤x�ͦ�y��ȡ�����е����ֵ��Ϊdistance��Ȼ������distance��ʼ��㣬
*              ÿ��ѭ�������е�������������һ�����꣬��ѡ��������߲�������
*              �㷨ԭ���ַ��
*              https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html
*       Input: (x1, y1) - ��ʼ������
*              (x2, y2) - �յ�����
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_draw_line(u16 x1, u16 y1, u16 x2, u16 y2)
{
    int i;
    int delta_x, delta_y;
    int inc_x, inc_y;          // ��������������
    int distance;
    int tmp_x = 0, tmp_y = 0, x, y;
    
    delta_x = x2 - x1;         // ��������֮����x����ļ�࣬�õ�������x����Ĳ���ֵ
    if (delta_x > 0) {         // б��(������)
        inc_x = 1;
    } else if (delta_x == 0) { // ��ֱб��(����)
        inc_x = 0;
    } else {                   // б��(���ҵ���)
        inc_x = -1;
        delta_x = -delta_x;
    }
    
    delta_y = y2 - y1;         // ��������֮����y����ļ�࣬�õ�������y����Ĳ���ֵ
    if (delta_y > 0) {
        inc_y = 1;
    } else if (delta_y == 0) {
        inc_y = 0;
    } else {
        inc_y = -1;
        delta_y = -delta_y;
    }
    
    /* ���㻭�ʴ�����(ȡ��������е����ֵ) */
    if (delta_x > delta_y)
        distance = delta_x;
    else
        distance = delta_y;
    
    /* ��ʼ���,��һ������Ч������i�Ĵ�����һ */
    x = x1;
    y = y1;
    for (i = 0; i <= distance + 1; i++) {
        ai_lcd_draw_point(x, y);
        
        // �ж���ʵ��ֵ��������ص�
        tmp_x += delta_x;
        if (tmp_x > distance) {
            tmp_x -= distance;        // x����Խ�磬��ȥ����ֵ��Ϊ��һ�μ����׼��
            x += inc_x;               // ��x����������
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
* Description: ����ַ�߻����㷨���ƾ���
*       Input: (x1, y1) - ������ʼ������
*              (x2, y2) - �����յ�����
*              ������Ϊ���εĶԽǵ�
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
* Description: ����ַ�߻����㷨���ƾ���
*       Input: (x0, y0) - Բ������
*              r        - Բ�İ뾶
*              ������Ϊ���εĶԽǵ�
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_draw_circle(u16 x0, u16 y0, u16 r)
{
    int a = 0, b = r;
    int d = 3 - (r << 1);        // �㷨���߲���,�����ж��¸���λ�õı�־
    
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
* Description: ��ָ��λ����ʾһ���ַ�
*       Input: (x, y) - ��ʼλ������
*                  ch - Ҫ��ʾ���ַ�
*                size - �����С��12/16/24/32
*                mode - ��ʾģʽ��0���ǵ���ģʽ��1������ģʽ
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_show_char(u16 x, u16 y, u8 ch, u8 size, u8 mode)
{
    u8 i, j, tmp;
    u16 y0 = y;
    // �õ�����һ���ַ���Ӧ������ռ���ֽ���
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
            if (y >= ai_lcd_dev.height)      // ������Ļ����
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

// m^n����
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
* Description: ��ָ��λ����ʾ����,��λΪ0,����ʾ
*       Input: (x, y) - ��ʼλ������
*                 num - Ҫ��ʾ������(0~4294967295)
*                 len - ���ֵ�λ��
*                size - �����С
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_show_num(u16 x, u16 y, u32 num, u8 len, u8 size)
{
    u8 i, tmp;
    u8 show_enable = 0;      // ����ȥ����ͷ����
    
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
* Description: ��ָ��λ����ʾ����,��λΪ0,��Ȼ��ʾ
*       Input: (x, y) - ��ʼλ������
*                 num - Ҫ��ʾ������(0~999999999)
*                 len - ���ֵ�λ��
*                size - �����С
*                mode - [7]: 0,�����;1,���0.
*                       [6:1]: ����
*                       [0]: 0,�ǵ�����ʾ;1,������ʾ.
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_lcd_show_full_num(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode)
{
    u8 i, tmp;
    u8 show_enable = 0;      // ����ȥ����ͷ����
    
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
* Description: ��ָ��λ���ַ���
*       Input:         (x, y) - ��ʼλ������
*               width, height - ָ����д����Ĵ�С
*                        size - �����С
*                           p - �ַ�����ʼ��ַ
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
