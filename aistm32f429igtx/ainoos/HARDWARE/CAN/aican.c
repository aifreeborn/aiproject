#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aisys.h"
#include "aican.h"

#if AI_CAN1_RX0_INT_EN
void CAN1_RX0_IRQHandler(void)
{
    u8 rx_buf[8];
    u32 id;
    u8 ide, rtr, len;
    
    ai_can1_rx_msg(0, &id, &ide, &rtr, &len, rx_buf);
    printf("id: %d\r\n", id);
    printf("ide: %d\r\n", ide);
    printf("rtr: %d\r\n", rtr);
    printf("len: %d\r\n", len);
    printf("rx_buf[0]: %d\r\n", rx_buf[0]);
    printf("rx_buf[1]: %d\r\n", rx_buf[1]);
    printf("rx_buf[2]: %d\r\n", rx_buf[2]);
    printf("rx_buf[3]: %d\r\n", rx_buf[3]);
    printf("rx_buf[4]: %d\r\n", rx_buf[4]);
    printf("rx_buf[5]: %d\r\n", rx_buf[5]);
    printf("rx_buf[6]: %d\r\n", rx_buf[6]);
    printf("rx_buf[7]: %d\r\n", rx_buf[7]);
}
#endif

/*
********************************************************************************
*    Function: ai_can1_mode_init
* Description: CAN1��ʼ��
*       Input: sjw - reSynchronization Jump Width,��ͬ����Ծ��ȣ���Χ:[0, 3]
*                    tRJW = tCAN x (SJW[1:0] + 1)
*              bs1 - ʱ���1�е�ʱ��Ƭ��Ŀ,��Χ:[0, 15]
*                    tBS1 = tCAN x (TS1[3:0] + 1)
*              bs2 - ʱ���2�е�ʱ��Ƭ��Ŀ,��Χ:[0, 7]
*                    tBS2 = tCAN x (TS2[2:0] + 1)
*              brp - �����ʷ�Ƶ��,��Χ:[0, 1023] 
*                    tq = (BRP[9:0] + 1) x tPCLK
*                    tPCLK = APB ʱ�ӵ�ʱ������
*              mode - 0,��ͨģʽ;1,�ػ�ģʽ
*      Output: None
*      Return: On success, 0 is returned,
*              On error, negative integer is returned.
*      Others: BaudRate = 1 / NominalBitTime
*              NominalBitTime = 1 * tq + tBS1 + tBS2
*              Fpclk1��ʱ���ڳ�ʼ����ʱ������Ϊ45M
********************************************************************************
*/
int ai_can1_mode_init(u8 sjw, u8 ts1, u8 ts2, u16 brp, u8 mode)
{
    u16 i = 0;
    
    if (sjw > 3 || ts1 > 15 || ts2 > 7 || brp > 1023 || mode > 1)
        return -1;
    
    RCC->APB1ENR |= 0x1;
    ai_gpio_set(GPIOA, PIN11 | PIN12, GPIO_MODE_AF,
                GPIO_OTYPE_PP, GPIO_SPEED_50M, GPIO_PUPD_PU);
    ai_gpio_set_af(GPIOA, 11, 9);
    ai_gpio_set_af(GPIOA, 12, 9);
    
    // ʹ��CAN1ʱ�� CAN1ʹ�õ���APB1��ʱ��(max:48M)
    RCC->APB1ENR |= 0x1 << 25;
    // �˳�˯��ģʽ(ͬʱ��������λΪ0)
    CAN1->MCR = 0x00000000;
    // ����CAN�����ʼ��ģʽ
    CAN1->MCR |= 0x1;
    while ((CAN1->MCR & 0x1) == 0) {
        i++;
        if (i > 100)
            return -2;           // �����ʼ��ģʽʧ��
    }
    
    CAN1->MCR &= ~(0x1 << 7);    // ��ֹʱ�䴥��ͨ��ģʽ
    // һ����⵽128������11������λ����ͨ��Ӳ���Զ��˳����߹ر�״̬
    CAN1->MCR &= ~(0x1 << 6);
    // �����ͨ���� CAN_MCR �Ĵ����� SLEEP λ���㷢��������˳�˯��ģʽ
    CAN1->MCR &= ~(0x1 << 5);
    CAN1->MCR |= 0x1 << 4;       // ��ֹ�����Զ�����
    CAN1->MCR &= ~(0x1 << 3);    // ���Ĳ�����,�µĸ��Ǿɵ�
    CAN1->MCR &= ~(0x1 << 2);    // ���ȼ��ɱ��ı�ʶ������
    
    CAN1->BTR = 0x00000000;
    CAN1->BTR |= (mode & 0x1) << 30;
    CAN1->BTR |= (sjw & 0x3) << 24;
    CAN1->BTR |= (ts2 & 0x7) << 20;
    CAN1->BTR |= (ts1 & 0xf) << 16;
    CAN1->BTR |= brp & 0x3ff;
    
    CAN1->MCR &= ~0x1;           // ����CAN�˳���ʼ��ģʽ
    while ((CAN1->MCR & 0x1) == 0x1) {
        i++;
        if (i > 0xfff0)
            return -3;
    }
    
    // ��������ʼ��
    CAN1->FMR |= 0x1;
    CAN1->FA1R &= ~0x1;
    CAN1->FS1R |= 0x1;          // ������λ��Ϊ32λ
    CAN1->FM1R &= ~0x1;         // ������0�����ڱ�ʶ������λģʽ
    CAN1->FFA1R &= ~0x1;        // ������0������FIFO0
    CAN1->sFilterRegister[0].FR1 = 0x00000000;    // 32λID
    CAN1->sFilterRegister[0].FR2 = 0x00000000;    // 32λMASK
    CAN1->FA1R |= 0x1;
    CAN1->FMR &= ~0x1;
    
#if AI_CAN1_RX0_INT_EN
    CAN1->IER |= 0x1 << 1;     // FIFO0��Ϣ�Һ��ж�����
    ai_nvic_init(1, 0, CAN1_RX0_IRQn, 2);
#endif
    return 0;
}

