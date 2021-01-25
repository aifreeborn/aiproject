#ifndef AI_ADC_H_
#define AI_ADC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "aitypes.h"

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
void ai_adc_init(void);
u16 ai_adc_get_data(u8 ch);
u16 ai_adc_get_average_val(u8 ch, u8 times);
int ai_adc_get_temperature(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_ADC_H_ */
