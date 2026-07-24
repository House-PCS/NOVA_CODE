/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_communication.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       设备通信协议实现文件，负责BMS与外部设备（FPGA/UI）的UART通信
  *              支持固件升级、BMS数据读写、调试信息发送等功能，包含协议打包/解包、
  *              校验和计算、数据收发等核心逻辑
  ******************************************************************************
  * @attention
  * 1. 通信依赖UART0（调试）和UART1（FPGA通信）硬件资源
  * 2. 固件升级过程中会暂停AFE采集，升级完成后恢复
  * 3. 所有数据包均包含校验和，确保数据传输完整性
  * 4. 协议格式需与通信对端（FPGA/UI）严格一致
  * @relation
  * @par 版本更新记录:
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1      <td>V1.0.0    <td>            <td>初始版本，实现基础通信功能
  * </table>
  ******************************************************************************
  */
#include "dev_communication.h"
#include "math.h"
#include "mid_shell.h"

// 日志标签，用于调试输出标识
#define LOG_TAG     "dev_communication"
// UART通信句柄定义（与FPGA软核通信使用UART1）
#define UartComHandle  Uart1_Fpga_Sofcore_Handle

/************************** 全局变量定义 **************************/
uint8_t garrComRxBuf[COM_RX_SIZE];          // 通信接收缓冲区，存储UART接收的数据
uint8_t garrComTxBuf[COM_TX_SIZE];          // 通信发送缓冲区，存储待发送的数据包
uint8_t pack_count = 0;                     // 固件升级数据包计数器，记录已接收的升级包数量
uint16_t updata_pack_size = 0;              // 固件升级总包数（从升级启动命令中获取）
uint8_t space_num = 0;                      // 升级数据包中无效数据长度（用于跳过头部无效字节）

// 协议请求数据包结构体（接收自对端的请求数据）
static ProtocalRequestPacket_t     _gstRequestPacketData         = {0};
// 调试用读取数据包结构体（用于UART0调试信息发送）
ProtocalRequestPacket_t             gstDebReadPacketData         = {0};
// 协议响应数据包结构体（发送给对端的响应数据）
static ProtocalResponsePacket_t     _gstResponsePacketData       = {0};
// 协议读取响应数据包结构体（读取BMS数据后的响应包）
static ProtocalReadResponsePacket_t _gstReadResponsePacketData   = {0};
// 协议数据缓冲区（存储解包后的有效数据字段）
static uint8_t  _ProtocolBuf[COM_RX_SIZE]   = {0};
// BMS固件升级结束计时（volatile确保多线程/中断中可见）
volatile static uint32_t _BMSFirmwareUpgradeEndTimes = 0;
// BMS硬件版本号（3字节，从升级包中获取）
uint8_t gBMSHardVersion[3] = {0};
// BMS硬件信息长度
uint32_t gBMSHardwareLen;
// BMS固件类型标识（固定值：0x0C,0x0C,0x0C,0x03，用于升级包校验）
uint8_t gBMSFirmwareType[4] = {0x0C, 0x0C, 0x0C, 0x03};
// 通信接收队列（用于缓存UART接收的数据帧）
Queue_t gComRevQueue;

/************************** 函数声明 **************************/
static void IrqRxProcess(void);             // UART接收中断处理函数（声明未实现）

/**
* @brief       校验和计算函数
* @param       *pdata: 待计算校验和的数据缓冲区指针
* @param       len: 待计算数据的长度（字节数）
* @param       mode: 校验和类型选择 0-16位校验和 1-32位校验和
* @return      计算得到的校验和（16位或32位，通过返回值类型自动适配）
* @note        采用累加和算法，将所有字节数据相加得到校验和，用于数据完整性校验
*/
uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode)
{
    uint16_t checksum16 = 0;  // 16位校验和变量
    uint32_t checksum32 = 0;  // 32位校验和变量
    uint16_t i = 0;           // 循环计数器

    if(mode)  // 模式1：计算32位校验和
    {
        for(i = 0; i < len; i ++)
        {
            checksum32 += pdata[i];  // 累加所有字节到32位变量
        }
        return checksum32;
    }
    else  // 模式0：计算16位校验和
    {
        for(i = 0; i < len; i ++)
        {
            checksum16 += pdata[i];  // 累加所有字节到16位变量
        }
        return checksum16;
    }
}

