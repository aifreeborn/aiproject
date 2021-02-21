#include "aiusb.h"
#include <aiuart.h>
#include <aipdiusbd12.h>
#include "aitype.h"
#include "aileds.h"

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
 * 是否需要发送0数据包的标志.在USB控制传输的数据过程中,
 * 当返回的数据包字节数少于最大包长时,会认为数据过程结束.
 * 当请求的字节数比实际需要返回的字节数长,而实际返回的字节
 * 数又刚好是端点0大小的整数倍时,就需要返回一个0长度的数据包
 * 来结束数据过程.因此这里增加一个标志,供程序决定是否需要返回
 * 一个0长度的数据包.
 */
uint8    ai_need_zero_packet;

// 当前的配置值,只有在设置非0配置后
uint8    ai_config_value;

// 端点1缓冲是否忙的标志.当缓冲区中有数据时,该标志为真.
// 当缓冲区中空闲时,该标志为假.
uint8    ai_ep1_in_is_busy;

/*******************************************************************************
* 设备描述符定义结构
*******************************************************************************/
static code uint8 ai_device_desc[18] = {
    0x12,    // bLength字段: 设备描述符的长度为18(0x12)字节
    0x01,    // bDescriptorType字段: 设备描述符的编号为0x01
    0x10,    // bcdUSB字段: 这里设置版本为USB1.1,即0x0110.
    0x01,    // 由于是小端结构,所以低字节在先,即0x10,0x01.
    0x00,    // bDeviceClass字段: 我们不在设备描述符中定义设备类,
             // 而在接口描述符中定义设备类,所以该字段的值为0.
    0x00,    // bDeviceSubClass字段: bDeviceClass字段为0时,该字段也为0.
    0x00,    // bDeviceProtocol字段: bDeviceClass字段为0时,该字段也为0.
    0x10,    // bMaxPacketSize0字段：PDIUSBD12的端点0大小的16字节.
    /* idVender字段.厂商ID号,我们这里取0x8888,仅供实验用.
       实际产品不能随便使用厂商ID号,必须跟USB协会申请厂商ID号.
       注意小端模式,低字节在先.*/
    0x88,
    0x88,
    /* idProduct字段.产品ID号,由于是第二个实验,我们这里取0x0002.
       注意小端模式,低字节应该在前.*/
    0x02,
    0x00,
    /* bcdDevice字段.我们这个USB鼠标刚开始做,就叫它1.0版吧,即0x0100.
       小端模式,低字节在先.*/
    0x00,
    0x01,
    /* iManufacturer字段.厂商字符串的索引值,为了方便记忆和管理,
       字符串索引就从1开始吧.*/
    0x01,
    /* iProduct字段.产品字符串的索引值.刚刚用了1,这里就取2吧.
       注意字符串索引值不要使用相同的值.*/
    0x02,
    0x03,     // iSerialNumber字段.设备的序列号字符串索引值.这里取3就可以了.
    /* bNumConfigurations字段.该设备所具有的配置数.
       我们只需要一种配置就行了,因此该值设置为1.*/
    0x01
};

