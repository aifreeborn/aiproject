#include "aisys.h"    

/*
********************************************************************************
*                           Private variables
********************************************************************************
*/                                                         
const static  uint8_t ai_apb_ahb_pre_tbl[16] = {0, 0, 0, 0,
                                                1, 2, 3, 4,
                                                1, 2, 3, 4,
                                                6, 7, 8, 9};

/*
********************************************************************************
* 设置向量表偏移地址
* base_addr: 向量表基址
* offset:偏移量
********************************************************************************
*/
void ai_nvic_set_vector_table(u32 base_addr, u32 offset)
{
    // 设置NVIC的向量表偏移寄存器,VTOR低9位保留,即[8:0]保留
	SCB->VTOR = base_addr | (offset & (u32)0xFFFFFE00);
}

/*
********************************************************************************
* 设置NVIC分组
* NVIC分组 0~4 总共5组
********************************************************************************
*/	   
void ai_nvic_priority_group_cfg(u8 group)
{
    u32 tmp,tmp1;
    
	tmp1 = (~group) & 0x07;   // 取后三位
	tmp1 <<= 8;
	tmp = SCB->AIRCR;         // 读取先前的设置
	tmp &= 0X0000F8FF;        // 清空先前分组
	tmp |= 0X05FA0000;        // 写入钥匙
	tmp |= tmp1;	   
	SCB->AIRCR = tmp;         //设置分组	    	  				   
}
	
/*
********************************************************************************
* 设置NVIC
* preempt_priority   :    抢占优先级
* sub_priority       :    响应优先级
* channel            :    中断编号
* group              :    中断分组 0~4
* 注意优先级不能超过设定的组的范围!否则会有意想不到的错误
* 组划分:
*        组0: 0位抢占优先级,4位响应优先级
*        组1: 1位抢占优先级,3位响应优先级
*        组2: 2位抢占优先级,2位响应优先级
*        组3: 3位抢占优先级,1位响应优先级
*        组4: 4位抢占优先级,0位响应优先级
* sub_priority和preemption_priority的原则是,数值越小,越优先
********************************************************************************
*/
void ai_nvic_init(u8 preempt_priority, u8 sub_priority, u8 channel, u8 group)
{
	u32 temp;
    
	ai_nvic_priority_group_cfg(group);
	temp = preempt_priority << (4 - group);
	temp |= sub_priority & (0x0f >> group);
	temp &= 0xf;
	// 使能中断位(要清除的话,设置ICER对应位为1即可)
    NVIC->ISER[channel / 32] |= 0x1 << channel % 32;
    // 设置响应优先级和抢断优先级
	NVIC->IP[channel] |= temp << 4;
}

/*
********************************************************************************
* 外部中断配置函数
* 只针对GPIOA~I;不包括PVD,RTC,USB_OTG,USB_HS,以太网唤醒等
* 参数:
*     gpiox    : 0~8,代表GPIOA~I
*     bitn     : 需要使能的位;
*     trim_mode: 触发模式,1,下升沿;2,上降沿;3，任意电平触发
*     该函数一次只能配置1个IO口,多个IO口,需多次调用
*     该函数会自动开启对应中断,以及屏蔽线
********************************************************************************
*/
void ai_nvic_exti_cfg(u8 gpiox, u8 bitn, u8 trim_mode)
{
	u8 exti_offset = (bitn % 4) * 4;
	RCC->APB2ENR |= 0x1 << 14;  						  // 使能SYSCFG时钟
	SYSCFG->EXTICR[bitn / 4] &= ~(0x000F << exti_offset); // 清除原来设置
	// EXTI.BITx映射到GPIOx.BITx,例如所有的GPIOx.0对应中断EXTI0
    SYSCFG->EXTICR[bitn / 4] |= gpiox << exti_offset;
	// 自动设置
	EXTI->IMR |= 0x1 << bitn;                        // 开启line BITx上的中断
	if (trim_mode & 0x01) EXTI->FTSR |= 0x1 << bitn; // line BITx上事件下降沿触发
	if (trim_mode & 0x02) EXTI->RTSR |= 0x1 << bitn; // line BITx上事件上升降沿触发
}

/*
********************************************************************************
*                             汇编函数
*
* THUMB指令不支持汇编内联,采用如下方法实现执行汇编指令WFI
********************************************************************************
*/
static void ai_interrupt_wfi_set(void)
{
    __ASM volatile("wfi");
}

/*
********************************************************************************
* 关闭所有中断(但是不包括fault和NMI中断)
********************************************************************************
*/
void ai_interrupt_disable_all(void)
{
    __ASM volatile("cpsid i");
}

/*
********************************************************************************
* 开启所有中断
********************************************************************************
*/
void ai_interrupt_enable_all(void)
{
    __ASM volatile("cpsie i");
}

