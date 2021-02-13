#include <stdio.h>
#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aimem.h"

/*
********************************************************************************
*                           PRIVATE VARIABLE
********************************************************************************
*/
// �ڴ��(32�ֽڶ���),�ڲ�SRAM�ڴ��
__align(32) u8 ai_mem1_base[AI_MEM1_MAX_SIZE];
// �ⲿSDRAM�ڴ��,ǰ��2M��LTDC����(1280*800*2)
__align(32) u8 ai_mem2_base[AI_MEM2_MAX_SIZE] __attribute((at(0xC01F4000)));
// �ڲ�CCM�ڴ��
__align(32) u8 ai_mem3_base[AI_MEM3_MAX_SIZE] __attribute((at(0x10000000)));

// �ڴ�����
u32 ai_mem1_map[AI_MEM1_ALLOC_TBL_SIZE];
// �ⲿSDRAM�ڴ��MAP
u32 ai_mem2_map[AI_MEM2_ALLOC_TBL_SIZE] __attribute((at(0xC01F4000
                                                        + AI_MEM2_MAX_SIZE)));
// �ڲ�CCM�ڴ��MAP
u32 ai_mem3_map[AI_MEM3_ALLOC_TBL_SIZE] __attribute((at(0x10000000
                                                        + AI_MEM3_MAX_SIZE)));
                                                        
// �ڴ�������
// �ڴ���С
const u32 ai_mem_tbl_size[AI_MEM_CNT] = {
    AI_MEM1_ALLOC_TBL_SIZE,
    AI_MEM2_ALLOC_TBL_SIZE,
    AI_MEM3_ALLOC_TBL_SIZE
};
// �ڴ�ֿ��С
const u32 ai_mem_blk_size[AI_MEM_CNT] = {
    AI_MEM1_BLK_SIZE,
    AI_MEM2_BLK_SIZE,
    AI_MEM3_BLK_SIZE
};
// �ڴ��ܴ�С
const u32 ai_mem_size[AI_MEM_CNT] = {
    AI_MEM1_MAX_SIZE,
    AI_MEM2_MAX_SIZE,
    AI_MEM3_MAX_SIZE
};

/*
********************************************************************************
*                           EXPORTED VARIABLE
********************************************************************************
*/
ai_mem_dev_t ai_mem_dev = {
    ai_mem_init,
    ai_mem_get_used,
    ai_mem1_base, ai_mem2_base, ai_mem3_base,
    ai_mem1_map, ai_mem2_map, ai_mem3_map,
    0, 0, 0,
};


/*
********************************************************************************
*    Function: ai_mem_cpy
* Description: �����ڴ�
*       Input: des - Ŀ�ĵ�ַ
*              src - Դ��ַ
*              len - ����
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_mem_cpy(void *des, void *src, u32 len)
{
    u8 *des_tmp = des;
    u8 *src_tmp = src;
    
    while (len--)
        *des_tmp++ = *src_tmp++;
}

/*
********************************************************************************
*    Function: ai_mem_set
* Description: �����ڴ�
*       Input: des - �ڴ��׵�ַ
*               ch - Ҫ���õ�ֵ
*              len - ����
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_mem_set(void *des, u8 ch, u32 len)
{
    u8 *des_tmp = des;
    
    while (len--)
        *des_tmp++ = ch;
}

/*
********************************************************************************
*    Function: ai_mem_init
* Description: �ڴ�����ʼ��
*       Input: num - �������ڴ���
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_mem_init(u8 num)
{
    // �ڴ�״̬����������
    ai_mem_set(ai_mem_dev.map[num], 0x00, ai_mem_tbl_size[num] * 4);
    ai_mem_dev.ready[num] = 1;
}

/*
********************************************************************************
*    Function: ai_mem_get_used
* Description: ��ȡ�ڴ�ʹ����
*       Input: num - �������ڴ���
*      Output: None
*      Return: ��Ӧ�ڴ���ʹ���ʣ�������10��,0~1000,����0.0%~100.0%
*      Others: None
********************************************************************************
*/
float ai_mem_get_used(u8 num)
{
    float used = 0.0;
    u32 i = 0;
    
    for (i = 0; i < ai_mem_tbl_size[num]; i++) {
        if (ai_mem_dev.map[num][i])
            used++;  
    }
    
    return used / (float)(ai_mem_tbl_size[num]);
}

