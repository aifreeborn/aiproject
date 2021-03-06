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
#define    AI_MEM_CNT  3                         // 定义支持的MEM块数

#define    AI_MEM_IN    0                        // 内部内存池
#define    AI_MEM_EXT   1                        // 外部内存池(SDRAM)
#define    AI_MEM_CCM   2                        // CCM内存池,仅CPU可以访问

// mem1内存参数设定, mem1完全处于内部MEM里面
#define    AI_MEM1_BLK_SIZE    64                // 内存块大小为64字节
#define    AI_MEM1_MAX_SIZE    (160 * 1024)      // 最大管理内存 160K
// 内存表大小
#define    AI_MEM1_ALLOC_TBL_SIZE    (AI_MEM1_MAX_SIZE / AI_MEM1_BLK_SIZE)

// mem2内存参数设定.mem2的内存池处于外部SDRAM里面
#define    AI_MEM2_BLK_SIZE    64                // 内存块大小为64字节
#define    AI_MEM2_MAX_SIZE    (28912 * 1024)    // 最大管理内存28912K
// 内存表大小
#define    AI_MEM2_ALLOC_TBL_SIZE    (AI_MEM2_MAX_SIZE / AI_MEM2_BLK_SIZE)

// mem3内存参数设定.mem3处于CCM,用于管理CCM(特别注意,这部分MEM,仅CPU可以访问!!)
#define    AI_MEM3_BLK_SIZE    64                // 内存块大小为64字节
#define    AI_MEM3_MAX_SIZE    (60 * 1024)       // 最大管理内存60K
// 内存表大小
#define    AI_MEM3_ALLOC_TBL_SIZE    (AI_MEM3_MAX_SIZE / AI_MEM3_BLK_SIZE)

/*
********************************************************************************
*                           EXPORTED STRUCT/VARIABLE
********************************************************************************
*/
typedef struct _ai_mem_dev_t {
    void (*init)(u8);                            // 初始化
    float (*used)(u8);                           // 内存使用率
    u8 *base[AI_MEM_CNT];                        // 内存池AI_MEM_BANK个区域的内存
    u32 *map[AI_MEM_CNT];                        // 内存管理状态表
    u8 ready[AI_MEM_CNT];                        // 内存管理是否就绪
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