/**
*******************************************************************************
* Function:    UartCtrlMSB2LSBU16
* @brief:      16位数据的大端模式（MSB）转小端模式（LSB）
* @details:    通信协议中可能使用大端传输，而MCU为小端存储，需转换后才能正确解析
* Parameters:  @param [in] *input: 大端模式的16位数据指针
* @return:     转换后的小端模式16位数据
* @note        示例：输入0x1234（大端，即内存中存储为0x12,0x34），转换后返回0x3412
*******************************************************************************
*/
uint16_t UartCtrlMSB2LSBU16(uint16_t* input)
{
    uint16_t uShort = 0;  // 转换结果变量

    uShort = *(input + 1);          // 取高字节（大端模式下input[1]是高8位）
    uShort = (uShort << 8) | (*input);  // 高字节左移8位，或上低字节（input[0]）

    return uShort;
}

/**
* @brief       固件升级响应数据包发送函数（轮询模式）
* @details     当接收到升级相关命令（启动/进行中/结束）时，发送响应包给对端
*              响应包包含命令确认、错误码等信息，确保升级流程同步
* @param       无
* @return      无
* @note        1. 响应包格式遵循ProtocalResponsePacket_t结构体定义
*              2. 自动填充包头部、地址、命令等字段，基于接收的请求包信息
*              3. 计算并填充校验和，确保响应包完整性
*/
void UpgradeDataSendPolling()
{
    uint16_t check_sum = 0;  // 校验和变量

    /* 组包：填充响应包基本信息 */
    _gstResponsePacketData.packet_head = RESPONSE_PACKET_HEAD;  // 响应包头部标识
    _gstResponsePacketData.send_addr   = _gstRequestPacketData.recv_addr;  // 发送地址=请求包的接收地址
    _gstResponsePacketData.recv_addr   = _gstRequestPacketData.send_addr;  // 接收地址=请求包的发送地址
    _gstResponsePacketData.cmd         = _gstRequestPacketData.cmd;        // 命令与请求包一致

    // 计算校验和：排除check_sum字段本身（结构体最后2字节）
    _gstResponsePacketData.check_sum = CheckSumFun((uint8_t*)&_gstResponsePacketData, 
                                                   sizeof(_gstResponsePacketData) - 2, 0);

    // 配置UART1发送参数
    Uart1_Fpga_Sofcore_Handle.pTxBuf = (uint8_t*)&_gstResponsePacketData;  // 发送缓冲区指针
    Uart1_Fpga_Sofcore_Handle.tx_size = sizeof(ProtocalResponsePacket_t);  // 发送数据长度

    UartSend(&Uart1_Fpga_Sofcore_Handle);  // 调用UART发送接口

    return;
}

/**
* @brief       所有BMS数据响应发送函数（轮询模式）
* @details     当接收到READ_ALLBMS_DATA命令时，打包所有BMS可读参数并发送给对端
* @param       [in] p_data_buff: 发送数据缓冲区指针（外部传入，需确保足够空间）
* @return      无
* @note        1. 数据包格式遵循ProtocalReadResponsePacket_t结构体定义
*              2. 包含BMS设备ID、数据长度、寄存器数据等完整信息
*              3. 末尾添加校验和，确保数据传输正确
*/
void AllBMSDataSendPolling(uint8_t* p_data_buff)
{
    uint16_t check_sum = 0;          // 校验和变量
    uint8_t* p_send_data_buff = p_data_buff;  // 发送缓冲区指针

    /* 组包：填充响应包基本信息 */
    _gstResponsePacketData.packet_head = RESPONSE_PACKET_HEAD;  // 响应包头部标识
    _gstResponsePacketData.send_addr   = _gstRequestPacketData.recv_addr;  // 发送地址
    _gstResponsePacketData.recv_addr   = _gstRequestPacketData.send_addr;  // 接收地址
    _gstResponsePacketData.cmd         = _gstRequestPacketData.cmd;        // 命令标识

    // 将基础响应包信息复制到读取响应包结构体（前5字节：头部+地址+命令）
    memcpy((uint8_t*)&_gstReadResponsePacketData, (uint8_t*)&_gstResponsePacketData, 5);
    // 将读取响应包头部复制到发送缓冲区
    memcpy(p_send_data_buff, (uint8_t*)&_gstReadResponsePacketData, sizeof(ProtocalReadResponsePacket_t));

    // 计算校验和：包含响应包头部 + 数据字段
    check_sum = CheckSumFun(p_send_data_buff, 
                           _gstReadResponsePacketData.data_field_len + sizeof(ProtocalReadResponsePacket_t), 
                           0);

    // 将校验和添加到数据包末尾
    memcpy((p_send_data_buff + sizeof(ProtocalReadResponsePacket_t) + _gstReadResponsePacketData.data_field_len), 
           (uint8_t*)&check_sum, sizeof(check_sum));

    // 配置UART1发送参数
    Uart1_Fpga_Sofcore_Handle.pTxBuf = p_send_data_buff;  // 发送缓冲区
    // 发送长度 = 响应包头部长度 + 数据长度 + 校验和长度
    Uart1_Fpga_Sofcore_Handle.tx_size = _gstReadResponsePacketData.data_field_len + 
                                        sizeof(ProtocalReadResponsePacket_t) + sizeof(check_sum);

    UartSend(&Uart1_Fpga_Sofcore_Handle);  // 发送数据

    return;
}

