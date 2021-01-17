#ifndef AI_LTDC_H_
#define AI_LTDC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "aitypes.h"

// ����Ĭ�ϱ�������ɫ
#define    AI_LTDC_BACKLAYER_COLOR          0x00000000	

/*
********************************************************************************
*                            LTDC�Ŀ��غ궨��
********************************************************************************
*/
enum _ai_ltdc_switch {
    AI_LTDC_OFF = 0,
    AI_LTDC_ON
};

enum _ai_ltdc_layer_switch {
    AI_LTDC_LAYER_OFF = 0,
    AI_LTDC_LAYER_ON
};

/*
********************************************************************************
*                            ���ݲ�ͬRGB LCD�Ĳ�������
********************************************************************************
*/
typedef struct {
    u32    panel_width;     // LCD���Ŀ��,�̶�����,������ʾ����ı�,
                            // ���Ϊ0,˵��û���κ�RGB������;��λΪ����
    u32    panel_height;    // LCD���ĸ߶�,�̶�����,������ʾ����ı䣻��λΪ����
    u16    hsw;             // ˮƽͬ�����
    u16    vsw;             // ��ֱͬ�����
    u16    hbp;             // ˮƽ����
    u16    vbp;             // ��ֱ����
    u16    hfp;             // ˮƽǰ��
    u16    vfp;             // ��ֱǰ��
    u8     active_layer;    // ��ǰ����:0/1
    u8     dir;             // 0,����;1,����;
    u16    width;           // LCD���
    u16    height;          // LCD�߶�
    u32    pixel_size;      // ÿ��������ռ�ֽ���
} ai_ltdc_dev_t;


extern ai_ltdc_dev_t ai_ltdc_dev;

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
int ai_ltdc_init(void);
void ai_ltdc_switch(u8 action);
void ai_ltdc_draw_point(u16 x, u16 y, u32 color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_LTDC_H_ */

