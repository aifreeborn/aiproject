#ifndef AI_GPIO_H_
#define AI_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
********************************************************************************
*                               INCLUDE FILES
********************************************************************************
*/
#include "stm32f4xx.h"
#include "aitypes.h"

/*
********************************************************************************
*                            bit-band convertion
********************************************************************************
*/
/* Convert bit-band address and bit number into bit-band alias address */
#define    BIT_BAND(addr, bitnum)    ((addr & 0xF0000000) + 0x2000000 \
                                       + ((addr & 0xFFFFF) << 5)      \
                                       + (bitnum << 2)) 
/* Convert the address as a pointer */
#define    MEM_ADDR(addr)            *((volatile unsigned long *)(addr)) 
#define    BIT_ADDR(addr, bitnum)    MEM_ADDR(BIT_BAND(addr, bitnum))

/*
********************************************************************************
*                            IO�����ú꼰��ַӳ��
********************************************************************************
*/
/* GPIO����ģʽ���� */
typedef enum {
    GPIO_MODE_IN = 0,                // ��ͨ����ģʽ
    GPIO_MODE_OUT,                   // ��ͨ���ģʽ
    GPIO_MODE_AF,                    // AF����ģʽ
    GPIO_MODE_AIN                    // ģ������ģʽ
} ai_gpio_mode_t;

typedef enum {
    GPIO_SPEED_2M = 0,               // GPIO�ٶ�2Mhz
    GPIO_SPEED_25M,                  // GPIO�ٶ�25Mhz
    GPIO_SPEED_50M,                  // GPIO�ٶ�50Mhz
    GPIO_SPEED_100M,                 // GPIO�ٶ�100Mhz
    GPIO_SPEED_NULL = 255
} ai_gpio_speed_t;

typedef enum {
    GPIO_PUPD_NONE = 0,              // ����������
    GPIO_PUPD_PU,                    // ����
    GPIO_PUPD_PD,                    // ����
    GPIO_PUPD_RES                    // ���� 
} ai_gpio_pupd_t;

typedef enum {
    GPIO_OTYPE_PP = 0,               // �������
    GPIO_OTYPE_OD,                   // ��©���
    GPIO_OTYPE_NULL = 255
} ai_gpio_otype_t;

/* GPIO���ű�Ŷ��� */
#define    PIN0            (0x1 <<  0)
#define    PIN1            (0x1 <<  1)
#define    PIN2            (0x1 <<  2)
#define    PIN3            (0x1 <<  3)
#define    PIN4            (0x1 <<  4)
#define    PIN5            (0x1 <<  5)
#define    PIN6            (0x1 <<  6)
#define    PIN7            (0x1 <<  7)
#define    PIN8            (0x1 <<  8)
#define    PIN9            (0x1 <<  9)
#define    PIN10           (0x1 << 10)
#define    PIN11           (0x1 << 11)
#define    PIN12           (0x1 << 12)
#define    PIN13           (0x1 << 13)
#define    PIN14           (0x1 << 14)
#define    PIN15           (0x1 << 15)

/* GPIO �˿�������ݼĴ��� (GPIOx_ODR) (x = A..I) */
#define    GPIOA_ODR_ADDR    (GPIOA_BASE + 0x14)    // 0x40020014
#define    GPIOB_ODR_ADDR    (GPIOB_BASE + 0x14)    // 0x40020414 
#define    GPIOC_ODR_ADDR    (GPIOC_BASE + 0x14)    // 0x40020814 
#define    GPIOD_ODR_ADDR    (GPIOD_BASE + 0x14)    // 0x40020C14 
#define    GPIOE_ODR_ADDR    (GPIOE_BASE + 0x14)    // 0x40021014 
#define    GPIOF_ODR_ADDR    (GPIOF_BASE + 0x14)    // 0x40021414    
#define    GPIOG_ODR_ADDR    (GPIOG_BASE + 0x14)    // 0x40021814   
#define    GPIOH_ODR_ADDR    (GPIOH_BASE + 0x14)    // 0x40021C14    
#define    GPIOI_ODR_ADDR    (GPIOI_BASE + 0x14)    // 0x40022014     

