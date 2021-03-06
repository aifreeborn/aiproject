#include "stm32f4xx.h"
#include "aitypes.h"
#include "aigpio.h"
#include "aidelay.h"
#include "aisys.h"
#include "ailcd.h"
#include "ailtdc.h"

/*
********************************************************************************
*                       LCD-TFT controller (LTDC)
* Ӳ���豸��
*         ALIENTEK 7" RGB TFTLCD  -> ATK-7" RGB TFTLCD-V1.3
* Ӳ���ӿڣ�
*                        RESET         -> NRST
*                        T_PEN         -> PH7
*                        T_SCK         -> PH6
*                        T_MISO        -> PG3
*                        T_MOSI        -> PI3
*                        T_CS          -> PI8
*                        LCD_BL        -> PB5
*         ����ʹ��        LCD_DE        -> PF10
*         ��ֱͬ���ź�    LCD_VSYNC     -> PI9
*         ˮƽͬ���ź�    LCD_HSYNC     -> PI10
*         ����ʱ���ź�    LCD_CLK       -> PG7
*         ��ɫ������      LCD_R[7:3]    -> PG6��PH[12:9]
*         ��ɫ������      LCD_G[7:2]    -> PI[2:0]��PH[15:13]
*         ��ɫ������      LCD_B[7:3]    -> PI[7:4]��PG11
* Ӳ�����ӣ�
*         LCD_LR - V3.3        LCD_UD - GND    LCDɨ�裺�����ң����ϵ��£�������
*         LCD_R7 - GND         LCD_G7 - V3.3
*         LCD_B7 - GND                         LCD ID:ATK-7016,1024*600
* ����ģʽ��
*         DEģʽ - ʹ��DE�ź���ȷ����Ч����,��ģʽ���Բ���HS�ź�
*         HVģʽ - ��Ҫ��ͬ���ͳ�ͬ������ʾɨ����к��У�
* ��ʾ��Ϣ��
*         ͼ���ʽRGB565
********************************************************************************
*/

#define    ATTR(x)    __attribute__((at(x)))

/* ֡�������飬ai_lcd_framebuf��LTDCһ֡ͼ����Դ��С */
#if AI_LCD_PIXEL_FMT == AI_LCD_PIXEL_FMT_ARGB8888 \
    || AI_LCD_PIXEL_FMT == AI_LCD_PIXEL_FMT_RGB888
    static u32 ai_ltdc_lcd_fb[1280][800] ATTR(AI_LCD_FRAMEBUF_ADDR);
#else
    static u16 ai_ltdc_lcd_fb[1280][800] ATTR(AI_LCD_FRAMEBUF_ADDR);
#endif

/* LCD֡��������ָ��,����ָ���Ӧ��С���ڴ�����,LTDC֧��2���� */
static u32 *ai_ltdc_lcd_fbp[2];
ai_ltdc_dev_t ai_ltdc_dev;

