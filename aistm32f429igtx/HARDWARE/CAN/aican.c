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
* Description: CAN1初始化
*       Input: sjw - reSynchronization Jump Width,再同步跳跃宽度，范围:[0, 3]
*                    tRJW = tCAN x (SJW[1:0] + 1)
*              bs1 - 时间段1中的时间片数目,范围:[0, 15]
*                    tBS1 = tCAN x (TS1[3:0] + 1)
*              bs2 - 时间段2中的时间片数目,范围:[0, 7]
*                    tBS2 = tCAN x (TS2[2:0] + 1)
*              brp - 波特率分频器,范围:[0, 1023] 
*                    tq = (BRP[9:0] + 1) x tPCLK
*                    tPCLK = APB 时钟的时间周期
*              mode - 0,普通模式;1,回环模式
*      Output: None
*      Return: On success, 0 is returned,
*              On error, negative integer is returned.
*      Others: BaudRate = 1 / NominalBitTime
*              NominalBitTime = 1 * tq + tBS1 + tBS2
*              Fpclk1的时钟在初始化的时候设置为45M
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
    
    // 使能CAN1时钟 CAN1使用的是APB1的时钟(max:48M)
    RCC->APB1ENR |= 0x1 << 25;
    // 退出睡眠模式(同时设置所有位为0)
    CAN1->MCR = 0x00000000;
    // 请求CAN进入初始化模式
    CAN1->MCR |= 0x1;
    while ((CAN1->MCR & 0x1) == 0) {
        i++;
        if (i > 100)
            return -2;           // 进入初始化模式失败
    }
    
    CAN1->MCR &= ~(0x1 << 7);    // 禁止时间触发通信模式
    // 一旦监测到128次连续11个隐性位，即通过硬件自动退出总线关闭状态
    CAN1->MCR &= ~(0x1 << 6);
    // 在软件通过将 CAN_MCR 寄存器的 SLEEP 位清零发出请求后，退出睡眠模式
    CAN1->MCR &= ~(0x1 << 5);
    CAN1->MCR |= 0x1 << 4;       // 禁止报文自动传送
    CAN1->MCR &= ~(0x1 << 3);    // 报文不锁定,新的覆盖旧的
    CAN1->MCR &= ~(0x1 << 2);    // 优先级由报文标识符决定
    
    CAN1->BTR = 0x00000000;
    CAN1->BTR |= (mode & 0x1) << 30;
    CAN1->BTR |= (sjw & 0x3) << 24;
    CAN1->BTR |= (ts2 & 0x7) << 20;
    CAN1->BTR |= (ts1 & 0xf) << 16;
    CAN1->BTR |= brp & 0x3ff;
    
    CAN1->MCR &= ~0x1;           // 请求CAN退出初始化模式
    while ((CAN1->MCR & 0x1) == 0x1) {
        i++;
        if (i > 0xfff0)
            return -3;
    }
    
    // 过滤器初始化
    CAN1->FMR |= 0x1;
    CAN1->FA1R &= ~0x1;
    CAN1->FS1R |= 0x1;          // 过滤器位宽为32位
    CAN1->FM1R &= ~0x1;         // 过滤器0工作在标识符屏蔽位模式
    CAN1->FFA1R &= ~0x1;        // 过滤器0关联到FIFO0
    CAN1->sFilterRegister[0].FR1 = 0x00000000;    // 32位ID
    CAN1->sFilterRegister[0].FR2 = 0x00000000;    // 32位MASK
    CAN1->FA1R |= 0x1;
    CAN1->FMR &= ~0x1;
    
#if AI_CAN1_RX0_INT_EN
    CAN1->IER |= 0x1 << 1;     // FIFO0消息挂号中断允许
    ai_nvic_init(1, 0, CAN1_RX0_IRQn, 2);
#endif
    return 0;
}

