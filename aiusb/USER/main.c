#include <aisys.h>
#include <aitype.h>
#include <aileds.h>
#include <aikeys.h>
#include <aiuart.h>
#include <aipdiusbd12.h>
#include <aiusb.h>

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

void ai_send_report(void);

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
    uint8 interrupt_src;

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

    ai_usb_disconnect();
    ai_usb_connect();
    ai_config_value = 0;

    while (1) {
        /* main loop */
        if (0 == ai_d12_get_int_pin()) {
            ai_d12_write_cmd(AI_D12_READ_INTERRUPT_REG);
            interrupt_src = ai_d12_read_byte();
            if (interrupt_src & 0x80)
                ai_usb_bus_suspend();
            else if (interrupt_src & 0x40)
                ai_usb_bus_reset();
            else if (interrupt_src & 0x01)
                ai_usb_ep0_out();
            else if (interrupt_src & 0x02)
                ai_usb_ep0_in();
            else if (interrupt_src & 0x04)
                ai_usb_ep1_out();
            else if (interrupt_src & 0x08)
                ai_usb_ep1_in();
            else if (interrupt_src & 0x10)
                ai_usb_ep2_out();
            else if (interrupt_src & 0x20)
                ai_usb_ep2_in();
        }
        
        // 如果已经设置为非0的配置，则可以返回报告数据
        if (ai_config_value != 0) {
            // 利用板上8个LED显示按键状态，按下时亮
            AI_ALL_LEDS = ~ai_keys_pressed;
            if (!ai_ep1_in_is_busy) {
                ai_keys_can_change = 0;    // 禁止按键扫描
                if (ai_keys_down || ai_keys_up || ai_keys_pressed)
                    ai_send_report();
                ai_keys_can_change = 1;
            }
        }
    }
}

/*******************************************************************************
* Function：根据按键情况返回报告的函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  : 需要返回的4字节报告的缓冲
*           Buf[0]的D0就是左键，D1就是右键，D2就是中键（这里没有）
*           Buf[1]为X轴，Buf[2]为Y轴，Buf[3]为滚轮。
*******************************************************************************/
void ai_send_report(void)
{
    uint8 buf[4] = {0};
    
    // 我们不需要KEY1~KEY6按键改变的信息，所以先将它们清0
    ai_keys_up &= ~(AI_KEY1 | AI_KEY2 | AI_KEY3 | AI_KEY4 | AI_KEY5 | AI_KEY6);
    ai_keys_down &= ~(AI_KEY1 | AI_KEY2 | AI_KEY3 | AI_KEY4 | AI_KEY5 | AI_KEY6);

    // 如果有按键按住，并且不是KEY7、KEY8（左、右键）
    // 或者KEY7、KEY8任何一个键有变动的话，则需要返回报告
    if ((ai_keys_pressed & (~(AI_KEY7 | AI_KEY8)))
        || ai_keys_up || ai_keys_down) {
        // 如果KEY1按住，则光标需要左移，即X轴为负值
        if (ai_keys_pressed & AI_KEY1) {
            buf[1] |= -1;                   // 这里一次往左移动一个单位
        }
        // 如果KEY2按住，则光标需要右移，即X轴为正值
        if (ai_keys_pressed & AI_KEY2) {
            buf[1] |= 1;                    // 这里一次往右移动一个单位
        }
        // 如果KEY3按住，则光标需要上移，即Y轴为负值
        if (ai_keys_pressed & AI_KEY3) {
            buf[2] |= -1;                   // 这里一次往上移动一个单位
        }
        // 如果KEY4按住，则光标需要下移，即Y轴为正值
        if (ai_keys_pressed & AI_KEY4) {
            buf[2] |= 1;                    // 这里一次往下移动一个单位
        }
        // 如果KEY5按住，则滚轮下滚，即滚轮值为负
        if (ai_keys_pressed & AI_KEY5) {
            buf[3] |= -1;                   // 这里一次往下滚动一个单位
        }
        // 如果KEY6按住，则滚轮上滚，既滚轮值为正
        if (ai_keys_pressed & AI_KEY6) {
            buf[3] |= 1;                    // KEY6为数字小键盘3键
        }
        // 鼠标左键
        if (ai_keys_pressed & AI_KEY7) {
            buf[0] |= 0x01;                 // D0为鼠标左键
        }
        // 鼠标右键
        if (ai_keys_pressed & AI_KEY8) {
            buf[0] |= 0x02;                 // D1为鼠标右键
        }
    
        ai_d12_write_endp_buf(3, 4, buf);
        ai_ep1_in_is_busy = 1;
    }
    ai_keys_down = 0;
    ai_keys_up = 0;
}
