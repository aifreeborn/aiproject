#ifndef AI_KEY_H_
#define AI_KEY_H_	

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
#define    AI_KEY0    PH_IN(3)
#define    AI_KEY1    PH_IN(2)
#define    AI_KEY2    PC_IN(13)
#define    AI_WK_UP   PA_IN(0)

typedef enum {
    AI_KEY_ALL_UP = 0,
    AI_KEY0_DOWN,
    AI_KEY1_DOWN,
    AI_KEY2_DOWN,
    AI_KEY_WK_DOWN
} ai_key_pressed_t;

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
void ai_key_init(void);
u8 ai_key_scan(u8 mode);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_KEY_H_ */