/*******************************************************************************
* USB报告描述符定义结构
* 通过下面的报告描述符的定义,我们知道返回的输入报告具有8字节。
* 第一字节的8个bit用来表示特殊键是否按下（例如Shift、Alt等键）。
* 第二字节为保留值，值为常量0。第三到第八字节是一个普通键键值的
* 数组，当没有键按下时，全部6个字节值都为0。当只有一个普通键按下时，
* 这六个字节中的第一字节值即为该按键的键值（具体的键值请看HID的
* 用途表文档），当有多个普通键同时按下时，则同时返回这些键的键值。
* 如果按下的键太多，则这六个字节都为0xFF（不能返回0x00，这样会让
* 操作系统认为所有键都已经释放）。至于键值在数组中的先后顺序是
* 无所谓的，操作系统会负责检查是否有新键按下。我们应该在中断端点1
* 中按照上面的格式返回实际的键盘数据。另外，报告中还定义了一个字节
* 的输出报告，是用来控制LED情况的。只使用了低7位，高1位是保留值0。
* 当某位的值为1时，则表示对应的LED要点亮。操作系统会负责同步各个
* 键盘之间的LED，例如你有两块键盘，一块的数字键盘灯亮时，另一块
* 也会跟着亮。键盘本身不需要判断各种LED应该何时亮，它只是等待主机
* 发送报告给它，然后根据报告值来点亮相应的LED。我们在端点1输出中断
* 中读出这1字节的输出报告，然后对它取反（因为学习板上的LED是低电平时
* 亮），直接发送到LED上。这样main函数中按键点亮LED的代码就不需要了。
*******************************************************************************/
static code uint8 ai_report_desc[] = {
    //每行开始的第一字节为该条目的前缀,前缀的格式为：
    // D7~D4：bTag.D3~D2：bType；D1~D0：bSize.以下分别对每个条目注释.
    
    // 这是一个全局（bType为1）条目,选择用途页为
    // 普通桌面Generic Desktop Page(0x01)后面跟一字节数据（bSize为1）,
    // 后面的字节数就不注释了,自己根据bSize来判断.
    0x05, 0x01,    // USAGE_PAGE (Generic Desktop)
    
    // 这是一个局部（bType为2）条目,说明接下来的应用集合用途用于鼠标
    0x09, 0x06,    // USAGE (Keyboard)
    
    // 这是一个主条目（bType为0）条目,开集合,后面跟的数据0x01表示
    // 该集合是一个应用集合.它的性质在前面由用途页和用途定义为
    // 普通桌面用的键盘.
    0xa1, 0x01,    // COLLECTION (Application)
    
    // 这是一个局部条目.选择用途页为键盘(Keyboard/Keypad(0x07))
    0x05, 0x07,    // USAGE_PAGE (Keyboard/Keypad)
    
    // 这是一个局部条目，说明用途的最小值为0xe0。实际上是键盘左Ctrl键
    // 具体的用途值可在HID用途表中查看
    0x19, 0xe0,    // USAGE_MINIMUM (Keyboard LeftControl)
    
    // 这是一个局部条目，说明用途的最大值为0xe7。实际上是键盘右GUI键。
    0x29, 0xe7,    // USAGE_MAXIMUM (Keyboard Right GUI)
    
    // 这是一个全局条目，说明返回的数据的逻辑值(就是我们返回的数据域的值)
    // 最小为0。因为我们这里用Bit来表示一个数据域，因此最小为0，最大为1。
    0x15, 0x00,    // LOGICAL_MINIMUM (0)
    
    // 这是一个全局条目，说明逻辑值最大为1。
    0x25, 0x01,    // LOGICAL_MAXIMUM (1)
    
    // 这是一个全局条目，说明数据域的数量为八个。
    0x95, 0x08,    // REPORT_COUNT (8)
    
    // 这是一个全局条目，说明每个数据域的长度为1个bit。
    0x75, 0x01,    // REPORT_SIZE (1)
    
    // 这是一个主条目，说明有8个长度为1bit的数据域（数量和长度
    // 由前面的两个全局条目所定义）用来做为输入，
    // 属性为：Data,Var,Abs。Data表示这些数据可以变动，Var表示
    // 这些数据域是独立的，每个域表示一个意思。Abs表示绝对值。
    // 这样定义的结果就是，当某个域的值为1时，就表示对应的键按下。
    // bit0就对应着用途最小值0xe0，bit7对应着用途最大值0xe7。
    0x81, 0x02,    // INPUT (Data,Var,Abs)
    
    // 这是一个全局条目，说明数据域数量为1个
    0x95, 0x01,    // REPORT_COUNT (1)
    
    // 这是一个全局条目，说明每个数据域的长度为8bit。
    0x75, 0x08,    // REPORT_SIZE (8)
    
    // 这是一个主条目，输入用，由前面两个全局条目可知，长度为8bit，
    // 数量为1个。它的属性为常量（即返回的数据一直是0）。
    // 该字节是保留字节（保留给OEM使用）。
    0x81, 0x03,    // INPUT (Cnst,Var,Abs)
    
    // 这是一个全局条目。定义位域数量为6个。
    0x95, 0x06,    // REPORT_COUNT (6)
    
    // 这是一个全局条目。定义每个位域长度为8bit。
    // 其实这里这个条目不要也是可以的，因为在前面已经有一个定义
    // 长度为8bit的全局条目了。
    0x75, 0x08,    // REPORT_SIZE (8)
    
    // 这是一个全局条目，定义逻辑最小值为0。
    // 同上，这里这个全局条目也是可以不要的，因为前面已经有一个
    // 定义逻辑最小值为0的全局条目了。
    0x15, 0x00,    // LOGICAL_MINIMUM (0)
    
    // 这是一个全局条目，定义逻辑最大值为255。
    0x25, 0xFF,    // LOGICAL_MAXIMUM (255)
    
    // 这是一个全局条目，选择用途页为键盘。
    // 前面已经选择过用途页为键盘了，所以该条目不要也可以。
    0x05, 0x07,    // USAGE_PAGE (Keyboard/Keypad)
    
    // 这是一个局部条目，定义用途最小值为0（0表示没有键按下）
    0x19, 0x00,    // USAGE_MINIMUM (Reserved (no event indicated))
    
    // 这是一个局部条目，定义用途最大值为0x65
    0x29, 0x65,    // USAGE_MAXIMUM (Keyboard Application)
    
    // 这是一个主条目。它说明这六个8bit的数据域是输入用的，
    // 属性为：Data,Ary,Abs。Data说明数据是可以变的，Ary说明
    // 这些数据域是一个数组，即每个8bit都可以表示某个键值，
    // 如果按下的键太多（例如超过这里定义的长度或者键盘本身无法
    // 扫描出按键情况时），则这些数据返回全1（二进制），表示按键无效
    // Abs表示这些值是绝对值。
    0x81, 0x00,    // INPUT (Data,Ary,Abs)
    
    // 以下为输出报告的描述
    // 逻辑最小值前面已经有定义为0了，这里可以省略。 
    // 这是一个全局条目，说明逻辑值最大为1。
    0x25, 0x01,    // LOGICAL_MAXIMUM (1)
    
    // 这是一个全局条目，说明数据域数量为5个。 
    0x95, 0x05,    // REPORT_COUNT (5)
    
    // 这是一个全局条目，说明数据域的长度为1bit。
    0x75, 0x01,    // REPORT_SIZE (1)
    
    // 这是一个全局条目，说明使用的用途页为指示灯（LED）
    0x05, 0x08,    // USAGE_PAGE (LEDs)
    
    // 这是一个局部条目，说明用途最小值为数字键盘灯。
    0x19, 0x01,    // USAGE_MINIMUM (Num Lock)
    
    // 这是一个局部条目，说明用途最大值为Kana灯。
    0x29, 0x05,    // USAGE_MAXIMUM (Kana)
    
    // 这是一个主条目。定义输出数据，即前面定义的5个LED。
    0x91, 0x02,    // OUTPUT (Data,Var,Abs)
    
    // 这是一个全局条目。定义位域数量为1个。
    0x95, 0x01,    // REPORT_COUNT (1)
    
    // 这是一个全局条目。定义位域长度为3bit。
    0x75, 0x03,    // REPORT_SIZE (3)
    
    // 这是一个主条目，定义输出常量，前面用了5bit，所以这里需要
    // 3个bit来凑成一字节。
    0x91, 0x03,    // OUTPUT (Cnst,Var,Abs)
    
    // 下面这个主条目用来关闭前面的集合。bSize为0，所以后面没数据。
    0xc0           // END_COLLECTION
};

