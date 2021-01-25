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
#include "aiadc.h"


/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 lcd_buf[40];
    u16 adc_val;
    float tmp;
    int temperature;
    
    ai_sys_clock_init(360, 25, 2, 8);    // 设置时钟180MHz
    ai_delay_init(180);
	
    /* 外设初始化 */
	ai_uart_init(90, 115200);
    // usmart_dev.init(90);
    ai_led_init();
    // ai_key_init();
    ai_wm9825g6kh_init();
    ai_lcd_init();
    ai_adc_init();
    ai_delay_ms(100);
     
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
    
    ai_brush_color = AI_BLUE;
    ai_lcd_show_str(30, 180, 200, 16, 16, "ADC1_CH5_VAL:");
    ai_lcd_show_str(30, 200, 200, 16, 16, "ADC1_CH5_VOL:0.000V");
    ai_lcd_show_str(30, 220, 200, 16, 16, "TEMPERATE: 00.00C");
     
    /* main loop */
    while (1) {
        adc_val = ai_adc_get_average_val(5, 20);
        sprintf((char *)lcd_buf, "%d", adc_val);
        printf("%s\r\n", lcd_buf);
        ai_lcd_show_full_num(134, 180, adc_val, 4, 16, 0);
        tmp = (float)adc_val * (3.3 / 4096);
        adc_val = tmp;
        ai_lcd_show_full_num(134, 200, adc_val, 1, 16, 0);
        tmp -= adc_val;
        tmp *= 1000;
        ai_lcd_show_full_num(150, 200, tmp, 3, 16, 0X80);
        
        // 温度
        temperature = ai_adc_get_temperature();
        if (temperature < 0) {
            temperature = -temperature;
            ai_lcd_show_str(30 + 10 * 8, 220, 16, 16, 16, "-" );
        } else {
            ai_lcd_show_str(30 + 10 * 8, 220, 16, 16, 16, " " );
        }
        
        ai_lcd_show_full_num(30 + 11 * 8, 220, temperature / 100, 2, 16, 0);
        ai_lcd_show_full_num(30 + 14 * 8, 220, temperature % 100, 2, 16, 0X80);
               
        AI_DS0 = !AI_DS0;
        ai_delay_ms(250);
    }
}
