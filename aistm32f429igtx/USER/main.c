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
#include "aican.h"

/*
********************************************************************************
*                        Private variables��define
********************************************************************************
*/
const u8 ai_text_buf[] = {"Apollo STM32F4 SPI TEST"};

/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 i = 0;
    u16 timeout = 0;
    u8 key = AI_KEY_ALL_UP;
    u8 data_tmp[8] = {0};
    u8 mode = 1;        // CAN����ģʽ;0,��ͨģʽ;1,����ģʽ
    u8 cnt = 0;
    int ret = 0;
    
    /* ����ʱ��180MHz */
    ai_sys_clock_init(360, 25, 2, 8);
    ai_delay_init(180);
	
    /* �����ʼ�� */
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    ai_wm9825g6kh_init();
    ai_lcd_init();
    ai_w25qxx_init();
    ai_can1_mode_init(0, 8, 4, 5, 1);        // CAN��ʼ��,������500Kbps
    ai_delay_ms(100);
     
    /* ��������Ŀ�ʼ����״̬ */
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
	ai_brush_color = AI_RED; 
    ai_lcd_show_str(30, 40, 240, 16, 16, (u8 *)"Apollo STM32 F4");
    ai_lcd_show_str(30, 60, 240, 16, 16, (u8 *)"CAN TEST");
    ai_lcd_show_str(30, 80, 240, 16, 16, (u8 *)"ATOM@ALIENTEK");
    ai_lcd_show_str(30, 100, 240, 16, 16, (u8 *)"2021/02/02");
    ai_lcd_show_str(30, 120, 240, 16, 16, (u8 *)"LoopBack Mode");
    ai_lcd_show_str(30, 140, 240, 16, 16, "KEY0:Send KEY1:Mode");
    ai_brush_color = AI_BLUE;
    
    ai_lcd_show_str(30, 160, 240, 16, 16, "Count:");
    ai_lcd_show_str(30, 180, 240, 16, 16, "Send Data:");
    ai_lcd_show_str(30, 240, 240, 16, 16, "Receive Data:");
    
    /* main loop */
    while (1) {
        key = ai_key_scan(0);
        if (key == AI_KEY1_DOWN) {
            mode = !mode;
            ai_can1_mode_init(0, 8, 4, 5, mode);
            ai_brush_color = AI_RED;
            if (mode == 0)
                ai_lcd_show_str(30, 120, 240, 16, 16, "Nnormal Mode ");
            else
                ai_lcd_show_str(30, 120, 240, 16, 16, "LoopBack Mode");
            ai_brush_color = AI_BLUE;
        }
        
        if (key == AI_KEY0_DOWN) {
            for (i = 0; i < 8; i++) {
                data_tmp[i] = cnt + i;
                if (i < 4)
                    ai_lcd_show_full_num(30 + i * 32, 200, data_tmp[i],
                                         3, 16, 0x80);
                else
                    ai_lcd_show_full_num(30 + (i - 4) * 32, 220, data_tmp[i],
                                         3, 16, 0x80);
            }
            
            ret = ai_can1_send_msg(data_tmp, 8);
            if (ret == -1)
                ai_lcd_show_str(30 + 80, 180, 240, 16, 16, "Failed");
            else
                ai_lcd_show_str(30 + 80, 180, 240, 16, 16, "OK    ");
        }
        
        ret = ai_can1_receive_msg(data_tmp);
        if (ret) {
            ai_lcd_fill(30, 260, 160, 310, AI_WHITE);
            for (i = 0; i < ret; i++) {
                if (i < 4)
                    ai_lcd_show_full_num(30 + i * 32, 260, data_tmp[i],
                                         3, 16, 0x80);
                else
                    ai_lcd_show_full_num(30 + (i - 4) * 32, 280, data_tmp[i],
                                         3, 16, 0x80);
            }
        }
        
        timeout++;
        ai_delay_ms(10);
        if (timeout == 20) {
            AI_DS0 = !AI_DS0;
            timeout = 0;
            cnt++;
            ai_lcd_show_full_num(30 + 48, 160, cnt, 3, 16, 0x80);
        }
    }
}