/*******************************************************************************
* USB配置描述符集合定义结构,配置描述符总长度为9+9+9+7字节
*******************************************************************************/
static code uint8 ai_cfg_desc[9 + 9 + 9 + 7 + 7] = {
    /*************** 配置描述符 ***********************/
    // bLength字段: 配置描述符的长度为9字节
    0x09,
    
    // bDescriptorType字段: 配置描述符编号为0x02
    0x02,
    
    // wTotalLength字段: 配置描述符集合的总长度,包括配置描述符本身、
    // 接口描述符、类描述符、端点描述符等.
    sizeof(ai_cfg_desc) & 0xFF,           // 低字节
    (sizeof(ai_cfg_desc) >> 8) & 0xFF,    // 高字节
    
    // bNumInterfaces字段: 该配置包含的接口数,只有一个接口.
    0x01,
    
    // bConfigurationValue字段: 该配置的值为1.
    0x01,
    
    // iConfigurationz字段: 该配置的字符串索引, 这里没有, 为0.
    0x00,
    
    // bmAttributes字段: 该设备的属性. 由于我们的板子是总线供电的,
    // 并且我们不想实现远程唤醒的功能,所以该字段的值为0x80.
    0x80,
    
    // bMaxPower字段,该设备需要的最大电流量.由于我们的板子需要的电流不到100mA,
    // 因此我们这里设置为100mA.由于每单位电流为2mA,所以这里设置为50(0x32).
    0x32,
    
    /******************* 接口描述符 *********************/
    // bLength字段: 接口描述符的长度为9字节.
    0x09,
    
    // bDescriptorType字段: 接口描述符的编号为0x04.
    0x04,
    
    // bInterfaceNumber字段: 该接口的编号,第一个接口,编号为0.
    0x00,
    
    // bAlternateSetting字段: 该接口的备用编号,为0.
    0x00,
    
    // bNumEndpoints字段: 非0端点的数目.由于USB鼠标只需要一个中断输入端点,
    // 因此该值为1.
    0x02,
    
    // bInterfaceClass字段: 该接口所使用的类.USB鼠标是HID类,编码为0x03.
    0x03,
    
    // bInterfaceSubClass字段: 该接口所使用的子类.在HID1.1协议中,只规定了一种
    // 子类：支持BIOS引导启动的子类.USB键盘、鼠标属于该子类,子类代码为0x01.
    0x01,
    
    // bInterfaceProtocol字段: 如果子类为支持引导启动的子类,则协议可选择鼠标和
    // 键盘.键盘代码为0x01,鼠标代码为0x02.
    0x01,
    
    // iInterface字段.该接口的字符串索引值.这里没有,为0.
    0x00,
    
    /****************** HID描述符 ************************/
    // bLength字段: 本HID描述符下只有一个下级描述符.所以长度为9字节.
    0x09,
    
    // bDescriptorType字段: HID描述符的编号为0x21.
    0x21,
    
    // bcdHID字段: 本协议使用的HID1.1协议.注意低字节在先.
    0x10,
    0x01,
    
    // bCountyCode字段: 设备适用的国家代码,这里选择为美国,代码0x21.
    0x21,
    
    // bNumDescriptors字段: 下级描述符的数目.我们只有一个报告描述符.
    0x01,
    
    // bDescritporType字段: 下级描述符的类型,为报告描述符,编号为0x22.
    0x22,
    
    // bDescriptorLength字段: 下级描述符的长度.下级描述符为报告描述符.
    sizeof(ai_report_desc)&0xFF,
    (sizeof(ai_report_desc)>>8)&0xFF,
    
    /********************** 输入端点描述符 ***********************/
    // bLength字段: 端点描述符长度为7字节.
    0x07,
    
    // bDescriptorType字段: 端点描述符编号为0x05.
    0x05,
    
    // bEndpointAddress字段: 端点的地址.我们使用D12的输入端点1.
    // D7位表示数据方向,输入端点D7为1.所以输入端点1的地址为0x81.
    0x81,
    
    // bmAttributes字段: D1~D0为端点传输类型选择.
    // 该端点为中断端点.中断端点的编号为3.其它位保留为0.
    0x03,
    
    // wMaxPacketSize字段: 该端点的最大包长.端点1的最大包长为16字节.
    // 注意低字节在先.
    0x10,
    0x00,
    
    // bInterval字段: 端点查询的时间,我们设置为10个帧时间,即10ms.
    0x0A,

    /********************** 输出端点描述符 ***********************/
    // bLength字段: 端点描述符长度为7字节.
    0x07,
    
    // bDescriptorType字段: 端点描述符编号为0x05.
    0x05,
    
    // bEndpointAddress字段: 端点的地址.我们使用D12的输入端点1.
    // D7位表示数据方向,输入端点D7为1.所以输入端点1的地址为0x81.
    0x01,
    
    // bmAttributes字段: D1~D0为端点传输类型选择.
    // 该端点为中断端点.中断端点的编号为3.其它位保留为0.
    0x03,
    
    // wMaxPacketSize字段: 该端点的最大包长.端点1的最大包长为16字节.
    // 注意低字节在先.
    0x10,
    0x00,
    
    // bInterval字段: 端点查询的时间,我们设置为10个帧时间,即10ms.
    0x0A
};

