#include <stdio.h>
#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aisys.h"
#include "airtc.h"
#include "ailed.h"

/*
********************************************************************************
*    Function: ai_rtc_read_bkr
* Description: ��ȡRTC���ݼĴ�������ֵ
*       Input: num - ���ݼĴ����ı�ţ���0��ʼ����ΧΪ0~19
*      Output: None
*      Return: ��Ӧ���ݼĴ���������
*      Others: None
********************************************************************************
*/
u32 ai_rtc_read_bkr(u8 num)
{
    u32 tmp = 0;
    
    // ���ݼĴ�����ƫ�Ƶ�ַΪ0x50 to 0x9c
    tmp = RTC_BASE + 0x50 + num * 4;
    return (*(u32 *)tmp);
}

/*
********************************************************************************
*    Function: ai_rtc_write_bkr
* Description: дRTC���ݼĴ���
*       Input: num - ���ݼĴ����ı�ţ���0��ʼ����ΧΪ0~19
*              val - ��д��Ĵ�������ֵ
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_rtc_write_bkr(u8 num, u32 val)
{
    u32 tmp = 0;
    
    // ���ݼĴ�����ƫ�Ƶ�ַΪ0x50 to 0x9c
    tmp = RTC_BASE + 0x50 + num * 4;
    (*(u32 *)tmp) = val;
}

/*
********************************************************************************
*    Function: ai_rtc_init_mode
* Description: RTC�����ʼ��ģʽ
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
static int ai_rtc_init_mode(void)
{
    u32 timeout = 0xfffff;
    
    if (RTC->ISR & (0x1 << 6))        // ������������Ĵ���
        return 0;
    RTC->ISR |= 0x1 << 7;             // �����ʼ��ģʽ
    while (timeout && ((RTC->ISR & (0x1 << 6)) == 0x00))
        timeout--;
    
    if (timeout == 0)
        return -1;
    else
        return 0;
}

/*
********************************************************************************
*    Function: ai_rtc_wait_sync
* Description: �ȴ�RSF(�Ĵ���ͬ����־)ͬ��
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
static int ai_rtc_wait_sync(void)
{
    u32 timeout = 0xfffff;
    
    RTC->WPR = 0xca;
    RTC->WPR = 0x53;
    RTC->ISR &= ~(0x1 << 5);     // ���RSF��־λ
    while (timeout && ((RTC->ISR & (0x1 << 5)) == 0x00))
        timeout--;
    
    if (timeout == 0)
        return -1;
    RTC->WPR = 0xff;
    return 0;
}

/*
********************************************************************************
*    Function: ai_rtc_dec2bcd
* Description: ʮ����ת��ΪBCD��
*       Input: val - Ҫת����ʮ������,ȡֵ0~99
*      Output: None
*      Return: BCD��
*      Others: None
********************************************************************************
*/
u8 ai_rtc_dec2bcd(u8 val)
{
    u8 high = (val % 100) / 10;
    
    return ((u8)(high << 4) | (val % 10));
}

/*
********************************************************************************
*    Function: ai_rtc_bcd2dec
* Description: BCD��ת��Ϊʮ��������
*       Input: val - Ҫת����BCD��
*      Output: None
*      Return: ʮ��������
*      Others: None
********************************************************************************
*/
u8 ai_rtc_bcd2dec(u8 val)
{
    u8 tmp = 0;
    
    tmp = (val >> 4) * 10;
    return (tmp + (val & 0x0f));
}

/*
********************************************************************************
*    Function: ai_rtc_set_time
* Description: RTCʱ������
*       Input: hour - Сʱ
*               min - ����
*               sec - ��
*             am_pm - �������绹������; 0,AM/24H; 1,PM
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_rtc_set_time(u8 hour, u8 min, u8 sec, u8 am_pm)
{
    u32 tmp = 0;
    
    RTC->WPR = 0xca;
    RTC->WPR = 0x53;
    if (ai_rtc_init_mode() < 0)
        return -1;
    
    tmp = (((u32)am_pm & 0x1) << 22) | ((u32)ai_rtc_dec2bcd(hour) << 16)
          | ((u32)ai_rtc_dec2bcd(min) << 8) | ((u32)ai_rtc_dec2bcd(sec));
    RTC->TR = tmp;
    RTC->ISR &= ~(0x1 << 7);
    
    return 0;
}

/*
********************************************************************************
*    Function: ai_rtc_get_time
* Description: ��ȡRTCʱ��
*       Input: void
*      Output: hour - Сʱ
*               min - ����
*               sec - ��
*             am_pm - �������绹������; 0,AM/24H; 1,PM
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_rtc_get_time(u8 *hour, u8 *min, u8 *sec, u8 *am_pm)
{
    u32 tmp = 0;
    
    while (ai_rtc_wait_sync() != 0)
        ; /* Empty */
    
    tmp = RTC->TR;
    *hour = ai_rtc_bcd2dec((tmp >> 16) & 0x3f);
    *min = ai_rtc_bcd2dec((tmp >> 8) & 0x7f);
    *sec = ai_rtc_bcd2dec(tmp & 0x7f);
    *am_pm = (tmp >> 22) & 0x1;
}

