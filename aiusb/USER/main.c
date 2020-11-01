#include <aisys.h>
#include <aitype.h>
#include <aileds.h>
#include <aikeys.h>
#include <aiuart.h>
#include <aipdiusbd12.h>

#define    AI_HDR_TBL_NUM    11U
 
static code uint8 ai_header_tbl[][74] = {
    "********************************************************************\r\n",
    "******            《圈圈教你玩USB》练习程序                   ******\r\n",
    "******                  STC89C52RC CPU                        ******\r\n",
    "******               建立日期：",__DATE__,"                    ******\r\n",
    "******               建立时间：",__TIME__,"                       ******\r\n",
    "******               作者：takeno                             ******\r\n",
    "********************************************************************\r\n",  
};

void ai_key_down_scan(void)
{
    if (ai_keys_down) {
        if (ai_keys_down & AI_KEY1) {
            ai_uart_send_str("KEY1 down\r\n");
            ai_keys_down &= ~AI_KEY1;
        }
        if (ai_keys_down & AI_KEY2) {
            ai_uart_send_str("KEY2 down\r\n");
            ai_keys_down &= ~AI_KEY2;
        }
        if (ai_keys_down & AI_KEY3) {
            ai_uart_send_str("KEY3 down\r\n");
            ai_keys_down &= ~AI_KEY3;
        }
        if (ai_keys_down & AI_KEY4) {
            ai_uart_send_str("KEY4 down\r\n");
            ai_keys_down &= ~AI_KEY4;
        }
        if (ai_keys_down & AI_KEY5) {
            ai_uart_send_str("KEY5 down\r\n");
            ai_keys_down &= ~AI_KEY5;
        }
        if (ai_keys_down & AI_KEY6) {
            ai_uart_send_str("KEY6 down\r\n");
            ai_keys_down &= ~AI_KEY6;
        }
        if (ai_keys_down & AI_KEY7) {
            ai_uart_send_str("KEY7 down\r\n");
            ai_keys_down &= ~AI_KEY7;
        }
        if (ai_keys_down & AI_KEY8) {
            ai_uart_send_str("KEY8 down\r\n");
            ai_keys_down &= ~AI_KEY8;
        }
    }
}

void ai_key_up_scan(void)
{
    if (ai_keys_up) {
        if (ai_keys_up & AI_KEY1) {
            ai_uart_send_str("KEY1 up\r\n");
            ai_keys_up &= ~AI_KEY1; 
        }
        if (ai_keys_up & AI_KEY2) {
            ai_uart_send_str("KEY2 up\r\n");
            ai_keys_up &= ~AI_KEY2; 
        }
        if (ai_keys_up & AI_KEY3) {
            ai_uart_send_str("KEY3 up\r\n");
            ai_keys_up &= ~AI_KEY3; 
        }
        if (ai_keys_up & AI_KEY4) {
            ai_uart_send_str("KEY4 up\r\n");
            ai_keys_up &= ~AI_KEY4; 
        }
        if (ai_keys_up & AI_KEY5) {
            ai_uart_send_str("KEY5 up\r\n");
            ai_keys_up &= ~AI_KEY5; 
        }
        if (ai_keys_up & AI_KEY6) {
            ai_uart_send_str("KEY6 up\r\n");
            ai_keys_up &= ~AI_KEY6; 
        }
        if (ai_keys_up & AI_KEY7) {
            ai_uart_send_str("KEY7 up\r\n");
            ai_keys_up &= ~AI_KEY7; 
        }
        if (ai_keys_up & AI_KEY8) {
            ai_uart_send_str("KEY8 up\r\n");
            ai_keys_up &= ~AI_KEY8; 
        }
    }
}

void main()
{
    uint8 i;
    uint16 d12_id = 0;

	ai_leds_init();
    ai_sys_enable_interrupt();
    ai_keys_init();
    ai_uart_init();

    // 显示提示信息
    for (i = 0; i < AI_HDR_TBL_NUM; i++) {
        ai_uart_send_str(ai_header_tbl[i]);
    }

    d12_id = ai_d12_read_id();
    ai_uart_send_str("My PDIUSBD12 chip\'s ID is: ");
    ai_uart_print_short(d12_id);
    if (d12_id == 0x1012) {
        ai_uart_send_str(". ID is correct! Congratulations!\r\n\r\n");
    } else {
        ai_uart_send_str(". ID is incorrect! What a pity!\r\n\r\n");
    }

    while (1) {
        /* main loop */
        AI_ALL_LEDS = ~ai_keys_pressed;
        ai_key_down_scan();
        ai_key_up_scan();    
    }
}