/*******************************************************************************
* 语言ID的定义
*******************************************************************************/
static code uint8 ai_lang_id[4] = {
    0x04,
    0x03,
    0x09,    // 0x0409为美式英语的ID
    0x04
};

/*******************************************************************************
* 字符串描述符的定义结构,8位小端格式
*******************************************************************************/
// 字符串“电脑圈圈的USB专区 Http://group.ednchina.com/93/”的Unicode编码
static code uint8 ai_mfr_str_desc[82] = {
    82,         //该描述符的长度为82字节
    0x03,       //字符串描述符的类型编码为0x03
    0x35, 0x75, //电
    0x11, 0x81, //脑
    0x08, 0x57, //圈
    0x08, 0x57, //圈
    0x84, 0x76, //的
    0x55, 0x00, //U
    0x53, 0x00, //S
    0x42, 0x00, //B
    0x13, 0x4e, //专
    0x3a, 0x53, //区
    0x20, 0x00, // 
    0x48, 0x00, //H
    0x74, 0x00, //t
    0x74, 0x00, //t
    0x70, 0x00, //p
    0x3a, 0x00, //:
    0x2f, 0x00, ///
    0x2f, 0x00, ///
    0x67, 0x00, //g
    0x72, 0x00, //r
    0x6f, 0x00, //o
    0x75, 0x00, //u
    0x70, 0x00, //p
    0x2e, 0x00, //.
    0x65, 0x00, //e
    0x64, 0x00, //d
    0x6e, 0x00, //n
    0x63, 0x00, //c
    0x68, 0x00, //h
    0x69, 0x00, //i
    0x6e, 0x00, //n
    0x61, 0x00, //a
    0x2e, 0x00, //.
    0x63, 0x00, //c
    0x6f, 0x00, //o
    0x6d, 0x00, //m
    0x2f, 0x00, ///
    0x39, 0x00, //9
    0x33, 0x00, //3
    0x2f, 0x00  /// 
};

