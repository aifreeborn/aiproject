#include "usmart.h"
#include "usmart_str.h"

/*
********************************************************************************
*                              �û�������
* ������Ҫ�������õ��ĺ�����������ͷ�ļ�(�û��Լ����) 
********************************************************************************
*/
#include "aidelay.h"
#include "ailed.h"
#include "aisys.h"
#include "ailcd.h"
#include "aiwm9825g6kh.h"
#include "ailtdc.h"
								 
extern void led_set(u8 sta);
extern void test_fun(void(*ledset)(u8),u8 sta);

/*
********************************************************************************
* �������б��ʼ��(�û��Լ����)
* �û�ֱ������������Ҫִ�еĺ�����������Ҵ�
********************************************************************************
*/
struct _m_usmart_nametab usmart_nametab[] = {
    (void *)ai_delay_ms, "void delay_ms(u16 nms)",
    (void *)ai_led_on, "void ai_led_on(u8 num)",
    (void *)ai_led_off, "void ai_led_off(u8 num)",
};

/*
********************************************************************************
* �������ƹ�������ʼ��
* �õ������ܿغ���������
* �õ�����������
********************************************************************************
*/
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),// ��������
	0,	  	// ��������
	0,	 	// ����ID
	1,		// ������ʾ����,0,10����;1,16����
	0,		// ��������.bitx:,0,����;1,�ַ���	    
	0,	  	// ÿ�������ĳ����ݴ��,��ҪMAX_PARM��0��ʼ��
	0,		// �����Ĳ���,��ҪPARM_LEN��0��ʼ��
};   