/*
********************************************************************************
*    Function: mem_malloc
* Description: �ڴ����(�ڲ�����)
*       Input:  num - �������ڴ���
*              size - Ҫ������ڴ��С(�ֽ�)
*      Output: None
*      Return: 0xffffffff,�������;����,�ڴ�ƫ�Ƶ�ַ
*      Others: None
********************************************************************************
*/
static u32 mem_malloc(u8 num, u32 size)
{
    u32 i = 0;
    u32 blk_size = 0;     // ��Ҫ���ڴ����
    u32 cblk_size = 0;    // �������ڴ����
    signed long offset = 0;
    
    if (!ai_mem_dev.ready[num])
        ai_mem_dev.init(num);
    if (size == 0)
        return 0xffffffff;    // ����Ҫ����
    
    // ��ȡ��Ҫ����������ڴ����
    blk_size = size / ai_mem_blk_size[num];
    if (size % ai_mem_blk_size[num])
        blk_size++;
    
    for (offset = ai_mem_tbl_size[num] - 1; offset >= 0; offset--) {
        if (!ai_mem_dev.map[num][offset])
            cblk_size++;
        else
            cblk_size = 0;
        
        if (blk_size == cblk_size) {
            for (i = 0; i < blk_size; i++) {
                ai_mem_dev.map[num][offset + i] = blk_size;
            }
            return (offset * ai_mem_blk_size[num]);
        }
    }
    
    return 0xffffffff;
}

/*
********************************************************************************
*    Function: ai_mem_malloc
* Description: �ڴ����
*       Input:  num - �������ڴ���
*              size - Ҫ������ڴ��С(�ֽ�)
*      Output: None
*      Return: ���䵽���ڴ��׵�ַ
*      Others: None
********************************************************************************
*/
void *ai_mem_malloc(u8 num, u32 size)
{
    u32 offset = 0;
    
    offset = mem_malloc(num, size);
    if (offset == 0xffffffff)
        return NULL;
    else
        return (void *)((u32)ai_mem_dev.base[num] + offset);
}

/*
********************************************************************************
*    Function: ai_mem_realloc
* Description: ���·����ڴ�
*       Input:  num - �������ڴ���
*               ptr - ���ڴ��׵�ַ
*              size - Ҫ������ڴ��С(�ֽ�)
*      Output: None
*      Return: �·��䵽���ڴ��׵�ַ
*      Others: None
********************************************************************************
*/
void *ai_mem_realloc(u8 num, void *ptr, u32 size)
{
    u32 offset = 0;
    
    offset = mem_malloc(num, size);
    if (offset == 0xffffffff) {
        return NULL;
    } else {
        ai_mem_cpy((void *)((u32)ai_mem_dev.base[num] + offset), ptr, size);
        ai_mem_free(num, ptr);
        return (void *)((u32)ai_mem_dev.base[num] + offset);
    }
}

/*
********************************************************************************
*    Function: mem_free
* Description: �ͷ��ڴ�(�ڲ�����)
*       Input:  num - �������ڴ���
*              size - �ڴ�ƫ�Ƶ�ַ
*      Output: None
*      Return: On success, 0 is returned,
*              On error, negative integer is returned.
*      Others: None
********************************************************************************
*/
static int mem_free(u8 num, u32 offset)
{
    int i;
    int idx, blk_size;
    
    // δ��ʼ��,��ִ�г�ʼ��
    if (!ai_mem_dev.ready[num]) {
        ai_mem_dev.init(num);
        return -1;
    }
    
    if (offset < ai_mem_size[num]) {
        idx = offset / ai_mem_blk_size[num];    // map�е��ڴ������
        blk_size = ai_mem_dev.map[num][idx];    // �õ��ڴ����
        for (i = 0; i < blk_size; i++) {
            ai_mem_dev.map[num][idx + i] = 0;
        }
        return 0;
    } else {
        return -2;
    }
}

/*
********************************************************************************
*    Function: ai_mem_free
* Description: �ͷ��ڴ�
*       Input:  num - �������ڴ���
*              size - �ڴ�ƫ�Ƶ�ַ
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_mem_free(u8 num, u8 *ptr)
{
    u32 offset = 0;
    
    if (ptr == NULL)
        return;
    offset = (u32)ptr - (u32)ai_mem_dev.base[num];
    mem_free(num, offset);
}
