#include <stdio.h>
#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aictpi2c.h"
#include "ailcd.h"
#include "aitouch.h"
#include "aift5206.h"

/*
********************************************************************************
*                          PRIVATE DEFINE/FUNCTIONS
********************************************************************************
*/
// FT5206复位引脚
#define    AI_FT5206_RST        PI_OUT(8)
// FT5206断引脚
#define    AI_FT5206_INT        PH_IN(7)

/*
********************************************************************************
*                          PRIVATE VARIABLE
********************************************************************************
*/
static const u16 ai_ft5206_tpx_tbl[5] = {
    AI_FT5206_REG_TP1,
    AI_FT5206_REG_TP2,
    AI_FT5206_REG_TP3,
    AI_FT5206_REG_TP4,
    AI_FT5206_REG_TP5
};

/*
********************************************************************************
*    Function: ai_ft5206_init
* Description: 初始化FT5206触摸屏
*              电容触摸屏使用的控制接口：
*              T_PEN         -> PH7      CT_INT
*              T_SCK         -> PH6      CT_SCL
*              T_MISO        -> PG3      
*              T_MOSI        -> PI3      CT_SDA
*              T_CS          -> PI8      CT_RST
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_ft5206_init(void)
{
    u8 tmp[2];
    
    RCC->AHB1ENR |= 0x1 << 7;
    RCC->AHB1ENR |= 0x1 << 8;
    ai_gpio_set(GPIOH, PIN7, GPIO_MODE_IN,
                GPIO_OTYPE_NULL, GPIO_SPEED_NULL, GPIO_PUPD_PU);
    ai_gpio_set(GPIOI, PIN8, GPIO_MODE_OUT,
                GPIO_OTYPE_NULL, GPIO_SPEED_100M, GPIO_PUPD_PD);
    ai_ctp_i2c_init();
    AI_FT5206_RST = 0;    // 复位
    ai_delay_ms(20);
    AI_FT5206_RST = 1;    // 释放复位
    ai_delay_ms(50);
    tmp[0] = 0;
    ai_ft5206_wr_reg(AI_FT5206_REG_DEV_MODE, tmp, 1);     // 进入正常操作模式
    ai_ft5206_wr_reg(AI_FT5206_REG_ID_G_MODE, tmp, 1);    // 查询模式
    tmp[0] = 22;
    // 触摸有效值，22，越小越灵敏
    ai_ft5206_wr_reg(AI_FT5206_REG_ID_G_THGROUP, tmp, 1); 
    tmp[0] = 12;
    // 激活周期，不能小于12，最大14
    ai_ft5206_wr_reg(AI_FT5206_REG_ID_G_PERIODACTIVE, tmp, 1);
    // 读取版本号，参考值：0x3003
    ai_ft5206_rd_reg(AI_FT5206_REG_ID_G_LIB_VER, &tmp[0], 2);
    // 驱动库版本:0X3003/0X0001/0X0002
    if ((tmp[0] == 0x30 && tmp[1] == 0x03)
        || tmp[1] == 0x01 || tmp[1] == 0x02) {
        printf("CTP ID:%x\r\n", ((u16)tmp[0] << 8) | tmp[1]);
        return 0;
    }
    return -1;
}

/*
********************************************************************************
*    Function: ai_ft5206_wr_reg
* Description: 向FT5206写入一次数据
*       Input: reg - 起始寄存器地址
*              buf - 数据缓缓存区
*              len - 写数据长度
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_ft5206_wr_reg(u16 reg, u8 *buf, u8 len)
{
    int i = 0;
    int ret = 0;
    
    ai_ctp_i2c_start();
    ai_ctp_i2c_send_byte(AI_FT5206_CMD_WR);
    ai_ctp_i2c_wait_ack();
    ai_ctp_i2c_send_byte(reg & 0xff);          // 发送低8位地址
    ai_ctp_i2c_wait_ack();
    for (i = 0; i < len; i++) {
        ai_ctp_i2c_send_byte(buf[i]);
        ret = ai_ctp_i2c_wait_ack();
        if (ret < 0)
            break;
    }
    ai_ctp_i2c_stop();
    return ret;
}

/*
********************************************************************************
*    Function: ai_ft5206_rd_reg
* Description: 从FT5206读出一次数据
*       Input: reg - 起始寄存器地址
*              buf - 数据缓缓存区
*              len - 读数据长度
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ft5206_rd_reg(u16 reg, u8 *buf, u8 len)
{
    int i = 0;
    
    ai_ctp_i2c_start();
    ai_ctp_i2c_send_byte(AI_FT5206_CMD_WR);
    ai_ctp_i2c_wait_ack();
    ai_ctp_i2c_send_byte(reg & 0xff);
    ai_ctp_i2c_wait_ack();
    ai_ctp_i2c_start();
    ai_ctp_i2c_send_byte(AI_FT5206_CMD_RD);
    ai_ctp_i2c_wait_ack();
    for (i = 0; i < len; i++) {
        buf[i] = ai_ctp_i2c_read_byte(i == (len - 1) ? 0 : 1);
    }
    ai_ctp_i2c_stop();   
}

/*
********************************************************************************
*    Function: ai_ft5206_scan
* Description: 扫描触摸屏(采用查询方式)
*       Input: mode - 扫描模式. 0 -> 正常扫描
*      Output: None
*      Return: 当前触屏状态
*              触摸屏无触摸 -> AI_FT5206_NO_TOUCH
*              触摸屏有触摸 -> AI_FT5206_TOUCH
*      Others: None
********************************************************************************
*/
u8 ai_ft5206_scan(u8 mode)
{
    static u8 time = 0;        // 控制查询间隔,从而降低CPU占用率
    u8 ret = 0;
    u8 tmp = 0;
    u8 i = 0;
    u8 buf[4];
    
    time++;
    // 空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
    if ((time % 10) == 0 || time < 10) {
        ai_ft5206_rd_reg(AI_FT5206_REG_TD_STATUS, &mode, 1);
        if ((mode & 0xf) && ((mode & 0xf) < 6)) {
            // 将点的个数转换为1的位数,匹配tp_dev.sta定义
            tmp = 0xff << (mode & 0xf);
            ai_tp_dev.stat = (~tmp) | AI_TP_DOWN_PREV | AI_TP_DOWN;
            for (i = 0; i < AI_TP_MAX; i++) {
                // 触摸有效?
                if (ai_tp_dev.stat & (0x1 << i)) {
                    // 读取XY坐标值
                    ai_ft5206_rd_reg(ai_ft5206_tpx_tbl[i], buf, sizeof(buf));
                    if (ai_tp_dev.type & 0x01) {    // 横屏
                        ai_tp_dev.y[i] = ((u16)(buf[0] & 0x0f) << 8) | buf[1];
                        ai_tp_dev.x[i] = ((u16)(buf[2] & 0x0f) << 8) | buf[3];
                    } else {
                        ai_tp_dev.x[i] = ai_lcd_dev.width
                                      - (((u16)(buf[0] & 0x0f) << 8) + buf[1]);
                        ai_tp_dev.y[i] = ((u16)(buf[2] & 0x0f) << 8) + buf[3];
                    }
                    // 必须是contact事件，才认为有效
                    if ((buf[0] & 0xf0) != 0x80)
                        ai_tp_dev.x[i] = ai_tp_dev.y[i] = 0;
                }
            }
            
            ret = 1;
            // 读到的数据都是0,则忽略此次数据
            if (ai_tp_dev.x[0] == 0 && ai_tp_dev.y[0] == 0)
                mode = 0;
            // 触发一次,则会最少连续监测10次,从而提高命中率
            time = 0;
        }
    }
    
    // 无触摸点按下
    if ((mode & 0x1f) == 0) {
        if (ai_tp_dev.stat & AI_TP_DOWN_PREV) {    // 之前是有被按下的
            ai_tp_dev.stat &= ~(0x1 << 7);         // 标记按键松开
        } else {                                   // 之前就没有被按下
            ai_tp_dev.x[0] = 0xffff;
            ai_tp_dev.y[0] = 0xffff;
            ai_tp_dev.stat &= 0xe0;
        }
    }
    
    if (time > 240)
        time = 10;
    return ret;
}

