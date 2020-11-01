#include <aisys.h>
#include <reg52.h>
#include "aiuart.h"

static volatile uint8 ai_uart_sending;
static code ai_uart_hex_tbl[] = {'0','1','2','3','4','5','6','7','8','9',
                                 'A','B','C','D','E','F'};

/*******************************************************************************
* Function：初始化串口
* Input   : void
* Output  ：None
* Return  ：void
* Others  : None
*******************************************************************************/
void ai_uart_init(void)
{
    EA = 0;
    TMOD &= 0x0f;
    TMOD |= 0x20;    // 8位自动重装定时器，当溢出时将TH1存放的值自动重装入TL1
    SCON = 0x50;     // 串口工作方式1，8位UART,波特率：(2^SMOD / 32)* (T1的溢出率) 
    PCON |= 0x80;     // 波特率加倍
    TH1 = 256 - AI_SYS_CLK / (AI_UART_BAUDRATE * 12 *16);
    TL1 = 256 - AI_SYS_CLK / (AI_UART_BAUDRATE * 12 *16);
    ES = 1;          // 允许串行口中断
    TR1 = 1;         // 启动定时器
    REN = 1;         // 允许接收
    EA = 1;          // CPU开中断

}

/*******************************************************************************
* Function：串口中断服务程序
* Input   : void
* Output  ：None
* Return  ：void
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
* Function：发送一个字符
* Input   ：ch -- 待发送的字符
* Output  ：None
* Return  ：void
* Others  : None
*******************************************************************************/
void ai_uart_send_char(uint8 ch)
{
    SBUF = ch;
    ai_uart_sending = 1;
    while (ai_uart_sending);    // 等待发送完毕
}

/*******************************************************************************
* Function: 以HEX格式发送一个字节数据
* Input   ：ch -- 待发送的字节数据
* Output  ：None
* Return  ：void
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
* Function：发送字符串
* Input   ：str为要发送的字符串指针（字符串以‘\0’结束）
* Output  ：None
* Return  ：void
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
* Function：将整数转换并按照十进制字符串发送
* Input   ：val -- 待显示的整数值
* Output  ：None
* Return  ：void
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
* Function：将整数转换并按十六进制进制字符串发送
* Input   ：val -- 待显示的整数值
* Output  ：None
* Return  ：void
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