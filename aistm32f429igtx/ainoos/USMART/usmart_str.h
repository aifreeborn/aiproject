/*
********************************************************************************
* usmart是从正点原子的工程移植过来的,自己做了格式整理,仅用于研究学习,下面是声明:
* 本程序只供学习使用，未经作者许可，不得用于其它任何用途
* ALIENTEK STM32开发板	   
* 正点原子@ALIENTEK
* 技术论坛:www.openedv.com 
* 版本：V3.3
* 版权所有，盗版必究。
* Copyright(C) 正点原子 2011-2021
* All rights reserved
********************************************************************************
*/
#ifndef __USMART_STR_H
#define __USMART_STR_H	 

#include "aisys.h"

// 得到某个参数在参数列里面的起始位置
u8 usmart_get_parmpos(u8 num);
// 对比两个字符串是否相等
u8 usmart_strcmp(u8*str1,u8 *str2);
// M^N次方
u32 usmart_pow(u8 m,u8 n);
// 字符串转为数字
u8 usmart_str2num(u8*str,u32 *res);
// 从str中得到指令名,并返回指令长度
u8 usmart_get_cmdname(u8*str,u8*cmdname,u8 *nlen,u8 maxlen);
// 从str中得到函数名
u8 usmart_get_fname(u8*str,u8*fname,u8 *pnum,u8 *rval);
// 从str中得到一个函数参数
u8 usmart_get_aparm(u8 *str,u8 *fparm,u8 *ptype);
// 得到str中所有的函数参数.
u8 usmart_get_fparam(u8*str,u8 *parn);
#endif