/**
* @brief       BMS数据写入响应发送函数（轮询模式）
* @details     当接收到WRITE_ALLBMS_DATA/WRITE_PARTBMS_DATA命令时，发送写入确认响应
* @param       无
* @return      无
* @note        响应包仅包含命令确认信息，无额外数据字段
*/
void WriteBMSDataSendPolling()
{
    uint16_t check_sum = 0;  // 校验和变量

    /* 组包：填充响应包基本信息 */
    _gstResponsePacketData.packet_head = RESPONSE_PACKET_HEAD;  // 响应包头部
    _gstResponsePacketData.send_addr   = _gstRequestPacketData.recv_addr;  // 发送地址
    _gstResponsePacketData.recv_addr   = _gstRequestPacketData.send_addr;  // 接收地址
    _gstResponsePacketData.cmd         = _gstRequestPacketData.cmd;        // 命令标识

    // 计算校验和（排除check_sum字段本身）
    _gstResponsePacketData.check_sum = CheckSumFun((uint8_t*)&_gstResponsePacketData, 
                                                   sizeof(_gstResponsePacketData) - 2, 0);

    // 配置UART1发送参数
    Uart1_Fpga_Sofcore_Handle.pTxBuf = (uint8_t*)&_gstResponsePacketData;  // 发送缓冲区
    Uart1_Fpga_Sofcore_Handle.tx_size = sizeof(ProtocalResponsePacket_t);  // 发送长度

    UartSend(&Uart1_Fpga_Sofcore_Handle);  // 发送响应包

    return;
}

/**
*******************************************************************************
* Function:    Uart0DebugSendPolling
* @brief:      UART0调试信息发送函数（轮询模式）
* @details:    周期性发送BMS调试数据到UI端，包含设备状态、寄存器数据等信息
* Parameters:  @param [in] p_data_buff: 调试数据发送缓冲区（外部传入）
* @return:     None
* @note        1. 使用UART0进行调试通信，波特率等参数需提前配置
*              2. 数据包格式遵循调试协议，包含设备地址、命令、数据长度、校验和
*              3. 数据字段涵盖UART_DEBUG_REG_BASE到UART_DEBUG_REG_END的所有寄存器
*******************************************************************************
*/
void Uart0DebugSendPolling(uint8_t* p_data_buff)
{
    uint8_t packet_len = 5;          // 基础包长度（头部+地址+命令）
    uint16_t check_sum = 0;          // 校验和变量
    uint16_t send_buffer_len = 0;    // 发送缓冲区总长度
    uint8_t* p_send_data_buff = p_data_buff;  // 发送缓冲区指针

    /* 组包：填充调试数据包基本信息 */
    gstDebReadPacketData.packet_head = REQUST_PACKET_HEAD;  // 数据包头部标识（请求包格式）
    gstDebReadPacketData.send_addr   = BMS_ADDR;            // 发送地址：BMS设备地址
    gstDebReadPacketData.recv_addr   = UI_ADDR;             // 接收地址：UI设备地址
    gstDebReadPacketData.cmd         = WRITE_ALLBMS_DATA;   // 命令：写入所有BMS数据（调试用）

    // 计算数据字段长度：
    // 寄存器长度 + BMS ID(1字节) + 数据长度(2字节) + 基地址(1字节) + 预留1字节
    gstDebReadPacketData.data_field_len = (uint16_t)(UART_DEBUG_REG_END_ADDR - UART_DEBUG_REG_BASE_ADDR) + 3 + 1 + 1;

    // 将调试包头部信息复制到发送缓冲区
    memcpy(p_send_data_buff, (uint8_t*)&gstDebReadPacketData, DEBUG_PACKET_LEN);

    /* 计算校验和 */
    send_buffer_len = gstDebReadPacketData.data_field_len + DEBUG_PACKET_LEN;  // 头部长度+数据长度
    check_sum = CheckSumFun(p_send_data_buff, send_buffer_len, 0);  // 计算校验和

    // 将校验和添加到数据包末尾
    memcpy((p_send_data_buff + send_buffer_len), (uint8_t*)&check_sum, sizeof(check_sum));

    // 配置UART0发送参数
    Uart0_Handle.pTxBuf = p_send_data_buff;  // 发送缓冲区
    Uart0_Handle.tx_size = send_buffer_len + sizeof(check_sum);  // 总长度=头部+数据+校验和

    UartSend(&Uart0_Handle);  // 发送调试数据

    return;
}

