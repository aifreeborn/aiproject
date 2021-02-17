#include "aiusb.h"
#include <aiuart.h>
#include <aipdiusbd12.h>

uint8 ai_usb_ep1_in_is_busy;

idata uint8 ai_buffer[16];
// USB设备请求的各字段
uint8     ai_request_type;
uint8     ai_request;
uint16    ai_value;
uint16    ai_index;
uint16    ai_length;
// 当前发送数据的位置
uint8     *ai_send_data;
// 需要发送数据的长度
uint16    ai_send_length;
/* 
 * 是否需要发送0数据包的标志。在USB控制传输的数据过程中，
 * 当返回的数据包字节数少于最大包长时，会认为数据过程结束。
 * 当请求的字节数比实际需要返回的字节数长，而实际返回的字节
 * 数又刚好是端点0大小的整数倍时，就需要返回一个0长度的数据包
 * 来结束数据过程。因此这里增加一个标志，供程序决定是否需要返回
 * 一个0长度的数据包。
 */
uint8    ai_need_zero_packet;

// 当前的配置值,只有在设置非0配置后
uint8    ai_config_value;

// 端点1缓冲是否忙的标志。当缓冲区中有数据时，该标志为真。
// 当缓冲区中空闲时，该标志为假。
uint8    ai_ep1_in_is_busy;

/*******************************************************************************
* 设备描述符定义结构
*******************************************************************************/
static code uint8 ai_device_desc[18] = {
    0x12,    // bLength字段: 设备描述符的长度为18(0x12)字节
    0x01,    // bDescriptorType字段: 设备描述符的编号为0x01
    0x10,    // bcdUSB字段: 这里设置版本为USB1.1，即0x0110.
    0x01,    // 由于是小端结构，所以低字节在先，即0x10，0x01.
    0x00,    // bDeviceClass字段: 我们不在设备描述符中定义设备类，
             // 而在接口描述符中定义设备类，所以该字段的值为0。
    0x00,    // bDeviceSubClass字段: bDeviceClass字段为0时，该字段也为0。
    0x00,    // bDeviceProtocol字段: bDeviceClass字段为0时，该字段也为0。
    0x10,    // bMaxPacketSize0字段：PDIUSBD12的端点0大小的16字节。
    /* idVender字段。厂商ID号，我们这里取0x8888，仅供实验用。
       实际产品不能随便使用厂商ID号，必须跟USB协会申请厂商ID号。
       注意小端模式，低字节在先。*/
    0x88,
    0x88,
    /* idProduct字段。产品ID号，由于是第一个实验，我们这里取0x0001。
       注意小端模式，低字节应该在前。*/
    0x01,
    0x00,
    /* bcdDevice字段。我们这个USB鼠标刚开始做，就叫它1.0版吧，即0x0100。
       小端模式，低字节在先。*/
    0x00,
    0x01,
    /* iManufacturer字段。厂商字符串的索引值，为了方便记忆和管理，
       字符串索引就从1开始吧。*/
    0x01,
    /* iProduct字段。产品字符串的索引值。刚刚用了1，这里就取2吧。
       注意字符串索引值不要使用相同的值。*/
    0x02,
    0x03,     // iSerialNumber字段。设备的序列号字符串索引值。这里取3就可以了。
    /* bNumConfigurations字段。该设备所具有的配置数。
       我们只需要一种配置就行了，因此该值设置为1。*/
    0x01
};

/*******************************************************************************
* Function：延时n毫秒函数
* Input   : n -- 延时的毫秒数
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_delay_nms(uint16 n)
{
    uint16 i;
    uint16 j;

    for (i = 0; i < n; i++)
        for (j = 0; j < 227; j++)
            /* empty statement */
            ;
}

