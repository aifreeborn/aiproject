#include <aisys.h>
#include <reg52.h>
#include "aiuart.h"

static volatile uint8 ai_uart_sending;
static code ai_uart_hex_tbl[] = {'0','1','2','3','4','5','6','7','8','9',
                                 'A','B','C','D','E','F'};

/*******************************************************************************
* Function����ʼ������
* Input   : void
* Output  ��None
* Return  ��void
* Others  : None
*******************************************************************************/
void ai_uart_init(void)
{
    EA = 0;
    TMOD &= 0x0f;
    TMOD |= 0x20;    // 8λ�Զ���װ��ʱ���������ʱ��TH1��ŵ�ֵ�Զ���װ��TL1
    SCON = 0x50;     // ���ڹ�����ʽ1��8λUART,�����ʣ�(2^SMOD / 32)* (T1�������) 
    PCON |= 0x80;     // �����ʼӱ�
    TH1 = 256 - AI_SYS_CLK / (AI_UART_BAUDRATE * 12 *16);
    TL1 = 256 - AI_SYS_CLK / (AI_UART_BAUDRATE * 12 *16);
    ES = 1;          // �����п��ж�
    TR1 = 1;         // ������ʱ��
    REN = 1;         // �������
    EA = 1;          // CPU���ж�

}

/*******************************************************************************
* Function�������жϷ������
* Input   : void
* Output  ��None
* Return  ��void
* Others  : None
*******************************************************************************/
static void ai_uart_isr(void) interrupt 4
{
    if (RI) {
        RI = 0;
    } else {
        TI = 0;
        ai_uart_sending = 0;
    }
}

/*******************************************************************************
* Function������һ���ַ�
* Input   ��ch -- �����͵��ַ�
* Output  ��None
* Return  ��void
* Others  : None
*******************************************************************************/
void ai_uart_send_char(uint8 ch)
{
    SBUF = ch;
    ai_uart_sending = 1;
    while (ai_uart_sending);    // �ȴ��������
}

/*******************************************************************************
* Function: ��HEX��ʽ����һ���ֽ�����
* Input   ��ch -- �����͵��ֽ�����
* Output  ��None
* Return  ��void
* Others  : None
*******************************************************************************/
void ai_uart_print_hex(uint8 ch)
{
    ai_uart_send_char('0');
    ai_uart_send_char('x');
    ai_uart_send_char(ai_uart_hex_tbl[ch >> 4]);
    ai_uart_send_char(ai_uart_hex_tbl[ch & 0x0f]);
}

/*******************************************************************************
* Function�������ַ���
* Input   ��strΪҪ���͵��ַ���ָ�루�ַ����ԡ�\0��������
* Output  ��None
* Return  ��void
* Others  : None
*******************************************************************************/
void ai_uart_send_str(uint8 *str)
{
    while ((*str) != '\0') {
        ai_uart_send_char(*str);
        str++;
    }
}

/*******************************************************************************
* Function��������ת��������ʮ�����ַ�������
* Input   ��val -- ����ʾ������ֵ
* Output  ��None
* Return  ��void
* Others  : None
*******************************************************************************/
void ai_uart_print_lint(uint32 val)
{
    int i;
    uint8 buf[10];

    for (i = 9; i >= 0; i--) {
        buf[i] = val % 10 + '0';
        val /= 10;
    }
    for (i = 0; i < 9; i++) {
        if (buf[i] != '0')
            break;
    }
    for (; i < 10; i++)
        ai_uart_send_char(buf[i]);   
}

/*******************************************************************************
* Function��������ת������ʮ�����ƽ����ַ�������
* Input   ��val -- ����ʾ������ֵ
* Output  ��None
* Return  ��void
* Others  : None
*******************************************************************************/
void ai_uart_print_short(uint16 val)
{
    int i;
    uint8 buf[7];

    buf[0] = '0';
    buf[1] = 'x';
    buf[6] = '\0';

    for (i = 5; i >= 2; i--) {
        buf[i] = ai_uart_hex_tbl[(val & 0x000f)];
        val >>= 4;
    }
    ai_uart_send_str(buf);
}