/*
********************************************************************************
*    Function: ai_ltdc_set_disp_dir
* Description: ����LCD����ʾ����
*       Input: direction - ��ʾ����Ĭ�Ϻ���
*                          0 - ����
*                          1 - ����
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ltdc_set_disp_dir(u8 direction)
{
    ai_ltdc_dev.dir = direction;
    
    if (direction == 0) {
        ai_ltdc_dev.width = ai_ltdc_dev.panel_height;
        ai_ltdc_dev.height = ai_ltdc_dev.panel_width;
    } else if (direction == 1) {
        ai_ltdc_dev.width = ai_ltdc_dev.panel_width;
        ai_ltdc_dev.height = ai_ltdc_dev.panel_height;
    }
}

/*
********************************************************************************
*    Function: ai_ltdc_draw_point
* Description: ��LCD��Ļ�ϻ�һ����
*       Input:     x - ��д���ĺ�����
*                  y - ��д����������
*              color - ʹ�õ���ɫֵ
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ltdc_draw_point(u16 x, u16 y, u32 color)
{
    u32 offset = 0;
    
    if (ai_ltdc_dev.dir) {
        // ����
        offset = ai_ltdc_dev.pixel_size * (ai_ltdc_dev.panel_width * y + x);
    } else {
        // ����
        offset = ai_ltdc_dev.pixel_size * (ai_ltdc_dev.panel_width
                                           * (ai_ltdc_dev.panel_height - x - 1)
                                           + y);
    }
    
#if AI_LCD_PIXEL_FMT == AI_LCD_PIXEL_FMT_ARGB8888
    || AI_LCD_PIXEL_FMT == AI_LCD_PIXEL_FMT_RGB888
    *(u32 *)((u32)ai_ltdc_lcd_fbp[ai_ltdc_dev.active_layer] + offset) = color;
#else
    *(u16 *)((u32)ai_ltdc_lcd_fbp[ai_ltdc_dev.active_layer] + offset) = color;
#endif
}

/*
********************************************************************************
*    Function: ai_ltdc_read_point
* Description: ��ȡLCD��Ļ�ϵ�һ����
*       Input:     x - ����ȡ��ĺ�����
*                  y - ����ȡ���������
*      Output: None
*      Return: 32bit����ɫֵ�����е�24bit��Ч
*      Others: None
********************************************************************************
*/
u32 ai_ltdc_read_point(u16 x, u16 y)
{
    u32 offset = 0;
    
    if (ai_ltdc_dev.dir) {
        // ����
        offset = ai_ltdc_dev.pixel_size * (ai_ltdc_dev.panel_width * y + x);
    } else {
        // ����
        offset = ai_ltdc_dev.pixel_size * (ai_ltdc_dev.panel_width
                                           * (ai_ltdc_dev.panel_height - x - 1)
                                           + y);
    }
    
#if AI_LCD_PIXEL_FMT == AI_LCD_PIXEL_FMT_ARGB8888
    || AI_LCD_PIXEL_FMT == AI_LCD_PIXEL_FMT_RGB888
    return *(u32 *)((u32)ai_ltdc_lcd_fbp[ai_ltdc_dev.active_layer] + offset);
#else
    return *(u16 *)((u32)ai_ltdc_lcd_fbp[ai_ltdc_dev.active_layer] + offset);
#endif
}