/* GPIO �˿��������ݼĴ��� (GPIOx_IDR) (x = A..I) */
#define    GPIOA_IDR_ADDR    (GPIOA_BASE + 0x10)    // 0x40020010 
#define    GPIOB_IDR_ADDR    (GPIOB_BASE + 0x10)    // 0x40020410 
#define    GPIOC_IDR_ADDR    (GPIOC_BASE + 0x10)    // 0x40020810 
#define    GPIOD_IDR_ADDR    (GPIOD_BASE + 0x10)    // 0x40020C10 
#define    GPIOE_IDR_ADDR    (GPIOE_BASE + 0x10)    // 0x40021010 
#define    GPIOF_IDR_ADDR    (GPIOF_BASE + 0x10)    // 0x40021410 
#define    GPIOG_IDR_ADDR    (GPIOG_BASE + 0x10)    // 0x40021810 
#define    GPIOH_IDR_ADDR    (GPIOH_BASE + 0x10)    // 0x40021C10 
#define    GPIOI_IDR_ADDR    (GPIOI_BASE + 0x10)    // 0x40022010 
 
/* IO�ڲ���,ֻ�Ե�һ��IO��, ȷ��n��ֵС��16! */
#define    PA_OUT(n)         BIT_ADDR(GPIOA_ODR_ADDR, n)  // ��� 
#define    PA_IN(n)          BIT_ADDR(GPIOA_IDR_ADDR, n)  // ����
 
#define    PB_OUT(n)         BIT_ADDR(GPIOB_ODR_ADDR, n)  // ��� 
#define    PB_IN(n)          BIT_ADDR(GPIOB_IDR_ADDR, n)  // ����
 
#define    PC_OUT(n)         BIT_ADDR(GPIOC_ODR_ADDR, n)  // ��� 
#define    PC_IN(n)          BIT_ADDR(GPIOC_IDR_ADDR, n)  // ���� 

#define    PD_OUT(n)         BIT_ADDR(GPIOD_ODR_ADDR, n)  // ��� 
#define    PD_IN(n)          BIT_ADDR(GPIOD_IDR_ADDR, n)  // ����
 
#define    PE_OUT(n)         BIT_ADDR(GPIOE_ODR_ADDR, n)  // ��� 
#define    PE_IN(n)          BIT_ADDR(GPIOE_IDR_ADDR, n)  // ����

#define    PF_OUT(n)         BIT_ADDR(GPIOF_ODR_ADDR, n)  // ��� 
#define    PF_IN(n)          BIT_ADDR(GPIOF_IDR_ADDR, n)  // ����

#define    PG_OUT(n)         BIT_ADDR(GPIOG_ODR_ADDR, n)  // ��� 
#define    PG_IN(n)          BIT_ADDR(GPIOG_IDR_ADDR, n)  // ����

#define    PH_OUT(n)         BIT_ADDR(GPIOH_ODR_ADDR, n)  // ��� 
#define    PH_IN(n)          BIT_ADDR(GPIOH_IDR_ADDR, n)  // ����

#define    PI_OUT(n)         BIT_ADDR(GPIOI_ODR_ADDR, n)  // ��� 
#define    PI_IN(n)          BIT_ADDR(GPIOI_IDR_ADDR, n)  // ����

/*
********************************************************************************
*                            Ex_NVIC_Configר�ö���
********************************************************************************
*/
typedef enum {
    GPIO_A = 0,
    GPIO_B,
    GPIO_C,
    GPIO_D,
    GPIO_E,
    GPIO_F,
    GPIO_G,
    GPIO_H,
    GPIO_I
} ai_gpio_group_t;

typedef enum {
    FTIR = 1,            // �½��ش���
    RTIR                 // �����ش���
} ai_nvic_trim_t; 

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/ 
void ai_gpio_set_af(GPIO_TypeDef *gpiox, u8 bitn, u8 afx);
void ai_gpio_set(GPIO_TypeDef *gpiox, u32 bitn,
                 u32 mode, u32 otype, u32 ospeed, u32 pupd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_GPIO_H_ */