/*
********************************************************************************
* 设置栈顶地址
* addr:栈顶地址
********************************************************************************
*/
__asm void ai_stack_msr_msp(u32 addr)
{
    MSR MSP, r0
    BX r14
}

/*
********************************************************************************
* 进入待机模式
********************************************************************************
*/
void ai_sys_standby(void)
{ 
	SCB->SCR |= 0x1 << 2;        // 使能SLEEPDEEP位
	RCC->APB1ENR |= 0x1 << 28;   // 使能电源时钟 
	PWR->CSR |= 0x1 << 8;        // 设置WKUP用于唤醒
	PWR->CR |= 0x1 << 2;         // 清除Wake-up 标志
	PWR->CR |= 0x1 << 1;         // PDDS置位   	
	ai_interrupt_wfi_set();      // 执行WFI指令,进入待机模式		 
}

/*
********************************************************************************
* 系统软复位
********************************************************************************
*/  
void ai_sys_soft_reset(void)
{
    SCB->AIRCR = 0X05FA0000 | (u32)0x04;	  
}

/*
********************************************************************************
* 时钟设置函数
* Fvco = Fs * (plln / pllm);
* Fsys = Fvco / pllp = Fs * (plln / (pllm * pllp));
* Fusb = Fvco / pllq = Fs * (plln / (pllm * pllq));
* 
* Fvco : VCO频率
* Fsys : 系统时钟频率
* Fusb : USB,SDIO,RNG等的时钟频率
* Fs   : PLL输入时钟频率,可以是HSI,HSE等. 
* plln : 主PLL倍频系数(PLL倍频),取值范围:64~432.
* pllm : 主PLL和音频PLL分频系数(PLL之前的分频),取值范围:2~63.
* pllp : 系统时钟的主PLL分频系数(PLL之后的分频),取值范围:2,4,6,8.(仅限这4个值!)
* pllq : USB/SDIO/随机数产生器等的主PLL分频系数(PLL之后的分频),取值范围:2~15.
* 
* 外部晶振为25M的时候,推荐值:plln=360,pllm=25,pllp=2,pllq=8.
* 得到:Fvco = 25 * (360 / 25) = 360Mhz
*      Fsys = 360 / 2 = 180Mhz
*      Fusb = 360 / 8 = 45Mhz（使用USB时，需设置plln=384，即可得到48Mhz频率）
* 返回值:0,成功;1,失败。
********************************************************************************
*/
static u8 ai_sys_clock_set(u32 plln, u32 pllm, u32 pllp, u32 pllq)
{
    u16 retry = 0;
    u8 status = 0;
    
    RCC->CR |= 0x1 << 16;				// HSE 开启
	while (((RCC->CR & (0x1 << 17)) == 0) && (retry < 0X1FFF))
        retry++;                        // 等待HSE RDY
	if (retry == 0X1FFF) {
        status = 1;                     // HSE无法就绪
    } else {
		RCC->APB1ENR |= 0x1 << 28;      // 电源接口时钟使能
		PWR->CR |= 3 << 14;             // 高性能模式,时钟可到180Mhz
        // HCLK 不分频;APB1 4分频;APB2 2分频.
		RCC->CFGR |= (0 << 4) | (5 << 10) | (4 << 13);
		RCC->CR &= ~(0x1 << 24);        // 关闭主PLL
        // 配置主PLL,PLL时钟源来自HSE
		RCC->PLLCFGR = pllm | (plln << 6) | (((pllp >> 1) - 1) << 16)
                       | (pllq << 24) | (1 << 22);
		RCC->CR |= 0x1 << 24;           // 打开主PLL
		while ((RCC->CR & (0x1 << 25)) == 0);        // 等待PLL准备好 
		FLASH->ACR |= 0x1 << 8;                      // 指令预取使能.
		FLASH->ACR |= 0x1 << 9;                      // 指令cache使能.
		FLASH->ACR |= 0x1 << 10;                     // 数据cache使能.
		FLASH->ACR |= 5 << 0;                        // 5个CPU等待周期. 
		RCC->CFGR &= ~( 3 << 0);                     // 清零
		RCC->CFGR |= 2 << 0;                         // 选择主PLL作为系统时钟	 
		while ((RCC->CFGR &( 3 << 2)) != (2 << 2));  // 等待主PLL作为系统时钟成功. 
	} 
	return status;
}  

