/*
********************************************************************************
*                          SYS INCLUDES
********************************************************************************
*/
#include <aigpio.h>
#include <aisys.h>
#include <aidelay.h>
#include <ailed.h>
#include <aikey.h>
#include <aiusart.h>
#include "aiexti.h"

/*
********************************************************************************
*                        HARDWARE INCLUDES
********************************************************************************
*/


/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
	u8 i;
	u8 len = 0;
	u16 times = 0;
    
    ai_sys_clock_init(360, 25, 2, 8);    // 设置时钟180MHz
    ai_delay_init(180);
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    aiextix_init();
    
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
    while (1) {
		if (AI_USART_RX_STA & 0x8000) {
			len = AI_USART_RX_STA & 0x3fff;
			printf("\r\n您发送的消息为：\r\n");
			for (i = 0; i < len; i++) {
				USART1->DR = AI_USART_RX_BUF[i];
				while ((USART1->SR & 0x40) == 0)
					;
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
}
