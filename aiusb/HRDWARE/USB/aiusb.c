#include "aiusb.h"
#include <aiuart.h>
#include <aipdiusbd12.h>

uint8 ai_usb_ep1_in_is_busy;

idata uint8 ai_buffer[16];
// USB�豸����ĸ��ֶ�
uint8     ai_request_type;
uint8     ai_request;
uint16    ai_value;
uint16    ai_index;
uint16    ai_length;
// ��ǰ�������ݵ�λ��
uint8     *ai_send_data;
// ��Ҫ�������ݵĳ���
uint16    ai_send_length;
/* 
 * �Ƿ���Ҫ����0���ݰ��ı�־����USB���ƴ�������ݹ����У�
 * �����ص����ݰ��ֽ�������������ʱ������Ϊ���ݹ��̽�����
 * ��������ֽ�����ʵ����Ҫ���ص��ֽ���������ʵ�ʷ��ص��ֽ�
 * ���ָպ��Ƕ˵�0��С��������ʱ������Ҫ����һ��0���ȵ����ݰ�
 * ���������ݹ��̡������������һ����־������������Ƿ���Ҫ����
 * һ��0���ȵ����ݰ���
 */
uint8    ai_need_zero_packet;

// ��ǰ������ֵ,ֻ�������÷�0���ú�
uint8    ai_config_value;

// �˵�1�����Ƿ�æ�ı�־������������������ʱ���ñ�־Ϊ�档
// ���������п���ʱ���ñ�־Ϊ�١�
uint8    ai_ep1_in_is_busy;

/*******************************************************************************
* �豸����������ṹ
*******************************************************************************/
static code uint8 ai_device_desc[18] = {
    0x12,    // bLength�ֶ�: �豸�������ĳ���Ϊ18(0x12)�ֽ�
    0x01,    // bDescriptorType�ֶ�: �豸�������ı��Ϊ0x01
    0x10,    // bcdUSB�ֶ�: �������ð汾ΪUSB1.1����0x0110.
    0x01,    // ������С�˽ṹ�����Ե��ֽ����ȣ���0x10��0x01.
    0x00,    // bDeviceClass�ֶ�: ���ǲ����豸�������ж����豸�࣬
             // ���ڽӿ��������ж����豸�࣬���Ը��ֶε�ֵΪ0��
    0x00,    // bDeviceSubClass�ֶ�: bDeviceClass�ֶ�Ϊ0ʱ�����ֶ�ҲΪ0��
    0x00,    // bDeviceProtocol�ֶ�: bDeviceClass�ֶ�Ϊ0ʱ�����ֶ�ҲΪ0��
    0x10,    // bMaxPacketSize0�ֶΣ�PDIUSBD12�Ķ˵�0��С��16�ֽڡ�
    /* idVender�ֶΡ�����ID�ţ���������ȡ0x8888������ʵ���á�
       ʵ�ʲ�Ʒ�������ʹ�ó���ID�ţ������USBЭ�����볧��ID�š�
       ע��С��ģʽ�����ֽ����ȡ�*/
    0x88,
    0x88,
    /* idProduct�ֶΡ���ƷID�ţ������ǵ�һ��ʵ�飬��������ȡ0x0001��
       ע��С��ģʽ�����ֽ�Ӧ����ǰ��*/
    0x01,
    0x00,
    /* bcdDevice�ֶΡ��������USB���տ�ʼ�����ͽ���1.0��ɣ���0x0100��
       С��ģʽ�����ֽ����ȡ�*/
    0x00,
    0x01,
    /* iManufacturer�ֶΡ������ַ���������ֵ��Ϊ�˷������͹���
       �ַ��������ʹ�1��ʼ�ɡ�*/
    0x01,
    /* iProduct�ֶΡ���Ʒ�ַ���������ֵ���ո�����1�������ȡ2�ɡ�
       ע���ַ�������ֵ��Ҫʹ����ͬ��ֵ��*/
    0x02,
    0x03,     // iSerialNumber�ֶΡ��豸�����к��ַ�������ֵ������ȡ3�Ϳ����ˡ�
    /* bNumConfigurations�ֶΡ����豸�����е���������
       ����ֻ��Ҫһ�����þ����ˣ���˸�ֵ����Ϊ1��*/
    0x01
};

/*******************************************************************************
* Function����ʱn���뺯��
* Input   : n -- ��ʱ�ĺ�����
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_delay_nms(uint16 n)
{
    uint16 i;
    uint16 j;

    for (i = 0; i < n; i++)
        for (j = 0; j < 227; j++)
            /* empty statement */
            ;
}