/*
********************************************************************************
*    Function: ai_rtc_set_date
* Description: RTC��������
*       Input:  year - ��,0~99
*              month - ��,1~12
*                day - ��,0~31
*               week - ��,1~7,0Ϊ�Ƿ�
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_rtc_set_date(u8 year, u8 month, u8 day, u8 week)
{
    u32 tmp = 0;
    
    RTC->WPR = 0xca;
    RTC->WPR = 0x53;
    if (ai_rtc_init_mode() < 0)
        return -1;
    
    tmp = ((u32)ai_rtc_dec2bcd(year) << 16)
          | (((u32)ai_rtc_dec2bcd(week) & 0x7) << 13)
          | (((u32)ai_rtc_dec2bcd(month) & 0x1f) << 8)
          | ((u32)ai_rtc_dec2bcd(day) & 0x3f);
    RTC->DR = tmp;
    RTC->ISR &= ~(0x1 << 7);
    
    return 0;
}

/*
********************************************************************************
*    Function: ai_rtc_get_date
* Description: RTC��������
*       Input:
*      Output:  year - ��,0~99
*              month - ��,1~12
*                day - ��,0~31
*               week - ��,1~7,0Ϊ�Ƿ�
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_rtc_get_date(u8 *year, u8 *month, u8 *day, u8 *week)
{
    u32 tmp = 0;
    
    RTC->WPR = 0xca;
    RTC->WPR = 0x53;
    while (ai_rtc_wait_sync() != 0)
        ; /* Empty */
    
    tmp = RTC->DR;
    *year = ai_rtc_bcd2dec((tmp >> 16) & 0xff);
    *week = ai_rtc_bcd2dec((tmp >> 13) & 0x07);
    *month = ai_rtc_bcd2dec((tmp >> 8) & 0x1f);
    *day = ai_rtc_bcd2dec(tmp & 0x3f);
}

void RTC_WKUP_IRQHandler(void)
{
    if (RTC->ISR & (0x1 << 10)) {
        RTC->ISR &= ~(0x1 << 10);
        AI_DS1 = !AI_DS1;
    }
    EXTI->PR |= 0x1 << 22;
}

void RTC_Alarm_IRQHandler(void)
{
    if (RTC->ISR & (0x1 << 8)) {
        RTC->ISR &= ~(0x1 << 8);
        printf("takeno ALARM A!\r\n");
    }
    EXTI->PR |= 0x1 << 17;
}

/*
********************************************************************************
*    Function: ai_rtc_set_alarma
* Description: ��������ʱ��(����������,24Сʱ��)
*       Input: week - ����
*              hour - Сʱ 
*               min - ����
*               sec - ��
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_rtc_set_alarma(u8 week, u8 hour, u8 min, u8 sec)
{
    RTC->WPR = 0xca;
    RTC->WPR = 0x53;
    RTC->CR &= ~(0x1 << 8);
    while ((RTC->ISR & 0x01) == 0)    // �ȴ�����A��д
        ; /* Empty */
    
    RTC->ALRMAR = 0;
    RTC->ALRMAR |= 0x1 << 30;         // 1: DU[3:0] represents the week day
    RTC->ALRMAR &= 0x1 << 22;         // AM or 24-hour format
    RTC->ALRMAR &= ~((0x3f << 24) | (0x3f << 16) | (0x7f << 8) | 0x7f);
    RTC->ALRMAR |= (((u32)ai_rtc_dec2bcd(week) & 0x3f) << 24)
                   | (((u32)ai_rtc_dec2bcd(hour) & 0x3f) << 16)
                   | (((u32)ai_rtc_dec2bcd(min) & 0x7f) << 8)
                   | ((u32)ai_rtc_dec2bcd(sec) & 0x7f);
    RTC->ALRMASSR = 0;
    RTC->CR |= 0x1 << 12;             // ��������A�ж�
    RTC->CR |= 0x1 << 8; 
    RTC->ISR &= ~(0x1 << 8);
    RTC->WPR = 0xff;
    
    EXTI->PR |= 0x1 << 17;
    EXTI->IMR |= 0x1 << 17;           // ����line17�ϵ��ж� 
    EXTI->RTSR |= 0x1 << 17;          // line17���¼��������ش���
    ai_nvic_init(2, 2, RTC_Alarm_IRQn, 2);
}

