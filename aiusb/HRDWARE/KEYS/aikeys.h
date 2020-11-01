#ifndef AI_KEYS_H_
#define AI_KEYS_H_

#include <reg52.h>
#include <aitype.h>

#define    AI_KEYS    P1
#define    AI_KEY1    0x01
#define    AI_KEY2    0x02
#define    AI_KEY3    0x04
#define    AI_KEY4    0x08
#define    AI_KEY5    0x10
#define    AI_KEY6    0x20
#define    AI_KEY7    0x40
#define    AI_KEY8    0x80

extern volatile uint8 idata ai_keys_curr, ai_keys_old, ai_keys_keep_time;
extern volatile uint8 idata ai_keys_pressed;
extern volatile uint8 idata ai_keys_down, ai_keys_up, ai_keys_last;
extern volatile uint8 ai_keys_can_change;

// 因为低电平表示按键按下，所以需要取反获得对应按键值
#define    ai_keys_get_val()    (~(P1))

void ai_keys_init(void);

#endif /* AI_KEYS_H_ */