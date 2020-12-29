/*
 * =============================================================================
 *
 *       Filename:  main_1.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/30/2020 11:27:48 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  takeno (takeno2020), takeno2020@163.com
 *   Organization:  
 *
 * =============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <sys/time.h>

#define    AI_VIDEO_DEV       "/dev/video0"
#define    AI_IMAGE_FILE      "./out/img/demo"
#define    AI_IMAGE_WIDTH     640
#define    AI_IMAGE_HEIGHT    480
#define    AI_FRAME_NUM       4

struct ai_buffer {
    void *start;
    unsigned int length;
    long long int time_stamp;
};

int fd;
struct v4l2_buffer v4l2_buf;
struct ai_buffer *ai_buffers;

int aiv4l2_init();
int aiv4l2_mem_ops();
int aiv4l2_frame_process();
int aiv4l2_release();

/************************************ main ************************************/
int main(int argc, char *argv[])
{
    printf("begin...\n");
    // sleep(5);

    printf("#################### init #####################\n");
    aiv4l2_init();
    // sleep(5);

    printf("#################### malloc ###################\n");
    aiv4l2_mem_ops();
    // sleep(5);

    printf("################### process ###################\n");
    aiv4l2_frame_process();
    // sleep(5);

    printf("################### release ###################\n");
    aiv4l2_release();
    // sleep(5);
    printf("end...\n");
    return EXIT_SUCCESS;
}

