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
// 红外数据输入脚
#define    AI_REMOTE_RDATA        PA_IN(8)

//红外遥控识别码(ID),每款遥控器的该值基本都不一样,但也有一样的.
#define    AI_REMOTE_ID           0

/*
********************************************************************************
*                          EXPORTED VARIABLE
********************************************************************************
*/
u8    ai_remote_cnt;        // 按键按下的次数

/*
********************************************************************************
*                          PRIVATE VARIABLE
********************************************************************************
*/
/*
 * 遥控器接收状态
 *   [7]:收到了引导码标志
 *   [6]:得到了一个按键的所有信息
 *   [5]:保留
 *   [4]:标记上升沿是否已经被捕获
 * [3:0]:溢出计时器
 */
static    u8    ai_remote_stat = 0;
static    u16   ai_remote_dval = 0;    // 下降沿时计数器的值
static    u32   ai_remote_rec = 0;     // 红外接收到的数据,从高位到地位以此为：
                                       // 地址码、地址反码、控制码、控制反码

/*
********************************************************************************
*    Function: ai_remote_init
* Description: 红外遥控初始化
*       Input: void
*      Output: None
*      Return: void
*      Others: 红外遥控器使用PA8(TIM1_CH1)引脚，与摄像头DCMI_XCLK公用
*              由于 TIM1 是高级定时器，它有 2 个中断服务函数：
*              TIM1_UP_TIM10_IRQHandler 用于处理 TIM1 的溢出事件
*              TIM1_CC_IRQHandler 用于处理 TIM1 的输入捕获事件，
********************************************************************************
*/
void ai_remote_init(void)
{
    RCC->AHB1ENR |= 0x1;
    RCC->APB2ENR |= 0x1;            // TIM1 clock enabled
    
    ai_gpio_set(GPIOA, PIN8, GPIO_MODE_AF,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set_af(GPIOA, 8, 1);    // AF1
    
    TIM1->ARR = 10000;              // 设定计数器自动重装值 最大10ms溢出
    TIM1->PSC = 180 - 1;            // 预分频器,1M的计数频率,1us加1.
    
    TIM1->CCMR1 &= 0xff00;
    // 配置输入滤波器 8个定时器时钟周期滤波;fSAMPLING=fCK_INT N=8
    // 无预分频器，捕获输入上每检测到一个边沿便执行捕获
    // CC1 通道配置为输入， IC1 映射到 TI1 上
    TIM1->CCMR1 |= (0x3 << 4) | (0x0 << 2) | 0x1;
    
    TIM1->CCER &= ~(0x1 << 1);                    // C1P=0 上升沿捕获
    TIM1->CCER |= 0x1;                            // 使能捕获
    TIM1->DIER |= 0x1 << 1;                       // 使能CC1中断
    TIM1->DIER |= 0x1;                            // 使能更新中断
    TIM1->CR1 |= 0x1;                             // 使能定时器1
    ai_nvic_init(1, 3, TIM1_CC_IRQn, 2);          // 抢占1，子优先级3，组2
    ai_nvic_init(1, 2, TIM1_UP_TIM10_IRQn, 2);
}

/*
********************************************************************************
*    Function: TIM1_UP_TIM10_IRQHandler
* Description: 定时器1溢出中断
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
        if (ai_remote_stat & 0x80) {    // 上次有数据被接收到了
            ai_remote_stat &= ~0x10;    // 取消上升沿已经被捕获标记
            // 标记已经完成一次按键的键值信息采集
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
* Description: 定时器1输入捕获中断服务程序
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
        if (AI_REMOTE_RDATA) {                  // 上升沿捕获
            TIM1->CCER |= 0x1 << 1;             // CC1P=1 设置为下降沿捕获
            TIM1->CNT = 0;                      // 清空定时器值
            ai_remote_stat |= 0x1 << 4;         // 标记上升沿已经被捕获
        } else {                                // 下降沿捕获
            ai_remote_dval = TIM1->CCR1;
            TIM1->CCER &= ~(0x1 << 1);          // CC1P=0 设置为上升沿捕获
            if (ai_remote_stat & 0x10) {
                if (ai_remote_stat & 0x80) {    // 接收到了引导码
                    if (ai_remote_dval > 300 && ai_remote_dval < 800) {
                        // 560为标准值,560us
                        ai_remote_rec <<= 1;
                        ai_remote_rec |= 0x0;
                    } else if (ai_remote_dval > 1400 && ai_remote_dval < 1800) {
                        // 1680为标准值,1680us
                        ai_remote_rec <<= 1;
                        ai_remote_rec |= 0x1;
                    } else if (ai_remote_dval > 2200 && ai_remote_dval < 2600) {
                        // 得到按键键值增加的信息 2500为标准值2.5ms
                        ai_remote_cnt++;
                        ai_remote_stat &= 0xf0;
                    }
                } else if (ai_remote_dval > 4200 && ai_remote_dval < 4700) {
                    // 4500为标准值4.5ms
                    ai_remote_stat |= 0x1 << 7;    // 标记成功接收到了引导码
                    ai_remote_cnt = 0;             // 清除按键次数计数器
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
* Description: 处理红外键盘
*       Input: void
*      Output: None
*      Return: 0,没有任何按键按下; 非0值,按下的按键键值.
*      Others: None
********************************************************************************
*/
u8 ai_remote_scan(void)
{
    u8 stat = 0;
    u8 addr, naddr;
    
    // 得到一个按键的所有信息了
    if (ai_remote_stat & (0x1 << 6)) {
        addr = ai_remote_rec >> 24;           // 得到地址码
        naddr = ai_remote_rec >> 16;
        // 检验遥控识别码(ID)及地址
        if ((addr == (u8)~naddr) && (addr == AI_REMOTE_ID)) {
            addr = ai_remote_rec >> 8;
            naddr = ai_remote_rec;
            if (addr == ((u8)~naddr))
                stat = addr;
        }
        // 按键数据错误/遥控已经没有按下了
        if ((stat == 0) || ((ai_remote_stat & 0x80) == 0)) {
            ai_remote_stat &= ~(0x1 << 6);    // 清除接收到有效按键标识
            ai_remote_cnt = 0;                // 清除按键次数计数器
        }
    }
    return stat;
}
