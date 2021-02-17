#ifndef AI_PDIUSBD12_H_
#define AI_PDIUSBD12_H_

#include <reg52.h>
#include <aitype.h>

/*******************************************************************************
* �ܽ����ӣ�
*     |---------------------------|
*     | PDIUSBD12   |   MCU       |
*     |---------------------------|
*     | D[0..7]     |   P0�˿�    |
*     | A0          |   T1/P3.5   |
*     | INT_N       |   INT0/P3.2 |
*     | WR_N        |   WR_N/P3.6 |
*     | RD_N        |   RD_N/P3.7 |
*     |---------------------------|
*******************************************************************************/
// PDIUSBD12оƬ��������
#define    AI_D12_DATA    P0
sbit       AI_D12_A0 = P3^5;
sbit       AI_D12_WR = P3^6;
sbit       AI_D12_RD = P3^7;
sbit       AI_D12_INT = P3^2;

// �����ַ�����ݵ�ַ
#define    AI_D12_CMD_ADDR    1
#define    AI_D12_DATA_ADDR   0

// ����PDIUSBD12�Ķ�д����
// ��ȡID����ֲ���û�й���
#define    AI_D12_READ_ID               0xFD
#define    AI_D12_SET_MODE              0xF3
#define    AI_D12_READ_INTERRUPT_REG    0xF4
// ���˵㻺��������
#define    AI_D12_READ_BUFFER           0xF0
#define    AI_D12_WRITE_BUFFER          0xF0
#define    AI_D12_CLEAR_BUFFER          0xF2
#define    AI_D12_VALIDATE_BUFFER       0xFA
// Ӧ�����ð�����
#define    AI_D12_ACK_SETUP             0xF1
// Set Address/Enable
#define    AI_D12_SET_ADDR_ENABLE       0xD0
#define    AI_D12_SET_ENDPOINT_ENABLE   0xD8

// ѡ����������ݵ�ַ
#define    ai_d12_set_cmd_addr() \
    do { \
        AI_D12_A0 = AI_D12_CMD_ADDR; \
    } while (0)
#define    ai_d12_set_data_addr() \
    do { \
        AI_D12_A0 = AI_D12_DATA_ADDR; \
    } while (0)
// WR/RD����
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
// ��ȡ�ж�����״̬
#define    ai_d12_get_int_pin()     AI_D12_INT
// ��д����
#define    ai_d12_get_data()        AI_D12_DATA
#define    ai_d12_set_data(val) \
    do { \
        AI_D12_DATA = (val); \
    } while (0)
// �����ݿ�����Ϊ����״̬��51��Ƭ���˿�д1����Ϊ����״̬
#define    ai_d12_set_port_in() \
    do { \
        AI_D12_DATA = 0xff; \
    } while (0)
// �����ݿ�����Ϊ���״̬������51��Ƭ����׼˫��IO�ڣ����Բ����л���Ϊ�պ�
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