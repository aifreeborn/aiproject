#include "aipdiusbd12.h"
#include <aiuart.h>


/*******************************************************************************
* Function��PDIUSBD12д����
* Input   : cmd -- һ�ֽ�����
* Ouput   : None
* Return  : void
* Others  :
*******************************************************************************/
void ai_d12_write_cmd(uint8 cmd)
{
    ai_d12_set_cmd_addr();
    ai_d12_clr_wr();
    aid12_set_port_out();
    ai_d12_set_data(cmd);
    ai_d12_set_wr();
    ai_d12_set_port_in();
}

/*******************************************************************************
* Function����һ�ֽ�PDIUSBD12����
* Input   : void
* Ouput   : None
* Return  : uint8 -- ���ص�һ�ֽ�
* Others  :
*******************************************************************************/
uint8 ai_d12_read_byte(void)
{
    uint8 tmp;

    ai_d12_set_data_addr();
    ai_d12_clr_rd();
    tmp = ai_d12_get_data();
    ai_d12_set_rd();
    return tmp;
}

/*******************************************************************************
* Function����PDIUSBD12��ID
* Input   : void
* Ouput   : None
* Return  : uint16 -- 16λ��IDֵ
* Others  :
*******************************************************************************/
uint16 ai_d12_read_id(void)
{
    uint16 id;

    ai_d12_write_cmd(AI_D12_READ_ID);
    id = ai_d12_read_byte();
    id |= ((uint16)ai_d12_read_byte()) << 8;

    return id;
}

/*******************************************************************************
* Function��дһ�ֽ�PDIUSBD12����
* Input   : value -- Ҫд��1�ֽ�����
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_d12_write_byte(uint8 value)
{
    ai_d12_set_data_addr();
    ai_d12_clr_wr();
    aid12_set_port_out();
    ai_d12_set_data(value);
    ai_d12_set_wr();
    ai_d12_set_port_in();
}

/*******************************************************************************
* Function��ѡ��˵�ĺ�����ѡ��һ���˵����ܶ����������ݲ���
* Input   : num -- �˵��
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_d12_select_end_point(uint8 num)
{
    ai_d12_write_cmd(0x00 + num);
}

/*******************************************************************************
* Function����ȡ�˵㻺��������
* Input   : num -- �˵��
*           len -- ��Ҫ��ȡ�ĳ��ȣ�
*           buf -- ��������ַ
* Ouput   : None
* Return  : uint8 -- ʵ�ʶ��������ݳ���
* Others  :
*******************************************************************************/
uint8 ai_d12_read_end_point_buf(uint8 num, uint8 len, uint8 *buf)
{
    uint8 i, j;

    ai_d12_select_end_point(num);
    ai_d12_write_cmd(AI_D12_READ_BUFFER);
    ai_d12_read_byte();      // ��һ���ֽ��Ǳ����ֽڣ�����
    j = ai_d12_read_byte();
    if (j > len)
        j = len;
    
    ai_uart_send_str("���˵�");
    ai_uart_print_lint(num / 2);    // �˵�š�����D12����Ķ˵���֯��ʽ��
                                    // �����0��1�ֱ��ʾ�˵�0����������룻
                                    // ��2��3�ֱ��ʾ�˵�1����������룻
                                    // 3��4�ֱ��ʾ�˵�2����������롣
                                    // ���Ҫ����2����ʾ��Ӧ�Ķ˵㡣
    ai_uart_send_str("������");
    ai_uart_print_lint(j);
    ai_uart_send_str("�ֽ�:\r\n");

    for (i = 0; i < j; i++) {
        ai_d12_clr_rd();
        *(buf + i) = ai_d12_get_data();
        ai_d12_set_rd();

    ai_uart_print_hex(*(buf + i));
    ai_uart_send_char(' ');
    if (((i + 1) % 16) == 0)
        ai_uart_send_str("\r\n");   // ÿ16�ֽڻ���

    }

    if ((j % 16) != 0)
        ai_uart_send_str("\r\n");

    return j;
}  

/*******************************************************************************
* Function��������ն˵㻺��������
* Input   : None
* Ouput   : None
* Return  : void
* Others  : ֻ��ʹ�øú�������˵㻺��󣬸ý��ն˵���ܽ����µ����ݰ�
*******************************************************************************/
void ai_d12_clear_buf(void)
{
    ai_d12_write_cmd(AI_D12_CLEAR_BUFFER);
}