// 字符串“《圈圈教你玩USB》之USB鼠标”的Unicode编码
static code uint8 ai_product_str_desc[34] = {
    34,         //该描述符的长度为34字节
    0x03,       //字符串描述符的类型编码为0x03
    0x0a, 0x30, //《
    0x08, 0x57, //圈
    0x08, 0x57, //圈
    0x59, 0x65, //教
    0x60, 0x4f, //你
    0xa9, 0x73, //玩
    0x55, 0x00, //U
    0x53, 0x00, //S
    0x42, 0x00, //B
    0x0b, 0x30, //》
    0x4b, 0x4e, //之
    0x55, 0x00, //U
    0x53, 0x00, //S
    0x42, 0x00, //B
    0x20, 0x9f, //鼠
    0x07, 0x68  //标
};

// 字符串“2008-07-07”的Unicode编码
static code uint8 ai_sn_str_desc[22] = {
    22,         //该描述符的长度为22字节
    0x03,       //字符串描述符的类型编码为0x03
    0x32, 0x00, //2
    0x30, 0x00, //0
    0x32, 0x00, //2
    0x31, 0x00, //1
    0x2d, 0x00, //-
    0x30, 0x00, //0
    0x32, 0x00, //2
    0x2d, 0x00, //-
    0x32, 0x00, //2
    0x31, 0x00  //1 
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
    ai_ep1_in_is_busy = 0;
}