/*
********************************************************************************
*    Function: ai_rtc_wakeup
* Description: �����Ի��Ѷ�ʱ������
*       Input: wucksel - ����ʱ��ѡ��
*                        000 - RTC/16
*                        001 - RTC/8
*                        010 - RTC/4
*                        011 - RTC/2;
*                        10x - ck_spre,1Hz
*                        11x - 1Hz,��cntֵ����2^16(��cnt+2^16)
*                        ע��:RTC����RTC��ʱ��Ƶ��,��RTCCLK!
*                  cnt - �Զ���װ��ֵ.����0,�����ж�.
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_rtc_wakeup(u8 wucksel, u16 cnt)
{
    RTC->WPR = 0xca;
    RTC->WPR = 0x53;
    RTC->CR &= ~(0x1 << 10);
    while ((RTC->ISR & (0x1 << 4)) == 0)
        ; /* Empty */
    
    RTC->CR &= ~0x7;
    RTC->CR |= wucksel & 0x7;
    RTC->WUTR = cnt;
    RTC->ISR &= ~(0x1 << 10);
    RTC->CR |= (0x1 << 14) | (0x1 << 10);
    RTC->WPR = 0xff;
    EXTI->PR |= 0x1 << 22;
    EXTI->IMR |= 0x1 << 22;           // ����line22�ϵ��ж� 
    EXTI->RTSR |= 0x1 << 22;          // line22���¼��������ش���
    ai_nvic_init(2, 2, RTC_WKUP_IRQn, 2);
}

/*
********************************************************************************
*    Function: ai_rtc_get_week
* Description: ������������ڼ������빫�����ڵõ�����(ֻ����1901-2099��)
*       Input: year, month, day������������ 
*      Output: None
*      Return: ���ں�(1~7,������1~����)
*      Others: None
********************************************************************************
*/
// ���������ݱ�,��0��ʼ��ʾ����
static u8 const ai_table_week[12] = {0, 3, 3, 6,
                                     1, 4, 6, 2,
                                     5, 0, 3, 5};

u8 ai_rtc_week(u16 year, u8 month, u8 day)
{
    u8 yearh, yearl;
    u16 tmp;
    
    // �⼸����������1900��֮��
    yearh = year / 100;
    yearl = year % 100;
    if (yearh > 19)
        yearl += 100;        // ���Ϊ21����,�������100
    // year/4�������������ƽ��һ��365�죬365%7=1��
    // temp2�����1900���������۵Ķ������������
    tmp = yearl + yearl / 4;
    // ��������1900��1��1�ն������������������ʵ�ʶ�����ģ�
    // table_week[month-1]�Ѿ�����7��
    tmp = tmp % 7;
    tmp = tmp + day + ai_table_week[month - 1];
    // ���������1�£�2�µĻ�Ҫ��ȥһ����ΪyearL/4�Ѱѵ���������һ������ȥ�ˣ�
    // �������һ���������·��Ժ���ܼӵ�
    if (yearl % 4 == 0 && month < 3)
        tmp--;
    tmp %= 7;
    if (tmp == 0)
        tmp = 7;
    return tmp;
}

/*
********************************************************************************
*    Function: ai_rtc_get_week
* Description: ������������ڼ����ӹ�Ԫ0��1��1�������տ�ʼ���빫�����ڵõ�����
*              ��ķ����ɭ���㹫ʽ:
*              W= (d+2*m+3*(m+1)/5+y+y/4-y/100+y/400) mod 7
*       Input: year, month, day������������ 
*      Output: None
*      Return: ���ں�(1~7,������1~����)
*      Others: None
********************************************************************************
*/
u8 ai_rtc_get_week(u16 year, u8 month, u8 day)
{
    u16 tmp = 0;
    
    if (month < 3) {
        month += 12;
        year--;
    }
    
    tmp = (day + 2 * month + 3 * (month + 1) / 5 + year
          + year / 4 - year / 100 + year / 400 + 1) % 7;
    return tmp;
}

