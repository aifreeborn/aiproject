/*
********************************************************************************
*                          SYS INCLUDES
********************************************************************************
*/
#include <aigpio.h>
#include <aisys.h>
#include <aidelay.h>
#include <ailed.h>
#include <aikey.h>

/*
********************************************************************************
*                        HARDWARE INCLUDES
********************************************************************************
*/


/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 key;
    
    ai_sys_clock_init(360, 25, 2, 8);    // …Ë÷√ ±÷”180MHz
    ai_delay_init(180);
    ai_led_init();
    ai_key_init();
    
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
    while (1) {
        key = ai_key_scan(0);
        if (key != AI_KEY_ALL_UP) {
            switch (key) {
            case AI_KEY_WK_DOWN:
                AI_DS0 = !AI_DS0;
                AI_DS1 = !AI_DS0;
                break;
            case AI_KEY0_DOWN:
                AI_DS0 = !AI_DS0;
                break;
            case AI_KEY1_DOWN:
                AI_DS1 = !AI_DS1;
                break;
            case AI_KEY2_DOWN:
                AI_DS0 = !AI_DS0;
                AI_DS1 = !AI_DS1;
                break;
            }
        } else {
            ai_delay_ms(10);
        }
	}
}