/*
********************************************************************************
*    Function: ai_ltdc_fill
* Description: LTDC������,DMA2D���
*              (sx,sy),(ex,ey)���������ζԽ����꣬
*              �����СΪ:(ex - sx + 1) * (ey - sy + 1)   
*              ע��:sx,ex,���ܴ���lcddev.width-1;sy,ey,���ܴ���lcddev.height-1
*       Input: sx, sy - Ҫ���ľ�����������ϽǺᡢ������
*              ex, ey - Ҫ���ľ�����������½Ǻᡢ������
*              color  - ���ʹ�õ���ɫֵ
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ltdc_fill(u16 sx, u16 sy, u16 ex, u16 ey, u32 color)
{
    // ��LCD���Ϊ��׼������ϵ,����������仯���仯
    u16 psx, psy, pex, pey;
    u16 line_offset = 0;
    u32 timeout = 0;
    u32 addr;
    
    if (ai_ltdc_dev.dir) {    // ����
        psx = sx;
        psy = sy;
        pex = ex;
        pey = ey;
    } else{
        psx = sy;
        psy = ai_ltdc_dev.panel_height - ex -1;
        pex = ey;
        pey = ai_ltdc_dev.panel_height - sx - 1;
    }
    line_offset = ai_ltdc_dev.panel_width - (pex - psx + 1);
    
    addr = (u32)ai_ltdc_lcd_fbp[ai_ltdc_dev.active_layer]
            + ai_ltdc_dev.pixel_size * (ai_ltdc_dev.panel_width * psy + psx);
    RCC->AHB1ENR |= 0x1 << 23;
    DMA2D->CR &= ~0x1;
    DMA2D->CR |= (u32)0x3 << 16;         // �Ĵ������洢��ģʽ
    DMA2D->OPFCCR = AI_LCD_PIXEL_FMT;    // ���PFC����ɫģʽ����
    DMA2D->OOR = line_offset;
    DMA2D->OMAR = addr;
    DMA2D->NLR = ((u32)(pex - psx + 1) << 16) | (u32)(pey - psy + 1);
    DMA2D->OCOLR = color;
    DMA2D->CR |= 0x1;
    while ((DMA2D->ISR & (0x1 << 1)) == 0) {    // �ȴ��������
        timeout++;
        if (timeout > 0x1fffff)
            break;
    }
    DMA2D->IFCR |= (0x1 << 1);
}

/*
********************************************************************************
*    Function: ai_ltdc_color_fill
* Description: ��ָ�����������ָ����ɫ��,DMA2D���
*              �˺�����֧��u16,RGB565��ʽ����ɫ�������.
*              (sx,sy),(ex,ey)���������ζԽ����꣬
*              �����СΪ:(ex - sx + 1) * (ey - sy + 1)   
*              ע��:sx,ex,���ܴ���lcddev.width-1;sy,ey,���ܴ���lcddev.height-1
*       Input: sx, sy - Ҫ���ľ�����������ϽǺᡢ������
*              ex, ey - Ҫ���ľ�����������½Ǻᡢ������
*              color  - Ҫ������ɫ�����׵�ַ
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ltdc_color_fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color)
{
    // ��LCD���Ϊ��׼������ϵ,����������仯���仯
    u16 psx, psy, pex, pey;
    u16 line_offset = 0;
    u32 timeout = 0;
    u32 addr;
    
    if (ai_ltdc_dev.dir) {    // ����
        psx = sx;
        psy = sy;
        pex = ex;
        pey = ey;
    } else{
        psx = sy;
        psy = ai_ltdc_dev.panel_height - ex -1;
        pex = ey;
        pey = ai_ltdc_dev.panel_height - sx - 1;
    }
    line_offset = ai_ltdc_dev.panel_width - (pex - psx + 1);
    
    addr = (u32)ai_ltdc_lcd_fbp[ai_ltdc_dev.active_layer]
            + ai_ltdc_dev.pixel_size * (ai_ltdc_dev.panel_width * psy + psx);
    RCC->AHB1ENR |= 0x1 << 23;
    DMA2D->CR &= ~0x1;
    DMA2D->CR &= ~((u32)0x3 << 16);      // �洢�����洢��ģʽ
    DMA2D->FGPFCCR = AI_LCD_PIXEL_FMT;    // ���PFC����ɫģʽ����
    DMA2D->FGOR = 0;                     // ǰ������ƫ��Ϊ0
    DMA2D->OOR = line_offset;
    DMA2D->FGMAR = (u32)color;           // ǰ����ͼ���������ݵĵ�ַ
    DMA2D->OMAR = addr;
    DMA2D->NLR = ((u32)(pex - psx + 1) << 16) | (u32)(pey - psy + 1);
    DMA2D->CR |= 0x1;
    while ((DMA2D->ISR & (0x1 << 1)) == 0) {    // �ȴ��������
        timeout++;
        if (timeout > 0x1fffff)
            break;
    }
    DMA2D->IFCR |= (0x1 << 1);
}
    
/*
********************************************************************************
*    Function: ai_ltdc_read_panel_id - ��ȡLCD��ID
* Description: ʵ���ϲ�û��LCD IDֵ��ȡ�ļĴ�����ֻ��Ӳ��ʹ��LCD_R/G/B[7]����ϡ���
*              ��������ʵ��ID���жϣ�
*              LCD_R[7] -> PG6        LCD_G[7] -> PI2        LCD_B[7] -> PI7
*       Input: void
*      Output: None
*      Return: 0 - ��ЧID����0ֵ - �������ЧID
*      Others: None
********************************************************************************
*/
u16 ai_ltdc_read_panel_id(void)
{
    u8 id = 0;
    u16 ret = 0;
    
    RCC->AHB1ENR |= 0x1 << 8 | 0x1 << 6;
    ai_gpio_set(GPIOG, PIN6, GPIO_MODE_IN, GPIO_OTYPE_NULL, 
                GPIO_SPEED_NULL, GPIO_PUPD_PU);
    ai_gpio_set(GPIOI, PIN7 | PIN2, GPIO_MODE_IN, GPIO_OTYPE_NULL,
                GPIO_SPEED_NULL, GPIO_PUPD_PU);
    
    // ��ӦӲ�����ӵ�M[2:0]
    id = PI_IN(7) << 2 | PI_IN(2) << 1 | PG_IN(6);
    switch (id) {
    case 0:
        ret = AI_LCD_INCH43_480_272;
        break;
    case 1:
        ret = AI_LCD_INCH7_800_480;
        break;
    case 2:
        ret = AI_LCD_INCH7_1024_600;
        break;
    case 3:
        ret = AI_LCD_INCH7_1280_800;
        break;
    case 4:
        ret = AI_LCD_INCH8_1024_600;
        break;
    default:
        ret = AI_LCD_TFT_MCU;
    };
    
    return ret;
}

