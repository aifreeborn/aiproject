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

/*
********************************************************************************
*                        Private variables、define
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
    u16 timeout = 0;
    u32 flash_size = 32 * 1024 * 1024;
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
    ai_w25qxx_init();
    ai_delay_ms(100);
     
    /* 设置外设的开始运行状态 */
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
	ai_brush_color = AI_RED; 
    ai_lcd_show_str(30, 40, 240, 16, 16, (u8 *)"Apollo STM32");
    ai_lcd_show_str(30, 60, 240, 16, 16, (u8 *)"SPI FLASH TEST");
    ai_lcd_show_str(30, 80, 240, 16, 16, (u8 *)"ATOM@ALIENTEK");
    ai_lcd_show_str(30, 100, 240, 16, 16, (u8 *)"2021/01/31");
    ai_lcd_show_str(30, 120, 240, 16, 16, "KEY1:Write  KEY0:Read");
    while (ai_w25qxx_read_id() != AI_W25Q256) {
        ai_lcd_show_str(30, 140, 240, 16, 16, "W25Q256 Check Failed!");
        ai_delay_ms(500);
        ai_lcd_show_str(30, 140, 240, 16, 16, "Please Check!        ");
        ai_delay_ms(500);
        AI_DS0 = !AI_DS0;
    }
    ai_lcd_show_str(30, 140, 240, 16, 16, "W25Q256 Ready!");
    ai_brush_color = AI_BLUE;
    
    /* main loop */
    while (1) {
        key = ai_key_scan(0);
        if (key == AI_KEY1_DOWN) {
            ai_lcd_fill(0, 160, 239, 319, AI_WHITE);
            ai_lcd_show_str(30, 160, 200, 16, 16, "Start Write W25Q256....");
            // 从倒数第100个地址处开始,写入sizeof(ai_text_buf)长度的数据
            ai_w25qxx_write((u8 *)ai_text_buf, flash_size - 100,
                             sizeof(ai_text_buf));
            ai_lcd_show_str(30, 160, 200, 16, 16, "W25Q256 Write Finished!");
        }
        
        if (key == AI_KEY0_DOWN) {
            ai_lcd_show_str(30, 160, 200, 16, 16, "Start Read W25Q256.... ");
            ai_w25qxx_read(data_tmp, flash_size - 100, sizeof(ai_text_buf));
            ai_lcd_show_str(30, 160, 200, 16, 16, "The Data Readed Is:   ");
            ai_lcd_show_str(30, 180, 200, 16, 16, data_tmp);
        }
        
        timeout++;
        ai_delay_ms(10);
        if (timeout == 20) {
            AI_DS0 = !AI_DS0;
            timeout = 0;
        }
    }
}
