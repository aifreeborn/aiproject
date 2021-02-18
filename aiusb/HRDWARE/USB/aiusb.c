#include "aiusb.h"
#include <aiuart.h>
#include <aipdiusbd12.h>

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
 * �Ƿ���Ҫ����0���ݰ��ı�־.��USB���ƴ�������ݹ�����,
 * �����ص����ݰ��ֽ�������������ʱ,����Ϊ���ݹ��̽���.
 * ��������ֽ�����ʵ����Ҫ���ص��ֽ�����,��ʵ�ʷ��ص��ֽ�
 * ���ָպ��Ƕ˵�0��С��������ʱ,����Ҫ����һ��0���ȵ����ݰ�
 * ���������ݹ���.�����������һ����־,����������Ƿ���Ҫ����
 * һ��0���ȵ����ݰ�.
 */
uint8    ai_need_zero_packet;

// ��ǰ������ֵ,ֻ�������÷�0���ú�
uint8    ai_config_value;

// �˵�1�����Ƿ�æ�ı�־.����������������ʱ,�ñ�־Ϊ��.
// ���������п���ʱ,�ñ�־Ϊ��.
uint8    ai_ep1_in_is_busy;

/*******************************************************************************
* �豸����������ṹ
*******************************************************************************/
static code uint8 ai_device_desc[18] = {
    0x12,    // bLength�ֶ�: �豸�������ĳ���Ϊ18(0x12)�ֽ�
    0x01,    // bDescriptorType�ֶ�: �豸�������ı��Ϊ0x01
    0x10,    // bcdUSB�ֶ�: �������ð汾ΪUSB1.1,��0x0110.
    0x01,    // ������С�˽ṹ,���Ե��ֽ�����,��0x10,0x01.
    0x00,    // bDeviceClass�ֶ�: ���ǲ����豸�������ж����豸��,
             // ���ڽӿ��������ж����豸��,���Ը��ֶε�ֵΪ0.
    0x00,    // bDeviceSubClass�ֶ�: bDeviceClass�ֶ�Ϊ0ʱ,���ֶ�ҲΪ0.
    0x00,    // bDeviceProtocol�ֶ�: bDeviceClass�ֶ�Ϊ0ʱ,���ֶ�ҲΪ0.
    0x10,    // bMaxPacketSize0�ֶΣ�PDIUSBD12�Ķ˵�0��С��16�ֽ�.
    /* idVender�ֶ�.����ID��,��������ȡ0x8888,����ʵ����.
       ʵ�ʲ�Ʒ�������ʹ�ó���ID��,�����USBЭ�����볧��ID��.
       ע��С��ģʽ,���ֽ�����.*/
    0x88,
    0x88,
    /* idProduct�ֶ�.��ƷID��,�����ǵ�һ��ʵ��,��������ȡ0x0001.
       ע��С��ģʽ,���ֽ�Ӧ����ǰ.*/
    0x01,
    0x00,
    /* bcdDevice�ֶ�.�������USB���տ�ʼ��,�ͽ���1.0���,��0x0100.
       С��ģʽ,���ֽ�����.*/
    0x00,
    0x01,
    /* iManufacturer�ֶ�.�����ַ���������ֵ,Ϊ�˷������͹���,
       �ַ��������ʹ�1��ʼ��.*/
    0x01,
    /* iProduct�ֶ�.��Ʒ�ַ���������ֵ.�ո�����1,�����ȡ2��.
       ע���ַ�������ֵ��Ҫʹ����ͬ��ֵ.*/
    0x02,
    0x03,     // iSerialNumber�ֶ�.�豸�����к��ַ�������ֵ.����ȡ3�Ϳ�����.
    /* bNumConfigurations�ֶ�.���豸�����е�������.
       ����ֻ��Ҫһ�����þ�����,��˸�ֵ����Ϊ1.*/
    0x01
};