/*
********************************************************************************
*    Function: ai_can1_tx_msg
* Description: ����CAN����:�Ȳ��ҿյķ������䣬Ȼ�����ñ�ʶ�� ID ����Ϣ�����
*              д�����ݳ��Ⱥ����ݣ��������ͣ�ʵ��һ�α��ĵķ���
*       Input: fifox - �������� FIFO, 0/1        
*      Output:    id - ��׼ID(11λ)/��չID(11λ+18λ)
*                ide - 0,��׼֡;1,��չ֡
*                rtr - 0,����֡;1,Զ��֡
*                len - ���յ������ݳ���
*               data - ���ݻ�����
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_can1_rx_msg(u8 fifox, u32 *id, u8 *ide, u8 *rtr, u8 *len, u8 *data)
{
    // �õ���ʶ��ѡ��λ��ֵ
    *ide = CAN1->sFIFOMailBox[fifox].RIR & (0x1 << 2);
    if (*ide == 0)
        *id = (CAN1->sFIFOMailBox[fifox].RIR & ((u32)0x7ff << 21)) >> 21;
    else
        *id = CAN1->sFIFOMailBox[fifox].RIR >> 3;
    
    *rtr = (CAN1->sFIFOMailBox[fifox].RIR & (0x1 << 1)) >> 1;
    *len = CAN1->sFIFOMailBox[fifox].RDTR & 0x0f;
    data[0] = CAN1->sFIFOMailBox[fifox].RDLR & 0xff;
    data[1] = (CAN1->sFIFOMailBox[fifox].RDLR >> 8) & 0xff;
    data[2] = (CAN1->sFIFOMailBox[fifox].RDLR >> 16) & 0xff;
    data[3] = (CAN1->sFIFOMailBox[fifox].RDLR >> 24) & 0xff;
    data[4] = CAN1->sFIFOMailBox[fifox].RDHR & 0xff;
    data[5] = (CAN1->sFIFOMailBox[fifox].RDHR >> 8) & 0xff;
    data[6] = (CAN1->sFIFOMailBox[fifox].RDHR >> 16) & 0xff;
    data[7] = (CAN1->sFIFOMailBox[fifox].RDHR >> 24) & 0xff;
    // �� FIFO ��������������Ϣ������������ͷ�������䣬���ܷ�����һ����Ϣ
    if (fifox == 0)
        CAN1->RF0R |= 0x1 << 5;    // �ͷ� FIFO 0 �������
    else
        CAN1->RF1R |= 0x1 << 5;    // �ͷ� FIFO 1 �������
}

/*
********************************************************************************
*    Function: ai_can1_tx_msg
* Description: ����CAN����:�Ȳ��ҿյķ������䣬Ȼ�����ñ�ʶ�� ID ����Ϣ�����
*              д�����ݳ��Ⱥ����ݣ��������ͣ�ʵ��һ�α��ĵķ���
*       Input:   id - ��׼ID(11λ)/��չID(11λ+18λ)
*               ide - 0,��׼֡;1,��չ֡
*               rtr - 0,����֡;1,Զ��֡
*               len - Ҫ���͵����ݳ���(�̶�Ϊ8���ֽ�,��ʱ�䴥��ģʽ��,
*                     ��Ч����Ϊ6���ֽ�)
*              data - ����ָ��
*      Output: None
*      Return: 0~3Ϊ����ı�ţ�-1Ϊ��Ч����
*      Others: None
********************************************************************************
*/
int ai_can1_tx_msg(u32 id, u8 ide, u8 rtr, u8 len, u8 *data)
{
    int mbox = 0;
    
    if (CAN1->TSR & (0x1 << 26))         // ����0Ϊ��
        mbox = 0;
    else if (CAN1->TSR & (0x1 << 27))    // ����1Ϊ��
        mbox = 1;
    else if (CAN1->TSR & (0x1 << 28))    // ����2Ϊ��
        mbox = 2;
    else                                 // �޿�����,�޷�����
        return -1;
    
    CAN1->sTxMailBox[mbox].TIR = 0;
    if (ide == 0) {                      // ��׼֡
        id &= 0x7ff;                     // ȡ��11λstdid
        id <<= 21;
    } else {
        id &= ~((u32)0x7 << 29);              // ȡ��29λ
        id <<= 3;
    }
    CAN1->sTxMailBox[mbox].TIR |= id;
    CAN1->sTxMailBox[mbox].TIR |= (ide & 0x1) << 2;
    CAN1->sTxMailBox[mbox].TIR |= (rtr & 0x1) << 1;
    CAN1->sTxMailBox[mbox].TDTR &= ~((u32)0xf);
    CAN1->sTxMailBox[mbox].TDTR |= len & 0xf;
    CAN1->sTxMailBox[mbox].TDLR = ((u32)data[3] << 24) | ((u32)data[2] << 16)
                                  | ((u32)data[1] << 8) | (u32)data[0];
    CAN1->sTxMailBox[mbox].TDHR = ((u32)data[7] << 24) | ((u32)data[6] << 16)
                                  | ((u32)data[5] << 8) | (u32)data[4];
    CAN1->sTxMailBox[mbox].TIR |= 0x1;   // ��������������
    
    return mbox;
}

