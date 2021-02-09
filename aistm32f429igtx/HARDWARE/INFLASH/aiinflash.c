#include "stm32f4xx.h"
#include "aitypes.h"
#include "aidelay.h"
#include "aigpio.h"
#include "aiinflash.h"

/*
********************************************************************************
*                           PRIVATE DEFINE
********************************************************************************
*/
// FLASH解锁键值
#define    AI_INFLASH_KEY1        0x45670123
#define    AI_INFLASH_KEY2        0xcdef89ab



/*
********************************************************************************
*    Function: ai_inflash_unlock
* Description: 解锁STM32内部FLASH
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
static void ai_inflash_unlock(void)
{
    FLASH->KEYR = AI_INFLASH_KEY1;
    FLASH->KEYR = AI_INFLASH_KEY2;
}

/*
********************************************************************************
*    Function: ai_inflash_lock
* Description: STM32内部FLASH上锁
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
static void ai_inflash_lock(void)
{
    FLASH->CR |= (u32)0x1 << 31;
}

/*
********************************************************************************
*    Function: ai_inflash_get_stat
* Description: 得到FLASH状态
*       Input: void
*      Output: None
*      Return: On success, 0 is returned,
*              On error, negative integer is returned.
*      Others: None
********************************************************************************
*/
static int ai_inflash_get_stat(void)
{
    u32 ret = AI_FLASH_SUCCESS;
    
    ret = FLASH->SR;
    if (ret & (0x1 << 16))
        return AI_FLASH_BSY;
    else if (ret & (0x1 << 7))
        return AI_FLASH_PGSERR;
    else if (ret & (0x1 << 6))
        return AI_FLASH_PGPERR;
    else if (ret & (0x1 << 5))
        return AI_FLASH_PGAERR;
    else if (ret & (0x1 << 4))
        return AI_FLASH_WRPERR;
    
    return AI_FLASH_SUCCESS;
}

/*
********************************************************************************
*    Function: ai_inflash_wait_done
* Description: 等待操作完成
*       Input: time -> 要延时的长短(单位:10us)
*      Output: None
*      Return: 头文件中定义的各个FLASH状态
*      Others: None
********************************************************************************
*/
static int ai_inflash_wait_done(u32 time)
{
    int ret = AI_FLASH_SUCCESS;
    do {
        ret = ai_inflash_get_stat();
        // 非忙,无需等待了,直接退出.
        if (ret != AI_FLASH_BSY)
            break;
        ai_delay_us(10);
        time--;
    } while (time);
    
    if (time == 0)
        ret = AI_FLASH_FAILURE;
    
    return ret;
}

/*
********************************************************************************
*    Function: ai_inflash_erase_sector
* Description: 擦除扇区
*       Input: num -> 扇区号，取值范围[0, 11]
*      Output: None
*      Return: 头文件中定义的各个FLASH状态
*      Others: None
********************************************************************************
*/
int ai_inflash_erase_sector(u8 num)
{
    int ret = AI_FLASH_SUCCESS;
    
    if (num > 11)
        return AI_FLASH_FAILURE;
    
    ret = ai_inflash_wait_done(200000);    // 等待上次操作结束,最大2s
    if (ret == AI_FLASH_SUCCESS) {
        FLASH->CR &= ~(0x3 << 8);              // 清除PSIZE原来的设置，设置为32bit
        FLASH->CR |= 0x2 << 8;                 // 宽,确保VCC=2.7~3.6V之间!!
        FLASH->CR &= ~(0x1f << 3);             // 清除原来的设置,设置要擦除的扇区 
        FLASH->CR |= (u32)num << 3;
        FLASH->CR |= 0x1 << 1;                 // 激活扇区擦除
        FLASH->CR |= 0x1 << 16;                // 启动扇区擦除
        ret = ai_inflash_wait_done(200000);
        if (ret != AI_FLASH_BSY)
            FLASH->CR &= ~(0x1 << 1);
    }
    
    return ret;
}

/*
********************************************************************************
*    Function: ai_inflash_wr_word
* Description: 在FLASH指定地址写一个字
*       Input: addr -> 指定地址(此地址必须为4的倍数!!)
*              data -> 待写入的数据
*      Output: None
*      Return: 头文件中定义的各个FLASH状态
*      Others: None
********************************************************************************
*/
int ai_inflash_wr_word(u32 addr, u32 data)
{
    int ret = AI_FLASH_SUCCESS;
        
    ret = ai_inflash_wait_done(0xff);
    if (ret == AI_FLASH_SUCCESS) {
        FLASH->CR &= ~(0x3 << 8);
        FLASH->CR |= 0x2 << 8;
        FLASH->CR |= 0x1;          // 激活Flash编程
        *(vu32 *)addr = data;
        ret = ai_inflash_wait_done(0xff);
        if (ret != AI_FLASH_BSY)
            FLASH->CR &= ~0x1;
    }
    
    return ret;
}

/*
********************************************************************************
*    Function: ai_inflash_rd_word
* Description: 取指定地址的一个字(32位数据)
*       Input: addr -> 指定地址(此地址必须为4的倍数!!)
*      Output: None
*      Return: 返回读取的数据
*      Others: None
********************************************************************************
*/
u32 ai_inflash_rd_word(u32 addr)
{
    return *((vu32 *)addr);
}

