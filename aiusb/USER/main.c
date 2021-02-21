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
            if (!ai_ep1_in_is_busy) {
                ai_keys_can_change = 0;    // ��ֹ����ɨ��
                if (ai_keys_down || ai_keys_up)
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
* Others  : ��Ҫ���ص�8�ֽڱ���Ļ���
*           ͨ�������������Ķ��弰HID��;���ĵ�����֪Buf[0]��D0����Ctrl����
*           D1����Shift����D2����Alt����D3����GUI����Window������
*           D4����Ctrl��D5����Shift��D6����Alt��D7����GUI����
*           Buf[1]������ֵΪ0��Buf[2]~Buf[7]Ϊ��ֵ����������6����
*           ��������������ͨ�����ֻ��5������˲��ᳬ��6����
*           ����ʵ�ʵļ��̣����������̫��ʱ�������6���ֽڶ�Ϊ0xFF��
*           ��ʾ���µļ�̫�࣬�޷���ȷ���ء�
*******************************************************************************/
void ai_send_report(void)
{
    uint8 buf[8] = {0};
    // ������Ҫ���ض��������������Ҫ����һ�����������浱ǰ��λ��
    uint8 i = 2;

    // ���ݲ�ͬ�İ����������뱨��
    if (ai_keys_pressed & AI_KEY1) {
        buf[0] |= 0x01;                   // KEY1Ϊ��Ctrl��
    }
    if (ai_keys_pressed & AI_KEY2) {
        buf[0] |= 0x02;                   // KEY2Ϊ��Shift��
    }
    if (ai_keys_pressed & AI_KEY3) {
        buf[0] |= 0x04;                   // KEY3Ϊ��Alt��
    }
    if (ai_keys_pressed & AI_KEY4) {
        buf[i] = 0x59;                    // KEY4Ϊ����С����1��
        i++;
    }
    if (ai_keys_pressed & AI_KEY5) {
        buf[i] = 0x5a;                    // KEY5����С����2��
        i++;
    }
    if (ai_keys_pressed & AI_KEY6) {  
        buf[i] = 0x5b;                    // KEY6Ϊ����С����3��
        i++;
    }
    if (ai_keys_pressed & AI_KEY7) {
        buf[i] = 0x39;                    // KEY7Ϊ��/Сд�л���
        i++;
    }
    if (ai_keys_pressed & AI_KEY8) {
        buf[i] = 0x53;                    // KEY8Ϊ����С���̹����л���
    }

    // ����׼�����ˣ�ͨ���˵�1���أ�����Ϊ8�ֽ�
    ai_d12_write_endp_buf(3, 8, buf);
    // ���ö˵�æ��־
    ai_ep1_in_is_busy = 1;
    ai_keys_down = 0;
    ai_keys_up = 0;
}