/*
********************************************************************************
*    Function: ai_can1_get_tx_stat
* Description: ��÷���״̬,��λ�ĺ������£�
*              +--------+--------+--------+
*              |  bit2  |  bit1  |  bit0  |
*              +--------+--------+--------+
*              |  TME   |  TXOK  |  RQCP  |
*              +--------+--------+--------+ 
*              ���У� TME -> ��������x��
*                    TXOK -> ����x���ͳɹ�
+                    RQCP -> ����x�������
*       Input: mbox - �����ţ�0~3
*      Output: None
*      Return: ����״̬
*              0 -> ����; 0x05 -> ����ʧ��; 0x07 -> ���ͳɹ�.
*      Others: None
********************************************************************************
*/
u8 ai_can1_get_tx_stat(u8 mbox)
{
    u8 stat = 0;
    
    switch (mbox) {
    case 0:
        stat |= CAN1->TSR & 0x1;
        stat |= CAN1->TSR & (0x1 << 1);
        stat |= (CAN1->TSR & (0x1 << 26)) >> 24;
        break;
    case 1:
        stat |= (CAN1->TSR & (0x1 << 8)) >> 8;
        stat |= (CAN1->TSR & (0x1 << 9)) >> 8;
        stat |= (CAN1->TSR & (0x1 << 27)) >> 25;
        break;
    case 2:
        stat |= (CAN1->TSR & (0x1 << 16)) >> 16;
        stat |= (CAN1->TSR & (0x1 << 17)) >> 16;
        stat |= (CAN1->TSR & (0x1 << 28)) >> 26;
        break;
    default:
        stat = 0x05;
    }
    return stat;
}

/*
********************************************************************************
*    Function: ai_can1_msg_pend
* Description: �õ���FIFO0/FIFO1�н��յ��ı��ĸ���
*       Input: fifo_num - FIFO���,0/1
*      Output: None
*      Return: FIFO0/FIFO1�еı��ĸ���
*      Others: None
********************************************************************************
*/
u8 ai_can1_msg_pend(u8 fifo_num)
{
    if (fifo_num == 0)
        return CAN1->RF0R & 0x03;
    else if (fifo_num == 1)
        return CAN1->RF1R & 0x03;
    else
        return 0;
}

/*
********************************************************************************
*    Function: ai_can1_send_msg
* Description: can����һ������(�̶���ʽ:IDΪ0X12,��׼֡,����֡)
*       Input: msg - ����ָ��,���Ϊ8���ֽ�
*              len - ���ݳ���(���Ϊ8)
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_can1_send_msg(u8 *msg, u8 len)
{
    u8 mbox;
    u16 i;
    
    mbox = ai_can1_tx_msg(0x12, 0, 0, len, msg);
    // �ȴ����ͽ���
    while ((ai_can1_get_tx_stat(mbox) != 0x07) && (i < 0xfff))
        i++;
    
    if (i >= 0xfff)
        return -1;
    return 0;
}

/*
********************************************************************************
*    Function: ai_can1_receive_msg
* Description: can�ڽ������ݲ�ѯ
*       Input: buf - �������ݵĻ������׵�ַ
*      Output: buf - ָ���������
*      Return: 0,�����ݱ��յ�;����,���յ����ݳ���;
*      Others: None
********************************************************************************
*/
u8 ai_can1_receive_msg(u8 *buf)
{
    u32 id;
    u8 ide, rtr, len;
    
    if (ai_can1_msg_pend(0) == 0)    // û�н��յ�����,ֱ���˳�
        return 0;
    ai_can1_rx_msg(0, &id, &ide, &rtr, &len, buf);
    if (id != 0x12 || ide != 0 || rtr != 0)
        len = 0;
    return len;
}
