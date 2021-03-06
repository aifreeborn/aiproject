#ifndef AI_PCF8574_H_
#define AI_PCF8574_H_


/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "aitypes.h"

/*
********************************************************************************
*                          Exported macro
********************************************************************************
*/
#define    AI_PCF8574_INT        PB_IN(12)        // PCF8574 INT脚

// PCF8574各个IO的功能
#define    AI_BEEP_IO             0        // 蜂鸣器控制引脚     -> P0
#define    AI_AP_INT_IO           1        // AP3216C中断引脚    -> P1
#define    AI_DCMI_PWDN_IO        2        // DCMI的电源控制引脚 -> P2
#define    AI_USB_PWR_IO          3        // USB电源控制引脚    -> P3
#define    AI_EX_IO               4        // 扩展IO,自定义使用  -> P4
#define    AI_MPU_INT_IO          5        // MPU9250中断引脚    -> P5
#define    AI_RS485_RE_IO         6        // RS485_RE引脚       -> P6
#define    AI_ETH_RESET_IO        7        // 以太网复位引脚     -> P7

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
int ai_pcf8574_init(void);
int ai_pcf8574_read_byte(void);
int ai_pcf8574_write_byte(u8 data);
int ai_pcf8574_read_bit(u8 bitn);
int ai_pcf8574_write_bit(u8 bitn, u8 stat);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_PCF8574_H_ */