/*
********************************************************************************
* 系统时钟初始化函数
* plln : 主PLL倍频系数(PLL倍频),取值范围:64~432.
* pllm : 主PLL和音频PLL分频系数(PLL之前的分频),取值范围:2~63.
* pllp : 系统时钟的主PLL分频系数(PLL之后的分频),取值范围:2,4,6,8.(仅限这4个值!)
* pllq : USB/SDIO/随机数产生器等的主PLL分频系数(PLL之后的分频),取值范围:2~15.
********************************************************************************
*/
void ai_sys_clock_init(u32 plln, u32 pllm, u32 pllp, u32 pllq)
{
    RCC->CR |= 0x00000001;		              // 设置HISON,开启内部高速RC振荡
	RCC->CFGR = 0x00000000;                   // CFGR清零 
	RCC->CR &= 0xFEF6FFFF;                    // HSEON, CSSON, PLLON清零 
	RCC->PLLCFGR = 0x24003010;                // PLLCFGR恢复复位值 
	RCC->CR &= ~(0x1 << 18);                  // HSEBYP清零,外部晶振不旁路
	RCC->CIR = 0x00000000;                    // 禁止RCC时钟中断 
	ai_sys_clock_set(plln, pllm, pllp, pllq); // 设置时钟 
	// 配置向量表				  
#ifdef  VECT_TAB_RAM
    ai_nvic_set_vector_table(0x1 << 29, 0x0);
#else   
	ai_nvic_set_vector_table(0x0, 0x0);
#endif 
}		    

/*
********************************************************************************
*    Function: ai_sys_get_sysclk_freq
* Description: Returns the SYSCLK frequency
*       Input: void
*      Output: None
*      Return: SYSCLK frequency
*      Others: The system frequency computed by this function is not the real
*              frequency in the chip. It is calculated based on the predefined 
*              constant and the selected clock source:
*              1) If SYSCLK source is HSI, function returns values based on 
*                 HSI_VALUE(*)
*              2) If SYSCLK source is HSE, function returns values based on
*                 HSE_VALUE(**)
*              3) If SYSCLK source is PLL, function returns values based on 
*                 HSE_VALUE(**) or HSI_VALUE(*) multiplied/divided by
*                 the PLL factors.         
*
*              The result of this function could be not correct when using
*              fractional value for HSE crystal.
*              This function can be used by the user application to compute the
*              baudrate for the communication peripherals or configure other
*              parameters.
*              note: Each time SYSCLK changes, this function must be called to
*                    update the right SYSCLK value. Otherwise, any configuration
*                    based on this function will be incorrect.
********************************************************************************
*/
uint32_t ai_sys_get_sysclk_freq(void)
{
    uint32_t pllm = 0, plln = 0, pllvco = 0, pllp = 0;
    uint32_t sysclockfreq = 0;
    
    // Get SYSCLK source
    switch (RCC->CFGR & RCC_CFGR_SWS) {
    case RCC_CFGR_SWS_HSI:        /* HSI used as system clock source */
        {
            sysclockfreq = AI_HSI_VALUE;
            break;
        }
    case RCC_CFGR_SWS_HSE:        /* HSE used as system clock  source */
        {
            sysclockfreq = AI_HSE_VALUE;
            break;
        }
    case RCC_CFGR_SWS_PLL:        /* PLL used as system clock  source */
        {
            /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLLM) * PLLN
             * SYSCLK = PLL_VCO / PLLP
             */
            pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
            if (AI_RCC_GET_PLL_OSCSOURCE() != AI_RCC_PLLSOURCE_HSI) {
                /* HSE used as PLL clock source */
                plln = RCC->PLLCFGR & RCC_PLLCFGR_PLLN;
                pllvco = ((AI_HSE_VALUE / pllm)
                          * (plln >> POSITION_VAL(RCC_PLLCFGR_PLLN)));
            } else {
                /* HSI used as PLL clock source */
                plln = RCC->PLLCFGR & RCC_PLLCFGR_PLLN;
                pllvco = ((AI_HSI_VALUE / pllm)
                          * (plln >> POSITION_VAL(RCC_PLLCFGR_PLLN)));
            }
            pllp = ((((RCC->PLLCFGR & RCC_PLLCFGR_PLLP)
                       >> POSITION_VAL(RCC_PLLCFGR_PLLP)) + 1) * 2);
            sysclockfreq = pllvco / pllp;
            break;
        }
    default:
        {
            sysclockfreq = AI_HSI_VALUE;
            break;
        }
    }
    
    return sysclockfreq;
}

/*
********************************************************************************
*    Function: ai_sys_get_hclk_freq
* Description: Returns the HCLK frequency.
*       Input: void
*      Output: None
*      Return: HCLK frequency
*      Others: Each time HCLK changes, this function must be called to update
*              the right HCLK value. Otherwise, any configuration based on this
*              function will be incorrect.
********************************************************************************
*/
uint32_t ai_sys_get_hclk_freq(void)
{
    uint32_t hclk_freq = AI_HSI_VALUE;
    uint32_t idx = 0;
    
    idx = (RCC->CFGR & RCC_CFGR_HPRE) >> POSITION_VAL(RCC_CFGR_HPRE);
    hclk_freq = ai_sys_get_sysclk_freq() >> ai_apb_ahb_pre_tbl[idx];
    return hclk_freq;
}
