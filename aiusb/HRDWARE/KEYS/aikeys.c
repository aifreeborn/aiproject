#include "aikeys.h"
#include <aisys.h>

/*******************************************************************************
* ����ɨ�谴��ʹ�õı�����
*     ai_keys_curr��ai_keys_old, ai_keys_keep_time
* �ṩ��Ӧ�ó���ʹ�õı�����
*     ai_keys_pressed����ʾ��ǰ����ס���ŵİ�����
*     ai_keys_down   ����ʾ�°��µİ�����
*     ai_keys_up     ����ʾ���ɿ��İ�����
*     ai_keys_last   ����ai_keys_pressed��Ӧ���水����ֵ
*     ai_keys_can_change ��Ӧ�ó������������Ƿ������µ�ɨ�裻
* ˵����22.1184M����Լ5ms�ж�һ�Ρ�
*******************************************************************************/
volatile uint8 idata ai_keys_curr, ai_keys_old, ai_keys_keep_time;
volatile uint8 idata ai_keys_pressed;
volatile uint8 idata ai_keys_down, ai_keys_up, ai_keys_last;
volatile uint8 ai_keys_can_change;

/*******************************************************************************
* �������ܣ���ʱ��0��ʼ��������������ɨ��
*     ��ʱ����������ֹͣ����TCON�Ĵ����е�TRX(X=0, 1)�����ƣ�
*     ��ʱģʽ����ʽ1�� 16Ϊ��ʱ��/������
*******************************************************************************/
void ai_keys_init_timer0(void)
{
    TMOD &= 0xf0;
    TMOD |= 0x01;
    ET0 = 1;               // ��ʱ��/������0�ж�����λ����T0�ж�
    TR0 = 1;               // ��ʱ��0���п���λ����1������ʱ��0
}

void ai_keys_timer0_isr(void) interrupt 1
{
    // ��ʱ��0��װ����ʱ���Ϊ5ms����15��Ϊ��������װ������ʱ��
    // ���ֵ����ͨ�����������ȷ�������������öϵ㣬����ʹ��������
    // ʱ���պ�Ϊ5ms���ɡ�
    TH0 = (65536 - AI_SYS_CLK / 1000 / 12 * 5 + 15) / 256;
    TL0 = (65536 - AI_SYS_CLK / 1000 / 12 * 5 + 15) % 256;

    // ������ڴ�����������ɨ�����
    if (!ai_keys_can_change)
        return;

    // ��ʼ����ɨ��,���水��״̬����ǰ�������,ai_keys_curr�ܹ���8��bit
    // ��ĳ�����ذ���ʱ����Ӧ��bitΪ1
    ai_keys_curr = ai_keys_get_val();
    if (ai_keys_curr != ai_keys_old) {
        ai_keys_keep_time = 0;
        ai_keys_old = ai_keys_curr;
        return;
    } else {
        // ����ʱ���ۼ�
        ai_keys_keep_time++;
        if (ai_keys_keep_time >= 1) {
            ai_keys_keep_time = 1;
            // ������ǰ���µ����а����ļ�ֵ
            ai_keys_pressed = ai_keys_old;
            // ����°��µļ�
            ai_keys_down |= (~ai_keys_last) & (ai_keys_pressed);
            // ������ͷŵİ���
            ai_keys_up |= ai_keys_last & (~ai_keys_pressed);
            ai_keys_last = ai_keys_pressed;
        }
    }
}

void ai_keys_init(void)
{
    AI_KEYS = 0xff;

    ai_keys_curr = 0;
    ai_keys_old = 0;
    ai_keys_keep_time = 0;
    ai_keys_pressed = 0;
    ai_keys_down = 0;
    ai_keys_up = 0;
    ai_keys_last = 0;
    
    ai_keys_init_timer0();
    ai_keys_can_change = 1;
}