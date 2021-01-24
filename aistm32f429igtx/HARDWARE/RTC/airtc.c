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
* Description: 读取RTC备份寄存器的数值
*       Input: num - 备份寄存器的编号，从0开始，范围为0~19
*      Output: None
*      Return: 对应备份寄存器的数据
*      Others: None
********************************************************************************
*/
u32 ai_rtc_read_bkr(u8 num)
{
    u32 tmp = 0;
    
    // 备份寄存器的偏移地址为0x50 to 0x9c
    tmp = RTC_BASE + 0x50 + num * 4;
    return (*(u32 *)tmp);
}

/*
********************************************************************************
*    Function: ai_rtc_write_bkr
* Description: 写RTC备份寄存器
*       Input: num - 备份寄存器的编号，从0开始，范围为0~19
*              val - 待写入寄存器的数值
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_rtc_write_bkr(u8 num, u32 val)
{
    u32 tmp = 0;
    
    // 备份寄存器的偏移地址为0x50 to 0x9c
    tmp = RTC_BASE + 0x50 + num * 4;
    (*(u32 *)tmp) = val;
}

/*
********************************************************************************
*    Function: ai_rtc_init_mode
* Description: RTC进入初始化模式
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
    
    if (RTC->ISR & (0x1 << 6))        // 允许更新日历寄存器
        return 0;
    RTC->ISR |= 0x1 << 7;             // 进入初始化模式
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
* Description: 等待RSF(寄存器同步标志)同步
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
    RTC->ISR &= ~(0x1 << 5);     // 清除RSF标志位
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
* Description: 十进制转换为BCD码
*       Input: val - 要转换的十进制数,取值0~99
*      Output: None
*      Return: BCD码
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
* Description: BCD码转换为十进制数据
*       Input: val - 要转换的BCD码
*      Output: None
*      Return: 十进制数据
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
* Description: RTC时间设置
*       Input: hour - 小时
*               min - 分钟
*               sec - 秒
*             am_pm - 设置上午还是下午; 0,AM/24H; 1,PM
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
* Description: 获取RTC时间
*       Input: void
*      Output: hour - 小时
*               min - 分钟
*               sec - 秒
*             am_pm - 设置上午还是下午; 0,AM/24H; 1,PM
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
* Description: RTC日期设置
*       Input:  year - 年,0~99
*              month - 月,1~12
*                day - 日,0~31
*               week - 周,1~7,0为非法
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
* Description: RTC日期设置
*       Input:
*      Output:  year - 年,0~99
*              month - 月,1~12
*                day - 日,0~31
*               week - 周,1~7,0为非法
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
* Description: 设置闹钟时间(按星期闹铃,24小时制)
*       Input: week - 星期
*              hour - 小时 
*               min - 分钟
*               sec - 秒
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
    while ((RTC->ISR & 0x01) == 0)    // 等待闹钟A可写
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
    RTC->CR |= 0x1 << 12;             // 开启闹钟A中断
    RTC->CR |= 0x1 << 8; 
    RTC->ISR &= ~(0x1 << 8);
    RTC->WPR = 0xff;
    
    EXTI->PR |= 0x1 << 17;
    EXTI->IMR |= 0x1 << 17;           // 开启line17上的中断 
    EXTI->RTSR |= 0x1 << 17;          // line17上事件上升降沿触发
    ai_nvic_init(2, 2, RTC_Alarm_IRQn, 2);
}

/*
********************************************************************************
*    Function: ai_rtc_wakeup
* Description: 周期性唤醒定时器设置
*       Input: wucksel - 唤醒时钟选择
*                        000 - RTC/16
*                        001 - RTC/8
*                        010 - RTC/4
*                        011 - RTC/2;
*                        10x - ck_spre,1Hz
*                        11x - 1Hz,且cnt值增加2^16(即cnt+2^16)
*                        注意:RTC就是RTC的时钟频率,即RTCCLK!
*                  cnt - 自动重装载值.减到0,产生中断.
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
    EXTI->IMR |= 0x1 << 22;           // 开启line22上的中断 
    EXTI->RTSR |= 0x1 << 22;          // line22上事件上升降沿触发
    ai_nvic_init(2, 2, RTC_WKUP_IRQn, 2);
}

/*
********************************************************************************
*    Function: ai_rtc_get_week
* Description: 获得现在是星期几，输入公历日期得到星期(只允许1901-2099年)
*       Input: year, month, day：公历年月日 
*      Output: None
*      Return: 星期号(1~7,代表周1~周日)
*      Others: None
********************************************************************************
*/
// 月修正数据表,以0开始表示星期
static u8 const ai_table_week[12] = {0, 3, 3, 6,
                                     1, 4, 6, 2,
                                     5, 0, 3, 5};

