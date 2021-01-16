/*
********************************************************************************
*                          SYS INCLUDES
********************************************************************************
*/
#include <aigpio.h>
#include <aisys.h>
#include <aidelay.h>

/*
********************************************************************************
*                        HARDWARE INCLUDES
********************************************************************************
*/
#include "ailed.h"
#include "aikey.h"
#include "aiusart.h"
#include "aiexti.h"
#include "aiiwdg.h"
#include "aitimer.h"
#include "aiwwdg.h"
#include "aitouchpad.h"
#include "aiwm9825g6kh.h"

/*
********************************************************************************
*                               Private variables
********************************************************************************
*/
static u16 ai_tests_ram[250000] __attribute__((at(0XC0000000)));  //测试用数组

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_test_key(void);
void ai_test_usart(void);
void ai_test_iwdg(void);
void ai_test_pwm_ds0(void);
void ai_test_capture(void);
void ai_test_touchpad(void);
void ai_test_sdram(void);

/*
********************************************************************************
*                              MAIN
********************************************************************************
*/
int main(void)
{
    u8 i = 0;
    u32 ts = 0;
    u8 key = AI_KEY_ALL_UP;
    
    ai_sys_clock_init(360, 25, 2, 8);    // 设置时钟180MHz
    ai_delay_init(180);
    /* 外设初始化 */
	ai_uart_init(90, 115200);
    ai_led_init();
    ai_key_init();
    ai_wm9825g6kh_init();
    ai_delay_ms(100);
     
    /* 设置外设的开始运行状态 */
    ai_led_on(AI_LED_DS0);
    ai_led_off(AI_LED_DS1);
    
    for (ts = 0; ts < 250000; ts++) {
		ai_tests_ram[ts] = ts;            //预存测试数据	 
  	}
    
    /* main loop */
    while (1) {
        key = ai_key_scan(0);
        if (key == AI_KEY0_DOWN) {
            ai_test_sdram();
        } else if (key == AI_KEY1_DOWN) {
            for (ts = 0; ts < 250000; ts++) {
                printf("ai_tests_ram[%d]: %d\r\n", ts, ai_tests_ram[ts]);
            }
        } else {
            ai_delay_ms(10);
        }
        i++;
        if (i == 20) {
            i = 0;
            AI_DS0 = !AI_DS0;
        }
    }
}

/*
********************************************************************************
*                              test functions
* 1 TIM3定时器中断测试
*   初始化：10KHz的计数频率，计数5K次为500ms
*          ai_timer3_interrupt_init(5000 - 1, 9000 - 1)
*          然后在main loop中设置DS0灯指示状态：AI_DS0 = !AI_DS0;ai_delay_ms(200);
********************************************************************************
*/
void ai_test_key(void)
{
    u8 key;
    
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

void ai_test_usart(void)
{
    u8 i;
    static u8 len = 0;
    static u16 times = 0;
    
    if (AI_USART_RX_STA & 0x8000) {
        len = AI_USART_RX_STA & 0x3fff;
        printf("\r\n您发送的消息为：\r\n");
        for (i = 0; i < len; i++) {
            USART1->DR = AI_USART_RX_BUF[i];
            while ((USART1->SR & 0x40) == 0)
                ; /* empty statments */
		}
        printf("\r\n\r\n");
        AI_USART_RX_STA = 0;
    } else {
        times++;
        if (times % 5000 == 0) {
            printf("\r\nSTM32F429IGTx开发板 - 串口实验\r\n");
            printf("takeno\r\n\r\n\r\n");
        }
        
        if (times % 200 == 0)
            printf("请输入数据，以回车键结束\r\n");
        ai_delay_ms(10);
    }
}

/*
********************************************************************************
*    Function: 独立看门狗测试程序
* Description: 1）初始化：预分频数为128，重载值为500，溢出时间为2s
*                        ai_iwdg_init(5, 500);
*              2）测试时配合DS0灯，在开始时一直点亮，如果看门狗复位会进行闪烁
*       Input: void
*      Output: None
*      Return: void
*      Others: 对于窗口看门狗的测试，初始化使用：
*              1）ai_delay_ms(1000);
*                 ai_wwdg_init(0x7f, 0x5f, 3);
*              2）配合在main loop中设置ai_led_off(AI_LED_DS0)
********************************************************************************
*/
void ai_test_iwdg(void)
{
    if (ai_key_scan(0) == AI_KEY_WK_DOWN) {
        ai_iwdg_feed();
        printf("ai_iwdg_feed success.\r\n\r\n");
    }
    ai_delay_ms(10);
}

/*
********************************************************************************
*    Function: TIM3 PWM测试函数
* Description: 测试使用DS0灯作为PWM控制的设备，测试前的初始化内容如下：
*              ai_timer3_pwm_init(500 - 1, 90 - 1);
*              设置为1MHz的计数频率，2KHz的PWM
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_test_pwm_ds0(void)
{
    static u16 ds0_pwm_val = 0;
    static u8 dir = 1;
    
    if (dir)
        ds0_pwm_val++;
    else
        ds0_pwm_val--;
    
    if (ds0_pwm_val > 300)
        dir = 0;
    if (ds0_pwm_val == 0)
        dir = 1;
    ai_timer3_set_pwm(ds0_pwm_val);
}

/*
********************************************************************************
*    Function: TIM5 CH1输入捕获测试函数
* Description: 使用此函数前需要初始化，如下：
*              ai_timer5_ch1_capture_init(0xffffffff, 90 - 1);
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_test_capture(void)
{
    static long long temp = 0;
    
    if (ai_timer5_ch1_capture_stat & 0x80) {
        temp = ai_timer5_ch1_capture_stat & 0x3f;
        temp *= 0xffffffff;
        temp += ai_timer5_ch1_capture_val;
        printf("HIGH: %lld us\r\n", temp);
        ai_timer5_ch1_capture_stat = 0;
    }
}

void ai_test_touchpad(void)
{
    static u8 t = 0;
    
    if (!ai_touchpad_scan(0))
        AI_DS1 = !AI_DS1;
    t++;
    if (t == 150) {
        t = 0;
        AI_DS0 = !AI_DS0;
    }
}

/*
********************************************************************************
*    Function: SDRAM测试函数
* Description: 1）初始化 ai_wm9825g6kh_init
*              
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_test_sdram(void)
{
    u32 i = 0;
    u32 tmp = 0;
    u32 sval = 0;
    
    for (i = 0; i < 32 * 1024 * 1024; i += 16 * 1024) {
        *(vu32 *)(AI_BANK5_SDRAM_ADDR + i) = tmp;
        tmp++;
    }
    for (i = 0; i < 32 * 1024 * 1024; i += 16 * 1024) {
        tmp = *(vu32 *)(AI_BANK5_SDRAM_ADDR + i);
        if (i == 0) {
            sval = tmp;
        } else if (tmp <= sval) {
            break;
        }
        printf("SDRAM Capacity: %dKB\r\n", (u16)(tmp - sval + 1) * 16);
    }
}