/*******************************************************************************
* Function��USB�Ͽ����Ӻ���
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_disconnect(void)
{
    ai_uart_send_str("�Ͽ�USB����.\r\n");
    ai_d12_write_cmd(AI_D12_SET_MODE);
    ai_d12_write_byte(0x06);           // ����ģʽ�ĵ�һ���ֽ�
    ai_d12_write_byte(0x47);           // ����ģʽ�ĵڶ����ֽ�
    ai_delay_nms(1000);
}

/*******************************************************************************
* Function��USB���Ӻ���
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_connect(void)
{
    ai_uart_send_str("USB����.\r\n");
    ai_d12_write_cmd(AI_D12_SET_MODE);
    ai_d12_write_byte(0x16);           // ����ģʽ�ĵ�һ���ֽ�
    ai_d12_write_byte(0x47);           // ����ģʽ�ĵڶ����ֽ�
}

/*******************************************************************************
* Function��USB���߹����жϴ�����
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_bus_suspend(void)
{
    ai_uart_send_str("USB���߹���\r\n");
}

/*******************************************************************************
* Function��USB���߸�λ�жϴ�����
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_bus_reset(void)
{
    ai_uart_send_str("USB���߸�λ\r\n");
    // ��λ��˵�1���뻺��������
    ai_usb_ep1_in_is_busy = 0;
}

/*******************************************************************************
* Function������ai_send_data��ai_send_length�����ݷ��͵��˵�0�ĺ�����
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
static void ai_usb_ep0_send_data(void)
{
    // ������д���˵���ȥ,׼������д֮ǰҪ���ж�һ����Ҫ���͵������Ƿ�ȶ˵�0
    // ��󳤶ȴ���������˵��С����һ��ֻ�ܷ��������������ݡ�
    // �˵�0����������ai_device_desc[7]
    if (ai_send_length > ai_device_desc[7]) {
        ai_d12_write_endp_buf(1, ai_device_desc[7], ai_send_data);
        ai_send_length -= ai_device_desc[7];
        ai_send_data += ai_device_desc[7];
    } else {
        if (ai_send_length != 0) {
            ai_d12_write_endp_buf(1, ai_send_length, ai_send_data);
            ai_send_length = 0;
        } else {
            if (ai_need_zero_packet == 1) {
                ai_d12_write_endp_buf(1, 0, ai_send_data);
                ai_need_zero_packet = 0;
            }
        }
    }
}

/*******************************************************************************
* Function���˵�0����жϴ�����
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep0_out(void)
{
    ai_uart_send_str("USB�˵�0����ж�.\r\n");

    // �ж��Ƿ��ǽ�����
    if (ai_d12_read_endp_last_stat(0) & 0x20) {
        ai_d12_read_end_point_buf(0, 16, ai_buffer);
        ai_d12_ack_setup();
        ai_d12_clear_buf();

        ai_request_type = ai_buffer[0];
        ai_request = ai_buffer[1];
        ai_value = ai_buffer[2] + (((uint16)ai_buffer[3]) << 8);
        ai_index = ai_buffer[4] + (((uint16)ai_buffer[5]) << 8);
        ai_length = ai_buffer[6] + (((uint16)ai_buffer[7]) << 8);

        // ���ݲ�ͬ�����������ز���
        if ((ai_request_type & 0x80) == 0x80) {
            // �豸������
            switch ((ai_request_type >> 5) & 0x03) {
            case 0:    // ��׼����
                ai_uart_send_str("USB��׼��������: ");
                switch (ai_request) {
                case AIUSB_GET_STATUS:
                    ai_uart_send_str("��ȡ״̬.\r\n");
                    break;
                case AIUSB_GET_DESCRIPTOR:
                    ai_uart_send_str("��ȡ������--");
                    // ai_value�ĵ�һ���ֽ�(���ֽ�)�������ţ�
                    // �ڶ����ֽ��������������ͱ��
                    switch ((ai_value >> 8) & 0xff) {
                    case AIUSB_DEVICE_DESCRIPTOR:
                        ai_uart_send_str("�豸������.\r\n");
                        ai_send_data = ai_device_desc;
                        // �ж�������ֽ����Ƿ��ʵ����Ҫ���͵��ֽ�����
                        // ������������豸��������������ݳ��Ⱦ���
                        // ai_device_desc[0]���������ı�ʵ�ʵĳ���
                        // ��ôֻ����ʵ�ʳ��ȵ�����
                        if (ai_length > ai_device_desc[0]) {
                            ai_send_length = ai_device_desc[0];
                            if (ai_send_length % ai_device_desc[7] == 0)
                                ai_need_zero_packet = 1;
                        } else {
                            ai_send_length = ai_length;
                        }
                        ai_usb_ep0_send_data();
                        break;
                    case AIUSB_CONFIGURATION_DESCRIPTOR:
                        ai_uart_send_str("����������.\r\n");
                        break;
                    case AIUSB_STRING_DESCRIPTOR:
                        ai_uart_send_str("�ַ���������.\r\n");
                        break;
                    case AIUSB_INTERFACE_DESCRIPTOR:
                        break;
                    case AIUSB_ENDPOINT_DESCRIPTOR:
                        break;
                    default:
                        ai_uart_send_str("����������, ����������:");
                        ai_uart_print_hex((ai_value >> 8) & 0xff);
                        ai_uart_send_str("\r\n");
                    }
                    break;
                case AIUSB_GET_CONFIGURATION:
                    ai_uart_send_str("��ȡ����.\r\n");
                    break;
                case AIUSB_GET_INTERFACE:
                    ai_uart_send_str("��ȡ�ӿ�.\r\n");
                    break;
                case AIUSB_SYNCH_FRAME:
                    ai_uart_send_str("ͬ��֡.\r\n");
                    break;
                default:
                    ai_uart_send_str("����δ����ı�׼��������.\r\n");
                }
                break;
            case 1:
                ai_uart_send_str("USB����������: \r\n");
                break;
            case 2:
                ai_uart_send_str("USB������������: \r\n");
                break;
            default:
                ai_uart_send_str("����δ�������������.\r\n");
            }
        } else {
            // �������豸
            switch ((ai_request_type >> 5) & 0x03) {
            case 0:
                ai_uart_send_str("USB��׼�������: ");
                switch (ai_request) {
                case AIUSB_CLEAR_FEATURE:
                    ai_uart_send_str("�������.\r\n");
                    break;
                case AIUSB_SET_FEATURE:
                    ai_uart_send_str("��������.\r\n");
                    break;
                case AIUSB_SET_ADDRESS:
                    ai_uart_send_str("���õ�ַ����ַΪ��");
                    ai_uart_print_hex(ai_value & 0xff);
                    ai_uart_send_str("\r\n");
                    ai_d12_set_addr(ai_value & 0xff);
                    // ���õ�ַû�����ݹ��̣�ֱ�ӽ��뵽״̬���̣�
                    // ����һ��0���ȵ����ݰ�
                    ai_send_length = 0;
                    ai_need_zero_packet = 1;
                    ai_usb_ep0_send_data();
                    break;
                case AIUSB_SET_DESCRIPTOR:
                    ai_uart_send_str("����������.\r\n");
                    break;
                case AIUSB_SET_CONFIGURATION:
                    ai_uart_send_str("��������.\r\n");
                    break;
                case AIUSB_SET_INTERFACE:
                    ai_uart_send_str("���ýӿ�.\r\n");
                    break;
                default:
                    ai_uart_send_str("����δ����ı�׼�������.\r\n");
                }
                break;
            case 1:
                ai_uart_send_str("USB���������: \r\n");
                break;
            case 2:
                ai_uart_send_str("USB�����������: \r\n");
                break;
            default:
                ai_uart_send_str("����δ������������.\r\n");
            }
        }
    } else {
        ai_d12_read_end_point_buf(0, 16, ai_buffer);
        ai_d12_clear_buf(); 
    }
}

/*******************************************************************************
* Function���˵�0�����жϴ�����
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep0_in(void)
{
    ai_uart_send_str("USB�˵�0�����ж�.\r\n");
    ai_d12_read_endp_last_stat(1);
    ai_usb_ep0_send_data();
}

/*******************************************************************************
* Function���˵�1����жϴ�����
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep1_out(void)
{
    ai_uart_send_str("USB�˵�1����ж�.\r\n");
}

/*******************************************************************************
* Function���˵�1�����жϴ�����
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep1_in(void)
{
    ai_uart_send_str("USB�˵�1�����ж�.\r\n");
}

/*******************************************************************************
* Function���˵�2����жϴ�����
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep2_out(void)
{
    ai_uart_send_str("USB�˵�2����ж�.\r\n");
}

/*******************************************************************************
* Function���˵�2�����жϴ�����
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep2_in(void)
{
    ai_uart_send_str("USB�˵�2�����ж�.\r\n");
}