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
            if (!ai_ep1_in_is_busy) {
                ai_keys_can_change = 0;    // 禁止按键扫描
                if (ai_keys_down || ai_keys_up)
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
* Others  : 需要返回的8字节报告的缓冲
*           通过报告描述符的定义及HID用途表文档，可知Buf[0]的D0是左Ctrl键，
*           D1是左Shift键，D2是左Alt键，D3是左GUI（即Window键），
*           D4是右Ctrl，D5是右Shift，D6是右Alt，D7是右GUI键。
*           Buf[1]保留，值为0。Buf[2]~Buf[7]为键值，最多可以有6个。
*           由于我们这里普通键最多只有5个，因此不会超过6个。
*           对于实际的键盘，如果按键数太多时，后面的6个字节都为0xFF，
*           表示按下的键太多，无法正确返回。
*******************************************************************************/
void ai_send_report(void)
{
    uint8 buf[8] = {0};
    // 由于需要返回多个按键，所以需要增加一个变量来保存当前的位置
    uint8 i = 2;

    // 根据不同的按键设置输入报告
    if (ai_keys_pressed & AI_KEY1) {
        buf[0] |= 0x01;                   // KEY1为左Ctrl键
    }
    if (ai_keys_pressed & AI_KEY2) {
        buf[0] |= 0x02;                   // KEY2为左Shift键
    }
    if (ai_keys_pressed & AI_KEY3) {
        buf[0] |= 0x04;                   // KEY3为左Alt键
    }
    if (ai_keys_pressed & AI_KEY4) {
        buf[i] = 0x59;                    // KEY4为数字小键盘1键
        i++;
    }
    if (ai_keys_pressed & AI_KEY5) {
        buf[i] = 0x5a;                    // KEY5数字小键盘2键
        i++;
    }
    if (ai_keys_pressed & AI_KEY6) {  
        buf[i] = 0x5b;                    // KEY6为数字小键盘3键
        i++;
    }
    if (ai_keys_pressed & AI_KEY7) {
        buf[i] = 0x39;                    // KEY7为大/小写切换键
        i++;
    }
    if (ai_keys_pressed & AI_KEY8) {
        buf[i] = 0x53;                    // KEY8为数字小键盘功能切换键
    }

    // 报告准备好了，通过端点1返回，长度为8字节
    ai_d12_write_endp_buf(3, 8, buf);
    // 设置端点忙标志
    ai_ep1_in_is_busy = 1;
    ai_keys_down = 0;
    ai_keys_up = 0;
}
