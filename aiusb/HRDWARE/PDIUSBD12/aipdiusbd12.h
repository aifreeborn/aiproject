#ifndef AI_PDIUSBD12_H_
#define AI_PDIUSBD12_H_

#include <reg52.h>
#include <aitype.h>

/*******************************************************************************
* 管脚连接：
*     |---------------------------|
*     | PDIUSBD12   |   MCU       |
*     |---------------------------|
*     | D[0..7]     |   P0端口    |
*     | A0          |   T1/P3.5   |
*     | INT_N       |   INT0/P3.2 |
*     | WR_N        |   WR_N/P3.6 |
*     | RD_N        |   RD_N/P3.7 |
*     |---------------------------|
*******************************************************************************/
// PDIUSBD12芯片连接引脚
#define    AI_D12_DATA    P0
sbit       AI_D12_A0 = P3^5;
sbit       AI_D12_WR = P3^6;
sbit       AI_D12_RD = P3^7;
sbit       AI_D12_INT = P3^2;

// 命令地址和数据地址
#define    AI_D12_CMD_ADDR    1
#define    AI_D12_DATA_ADDR   0

// 定义PDIUSBD12的读写命令
// 读取ID命令，手册里没有公布
#define    AI_D12_READ_ID               0xFD
#define    AI_D12_SET_MODE              0xF3
#define    AI_D12_READ_INTERRUPT_REG    0xF4
// 读端点缓冲区命令
#define    AI_D12_READ_BUFFER           0xF0
#define    AI_D12_WRITE_BUFFER          0xF0
#define    AI_D12_CLEAR_BUFFER          0xF2
#define    AI_D12_VALIDATE_BUFFER       0xFA
// 应答设置包命令
#define    AI_D12_ACK_SETUP             0xF1
// Set Address/Enable
#define    AI_D12_SET_ADDR_ENABLE       0xD0
#define    AI_D12_SET_ENDPOINT_ENABLE   0xD8

// 选择命令或数据地址
#define    ai_d12_set_cmd_addr() \
    do { \
        AI_D12_A0 = AI_D12_CMD_ADDR; \
    } while (0)
#define    ai_d12_set_data_addr() \
    do { \
        AI_D12_A0 = AI_D12_DATA_ADDR; \
    } while (0)
// WR/RD控制
#define    ai_d12_set_wr() \
    do { \
        AI_D12_WR = 1; \
    } while (0)
#define    ai_d12_clr_wr() \
    do { \
        AI_D12_WR = 0; \
    } while (0)
#define    ai_d12_set_rd() \
    do { \
        AI_D12_RD = 1; \
    } while (0)
#define    ai_d12_clr_rd() \
    do { \
        AI_D12_RD = 0; \
    } while (0)
// 获取中断引脚状态
#define    ai_d12_get_int_pin()     AI_D12_INT
// 读写数据
#define    ai_d12_get_data()        AI_D12_DATA
#define    ai_d12_set_data(val) \
    do { \
        AI_D12_DATA = (val); \
    } while (0)
// 将数据口设置为输入状态，51单片机端口写1就是为输入状态
#define    ai_d12_set_port_in() \
    do { \
        AI_D12_DATA = 0xff; \
    } while (0)
// 将数据口设置为输出状态，由于51单片机是准双向IO口，所以不用切换，为空宏
#define    aid12_set_port_out()


void ai_d12_write_cmd(uint8 cmd);
uint8 ai_d12_read_byte(void);
uint16 ai_d12_read_id(void);
void ai_d12_write_byte(uint8 value);
void ai_d12_select_end_point(uint8 num);
uint8 ai_d12_read_end_point_buf(uint8 num, uint8 len, uint8 *buf);
void ai_d12_clear_buf(void);
void ai_d12_ack_setup(void);
uint8 ai_d12_read_endp_last_stat(uint8 num);
void ai_d12_validate_buf(void);
uint8 ai_d12_write_endp_buf(uint8 num, uint8 len, uint8 *buf);
void ai_d12_set_addr(uint8 addr);

#endif /* AI_PDIUSBD12_H_ */