/**
*******************************************************************************
* Function:    _CmdAnalysis
* @brief:      通信命令解析函数（静态函数，内部使用）
* @details:    解析接收到的请求包命令，执行对应操作（升级/读写数据）
* Parameters:  @param [in] p_request_packet: 解析后的请求包结构体指针
* @return:     true-命令解析成功 false-命令解析失败（仅升级包校验失败时返回）
* @note        1. 支持的命令：升级启动/进行中/结束、读取所有BMS数据、写入全部/部分BMS数据
*              2. 固件升级过程中会暂停AFE采集，避免数据冲突
*              3. 升级包包含固件类型校验、版本号、长度、校验和等信息，确保升级可靠性
*******************************************************************************
*/
static bool _CmdAnalysis(ProtocalRequestPacket_t *p_request_packet)
{
    uint16_t cmd = p_request_packet->cmd;  // 提取命令字
    static uint32_t base_firmware_addr;    // 固件存储基地址（静态变量，保持升级过程中值不变）
    static uint32_t firmware_checksum_compute;  // 计算的固件校验和
    static uint32_t firmware_checksum_rec;      // 接收的固件校验和（来自升级包）
//    static uint32_t num_index;  // 升级包序号（预留，未使用）

    // 根据命令字分支处理
    switch(cmd)
    {
        case UPGRADE_START:  // 固件升级启动命令
        {
            /* 初始化固件升级相关参数 */
            base_firmware_addr          = BMS_FIRMWARE_ADDR;  // 固件存储基地址（Flash指定地址）
            _gstResponsePacketData.error_code = ACK_OK;       // 响应错误码：成功
            firmware_checksum_rec       = 0;                  // 重置接收的校验和
            firmware_checksum_compute   = 0;                  // 重置计算的校验和
            updata_pack_size = *((uint32_t*)&p_request_packet->data_field[0]);  // 读取升级总包数
            space_num = *((uint8_t*)&p_request_packet->data_field[4]);          // 读取无效数据长度
            pack_count = 0;                                   // 重置包计数器

            /* 升级期间暂停AFE采集，避免Flash操作与数据采集冲突 */
            StopAFESample();

            break;
        }
        case UPGRADE_BUSY:  // 固件升级数据传输命令（升级进行中）
        {
            _gstResponsePacketData.error_code = ACK_OK;  // 默认响应成功

            /* 处理第一包升级数据（包含固件信息头） */
            if((base_firmware_addr == BMS_FIRMWARE_ADDR) && (pack_count == 0))
            {
                // 校验固件类型是否匹配（必须与gBMSFirmwareType一致）
                if(memcmp(&p_request_packet->data_field[2], gBMSFirmwareType, 4) == 0)
                {
                    base_firmware_addr = BMS_FIRMWARE_ADDR;  // 类型匹配，确认存储地址
                }
                else
                {
                    _gstResponsePacketData.error_code = ACK_ERROR;  // 类型不匹配，返回错误
                    return false;  // 终止命令处理
                }

                /* 从第一包数据中提取固件信息 */
                memcpy(gBMSHardVersion, &p_request_packet->data_field[6], 3);  // 固件版本号（3字节）
                gBMSHardwareLen = *((uint32_t*)&p_request_packet->data_field[9]);  // 固件总长度
                firmware_checksum_rec = *((uint32_t*)&p_request_packet->data_field[254]);  // 固件校验和（254字节偏移处）
            }

            /* 升级包序号（预留代码，未使用） */
//            num_index = *((uint16_t*)&p_request_packet->data_field[0]);

            /* 写入固件数据到Flash：跳过无效字节，擦除后写入 */
            spi_flash_write_bytes_with_erase(base_firmware_addr, 
                                            &p_request_packet->data_field[space_num], 
                                            p_request_packet->data_field_len - space_num);

            /* 读取已写入的Flash数据，用于校验 */
            spi_flash_read_bytes(base_firmware_addr, 
                               _ProtocolBuf, 
                               p_request_packet->data_field_len - space_num);

            /* 计算固件校验和 */
            if((base_firmware_addr == BMS_FIRMWARE_ADDR) && (pack_count == 0))
            {
                // 第一包数据：跳过前256字节（固件信息头），计算有效数据校验和
                firmware_checksum_compute += CheckSumFun(&_ProtocolBuf[256], 
                                                       p_request_packet->data_field_len - 256 - space_num, 
                                                       1);  // 32位校验和
            }
            else
            {
                // 非第一包：计算整包数据校验和
                firmware_checksum_compute += CheckSumFun(_ProtocolBuf, 
                                                       p_request_packet->data_field_len - space_num, 
                                                       1);
            }

            /* 更新固件存储地址（偏移已写入数据长度） */
            base_firmware_addr += p_request_packet->data_field_len - space_num;
            pack_count++;  // 包计数器自增

            break;
        }
        case UPGRADE_END:  // 固件升级结束命令
        {
            /* 升级完成，恢复AFE采集 */
            StartAFESample();
            _gstResponsePacketData.error_code = ACK_OK;  // 默认响应成功

            /* 校验固件完整性：计算的校验和 vs 接收的校验和 */
            if(firmware_checksum_compute != firmware_checksum_rec)
            {
                _gstResponsePacketData.error_code = ACK_ERROR;  // 校验失败，返回错误
            }
            else
            {
                /* 校验成功，写入升级标志（Flash指定地址） */
                uint8_t firmware_upgrade[256];
                memset(firmware_upgrade, 0xA0, 256);  // 填充升级标志位（0xA0）
                spi_flash_write_bytes_with_erase(UPGRADE_FLAG_ADDR, firmware_upgrade, 256);
                _gstResponsePacketData.error_code = ACK_OK;  // 响应成功
            }

            break;
        }
        case READ_ALLBMS_DATA:  // 读取所有BMS数据命令
        {
            uint8_t i = 0;  // BMS设备ID（默认0）
            // 发送缓冲区数据字段起始地址：跳过响应包头部
            uint8_t  *p_addr  = (uint8_t *)garrComTxBuf + sizeof(ProtocalReadResponsePacket_t);
            uint16_t  read_reg_size = sizeof(BMSPara_t);  // 读取的寄存器总长度（BMS参数结构体大小）

            /* 配置响应包数据长度：BMS参数长度 + 3字节（ID+长度） */
            _gstReadResponsePacketData.data_field_len = (sizeof(BMSPara_t) + 3);

            /* 填充数据字段内容 */
            memcpy(&p_addr[0], (uint8_t*)&i, 1);          // 第0字节：BMS设备ID
            memcpy(&p_addr[1], (uint8_t*)&read_reg_size, 2);  // 第1-2字节：数据长度
            memcpy(&p_addr[3], (uint8_t*)&garrBMSReg.bms_only_read_para, sizeof(BMSPara_t));  // 第3字节起：BMS只读参数

            break;
        }
        case WRITE_ALLBMS_DATA:  // 写入所有BMS保持参数命令
        {
            /* 将请求包数据字段复制到BMS保持参数结构体 */
            memcpy((uint8_t*)&garrBMSReg.bms_hold_para, 
                   &p_request_packet->data_field[0], 
                   p_request_packet->data_field_len);

            break;
        }
        case WRITE_PARTBMS_DATA:  // 写入部分BMS数据命令
        {
            uint16_t reg_start_addr = 0;  // 寄存器起始地址（从数据字段提取）

            /* 提取寄存器起始地址（数据字段第1-2字节） */
            reg_start_addr = *((uint16_t*)&p_request_packet->data_field[1]);

            /* 写入部分参数：从指定地址开始，写入对应长度的数据 */
            memcpy((uint8_t*)((uint8_t*) & (garrBMSReg.bms_only_read_para.model_id) + reg_start_addr), 
                   &p_request_packet->data_field[5],  // 数据字段起始偏移（第5字节）
                   p_request_packet->data_field_len - 5);  // 写入数据长度

            break;
        }
        default:  // 未知命令：不处理
            break;
    }

    return true;  // 命令解析成功
}

