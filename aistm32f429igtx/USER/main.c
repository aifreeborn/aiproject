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
#include "aidac.h"
#include "aitimer.h"


/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 lcd_buf[40];
    u16 adc_val, dac_val = 0, dac_tmp;
    float tmp;
    int temperature;
    u8 key = AI_KEY_ALL_UP;
    u8 time = 0;
    
    ai_sys_clock_init(360, 25, 2, 8);    // 设置时钟180MHz
    ai_delay_init(180);
	
    /* 外设初始化 */
	ai_uart_init(90, 115200);
    // usmart_dev.init(90);
    ai_led_init();
    ai_key_init();
    ai_wm9825g6kh_init();
    ai_lcd_init();
    ai_adc_init();
    ai_timer9_ch2_pwm_init(255, 1);
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
    ai_lcd_show_str(30, 240, 200, 16, 16, "KEY0:+  KEY1:-");
    ai_lcd_show_str(30, 260, 200, 16, 16, "DAC VAL:");
    ai_lcd_show_str(30, 280, 200, 16, 16, "DAC VOL:0.000V");
    ai_timer9_pwm_dac_set(0);
     
    /* main loop */
    while (1) {
        time++;
        key = ai_key_scan(0);
        if (key == AI_KEY0_DOWN) {
            if (dac_val < 250) {
                dac_val += 10;
                if (dac_val >= 250)
                    dac_val = 250;
            }
            ai_timer9_pwm_dac_set(dac_val);
        } else if (key == AI_KEY1_DOWN) {
            if (dac_val > 10) {
                dac_val -= 10;
            }
            ai_timer9_pwm_dac_set(dac_val);
        }
        
        if (time == 10 || key == AI_KEY0_DOWN || key == AI_KEY1_DOWN) {
            dac_tmp = ai_timer9_pwm_dac_get();
            ai_lcd_show_full_num(94, 260, dac_tmp, 4, 16, 0);
            tmp = (float)dac_tmp * (3.3 / 255);
            dac_tmp = tmp;
            ai_lcd_show_full_num(94, 280, dac_tmp, 1, 16, 0);
            tmp -= dac_tmp;
            tmp *= 1000;
            ai_lcd_show_full_num(110, 280, tmp, 3, 16, 0X80);
            
            // ADC获取模拟数值
            adc_val = ai_adc_get_average_val(5, 20);
            sprintf((char *)lcd_buf, "%d", adc_val);
            printf("%s\r\n", lcd_buf);
            ai_lcd_show_full_num(134, 180, adc_val, 4, 16, 0);
            tmp = (float)adc_val * (3.3 / 4095);
            adc_val = tmp;
            ai_lcd_show_full_num(134, 200, adc_val, 1, 16, 0);
            tmp -= adc_val;
            tmp *= 1000;
            ai_lcd_show_full_num(150, 200, tmp, 3, 16, 0X80);
            
            AI_DS0 = !AI_DS0;
            time = 0;
        }
        
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
               
        ai_delay_ms(10);
    }
}
