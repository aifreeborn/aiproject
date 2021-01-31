#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aispi.h"
#include "aiw25qxx.h" 

/*
********************************************************************************
*                          PRIVATE DEFINE/FUNCTIONS
********************************************************************************
*/
// W25QXX的片选信号
#define    AI_W25QXX_CS        PF_OUT(6)

#define    ai_w25qxx_enable() \
    do { \
        AI_W25QXX_CS = 0; \
    } while (0)
#define    ai_w25qxx_disable() \
    do { \
        AI_W25QXX_CS = 1; \
    } while (0)
    
#define    AI_BUF_SIZE        4096

/*
********************************************************************************
*                          PRIVATE VARIABLES
********************************************************************************
*/
static u16 ai_chip_type = AI_W25Q256;
static u8 ai_w25qxx_buf[AI_BUF_SIZE];

/*
********************************************************************************
*    Function: ai_w25qxx_read_sr
* Description: 读取W25QXX的状态寄存器1/2/3的数据
*       Input: num - 状态寄存器号，取值为1, 2, 3
*      Output: None
*      Return: 状态寄存器值
*      Others: 状态寄存器1: BIT7    6    5    4    3    2    1    0
*                          SPR     RV   TB   BP2  BP1  BP0  WEL  BUSY
*              SPR: 默认0,状态寄存器保护位,配合WP使用
*              TB,BP2,BP1,BP0: FLASH区域写保护设置
*              WEL: 写使能锁定
*              BUSY: 忙标记位(1,忙;0,空闲)
*              默认:0x00
*              
*              状态寄存器2： BIT7    6    5    4    3    2    1    0
*                           SUS     CMP  LB3  LB2  LB1  (R)  QE   SRP1
*              状态寄存器3： BIT7      6    5    4    3    2    1    0
*                           HOLD/RST  DRV1 DRV0 (R) (R)   WPS  ADP  ADS
********************************************************************************
*/
u8 ai_w25qxx_read_sr(u8 num)
{
    u8 reg_val = 0, cmd = 0;
    
    switch (num) {
    case 1:
        cmd = AI_W25QXX_RD_SR1;
        break;
    case 2:
        cmd = AI_W25QXX_RD_SR2;
        break;
    case 3:
        cmd = AI_W25QXX_RD_SR3;
        break;
    default:
        cmd = AI_W25QXX_RD_SR1;
    }
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(cmd);
    reg_val = ai_spi5_rdwr_byte(0xff);    // 发送读取状态寄存器命令 
    ai_w25qxx_disable();
    return reg_val;
}

/*
********************************************************************************
*    Function: ai_w25qxx_write_sr
* Description: 写W25QXX状态寄存器1/2/3的数据
*       Input:  num - 状态寄存器号，取值为1, 2, 3
*              data - 待写的数据值
*      Output: None
*      Return: void
*      Others: 状态寄存器1: BIT7    6    5    4    3    2    1    0
*                          SPR     RV   TB   BP2  BP1  BP0  WEL  BUSY
*              SPR: 默认0,状态寄存器保护位,配合WP使用
*              TB,BP2,BP1,BP0: FLASH区域写保护设置
*              WEL: 写使能锁定
*              BUSY: 忙标记位(1,忙;0,空闲)
*              默认:0x00
*              
*              状态寄存器2： BIT7    6    5    4    3    2    1    0
*                           SUS     CMP  LB3  LB2  LB1  (R)  QE   SRP1
*              状态寄存器3： BIT7      6    5    4    3    2    1    0
*                           HOLD/RST  DRV1 DRV0 (R) (R)   WPS  ADP  ADS
********************************************************************************
*/
void ai_w25qxx_write_sr(u8 num, u8 data)
{
    u8 cmd = 0;
    
    switch (num) {
    case 1:
        cmd = AI_W25QXX_RD_SR1;
        break;
    case 2:
        cmd = AI_W25QXX_RD_SR2;
        break;
    case 3:
        cmd = AI_W25QXX_RD_SR3;
        break;
    default:
        cmd = AI_W25QXX_RD_SR1;
    }
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(cmd);
    ai_spi5_rdwr_byte(data);    // 发送写状态寄存器命令 
    ai_w25qxx_disable();
}
    
