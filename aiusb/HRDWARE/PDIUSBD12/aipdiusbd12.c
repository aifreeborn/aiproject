#include "aipdiusbd12.h"
#include <aiuart.h>


/*******************************************************************************
* Function：PDIUSBD12写命令
* Input   : cmd -- 一字节命令
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
* Function：读一字节PDIUSBD12数据
* Input   : void
* Ouput   : None
* Return  : uint8 -- 读回的一字节
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
* Function：读PDIUSBD12的ID
* Input   : void
* Ouput   : None
* Return  : uint16 -- 16位的ID值
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
* Function：写一字节PDIUSBD12数据
* Input   : value -- 要写的1字节数据
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
* Function：选择端点的函数，选择一个端点后才能对它进行数据操作
* Input   : num -- 端点号
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_d12_select_end_point(uint8 num)
{
    ai_d12_write_cmd(0x00 + num);
}

/*******************************************************************************
* Function：读取端点缓冲区函数
* Input   : num -- 端点号
*           len -- 需要读取的长度；
*           buf -- 缓冲区地址
* Ouput   : None
* Return  : uint8 -- 实际读到的数据长度
* Others  :
*******************************************************************************/
uint8 ai_d12_read_end_point_buf(uint8 num, uint8 len, uint8 *buf)
{
    uint8 i, j;

    ai_d12_select_end_point(num);
    ai_d12_write_cmd(AI_D12_READ_BUFFER);
    ai_d12_read_byte();      // 第一个字节是保留字节，不用
    j = ai_d12_read_byte();
    if (j > len)
        j = len;
    
    ai_uart_send_str("读端点");
    ai_uart_print_lint(num / 2);    // 端点号。由于D12特殊的端点组织形式，
                                    // 这里的0和1分别表示端点0的输出和输入；
                                    // 而2、3分别表示端点1的输出和输入；
                                    // 3、4分别表示端点2的输出和输入。
                                    // 因此要除以2才显示对应的端点。
    ai_uart_send_str("缓冲区");
    ai_uart_print_lint(j);
    ai_uart_send_str("字节:\r\n");

    for (i = 0; i < j; i++) {
        ai_d12_clr_rd();
        *(buf + i) = ai_d12_get_data();
        ai_d12_set_rd();

    ai_uart_print_hex(*(buf + i));
    ai_uart_send_char(' ');
    if (((i + 1) % 16) == 0)
        ai_uart_send_str("\r\n");   // 每16字节换行

    }

    if ((j % 16) != 0)
        ai_uart_send_str("\r\n");

    return j;
}  

/*******************************************************************************
* Function：清除接收端点缓冲区函数
* Input   : None
* Ouput   : None
* Return  : void
* Others  : 只有使用该函数清除端点缓冲后，该接收端点才能接收新的数据包
*******************************************************************************/
void ai_d12_clear_buf(void)
{
    ai_d12_write_cmd(AI_D12_CLEAR_BUFFER);
}

/*******************************************************************************
* Function：应答建立包函数
* Input   : None
* Ouput   : None
* Return  : void
* Others  : 在建立过程，需要先发送ack_setup，然后清除buffer
*******************************************************************************/
void ai_d12_ack_setup(void)
{
    ai_d12_select_end_point(1);            // 选择端点0输入
    ai_d12_write_cmd(AI_D12_ACK_SETUP);    // 发送应答设置到端点0输入
    ai_d12_select_end_point(0);            // 选择端点0输出
    ai_d12_write_cmd(AI_D12_ACK_SETUP);    // 发送应答设置到端点0输出
} 

/*******************************************************************************
* Function：读取D12最后传输状态寄存器的函数
* Input   : num -- 端点号
* Ouput   : None
* Return  : uint8 端点的最后传输状态
* Others  : 该操作将清除该端点的中断标志位
*******************************************************************************/
uint8 ai_d12_read_endp_last_stat(uint8 num)
{
    ai_d12_write_cmd(0x40 + num);
    return ai_d12_read_byte();
}
  
/*******************************************************************************
* Function：使能发送端点缓冲区数据有效的函数
* Input   : void
* Ouput   : None
* Return  : void
* Others  : 只有使用该函数使能发送端点数据有效之后，数据才能发送出去
*******************************************************************************/
void ai_d12_validate_buf(void)
{
    ai_d12_write_cmd(AI_D12_VALIDATE_BUFFER);
}
  
/*******************************************************************************
* Function：将数据写入端点缓冲区函数
* Input   : num -- 端点号
*           len -- 需要发送的长度；
*           buf -- 保存数据的缓冲区；
* Ouput   : None
* Return  : uint8 -- len的值
* Others  :
*******************************************************************************/
uint8 ai_d12_write_endp_buf(uint8 num, uint8 len, uint8 *buf)
{
    uint8 i;

    ai_d12_select_end_point(num);
    ai_d12_write_cmd(AI_D12_WRITE_BUFFER);
    ai_d12_write_byte(0x0);
    ai_d12_write_byte(len);

    ai_uart_send_str("写端点");
    ai_uart_print_lint(len);
    ai_uart_send_str("字节.\r\n");

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
* Function：设置地址函数
* Input   : addr -- 要设置的地址值
* Ouput   : None
* Return  : void
* Others  :
*******************************************************************************/
void ai_d12_set_addr(uint8 addr)
{
    ai_d12_write_cmd(AI_D12_SET_ADDR_ENABLE);
    ai_d12_write_byte(0x80 | addr);              // 写一字节数据：使能及地址
}

/*******************************************************************************
* Function：使能端点函数
* Input   : enable -- 是否使能。0值为不使能，非0值为使能。
* Ouput   : None
* Return  : void
* Others  :
*******************************************************************************/
void ai_d12_set_endp_enable(uint8 enable)
{
    ai_d12_write_cmd(AI_D12_SET_ENDPOINT_ENABLE);
    if (enable != 0)
        ai_d12_write_byte(0x01);    // D0为1使能端点
    else
        ai_d12_write_byte(0x00);
}
