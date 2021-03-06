#include "stm32f4xx.h"
#include "aitypes.h"
#include "aigpio.h"
#include "aidelay.h"
#include "aiwm9825g6kh.h"

/*
******************************************************************************** 
* Description:      CLK -> PG8        -  时钟
*                   CKE -> PC3        -  时钟使能
*                   /CS -> PC2        -  片选
*                  /RAS -> PF11       -  Row Address Strobe
*                  /CAS -> PG15       -  Column Address Strobe
*                   /WE -> PC0        -  写使能
*               A[12:0] -> A[ 5: 0]   -> PF[5:0]
*                          A[ 9: 6]   -> PF[15:12]
*                          A[12:10]   -> PG[2:0]
*                          Row address:A0-A12,Column address:A0-A8    
*               BS[1:0] -> PG[ 5: 4]  -  Bank选择
*              DQ[15:0] -> DQ[15:13]  -> PD[10:8]
*                          DQ[12: 4]  -> PE[15:7]
*                          DQ[ 3: 2]  -> PD[1:0]
*                          DQ[ 1: 0]  -> PD[15:14]
*                          数据输入/输出
*             LDQM,UDQM -> PE0,PE1    - 输入/输出掩码
********************************************************************************
*/

/*
********************************************************************************
*    Function: ai_wm9825g6kh_send_cmd
* Description: 向SDRAM发送命令
*       Input:   bankn - 0, 向BANK5上面的SDRAM发送指令
*                        1, 向BANK6上面的SDRAM发送指令
*                  cmd - 指令
*                        0, 正常模式
*                        1, 时钟配置使能
*                        2, 预充电所有存储区
*                        3, 自动刷新
*                        4, 加载模式寄存器
*                        5, 自刷新
*                        6, 掉电
*              refresh - 自刷新次数(cmd=3时有效)
*               regval - 模式寄存器的定义
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
static int ai_wm9825g6kh_send_cmd(u8 bankn, u8 cmd, u8 refresh, u16 regval)
{
    u32 tmp = 0;
    u32 timeout = 0;
    
    if (bankn > 1 || cmd > 7 || refresh > 0xf)
        return -1;
    
    tmp |= cmd & 0x7;
    tmp |= 1 << (4 - bankn);
    tmp |= refresh << 5;
    tmp |= regval << 9;
    FMC_Bank5_6->SDCMR = tmp;
    
    while (FMC_Bank5_6->SDSR & (0x1 << 5)) {
        if (timeout++ > 0x1fffff)
            return -1;
    }
    
    return 0;
}

/*
********************************************************************************
*    Function: ai_wm9825g6kh_init
* Description: SDRAM初始化
*              FMC的寄存器设置要用临时变量的方式设置好值，然后赋值给寄存器，不能采用
*              先清零再赋值的方式，分两步来设置，否则SDRAM会初始化失败，这样读写SDRAM
*              会造成硬件错误.(见程序中注释的地方)
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_wm9825g6kh_init(void)
{
    u16 mode_reg_val = 0;
    u32 sdcr = 0, sdtr = 0;
    
    RCC->AHB3ENR |= 0x1;
    RCC->AHB1ENR |= 0x1 << 6 | 0x1 << 5 | 0x1 << 4 | 0x1 << 3 | 0x1 << 2;
    
    // GPIO初始化
    ai_gpio_set(GPIOC, PIN0 | PIN2 | PIN3, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set(GPIOD, 0x3 << 14 | 0x7 << 8 | 0x3, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set(GPIOE, 0x1ff << 7 | 0x3, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set(GPIOF, 0x1f << 11 | 0x3f, GPIO_MODE_AF, GPIO_OTYPE_PP,
                GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set(GPIOG, PIN15 | PIN8 | PIN5 | PIN4 | 0x7, GPIO_MODE_AF,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    
    ai_gpio_set_af(GPIOC, 0, 12);
    ai_gpio_set_af(GPIOC, 2, 12);
    ai_gpio_set_af(GPIOC, 3, 12);
    
    ai_gpio_set_af(GPIOD,  0, 12);
    ai_gpio_set_af(GPIOD,  1, 12);
    ai_gpio_set_af(GPIOD,  8, 12);
    ai_gpio_set_af(GPIOD,  9, 12);
    ai_gpio_set_af(GPIOD, 10, 12);
    ai_gpio_set_af(GPIOD, 14, 12);
    ai_gpio_set_af(GPIOD, 15, 12);
    
    ai_gpio_set_af(GPIOE,  0, 12);
    ai_gpio_set_af(GPIOE,  1, 12);
    ai_gpio_set_af(GPIOE,  7, 12);
    ai_gpio_set_af(GPIOE,  8, 12);
    ai_gpio_set_af(GPIOE,  9, 12);
    ai_gpio_set_af(GPIOE, 10, 12);
    ai_gpio_set_af(GPIOE, 11, 12);
    ai_gpio_set_af(GPIOE, 12, 12);
    ai_gpio_set_af(GPIOE, 13, 12);
    ai_gpio_set_af(GPIOE, 14, 12);
    ai_gpio_set_af(GPIOE, 15, 12);
    
    ai_gpio_set_af(GPIOF,  0, 12);
    ai_gpio_set_af(GPIOF,  1, 12);
    ai_gpio_set_af(GPIOF,  2, 12);
    ai_gpio_set_af(GPIOF,  3, 12);
    ai_gpio_set_af(GPIOF,  4, 12);
    ai_gpio_set_af(GPIOF,  5, 12);
    ai_gpio_set_af(GPIOF, 11, 12);
    ai_gpio_set_af(GPIOF, 12, 12);
    ai_gpio_set_af(GPIOF, 13, 12);
    ai_gpio_set_af(GPIOF, 14, 12);
    ai_gpio_set_af(GPIOF, 15, 12);
    
    ai_gpio_set_af(GPIOG,  0, 12);
    ai_gpio_set_af(GPIOG,  1, 12);
    ai_gpio_set_af(GPIOG,  2, 12);
    ai_gpio_set_af(GPIOG,  4, 12);
    ai_gpio_set_af(GPIOG,  5, 12);
    ai_gpio_set_af(GPIOG,  8, 12);
    ai_gpio_set_af(GPIOG, 15, 12);
    
    /* 
     * FMC初始化
     * 16bit数据位宽、允许写访问、3个CAS延迟
     * SDRAM时钟 = HCLK / 2 = 180MHz / 2 = 90MHz = 11.1ns
     *
     * 不合适的设置方式
     * FMC_Bank5_6->SDCR[0] &= ~0xffff0000;
     * FMC_Bank5_6->SDCR[0] |= ...;
     */
    sdcr |= 0x0 << 13 | 0x1 << 12 | 0x2 << 10 | 0x0 << 9
            | 0x3 << 7 | 0x1 << 6 | 0x1 << 4 | 0x2 << 2 | 0x1;
    FMC_Bank5_6->SDCR[0] = sdcr;
    
    sdtr |= 0x1 << 24 | 0x1 << 20 | 0x1 << 16 | 0x5 << 12
            | 0x5 << 8 | 0x6 << 4 | 0x1;
    FMC_Bank5_6->SDTR[0] = sdtr;
    
    // SDRAM初始化
    ai_wm9825g6kh_send_cmd(0, 1, 0, 0);
    ai_delay_us(500);    // 至少延迟200us
    ai_wm9825g6kh_send_cmd(0, 2, 0, 0);
    ai_wm9825g6kh_send_cmd(0, 3, 8, 0);
    mode_reg_val |= 0x1 << 9 | 0x0 << 7 | 0x3 << 4 | 0x0 << 3 | 0x3;
    ai_wm9825g6kh_send_cmd(0, 4, 0, mode_reg_val);
    
    /* 
     * wm9825g6kh的刷新频率为64ms，SDCLK=180 / 2 = 90MHz，行数为2^13行
     * 所以，COUNT = 64 * 1000 * 90 / 8192 - 20 = 683
     */
    FMC_Bank5_6->SDRTR = 683 << 1;
}

/*
********************************************************************************
*    Function: ai_wm9825g6kh_write_buf
* Description: 在指定地址(addr + AI_BANK5_SDRAM_ADDR)开始,连续写入nbytes个字节.
*       Input: pbuf   - 待写入的数据首字节地址
*              addr   - 要写入的地址
*              nbytes - 要写入的字节数
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_wm9825g6kh_write_buf(u8 *pbuf, u32 addr, u32 nbytes)
{
    for (; nbytes != 0; nbytes--) {
        *(vu8 *)(AI_BANK5_SDRAM_ADDR + addr) = *pbuf;
        addr++;
        pbuf++;
    }
}

/*
********************************************************************************
*    Function: ai_wm9825g6kh_write_buf
* Description: 在指定地址(addr + AI_BANK5_SDRAM_ADDR)开始,连续读取nbytes个字节.
*       Input: pbuf   - 读取的数据首字节地址
*              addr   - 要读取的地址
*              nbytes - 要读取的字节数
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_wm9825g6kh_read_buf(u8 *pbuf, u32 addr, u32 nbytes)
{
    for (; nbytes != 0; nbytes--) {
        *pbuf++ = *(vu8 *)(AI_BANK5_SDRAM_ADDR + addr);
    }
}