/*
********************************************************************************
*    Function: ai_can1_tx_msg
* Description: 发送CAN报文:先查找空的发送邮箱，然后设置标识符 ID 等信息，最后
*              写入数据长度和数据，并请求发送，实现一次报文的发送
*       Input: fifox - 两个接收 FIFO, 0/1        
*      Output:    id - 标准ID(11位)/扩展ID(11位+18位)
*                ide - 0,标准帧;1,扩展帧
*                rtr - 0,数据帧;1,远程帧
*                len - 接收到的数据长度
*               data - 数据缓存区
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_can1_rx_msg(u8 fifox, u32 *id, u8 *ide, u8 *rtr, u8 *len, u8 *data)
{
    // 得到标识符选择位的值
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
    // 果 FIFO 中至少有两条消息挂起，软件必须释放输出邮箱，才能访问下一条消息
    if (fifox == 0)
        CAN1->RF0R |= 0x1 << 5;    // 释放 FIFO 0 输出邮箱
    else
        CAN1->RF1R |= 0x1 << 5;    // 释放 FIFO 1 输出邮箱
}

/*
********************************************************************************
*    Function: ai_can1_tx_msg
* Description: 发送CAN报文:先查找空的发送邮箱，然后设置标识符 ID 等信息，最后
*              写入数据长度和数据，并请求发送，实现一次报文的发送
*       Input:   id - 标准ID(11位)/扩展ID(11位+18位)
*               ide - 0,标准帧;1,扩展帧
*               rtr - 0,数据帧;1,远程帧
*               len - 要发送的数据长度(固定为8个字节,在时间触发模式下,
*                     有效数据为6个字节)
*              data - 数据指针
*      Output: None
*      Return: 0~3为邮箱的编号；-1为无效邮箱
*      Others: None
********************************************************************************
*/
int ai_can1_tx_msg(u32 id, u8 ide, u8 rtr, u8 len, u8 *data)
{
    int mbox = 0;
    
    if (CAN1->TSR & (0x1 << 26))         // 邮箱0为空
        mbox = 0;
    else if (CAN1->TSR & (0x1 << 27))    // 邮箱1为空
        mbox = 1;
    else if (CAN1->TSR & (0x1 << 28))    // 邮箱2为空
        mbox = 2;
    else                                 // 无空邮箱,无法发送
        return -1;
    
    CAN1->sTxMailBox[mbox].TIR = 0;
    if (ide == 0) {                      // 标准帧
        id &= 0x7ff;                     // 取低11位stdid
        id <<= 21;
    } else {
        id &= ~((u32)0x7 << 29);              // 取低29位
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
    CAN1->sTxMailBox[mbox].TIR |= 0x1;   // 请求发送邮箱数据
    
    return mbox;
}

/*
********************************************************************************
*    Function: ai_can1_get_tx_stat
* Description: 获得发送状态,各位的含义如下：
*              +--------+--------+--------+
*              |  bit2  |  bit1  |  bit0  |
*              +--------+--------+--------+
*              |  TME   |  TXOK  |  RQCP  |
*              +--------+--------+--------+ 
*              其中： TME -> 发送邮箱x空
*                    TXOK -> 邮箱x发送成功
+                    RQCP -> 邮箱x请求完成
*       Input: mbox - 邮箱编号，0~3
*      Output: None
*      Return: 发送状态
*              0 -> 挂起; 0x05 -> 发送失败; 0x07 -> 发送成功.
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
* Description: 得到在FIFO0/FIFO1中接收到的报文个数
*       Input: fifo_num - FIFO编号,0/1
*      Output: None
*      Return: FIFO0/FIFO1中的报文个数
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
* Description: can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)
*       Input: msg - 数据指针,最大为8个字节
*              len - 数据长度(最大为8)
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
    // 等待发送结束
    while ((ai_can1_get_tx_stat(mbox) != 0x07) && (i < 0xfff))
        i++;
    
    if (i >= 0xfff)
        return -1;
    return 0;
}

/*
********************************************************************************
*    Function: ai_can1_receive_msg
* Description: can口接收数据查询
*       Input: buf - 接收数据的缓冲区首地址
*      Output: buf - 指向接收数据
*      Return: 0,无数据被收到;其他,接收的数据长度;
*      Others: None
********************************************************************************
*/
u8 ai_can1_receive_msg(u8 *buf)
{
    u32 id;
    u8 ide, rtr, len;
    
    if (ai_can1_msg_pend(0) == 0)    // 没有接收到数据,直接退出
        return 0;
    ai_can1_rx_msg(0, &id, &ide, &rtr, &len, buf);
    if (id != 0x12 || ide != 0 || rtr != 0)
        len = 0;
    return len;
}
