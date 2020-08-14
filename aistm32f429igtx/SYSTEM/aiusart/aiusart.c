#include <aisys.h>
#include <aigpio.h>
#include "aiusart.h"

/*
********************************************************************************
*                             配置支持printf函数
********************************************************************************
*/	  
#if AI_PRINTF_SUPPORTED

#pragma import(__use_no_semihosting)  
/* 解决HAL库使用时,某些情况可能报错的bug */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 标准库需要的支持函数 */
struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
};

/* FILE is typedefed in stdio.h. */ 
FILE __stdout;

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

/* 重定义fputc函数 */ 
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);    // 循环发送,直到发送完毕   
	USART1->DR = (u8)ch;
    return ch;
}

#endif /* AI_PRINTF_SUPPORTED */

/*
********************************************************************************
*                             串口1中断服务程序
* 注意,读取USARTx->SR能避免莫名其妙的错误
********************************************************************************
*/	
#if AI_EN_USART1_RX                       // 如果使能了接收

u8 AI_USART_RX_BUF[AI_USART_REC_LEN];     // 接收缓冲,最大AI_USART_REC_LEN个字节.
u16 AI_USART_RX_STA = 0;                  // 接收状态标记 
                                          // bit15   - 接收完成标志
                                          // bit14   - 接收到0x0d
                                          // bit13~0 - 接收到的有效字节数目 

void USART1_IRQHandler(void)
{
	u8 res;	

    // 接收到数据
	if (USART1->SR &(0x1 << 5)) {	 
		res = USART1->DR;
		if ((AI_USART_RX_STA & 0x8000) == 0) {           // 接收未完成
			if (AI_USART_RX_STA & 0x4000) {              // 接收到了0x0d
				if (res != 0x0a)
                    AI_USART_RX_STA = 0;                 // 接收错误,重新开始
				else 
                    AI_USART_RX_STA |= 0x8000;           // 接收完成了 
			} else {                                     // 还没收到0X0D	
				if (res == 0x0d) {
                    AI_USART_RX_STA |= 0x4000;
				} else {
					AI_USART_RX_BUF[AI_USART_RX_STA & 0X3FFF] = res;
					AI_USART_RX_STA++;
					if (AI_USART_RX_STA > (AI_USART_REC_LEN - 1))
                        AI_USART_RX_STA = 0;        // 接收数据错误,重新开始接收	  
				}		 
			}
		}  		 									     
	} 
} 
#endif /* AI_EN_USART1_RX */

/*
********************************************************************************
*                             初始化IO 串口1
* pclk2    : PCLK2时钟频率(MHz)
* baudrate : 波特率
********************************************************************************
*/
void ai_uart_init(u32 pclk2, u32 baudrate)
{
    float temp;
    u16 mantissa;
    u16 fraction;
    
	temp = (float)(pclk2 * 1000000) / (baudrate * 16);  // 得到USARTDIV@OVER8=0
	mantissa = temp;                                    // 得到整数部分
	fraction = (temp - mantissa) * 16;                  // 得到小数部分@OVER8=0 
    mantissa <<= 4;
	mantissa += fraction;
	RCC->AHB1ENR |= 0x1 << 0;                           // 使能PORTA口时钟  
	RCC->APB2ENR |= 0x1 << 4;                           // 使能串口1时钟
	// PA9, PA10, 复用功能, 上拉输出
    ai_gpio_set(GPIOA, PIN9 | PIN10,
                GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_50M, GPIO_PUPD_PU);
 	ai_gpio_set_af(GPIOA, 9, 7);                        // PA9, AF7
	ai_gpio_set_af(GPIOA, 10, 7);                       // PA10,AF7
    
 	USART1->BRR = mantissa;                             // 波特率设置
	USART1->CR1 &= ~(0x1 << 15);                        // 设置OVER8 = 0
	USART1->CR1 |= 0x1 << 3;                            // 串口发送使能
#if AI_EN_USART1_RX                                     // 如果使能了接收
	USART1->CR1 |= 0x1 << 2;                            // 串口接收使能
	USART1->CR1 |= 0x1 << 5;                            // 接收缓冲区非空中断使能	    	
	ai_nvic_init(3, 3, USART1_IRQn, 2);                 // 组2，最低优先级
#endif
	USART1->CR1 |= 0x1 << 13;                           // 串口使能
}
