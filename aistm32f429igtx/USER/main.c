/*
********************************************************************************
*                          SYS INCLUDES
********************************************************************************
*/
#include <stdio.h>
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
#include "aiinflash.h"
#include "aimem.h"

/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 time = 0;
    u8 key = AI_KEY_ALL_UP;
    u8 *ptr = NULL, *tptr = NULL;
    u8 mem_num = AI_MEM_IN;
    u8 paddr[20];    // 存放P Addr:+p地址的ASCII值
    u16 mem_used = 0;
    
    /* 设置时钟180MHz */
    ai_sys_clock_init(360, 25, 2, 8);
    ai_delay_init(180);
	
    /* 外设初始化 */
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    ai_wm9825g6kh_init();
    ai_lcd_init();
    ai_mem_init(AI_MEM_IN);
    ai_mem_init(AI_MEM_EXT);
    ai_mem_init(AI_MEM_CCM);
     
    /* 设置外设的开始运行状态 */
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
	ai_brush_color = AI_RED; 
    ai_lcd_show_str(30, 50, 200, 16, 16, (u8 *)"Apollo STM32 F4");
    ai_lcd_show_str(30, 70, 200, 16, 16, (u8 *)"MALLOC TEST");
    ai_lcd_show_str(30, 90, 200, 16, 16, (u8 *)"ATOM@ALIENTEK");
    ai_lcd_show_str(30, 110, 200, 16, 16, (u8 *)"2021-02-13");
    ai_lcd_show_str(30, 130, 200, 16, 16, (u8 *)"KEY0:Malloc  KEY2:Free");
    ai_lcd_show_str(30, 150, 200, 16, 16, (u8 *)"KEY_UP:SRAMx KEY1:Read");
    ai_brush_color = AI_BLUE;
    ai_lcd_show_str(30, 170, 200, 16, 16, (u8 *)"SRAMIN");
    ai_lcd_show_str(30, 190, 200, 16, 16, (u8 *)"SRAMIN  USED:");
    ai_lcd_show_str(30, 210, 200, 16, 16, (u8 *)"SRAMEX  USED:");
    ai_lcd_show_str(30, 230, 200, 16, 16, (u8 *)"SRAMCCM USED:");
    
    /* main loop */
    while (1) {
        key = ai_key_scan(0);
        switch (key) {
        case AI_KEY0_DOWN:
            ptr = ai_mem_malloc(mem_num, 2048);    // 申请2K字节
            if (ptr != NULL)
                sprintf((char *)ptr, "Memory Malloc Test%03d", time);
            break;
        case AI_KEY1_DOWN:
            if (ptr != NULL) {
                sprintf((char *)ptr, "Memory Malloc Test%03d", time);
                ai_lcd_show_str(30, 270, 200, 16, 16, ptr);
            }
            break;
        case AI_KEY2_DOWN:
            ai_mem_free(mem_num, ptr);
            ptr = NULL;
            break;
        case AI_KEY_WK_DOWN:
            mem_num++;
            if (mem_num > 2)
                mem_num = 0;
            if (mem_num == AI_MEM_IN)
                ai_lcd_show_str(30, 170, 200, 16, 16, "SRAMIN ");
            else if (mem_num == AI_MEM_EXT)
                ai_lcd_show_str(30, 170, 200, 16, 16, "SDRAMEX ");
            else if (mem_num == AI_MEM_CCM)
                ai_lcd_show_str(30, 170, 200, 16, 16, "SRAMCCM"); 
            break;
        }
        
        if (ptr != NULL && tptr != ptr) {
            tptr = ptr;
            sprintf((char *)paddr, "P Addr:0X%08X", (u32)tptr);
            ai_lcd_show_str(30, 250, 200, 16, 16, paddr);
            if (ptr)
                ai_lcd_show_str(30, 270, 200, 16, 16, ptr);
            else
                ai_lcd_fill(30, 270, 239, 266, AI_WHITE);
        }
        
        time++;
        ai_delay_ms(10);
        if ((time % 20) == 0) {
            // 显示内部内存使用率
            mem_used = ai_mem_get_used(AI_MEM_IN) * 1000;
            sprintf((char *)paddr, "%d.%01d%%", mem_used / 10, mem_used % 10);
            ai_lcd_show_str(30 + 104, 190, 200, 16, 16, paddr);
            // 显示外部内存使用率
            mem_used = ai_mem_get_used(AI_MEM_EXT) * 1000;
            sprintf((char *)paddr, "%d.%01d%%", mem_used / 10, mem_used % 10);
            ai_lcd_show_str(30 + 104, 210, 200, 16, 16, paddr);
            // 显示CCM内存使用率 
            mem_used = ai_mem_get_used(AI_MEM_CCM) * 1000;
            sprintf((char *)paddr, "%d.%01d%%", mem_used / 10, mem_used % 10);
            ai_lcd_show_str(30 + 104, 230, 200, 16, 16, paddr);
            AI_DS0 = !AI_DS0;
        }
    }
}
