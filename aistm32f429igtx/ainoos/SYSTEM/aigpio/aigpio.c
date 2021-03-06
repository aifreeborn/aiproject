#include "aigpio.h"

/*
********************************************************************************
* GPIO复用设置
* 参数：
*     gpiox : GPIOA ~ GPIOI.
*     bitx  : 0 ~ 15,代表IO引脚编号.
*     afx   : 0 ~ 15,代表AF0 ~ AF15.
*     AF0~15设置情况(这里仅是列出常用的,详细的请见429/746数据手册,Table 12):
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
* GPIO通用设置 
* 参数：
*     gpiox : GPIOA ~ GPIOI.
*     bitn  : PIN0 ~ PIN15,位设置,每个位代表一个IO,可同时设置多个位
*     mode  : 模式选择，
*                 0 - 输入(系统复位默认状态);
*                 1 - 普通输出;
*                 2 - 复用功能;
*                 3 - 模拟输入.
*     otype : 输出类型选择
*                 0 - 推挽输出;
*                 1 - 开漏输出.
*     ospeed: 输出速度设置
*                 0 - 2Mhz;
*                 1 - 25Mhz;
*                 2 - 50Mhz;
*                 3 - 100Mh. 
*     pupd  : 上下拉设置
*                 0 - 不带上下拉;
*                 1 - 上拉;
*                 2 - 下拉;
*                 3 - 保留.
* 注意:在输入模式(普通输入/模拟输入)下,OTYPE和OSPEED参数无效!!
*      上面所有参数在swgpio.h中均有相应的宏定义,调用采用相应的宏定义进行
********************************************************************************
*/
void ai_gpio_set(GPIO_TypeDef *gpiox, u32 bitn,
                 u32 mode, u32 otype, u32 ospeed, u32 pupd)
{  
	u32 pin_pos = 0, pos = 0, cur_pin = 0;
    
	for (pin_pos = 0; pin_pos < 16; pin_pos++) {
		pos = 0x1 << pin_pos;
		cur_pin = bitn & pos;                            // 检查引脚是否要设置
		if (cur_pin == pos) {
			gpiox->MODER &= ~(0x3 << (pin_pos * 2));     // 先清除原来的设置
			gpiox->MODER |= mode << (pin_pos * 2);	     // 设置新的模式 
			// 如果是输出模式/复用功能模式
            if ((mode == 0x01) || (mode == 0x02)) { 
				gpiox->OSPEEDR &= ~(0x3 << (pin_pos * 2));    // 清除原来的设置
				gpiox->OSPEEDR |= (ospeed << (pin_pos * 2));  // 设置新的速度值  
				gpiox->OTYPER &= ~(0x1 << pin_pos) ;          // 清除原来的设置
				gpiox->OTYPER |= otype << pin_pos;		      // 设置新的输出模式
			}  
			gpiox->PUPDR &= ~(0x3 << (pin_pos * 2));	      // 先清除原来的设置
			gpiox->PUPDR |= pupd << (pin_pos * 2);            // 设置新的上下拉
		}
	}
} 
