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
#define ai_led1_on() \
    do { \
        AI_LED1 = 0; \
    } while (0)
#define ai_led2_on() \
    do { \
        AI_LED2 = 0; \
    } while (0)
#define ai_led3_on() \
    do { \
        AI_LED3 = 0; \
    } while (0)
#define ai_led4_on() \
    do { \
        AI_LED4 = 0; \
    } while (0)
#define ai_led5_on() \
    do { \
        AI_LED5 = 0; \
    } while (0)
#define ai_led6_on() \
    do { \
        AI_LED6 = 0; \
    } while (0)
#define ai_led7_on() \
    do { \
        AI_LED7 = 0; \
    } while (0)
#define ai_led8_on() \
    do { \
        AI_LED8 = 0; \
    } while (0)

// 关闭某个LED
#define ai_led1_off() \
    do { \
        AI_LED1 = 1; \
    } while (0)
#define ai_led2_off() \
    do { \
        AI_LED2 = 1; \
    } while (0)
#define ai_led3_off() \
    do { \
        AI_LED3 = 1; \
    } while (0)
#define ai_led4_off() \
    do { \
        AI_LED4 = 1; \
    } while (0)
#define ai_led5_off() \
    do { \
        AI_LED5 = 1; \
    } while (0)
#define ai_led6_off() \
    do { \
        AI_LED6 = 1; \
    } while (0)
#define ai_led7_off() \
    do { \
        AI_LED7 = 1; \
    } while (0)
#define ai_led8_off() \
    do { \
        AI_LED8 = 1; \
    } while (0)

void ai_leds_init();

#endif /* AI_LEDS_H_ */