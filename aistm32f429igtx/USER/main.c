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
#include "aiexti.h"
#include "aiiwdg.h"
#include "aitimer.h"
#include "aiwwdg.h"

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_test_key(void);
void ai_test_usart(void);
void ai_test_iwdg(void);

/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    ai_sys_clock_init(360, 25, 2, 8);    // 设置时钟180MHz
    ai_delay_init(180);
    
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    // 10KHz的计数频率，计数5K次为500ms
    ai_timer3_interrupt_init(5000 - 1, 9000 - 1);
    // aiextix_init();
    // ai_delay_ms(100);
    // 预分频数为128，重载值为500，溢出时间为2s
    // ai_iwdg_init(5, 500); 
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
#ifdef AI_TEST_WWDG
    ai_delay_ms(1000);
    ai_wwdg_init(0x7f, 0x5f, 3);
#endif
    
    while (1) {
		/* main loop */
#ifdef AI_TEST_WWDG
        ai_led_off(AI_LED_DS0);
#endif
        AI_DS0 = !AI_DS0;
        ai_delay_ms(200);
        // ai_test_iwdg();
	}
}

/*
********************************************************************************
*                              test functions
********************************************************************************
*/
void ai_test_key(void)
{
    u8 key;
    
    key = ai_key_scan(0);
    if (key != AI_KEY_ALL_UP) {
        switch (key) {
        case AI_KEY_WK_DOWN:
            AI_DS0 = !AI_DS0;
            AI_DS1 = !AI_DS0;
            break;
        case AI_KEY0_DOWN:
            AI_DS0 = !AI_DS0;
            break;
        case AI_KEY1_DOWN:
            AI_DS1 = !AI_DS1;
            break;
        case AI_KEY2_DOWN:
            AI_DS0 = !AI_DS0;
            AI_DS1 = !AI_DS1;
            break;
        }
    } else {
        ai_delay_ms(10);
    }
}

void ai_test_usart(void)
{
    u8 i;
    static u8 len = 0;
    static u16 times = 0;
    
    if (AI_USART_RX_STA & 0x8000) {
        len = AI_USART_RX_STA & 0x3fff;
        printf("\r\n您发送的消息为：\r\n");
        for (i = 0; i < len; i++) {
            USART1->DR = AI_USART_RX_BUF[i];
            while ((USART1->SR & 0x40) == 0)
                ; /* empty statments */
		}
        printf("\r\n\r\n");
        AI_USART_RX_STA = 0;
    } else {
        times++;
        if (times % 5000 == 0) {
            printf("\r\nSTM32F429IGTx开发板 - 串口实验\r\n");
            printf("takeno\r\n\r\n\r\n");
        }
        
        if (times % 200 == 0)
            printf("请输入数据，以回车键结束\r\n");
        ai_delay_ms(10);
    }
}

void ai_test_iwdg(void)
{
    if (ai_key_scan(0) == AI_KEY_WK_DOWN) {
        ai_iwdg_feed();
        printf("ai_iwdg_feed success.\r\n\r\n");
    }
    ai_delay_ms(10);
}
