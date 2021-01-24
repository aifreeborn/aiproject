#include "usmart.h"
#include "usmart_str.h"

/*
********************************************************************************
*                              用户配置区
* 这下面要包含所用到的函数所申明的头文件(用户自己添加) 
********************************************************************************
*/
#include "aidelay.h"
#include "ailed.h"
#include "aisys.h"
#include "ailcd.h"
#include "aiwm9825g6kh.h"
#include "ailtdc.h"
#include "airtc.h"
								 
extern void led_set(u8 sta);
extern void test_fun(void(*ledset)(u8),u8 sta);

/*
********************************************************************************
* 函数名列表初始化(用户自己添加)
* 用户直接在这里输入要执行的函数名及其查找串
********************************************************************************
*/
struct _m_usmart_nametab usmart_nametab[] = {
    (void *)ai_delay_ms, "void delay_ms(u16 nms)",
    (void *)ai_led_on, "void ai_led_on(u8 num)",
    (void *)ai_led_off, "void ai_led_off(u8 num)",
    (void *)ai_rtc_set_time,
    "int ai_rtc_set_time(u8 hour, u8 min, u8 sec, u8 am_pm)",
    (void *)ai_rtc_set_date,
    "int ai_rtc_set_date(u8 year, u8 month, u8 day, u8 week)",
    (void *)ai_rtc_set_alarma,
    "void ai_rtc_set_alarma(u8 week, u8 hour, u8 min, u8 sec)",
    (void *)ai_rtc_wakeup,
    "void ai_rtc_wakeup(u8 wucksel, u16 cnt)",
    (void *)ai_rtc_read_bkr,
    "ai_rtc_read_bkr(u8 num)",
    (void *)ai_rtc_write_bkr,
    "ai_rtc_write_bkr(u8 num, u32 val)",
};

/*
********************************************************************************
* 函数控制管理器初始化
* 得到各个受控函数的名字
* 得到函数总数量
********************************************************************************
*/
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),// 函数数量
	0,	  	// 参数数量
	0,	 	// 函数ID
	1,		// 参数显示类型,0,10进制;1,16进制
	0,		// 参数类型.bitx:,0,数字;1,字符串	    
	0,	  	// 每个参数的长度暂存表,需要MAX_PARM个0初始化
	0,		// 函数的参数,需要PARM_LEN个0初始化
};   
