/*
 * =============================================================================
 *
 *       Filename:  ailibv4l2.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/19/2020 09:38:56 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  takeno (takeno2020), takeno2020@163.com
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef AI_LIB_V4L2_H_
#define AI_LIB_V4L2_H_

#include <linux/videodev2.h>
#include <stdint.h>

/*******************************************************************************
 *    Function: aiv4l2_open
 * Description: 打开v4l2设备
 *       Input: dev  - 设备节点名字
 *              flag - 打开标志位，与系统调用open相同
 *      Output: None
 *      Return: 失败返回-1; 成功返回0
 *      Others:
 ******************************************************************************/
int aiv4l2_open(const char *dev, int flag);

/*******************************************************************************
 *    Function: aiv4l2_close
 * Description: 关闭v4l2设备
 *       Input: fd  - aiv4l2_open返回的设备文件描述符
 *      Output: None
 *      Return: 失败返回-1; 成功返回0
 *      Others:
 ******************************************************************************/
int aiv4l2_close(int fd);

/*******************************************************************************
 *    Function: aiv4l2_query_cap
 * Description: 查询v4l2设备的功能.
 *       Input: fd   - aiv4l2_open返回的设备文件描述符
 *              argp - Pointer to struct v4l2_capability.
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_query_cap(int fd, struct v4l2_capability *argp);

#endif /* AI_LIB_V4L2_H_ */