/*******************************************************************************
* USB��������������ṹ
* ͨ������ı����������Ķ���,����֪�����ص����뱨�����4�ֽ�.
* ��һ�ֽڵĵ�3λ������ʾ�����Ƿ��µ�,��5λΪ����0,����.
* �ڶ��ֽڱ�ʾX��ĵı���,�����ֽڱ�ʾY��ĸı���,�����ֽڱ�ʾ
* ���ֵĸı���.�������ж϶˵�1��Ӧ��Ҫ��������ĸ�ʽ����ʵ�ʵ�
* �������.
*******************************************************************************/
static code uint8 ai_report_desc[] = {
    //ÿ�п�ʼ�ĵ�һ�ֽ�Ϊ����Ŀ��ǰ׺,ǰ׺�ĸ�ʽΪ��
    // D7~D4��bTag.D3~D2��bType��D1~D0��bSize.���·ֱ��ÿ����Ŀע��.
    
    // ����һ��ȫ�֣�bTypeΪ1����Ŀ,ѡ����;ҳΪ
    // ��ͨ����Generic Desktop Page(0x01)�����һ�ֽ����ݣ�bSizeΪ1��,
    // ������ֽ����Ͳ�ע����,�Լ�����bSize���ж�.
    0x05, 0x01,    // USAGE_PAGE (Generic Desktop)
    
    // ����һ���ֲ���bTypeΪ2����Ŀ,˵����������Ӧ�ü�����;�������
    0x09, 0x02,    // USAGE (Mouse)
    
    // ����һ������Ŀ��bTypeΪ0����Ŀ,������,�����������0x01��ʾ
    // �ü�����һ��Ӧ�ü���.����������ǰ������;ҳ����;����Ϊ
    // ��ͨ�����õ����.
    0xa1, 0x01,    // COLLECTION (Application)
    
    // ����һ���ֲ���Ŀ.˵����;Ϊָ�뼯��
    0x09, 0x01,    // USAGE (Pointer)
    
    // ����һ������Ŀ,������,�����������0x00��ʾ�ü�����һ��
    // ������,��;��ǰ��ľֲ���Ŀ����Ϊָ�뼯��.
    0xa1, 0x00,    // COLLECTION (Physical)
    
    // ����һ��ȫ����Ŀ,ѡ����;ҳΪ������Button Page(0x09))
    0x05, 0x09,    // USAGE_PAGE (Button)
    
    // ����һ���ֲ���Ŀ,˵����;����СֵΪ1.ʵ������������.
    0x19, 0x01,    // USAGE_MINIMUM (Button 1)
    
    // ����һ���ֲ���Ŀ,˵����;�����ֵΪ3.ʵ����������м�.
    0x29, 0x03,    // USAGE_MAXIMUM (Button 3)
    
    // ����һ��ȫ����Ŀ,˵�����ص����ݵ��߼�ֵ���������Ƿ��ص��������ֵ����
    // ��СΪ0.��Ϊ����������Bit����ʾһ��������,�����СΪ0,���Ϊ1.
    0x15, 0x00,    // LOGICAL_MINIMUM (0)
    
    // ����һ��ȫ����Ŀ,˵���߼�ֵ���Ϊ1.
    0x25, 0x01,    // LOGICAL_MAXIMUM (1)
    
    // ����һ��ȫ����Ŀ,˵�������������Ϊ����.
    0x95, 0x03,    // REPORT_COUNT (3)
    
    // ����һ��ȫ����Ŀ,˵��ÿ��������ĳ���Ϊ1��bit.
    0x75, 0x01,    // REPORT_SIZE (1)
    
    // ����һ������Ŀ,˵����3������Ϊ1bit�������������ͳ���
    // ��ǰ�������ȫ����Ŀ�����壩������Ϊ����,
    // ����Ϊ��Data,Var,Abs.Data��ʾ��Щ���ݿ��Ա䶯,Var��ʾ
    // ��Щ�������Ƕ�����,ÿ�����ʾһ����˼.Abs��ʾ����ֵ.
    // ��������Ľ������,��һ��������bit0��ʾ����1��������Ƿ���,
    // �ڶ���������bit1��ʾ����2���Ҽ����Ƿ���,������������bit2��ʾ
    // ����3���м����Ƿ���.
    0x81, 0x02,    // INPUT (Data,Var,Abs)
    
    // ����һ��ȫ����Ŀ,˵������������Ϊ1��
    0x95, 0x01,    // REPORT_COUNT (1)
    
    // ����һ��ȫ����Ŀ,˵��ÿ��������ĳ���Ϊ5bit.
    0x75, 0x05,    // REPORT_SIZE (5)
    
    // ����һ������Ŀ,������,��ǰ������ȫ����Ŀ��֪,����Ϊ5bit,
    // ����Ϊ1��.��������Ϊ����(�����ص�����һֱ��0).
    // ���ֻ��Ϊ�˴���һ���ֽ�(ǰ������3��bit)������һЩ����
    // ����,��������û��ʵ����;��.
    0x81, 0x03,    // INPUT (Cnst,Var,Abs)
    
    // ����һ��ȫ����Ŀ,ѡ����;ҳΪ��ͨ����Generic Desktop Page(0x01)
    0x05, 0x01,    // USAGE_PAGE (Generic Desktop)
    
    // ����һ���ֲ���Ŀ,˵����;ΪX��
    0x09, 0x30,    // USAGE (X)
    
    // ����һ���ֲ���Ŀ,˵����;ΪY��
    0x09, 0x31,    // USAGE (Y)
    
    // ����һ���ֲ���Ŀ,˵����;Ϊ����
    0x09, 0x38,    // USAGE (Wheel)
    
    // ��������Ϊȫ����Ŀ,˵�����ص��߼���С�����ֵ.
    // ��Ϊ���ָ���ƶ�ʱ,ͨ���������ֵ����ʾ��,
    // ���ֵ����˼����,��ָ���ƶ�ʱ,ֻ�����ƶ���.
    // �����ƶ�ʱ,XֵΪ���������ƶ�ʱ,YֵΪ��.
    // ���ڹ���,���������Ϲ�ʱ,ֵΪ��.
    0x15, 0x81,    // LOGICAL_MINIMUM (-127)
    0x25, 0x7f,    // LOGICAL_MAXIMUM (127)
    
    // ����һ��ȫ����Ŀ,˵��������ĳ���Ϊ8bit.
    0x75, 0x08,    // REPORT_SIZE (8)
    
    // ����һ��ȫ����Ŀ,˵��������ĸ���Ϊ3��.
    0x95, 0x03,    // REPORT_COUNT (3)
    
    // ����һ������Ŀ.��˵��������8bit���������������õ�,
    // ����Ϊ��Data,Var,Rel.Data˵�������ǿ��Ա��,Var˵��
    // ��Щ�������Ƕ�����,����һ��8bit��ʾX��,�ڶ���8bit��ʾ
    // Y��,������8bit��ʾ����.Rel��ʾ��Щֵ�����ֵ.
    0x81, 0x06,    // INPUT (Data,Var,Rel)
    
    // ��������������Ŀ�����ر�ǰ��ļ�����.
    // ���ǿ�����������,����Ҫ������.bSizeΪ0,���Ժ���û����.
    0xc0,          // END_COLLECTION
    0xc0           // END_COLLECTION
};

