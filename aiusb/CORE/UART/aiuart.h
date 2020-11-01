#ifndef AI_UART_H_
#define AI_UART_H_

#include <aitype.h>

// 串口参数设置：波特率-9600
#define    AI_UART_BAUDRATE    9600UL


void ai_uart_init(void);
void ai_uart_send_char(uint8 ch);
void ai_uart_print_hex(uint8 ch);
void ai_uart_send_str(uint8 *str);
void ai_uart_print_lint(uint32 val);
void ai_uart_print_short(uint16 val);

#endif /* AI_UART_H_ */