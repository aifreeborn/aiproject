#include "aigpio.h"

/*
********************************************************************************
* GPIO��������
* ������
*     gpiox : GPIOA ~ GPIOI.
*     bitx  : 0 ~ 15,����IO���ű��.
*     afx   : 0 ~ 15,����AF0 ~ AF15.
*     AF0~15�������(��������г����õ�,��ϸ�����429/746�����ֲ�,Table 12):
*     AF0:MCO/SWD/SWCLK/RTC             AF1:TIM1/TIM2;        AF2:TIM3~5;
*     AF3:TIM8~11                       AF4:I2C1~I2C4;        AF5:SPI1~SPI6;
*     AF6:SPI3/SAI1;                    AF7:SPI2/3/USART1~3/UART5/SPDIFRX;
*     AF8:USART4~8/SPDIFRX/SAI2;        AF9;CAN1~2/TIM12~14/LCD/QSPI;
*     AF10:USB_OTG/USB_HS/SAI2/QSPI     AF11:ETH
*     AF12:FMC/SDIO/OTG/HS              AF13:DCIM             AF14:LCD;
*     AF15:EVENTOUT
********************************************************************************
*/
void ai_gpio_set_af(GPIO_TypeDef *gpiox, u8 bitn, u8 afx)
{  
	gpiox->AFR[bitn >> 3] &= ~( 0x0f << ((bitn & 0x07) * 4));
	gpiox->AFR[bitn >> 3] |= (u32)afx << ((bitn & 0x07) * 4);
} 

/*
********************************************************************************
* GPIOͨ������ 
* ������
*     gpiox : GPIOA ~ GPIOI.
*     bitn  : PIN0 ~ PIN15,λ����,ÿ��λ����һ��IO,��ͬʱ���ö��λ
*     mode  : ģʽѡ��
*                 0 - ����(ϵͳ��λĬ��״̬);
*                 1 - ��ͨ���;
*                 2 - ���ù���;
*                 3 - ģ������.
*     otype : �������ѡ��
*                 0 - �������;
*                 1 - ��©���.
*     ospeed: ����ٶ�����
*                 0 - 2Mhz;
*                 1 - 25Mhz;
*                 2 - 50Mhz;
*                 3 - 100Mh. 
*     pupd  : ����������
*                 0 - ����������;
*                 1 - ����;
*                 2 - ����;
*                 3 - ����.
* ע��:������ģʽ(��ͨ����/ģ������)��,OTYPE��OSPEED������Ч!!
*      �������в�����swgpio.h�о�����Ӧ�ĺ궨��,���ò�����Ӧ�ĺ궨�����
********************************************************************************
*/
void ai_gpio_set(GPIO_TypeDef *gpiox, u32 bitn,
                 u32 mode, u32 otype, u32 ospeed, u32 pupd)
{  
	u32 pin_pos = 0, pos = 0, cur_pin = 0;
    
	for (pin_pos = 0; pin_pos < 16; pin_pos++) {
		pos = 0x1 << pin_pos;
		cur_pin = bitn & pos;                            // ��������Ƿ�Ҫ����
		if (cur_pin == pos) {
			gpiox->MODER &= ~(0x3 << (pin_pos * 2));     // �����ԭ��������
			gpiox->MODER |= mode << (pin_pos * 2);	     // �����µ�ģʽ 
			// ��������ģʽ/���ù���ģʽ
            if ((mode == 0x01) || (mode == 0x02)) { 
				gpiox->OSPEEDR &= ~(0x3 << (pin_pos * 2));    // ���ԭ��������
				gpiox->OSPEEDR |= (ospeed << (pin_pos * 2));  // �����µ��ٶ�ֵ  
				gpiox->OTYPER &= ~(0x1 << pin_pos) ;          // ���ԭ��������
				gpiox->OTYPER |= otype << pin_pos;		      // �����µ����ģʽ
			}  
			gpiox->PUPDR &= ~(0x3 << (pin_pos * 2));	      // �����ԭ��������
			gpiox->PUPDR |= pupd << (pin_pos * 2);            // �����µ�������
		}
	}
} 
