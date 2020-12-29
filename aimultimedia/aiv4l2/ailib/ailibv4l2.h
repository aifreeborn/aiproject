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

struct aiv4l2_buf_unit {
         int index;
        void *start;
    uint32_t length;
    uint32_t offset;
};

struct aiv4l2_buf {
    struct aiv4l2_buf_unit *buf;
    int count;
    enum v4l2_buf_type type;
};

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
 *    Function: aiv4l2_querycap
 * Description: 查询v4l2设备的功能.
 *       Input: fd   - aiv4l2_open返回的设备文件描述符
 *              argp - Pointer to struct v4l2_capability.
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_querycap(int fd, struct v4l2_capability *argp);

/*******************************************************************************
 *    Function: aiv4l2_enuminput
 * Description: 枚举输入设备.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符
 *              index - 输入设备的索引
 *      Output: name  - Name of the video input, a NUL-terminated ASCII string.
 *                      必须是一个buffer，并至少32字节.
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_enuminput(int fd, int index, char *name);

/*******************************************************************************
 *    Function: aiv4l2_set_input
 * Description: 设置输入设备.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符
 *              index - 输入设备的索引
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_set_input(int fd, int index);

/*******************************************************************************
 *    Function: aiv4l2_enum_fmt
 * Description: 枚举设备支持的格式，判断是否支持指定type的格式.
 *       Input: fd   - aiv4l2_open返回的设备文件描述符;
 *              fmt  - 图像格式标识;
 *              type - Type of the data stream, set by the application.
 *      Output: None
 *      Return: 不支持返回-1；支持返回0，and the errno variable is
 *              set appropriately.
 *      Others:
 *              fmt:
 *                  V4L2_PIX_FMT_RGB565
 *                  V4L2_PIX_FMT_RGB32
 *                  V4L2_PIX_FMT_YUYV
 *                  V4L2_PIX_FMT_UYVY
 *                  V4L2_PIX_FMT_VYUY
 *                  V4L2_PIX_FMT_YVYU
 *                  V4L2_PIX_FMT_YUV422P
 *                  V4L2_PIX_FMT_NV12
 *                  V4L2_PIX_FMT_NV12T
 *                  V4L2_PIX_FMT_NV21
 *                  V4L2_PIX_FMT_NV16
 *                  V4L2_PIX_FMT_NV61
 *                  V4L2_PIX_FMT_YUV420
 *                  V4L2_PIX_FMT_JPEG
 *              type仅支持下面的格式：
 *                  V4L2_BUF_TYPE_VIDEO_CAPTURE
 *                  V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
 *                  V4L2_BUF_TYPE_VIDEO_OUTPUT
 *                  V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
 *                  V4L2_BUF_TYPE_VIDEO_OVERLAY
 *                  V4L2_BUF_TYPE_SDR_CAPTURE
 *                  V4L2_BUF_TYPE_SDR_OUTPUT 
 *                  V4L2_BUF_TYPE_META_CAPTURE
 *                  V4L2_BUF_TYPE_META_OUTPUT
 ******************************************************************************/
int aiv4l2_enum_fmt(int fd, uint32_t fmt, enum v4l2_buf_type type);

/*******************************************************************************
 *    Function: aiv4l2_set_fmt
 * Description: 设置图像格式.
 *       Input: fd     - aiv4l2_open返回的设备文件描述符;
 *              width  - 图像的宽度;
 *              height - 图像的高度;
 *              fmt    - 图像像素格式;
 *              type   - 图像数据流类型;
 *      Output: width  - 修改过后的图像宽度;
 *              height - 修改过后的图像高度;
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_set_fmt(int fd, uint32_t *width, uint32_t *height, uint32_t fmt, 
                   enum v4l2_buf_type type);

/*******************************************************************************
 *    Function: aiv4l2_requset_bufs
 * Description: 申请缓存.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符;
 *              type  - 缓存类型;
 *              count - 缓存数量；
 *      Output: None
 *      Return: 失败返回NULL；成功返回struct aiv4l2_buf类型指针.
 *      Others:
 ******************************************************************************/
struct aiv4l2_buf *aiv4l2_requset_bufs(int fd, enum v4l2_buf_type type, int count);