/*
********************************************************************************
*    Function: ai_rtc_init
* Description: RTC��ʼ��,Ĭ�ϳ���ʹ��LSE,��LSE����ʧ�ܺ�,�л�ΪLSI.
*              ͨ��BKP�Ĵ���0��ֵ,�����ж�RTCʹ�õ���LSE/LSI:
*                 ��BKP0==0X5050ʱ,ʹ�õ���LSE
*                 ��BKP0==0X5051ʱ,ʹ�õ���LSI
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: ע��:�л�LSI/LSE������ʱ��/���ڶ�ʧ,�л�������������.
********************************************************************************
*/
int ai_rtc_init(void)
{
    u32 bkp_reg = 0;
    u16 timeout = 0x1fff;
    u32 tmp = 0;
    u16 ssr_reg = 0;
    
    RCC->APB1ENR |= 0x1 << 28;             // PWR��Դ����ʹ��
    // Access to RTC and RTC Backup registers and backup SRAM enabled
    PWR->CR |= 0x1 << 8;
    
    bkp_reg = ai_rtc_read_bkr(0);
    if (bkp_reg != 0x5050) {               // ֮ǰʹ�õĲ���LSE
        RCC->CSR |= 0x1;                   // ʹ��LSI RC
        while (!(RCC->CSR & (0x1 << 1)))   // �ȴ�LSI����
            ; /* Empty */
        RCC->BDCR |= 0x1;                  //  ���Կ���LSE
        while (timeout && !(RCC->BDCR & (0x1 << 1))) {
            timeout--;
            ai_delay_ms(5);
        }
        
        tmp = RCC->BDCR;
        tmp &= ~(0x3 << 8);                // RTCʱ��Դѡ��
        if (timeout == 0)
            tmp |= 0x2 << 8;               // ѡ��LSI��Ϊʱ��Դ
        else
            tmp |= 0x1 << 8;               // ѡ��LSE��Ϊʱ��Դ
        tmp |= 0x1 << 15;                  // ʱ��RTCʱ��
        RCC->BDCR = tmp;
        
        /* ��������RTC�Ĵ�����д���� */
        RTC->WPR = 0xca;
        RTC->WPR = 0x53;
        RTC->CR = 0;
        if (ai_rtc_init_mode()) {
            RCC->BDCR = 0x1 << 16;
            ai_delay_ms(10);
            RCC->BDCR = 0;
            return -1;
        }
        // RTCͬ����Ƶϵ��(0~7FFF),����������ͬ����Ƶ,�������첽��Ƶ,
        // Frtc = Fclks / ((Sprec + 1) * (Asprec + 1))
        RTC->PRER = 0xff;
        RTC->PRER |= 0x7f << 16;
        RTC->CR &= ~(0x1 << 6);            // RTC����Ϊ,24Сʱ��ʽ
        RTC->ISR &= ~(0x1 << 7);           // �˳�RTC��ʼ��ģʽ
        RTC->WPR = 0xff;                   // ʹ��д����
        // BKP0�����ݼȲ���0X5050,Ҳ����0X5051,˵���ǵ�һ������,��Ҫ����ʱ������.
        if (bkp_reg != 0x5051) {
            ai_rtc_set_time(23, 59, 56, 0);
            ai_rtc_set_date(20, 1, 23, 7);
        }
        if (timeout == 0)
            ai_rtc_write_bkr(0, 0x5051);   // ����Ѿ���ʼ������,ʹ��LSI
        else
            ai_rtc_write_bkr(0, 0x5050);   // ����Ѿ���ʼ������,ʹ��LSE
           
    } else {
        timeout = 10;                      // ����10��SSR��ֵ��û�仯,��LSE����.
        ssr_reg = RTC->SSR;
        while (timeout) {                  // ���ssr�Ĵ����Ķ�̬,���ж�LSE�Ƿ�����
            ai_delay_ms(10);
            if (ssr_reg == RTC->SSR)
                timeout--;
            else
                break;
        }
        if (timeout == 0) {                // LSE����,������õȴ��´ν�����������
            ai_rtc_write_bkr(0, 0xffff);
            RCC->BDCR = 0x1 << 16;
            ai_delay_ms(10);
            RCC->BDCR = 0;
        }
    }
    
    return 0;
}