/*
********************************************************************************
*    Function: ai_ltdc_set_clk - LTDCʱ��(Fdclk)���ú���
* Description: PLLSAIʱ���������£�
*              Fvco = Fsys * (n / pllm)
*              Fdclk = (Fvco / r) / (2 * 2^div)           
*              ���У�Fsys = 25MHz��pllm = 25��FvcoΪPLLSAI VCOƵ��
*       Input: 
*               n - PLLSAIʱ�ӱ�Ƶϵ��N,ȡֵ��Χ:50~432
*               r - ʱ�ӵķ�Ƶϵ��R,ȡֵ��Χ:2~7
*             div - LCDʱ�ӷ�Ƶϵ��,ȡֵ��Χ:0~3,��Ӧ��Ƶ2^(pllsaidivr+1)     
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
static int ai_ltdc_set_pllsai_clk(u32 n, u32 r, u32 div)
{
    int ret = 0;
    u16 retry = 0;
    
    if ((r < 2 || r > 7) || (n < 50 || n > 432) || div > 3)
        return -1;
    
    RCC->CR &= ~(0x1 << 28);    // ����ǰ�ȹر�PLL
    while ((RCC->CR & (0x1 << 29)) && (retry < 0x1fff))
        retry++;
    if (retry == 0x1fff) {
        ret = -1;
    } else {
        RCC->PLLSAICFGR = (RCC->PLLSAICFGR & ~(0x7 << 28 | 0x1ff << 6))
                          | r << 28 | n << 6;
        RCC->DCKCFGR = (RCC->DCKCFGR & ~(0x3 << 16)) | (div << 16);
        RCC->CR |= 0x1 << 28;
        while (((RCC->CR & (0x1 << 29)) == 0) && (retry < 0x1fff))
            retry++;
        if (retry == 0x1fff) {
            ret = -1;
        }
    }
    
    return ret;
}

/*
********************************************************************************
*    Function: ai_ltdc_switch - LTDCʹ��/�رպ���
* Description: ��������LTDC�Ŀ���
*       Input: action - ���ؿ��ƣ�ʹ��AI_LTDC_ON,AI_LTDC_OFF
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ltdc_switch(u8 action)
{
    if (action == AI_LTDC_ON)
        LTDC->GCR |= 0x1;
    else if (action == AI_LTDC_OFF)
        LTDC->GCR &= ~0x1;
}

/*
********************************************************************************
*    Function: ai_ltdc_set_layer_param - ����LTDC�Ļ�������
* Description: �˺���,������ai_ltdc_set_layer_window֮ǰ����
*       Input: layerx          - ��ֵ,0/1.
*              buf_addr        - ��x����ɫ֡����������ʼ��ַ
*              pixel_fmt       - ��x�����ظ�ʽ.
*                                 0 - ARGB8888
*                                 1 - RGB888
*                                 2 - RGB565
*                                 3 - ARGB1555
*                                 4 - ARGB4444
*                                 5 - L8
*                                 6 - AL44
*                                 7 - AL88
*              alpha            - ��alpha�����ʹ�õĺ㶨alphaֵ
*                                 0   - ȫ͸��
*                                 255 - ��͸��
*              default_alpha    - Ĭ����ɫAlphaֵ
*                                 0   - ȫ͸��
*                                 255 - ��͸��
*              color            - ��x��Ĭ����ɫ,32λ,��24λ��Ч,RGB888��ʽ
*              bfac1            - ���ϵ��1
*                                 4 - �㶨��Alpha
*                                 6 - ����Alpha * �㶨Alpha
*              bfac2            - ���ϵ��2
*                                 5 - �㶨��Alpha
*                                 7 - ����Alpha * �㶨Alpha
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_ltdc_set_layer_param(u8 layerx, u32 buf_addr, u8 pixel_fmt, u8 alpha,
                             u8 default_alpha, u32 color, u8 bfac1, u8 bfac2)
{
    if (bfac1 > 7 || bfac2 > 7)
        return -1;
    
    if (layerx == 0) {
        LTDC_Layer1->CFBAR = buf_addr;
        LTDC_Layer1->PFCR = pixel_fmt & 0x7;
        LTDC_Layer1->CACR = alpha;
        LTDC_Layer1->DCCR = ((u32)default_alpha << 24)
                            | (color & ~((u32)0xff << 24));
        LTDC_Layer1->BFCR = ((u32)bfac1 << 8) | ((u32)bfac2);
    } else {
        LTDC_Layer2->CFBAR = buf_addr;
        LTDC_Layer2->PFCR = pixel_fmt & 0x7;
        LTDC_Layer2->CACR = alpha;
        LTDC_Layer2->DCCR = ((u32)default_alpha << 24)
                            | (color & ~((u32)0xff << 24));
        LTDC_Layer2->BFCR = ((u32)bfac1 << 8) | ((u32)bfac2);
    }
    
    return 0;
}

/*
********************************************************************************
*    Function: ai_ltdc_layer_switch - ����ָ����
* Description: ����ָ���Ĳ�
*       Input: layerx - Ҫ���õĲ��
*                       0 - ��һ��
*                       1 - �ڶ���
*              action - Ҫ���õĿ�/�ض���
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ltdc_layer_switch(u8 layerx, u8 action)
{
    if (action == AI_LTDC_LAYER_ON) {
        if (layerx == 0)
            LTDC_Layer1->CR |= 0x1;
        else
            LTDC_Layer2->CR |= 0x1;
    } else if (action == AI_LTDC_LAYER_OFF) {
        if (layerx == 0)
            LTDC_Layer1->CR &= ~0x1;
        else
            LTDC_Layer2->CR &= ~0x1;    
    }
    
    // LTDCӰ���������� -> Ӱ�ӼĴ�����������
    LTDC->SRCR |= 0x1;
}

static u8 ai_ltdc_get_pixel_bytes(u8 fmt)
{
    u8 tmp = 2;        // ÿ�����ص���ֽ���
    
    switch (fmt) {
    case 0:
        tmp = 4;
        break;
    case 1:
        tmp = 3;
        break;
    case 5:
    case 6:
        tmp = 1;
        break;
    default:
        tmp = 2;
    };
    
    return tmp;
}

/*
********************************************************************************
*    Function: ai_ltdc_set_layer_window - �㴰�����ú���
* Description: �˺���������ai_ltdc_set_layer_param֮��������.����,
*              �����õĴ���ֵ���������ĳߴ�ʱ,GRAM�Ĳ���(��/д�㺯��),
*              ҲҪ���ݴ��ڵĿ���������޸�,������ʾ������(�����̾�δ���޸�).
*       Input: layerx        - ��ֵ,0/1
*              sx, sy        - ��ʼ����
*              width, height - ��Ⱥ͸߶�
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ltdc_set_layer_window(u8 layerx, u16 sx, u16 sy, u16 width, u16 height)
{
    u32 temp = 0;
    u8 pixel_fmt = 0;
    
    if (layerx == 0) {
        temp = (sx + width + ((LTDC->BPCR & 0x0FFF0000) >> 16)) << 16;
        LTDC_Layer1->WHPCR = (sx + ((LTDC->BPCR & 0x0FFF0000) >> 16) + 1) | temp;
        temp = (sy + height + (LTDC->BPCR & 0x7ff)) << 16;
        LTDC_Layer1->WVPCR = (sy + (LTDC->BPCR & 0x7ff) + 1) | temp;
        pixel_fmt = LTDC_Layer1->PFCR & 0x07;
        temp = ai_ltdc_get_pixel_bytes(pixel_fmt);
        LTDC_Layer1->CFBLR = (width * temp << 16) | (width * temp + 3);
        LTDC_Layer1->CFBLNR = height;
        
    } else {
        temp = (sx + width + ((LTDC->BPCR & 0x0FFF0000) >> 16)) << 16;
        LTDC_Layer2->WHPCR = (sx + ((LTDC->BPCR & 0x0FFF0000) >> 16) + 1) | temp;
        temp = (sy + height + (LTDC->BPCR & 0x7ff)) << 16;
        LTDC_Layer2->WVPCR = (sy + (LTDC->BPCR & 0x7ff) + 1) | temp;
        pixel_fmt = LTDC_Layer2->PFCR & 0x07;
        temp = ai_ltdc_get_pixel_bytes(pixel_fmt);
        LTDC_Layer2->CFBLR = (width * temp << 16) | (width * temp + 3);
        LTDC_Layer2->CFBLNR = height;
    }
    
    ai_ltdc_layer_switch(layerx, AI_LTDC_LAYER_ON);
}
    
/*
********************************************************************************
*    Function: ai_ltdc_select_layer - ѡ��ĳһ�㺯��
* Description: �������ѡ��ĳһ��ͼ��
*       Input: num - ��ţ���0��ʼ��ʾ��һ��
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
void ai_ltdc_select_layer(u8 num)
{
    ai_ltdc_dev.active_layer = num;
}

/*
********************************************************************************
*    Function: ai_ltdc_clear - ��������
* Description: ��ָ������ɫcolor����
*       Input: color - ��ɫֵ
*      Output: None
*      Return: void
*      Others: None
********************************************************************************
*/
void ai_ltdc_clear(u32 color)
{
    ai_ltdc_fill(0, 0, ai_ltdc_dev.width - 1, ai_ltdc_dev.height - 1, color);
}