/*******************************************************************************
* USB�������������϶���ṹ,�����������ܳ���Ϊ9+9+9+7�ֽ�
*******************************************************************************/
static code uint8 ai_cfg_desc[9 + 9 + 9 + 7] = {
    /*************** ���������� ***********************/
    // bLength�ֶ�: �����������ĳ���Ϊ9�ֽ�
    0x09,
    
    // bDescriptorType�ֶ�: �������������Ϊ0x02
    0x02,
    
    // wTotalLength�ֶ�: �������������ϵ��ܳ���,������������������
    // �ӿ��������������������˵���������.
    sizeof(ai_cfg_desc) & 0xFF,           // ���ֽ�
    (sizeof(ai_cfg_desc) >> 8) & 0xFF,    // ���ֽ�
    
    // bNumInterfaces�ֶ�: �����ð����Ľӿ���,ֻ��һ���ӿ�.
    0x01,
    
    // bConfigurationValue�ֶ�: �����õ�ֵΪ1.
    0x01,
    
    // iConfigurationz�ֶ�: �����õ��ַ�������, ����û��, Ϊ0.
    0x00,
    
    // bmAttributes�ֶ�: ���豸������. �������ǵİ��������߹����,
    // �������ǲ���ʵ��Զ�̻��ѵĹ���,���Ը��ֶε�ֵΪ0x80.
    0x80,
    
    // bMaxPower�ֶ�,���豸��Ҫ����������.�������ǵİ�����Ҫ�ĵ�������100mA,
    // ���������������Ϊ100mA.����ÿ��λ����Ϊ2mA,������������Ϊ50(0x32).
    0x32,
    
    /******************* �ӿ������� *********************/
    // bLength�ֶ�: �ӿ��������ĳ���Ϊ9�ֽ�.
    0x09,
    
    // bDescriptorType�ֶ�: �ӿ��������ı��Ϊ0x04.
    0x04,
    
    // bInterfaceNumber�ֶ�: �ýӿڵı��,��һ���ӿ�,���Ϊ0.
    0x00,
    
    // bAlternateSetting�ֶ�: �ýӿڵı��ñ��,Ϊ0.
    0x00,
    
    // bNumEndpoints�ֶ�: ��0�˵����Ŀ.����USB���ֻ��Ҫһ���ж�����˵�,
    // ��˸�ֵΪ1.
    0x01,
    
    // bInterfaceClass�ֶ�: �ýӿ���ʹ�õ���.USB�����HID��,����Ϊ0x03.
    0x03,
    
    // bInterfaceSubClass�ֶ�: �ýӿ���ʹ�õ�����.��HID1.1Э����,ֻ�涨��һ��
    // ���ࣺ֧��BIOS��������������.USB���̡�������ڸ�����,�������Ϊ0x01.
    0x01,
    
    // bInterfaceProtocol�ֶ�: �������Ϊ֧����������������,��Э���ѡ������
    // ����.���̴���Ϊ0x01,������Ϊ0x02.
    0x02,
    
    // iInterface�ֶ�.�ýӿڵ��ַ�������ֵ.����û��,Ϊ0.
    0x00,
    
    /****************** HID������ ************************/
    // bLength�ֶ�: ��HID��������ֻ��һ���¼�������.���Գ���Ϊ9�ֽ�.
    0x09,
    
    // bDescriptorType�ֶ�: HID�������ı��Ϊ0x21.
    0x21,
    
    // bcdHID�ֶ�: ��Э��ʹ�õ�HID1.1Э��.ע����ֽ�����.
    0x10,
    0x01,
    
    // bCountyCode�ֶ�: �豸���õĹ��Ҵ���,����ѡ��Ϊ����,����0x21.
    0x21,
    
    // bNumDescriptors�ֶ�: �¼�����������Ŀ.����ֻ��һ������������.
    0x01,
    
    // bDescritporType�ֶ�: �¼�������������,Ϊ����������,���Ϊ0x22.
    0x22,
    
    // bDescriptorLength�ֶ�: �¼��������ĳ���.�¼�������Ϊ����������.
    sizeof(ai_report_desc)&0xFF,
    (sizeof(ai_report_desc)>>8)&0xFF,
    
    /********************** �˵������� ***********************/
    // bLength�ֶ�: �˵�����������Ϊ7�ֽ�.
    0x07,
    
    // bDescriptorType�ֶ�: �˵����������Ϊ0x05.
    0x05,
    
    // bEndpointAddress�ֶ�: �˵�ĵ�ַ.����ʹ��D12������˵�1.
    // D7λ��ʾ���ݷ���,����˵�D7Ϊ1.��������˵�1�ĵ�ַΪ0x81.
    0x81,
    
    // bmAttributes�ֶ�: D1~D0Ϊ�˵㴫������ѡ��.
    // �ö˵�Ϊ�ж϶˵�.�ж϶˵�ı��Ϊ3.����λ����Ϊ0.
    0x03,
    
    // wMaxPacketSize�ֶ�: �ö˵��������.�˵�1��������Ϊ16�ֽ�.
    // ע����ֽ�����.
    0x10,
    0x00,
    
    // bInterval�ֶ�: �˵��ѯ��ʱ��,��������Ϊ10��֡ʱ��,��10ms.
    0x0A
};

