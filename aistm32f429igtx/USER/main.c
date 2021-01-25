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
#include "aiwkup.h"


/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 lcd_buf[40];
    
    ai_sys_clock_init(360, 25, 2, 8);    // 设置时钟180MHz
    ai_delay_init(180);
	
    /* 外设初始化 */
	ai_uart_init(90, 115200);
    // usmart_dev.init(90);
    ai_led_init();
    ai_key_init();
    ai_wm9825g6kh_init();
    ai_lcd_init();
    ai_delay_ms(100);
    ai_wkup_init();
     
    /* 设置外设的开始运行状态 */
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
	ai_brush_color = AI_RED;

    sprintf((char*)lcd_buf, "LCD ID:%04X", ai_lcd_dev.id);
    ai_lcd_show_str(10, 40, 240, 32, 32, (u8 *)"Apollo STM32");
    ai_lcd_show_str(10, 80, 240, 24, 24, (u8 *)"LTDC LCD TEST");
    ai_lcd_show_str(10, 110, 240, 16, 16, (u8 *)"ATOM@ALIENTEK");
    ai_lcd_show_str(10, 130, 240, 16, 16, lcd_buf);     //显示LCD ID
    ai_lcd_show_str(10, 150, 240, 12, 12, (u8 *)"2021/01/24");
    
    
    /* main loop */
    while (1) {
        AI_DS1 = !AI_DS1;
        ai_delay_ms(250);
    }
}
