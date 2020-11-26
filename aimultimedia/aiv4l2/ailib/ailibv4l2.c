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
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <poll.h>
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

int aiv4l2_querycap(int fd, struct v4l2_capability *argp)
{
    if (ioctl(fd, VIDIOC_QUERYCAP, argp) < 0) {
        AI_LOG_ERROR("VIDIOC_QUERYCAP failed!\n");
        return -1;
    }

    return 0;
}

int aiv4l2_enuminput(int fd, int index, char *name)
{
    struct v4l2_input input;
    int found = 0;

    input.index = 0;
    while (!ioctl(fd, VIDIOC_ENUMINPUT, &input)) {
        AI_LOG_INFO("input: %s\n", input.name);
        if (input.index == index) {
            found = 1;
            strcpy(name, (const char *)input.name);
            break;
        }

        ++input.index;
    }

    if (!found) {
        AI_LOG_ERROR("Can't find input device.\n");
        return -1;
    }

    return 0;
}

int aiv4l2_set_input(int fd, int index)
{
    struct v4l2_input input;

    input.index = index;
    if (ioctl(fd, VIDIOC_S_INPUT, &input) < 0) {
        AI_LOG_ERROR("VIDIOC_S_INPUT failed!\n");
        return -1;
    }

    return 0;
}

int aiv4l2_enum_fmt(int fd, uint32_t fmt, enum v4l2_buf_type type)
{
    struct v4l2_fmtdesc fmtdesc;
    int found = 0;

    fmtdesc.type = type;
    fmtdesc.index = 0;
    while (!ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
        AI_LOG_INFO("fmt: %s\n", fmtdesc.description);
        if (fmtdesc.pixelformat == fmt) {
            found = 1;
            break;
        }
        fmtdesc.index++;
    }

    if (!found) {
        AI_LOG_ERROR("Unsupported pixel format.\n");
        return -1;
    }
    return 0;
}

int aiv4l2_set_fmt(int fd, uint32_t *width, uint32_t *height, uint32_t fmt, 
                   enum v4l2_buf_type type)
{
    struct v4l2_format v4l2_fmt;
    struct v4l2_pix_format pix_fmt;

    memset(&v4l2_fmt, 0, sizeof(v4l2_fmt));
    v4l2_fmt.type = type;

    memset(&pix_fmt, 0, sizeof(pix_fmt));
    pix_fmt.width = *width;
    pix_fmt.height = *height;
    pix_fmt.pixelformat = fmt;
    pix_fmt.sizeimage = (*width * *height * aiv4l2_get_pixel_depth(fmt) / 8);
    pix_fmt.field = V4L2_FIELD_ANY;

    v4l2_fmt.fmt.pix = pix_fmt;

    if (ioctl(fd, VIDIOC_S_FMT, &v4l2_fmt) < 0) {
        AI_LOG_ERROR("VIDIOC_S_FMT failed.\n");
        return -1;
    }

    *width = v4l2_fmt.fmt.pix.width;
    *height = v4l2_fmt.fmt.pix.height;
    return 0;
}

struct aiv4l2_buf *aiv4l2_requset_bufs(int fd, enum v4l2_buf_type type, int count)
{
    struct v4l2_requestbuffers req;
    struct aiv4l2_buf *buffer;
    size_t tmp_cnt = 0;

    req.count = count;
    req.type = type;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        AI_LOG_ERROR("VIDIOC_REQBUFS failed.\n");
        return NULL;
    }

    buffer = (struct aiv4l2_buf *)malloc(sizeof(buffer));
    buffer->count = req.count;
    tmp_cnt = sizeof(struct aiv4l2_buf_unit) * buffer->count;
    buffer->buf = (struct aiv4l2_buf_unit *)malloc(tmp_cnt);
    buffer->type = type;
    memset(buffer, 0, tmp_cnt);
    
    return buffer;
}

int aiv4l2_release_bufs(struct aiv4l2_buf *buf)
{
    free(buf->buf);
    free(buf);
    return 0;
}

int aiv4l2_query_buf(int fd, struct aiv4l2_buf *buf)
{
    struct v4l2_buffer v4l2_buf;
    struct aiv4l2_buf_unit *buf_unit;
    int i;

    for (i = 0; i < buf->count; i++) {
        v4l2_buf.type = buf->type;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        v4l2_buf.index = i;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            AI_LOG_ERROR("VIDIOC_QUERYBUF failed.\n");
            return -1;
        }

        buf_unit = &buf->buf[i];
        buf_unit->index = i;
        buf_unit->offset = v4l2_buf.m.offset;
        buf_unit->length = v4l2_buf.length;
        buf_unit->start = NULL;
    }

    return 0;
}

