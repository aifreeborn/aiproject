#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aidma.h"

/*
********************************************************************************
*    Function: ai_dma_config
* Description: DMAx的各通道配置,
*              从存储器 -> 外设模式/8位数据宽度/存储器增量模式
*       Input: streamx - DMA数据流,取值：DMA1_Stream0~7 / DMA2_Stream0~7
*              channel - DMA通道选择，取值范围[0, 7]
*                paddr - 外设地址
*                maddr - 存储器地址
*                 ndtr - Number of data items to transfer
*      Output: None
*      Return: void
*      Others: 这里使用从存储器 -> 外设模式，如果需要其它模式要根据情况更改
********************************************************************************
*/
void ai_dma_config(DMA_Stream_TypeDef *streamx, u8 channel,
                   u32 paddr, u32 maddr, u16 ndtr)
{
    DMA_TypeDef *dma;
    u8 stream_num;
    
    // 得到当前stream是属于DMA2还是DMA1,并使能相应时钟
    if ((u32)streamx > (u32)DMA2) {
        dma = DMA2;
        RCC->AHB1ENR |= 0x1 << 22;
    } else {
        dma = DMA1;
        RCC->AHB1ENR |= 0x1 << 21;
    }
    while (streamx->CR & 0x01)
        ; /* Empty */
    
    // 获取通道号并清除stream上的中断标志
    stream_num = (((u32)streamx - (u32)dma) - 0x10) / 0x18;
    if (stream_num >= 6)
        dma->HIFCR |= 0x3d << (6 * (stream_num - 6) + 16);
    else if (stream_num >= 4)
        dma->HIFCR |= 0x3d << (6 * (stream_num - 4));
    else if (stream_num >= 2)
        dma->LIFCR |= 0x3d << (6 * (stream_num - 2) + 16);
    else
        dma->LIFCR |= 0x3d << 6 * stream_num;
    
    streamx->PAR = paddr;
    streamx->M0AR = maddr;
    streamx->NDTR = ndtr;
    
    streamx->CR = 0x0;
    streamx->CR |= ((u32)channel & 0x7) << 25;    // 通道选择
    streamx->CR |= 0x0 << 23;                     // 存储器突发单次传输
    streamx->CR |= 0x0 << 21;                     // 外设突发单次传输
    streamx->CR |= 0x1 << 16;                     // 中等优先级
    streamx->CR |= 0x0 << 13;                     // 存储器数据长度:8位
    streamx->CR |= 0x0 << 11;                     // 外设数据长度:8位
    streamx->CR |= 0x1 << 10;                     // 存储器增量模式
    streamx->CR |= 0x0 << 9;
    streamx->CR |= 0x0 << 8;                      // 非循环模式(即使用普通模式)
    streamx->CR |= 0x1 << 6;                      // 存储器到外设
}

/*
********************************************************************************
*    Function: ai_dma_enable
* Description: 开启一次DMA传输
*       Input: streamx - DMA数据流,取值：DMA1_Stream0~7 / DMA2_Stream0~7
*                 ndtr - Number of data items to transfer
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_dma_enable(DMA_Stream_TypeDef *streamx, u16 ndtr)
{
    streamx->CR &= ~0x1;
    while (streamx->CR & 0x1)    // 确保DMA可以被设置 
        ;
    streamx->NDTR = ndtr;        // DMA 存储器0地址
    streamx->CR |= 0x1;
}
