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
// W25QXX��Ƭѡ�ź�
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
* Description: ��ȡW25QXX��״̬�Ĵ���1/2/3������
*       Input: num - ״̬�Ĵ����ţ�ȡֵΪ1, 2, 3
*      Output: None
*      Return: ״̬�Ĵ���ֵ
*      Others: ״̬�Ĵ���1: BIT7    6    5    4    3    2    1    0
*                          SPR     RV   TB   BP2  BP1  BP0  WEL  BUSY
*              SPR: Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
*              TB,BP2,BP1,BP0: FLASH����д��������
*              WEL: дʹ������
*              BUSY: æ���λ(1,æ;0,����)
*              Ĭ��:0x00
*              
*              ״̬�Ĵ���2�� BIT7    6    5    4    3    2    1    0
*                           SUS     CMP  LB3  LB2  LB1  (R)  QE   SRP1
*              ״̬�Ĵ���3�� BIT7      6    5    4    3    2    1    0
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
    reg_val = ai_spi5_rdwr_byte(0xff);    // ���Ͷ�ȡ״̬�Ĵ������� 
    ai_w25qxx_disable();
    return reg_val;
}

/*
********************************************************************************
*    Function: ai_w25qxx_write_sr
* Description: дW25QXX״̬�Ĵ���1/2/3������
*       Input:  num - ״̬�Ĵ����ţ�ȡֵΪ1, 2, 3
*              data - ��д������ֵ
*      Output: None
*      Return: void
*      Others: ״̬�Ĵ���1: BIT7    6    5    4    3    2    1    0
*                          SPR     RV   TB   BP2  BP1  BP0  WEL  BUSY
*              SPR: Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
*              TB,BP2,BP1,BP0: FLASH����д��������
*              WEL: дʹ������
*              BUSY: æ���λ(1,æ;0,����)
*              Ĭ��:0x00
*              
*              ״̬�Ĵ���2�� BIT7    6    5    4    3    2    1    0
*                           SUS     CMP  LB3  LB2  LB1  (R)  QE   SRP1
*              ״̬�Ĵ���3�� BIT7      6    5    4    3    2    1    0
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
    ai_spi5_rdwr_byte(data);    // ����д״̬�Ĵ������� 
    ai_w25qxx_disable();
}
    
/*
********************************************************************************
*    Function: ai_aiw25qxx_init
* Description: SPI Flash W25QXX��ʼ��
*       Input: void
*      Output: None
*      Return: void
*      Others: F_CS -> PF6����Flash��Ƭѡ��ʹ��spi���ƶ�д
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
    ai_spi5_set_speed(AI_SPI_SPEED_2);    // ����Ϊ48Mʱ��(��Ƶ��һ���),����ģʽ
    ai_chip_type = ai_w25qxx_read_id();
    if (ai_chip_type == AI_W25Q256) {
        tmp = ai_w25qxx_read_sr(3);
        if ((tmp & 0x01) == 0) {          // ��ȡ״̬�Ĵ���3���жϵ�ַģʽ
            ai_w25qxx_enable();
            // �������4�ֽڵ�ַģʽ,�����4�ֽڵ�ַģʽ
            ai_spi5_rdwr_byte(AI_W25QXX_4BYTE_ADDR_EN);
            ai_w25qxx_disable();
        }
    }
}

/*
********************************************************************************
*    Function: ai_w25qxx_read_id
* Description: ��ȡоƬID
*       Input: void
*      Output: None
*      Return: 0XEF13 -> ��ʾоƬ�ͺ�ΪW25Q80
*              0XEF14 -> ��ʾоƬ�ͺ�ΪW25Q16
*              0XEF15 -> ��ʾоƬ�ͺ�ΪW25Q32
*              0XEF16 -> ��ʾоƬ�ͺ�ΪW25Q64
*              0XEF17 -> ��ʾоƬ�ͺ�ΪW25Q128
*              0XEF18 -> ��ʾоƬ�ͺ�ΪW25Q256
*      Others: д��0xffӦ����û���ر�����壬���ܾ��������ڵ�Ƭ��˫�����ŵ�д0xff
*              Ȼ���ȡ����һ����˼
********************************************************************************
*/
u16 ai_w25qxx_read_id(void)
{
    u16 ret = 0;
    
    ai_w25qxx_enable();
    // 0x90����������24bit��ַ
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
* Description: W25QXXдʹ��,��WEL��λ 
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
* Description: W25QXXд��ֹ,��WEL����
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
* Description: �ȴ�����
*       Input: void
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
static void ai_w25qxx_wait_busy(void)
{
    // �ȴ�BUSYλ���
    while ((ai_w25qxx_read_sr(1) & 0x01) == 0x01)
        ; /* Empty */
}

/*
********************************************************************************
*    Function: ai_w25qxx_read
* Description: ��ȡSPI FLASH,��ָ����ַ��ʼ��ȡָ�����ȵ�����
*       Input: pbuf - ���ݵĴ洢��
*              addr - ��ʼ��ȡ��flash�ĵ�ַ��32bit
*               len - Ҫ��ȡ���ֽ���
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
        // �����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
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
* Description: SPI��һҳ(0~65535)��д������256���ֽڵ�����
*       Input: pbuf - ���ݵĴ洢��
*              addr - ��ʼд���flash�ĵ�ַ��32bit
*               len - Ҫд����ֽ���,���256�ֽ�
*      Output: None
*      Return: void
*      Others: W25Q256 �� 32M ��������Ϊ 512 ���飨Block����ÿ�����СΪ 64K �ֽڣ�
*              ÿ�����ַ�Ϊ16 ��������Sector����ÿ������ 4K ���ֽڡ� 
*              W25Q256 ����С������λΪһ��������Ҳ����ÿ�α������ 4K ���ֽ�
********************************************************************************
*/
void ai_w25qxx_write_page(u8 *pbuf, u32 addr, u16 len)
{
    u16 i = 0;
    
    ai_w25qxx_write_enable();
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_PAGE_PROGRAM);
    if (ai_chip_type == AI_W25Q256) {
        // �����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
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
* Description: �޼���дSPI FLASH 
*              ����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,
*              �����ڷ�0XFF��д������ݽ�ʧ��!
*              �����Զ���ҳ���� 
*              ��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
*       Input: pbuf - ���ݵĴ洢��
*              addr - ��ʼд���flash�ĵ�ַ��32bit
*               len - Ҫд����ֽ���,���256�ֽ�
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_w25qxx_wr_nocheck(u8 *pbuf, u32 addr, u16 len)
{
    u16 page_remain = 0;
    
    // ��ҳʣ����ֽ���
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
                page_remain = 256;    // һ�ο���д��256���ֽ�
            else
                page_remain = len;    // ����256���ֽ���
        }
        
    }
}