/*******************************************************************************
* ����ID�Ķ���
*******************************************************************************/
static code uint8 ai_lang_id[4] = {
    0x04,
    0x03,
    0x09,    // 0x0409Ϊ��ʽӢ���ID
    0x04
};

/*******************************************************************************
* �ַ����������Ķ���ṹ,8λС�˸�ʽ
*******************************************************************************/
// �ַ���������ȦȦ��USBר�� Http://group.ednchina.com/93/����Unicode����
static code uint8 ai_mfr_str_desc[82] = {
    82,         //���������ĳ���Ϊ82�ֽ�
    0x03,       //�ַ��������������ͱ���Ϊ0x03
    0x35, 0x75, //��
    0x11, 0x81, //��
    0x08, 0x57, //Ȧ
    0x08, 0x57, //Ȧ
    0x84, 0x76, //��
    0x55, 0x00, //U
    0x53, 0x00, //S
    0x42, 0x00, //B
    0x13, 0x4e, //ר
    0x3a, 0x53, //��
    0x20, 0x00, // 
    0x48, 0x00, //H
    0x74, 0x00, //t
    0x74, 0x00, //t
    0x70, 0x00, //p
    0x3a, 0x00, //:
    0x2f, 0x00, ///
    0x2f, 0x00, ///
    0x67, 0x00, //g
    0x72, 0x00, //r
    0x6f, 0x00, //o
    0x75, 0x00, //u
    0x70, 0x00, //p
    0x2e, 0x00, //.
    0x65, 0x00, //e
    0x64, 0x00, //d
    0x6e, 0x00, //n
    0x63, 0x00, //c
    0x68, 0x00, //h
    0x69, 0x00, //i
    0x6e, 0x00, //n
    0x61, 0x00, //a
    0x2e, 0x00, //.
    0x63, 0x00, //c
    0x6f, 0x00, //o
    0x6d, 0x00, //m
    0x2f, 0x00, ///
    0x39, 0x00, //9
    0x33, 0x00, //3
    0x2f, 0x00  /// 
};

