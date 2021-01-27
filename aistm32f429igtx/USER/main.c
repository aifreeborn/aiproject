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
#include "aidma.h"

/*
********************************************************************************
*                        Private variables��define
********************************************************************************
*/
// �������ݳ���,��õ���sizeof(ai_text_to_send)+2��������.
#define SEND_BUF_SIZE 7800

const u8 ai_text_to_send[] = {"ALIENTEK Apollo STM32F4 DMA ����ʵ��"};
u8 ai_send_buf[SEND_BUF_SIZE]; // �������ݻ�����

/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 lcd_buf[40];
    u8 key = AI_KEY_ALL_UP;
    int i;
    u8 t = 0, tmp, mask = 0;
    float pro = 0;    // ����
    
    ai_sys_clock_init(360, 25, 2, 8);    // ����ʱ��180MHz
    ai_delay_init(180);
	
    /* �����ʼ�� */
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    ai_wm9825g6kh_init();
    ai_lcd_init();
    ai_dma_config(DMA2_Stream7, 4, (u32)&USART1->DR,
                  (u32)ai_send_buf, SEND_BUF_SIZE);
    ai_delay_ms(100);
     
    /* ��������Ŀ�ʼ����״̬ */
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
	ai_brush_color = AI_RED;

    sprintf((char*)lcd_buf, "LCD ID:%04X", ai_lcd_dev.id);
    ai_lcd_show_str(10, 40, 240, 32, 32, (u8 *)"Apollo STM32");
    ai_lcd_show_str(10, 80, 240, 24, 24, (u8 *)"LTDC LCD TEST");
    ai_lcd_show_str(10, 110, 240, 16, 16, (u8 *)"ATOM@ALIENTEK");
    ai_lcd_show_str(10, 130, 240, 16, 16, lcd_buf);     //��ʾLCD ID
    ai_lcd_show_str(10, 150, 240, 12, 12, (u8 *)"2021/01/27");
    ai_lcd_show_str(10, 170, 240, 16, 16, "KEY0:Start");
    ai_brush_color = AI_BLUE;
    
    tmp = sizeof(ai_text_to_send);
    for (i = 0; i < SEND_BUF_SIZE; i++) {
        if (t >= tmp) {
            // ���뻻�з�
            if (mask) {
                ai_send_buf[i] = 0x0a;
                t = 0;
            } else {
                ai_send_buf[i] = 0x0d;
                mask++;
            }
        } else {
            mask = 0;
            ai_send_buf[i] = ai_text_to_send[t];
            t++;
        }
    }
    i = 0;
     
    /* main loop */
    while (1) {
        key = ai_key_scan(0);
        if (key == AI_KEY0_DOWN) {
            printf("\r\nDMA DATA:\r\n");
            ai_lcd_show_str(30, 190, 200, 16, 16, "Start Transimit....");
            // ��ʾ�ٷֺ�
            ai_lcd_show_str(30, 210, 200, 16, 16, "   %");
            // ʹ�ܴ���1��DMA����
            USART1->CR3 = 0x1 << 7;
            ai_dma_enable(DMA2_Stream7, SEND_BUF_SIZE);
            
            // �ȴ�DMA������ɣ���ʱ������������һЩ�£����
            // ʵ��Ӧ���У����������ڼ䣬����ִ�����������
            while (1) {
                if (DMA2->HISR & (0x1 << 27)) {    // �ȴ�DMA2_Steam7�������
                    DMA2->HIFCR |= 0x1 << 27;
                    break;
                }
                pro = DMA2_Stream7->NDTR;
                pro = 1 - pro / SEND_BUF_SIZE;
                pro *= 100;
                ai_lcd_show_num(30, 210, pro, 3, 16);
            }
            ai_lcd_show_num(30, 210, 100, 3, 16);    // ��ʾ100%	
            ai_lcd_show_str(30, 190, 200, 16, 16, "Transimit Finished!");
        }
        i++;
        ai_delay_ms(10);
        if (i == 20) {
            AI_DS0 = !AI_DS0;
            i = 0;
        }
    }
}
