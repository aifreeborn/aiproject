/*
********************************************************************************
* usmart�Ǵ�����ԭ�ӵĹ�����ֲ������,�Լ����˸�ʽ����,�������о�ѧϰ,����������:
* ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
* ALIENTEK STM32������	   
* ����ԭ��@ALIENTEK
* ������̳:www.openedv.com 
* �汾��V3.3
* ��Ȩ���У�����ؾ���
* Copyright(C) ����ԭ�� 2011-2021
* All rights reserved
********************************************************************************
*/
#ifndef __USMART_STR_H
#define __USMART_STR_H	 

#include "aisys.h"

// �õ�ĳ�������ڲ������������ʼλ��
u8 usmart_get_parmpos(u8 num);
// �Ա������ַ����Ƿ����
u8 usmart_strcmp(u8*str1,u8 *str2);
// M^N�η�
u32 usmart_pow(u8 m,u8 n);
// �ַ���תΪ����
u8 usmart_str2num(u8*str,u32 *res);
// ��str�еõ�ָ����,������ָ���
u8 usmart_get_cmdname(u8*str,u8*cmdname,u8 *nlen,u8 maxlen);
// ��str�еõ�������
u8 usmart_get_fname(u8*str,u8*fname,u8 *pnum,u8 *rval);
// ��str�еõ�һ����������
u8 usmart_get_aparm(u8 *str,u8 *fparm,u8 *ptype);
// �õ�str�����еĺ�������.
u8 usmart_get_fparam(u8*str,u8 *parn);
#endif