// �ַ�������ȦȦ������USB��֮USB��ꡱ��Unicode����
static code uint8 ai_product_str_desc[34] = {
    34,         //���������ĳ���Ϊ34�ֽ�
    0x03,       //�ַ��������������ͱ���Ϊ0x03
    0x0a, 0x30, //��
    0x08, 0x57, //Ȧ
    0x08, 0x57, //Ȧ
    0x59, 0x65, //��
    0x60, 0x4f, //��
    0xa9, 0x73, //��
    0x55, 0x00, //U
    0x53, 0x00, //S
    0x42, 0x00, //B
    0x0b, 0x30, //��
    0x4b, 0x4e, //֮
    0x55, 0x00, //U
    0x53, 0x00, //S
    0x42, 0x00, //B
    0x20, 0x9f, //��
    0x07, 0x68  //��
};

// �ַ�����2008-07-07����Unicode����
static code uint8 ai_sn_str_desc[22] = {
    22,         //���������ĳ���Ϊ22�ֽ�
    0x03,       //�ַ��������������ͱ���Ϊ0x03
    0x32, 0x00, //2
    0x30, 0x00, //0
    0x32, 0x00, //2
    0x31, 0x00, //1
    0x2d, 0x00, //-
    0x30, 0x00, //0
    0x32, 0x00, //2
    0x2d, 0x00, //-
    0x31, 0x00, //1
    0x38, 0x00  //8 
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
    ai_ep1_in_is_busy = 0;
}