int aiv4l2_mmap(int fd, struct aiv4l2_buf *buf)
{
    int i;
    struct aiv4l2_buf_unit *buf_unit;

    for (i = 0; i < buf->count; i++) {
        buf_unit = &buf->buf[i];
        buf_unit->start = mmap(0, buf_unit->length,
                              PROT_READ | PROT_WRITE, MAP_SHARED,
                              fd, buf_unit->offset);
        if (buf_unit->start == MAP_FAILED) {
            AI_LOG_ERROR("v4l2 mmap failed.\n");
            goto err;
        }
    }

    return 0;

err:
    while (--i >= 0) {
        buf_unit = &buf->buf[i];
        munmap(buf_unit->start, buf_unit->length);
        buf_unit->start = NULL;
    }
    return -1;
}

int aiv4l2_munmap(int fd, struct aiv4l2_buf *buf)
{
    int i;
    struct aiv4l2_buf_unit *buf_unit;

    for (i = 0; i < buf->count; i++) {
        buf_unit = &buf->buf[i];
        munmap(buf_unit->start, buf_unit->length);
        buf_unit->start = NULL;
    }

    return 0;
}

int aiv4l2_stream_on(int fd)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        AI_LOG_ERROR("VIDIOC_STREAMON failed.\n");
        return -1;
    }

    if (aiv4l2_poll(fd) < 0)
        return -1;

    return 0;
}

int aiv4l2_stream_off(int fd)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
        AI_LOG_ERROR("VIDIOC_STREAMOFF failed.\n");
        return -1;
    }
    
    return 0;
}

int aiv4l2_qbuf(int fd, struct aiv4l2_buf_unit* buf)
{
    struct v4l2_buffer v4l2_buf;

    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;
    v4l2_buf.index = buf->index;

    if (ioctl(fd, VIDIOC_QBUF, &v4l2_buf) < 0) {
        AI_LOG_ERROR("VIDIOC_QBUF failed\n");
        return -1;
    }

    return 0;
}

int aiv4l2_qbuf_all(int fd, struct aiv4l2_buf* buf)
{
    int i;
    
    for (i = 0; i < buf->count; i++) {
        if (aiv4l2_qbuf(fd, &buf->buf[i]) < 0)
            return -1;
    }

    return 0;
}

struct aiv4l2_buf_unit* aiv4l2_dqbuf(int fd, struct aiv4l2_buf* buf)
{
    struct v4l2_buffer buffer;

    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_DQBUF, &buffer) < 0) {
        AI_LOG_ERROR("VIDIOC_DQBUF failed, dropped frame.\n");
        return NULL;
    }

    return &buf->buf[buffer.index];
}

int aiv4l2_get_ctrl(int fd, uint32_t id)
{
    struct v4l2_control ctrl;

    ctrl.id = id;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        AI_LOG_ERROR("VIDIOC_G_CTRL(id = 0x%x (%d)) failed.\n",
                     id, id - V4L2_CID_PRIVATE_BASE);
        return -1;
    }

    return ctrl.value;
}

int aiv4l2_set_ctrl(int fd, uint32_t id, int32_t value)
{
    struct v4l2_control ctrl;

    ctrl.id = id;
    ctrl.value = value;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        AI_LOG_ERROR("VIDIOC_S_CTRL(id = %#x (%d), value = %d) failed.\n",
                      id, id - V4L2_CID_PRIVATE_BASE, value);

        return -1;
    }

    return ctrl.value;
}

int aiv4l2_get_parm(int fd, struct v4l2_streamparm *stream_parm)
{
    stream_parm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_G_PARM, stream_parm) < 0) {
        AI_LOG_ERROR("VIDIOC_G_PARM failed.\n");
        return -1;
    }
    
    return 0;
}

int aiv4l2_set_parm(int fd, struct v4l2_streamparm *stream_parm)
{
    stream_parm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_S_PARM, stream_parm) < 0) {
        printf("VIDIOC_S_PARM failed.\n");
        return -1;
    }

    return 0;
}

int aiv4l2_poll(int fd)
{
    int ret;
    struct pollfd poll_fds[1];

    poll_fds[0].fd = fd;
    poll_fds[0].events = POLLIN;

    ret = poll(poll_fds, 1, 10000);
    if (ret < 0) {
        AI_LOG_ERROR("poll error.\n");
        return -1;
    } else if (ret == 0) {
        AI_LOG_ERROR("No data in 10 secs..\n");
        return -1;
    }

    return 0;
}
