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
#include "aiat24cxx.h"

/*
********************************************************************************
*                        Private variables、define
********************************************************************************
*/
const u8 ai_text_buf[] = {"ALIENTEK Apollo STM32F429 IIC TEST2"};

/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u16 timeout = 0;
    u8 key = AI_KEY_ALL_UP;
    u8 data_tmp[sizeof(ai_text_buf)] = {0};
    
    /* 设置时钟180MHz */
    ai_sys_clock_init(360, 25, 2, 8);
    ai_delay_init(180);
	
    /* 外设初始化 */
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    ai_wm9825g6kh_init();
    ai_lcd_init();
    ai_at24cxx_init();
    ai_delay_ms(100);
     
    /* 设置外设的开始运行状态 */
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
	ai_brush_color = AI_RED; 
    ai_lcd_show_str(10, 40, 240, 16, 16, (u8 *)"Apollo STM32");
    ai_lcd_show_str(10, 60, 240, 16, 16, (u8 *)"IIC TEST");
    ai_lcd_show_str(10, 80, 240, 16, 16, (u8 *)"ATOM@ALIENTEK");
    ai_lcd_show_str(10, 100, 240, 16, 16, (u8 *)"2021/01/28");
    ai_lcd_show_str(10, 120, 240, 16, 16, "KEY1:Write  KEY0:Read");
    
    while (ai_at24cxx_check() != 0) {
        ai_lcd_show_str(10, 140, 240, 16, 16, "24C02 Check Failed!");
        ai_delay_ms(500);
        ai_lcd_show_str(10, 140, 240, 16, 16, "Please Check!      ");
        ai_delay_ms(500);
        AI_DS0 = !AI_DS0;
    }
    ai_lcd_show_str(10, 140, 240, 16, 16, "24C02 Ready!");
    ai_brush_color = AI_BLUE;
    
    /* main loop */
    while (1) {
        key = ai_key_scan(0);
        if (key == AI_KEY1_DOWN) {
            ai_lcd_fill(0, 160, 500, 319, AI_WHITE);
            ai_lcd_show_str(10, 160, 240, 16, 16, "Start Write 24C02....");
            ai_at24cxx_write(0, (u8 *)ai_text_buf, sizeof(ai_text_buf));
            ai_lcd_show_str(10, 160, 240, 16, 16, "24C02 Write Finished!");
            // 提示传送完成
        }
        if (key == AI_KEY0_DOWN) {
            ai_lcd_show_str(10, 160, 240, 16, 16, "Start Read 24C02.... ");
            ai_at24cxx_read(0, data_tmp, sizeof(ai_text_buf));
            ai_lcd_show_str(10, 160, 240, 16, 16, "The Data Readed Is:  ");
            ai_lcd_show_str(10, 180, 500, 16, 16, data_tmp); // 显示读到的字符串
        }
        
        timeout++;
        ai_delay_ms(10);
        if (timeout == 20) {
            AI_DS0 = !AI_DS0;
            timeout = 0;
        }
    }
}