/*******************************************************************************
* Function������ai_send_data��ai_send_length�����ݷ��͵��˵�0�ĺ���.
* Input   : ��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
static void ai_usb_ep0_send_data(void)
{
    // ������д���˵���ȥ,׼������д֮ǰҪ���ж�һ����Ҫ���͵������Ƿ�ȶ˵�0
    // ��󳤶ȴ���������˵��С����һ��ֻ�ܷ���������������
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
                        // ������������豸������,������ݳ��Ⱦ���
                        // ai_device_desc[0].�������ı�ʵ�ʵĳ�,
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
                        ai_send_data = ai_cfg_desc;
                        // �ж�������ֽ����Ƿ��ʵ����Ҫ���͵��ֽ�����
                        // ������������������������ϣ�������ݳ��Ⱦ���
                        // ai_cfg_desc[3] * 256 + ai_cfg_desc[2]
                        // �������ı�ʵ�ʵĳ�����ôֻ����ʵ�ʳ��ȵ���
                        ai_send_length = ai_cfg_desc[3] * 256 + ai_cfg_desc[2];
                        if (ai_length > ai_send_length) {
                            // ���Ҹպ������������ݰ�ʱ
                            if (ai_send_length % ai_device_desc[7] == 0) {
                                ai_need_zero_packet = 1;
                            }
                        } else {
                            ai_send_length = ai_length;
                        }
                        // ������ͨ��EP0����
                        ai_usb_ep0_send_data();
                        break;
                    case AIUSB_STRING_DESCRIPTOR:
                        ai_uart_send_str("�ַ���������");
                        // ����wValue�ĵ��ֽڣ�����ֵ��ɢת
                        switch (ai_value & 0xff) {
                        case 0:    // ��ȡ����ID
                            ai_uart_send_str("(����ID).\r\n");
                            ai_send_data = ai_lang_id;
                            ai_send_length = ai_lang_id[0];
                            break;
                        case 1:    // ��ȡ�����ַ���
                            ai_uart_send_str("(��������).\r\n");
                            ai_send_data = ai_mfr_str_desc;
                            ai_send_length = ai_mfr_str_desc[0];
                            break;
                        case 2:
                            ai_uart_send_str("(��Ʒ����).\r\n");
                            ai_send_data = ai_product_str_desc;
                            ai_send_length = ai_product_str_desc[0];
                            break;
                        case 3:
                            ai_uart_send_str("(��Ʒ���к�).\r\n");
                            ai_send_data = ai_sn_str_desc;
                            ai_send_length = ai_sn_str_desc[0];
                            break;
                        default:
                            ai_uart_send_str("(δ֪������ֵ).\r\n");
                            ai_send_length = 0;
                            ai_need_zero_packet = 1;
                        }
                        if (ai_length > ai_send_length) {
                            // ���Ҹպ������������ݰ�ʱ
                            if (ai_send_length % ai_device_desc[7] == 0) {
                                ai_need_zero_packet = 1;
                            }
                        } else {
                            ai_send_length = ai_length;
                        }
                        // ������ͨ��EP0����
                        ai_usb_ep0_send_data();
                        break;
                    case AIUSB_INTERFACE_DESCRIPTOR:
                        break;
                    case AIUSB_ENDPOINT_DESCRIPTOR:
                        break;
                    case AIUSB_REPORT_DESCRIPTOR:
                        ai_uart_send_str("����������.\r\n");
                        ai_send_data = ai_report_desc;
                        ai_send_length = sizeof(ai_report_desc);
                        if (ai_length > ai_send_length) {
                            // ���Ҹպ������������ݰ�ʱ
                            if (ai_send_length % ai_device_desc[7] == 0) {
                                ai_need_zero_packet = 1;
                            }
                        } else {
                            ai_send_length = ai_length;
                        }
                        // ������ͨ��EP0����
                        ai_usb_ep0_send_data();
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
                    ai_uart_send_str("���õ�ַ,��ַΪ��");
                    ai_uart_print_hex(ai_value & 0xff);
                    ai_uart_send_str("\r\n");
                    ai_d12_set_addr(ai_value & 0xff);
                    // ���õ�ַû�����ݹ���,ֱ�ӽ��뵽״̬����,
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
                    // ʹ�ܷ�0�˵�.��0�˵�ֻ��������Ϊ��0�����ú����ʹ��.
                    // ai_value�ĵ��ֽ�Ϊ���õ�ֵ�������ֵΪ��0��
                    // ����ʹ�ܷ�0�˵�.���浱ǰ����ֵ
                    ai_config_value = ai_value & 0xff;
                    ai_d12_set_endp_enable(ai_config_value);
                    ai_send_length = 0;
                    ai_need_zero_packet = 1;
                    ai_usb_ep0_send_data();
                    break;
                case AIUSB_SET_INTERFACE:
                    ai_uart_send_str("���ýӿ�.\r\n");
                    break;
                default:
                    ai_uart_send_str("����δ����ı�׼�������.\r\n");
                }
                break;
            case 1:
                ai_uart_send_str("USB���������: ");
                switch (ai_request) {
                case AIUSB_REQUSET_SET_IDLE:
                    ai_uart_send_str("���ÿ���.\r\n");
                    ai_send_length = 0;
                    ai_need_zero_packet = 1;
                    ai_usb_ep0_send_data();
                    break;
                case AIUSB_REQUSET_GET_REPORT:
                    ai_uart_send_str("��ȡ����.\r\n");
                    break;
                case AIUSB_REQUSET_GET_IDLE:
                    ai_uart_send_str("��ȡ����.\r\n");
                    break;
                default:
                   ai_uart_send_str("δ֪����.\r\n"); 
                };
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
    // �������״̬���⽫����˵�1������жϱ�־λ
    ai_d12_read_endp_last_stat(3);
    // �˵�1���봦�ڿ���״̬
    ai_ep1_in_is_busy = 0;
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