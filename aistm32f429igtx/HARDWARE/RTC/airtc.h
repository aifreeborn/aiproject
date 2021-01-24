#ifndef AI_RTC_H_
#define AI_RTC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "aitypes.h"

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
// 备份寄存器    
u32 ai_rtc_read_bkr(u8 num);
void ai_rtc_write_bkr(u8 num, u32 val);

// dec <-> bcd转换函数
u8 ai_rtc_dec2bcd(u8 val);
u8 ai_rtc_bcd2dec(u8 val);

// 时间设置/获取函数
int ai_rtc_set_time(u8 hour, u8 min, u8 sec, u8 am_pm);
void ai_rtc_get_time(u8 *hour, u8 *min, u8 *sec, u8 *am_pm);
    
// 日期设置/获取函数
int ai_rtc_set_date(u8 year, u8 month, u8 day, u8 week);
void ai_rtc_get_date(u8 *year, u8 *month, u8 *day, u8 *week);

// 闹钟操作
void ai_rtc_set_alarma(u8 week, u8 hour, u8 min, u8 sec);
void ai_rtc_wakeup(u8 wucksel, u16 cnt);
u8 ai_rtc_get_week(u16 year, u8 month, u8 day);
u8 ai_rtc_week(u16 year, u8 month, u8 day);

// RTC初始化操作
int ai_rtc_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_RTC_H_ */
