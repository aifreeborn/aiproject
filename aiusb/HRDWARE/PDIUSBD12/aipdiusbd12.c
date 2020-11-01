#include "aipdiusbd12.h"


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