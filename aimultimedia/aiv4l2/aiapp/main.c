/*
 * =============================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  主程序文件  
 *
 *        Version:  1.0
 *        Created:  11/19/2020 09:40:26 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  takeno (takeno2020), takeno2020@163.com
 *   Organization:  
 *
 * =============================================================================
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "ailog.h"
#include "ailibv4l2.h"

#undef AI_LOG_TAG
#define AI_LOG_TAG "aimain"

/************************************ main ************************************/
int main(int argc, char *argv[])
{
    int ret = -1;
    int fd = -1;
    FILE *fp = NULL;
    char name[64] = {0};
    struct v4l2_capability cap;
    uint32_t width = 640, height = 480;
    struct aiv4l2_buf *v4l2_buf = NULL;
    struct aiv4l2_buf_unit *v4l2_buf_unit;
    int buf_count = 4;

    if (argc != 2) {
        AI_LOG_INFO("Usage: %s </dev/videox>\n", argv[0]);
        return EXIT_FAILURE;
    }

    fd = aiv4l2_open(argv[1], O_RDWR);
    if (fd < 0) {
        AI_LOG_INFO("Err: fail to open %s.\n", argv[1]);
        return EXIT_FAILURE;
    }

    fp = fopen("pic.yuv", "w");
    if (!fp) {
        AI_LOG_INFO("Err: fail to open pic.yuv.\n");
        goto err;
    }

    memset(&cap, 0, sizeof(cap));
    ret = aiv4l2_querycap(fd, &cap);
    if (ret < 0)
        goto err;

    if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
        AI_LOG_INFO("dev support capture\n");

    if(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)
        AI_LOG_INFO("dev support output\n");

    if(cap.capabilities & V4L2_CAP_VIDEO_OVERLAY)
        AI_LOG_INFO("dev support overlay\n");

    if(cap.capabilities & V4L2_CAP_STREAMING)
        AI_LOG_INFO("dev support streaming\n");

    if(cap.capabilities & V4L2_CAP_READWRITE)
        AI_LOG_INFO("dev support read write\n");
        
    ret = aiv4l2_enuminput(fd, 0, name);
    if (ret < 0)
        goto err;
    AI_LOG_INFO("input device name: %s\n", name);

    ret = aiv4l2_set_input(fd, 0);
    if (ret < 0)
        goto err;

    ret = aiv4l2_enum_fmt(fd, V4L2_PIX_FMT_YUYV, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    if (ret < 0)
        goto err;

    ret = aiv4l2_set_fmt(fd, &width, &height, 
                         V4L2_PIX_FMT_YUYV,
                         V4L2_BUF_TYPE_VIDEO_CAPTURE);
    if (ret < 0)
        goto err;
    AI_LOG_INFO("image width: %d, height: %d\n", width, height);

    v4l2_buf = aiv4l2_requset_bufs(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, buf_count);
    if (!v4l2_buf)
        goto err;

    ret = aiv4l2_query_buf(fd, v4l2_buf);
    if (ret < 0)
        goto err;

    ret = aiv4l2_mmap(fd, v4l2_buf);
    if (ret < 0)
        goto err;

    ret = aiv4l2_qbuf_all(fd, v4l2_buf);
    if (ret < 0)
        goto err;

    ret = aiv4l2_stream_on(fd);
    if (ret < 0)
        goto err;

    ret = aiv4l2_poll(fd);
    if (ret < 0)
        goto err;

    v4l2_buf_unit = aiv4l2_dqbuf(fd, v4l2_buf);
    if (!v4l2_buf_unit)
        goto err;

    fwrite(v4l2_buf_unit->start, 1, v4l2_buf_unit->length, fp);

    ret = aiv4l2_qbuf(fd, v4l2_buf_unit);
    if (ret < 0)
        goto err;

    ret = aiv4l2_stream_off(fd);
    if (ret < 0)
        goto err;

    ret = aiv4l2_munmap(fd, v4l2_buf);
    if (ret < 0)
        goto err;

    ret = aiv4l2_release_bufs(v4l2_buf);
    if (ret < 0)
        goto err;
    

    fclose(fp);
    fp = NULL;
    aiv4l2_close(fd);
    return EXIT_SUCCESS;

err:
    if (fp) {
        fclose(fp);
        fp = NULL;
    }
    aiv4l2_close(fd);
    perror("Rrr");
    return EXIT_FAILURE;
}
