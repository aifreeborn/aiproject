/*
 * =============================================================================
 *
 *       Filename:  ailibv4l2.c
 *
 *    Description:  v4l2接口封装
 *
 *        Version:  1.0
 *        Created:  11/19/2020 09:37:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  takeno (takeno2020), takeno2020@163.com
 *   Organization:  
 *
 * =============================================================================
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "ailog.h"
#include "ailibv4l2.h"

#undef AI_LOG_TAG
#define AI_LOG_TAG "ailibv4l2"

static int aiv4l2_get_pixel_depth(unsigned int fmt)
{
    int depth = 0;

    switch (fmt) {
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YVU420:
        depth = 12;
        break;
    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_YVYU:
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_VYUY:
    case V4L2_PIX_FMT_NV16:
    case V4L2_PIX_FMT_NV61:
    case V4L2_PIX_FMT_YUV422P:
        depth = 16;
        break;
    case V4L2_PIX_FMT_RGB32:
        depth = 32;
        break;
    }

    return depth;
}

int aiv4l2_open(const char *dev, int flag)
{
    int fd = open(dev, flag);
    if (fd < 0) {
        AI_LOG_ERROR("Fail to open %s\n", dev);
        return -1;
    }

    return 0;
}

int aiv4l2_close(int fd)
{
    if (close(fd)) {
        AI_LOG_ERROR("Fail to close v4l2 device.\n");
        return -1;
    }

    return 0;
}

int aiv4l2_query_cap(int fd, struct v4l2_capability *argp)
{
    if (ioctl(fd, VIDIOC_QUERYCAP, argp) < 0) {
        AI_LOG_ERROR("VIDIOC_QUERYCAP failed!\n");
        return -1;
    }

    return 0;
}