/*
********************************************************************************
*    Function: ai_inflash_get_sector_num
* Description: 获取某个地址所在的flash扇区
*       Input: addr -> 指定地址(此地址必须为4的倍数!!)
*      Output: None
*      Return: 0~11,即addr所在的扇区号
*      Others: None
********************************************************************************
*/
u8 ai_inflash_get_sector_num(u32 addr)
{
    if (addr < AI_INFLASH_ADDR_SECTOR1)
        return 0;
    else if (addr < AI_INFLASH_ADDR_SECTOR2)
        return 1;
    else if (addr < AI_INFLASH_ADDR_SECTOR3)
        return 2;
    else if (addr < AI_INFLASH_ADDR_SECTOR4)
        return 3;
    else if (addr < AI_INFLASH_ADDR_SECTOR5)
        return 4;
    else if (addr < AI_INFLASH_ADDR_SECTOR6)
        return 5;
    else if (addr < AI_INFLASH_ADDR_SECTOR7)
        return 6;
    else if (addr < AI_INFLASH_ADDR_SECTOR8)
        return 7;
    else if (addr < AI_INFLASH_ADDR_SECTOR9)
        return 8;
    else if (addr < AI_INFLASH_ADDR_SECTOR10)
        return 9;
    else if (addr < AI_INFLASH_ADDR_SECTOR11)
        return 10;
    else if (addr < AI_INFLASH_ADDR_SECTOR12)
        return 11;
    else if (addr < AI_INFLASH_ADDR_SECTOR13)
        return 12;
    else if (addr < AI_INFLASH_ADDR_SECTOR14)
        return 13;
    else if (addr < AI_INFLASH_ADDR_SECTOR15)
        return 14;
    else if (addr < AI_INFLASH_ADDR_SECTOR16)
        return 15;
    else if (addr < AI_INFLASH_ADDR_SECTOR17)
        return 16;
    else if (addr < AI_INFLASH_ADDR_SECTOR18)
        return 17;
    else if (addr < AI_INFLASH_ADDR_SECTOR19)
        return 18;
    else if (addr < AI_INFLASH_ADDR_SECTOR20)
        return 19;
    else if (addr < AI_INFLASH_ADDR_SECTOR21)
        return 20;
    else if (addr < AI_INFLASH_ADDR_SECTOR22)
        return 21;
    else if (addr < AI_INFLASH_ADDR_SECTOR23)
        return 22;
    else
        return 23;
}

/*
********************************************************************************
*    Function: ai_inflash_write
* Description: 从指定地址开始写入指定长度的数据
*       Input: addr - 写入flash的起始地址
*               buf - 待写入数据指针
*               len - 字(32位)数(就是要写入的32位数据的个数.) 
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: 因为STM32F4的扇区实在太大,没办法本地保存扇区数据,所以本函数写地址
*              如果非0XFF,那么会先擦除整个扇区且不保存扇区数据.所以写非0XFF的地址,
*              将导致整个扇区数据丢失.建议写之前确保扇区里没有重要数据,最好是整个扇区
*              先擦除了,然后慢慢往后写. 
*              该函数对OTP区域也有效!可以用来写OTP区!
*              OTP区域地址范围:0X1FFF7800~0X1FFF7A0F
*              (注意：最后16字节，用于OTP数据块锁定，别乱写！！)
********************************************************************************
*/
int ai_inflash_write(u32 addr, u32 *buf, u32 len)
{
    u32 addr_start = 0;         // 写入的起始地址
    u32 addr_end = 0;           // 写入的结束地址
    int stat = AI_FLASH_SUCCESS;
    u8 sector_num = 0;
    
    if (addr < AI_INFLASH_BASE || addr % 4)
        return -1;
    
    addr_start = addr;
    addr_end = addr + len * 4;
    ai_inflash_unlock();
    // FLASH擦除期间,必须禁止数据缓存
    FLASH->ACR &= ~(0x1 << 10);
    if (addr_start < 0x1FFF0000) {    // 只有主存储区,才需要执行擦除操作
        while (addr_start < addr_end) {
            if (ai_inflash_rd_word(addr_start) != 0xFFFFFFFF) {
                sector_num = ai_inflash_get_sector_num(addr_start);
                stat = ai_inflash_erase_sector(sector_num);
                if (stat != AI_FLASH_SUCCESS)
                    break;
            } else {
                addr_start += 4;
            }
        }
    }
    
    if (stat == AI_FLASH_SUCCESS) {
        while (addr < addr_end) {
            if (ai_inflash_wr_word(addr, *buf) != AI_FLASH_SUCCESS) {
                break;
            }
            addr += 4;
            buf++;
        }
    }
    FLASH->ACR |= 0x1 << 10;
    ai_inflash_lock();
    
    return stat;
}

/*
********************************************************************************
*    Function: ai_inflash_read
* Description: 从指定地址开始读出指定长度的数据
*       Input: addr - flash的起始地址
*               buf - 数据指针
*               len - 字(32位)数(就是要写入的32位数据的个数.) 
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_inflash_read(u32 addr, u32 *buf, u32 len)
{
    u32 i;
    
    for (i = 0; i < len; i++) {
        buf[i] = ai_inflash_rd_word(addr);
        addr += 4;
    }
}