/*******************************************************************************
* Function��Ӧ����������
* Input   : None
* Ouput   : None
* Return  : void
* Others  : �ڽ������̣���Ҫ�ȷ���ack_setup��Ȼ�����buffer
*******************************************************************************/
void ai_d12_ack_setup(void)
{
    ai_d12_select_end_point(1);            // ѡ��˵�0����
    ai_d12_write_cmd(AI_D12_ACK_SETUP);    // ����Ӧ�����õ��˵�0����
    ai_d12_select_end_point(0);            // ѡ��˵�0���
    ai_d12_write_cmd(AI_D12_ACK_SETUP);    // ����Ӧ�����õ��˵�0���
} 

/*******************************************************************************
* Function����ȡD12�����״̬�Ĵ����ĺ���
* Input   : num -- �˵��
* Ouput   : None
* Return  : uint8 �˵�������״̬
* Others  : �ò���������ö˵���жϱ�־λ
*******************************************************************************/
uint8 ai_d12_read_endp_last_stat(uint8 num)
{
    ai_d12_write_cmd(0x40 + num);
    return ai_d12_read_byte();
}
  
/*******************************************************************************
* Function��ʹ�ܷ��Ͷ˵㻺����������Ч�ĺ���
* Input   : void
* Ouput   : None
* Return  : void
* Others  : ֻ��ʹ�øú���ʹ�ܷ��Ͷ˵�������Ч֮�����ݲ��ܷ��ͳ�ȥ
*******************************************************************************/
void ai_d12_validate_buf(void)
{
    ai_d12_write_cmd(AI_D12_VALIDATE_BUFFER);
}
  
/*******************************************************************************
* Function��������д��˵㻺��������
* Input   : num -- �˵��
*           len -- ��Ҫ���͵ĳ��ȣ�
*           buf -- �������ݵĻ�������
* Ouput   : None
* Return  : uint8 -- len��ֵ
* Others  :
*******************************************************************************/
uint8 ai_d12_write_endp_buf(uint8 num, uint8 len, uint8 *buf)
{
    uint8 i;

    ai_d12_select_end_point(num);
    ai_d12_write_cmd(AI_D12_WRITE_BUFFER);
    ai_d12_write_byte(0x0);
    ai_d12_write_byte(len);

    ai_uart_send_str("д�˵�");
    ai_uart_print_lint(len);
    ai_uart_send_str("�ֽ�.\r\n");

    aid12_set_port_out();
    for (i = 0; i < len; i++) {
        ai_d12_clr_wr();
        ai_d12_set_data(*(buf + i));
        ai_d12_set_wr();

        ai_uart_print_hex(*(buf + i));
        ai_uart_send_char(' ');
        if ((i + 1) % 16 == 0)
            ai_uart_send_str("\r\n");
    }

    if ((len % 16) != 0)
        ai_uart_send_str("\r\n");

    ai_d12_set_port_in();
    ai_d12_validate_buf();

    return len;
} 

/*******************************************************************************
* Function�����õ�ַ����
* Input   : addr -- Ҫ���õĵ�ֵַ
* Ouput   : None
* Return  : void
* Others  :
*******************************************************************************/
void ai_d12_set_addr(uint8 addr)
{
    ai_d12_write_cmd(AI_D12_SET_ADDR_ENABLE);
    ai_d12_write_byte(0x80 | addr);              // дһ�ֽ����ݣ�ʹ�ܼ���ַ
}

/*******************************************************************************
* Function��ʹ�ܶ˵㺯��
* Input   : enable -- �Ƿ�ʹ�ܡ�0ֵΪ��ʹ�ܣ���0ֵΪʹ�ܡ�
* Ouput   : None
* Return  : void
* Others  :
*******************************************************************************/
void ai_d12_set_endp_enable(uint8 enable)
{
    ai_d12_write_cmd(AI_D12_SET_ENDPOINT_ENABLE);
    if (enable != 0)
        ai_d12_write_byte(0x01);    // D0Ϊ1ʹ�ܶ˵�
    else
        ai_d12_write_byte(0x00);
}
