/*
 * =============================================================================
 *
 *       Filename:  ailog.h
 *
 *    Description:  用于定义调试信息的宏等
 *
 *        Version:  1.0
 *        Created:  11/19/2020 09:33:36 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  takeno (takeno2020), takeno2020@163.com
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef AI_LOG_H_
#define AI_LOG_H_

#include <stdio.h>

// 使用时在自己的源码文件中定义AI_LOG_TAG为自己的提示内容
// C99方式
#ifndef AI_LOG_INFO
#define AI_LOG_INFO(...) \
    do { \
        printf("[" AI_LOG_TAG "][%s, L%d]: ", __func__, __LINE__); \
        printf(__VA_ARGS__); \
    } while (0)
#endif

// GCC支持的形式
#ifndef AI_LOG_ERROR
#define AI_LOG_ERROR(args...) \
    do { \
        printf("[" AI_LOG_TAG "][%s, L%d]: ", __func__, __LINE__); \
        printf(args); \
    } while (0)
#endif

#endif /* AI_LOG_H_ */
