#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aidma.h"

/*
********************************************************************************
*    Function: ai_dma_config
* Description: DMAx�ĸ�ͨ������,
*              �Ӵ洢�� -> ����ģʽ/8λ���ݿ��/�洢������ģʽ
*       Input: streamx - DMA������,ȡֵ��DMA1_Stream0~7 / DMA2_Stream0~7
*              channel - DMAͨ��ѡ��ȡֵ��Χ[0, 7]
*                paddr - �����ַ
*                maddr - �洢����ַ
*                 ndtr - Number of data items to transfer
*      Output: None
*      Return: void
*      Others: ����ʹ�ôӴ洢�� -> ����ģʽ�������Ҫ����ģʽҪ�����������
********************************************************************************
*/
void ai_dma_config(DMA_Stream_TypeDef *streamx, u8 channel,
                   u32 paddr, u32 maddr, u16 ndtr)
{
    DMA_TypeDef *dma;
    u8 stream_num;
    
    // �õ���ǰstream������DMA2����DMA1,��ʹ����Ӧʱ��
    if ((u32)streamx > (u32)DMA2) {
        dma = DMA2;
        RCC->AHB1ENR |= 0x1 << 22;
    } else {
        dma = DMA1;
        RCC->AHB1ENR |= 0x1 << 21;
    }
    while (streamx->CR & 0x01)
        ; /* Empty */
    
    // ��ȡͨ���Ų����stream�ϵ��жϱ�־
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
    streamx->CR |= ((u32)channel & 0x7) << 25;    // ͨ��ѡ��
    streamx->CR |= 0x0 << 23;                     // �洢��ͻ�����δ���
    streamx->CR |= 0x0 << 21;                     // ����ͻ�����δ���
    streamx->CR |= 0x1 << 16;                     // �е����ȼ�
    streamx->CR |= 0x0 << 13;                     // �洢�����ݳ���:8λ
    streamx->CR |= 0x0 << 11;                     // �������ݳ���:8λ
    streamx->CR |= 0x1 << 10;                     // �洢������ģʽ
    streamx->CR |= 0x0 << 9;
    streamx->CR |= 0x0 << 8;                      // ��ѭ��ģʽ(��ʹ����ͨģʽ)
    streamx->CR |= 0x1 << 6;                      // �洢��������
}

/*
********************************************************************************
*    Function: ai_dma_enable
* Description: ����һ��DMA����
*       Input: streamx - DMA������,ȡֵ��DMA1_Stream0~7 / DMA2_Stream0~7
*                 ndtr - Number of data items to transfer
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_dma_enable(DMA_Stream_TypeDef *streamx, u16 ndtr)
{
    streamx->CR &= ~0x1;
    while (streamx->CR & 0x1)    // ȷ��DMA���Ա����� 
        ;
    streamx->NDTR = ndtr;        // DMA �洢��0��ַ
    streamx->CR |= 0x1;
}
