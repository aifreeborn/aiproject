#include "aikeys.h"
#include <aisys.h>

/*******************************************************************************
* 用于扫描按键使用的变量：
*     ai_keys_curr、ai_keys_old, ai_keys_keep_time
* 提供给应用程序使用的变量：
*     ai_keys_pressed：表示当前被按住不放的按键；
*     ai_keys_down   ：表示新按下的按键；
*     ai_keys_up     ：表示新松开的按键；
*     ai_keys_last   ：与ai_keys_pressed对应保存按键键值
*     ai_keys_can_change ：应用程序用来控制是否允许新的扫描；
* 说明：22.1184M晶体约5ms中断一次。
*******************************************************************************/
volatile uint8 idata ai_keys_curr, ai_keys_old, ai_keys_keep_time;
volatile uint8 idata ai_keys_pressed;
volatile uint8 idata ai_keys_down, ai_keys_up, ai_keys_last;
volatile uint8 ai_keys_can_change;

/*******************************************************************************
* 函数功能：定时器0初始化，用来做键盘扫描
*     定时器的启动与停止仅受TCON寄存器中的TRX(X=0, 1)来控制；
*     定时模式：方式1， 16为定时器/计数器
*******************************************************************************/
void ai_keys_init_timer0(void)
{
    TMOD &= 0xf0;
    TMOD |= 0x01;
    ET0 = 1;               // 定时器/计数器0中断允许位：打开T0中断
    TR0 = 1;               // 定时器0运行控制位，置1启动定时器0
}

void ai_keys_timer0_isr(void) interrupt 1
{
    // 定时器0重装，定时间隔为5ms，加15是为了修正重装所花费时间
    // 这个值可以通过软件仿真来确定，在这里设置断点，调整使两次运行
    // 时间差刚好为5ms即可。
    TH0 = (65536 - AI_SYS_CLK / 1000 / 12 * 5 + 15) / 256;
    TL0 = (65536 - AI_SYS_CLK / 1000 / 12 * 5 + 15) % 256;

    // 如果正在处理按键，则不再扫描键盘
    if (!ai_keys_can_change)
        return;

    // 开始键盘扫描,保存按键状态到当前按键情况,ai_keys_curr总共有8个bit
    // 当某个开关按下时，对应的bit为1
    ai_keys_curr = ai_keys_get_val();
    if (ai_keys_curr != ai_keys_old) {
        ai_keys_keep_time = 0;
        ai_keys_old = ai_keys_curr;
        return;
    } else {
        // 按下时间累计
        ai_keys_keep_time++;
        if (ai_keys_keep_time >= 1) {
            ai_keys_keep_time = 1;
            // 包含当前按下的所有按键的键值
            ai_keys_pressed = ai_keys_old;
            // 求出新按下的键
            ai_keys_down |= (~ai_keys_last) & (ai_keys_pressed);
            // 求出新释放的按键
            ai_keys_up |= ai_keys_last & (~ai_keys_pressed);
            ai_keys_last = ai_keys_pressed;
        }
    }
}

void ai_keys_init(void)
{
    AI_KEYS = 0xff;

    ai_keys_curr = 0;
    ai_keys_old = 0;
    ai_keys_keep_time = 0;
    ai_keys_pressed = 0;
    ai_keys_down = 0;
    ai_keys_up = 0;
    ai_keys_last = 0;
    
    ai_keys_init_timer0();
    ai_keys_can_change = 1;
}