u8 ai_rtc_week(u16 year, u8 month, u8 day)
{
    u8 yearh, yearl;
    u16 tmp;
    
    // 这几行算出本年和1900年之差
    yearh = year / 100;
    yearl = year % 100;
    if (yearh > 19)
        yearl += 100;        // 如果为21世纪,年份数加100
    // year/4是闰年的年数，平年一年365天，365%7=1；
    // temp2算出自1900年以来积累的多出来的天数；
    tmp = yearl + yearl / 4;
    // 算出当天比1900年1月1日多出来的天数，并不是实际多出来的，
    // table_week[month-1]已经除过7了
    tmp = tmp % 7;
    tmp = tmp + day + ai_table_week[month - 1];
    // 如果是润年1月，2月的话要减去一，因为yearL/4已把当年多出来的一天计算进去了，
    // 多出来的一天是在三月份以后才能加的
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
* Description: 获得现在是星期几，从公元0年1月1日星期日开始输入公历日期得到星期
*              基姆拉尔森计算公式:
*              W= (d+2*m+3*(m+1)/5+y+y/4-y/100+y/400) mod 7
*       Input: year, month, day：公历年月日 
*      Output: None
*      Return: 星期号(1~7,代表周1~周日)
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
* Description: RTC初始化,默认尝试使用LSE,当LSE启动失败后,切换为LSI.
*              通过BKP寄存器0的值,可以判断RTC使用的是LSE/LSI:
*                 当BKP0==0X5050时,使用的是LSE
*                 当BKP0==0X5051时,使用的是LSI
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: 注意:切换LSI/LSE将导致时间/日期丢失,切换后需重新设置.
********************************************************************************
*/
int ai_rtc_init(void)
{
    u32 bkp_reg = 0;
    u16 timeout = 0x1fff;
    u32 tmp = 0;
    u16 ssr_reg = 0;
    
    RCC->APB1ENR |= 0x1 << 28;             // PWR电源控制使能
    // Access to RTC and RTC Backup registers and backup SRAM enabled
    PWR->CR |= 0x1 << 8;
    
    bkp_reg = ai_rtc_read_bkr(0);
    if (bkp_reg != 0x5050) {               // 之前使用的不是LSE
        RCC->CSR |= 0x1;                   // 使能LSI RC
        while (!(RCC->CSR & (0x1 << 1)))   // 等待LSI就绪
            ; /* Empty */
        RCC->BDCR |= 0x1;                  //  尝试开启LSE
        while (timeout && !(RCC->BDCR & (0x1 << 1))) {
            timeout--;
            ai_delay_ms(5);
        }
        
        tmp = RCC->BDCR;
        tmp &= ~(0x3 << 8);                // RTC时钟源选择
        if (timeout == 0)
            tmp |= 0x2 << 8;               // 选择LSI作为时钟源
        else
            tmp |= 0x1 << 8;               // 选择LSE作为时钟源
        tmp |= 0x1 << 15;                  // 时钟RTC时钟
        RCC->BDCR = tmp;
        
        /* 解锁所有RTC寄存器的写保护 */
        RTC->WPR = 0xca;
        RTC->WPR = 0x53;
        RTC->CR = 0;
        if (ai_rtc_init_mode()) {
            RCC->BDCR = 0x1 << 16;
            ai_delay_ms(10);
            RCC->BDCR = 0;
            return -1;
        }
        // RTC同步分频系数(0~7FFF),必须先设置同步分频,再设置异步分频,
        // Frtc = Fclks / ((Sprec + 1) * (Asprec + 1))
        RTC->PRER = 0xff;
        RTC->PRER |= 0x7f << 16;
        RTC->CR &= ~(0x1 << 6);            // RTC设置为,24小时格式
        RTC->ISR &= ~(0x1 << 7);           // 退出RTC初始化模式
        RTC->WPR = 0xff;                   // 使能写保护
        // BKP0的内容既不是0X5050,也不是0X5051,说明是第一次配置,需要设置时间日期.
        if (bkp_reg != 0x5051) {
            ai_rtc_set_time(23, 59, 56, 0);
            ai_rtc_set_date(20, 1, 23, 7);
        }
        if (timeout == 0)
            ai_rtc_write_bkr(0, 0x5051);   // 标记已经初始化过了,使用LSI
        else
            ai_rtc_write_bkr(0, 0x5050);   // 标记已经初始化过了,使用LSE
           
    } else {
        timeout = 10;                      // 连续10次SSR的值都没变化,则LSE死了.
        ssr_reg = RTC->SSR;
        while (timeout) {                  // 检测ssr寄存器的动态,来判断LSE是否正常
            ai_delay_ms(10);
            if (ssr_reg == RTC->SSR)
                timeout--;
            else
                break;
        }
        if (timeout == 0) {                // LSE挂了,清除配置等待下次进入重新设置
            ai_rtc_write_bkr(0, 0xffff);
            RCC->BDCR = 0x1 << 16;
            ai_delay_ms(10);
            RCC->BDCR = 0;
        }
    }
    
    return 0;
}