/*
********************************************************************************
*    Function: ai_aiw25qxx_init
* Description: SPI Flash W25QXX初始化
*       Input: void
*      Output: None
*      Return: void
*      Others: F_CS -> PF6控制Flash的片选，使用spi控制读写
********************************************************************************
*/
void ai_w25qxx_init(void)
{
    u8 tmp = 0;
    
    RCC->AHB1ENR |= 0x1 << 5;
    ai_gpio_set(GPIOF, PIN6, GPIO_MODE_OUT,
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_w25qxx_disable();
    ai_spi5_init();
    ai_spi5_set_speed(AI_SPI_SPEED_2);    // 设置为48M时钟(超频了一点点),高速模式
    ai_chip_type = ai_w25qxx_read_id();
    if (ai_chip_type == AI_W25Q256) {
        tmp = ai_w25qxx_read_sr(3);
        if ((tmp & 0x01) == 0) {          // 读取状态寄存器3，判断地址模式
            ai_w25qxx_enable();
            // 如果不是4字节地址模式,则进入4字节地址模式
            ai_spi5_rdwr_byte(AI_W25QXX_4BYTE_ADDR_EN);
            ai_w25qxx_disable();
        }
    }
}

/*
********************************************************************************
*    Function: ai_w25qxx_read_id
* Description: 读取芯片ID
*       Input: void
*      Output: None
*      Return: 0XEF13 -> 表示芯片型号为W25Q80
*              0XEF14 -> 表示芯片型号为W25Q16
*              0XEF15 -> 表示芯片型号为W25Q32
*              0XEF16 -> 表示芯片型号为W25Q64
*              0XEF17 -> 表示芯片型号为W25Q128
*              0XEF18 -> 表示芯片型号为W25Q256
*      Others: 写入0xff应该是没有特别的意义，可能就是类似于单片机双向引脚的写0xff
*              然后读取输入一个意思
********************************************************************************
*/
u16 ai_w25qxx_read_id(void)
{
    u16 ret = 0;
    
    ai_w25qxx_enable();
    // 0x90命令后面跟随24bit地址
    ai_spi5_rdwr_byte(AI_W25QXX_MANUFACT_DEV_ID);
    ai_spi5_rdwr_byte(0x00);
    ai_spi5_rdwr_byte(0x00);
    ai_spi5_rdwr_byte(0x00);
    ret = ai_spi5_rdwr_byte(0xff) << 8;
    ret |= ai_spi5_rdwr_byte(0xff);
    ai_w25qxx_disable();
    return ret;
}

/*
********************************************************************************
*    Function: ai_w25qxx_write_enable
* Description: W25QXX写使能,将WEL置位 
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_w25qxx_write_enable(void)
{
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_WR_EN);
    ai_w25qxx_disable();
}

/*
********************************************************************************
*    Function: ai_w25qxx_write_disable
* Description: W25QXX写禁止,将WEL清零
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_w25qxx_write_disable(void)
{
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_WR_DISABLE);
    ai_w25qxx_disable();
}

/*
********************************************************************************
*    Function: ai_w25qxx_wait_busy
* Description: 等待空闲
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
static void ai_w25qxx_wait_busy(void)
{
    // 等待BUSY位清空
    while ((ai_w25qxx_read_sr(1) & 0x01) == 0x01)
        ; /* Empty */
}

/*
********************************************************************************
*    Function: ai_w25qxx_read
* Description: 读取SPI FLASH,在指定地址开始读取指定长度的数据
*       Input: pbuf - 数据的存储区
*              addr - 开始读取的flash的地址，32bit
*               len - 要读取的字节数
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_w25qxx_read(u8 *pbuf, u32 addr, u16 len)
{
    u16 i = 0;
    
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_RD_DATA);
    if (ai_chip_type == AI_W25Q256) {
        // 如果是W25Q256的话地址为4字节的，要发送最高8位
        ai_spi5_rdwr_byte((u8)(addr >> 24));
    }
    ai_spi5_rdwr_byte((u8)(addr >> 16));
    ai_spi5_rdwr_byte((u8)(addr >> 8));
    ai_spi5_rdwr_byte((u8)addr);
    for (i = 0; i < len; i++) {
        pbuf[i] = ai_spi5_rdwr_byte(0xff);
    }
    ai_w25qxx_disable();
}

/*
********************************************************************************
*    Function: ai_w25qxx_write_page
* Description: SPI在一页(0~65535)内写入少于256个字节的数据
*       Input: pbuf - 数据的存储区
*              addr - 开始写入的flash的地址，32bit
*               len - 要写入的字节数,最大256字节
*      Output: None
*      Return: void
*      Others: W25Q256 将 32M 的容量分为 512 个块（Block），每个块大小为 64K 字节，
*              每个块又分为16 个扇区（Sector），每个扇区 4K 个字节。 
*              W25Q256 的最小擦除单位为一个扇区，也就是每次必须擦除 4K 个字节
********************************************************************************
*/
void ai_w25qxx_write_page(u8 *pbuf, u32 addr, u16 len)
{
    u16 i = 0;
    
    ai_w25qxx_write_enable();
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_PAGE_PROGRAM);
    if (ai_chip_type == AI_W25Q256) {
        // 如果是W25Q256的话地址为4字节的，要发送最高8位
        ai_spi5_rdwr_byte((u8)(addr >> 24));
    }
    ai_spi5_rdwr_byte((u8)(addr >> 16));
    ai_spi5_rdwr_byte((u8)(addr >> 8));
    ai_spi5_rdwr_byte((u8)addr);
    for (i = 0; i < len; i++) {
        ai_spi5_rdwr_byte(pbuf[i]);
    }
    ai_w25qxx_disable();
    ai_w25qxx_wait_busy();
}