/*
********************************************************************************
*    Function: ai_w25qxx_erase_sector
* Description: ����һ������
*       Input: addr - ������ַ ����ʵ����������
*      Output: None
*      Return: void
*      Others: ����һ������������ʱ��:150ms
********************************************************************************
*/
static void ai_w25qxx_erase_sector(u32 addr)
{
    addr *= 4096;
    ai_w25qxx_write_enable();    // SET WEL
    ai_w25qxx_wait_busy();
    ai_w25qxx_enable();
    ai_spi5_rdwr_byte(AI_W25QXX_SECTOR_ERASE);
    // �����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
    if (ai_chip_type == AI_W25Q256) {
        ai_spi5_rdwr_byte((u8)(addr >> 24));
    }
    ai_spi5_rdwr_byte((u8)(addr >> 16));
    ai_spi5_rdwr_byte((u8)(addr >> 8));
    ai_spi5_rdwr_byte((u8)addr);
    ai_w25qxx_disable();
    ai_w25qxx_wait_busy();       // �ȴ��������
}


/*
********************************************************************************
*    Function: ai_w25qxx_write
* Description: ��ָ����ַ��ʼд��ָ�����ȵ�����
*       Input: pbuf - ���ݵĴ洢��
*              addr - ��ʼд���flash�ĵ�ַ��32bit
*               len - Ҫд����ֽ���,���65535
*      Output: None
*      Return: void
*      Others: �ú�������������!
********************************************************************************
*/
void ai_w25qxx_write(u8 *pbuf, u32 addr, u16 len)
{
    u8 *w25qxx_buf = ai_w25qxx_buf;
    u32 sec_pos;
    u16 sec_off, sec_remain;
    u16 i;
    
    sec_pos = addr / 4096;          // ������ַ
    sec_off = addr % 4096;          // �������ڵ�ƫ��
    sec_remain = 4096 - sec_off;    // ����ʣ��ռ��С
    if (len <= sec_remain)          // ������4096���ֽ�
        sec_remain = len;
    
    while (1) {
        // ������������������
        ai_w25qxx_read(w25qxx_buf, sec_pos * 4096, 4096);
        for (i = 0; i < sec_remain; i++) {
            if (w25qxx_buf[sec_off + i] != 0xff)    // ��Ҫ����
                break;
        }
        if (i < sec_remain) {
            ai_w25qxx_erase_sector(sec_pos);
            for (i = 0; i < sec_remain; i++) {
                w25qxx_buf[sec_off + i] = pbuf[i];
            }
            // д����������
            ai_w25qxx_wr_nocheck(w25qxx_buf, sec_pos * 4096, 4096);
        } else {
            ai_w25qxx_wr_nocheck(pbuf, addr, sec_remain);
        }
        // д�Ѿ������˵�,ֱ��д������ʣ������.
        if (len == sec_remain) {      // д�������
            break;
        } else {                      // д��δ����
            sec_pos++;
            sec_off = 0;
            
            // �����´�д�������λ�á�����λ��
            pbuf += sec_remain;
            addr += sec_remain;
            len -= sec_remain;
            if (len > 4096)
                sec_remain = 4096;    // ��һ����������д����
            else
                sec_remain = len;     // ��һ����������д����
        }
    }
}

/*
********************************************************************************
*    Function: ai_w25qxx_erase_chip
* Description: ��������оƬ
*       Input: void
*      Output: None
*      Return: void
*      Others: ����ʱ���Ƚϳ�
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
* Description: �������ģʽ
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
    ai_delay_us(3);                                // �ȴ�TPD
}

/*
********************************************************************************
*    Function: ai_w25qxx_wake_up
* Description: ����flash
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
    ai_delay_us(3);                                // �ȴ�TRES1
}
