#ifndef AI_LEDS_H_
#define AI_LEDS_H_

#include <reg52.h>

// 定义LED
#define    AI_ALL_LEDS    P2
sbit AI_LED1 = P2^0;
sbit AI_LED2 = P2^1;
sbit AI_LED3 = P2^2;
sbit AI_LED4 = P2^3;
sbit AI_LED5 = P2^4;
sbit AI_LED6 = P2^5;
sbit AI_LED7 = P2^6;
sbit AI_LED8 = P2^7;

// 点亮某个LED
#define ai_led1_on() AI_LED1=0;
#define ai_led2_on() AI_LED2=0;
#define ai_led3_on() AI_LED3=0;
#define ai_led4_on() AI_LED4=0;
#define ai_led5_on() AI_LED5=0;
#define ai_led6_on() AI_LED6=0;
#define ai_led7_on() AI_LED7=0;
#define ai_led8_on() AI_LED8=0;

// 关闭某个LED
#define ai_led1_off() AI_LED1=1;
#define ai_led2_off() AI_LED2=1;
#define ai_led3_off() AI_LED3=1;
#define ai_led4_off() AI_LED4=1;
#define ai_led5_off() AI_LED5=1;
#define ai_led6_off() AI_LED6=1;
#define ai_led7_off() AI_LED7=1;
#define ai_led8_off() AI_LED8=1;

void ai_leds_init();

#endif /* AI_LEDS_H_ */