/*******************************************************************************
* Function：USB断开连接函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_disconnect(void)
{
    ai_uart_send_str("断开USB连接.\r\n");
    ai_d12_write_cmd(AI_D12_SET_MODE);
    ai_d12_write_byte(0x06);           // 设置模式的第一个字节
    ai_d12_write_byte(0x47);           // 设置模式的第二个字节
    ai_delay_nms(1000);
}

/*******************************************************************************
* Function：USB连接函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_connect(void)
{
    ai_uart_send_str("USB连接.\r\n");
    ai_d12_write_cmd(AI_D12_SET_MODE);
    ai_d12_write_byte(0x16);           // 设置模式的第一个字节
    ai_d12_write_byte(0x47);           // 设置模式的第二个字节
}

/*******************************************************************************
* Function：USB总线挂起中断处理函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_bus_suspend(void)
{
    ai_uart_send_str("USB总线挂起\r\n");
}

/*******************************************************************************
* Function：USB总线复位中断处理函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_bus_reset(void)
{
    ai_uart_send_str("USB总线复位\r\n");
    // 复位后端点1输入缓冲区空闲
    ai_usb_ep1_in_is_busy = 0;
}

/*******************************************************************************
* Function：根据ai_send_data和ai_send_length将数据发送到端点0的函数。
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
static void ai_usb_ep0_send_data(void)
{
    // 将数据写到端点中去,准备发送写之前要先判断一下需要发送的数据是否比端点0
    // 最大长度大，如果超过端点大小，则一次只能发送最大包长的数据。
    // 端点0的最大包长在ai_device_desc[7]
    if (ai_send_length > ai_device_desc[7]) {
        ai_d12_write_endp_buf(1, ai_device_desc[7], ai_send_data);
        ai_send_length -= ai_device_desc[7];
        ai_send_data += ai_device_desc[7];
    } else {
        if (ai_send_length != 0) {
            ai_d12_write_endp_buf(1, ai_send_length, ai_send_data);
            ai_send_length = 0;
        } else {
            if (ai_need_zero_packet == 1) {
                ai_d12_write_endp_buf(1, 0, ai_send_data);
                ai_need_zero_packet = 0;
            }
        }
    }
}

/*******************************************************************************
* Function：端点0输出中断处理函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep0_out(void)
{
    ai_uart_send_str("USB端点0输出中断.\r\n");

    // 判断是否是建立包
    if (ai_d12_read_endp_last_stat(0) & 0x20) {
        ai_d12_read_end_point_buf(0, 16, ai_buffer);
        ai_d12_ack_setup();
        ai_d12_clear_buf();

        ai_request_type = ai_buffer[0];
        ai_request = ai_buffer[1];
        ai_value = ai_buffer[2] + (((uint16)ai_buffer[3]) << 8);
        ai_index = ai_buffer[4] + (((uint16)ai_buffer[5]) << 8);
        ai_length = ai_buffer[6] + (((uint16)ai_buffer[7]) << 8);

        // 根据不同的请求进行相关操作
        if ((ai_request_type & 0x80) == 0x80) {
            // 设备到主机
            switch ((ai_request_type >> 5) & 0x03) {
            case 0:    // 标准请求
                ai_uart_send_str("USB标准输入请求: ");
                switch (ai_request) {
                case AIUSB_GET_STATUS:
                    ai_uart_send_str("获取状态.\r\n");
                    break;
                case AIUSB_GET_DESCRIPTOR:
                    ai_uart_send_str("获取描述符--");
                    // ai_value的第一个字节(低字节)是索引号；
                    // 第二个字节是描述符的类型编号
                    switch ((ai_value >> 8) & 0xff) {
                    case AIUSB_DEVICE_DESCRIPTOR:
                        ai_uart_send_str("设备描述符.\r\n");
                        ai_send_data = ai_device_desc;
                        // 判断请求的字节数是否比实际需要发送的字节数多
                        // 这里请求的是设备描述符，因此数据长度就是
                        // ai_device_desc[0]。如果请求的比实际的长，
                        // 那么只返回实际长度的数据
                        if (ai_length > ai_device_desc[0]) {
                            ai_send_length = ai_device_desc[0];
                            if (ai_send_length % ai_device_desc[7] == 0)
                                ai_need_zero_packet = 1;
                        } else {
                            ai_send_length = ai_length;
                        }
                        ai_usb_ep0_send_data();
                        break;
                    case AIUSB_CONFIGURATION_DESCRIPTOR:
                        ai_uart_send_str("配置描述符.\r\n");
                        break;
                    case AIUSB_STRING_DESCRIPTOR:
                        ai_uart_send_str("字符串描述符.\r\n");
                        break;
                    case AIUSB_INTERFACE_DESCRIPTOR:
                        break;
                    case AIUSB_ENDPOINT_DESCRIPTOR:
                        break;
                    default:
                        ai_uart_send_str("其它描述符, 描述符代码:");
                        ai_uart_print_hex((ai_value >> 8) & 0xff);
                        ai_uart_send_str("\r\n");
                    }
                    break;
                case AIUSB_GET_CONFIGURATION:
                    ai_uart_send_str("获取配置.\r\n");
                    break;
                case AIUSB_GET_INTERFACE:
                    ai_uart_send_str("获取接口.\r\n");
                    break;
                case AIUSB_SYNCH_FRAME:
                    ai_uart_send_str("同步帧.\r\n");
                    break;
                default:
                    ai_uart_send_str("错误：未定义的标准输入请求.\r\n");
                }
                break;
            case 1:
                ai_uart_send_str("USB类输入请求: \r\n");
                break;
            case 2:
                ai_uart_send_str("USB厂商输入请求: \r\n");
                break;
            default:
                ai_uart_send_str("错误：未定义的输入请求.\r\n");
            }
        } else {
            // 主机到设备
            switch ((ai_request_type >> 5) & 0x03) {
            case 0:
                ai_uart_send_str("USB标准输出请求: ");
                switch (ai_request) {
                case AIUSB_CLEAR_FEATURE:
                    ai_uart_send_str("清除特性.\r\n");
                    break;
                case AIUSB_SET_FEATURE:
                    ai_uart_send_str("设置特性.\r\n");
                    break;
                case AIUSB_SET_ADDRESS:
                    ai_uart_send_str("设置地址，地址为：");
                    ai_uart_print_hex(ai_value & 0xff);
                    ai_uart_send_str("\r\n");
                    ai_d12_set_addr(ai_value & 0xff);
                    // 设置地址没有数据过程，直接进入到状态过程，
                    // 返回一个0长度的数据包
                    ai_send_length = 0;
                    ai_need_zero_packet = 1;
                    ai_usb_ep0_send_data();
                    break;
                case AIUSB_SET_DESCRIPTOR:
                    ai_uart_send_str("设置描述符.\r\n");
                    break;
                case AIUSB_SET_CONFIGURATION:
                    ai_uart_send_str("设置配置.\r\n");
                    break;
                case AIUSB_SET_INTERFACE:
                    ai_uart_send_str("设置接口.\r\n");
                    break;
                default:
                    ai_uart_send_str("错误：未定义的标准输出请求.\r\n");
                }
                break;
            case 1:
                ai_uart_send_str("USB类输出请求: \r\n");
                break;
            case 2:
                ai_uart_send_str("USB厂商输出请求: \r\n");
                break;
            default:
                ai_uart_send_str("错误：未定义的输出请求.\r\n");
            }
        }
    } else {
        ai_d12_read_end_point_buf(0, 16, ai_buffer);
        ai_d12_clear_buf(); 
    }
}

/*******************************************************************************
* Function：端点0输入中断处理函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep0_in(void)
{
    ai_uart_send_str("USB端点0输入中断.\r\n");
    ai_d12_read_endp_last_stat(1);
    ai_usb_ep0_send_data();
}

/*******************************************************************************
* Function：端点1输出中断处理函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep1_out(void)
{
    ai_uart_send_str("USB端点1输出中断.\r\n");
}

/*******************************************************************************
* Function：端点1输入中断处理函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep1_in(void)
{
    ai_uart_send_str("USB端点1输入中断.\r\n");
}

/*******************************************************************************
* Function：端点2输出中断处理函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep2_out(void)
{
    ai_uart_send_str("USB端点2输出中断.\r\n");
}

/*******************************************************************************
* Function：端点2输入中断处理函数
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
void ai_usb_ep2_in(void)
{
    ai_uart_send_str("USB端点2输入中断.\r\n");
}