/*
********************************************************************************
*    Function: ai_w25qxx_wr_nocheck
* Description: 无检验写SPI FLASH 
*              必须确保所写的地址范围内的数据全部为0XFF,
*              否则在非0XFF处写入的数据将失败!
*              具有自动换页功能 
*              在指定地址开始写入指定长度的数据,但是要确保地址不越界!
*       Input: pbuf - 数据的存储区
*              addr - 开始写入的flash的地址，32bit
*               len - 要写入的字节数,最大256字节
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_w25qxx_wr_nocheck(u8 *pbuf, u32 addr, u16 len)
{
    u16 page_remain = 0;
    
    // 单页剩余的字节数
    page_remain = 256 - addr % 256;
    if (len <= page_remain)
        page_remain = len;
    
    while (1) {
        ai_w25qxx_write_page(pbuf, addr, page_remain);
        if (len == page_remain) {
            break;
        } else {
            pbuf += page_remain;
            addr += page_remain;
            
            len -= page_remain;
            if (len > 256)
                page_remain = 256;    // 一次可以写入256个字节
            else
                page_remain = len;    // 不够256个字节了
        }
        
    }
}

/*
********************************************************************************
*    Function: ai_w25qxx_erase_sector
* Description: 擦除一个扇区
*       Input: addr - 扇区地址 根据实际容量设置
*      Output: None
*      Return: void
*      Others: 擦除一个扇区的最少时间:150ms
********************************************************************************
*/
static void ai_w25qxx_erase_sector(u32 addr)
{
    addr *= 4096;
    ai_w25qxx_write_enable();    // SET WEL
    ai_w25qxx_wait_busy();
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_SECTOR_ERASE);
    // 如果是W25Q256的话地址为4字节的，要发送最高8位
    if (ai_chip_type == AI_W25Q256) {
        ai_spi5_rdwr_byte((u8)(addr >> 24));
    }
    ai_spi5_rdwr_byte((u8)(addr >> 16));
    ai_spi5_rdwr_byte((u8)(addr >> 8));
    ai_spi5_rdwr_byte((u8)addr);
    ai_w25qxx_disable();
    ai_w25qxx_wait_busy();       // 等待擦除完成
}


/*
********************************************************************************
*    Function: ai_w25qxx_write
* Description: 在指定地址开始写入指定长度的数据
*       Input: pbuf - 数据的存储区
*              addr - 开始写入的flash的地址，32bit
*               len - 要写入的字节数,最大65535
*      Output: None
*      Return: void
*      Others: 该函数带擦除操作!
********************************************************************************
*/
void ai_w25qxx_write(u8 *pbuf, u32 addr, u16 len)
{
    u8 *w25qxx_buf = ai_w25qxx_buf;
    u32 sec_pos;
    u16 sec_off, sec_remain;
    u16 i;
    
    sec_pos = addr / 4096;          // 扇区地址
    sec_off = addr % 4096;          // 在扇区内的偏移
    sec_remain = 4096 - sec_off;    // 扇区剩余空间大小
    if (len <= sec_remain)          // 不大于4096个字节
        sec_remain = len;
    
    while (1) {
        // 读出整个扇区的内容
        ai_w25qxx_read(w25qxx_buf, sec_pos * 4096, 4096);
        for (i = 0; i < sec_remain; i++) {
            if (w25qxx_buf[sec_off + i] != 0xff)    // 需要擦除
                break;
        }
        if (i < sec_remain) {
            ai_w25qxx_erase_sector(sec_pos);
            for (i = 0; i < sec_remain; i++) {
                w25qxx_buf[sec_off + i] = pbuf[i];
            }
            // 写入整个扇区
            ai_w25qxx_wr_nocheck(w25qxx_buf, sec_pos * 4096, 4096);
        } else {
            ai_w25qxx_wr_nocheck(pbuf, addr, sec_remain);
        }
        // 写已经擦除了的,直接写入扇区剩余区间.
        if (len == sec_remain) {      // 写入结束了
            break;
        } else {                      // 写入未结束
            sec_pos++;
            sec_off = 0;
            
            // 修正下次写入的扇区位置、数据位置
            pbuf += sec_remain;
            addr += sec_remain;
            len -= sec_remain;
            if (len > 4096)
                sec_remain = 4096;    // 下一个扇区还是写不完
            else
                sec_remain = len;     // 下一个扇区可以写完了
        }
    }
}

/*
********************************************************************************
*    Function: ai_w25qxx_erase_chip
* Description: 擦除整个芯片
*       Input: void
*      Output: None
*      Return: void
*      Others: 擦除时间会比较长
********************************************************************************
*/
void ai_w25qxx_erase_chip(void)
{
    ai_w25qxx_write_enable();    // SET WEL
    ai_w25qxx_wait_busy();
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_CHIP_ERASE);
    ai_w25qxx_disable();
    ai_w25qxx_wait_busy();
}

/*
********************************************************************************
*    Function: ai_w25qxx_power_down
* Description: 进入掉电模式
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_w25qxx_power_down(void)
{
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_POWER_DOWN);
    ai_w25qxx_disable();
    ai_delay_us(3);                                // 等待TPD
}

/*
********************************************************************************
*    Function: ai_w25qxx_wake_up
* Description: 唤醒flash
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_w25qxx_wake_up(void)
{
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_RELEASE_POWER_DOWN);
    ai_w25qxx_disable();
    ai_delay_us(3);                                // 等待TRES1
}
