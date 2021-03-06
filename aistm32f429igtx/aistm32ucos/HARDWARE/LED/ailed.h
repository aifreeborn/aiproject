#ifndef AI_LED_H_
#define AI_LED_H_	

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
********************************************************************************
*                               INCLUDE FILES
********************************************************************************
*/    
#include <aigpio.h>	   

/*
********************************************************************************
*                                 MACRO
********************************************************************************
*/
#define    AI_LED0    PB_OUT(1)
#define    AI_LED1    PB_OUT(0)
#define    AI_DS0     AI_LED0
#define    AI_DS1     AI_LED1
    
typedef enum {
    AI_LED_ON = 0,
    AI_LED_OFF
} ai_led_switch_t;

typedef enum {
    AI_LED_DS0 = 0,
    AI_LED_DS1
} ai_led_name_t;

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
void ai_led_init(void);
void ai_led_on(u8 num);
void ai_led_off(u8 num);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_LED_H_ */