/*******************************************************************************
 *    Function: aiv4l2_release_bufs 
 * Description: 释放申请的缓存.
 *       Input: buf - aiv4l2_requset_bufs申请的缓存指针
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_release_bufs(struct aiv4l2_buf *buf);

/*******************************************************************************
 *    Function: aiv4l2_query_buf 
 * Description: 查询缓存信息.
 *       Input: fd  - aiv4l2_open返回的设备文件描述符
 *              buf - aiv4l2_requset_bufs申请的缓存指针 
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others: 把VIDIOC_REQBUFS中分配的数据缓存转换成物理地址 
 ******************************************************************************/
int aiv4l2_query_buf(int fd, struct aiv4l2_buf *buf);

/*******************************************************************************
 *    Function: aiv4l2_mmap 
 * Description: 映射缓存.
 *       Input: fd  - aiv4l2_open返回的设备文件描述符
 *              buf - aiv4l2_requset_bufs申请的缓存指针 
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_mmap(int fd, struct aiv4l2_buf *buf);

/*******************************************************************************
 *    Function: aiv4l2_munmap
 * Description: 取消内存映射.
 *       Input: fd  - aiv4l2_open返回的设备文件描述符
 *              buf - aiv4l2_requset_bufs申请的缓存指针 
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_munmap(int fd, struct aiv4l2_buf *buf);

/*******************************************************************************
 *    Function: aiv4l2_stream_on
 * Description: 开始采集流.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_stream_on(int fd);

/*******************************************************************************
 *    Function: aiv4l2_stream_off 
 * Description: 停止采集流.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_stream_off(int fd);

/*******************************************************************************
 *    Function: aiv4l2_qbuf 
 * Description: 缓存入队列.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符
 *              buf   - 缓存入队列；
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_qbuf(int fd, struct aiv4l2_buf_unit* buf);

/*******************************************************************************
 *    Function: aiv4l2_qbuf_all 
 * Description: 所有缓存入队列.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符
 *              buf   - 缓存
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_qbuf_all(int fd, struct aiv4l2_buf* buf);

/*******************************************************************************
 *    Function: aiv4l2_dqbuf
 * Description: 缓存出队列.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符
 *              buf   - 缓存;
 *      Output: None
 *      Return: 失败返回NULL，成功返回buf单元.
 *      Others:
 ******************************************************************************/
struct aiv4l2_buf_unit* aiv4l2_dqbuf(int fd, struct aiv4l2_buf* buf);

/*******************************************************************************
 *    Function: aiv4l2_get_ctrl 
 * Description: 获取指定control信息.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符
 *              id    - control id;
 *      Output: None
 *      Return: 成功返回control id对应的value；失败返回-1.
 *      Others:
 ******************************************************************************/
int aiv4l2_get_ctrl(int fd, uint32_t id);

/*******************************************************************************
 *    Function: aiv4l2_set_ctrl
 * Description: 设置control id对应的value值.
 *       Input: fd    - aiv4l2_open返回的设备文件描述符
 *              id    - control id;
 *              value - control id对应的value值.
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_set_ctrl(int fd, uint32_t id, int32_t value);

/*******************************************************************************
 *    Function: aiv4l2_get_parm
 * Description: 获取流参数信息.
 *       Input: fd          - aiv4l2_open返回的设备文件描述符
 *              stream_parm - 要设置的参数数据
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_get_parm(int fd, struct v4l2_streamparm *stream_parm);

/*******************************************************************************
 *    Function: aiv4l2_set_parm 
 * Description: 设置流参数.
 *       Input: fd          - aiv4l2_open返回的设备文件描述符
 *              stream_parm - 要设置的参数数据
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_set_parm(int fd, struct v4l2_streamparm *stream_parm);

/*******************************************************************************
 *    Function: aiv4l2_poll
 * Description: 等待缓存就绪.
 *       Input: fd - aiv4l2_open返回的设备文件描述符
 *      Output: None
 *      Return: On success 0 is returned, on error -1 and the errno variable is
 *              set appropriately.
 *      Others:
 ******************************************************************************/
int aiv4l2_poll(int fd);

#endif /* AI_LIB_V4L2_H_ */
