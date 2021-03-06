#include "stm32f4xx.h"
#include "aitypes.h"
#include "aigpio.h"
#include "aidelay.h"
#include "aiwm9825g6kh.h"

/*
******************************************************************************** 
* Description:      CLK -> PG8        -  ʱ��
*                   CKE -> PC3        -  ʱ��ʹ��
*                   /CS -> PC2        -  Ƭѡ
*                  /RAS -> PF11       -  Row Address Strobe
*                  /CAS -> PG15       -  Column Address Strobe
*                   /WE -> PC0        -  дʹ��
*               A[12:0] -> A[ 5: 0]   -> PF[5:0]
*                          A[ 9: 6]   -> PF[15:12]
*                          A[12:10]   -> PG[2:0]
*                          Row address:A0-A12,Column address:A0-A8    
*               BS[1:0] -> PG[ 5: 4]  -  Bankѡ��
*              DQ[15:0] -> DQ[15:13]  -> PD[10:8]
*                          DQ[12: 4]  -> PE[15:7]
*                          DQ[ 3: 2]  -> PD[1:0]
*                          DQ[ 1: 0]  -> PD[15:14]
*                          ��������/���
*             LDQM,UDQM -> PE0,PE1    - ����/�������
********************************************************************************
*/

/*
********************************************************************************
*    Function: ai_wm9825g6kh_send_cmd
* Description: ��SDRAM��������
*       Input:   bankn - 0, ��BANK5�����SDRAM����ָ��
*                        1, ��BANK6�����SDRAM����ָ��
*                  cmd - ָ��
*                        0, ����ģʽ
*                        1, ʱ������ʹ��
*                        2, Ԥ������д洢��
*                        3, �Զ�ˢ��
*                        4, ����ģʽ�Ĵ���
*                        5, ��ˢ��
*                        6, ����
*              refresh - ��ˢ�´���(cmd=3ʱ��Ч)
*               regval - ģʽ�Ĵ����Ķ���
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
* Description: SDRAM��ʼ��
*              FMC�ļĴ�������Ҫ����ʱ�����ķ�ʽ���ú�ֵ��Ȼ��ֵ���Ĵ��������ܲ���
*              �������ٸ�ֵ�ķ�ʽ�������������ã�����SDRAM���ʼ��ʧ�ܣ�������дSDRAM
*              �����Ӳ������.(��������ע�͵ĵط�)
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
    
    // GPIO��ʼ��
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
     * FMC��ʼ��
     * 16bit����λ������д���ʡ�3��CAS�ӳ�
     * SDRAMʱ�� = HCLK / 2 = 180MHz / 2 = 90MHz = 11.1ns
     *
     * �����ʵ����÷�ʽ
     * FMC_Bank5_6->SDCR[0] &= ~0xffff0000;
     * FMC_Bank5_6->SDCR[0] |= ...;
     */
    sdcr |= 0x0 << 13 | 0x1 << 12 | 0x2 << 10 | 0x0 << 9
            | 0x3 << 7 | 0x1 << 6 | 0x1 << 4 | 0x2 << 2 | 0x1;
    FMC_Bank5_6->SDCR[0] = sdcr;
    
    sdtr |= 0x1 << 24 | 0x1 << 20 | 0x1 << 16 | 0x5 << 12
            | 0x5 << 8 | 0x6 << 4 | 0x1;
    FMC_Bank5_6->SDTR[0] = sdtr;
    
    // SDRAM��ʼ��
    ai_wm9825g6kh_send_cmd(0, 1, 0, 0);
    ai_delay_us(500);    // �����ӳ�200us
    ai_wm9825g6kh_send_cmd(0, 2, 0, 0);
    ai_wm9825g6kh_send_cmd(0, 3, 8, 0);
    mode_reg_val |= 0x1 << 9 | 0x0 << 7 | 0x3 << 4 | 0x0 << 3 | 0x3;
    ai_wm9825g6kh_send_cmd(0, 4, 0, mode_reg_val);
    
    /* 
     * wm9825g6kh��ˢ��Ƶ��Ϊ64ms��SDCLK=180 / 2 = 90MHz������Ϊ2^13��
     * ���ԣ�COUNT = 64 * 1000 * 90 / 8192 - 20 = 683
     */
    FMC_Bank5_6->SDRTR = 683 << 1;
}

/*
********************************************************************************
*    Function: ai_wm9825g6kh_write_buf
* Description: ��ָ����ַ(addr + AI_BANK5_SDRAM_ADDR)��ʼ,����д��nbytes���ֽ�.
*       Input: pbuf   - ��д����������ֽڵ�ַ
*              addr   - Ҫд��ĵ�ַ
*              nbytes - Ҫд����ֽ���
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
* Description: ��ָ����ַ(addr + AI_BANK5_SDRAM_ADDR)��ʼ,������ȡnbytes���ֽ�.
*       Input: pbuf   - ��ȡ���������ֽڵ�ַ
*              addr   - Ҫ��ȡ�ĵ�ַ
*              nbytes - Ҫ��ȡ���ֽ���
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
