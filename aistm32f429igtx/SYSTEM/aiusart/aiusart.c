#include <aisys.h>
#include <aigpio.h>
#include "aiusart.h"

/*
********************************************************************************
*                             ����֧��printf����
********************************************************************************
*/	  
#if AI_PRINTF_SUPPORTED

#pragma import(__use_no_semihosting)  
/* ���HAL��ʹ��ʱ,ĳЩ������ܱ����bug */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* ��׼����Ҫ��֧�ֺ��� */
struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
};

/* FILE is typedefed in stdio.h. */ 
FILE __stdout;

/* ����_sys_exit()�Ա���ʹ�ð�����ģʽ */
void _sys_exit(int x)
{
    x = x;
}

/* �ض���fputc���� */ 
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);    // ѭ������,ֱ���������   
	USART1->DR = (u8)ch;
    return ch;
}

#endif /* AI_PRINTF_SUPPORTED */

/*
********************************************************************************
*                             ����1�жϷ������
* ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
********************************************************************************
*/	
#if AI_EN_USART1_RX                       // ���ʹ���˽���

u8 AI_USART_RX_BUF[AI_USART_REC_LEN];     // ���ջ���,���AI_USART_REC_LEN���ֽ�.
u16 AI_USART_RX_STA = 0;                  // ����״̬��� 
                                          // bit15   - ������ɱ�־
                                          // bit14   - ���յ�0x0d
                                          // bit13~0 - ���յ�����Ч�ֽ���Ŀ 

void USART1_IRQHandler(void)
{
	u8 res;	

    // ���յ�����
	if (USART1->SR &(0x1 << 5)) {	 
		res = USART1->DR;
		if ((AI_USART_RX_STA & 0x8000) == 0) {           // ����δ���
			if (AI_USART_RX_STA & 0x4000) {              // ���յ���0x0d
				if (res != 0x0a)
                    AI_USART_RX_STA = 0;                 // ���մ���,���¿�ʼ
				else 
                    AI_USART_RX_STA |= 0x8000;           // ��������� 
			} else {                                     // ��û�յ�0X0D	
				if (res == 0x0d) {
                    AI_USART_RX_STA |= 0x4000;
				} else {
					AI_USART_RX_BUF[AI_USART_RX_STA & 0X3FFF] = res;
					AI_USART_RX_STA++;
					if (AI_USART_RX_STA > (AI_USART_REC_LEN - 1))
                        AI_USART_RX_STA = 0;        // �������ݴ���,���¿�ʼ����	  
				}		 
			}
		}  		 									     
	} 
} 
#endif /* AI_EN_USART1_RX */

/*
********************************************************************************
*                             ��ʼ��IO ����1
* pclk2    : PCLK2ʱ��Ƶ��(MHz)
* baudrate : ������
********************************************************************************
*/
void ai_uart_init(u32 pclk2, u32 baudrate)
{
    float temp;
    u16 mantissa;
    u16 fraction;
    
	temp = (float)(pclk2 * 1000000) / (baudrate * 16);  // �õ�USARTDIV@OVER8=0
	mantissa = temp;                                    // �õ���������
	fraction = (temp - mantissa) * 16;                  // �õ�С������@OVER8=0 
    mantissa <<= 4;
	mantissa += fraction;
	RCC->AHB1ENR |= 0x1 << 0;                           // ʹ��PORTA��ʱ��  
	RCC->APB2ENR |= 0x1 << 4;                           // ʹ�ܴ���1ʱ��
	// PA9, PA10, ���ù���, �������
    ai_gpio_set(GPIOA, PIN9 | PIN10,
                GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_50M, GPIO_PUPD_PU);
 	ai_gpio_set_af(GPIOA, 9, 7);                        // PA9, AF7
	ai_gpio_set_af(GPIOA, 10, 7);                       // PA10,AF7
    
 	USART1->BRR = mantissa;                             // ����������
	USART1->CR1 &= ~(0x1 << 15);                        // ����OVER8 = 0
	USART1->CR1 |= 0x1 << 3;                            // ���ڷ���ʹ��
#if AI_EN_USART1_RX                                     // ���ʹ���˽���
	USART1->CR1 |= 0x1 << 2;                            // ���ڽ���ʹ��
	USART1->CR1 |= 0x1 << 5;                            // ���ջ������ǿ��ж�ʹ��	    	
	ai_nvic_init(3, 3, USART1_IRQn, 2);                 // ��2��������ȼ�
#endif
	USART1->CR1 |= 0x1 << 13;                           // ����ʹ��
}