/*
********************************************************************************
*    Function: ai_ltdc_init - LTDC��ʼ������
* Description: 
*       Input: 
*      Output: None
*      Return: On success, 0 is returned,
*              On error, -1 is returned.
*      Others: None
********************************************************************************
*/
int ai_ltdc_init(void)
{
    u16 lcd_id = 0;
    u32 tmp_reg = 0;
    
    lcd_id = ai_ltdc_read_panel_id();
    RCC->AHB1ENR |= 0x1 << 8 | 0x1 << 7 | 0x1 << 6 | 0x1 << 5 | 0x1 << 1;
    RCC->APB2ENR |= 0x1 << 26;
    
    ai_gpio_set(GPIOB, PIN5, GPIO_MODE_OUT, 
                GPIO_OTYPE_PP, GPIO_SPEED_50M, GPIO_PUPD_PU);
    ai_gpio_set(GPIOF, PIN10, GPIO_MODE_AF, 
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);   
    ai_gpio_set(GPIOG, PIN11 | PIN7 | PIN6, GPIO_MODE_AF, 
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);   
    ai_gpio_set(GPIOH, 0x7f << 9, GPIO_MODE_AF, 
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    ai_gpio_set(GPIOI, PIN10 | PIN9 | 0xf << 4 | 0x7, GPIO_MODE_AF, 
                GPIO_OTYPE_PP, GPIO_SPEED_100M, GPIO_PUPD_PU);
    
    ai_gpio_set_af(GPIOF, 10, 14);    // GPIOF
    ai_gpio_set_af(GPIOG, 6, 14);     // GPIOG
    ai_gpio_set_af(GPIOG, 7, 14);
    ai_gpio_set_af(GPIOG, 11, 14);    
    ai_gpio_set_af(GPIOH, 9, 14);     // GPIOH
    ai_gpio_set_af(GPIOH, 10, 14);
    ai_gpio_set_af(GPIOH, 11, 14);
    ai_gpio_set_af(GPIOH, 12, 14);
    ai_gpio_set_af(GPIOH, 13, 14);
    ai_gpio_set_af(GPIOH, 14, 14);
    ai_gpio_set_af(GPIOH, 15, 14);   
    ai_gpio_set_af(GPIOI, 0, 14);     // GPIOI
    ai_gpio_set_af(GPIOI, 1, 14);
    ai_gpio_set_af(GPIOI, 2, 14);
    ai_gpio_set_af(GPIOI, 4, 14);
    ai_gpio_set_af(GPIOI, 5, 14);
    ai_gpio_set_af(GPIOI, 6, 14);
    ai_gpio_set_af(GPIOI, 7, 14);
    ai_gpio_set_af(GPIOI, 9, 14);
    ai_gpio_set_af(GPIOI, 10, 14);
    
    if (lcd_id == AI_LCD_INCH43_480_272) {
        ai_ltdc_dev.panel_width = 480;
        ai_ltdc_dev.panel_height = 272;
        ai_ltdc_dev.hsw = 1;
        ai_ltdc_dev.vsw = 1;
        ai_ltdc_dev.hbp = 40;
        ai_ltdc_dev.vbp = 8;
        ai_ltdc_dev.hfp = 5;
        ai_ltdc_dev.vfp = 8;
        ai_ltdc_set_pllsai_clk(288, 4, 2);              // ��������ʱ��9MHz
    } else if (lcd_id == AI_LCD_INCH7_800_480) {
        ai_ltdc_dev.panel_width = 800;
        ai_ltdc_dev.panel_height = 480;
        ai_ltdc_dev.hsw = 1;
        ai_ltdc_dev.vsw = 1;
        ai_ltdc_dev.hbp = 46;
        ai_ltdc_dev.vbp = 23;
        ai_ltdc_dev.hfp = 210;
        ai_ltdc_dev.vfp = 22;
        ai_ltdc_set_pllsai_clk(396, 3, 1);              // ��������ʱ��33MHz
    } else if (lcd_id == AI_LCD_INCH7_1024_600) {
        ai_ltdc_dev.panel_width = 1024;
        ai_ltdc_dev.panel_height = 600;
        ai_ltdc_dev.hsw = 20;
        ai_ltdc_dev.vsw = 3;
        ai_ltdc_dev.hbp = 140;
        ai_ltdc_dev.vbp = 20;
        ai_ltdc_dev.hfp = 160;
        ai_ltdc_dev.vfp = 12;
        ai_ltdc_set_pllsai_clk(360, 2, 1);              // ��������ʱ��45MHz
    } else if (lcd_id == AI_LCD_INCH7_1280_800) {
        ai_ltdc_dev.panel_width = 1280;
        ai_ltdc_dev.panel_height = 800;
    } else if (lcd_id == AI_LCD_INCH8_1024_600) {
        ai_ltdc_dev.panel_width = 1024;
        ai_ltdc_dev.panel_height = 600;
    } else {
        return -1;
    }
    
    LTDC->GCR &= ~0xf0000000;
    tmp_reg = (ai_ltdc_dev.hsw - 1) << 16 | (ai_ltdc_dev.vsw - 1);
    LTDC->SSCR = tmp_reg;
    tmp_reg = (ai_ltdc_dev.hsw + ai_ltdc_dev.hbp - 1) << 16 
              | (ai_ltdc_dev.vsw + ai_ltdc_dev.vbp - 1);
    LTDC->BPCR = tmp_reg;
    tmp_reg = (ai_ltdc_dev.hsw + ai_ltdc_dev.hbp
               + ai_ltdc_dev.panel_width - 1) << 16
              | (ai_ltdc_dev.vsw + ai_ltdc_dev.vbp
                 + ai_ltdc_dev.panel_height - 1);
    LTDC->AWCR = tmp_reg;
    tmp_reg = (ai_ltdc_dev.hsw + ai_ltdc_dev.hbp + ai_ltdc_dev.panel_width
               + ai_ltdc_dev.hfp - 1) << 16
              | (ai_ltdc_dev.vsw + ai_ltdc_dev.vbp + ai_ltdc_dev.panel_height
                 + ai_ltdc_dev.vfp - 1);
    LTDC->TWCR = tmp_reg;
    
    // ���ñ�����ɫ
    LTDC->BCCR = AI_LTDC_BACKLAYER_COLOR;
    ai_ltdc_switch(AI_LTDC_ON);
    
#if AI_LCD_PIXEL_FMT == AI_LCD_PIXEL_FMT_ARGB8888 \
    || AI_LCD_PIXEL_FMT == AI_LCD_PIXEL_FMT_RGB888
    ai_ltdc_lcd_fbp[0] = (u32 *)&ai_ltdc_lcd_fb;
    ai_ltdc_dev.pixel_size = 4;   // ÿ������ռ4���ֽ�
#else
    ai_ltdc_lcd_fbp[0] = (u32 *)&ai_ltdc_lcd_fb;
    ai_ltdc_dev.pixel_size = 2;
#endif

    ai_ltdc_set_layer_param(0, (u32)ai_ltdc_lcd_fbp[0], AI_LCD_PIXEL_FMT,
                             255, 0,0x000000, 6, 7);
    // �㴰������,��LCD�������ϵΪ��׼,��Ҫ����޸�!
    ai_ltdc_set_layer_window(0, 0, 0, ai_ltdc_dev.panel_width,
                             ai_ltdc_dev.panel_height);
    
    ai_lcd_dev.width = ai_ltdc_dev.panel_width;
    ai_lcd_dev.height = ai_ltdc_dev.panel_height;
    ai_ltdc_select_layer(0);
    ai_lcd_backlight_on();
    ai_ltdc_clear(0xffffffff);
    
    return 0;
}