int aiv4l2_init()
{
    struct v4l2_capability cap;
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_format fmt_test;
    struct v4l2_format fmt;
    struct v4l2_streamparm stream_parm;
    
    fd = open(AI_VIDEO_DEV, O_RDWR);
    if (fd < 0) {
        printf("Error open "AI_VIDEO_DEV"\n");
        return -1;
    }

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        printf("Error opening device %s: unable to query device.\n",
                AI_VIDEO_DEV);
        return -1;
    } else {
        printf("driver:\t\t%s\n", cap.driver);
        printf("card:\t\t%s\n", cap.card);
        printf("bus_info:\t%s\n", cap.bus_info);
        printf("version:\t%d\n", cap.version);
        printf("capabilities:\t%x\n", cap.capabilities);

        if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
            printf("Device %s: supports capture.\n", AI_VIDEO_DEV);
        }

        if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING) {
            printf("Device %s: supports streaming.\n", AI_VIDEO_DEV);
        }
    }

    // 显示所有支持帧格式
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Support format: \n");
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
        printf("\t%d.%s\n", fmtdesc.index + 1, fmtdesc.description);
        fmtdesc.index++;
    }

    // 检查是否支持某帧格式
    fmt_test.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt_test.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
    if (ioctl(fd, VIDIOC_TRY_FMT, &fmt_test) == -1) {
        printf("not support format RGB32!\n");
    } else {
        printf("support format RGB32.\n");
    }

    // 查看及设置当前格式
    printf("################### set fmt ###################\n");
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
    fmt.fmt.pix.height = AI_IMAGE_HEIGHT;
    fmt.fmt.pix.width = AI_IMAGE_WIDTH;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    printf("fmt.type:\t\t%d\n", fmt.type);
    printf("pix.pixelformat:\t%c%c%c%c\n",
            fmt.fmt.pix.pixelformat & 0xFF,
            (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    printf("pix.height:\t\t%d\n", fmt.fmt.pix.height);
    printf("pix.width:\t\t%d\n", fmt.fmt.pix.width);
    printf("pix.field:\t\t%d\n", fmt.fmt.pix.field);
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        printf("Unable to set format\n");
        return -1;
    }

    printf("################### get fmt ###################\n");
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1) {
        printf("Unable to get format\n");
        return -1;
    } else {
        printf("fmt.type:\t\t%d\n",fmt.type);
        printf("pix.pixelformat:\t%c%c%c%c\n",
                fmt.fmt.pix.pixelformat & 0xFF,
                (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
                (fmt.fmt.pix.pixelformat >> 16) & 0xFF, 
                (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
        printf("pix.height:\t\t%d\n",fmt.fmt.pix.height);
        printf("pix.width:\t\t%d\n",fmt.fmt.pix.width);
        printf("pix.field:\t\t%d\n",fmt.fmt.pix.field);
    }

    // 设置及查看帧速率，这里只能是30帧，就是1秒采集30张图
    memset(&stream_parm, 0, sizeof(struct v4l2_streamparm));
    stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_parm.parm.capture.timeperframe.denominator = 30;
    stream_parm.parm.capture.timeperframe.numerator = 1;
    if (ioctl(fd, VIDIOC_S_PARM, &stream_parm) == -1) {
        printf("Unable to set frame rate\n");
        return -1;
    }
    if(ioctl(fd, VIDIOC_G_PARM, &stream_parm) == -1) {
        printf("Unable to get frame rate\n");
        return -1;
    } else {
        printf("numerator:%d\ndenominator:%d\n",
                stream_parm.parm.capture.timeperframe.numerator,
                stream_parm.parm.capture.timeperframe.denominator);
    }

    return 0;
}

int aiv4l2_mem_ops()
{
    unsigned int buf_cnt;
    struct v4l2_requestbuffers req;

    
    //申请帧缓冲
    req.count = AI_FRAME_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        printf("request for buffers error\n");
        return -1;
    }

    // 申请用户空间的地址列
    ai_buffers = malloc(req.count * sizeof(*ai_buffers));
    if (!ai_buffers) {
        printf ("out of memory!\n");
        return -1;
    }
    
    // 进行内存映射
    for (buf_cnt = 0; buf_cnt < AI_FRAME_NUM; buf_cnt++) {
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        v4l2_buf.index = buf_cnt;
        //查询
        if (ioctl (fd, VIDIOC_QUERYBUF, &v4l2_buf) < 0) {
            printf("query buffer error\n");
            return -1;
        }

        //映射
        ai_buffers[buf_cnt].length = v4l2_buf.length;
        ai_buffers[buf_cnt].start = mmap(NULL, v4l2_buf.length,
                                         PROT_READ | PROT_WRITE,
                                         MAP_SHARED, fd,
                                         v4l2_buf.m.offset);
        if (ai_buffers[buf_cnt].start == MAP_FAILED) {
            printf("buffer map error\n");
            return -1;
        }
    }

    return 0;
}

int aiv4l2_frame_process()
{
    unsigned int buf_cnt;
    enum v4l2_buf_type type;
    char file_name[64] = {0};
    char index_str[10] = {0};
    int loop = 0;
    long long int extra_time = 0;
    long long int cur_time = 0;
    long long int last_time = 0;

    // 入队和开始采集
    for (buf_cnt = 0; buf_cnt < AI_FRAME_NUM; buf_cnt++) {
        v4l2_buf.index = buf_cnt;
        ioctl(fd, VIDIOC_QBUF, &v4l2_buf);
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);

    // 出队，处理，写入yuv文件，入队，循环进行
    while ( loop < 15) {
        for(buf_cnt = 0; buf_cnt < AI_FRAME_NUM; buf_cnt++) {
            //出队
            v4l2_buf.index = buf_cnt;
            ioctl(fd, VIDIOC_DQBUF, &v4l2_buf);

            //查看采集数据的时间戳之差，单位为微妙
            ai_buffers[buf_cnt].time_stamp = v4l2_buf.timestamp.tv_sec * 1000000
                                             + v4l2_buf.timestamp.tv_usec;
            cur_time = ai_buffers[buf_cnt].time_stamp;
            extra_time = cur_time - last_time;
            last_time = cur_time;
            printf("time_deta:%lld\n\n", extra_time);
            printf("buf_len:%d\n", ai_buffers[buf_cnt].length);

            //处理数据只是简单写入文件，名字以loop的次数和帧缓冲数目有关
            printf("grab image data OK\n");
            memset(file_name, 0, sizeof(file_name));
            memset(index_str, 0, sizeof(index_str));
            sprintf(index_str, "%d", loop * 4 + buf_cnt);
            strcpy(file_name, AI_IMAGE_FILE);
            strcat(file_name, index_str);
            strcat(file_name, ".jpg");
            FILE *fp2 = fopen(file_name, "wb");
            if (!fp2) {
                printf("open %s error\n", file_name);
                return -1;
            }
            fwrite(ai_buffers[buf_cnt].start, AI_IMAGE_WIDTH * AI_IMAGE_HEIGHT * 2, 1, fp2);
            fclose(fp2);
            printf("save %s OK\n", file_name);

            //入队循环
            ioctl(fd, VIDIOC_QBUF, &v4l2_buf);       
        }

        loop++;
    }
    return 0;
}

int aiv4l2_release()
{
    unsigned int buf_cnt;
    enum v4l2_buf_type type;

    //关闭流
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);
    
    //关闭内存映射
    for (buf_cnt = 0; buf_cnt < AI_FRAME_NUM; buf_cnt++) {
        munmap(ai_buffers[buf_cnt].start, ai_buffers[buf_cnt].length);
    }
    
    //释放自己申请的内存
    free(ai_buffers);
    ai_buffers = NULL;
    
    //关闭设备
    close(fd);
    return 0;
}
