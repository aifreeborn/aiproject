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
// FLASH������ֵ
#define    AI_INFLASH_KEY1        0x45670123
#define    AI_INFLASH_KEY2        0xcdef89ab



/*
********************************************************************************
*    Function: ai_inflash_unlock
* Description: ����STM32�ڲ�FLASH
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
* Description: STM32�ڲ�FLASH����
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
* Description: �õ�FLASH״̬
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
* Description: �ȴ��������
*       Input: time -> Ҫ��ʱ�ĳ���(��λ:10us)
*      Output: None
*      Return: ͷ�ļ��ж���ĸ���FLASH״̬
*      Others: None
********************************************************************************
*/
static int ai_inflash_wait_done(u32 time)
{
    int ret = AI_FLASH_SUCCESS;
    do {
        ret = ai_inflash_get_stat();
        // ��æ,����ȴ���,ֱ���˳�.
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
* Description: ��������
*       Input: num -> �����ţ�ȡֵ��Χ[0, 11]
*      Output: None
*      Return: ͷ�ļ��ж���ĸ���FLASH״̬
*      Others: None
********************************************************************************
*/
int ai_inflash_erase_sector(u8 num)
{
    int ret = AI_FLASH_SUCCESS;
    
    if (num > 11)
        return AI_FLASH_FAILURE;
    
    ret = ai_inflash_wait_done(200000);    // �ȴ��ϴβ�������,���2s
    if (ret == AI_FLASH_SUCCESS) {
        FLASH->CR &= ~(0x3 << 8);              // ���PSIZEԭ�������ã�����Ϊ32bit
        FLASH->CR |= 0x2 << 8;                 // ��,ȷ��VCC=2.7~3.6V֮��!!
        FLASH->CR &= ~(0x1f << 3);             // ���ԭ��������,����Ҫ���������� 
        FLASH->CR |= (u32)num << 3;
        FLASH->CR |= 0x1 << 1;                 // ������������
        FLASH->CR |= 0x1 << 16;                // ������������
        ret = ai_inflash_wait_done(200000);
        if (ret != AI_FLASH_BSY)
            FLASH->CR &= ~(0x1 << 1);
    }
    
    return ret;
}

/*
********************************************************************************
*    Function: ai_inflash_wr_word
* Description: ��FLASHָ����ַдһ����
*       Input: addr -> ָ����ַ(�˵�ַ����Ϊ4�ı���!!)
*              data -> ��д�������
*      Output: None
*      Return: ͷ�ļ��ж���ĸ���FLASH״̬
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
        FLASH->CR |= 0x1;          // ����Flash���
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
* Description: ȡָ����ַ��һ����(32λ����)
*       Input: addr -> ָ����ַ(�˵�ַ����Ϊ4�ı���!!)
*      Output: None
*      Return: ���ض�ȡ������
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
* Description: ��ȡĳ����ַ���ڵ�flash����
*       Input: addr -> ָ����ַ(�˵�ַ����Ϊ4�ı���!!)
*      Output: None
*      Return: 0~11,��addr���ڵ�������
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
* Description: ��ָ����ַ��ʼд��ָ�����ȵ�����
*       Input: addr - д��flash����ʼ��ַ
*               buf - ��д������ָ��
*               len - ��(32λ)��(����Ҫд���32λ���ݵĸ���.) 
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: ��ΪSTM32F4������ʵ��̫��,û�취���ر�����������,���Ա�����д��ַ
*              �����0XFF,��ô���Ȳ������������Ҳ�������������.����д��0XFF�ĵ�ַ,
*              �����������������ݶ�ʧ.����д֮ǰȷ��������û����Ҫ����,�������������
*              �Ȳ�����,Ȼ����������д. 
*              �ú�����OTP����Ҳ��Ч!��������дOTP��!
*              OTP�����ַ��Χ:0X1FFF7800~0X1FFF7A0F
*              (ע�⣺���16�ֽڣ�����OTP���ݿ�����������д����)
********************************************************************************
*/
int ai_inflash_write(u32 addr, u32 *buf, u32 len)
{
    u32 addr_start = 0;         // д�����ʼ��ַ
    u32 addr_end = 0;           // д��Ľ�����ַ
    int stat = AI_FLASH_SUCCESS;
    u8 sector_num = 0;
    
    if (addr < AI_INFLASH_BASE || addr % 4)
        return -1;
    
    addr_start = addr;
    addr_end = addr + len * 4;
    ai_inflash_unlock();
    // FLASH�����ڼ�,�����ֹ���ݻ���
    FLASH->ACR &= ~(0x1 << 10);
    if (addr_start < 0x1FFF0000) {    // ֻ�����洢��,����Ҫִ�в�������
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
* Description: ��ָ����ַ��ʼ����ָ�����ȵ�����
*       Input: addr - flash����ʼ��ַ
*               buf - ����ָ��
*               len - ��(32λ)��(����Ҫд���32λ���ݵĸ���.) 
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
