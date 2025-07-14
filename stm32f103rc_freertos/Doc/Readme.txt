/*******************************************************************************
 *
 * 创建RTOS工程模板
 *
*******************************************************************************/
1) 拷贝HAL库文件
   将STM32Cube_FW_F1_V1.8.0/Drivers/STM32F1xx_HAL_Driver下的Inc、Src
   拷贝Libraries下;

2) 拷贝STM32Cube_FW_F1_V1.8.0\Drivers\CMSIS\Device\ST\STM32F1xx\Source\Templates\arm
   下的启动文件到Startup目录;

3) 拷贝STM32Cube_FW_F1_V1.8.0\Drivers\CMSIS\Include下的
   cmsis_armcc.h、core_cm3.h、cmsis_compiler.h、cmsis_version.h
   到Core/Include下;
   
4) 拷贝STM32Cube_FW_F1_V1.8.0\Drivers\CMSIS\Device\ST\STM32F1xx\Include下的
   stm32f1xx.h、system_stm32f1xx.h和stm32f103xe.h
   到User\Include下;

5) 拷贝STM32Cube_FW_F1_V1.8.0\Projects\STM32F103RB-Nucleo\Templates\Inc下的
   stm32f1xx_it.h、stm32f1xx_hal_conf.h和main.h
   到User\Include下;

6) 拷贝STM32Cube_FW_F1_V1.8.0\Projects\STM32F103RB-Nucleo\Templates\Src下的
   system_stm32f1xx.c、stm32f1xx_it.c、stm32f1xx_hal_msp.c和main.c
   到User\Src下
   
7) 新建工程添加宏定义：
   USE_HAL_DRIVER,USE_HAL_LEGACY,STM32F103xE