/**
*******************************************************************************
* Function:    ProtoUnpackJudge
* @brief:      协议解包与合法性判断函数
* @details:    从接收队列中提取数据帧，进行解包、校验和验证，调用命令解析函数
*              根据命令类型发送对应响应包，是通信协议的核心入口函数
* Parameters:  @param [in] p: 接收数据结构体指针（包含数据缓冲区和长度）
* @return:     true-解包成功（无论命令是否支持） false-解包失败（参数无效）
* @note        1. 先验证输入参数有效性，再进行解包
*              2. 解包后验证校验和，校验失败则返回错误响应
*              3. 校验成功后调用命令解析函数，再根据命令类型发送响应
*******************************************************************************
*/
bool ProtoUnpackJudge(ComRevData_t *p)
{
    uint16_t checksum      = 0;  // 计算的校验和（用于验证）

    /* 输入参数有效性检查 */
    if(p == NULL || p->pData == NULL)
    {
        return false;  // 参数无效，解包失败
    }

    /* 解包：从接收缓冲区提取请求包信息 */
    memcpy(&_gstRequestPacketData, p->pData, 7);  // 复制前7字节（头部+地址+命令+数据长度）
    _gstRequestPacketData.data_field = _ProtocolBuf;  // 数据字段指针指向协议缓冲区
    // 提取校验和（接收数据最后2字节，大端模式转小端）
    _gstRequestPacketData.check_sum = (uint16_t)p->pData[p->data_size - 2] + 
                                     (((uint16_t)p->pData[p->data_size - 1]) << 8);
    // 复制数据字段到协议缓冲区（跳过前7字节头部）
    memcpy(_ProtocolBuf, (uint8_t*)(p->pData + 7), _gstRequestPacketData.data_field_len);

    /* 校验和验证：计算接收数据（除最后2字节校验和）的校验和 */
    checksum = CheckSumFun(p->pData, (p->data_size - 2), 0);

    if(_gstRequestPacketData.check_sum != checksum)
    {
        /* 校验和不匹配，返回错误响应 */
        _gstResponsePacketData.error_code = ACK_ERROR;
    }
    else
    {
        /* 校验和匹配，调用命令解析函数 */
        _CmdAnalysis(&_gstRequestPacketData);
    }

    /* 根据命令类型发送对应响应包 */
    if((_gstRequestPacketData.cmd == UPGRADE_START) || 
       (_gstRequestPacketData.cmd == UPGRADE_BUSY) || 
       (_gstRequestPacketData.cmd == UPGRADE_END))
    {
        UpgradeDataSendPolling();  // 升级相关命令：发送升级响应
    }
    else if(_gstRequestPacketData.cmd == READ_ALLBMS_DATA)
    {
        AllBMSDataSendPolling(garrComTxBuf);  // 读取数据命令：发送所有BMS数据
    }
    else if(_gstRequestPacketData.cmd == WRITE_ALLBMS_DATA)
    {
//        DataSendPolling(garrComTxBuf);  // 预留：写入所有数据响应（当前未启用）
    }
    else if(_gstRequestPacketData.cmd == WRITE_PARTBMS_DATA)
    {
        WriteBMSDataSendPolling();  // 写入部分数据命令：发送写入响应
    }

    return true;  // 解包流程完成（无论命令是否成功执行）
}

