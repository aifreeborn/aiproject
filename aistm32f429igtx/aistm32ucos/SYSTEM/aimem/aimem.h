#ifndef AI_MEM_H_
#define AI_MEM_H_


/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "aitypes.h"

/*
********************************************************************************
*                           EXPORTED MACRO
********************************************************************************
*/
#define    AI_MEM_CNT  3                         // ����֧�ֵ�MEM����

#define    AI_MEM_IN    0                        // �ڲ��ڴ��
#define    AI_MEM_EXT   1                        // �ⲿ�ڴ��(SDRAM)
#define    AI_MEM_CCM   2                        // CCM�ڴ��,��CPU���Է���

// mem1�ڴ�����趨, mem1��ȫ�����ڲ�MEM����
#define    AI_MEM1_BLK_SIZE    64                // �ڴ���СΪ64�ֽ�
#define    AI_MEM1_MAX_SIZE    (160 * 1024)      // �������ڴ� 160K
// �ڴ���С
#define    AI_MEM1_ALLOC_TBL_SIZE    (AI_MEM1_MAX_SIZE / AI_MEM1_BLK_SIZE)

// mem2�ڴ�����趨.mem2���ڴ�ش����ⲿSDRAM����
#define    AI_MEM2_BLK_SIZE    64                // �ڴ���СΪ64�ֽ�
#define    AI_MEM2_MAX_SIZE    (28912 * 1024)    // �������ڴ�28912K
// �ڴ���С
#define    AI_MEM2_ALLOC_TBL_SIZE    (AI_MEM2_MAX_SIZE / AI_MEM2_BLK_SIZE)

// mem3�ڴ�����趨.mem3����CCM,���ڹ���CCM(�ر�ע��,�ⲿ��MEM,��CPU���Է���!!)
#define    AI_MEM3_BLK_SIZE    64                // �ڴ���СΪ64�ֽ�
#define    AI_MEM3_MAX_SIZE    (60 * 1024)       // �������ڴ�60K
// �ڴ���С
#define    AI_MEM3_ALLOC_TBL_SIZE    (AI_MEM3_MAX_SIZE / AI_MEM3_BLK_SIZE)

/*
********************************************************************************
*                           EXPORTED STRUCT/VARIABLE
********************************************************************************
*/
typedef struct _ai_mem_dev_t {
    void (*init)(u8);                            // ��ʼ��
    float (*used)(u8);                           // �ڴ�ʹ����
    u8 *base[AI_MEM_CNT];                        // �ڴ��AI_MEM_BANK��������ڴ�
    u32 *map[AI_MEM_CNT];                        // �ڴ����״̬��
    u8 ready[AI_MEM_CNT];                        // �ڴ�����Ƿ����
} ai_mem_dev_t;

extern ai_mem_dev_t ai_mem_dev;

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_mem_cpy(void *des, void *src, u32 len);
void ai_mem_set(void *des, u8 ch, u32 len);
void ai_mem_init(u8 num);
float ai_mem_get_used(u8 num);
void *ai_mem_malloc(u8 num, u32 size);
void *ai_mem_realloc(u8 num, void *ptr, u32 size);
void ai_mem_free(u8 num, u8 *ptr);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_MEM_H_ */
