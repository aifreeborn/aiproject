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
    "******            ��ȦȦ������USB����ϰ����                   ******\r\n",
    "******                  STC89C52RC CPU                        ******\r\n",
    "******               �������ڣ�",__DATE__,"                    ******\r\n",
    "******               ����ʱ�䣺",__TIME__,"                       ******\r\n",
    "******               ���ߣ�takeno                             ******\r\n",
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

    // ��ʾ��ʾ��Ϣ
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
        
        // ����Ѿ�����Ϊ��0�����ã�����Է��ر�������
        if (ai_config_value != 0) {
            // ���ð���8��LED��ʾ����״̬������ʱ��
            AI_ALL_LEDS = ~ai_keys_pressed;
            if (!ai_ep1_in_is_busy) {
                ai_keys_can_change = 0;    // ��ֹ����ɨ��
                if (ai_keys_down || ai_keys_up || ai_keys_pressed)
                    ai_send_report();
                ai_keys_can_change = 1;
            }
        }
    }
}

/*******************************************************************************
* Function�����ݰ���������ر���ĺ���
* Input   : ��
* Ouput   : None
* Return  : None
* Others  : ��Ҫ���ص�4�ֽڱ���Ļ���
*           Buf[0]��D0���������D1�����Ҽ���D2�����м�������û�У�
*           Buf[1]ΪX�ᣬBuf[2]ΪY�ᣬBuf[3]Ϊ���֡�
*******************************************************************************/
void ai_send_report(void)
{
    uint8 buf[4] = {0};
    
    // ���ǲ���ҪKEY1~KEY6�����ı����Ϣ�������Ƚ�������0
    ai_keys_up &= ~(AI_KEY1 | AI_KEY2 | AI_KEY3 | AI_KEY4 | AI_KEY5 | AI_KEY6);
    ai_keys_down &= ~(AI_KEY1 | AI_KEY2 | AI_KEY3 | AI_KEY4 | AI_KEY5 | AI_KEY6);

    // ����а�����ס�����Ҳ���KEY7��KEY8�����Ҽ���
    // ����KEY7��KEY8�κ�һ�����б䶯�Ļ�������Ҫ���ر���
    if ((ai_keys_pressed & (~(AI_KEY7 | AI_KEY8)))
        || ai_keys_up || ai_keys_down) {
        // ���KEY1��ס��������Ҫ���ƣ���X��Ϊ��ֵ
        if (ai_keys_pressed & AI_KEY1) {
            buf[1] |= -1;                   // ����һ�������ƶ�һ����λ
        }
        // ���KEY2��ס��������Ҫ���ƣ���X��Ϊ��ֵ
        if (ai_keys_pressed & AI_KEY2) {
            buf[1] |= 1;                    // ����һ�������ƶ�һ����λ
        }
        // ���KEY3��ס��������Ҫ���ƣ���Y��Ϊ��ֵ
        if (ai_keys_pressed & AI_KEY3) {
            buf[2] |= -1;                   // ����һ�������ƶ�һ����λ
        }
        // ���KEY4��ס��������Ҫ���ƣ���Y��Ϊ��ֵ
        if (ai_keys_pressed & AI_KEY4) {
            buf[2] |= 1;                    // ����һ�������ƶ�һ����λ
        }
        // ���KEY5��ס��������¹���������ֵΪ��
        if (ai_keys_pressed & AI_KEY5) {
            buf[3] |= -1;                   // ����һ�����¹���һ����λ
        }
        // ���KEY6��ס��������Ϲ����ȹ���ֵΪ��
        if (ai_keys_pressed & AI_KEY6) {
            buf[3] |= 1;                    // KEY6Ϊ����С����3��
        }
        // ������
        if (ai_keys_pressed & AI_KEY7) {
            buf[0] |= 0x01;                 // D0Ϊ������
        }
        // ����Ҽ�
        if (ai_keys_pressed & AI_KEY8) {
            buf[0] |= 0x02;                 // D1Ϊ����Ҽ�
        }
    
        ai_d12_write_endp_buf(3, 4, buf);
        ai_ep1_in_is_busy = 1;
    }
    ai_keys_down = 0;
    ai_keys_up = 0;
}