/*******************************************************************************
* Function：根据ai_send_data和ai_send_length将数据发送到端点0的函数.
* Input   : 无
* Ouput   : None
* Return  : None
* Others  :
*******************************************************************************/
static void ai_usb_ep0_send_data(void)
{
    // 将数据写到端点中去,准备发送写之前要先判断一下需要发送的数据是否比端点0
    // 最大长度大，如果超过端点大小，则一次只能发送最大包长的数据
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
                        // 这里请求的是设备描述符,因此数据长度就是
                        // ai_device_desc[0].如果请求的比实际的长,
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
                        ai_send_data = ai_cfg_desc;
                        // 判断请求的字节数是否比实际需要发送的字节数多
                        // 这里请求的是配置描述符集合，因此数据长度就是
                        // ai_cfg_desc[3] * 256 + ai_cfg_desc[2]
                        // 如果请求的比实际的长，那么只返回实际长度的数
                        ai_send_length = ai_cfg_desc[3] * 256 + ai_cfg_desc[2];
                        if (ai_length > ai_send_length) {
                            // 并且刚好是整数个数据包时
                            if (ai_send_length % ai_device_desc[7] == 0) {
                                ai_need_zero_packet = 1;
                            }
                        } else {
                            ai_send_length = ai_length;
                        }
                        // 将数据通过EP0返回
                        ai_usb_ep0_send_data();
                        break;
                    case AIUSB_STRING_DESCRIPTOR:
                        ai_uart_send_str("字符串描述符");
                        // 根据wValue的低字节（索引值）散转
                        switch (ai_value & 0xff) {
                        case 0:    // 获取语言ID
                            ai_uart_send_str("(语言ID).\r\n");
                            ai_send_data = ai_lang_id;
                            ai_send_length = ai_lang_id[0];
                            break;
                        case 1:    // 获取厂商字符串
                            ai_uart_send_str("(厂商描述).\r\n");
                            ai_send_data = ai_mfr_str_desc;
                            ai_send_length = ai_mfr_str_desc[0];
                            break;
                        case 2:
                            ai_uart_send_str("(产品描述).\r\n");
                            ai_send_data = ai_product_str_desc;
                            ai_send_length = ai_product_str_desc[0];
                            break;
                        case 3:
                            ai_uart_send_str("(产品序列号).\r\n");
                            ai_send_data = ai_sn_str_desc;
                            ai_send_length = ai_sn_str_desc[0];
                            break;
                        default:
                            ai_uart_send_str("(未知的索引值).\r\n");
                            ai_send_length = 0;
                            ai_need_zero_packet = 1;
                        }
                        if (ai_length > ai_send_length) {
                            // 并且刚好是整数个数据包时
                            if (ai_send_length % ai_device_desc[7] == 0) {
                                ai_need_zero_packet = 1;
                            }
                        } else {
                            ai_send_length = ai_length;
                        }
                        // 将数据通过EP0返回
                        ai_usb_ep0_send_data();
                        break;
                    case AIUSB_INTERFACE_DESCRIPTOR:
                        break;
                    case AIUSB_ENDPOINT_DESCRIPTOR:
                        break;
                    case AIUSB_REPORT_DESCRIPTOR:
                        ai_uart_send_str("报告描述符.\r\n");
                        ai_send_data = ai_report_desc;
                        ai_send_length = sizeof(ai_report_desc);
                        if (ai_length > ai_send_length) {
                            // 并且刚好是整数个数据包时
                            if (ai_send_length % ai_device_desc[7] == 0) {
                                ai_need_zero_packet = 1;
                            }
                        } else {
                            ai_send_length = ai_length;
                        }
                        // 将数据通过EP0返回
                        ai_usb_ep0_send_data();
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
                    ai_uart_send_str("设置地址,地址为：");
                    ai_uart_print_hex(ai_value & 0xff);
                    ai_uart_send_str("\r\n");
                    ai_d12_set_addr(ai_value & 0xff);
                    // 设置地址没有数据过程,直接进入到状态过程,
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
                    // 使能非0端点.非0端点只有在设置为非0的配置后才能使能.
                    // ai_value的低字节为配置的值，如果该值为非0，
                    // 才能使能非0端点.保存当前配置值
                    ai_config_value = ai_value & 0xff;
                    ai_d12_set_endp_enable(ai_config_value);
                    ai_send_length = 0;
                    ai_need_zero_packet = 1;
                    ai_usb_ep0_send_data();
                    break;
                case AIUSB_SET_INTERFACE:
                    ai_uart_send_str("设置接口.\r\n");
                    break;
                default:
                    ai_uart_send_str("错误：未定义的标准输出请求.\r\n");
                }
                break;
            case 1:
                ai_uart_send_str("USB类输出请求: ");
                switch (ai_request) {
                case AIUSB_REQUSET_SET_IDLE:
                    ai_uart_send_str("设置空闲.\r\n");
                    ai_send_length = 0;
                    ai_need_zero_packet = 1;
                    ai_usb_ep0_send_data();
                    break;
                case AIUSB_REQUSET_GET_REPORT:
                    ai_uart_send_str("获取报告.\r\n");
                    break;
                case AIUSB_REQUSET_GET_IDLE:
                    ai_uart_send_str("获取空闲.\r\n");
                    break;
                default:
                   ai_uart_send_str("未知请求.\r\n"); 
                };
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
    uint8 buf[1] = {0};
    ai_uart_send_str("USB端点1输出中断.\r\n");

    ai_d12_read_endp_last_stat(2);
    ai_d12_read_end_point_buf(2, 1, buf);
    ai_d12_clear_buf();
    AI_ALL_LEDS = ~buf[0];
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
    // 读最后发送状态，这将清除端点1输入的中断标志位
    ai_d12_read_endp_last_stat(3);
    // 端点1输入处于空闲状态
    ai_ep1_in_is_busy = 0;
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