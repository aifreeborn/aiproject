#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aisys.h"
#include "airemote.h"

/*
********************************************************************************
*                          PRIVATE DEFINE/FUNCTIONS
********************************************************************************
*/
// �������������
#define    AI_REMOTE_RDATA        PA_IN(8)

//����ң��ʶ����(ID),ÿ��ң�����ĸ�ֵ��������һ��,��Ҳ��һ����.
#define    AI_REMOTE_ID           0

/*
********************************************************************************
*                          EXPORTED VARIABLE
********************************************************************************
*/
u8    ai_remote_cnt;        // �������µĴ���

/*
********************************************************************************
*                          PRIVATE VARIABLE
********************************************************************************
*/
/*
 * ң��������״̬
 *   [7]:�յ����������־
 *   [6]:�õ���һ��������������Ϣ
 *   [5]:����
 *   [4]:����������Ƿ��Ѿ�������
 * [3:0]:�����ʱ��
 */
static    u8    ai_remote_stat = 0;
static    u16   ai_remote_dval = 0;    // �½���ʱ��������ֵ
static    u32   ai_remote_rec = 0;     // ������յ�������,�Ӹ�λ����λ�Դ�Ϊ��
                                       // ��ַ�롢��ַ���롢�����롢���Ʒ���

/*
********************************************************************************
*    Function: ai_remote_init
* Description: ����ң�س�ʼ��
*       Input: void
*      Output: None
*      Return: void
*      Others: ����ң����ʹ��PA8(TIM1_CH1)���ţ�������ͷDCMI_XCLK����
*              ���� TIM1 �Ǹ߼���ʱ�������� 2 ���жϷ�������
*              TIM1_UP_TIM10_IRQHandler ���ڴ��� TIM1 ������¼�
*              TIM1_CC_IRQHandler ���ڴ��� TIM1 �����벶���¼���
********************************************************************************
*/
void ai_remote_init(void)
{
    RCC->AHB1ENR |= 0x1;
    RCC->APB2ENR |= 0x1;            // TIM1 clock enabled
    
    ai_gpio_set(GPIOA, PIN8, GPIO_MODE_AF,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set_af(GPIOA, 8, 1);    // AF1
    
    TIM1->ARR = 10000;              // �趨�������Զ���װֵ ���10ms���
    TIM1->PSC = 180 - 1;            // Ԥ��Ƶ��,1M�ļ���Ƶ��,1us��1.
    
    TIM1->CCMR1 &= 0xff00;
    // ���������˲��� 8����ʱ��ʱ�������˲�;fSAMPLING=fCK_INT N=8
    // ��Ԥ��Ƶ��������������ÿ��⵽һ�����ر�ִ�в���
    // CC1 ͨ������Ϊ���룬 IC1 ӳ�䵽 TI1 ��
    TIM1->CCMR1 |= (0x3 << 4) | (0x0 << 2) | 0x1;
    
    TIM1->CCER &= ~(0x1 << 1);                    // C1P=0 �����ز���
    TIM1->CCER |= 0x1;                            // ʹ�ܲ���
    TIM1->DIER |= 0x1 << 1;                       // ʹ��CC1�ж�
    TIM1->DIER |= 0x1;                            // ʹ�ܸ����ж�
    TIM1->CR1 |= 0x1;                             // ʹ�ܶ�ʱ��1
    ai_nvic_init(1, 3, TIM1_CC_IRQn, 2);          // ��ռ1�������ȼ�3����2
    ai_nvic_init(1, 2, TIM1_UP_TIM10_IRQn, 2);
}

/*
********************************************************************************
*    Function: TIM1_UP_TIM10_IRQHandler
* Description: ��ʱ��1����ж�
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void TIM1_UP_TIM10_IRQHandler(void)
{
    u16 tsr = TIM1->SR;
    if (tsr & 0x1) {
        if (ai_remote_stat & 0x80) {    // �ϴ������ݱ����յ���
            ai_remote_stat &= ~0x10;    // ȡ���������Ѿ���������
            // ����Ѿ����һ�ΰ����ļ�ֵ��Ϣ�ɼ�
            if ((ai_remote_stat & 0x0f) == 0x00)
                ai_remote_stat |= 0x1 << 6;
            if ((ai_remote_stat & 0x0f) < 14) {
                ai_remote_stat++;
            } else {
                ai_remote_stat &= ~(0x1 << 7);
                ai_remote_stat &= 0xf0;
            }
        }
    }
    TIM1->SR &= ~0x1;
}

/*
********************************************************************************
*    Function: TIM1_CC_IRQHandler
* Description: ��ʱ��1���벶���жϷ������
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void TIM1_CC_IRQHandler(void)
{
    u16 tsr = TIM1->SR;
    
    if (tsr & 0x02) {
        if (AI_REMOTE_RDATA) {                  // �����ز���
            TIM1->CCER |= 0x1 << 1;             // CC1P=1 ����Ϊ�½��ز���
            TIM1->CNT = 0;                      // ��ն�ʱ��ֵ
            ai_remote_stat |= 0x1 << 4;         // ����������Ѿ�������
        } else {                                // �½��ز���
            ai_remote_dval = TIM1->CCR1;
            TIM1->CCER &= ~(0x1 << 1);          // CC1P=0 ����Ϊ�����ز���
            if (ai_remote_stat & 0x10) {
                if (ai_remote_stat & 0x80) {    // ���յ���������
                    if (ai_remote_dval > 300 && ai_remote_dval < 800) {
                        // 560Ϊ��׼ֵ,560us
                        ai_remote_rec <<= 1;
                        ai_remote_rec |= 0x0;
                    } else if (ai_remote_dval > 1400 && ai_remote_dval < 1800) {
                        // 1680Ϊ��׼ֵ,1680us
                        ai_remote_rec <<= 1;
                        ai_remote_rec |= 0x1;
                    } else if (ai_remote_dval > 2200 && ai_remote_dval < 2600) {
                        // �õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
                        ai_remote_cnt++;
                        ai_remote_stat &= 0xf0;
                    }
                } else if (ai_remote_dval > 4200 && ai_remote_dval < 4700) {
                    // 4500Ϊ��׼ֵ4.5ms
                    ai_remote_stat |= 0x1 << 7;    // ��ǳɹ����յ���������
                    ai_remote_cnt = 0;             // �����������������
                }
            }
            ai_remote_stat &= ~(0x1 << 4);
        }
    }
    TIM1->SR &= ~((0x1 << 9) | (0x1 << 1));
}

/*
********************************************************************************
*    Function: ai_remote_scan
* Description: ����������
*       Input: void
*      Output: None
*      Return: 0,û���κΰ�������; ��0ֵ,���µİ�����ֵ.
*      Others: None
********************************************************************************
*/
u8 ai_remote_scan(void)
{
    u8 stat = 0;
    u8 addr, naddr;
    
    // �õ�һ��������������Ϣ��
    if (ai_remote_stat & (0x1 << 6)) {
        addr = ai_remote_rec >> 24;           // �õ���ַ��
        naddr = ai_remote_rec >> 16;
        // ����ң��ʶ����(ID)����ַ
        if ((addr == (u8)~naddr) && (addr == AI_REMOTE_ID)) {
            addr = ai_remote_rec >> 8;
            naddr = ai_remote_rec;
            if (addr == ((u8)~naddr))
                stat = addr;
        }
        // �������ݴ���/ң���Ѿ�û�а�����
        if ((stat == 0) || ((ai_remote_stat & 0x80) == 0)) {
            ai_remote_stat &= ~(0x1 << 6);    // ������յ���Ч������ʶ
            ai_remote_cnt = 0;                // �����������������
        }
    }
    return stat;
}
