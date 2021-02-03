#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "ailcd.h"
#include "aift5206.h"
#include "aitouch.h"

/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/
ai_tp_dev_t    ai_tp_dev = {
    ai_tp_init,
    ai_ft5206_scan,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

/*
********************************************************************************
*    Function: ai_tp_init
* Description: ��������ʼ��
*       Input: void
*      Output: None
*      Return:  0 -> ���й�У׼
*              -1 -> û�н��й�У׼
*      Others: ���ļ��Ǵ������Ķ����ļ���Ŀ����֧�ֵ������͵���������������ֻ��
*              LCD-RGB 1024*600������������ֻʵ�������Ļ�ģ�����δʵ��.
********************************************************************************
*/
int ai_tp_init(void)
{
    if (ai_lcd_dev.id == AI_LCD_INCH7_1024_600) {
        ai_ft5206_init();
        ai_tp_dev.scan = ai_ft5206_scan;
        ai_tp_dev.type |= 0x80;             // ������
        ai_tp_dev.type |= ai_lcd_dev.dir & 0x01;
    }
    // �������else��֧ʵ��������Ļ��֧�֣����ﲻʵ��
    
    return 0;
}
