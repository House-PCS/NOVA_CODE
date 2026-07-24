/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "modbus_analysis.h"  // Modbus协议解析核心头文件

/************************** 宏定义 **************************/
#define CAPACITY_STD    (314)  // 电池包标准容量基准值（单位：Ah，实际使用时需结合倍率换算）
// 位检查宏：提取data的第number位（0-7），返回0或1
#define BITCHECK(data,number)           ((data >> number) & (0x01))
/************************** 全局变量 **************************/
static uint8_t _ModbusAckData[300];  // Modbus响应数据缓冲区（最大支持300字节，适配多寄存器读写）
static uint16_t _ModbusAckSize;      // Modbus响应数据长度（字节数）
BMSPara_t garrBMSReg = {0};          // BMS核心寄存器结构体（包含只读参数和保持寄存器，初始化清零）

/*********************************************************************
 * 函数名：_ModbusReadInput
 * 功能：处理Modbus读输入寄存器指令（功能码0x04）
 * 参数：
 *      modbus_type：Modbus通信类型（EN_MODBUS_RTU/TCP）
 *      reg_start_addr：起始寄存器地址（16位）
 *      reg_num：读取的寄存器数量
 *      slave_addr：从机地址（1-247）
 * 返回值：无
 * 说明：
 *      1. 输入寄存器为只读，通常存储实时采集数据（如电压/电流/温度）
 *      2. 响应格式：从机地址 + 功能码(0x04) + 数据字节数 + 寄存器数据 + CRC/LRC
 *      3. 错误响应：功能码最高位置1(0x84) + 错误码
 *********************************************************************/
static void _ModbusReadInput(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr)
{
    uint8_t error = 0;          // 错误码（0=无错误，非0=对应错误类型）
    uint8_t offset = 0;         // 数据偏移量（RTU=0，TCP=6，适配不同协议头长度）

    // 根据Modbus类型设置数据偏移量（TCP协议需预留6字节MBAP头空间）
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }

    _ModbusAckData[offset] = slave_addr;  // 响应帧：从机地址

    /************************** 数据正确性判断（用户需根据业务扩展）**************************/
    // 示例：可添加寄存器地址合法性检查（如是否超出输入寄存器地址范围）
    // if(reg_start_addr < INPUT_REG_MIN || reg_start_addr + reg_num > INPUT_REG_MAX) error = EN_MODBUS_ERROR_CODE_INVALID_ADDR;
    /****************************************************************************************/

    if(error == 0)  // 无错误：正常响应数据
    {
        _ModbusAckData[offset + 1] = 0x04;                  // 功能码：读输入寄存器
        _ModbusAckData[offset + 2] = reg_num * 2;           // 数据字节数（1个寄存器=2字节）
        /************************** 寄存器数据填充（用户需根据业务实现）**************************/
        // 要求：按寄存器定义顺序，通过memcpy将数据写入响应缓冲区
        // 示例：memcpy(&_ModbusAckData[offset+3], &input_regs[reg_start_addr], reg_num*2);
        /****************************************************************************************/
        // 响应长度 = 数据字节数 + 3（地址+功能码+字节数） + 偏移量
        _ModbusAckSize = reg_num * 2 + 3 + offset;
    }
    else  // 有错误：返回错误响应
    {
        _ModbusAckData[offset + 1] = 0x84;  // 错误功能码（原功能码|0x80）
        _ModbusAckData[offset + 2] = error; // 错误码（如无效地址、无效数据等）
        _ModbusAckSize = 3 + offset;        // 错误响应长度：地址+错误功能码+错误码 + 偏移量
    }
}

/*********************************************************************
 * 函数名：_ModbusReadRemain
 * 功能：处理Modbus读保持寄存器指令（功能码0x03）
 * 参数：
 *      modbus_type：Modbus通信类型（EN_MODBUS_RTU/TCP）
 *      reg_start_addr：起始寄存器地址（16位）
 *      reg_num：读取的寄存器数量
 *      slave_addr：从机地址（1-247）
 *      error_code：外部传入的错误码（如指令格式错误）
 * 返回值：无
 * 说明：
 *      1. 保持寄存器支持读写，存储配置参数/状态数据（如阈值、故障码）
 *      2. 仅地址≤0x0AE9的寄存器支持读取，数据取自garrBMSReg.bms_only_read_para
 *      3. 响应格式与读输入寄存器一致，功能码为0x03
 *********************************************************************/
static void _ModbusReadRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t error = 0;          // 错误码
    uint8_t offset = 0;         // 数据偏移量（RTU=0，TCP=6）
    uint8_t slave_reg_serial = 0;  // 预留：从机寄存器序列号（未使用，可根据扩展需求启用）

    error = error_code;  // 接收外部传入的错误码（如指令长度错误）

    // 设置协议偏移量
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }

    _ModbusAckData[offset] = slave_addr;  // 响应帧：从机地址

    if(error == 0)  // 无错误：正常读取保持寄存器
    {
        _ModbusAckData[offset + 1] = 0x03;                  // 功能码：读保持寄存器
        _ModbusAckData[offset + 2] = reg_num * 2;           // 数据字节数
        /************************** 寄存器数据填充（按地址范围读取）**************************/
        // 仅处理地址≤0x0AE9的寄存器，数据取自只读参数结构体（偏移量=寄存器地址-MODBUS_HARDWARE_VERSION）
        if(reg_start_addr <= 0x0AE9)
        {
            memcpy(&_ModbusAckData[offset + 3],
                   (uint8_t*)&garrBMSReg.bms_only_read_para + ((reg_start_addr - MODBUS_HARDWARE_VERSION) * 2),
                   reg_num * 2);
        }
        /****************************************************************************************/
        _ModbusAckSize = reg_num * 2 + 3 + offset;  // 响应长度
    }
    else  // 有错误：返回错误响应
    {
        _ModbusAckData[offset + 1] = 0x83;  // 错误功能码（0x03|0x80）
        _ModbusAckData[offset + 2] = error; // 错误码
        _ModbusAckSize = 3 + offset;        // 错误响应长度
    }
}

/*********************************************************************
 * 函数名：_ModbusWriteSingleRemain
 * 功能：处理Modbus写单个保持寄存器指令（功能码0x06）
 * 参数：
 *      modbus_type：Modbus通信类型（EN_MODBUS_RTU/TCP）
 *      reg_start_addr：目标寄存器地址（16位）
 *      reg_data：要写入的寄存器数据（16位）
 *      slave_addr：从机地址（1-247）
 * 返回值：无
 * 说明：
 *      1. 写入成功后，响应帧需回显写入的寄存器地址和数据
 *      2. 错误响应功能码为0x86，携带对应错误码
 *********************************************************************/
static void _ModbusWriteSingleRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_data, uint8_t slave_addr)
{
    uint8_t error = 0;          // 错误码
    uint8_t offset = 0;         // 数据偏移量（RTU=0，TCP=6）

    // 设置协议偏移量
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }

    _ModbusAckData[offset] = slave_addr;  // 响应帧：从机地址

    /************************** 数据正确性判断（用户需根据业务扩展）**************************/
    // 示例：可添加寄存器地址合法性（是否为可写保持寄存器）、数据范围检查
    // if(reg_start_addr < HOLD_REG_MIN || reg_start_addr > HOLD_REG_MAX) error = EN_MODBUS_ERROR_CODE_INVALID_ADDR;
    // if(reg_data > REG_DATA_MAX) error = EN_MODBUS_ERROR_CODE_INVALID_DATA;
    /****************************************************************************************/

    if(error == 0)  // 无错误：执行写操作并响应
    {
        _ModbusAckData[offset + 1] = 0x06;                  // 功能码：写单个保持寄存器
        _ModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;  // 目标寄存器地址（高字节）
        _ModbusAckData[offset + 3] = reg_start_addr & 0xFF;         // 目标寄存器地址（低字节）
        _ModbusAckData[offset + 4] = (reg_data >> 8) & 0xFF;         // 写入的数据（高字节）
        _ModbusAckData[offset + 5] = reg_data & 0xFF;                // 写入的数据（低字节）
        _ModbusAckSize = 4 + 2 + offset;  // 响应长度：地址+功能码(2字节) + 地址(2字节) + 数据(2字节) + 偏移量
    }
    else  // 有错误：返回错误响应
    {
        _ModbusAckData[offset + 1] = 0x86;  // 错误功能码（0x06|0x80）
        _ModbusAckData[offset + 2] = error; // 错误码
        _ModbusAckSize = 3 + offset;        // 错误响应长度
    }
}

/*********************************************************************
 * 函数名：_ModbusWriteMultiRemain
 * 功能：处理Modbus写多个保持寄存器指令（功能码0x10）
 * 参数：
 *      modbus_type：Modbus通信类型（EN_MODBUS_RTU/TCP）
 *      reg_start_addr：起始寄存器地址（16位）
 *      p_reg_data：待写入数据缓冲区（大端格式）
 *      reg_num：写入的寄存器数量
 *      slave_addr：从机地址（1-247）
 *      error_code：外部传入的错误码（如指令格式/数据长度错误）
 * 返回值：无
 * 说明：
 *      1. 支持批量写入多个连续保持寄存器，数据需从大端转小端
 *      2. 仅地址≤0x0AE9的寄存器支持写入，数据存入保持参数结构体
 *      3. 响应帧回显起始地址和写入的寄存器数量
 *********************************************************************/
static void _ModbusWriteMultiRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint8_t *p_reg_data, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t rtc_flag = 0;              // 预留：RTC操作标志（未使用）
    uint8_t error = 0;                  // 错误码
    uint8_t offset = 0;                 // 数据偏移量（RTU=0，TCP=6）
    uint8_t i = 0;                      // 循环变量
    uint16_t value = 0;                 // 转换后的小端数据
    uint8_t slave_reg_serial = 0;       // 预留：从机寄存器序列号（未使用）
    uint32_t rtc_times = 0;             // 预留：RTC时间戳（未使用）
    uint8_t sendcontrolinfoenableflge = 0; // 预留：控制信息发送使能（未使用）

    error = error_code;  // 接收外部传入的错误码

    // 设置协议偏移量
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }

    _ModbusAckData[offset] = slave_addr;  // 响应帧：从机地址

    /************************** 数据正确性判断（用户需根据业务扩展）**************************/
    // 示例：添加寄存器地址范围检查、数据合法性检查
    // if(reg_start_addr + reg_num > 0x0AE9) error = EN_MODBUS_ERROR_CODE_INVALID_ADDR;
    /****************************************************************************************/

    if(error == 0)  // 无错误：执行批量写操作
    {
        /* 写寄存器操作：循环处理每个寄存器，大端转小端后写入保持参数结构体 */
        for (i = 0; i < reg_num; i++)
        {
            value = BEBufToUint16(&p_reg_data[2 * i]);  // 大端字节流转16位小端数据
            // 仅处理地址≤0x0AE9的寄存器，数据存入bms_hold_para（偏移量=寄存器地址-MODBUS_DSG_MOS_CTL）
            if(reg_start_addr <= 0x0AE9)
            {
                ((uint16_t*)&garrBMSReg.bms_hold_para)[reg_start_addr + i - MODBUS_DSG_MOS_CTL] = value;
            }
        }

        /* 设置阈值信息（用户可根据业务扩展，如写入的是阈值参数时执行生效逻辑）*/

        // 响应帧构造
        _ModbusAckData[offset + 1] = 0x10;                  // 功能码：写多个保持寄存器
        _ModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;  // 起始寄存器地址（高字节）
        _ModbusAckData[offset + 3] = reg_start_addr & 0xFF;         // 起始寄存器地址（低字节）
        _ModbusAckData[offset + 4] = (reg_num >> 8) & 0xFF;         // 写入的寄存器数量（高字节）
        _ModbusAckData[offset + 5] = reg_num & 0xFF;                // 写入的寄存器数量（低字节）
        _ModbusAckSize = 4 + 2 + offset;  // 响应长度：地址+功能码(2字节) + 地址(2字节) + 数量(2字节) + 偏移量
    }
    else  // 有错误：返回错误响应
    {
        _ModbusAckData[offset + 1] = 0x90;  // 错误功能码（0x10|0x80）
        _ModbusAckData[offset + 2] = error; // 错误码
        _ModbusAckSize = 3 + offset;        // 错误响应长度
    }
}

/************************** 全局状态变量 **************************/
static EN_MODBUS_REV_STATUS sta = EN_MODBUS_REV_STATUS_IDLE;  // Modbus接收状态机（初始为空闲）

/*********************************************************************
 * 函数名：ModbusCmdProcess
 * 功能：Modbus指令处理主函数（状态机驱动）
 * 状态机说明：
 *      EN_MODBUS_REV_STATUS_IDLE：空闲状态，等待接收指令
 *      EN_MODBUS_REV_STATUS_ANALYSIS：指令解析状态，识别功能码并分发处理
 *      EN_MODBUS_READ_MPPT：预留状态，MPPT相关读取（暂未实现完整逻辑）
 *      EN_MODBUS_REV_STATUS_ACK：响应发送状态，发送处理结果
 *      EN_MODBUS_REV_STATUS_ACK_WAIT：响应等待状态，等待发送完成（RTU模式）
 * 返回值：无
 * 核心逻辑：从接收队列获取指令，按功能码分发到对应处理函数，最后发送响应
 *********************************************************************/
void ModbusCmdProcess()
{
    static ModbusRTURevData_t *p_rtu_cmd;  // Modbus RTU指令缓冲区指针（静态变量，保持上下文）
    static void *p_cmd;                    // 通用指令指针（预留扩展）
    static uint8_t func_code;              // 指令功能码（0x03/0x04/0x06/0x10等）
    static uint32_t wait_time_count = 0;   // 预留：等待超时计数器（未使用）
    static uint32_t wait_max_time = 0;     // 预留：最大等待时间（未使用）
    static EN_MODBUS_TYPE modbus_type = EN_MODBUS_RTU;  // Modbus通信类型（默认RTU）
    static uint8_t slave_addr = 0;         // 从机地址（从指令中解析）
    uint8_t error_code = 0;                // 错误码（初始化无错误）
    ComRevData_t com_revdata ;             // 通用通信接收数据结构体

    memset((uint8_t *)&com_revdata, 0, sizeof(ComRevData_t));  // 清零接收结构体

    // 状态机驱动
    switch(sta)
    {
        case EN_MODBUS_REV_STATUS_IDLE:  // 空闲状态：等待接收指令
        {
            // 从RTU接收队列获取指令（队列有数据则返回true）
            if(QueueGet(&gModbusRTURevQueue, (void *)&p_rtu_cmd) == true)
            {
                if(p_rtu_cmd->data_type == TYPE_MODBUS_DATA) // 数据类型为Modbus指令
                {
                    func_code = p_rtu_cmd->pData[1];        // 提取功能码（RTU帧第2字节）
                    modbus_type = EN_MODBUS_RTU;            // 通信类型为RTU
                    slave_addr = p_rtu_cmd->slave_addr;     // 提取从机地址
                    sta = EN_MODBUS_REV_STATUS_ANALYSIS;    // 切换到指令解析状态

                    // 特殊判断：从机地址0x04且功能码0x10，强制进入解析状态（适配特定设备需求）
                    if((slave_addr == 0x04) && (func_code == 0x10))
                    {
                        sta = EN_MODBUS_REV_STATUS_ANALYSIS;
                    }
                }
                else if(p_rtu_cmd->data_type == TYPE_COM_DATA)  // 数据类型为通用通信数据
                {
                    // 填充通用接收结构体，调用协议解包判断函数
                    com_revdata.data_size = p_rtu_cmd->data_size;
                    com_revdata.pData = p_rtu_cmd->pData;
                    ProtoUnpackJudge(&com_revdata);  // 外部协议解包函数（非Modbus逻辑）
                    sta = EN_MODBUS_REV_STATUS_IDLE;  // 保持空闲状态
                }
            }
            break;
        }

        case EN_MODBUS_REV_STATUS_ANALYSIS:  // 指令解析状态：按功能码分发处理
        {
            switch(func_code)
            {
                case EN_MODBUS_FUNC_CODE_READ_INPUT:  // 功能码0x04：读输入寄存器
                {
                    uint16_t reg_start_addr;  // 起始寄存器地址
                    uint16_t reg_num;         // 读取寄存器数量

                    if(modbus_type == EN_MODBUS_RTU)  // RTU模式：解析地址和数量（大端转小端）
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];  // 地址：第3-4字节
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];        // 数量：第5-6字节
                    }
                    else  // TCP模式：预留扩展（需解析MBAP头后提取地址和数量）
                    {
                        /*后期增加MODBUS_TCP*/
                    }

                    // 调用输入寄存器读取处理函数
                    _ModbusReadInput(modbus_type, reg_start_addr, reg_num, slave_addr);
                    break;
                }

                case EN_MODBUS_FUNC_CODE_READ_REMAIN:  // 功能码0x03：读保持寄存器
                {
                    uint16_t reg_start_addr;  // 起始寄存器地址
                    uint16_t reg_num;         // 读取寄存器数量

                    if(modbus_type == EN_MODBUS_RTU)  // RTU模式：解析地址和数量
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];

                        // 指令格式检查：RTU读保持寄存器指令长度应为8字节（地址1+功能码1+地址2+数量2+CRC2）
                        // 寄存器数量限制：最大128个（Modbus协议规范，可根据硬件调整）
                        if((p_rtu_cmd->data_size != 8) || (reg_num > 128))
                        {
                            error_code = EN_MODBUS_ERROR_CODE_INVALID_CMD;  // 无效指令错误码
                        }
                    }
                    else  // TCP模式：预留扩展
                    {
                        /*后期增加MODBUS_TCP*/
                    }

                    // 调用保持寄存器读取处理函数
                    _ModbusReadRemain(modbus_type, reg_start_addr, reg_num, slave_addr, error_code);
                    break;
                }

                case EN_MODBUS_FUNC_CODE_WRITE_SINGLE:  // 功能码0x06：写单个保持寄存器
                {
                    uint16_t reg_start_addr;  // 目标寄存器地址
                    uint16_t reg_data;        // 待写入数据

                    if(modbus_type == EN_MODBUS_RTU)  // RTU模式：解析地址和数据
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];  // 地址：第3-4字节
                        reg_data = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];        // 数据：第5-6字节
                    }
                    else  // TCP模式：预留扩展
                    {
                        /*后期增加MODBUS_TCP*/
                    }

                    // 调用单个寄存器写入处理函数
                    _ModbusWriteSingleRemain(modbus_type, reg_start_addr, reg_data, slave_addr);
                    break;
                }

                case EN_MODBUS_FUNC_CODE_WRITE_MULTI:  // 功能码0x10：写多个保持寄存器
                {
                    uint16_t reg_start_addr;  // 起始寄存器地址
                    uint16_t _reg_num;        // 写入寄存器数量
                    uint8_t *p_reg_data = NULL;  // 待写入数据缓冲区指针
                    uint8_t byte_num;         // 数据字节数（RTU帧中指定）

                    if(modbus_type == EN_MODBUS_RTU)  // RTU模式：解析指令参数
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];  // 起始地址：第3-4字节
                        _reg_num = ((uint16_t)p_rtu_cmd->pData[4] << 8) | (uint16_t)p_rtu_cmd->pData[5];  // 数量：第5-6字节
                        byte_num = p_rtu_cmd->pData[6];  // 数据字节数：第7字节
                        p_reg_data = &p_rtu_cmd->pData[7];  // 数据缓冲区起始地址：第8字节开始

                        // 指令格式检查：
                        // 1. RTU写多个寄存器指令最小长度为11字节（地址1+功能码1+地址2+数量2+字节数1+数据n+CRC2）
                        // 2. 数据字节数必须等于寄存器数量×2（1个寄存器=2字节）
                        if((p_rtu_cmd->data_size < 11) || (byte_num != (2 * _reg_num)))
                        {
                            error_code = EN_MODBUS_ERROR_CODE_INVALID_CMD;  // 无效指令错误码
                        }
                    }
                    else  // TCP模式：预留扩展
                    {
                        /*后期增加MODBUS_TCP*/
                    }

                    // 调用多个寄存器写入处理函数
                    _ModbusWriteMultiRemain(modbus_type, reg_start_addr, p_reg_data, _reg_num, slave_addr, error_code);
                    sta = EN_MODBUS_REV_STATUS_ACK;  // 直接切换到响应状态
                    break;
                }

                default:  // 未知功能码：返回无效功能码错误
                {
                    uint8_t offset;

                    // 设置协议偏移量
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        offset = 0;
                    }
                    else
                    {
                        offset = 6;
                    }

                    _ModbusAckData[offset] = slave_addr;                  // 从机地址
                    _ModbusAckData[offset + 1] = func_code | 0x80;       // 错误功能码（原功能码|0x80）
                    _ModbusAckData[offset + 2] = EN_MODBUS_ERROR_CODE_INVALID_CMD;  // 错误码：无效指令
                    _ModbusAckSize = 3 + offset;  // 错误响应长度
                    break;
                }
            }
            sta = EN_MODBUS_REV_STATUS_ACK;  // 解析完成，切换到响应状态
            break;
        }

        case EN_MODBUS_READ_MPPT:  // 预留状态：MPPT相关读取（暂未实现完整逻辑）
        {
            // 固定响应数据（示例），实际需根据MPPT寄存器定义填充
            _ModbusAckData[0] = 0x03;  // 从机地址
            _ModbusAckData[1] = 0x10;  // 功能码
            _ModbusAckData[2] = 0x31;  // 数据字节数
            _ModbusAckData[3] = 0x04;  // 数据1
            _ModbusAckData[4] = 0x00;  // 数据2
            _ModbusAckData[5] = 0x01;  // 数据3
            _ModbusAckData[6] = 0x02;  // 数据4
            _ModbusAckData[7] = 0x00;  // 数据5
            _ModbusAckData[8] = 0x00;  // 数据6
            _ModbusAckSize = 9;        // 响应长度9字节
            sta = EN_MODBUS_REV_STATUS_ACK;  // 切换到响应状态
            break;
        }

        case EN_MODBUS_REV_STATUS_ACK:  // 响应状态：发送处理结果
        {
            if(modbus_type == EN_MODBUS_RTU)  // RTU模式：调用RTU发送函数
            {
                ModbusRTUSend(_ModbusAckData, _ModbusAckSize);  // 发送响应数据（含CRC校验）
                sta = EN_MODBUS_REV_STATUS_ACK_WAIT;  // 切换到响应等待状态
            }
            else  // TCP模式：预留扩展（调用TCP发送函数）
            {
                /*后期增加MODBUS_TCP*/
            }
            break;
        }

        case EN_MODBUS_REV_STATUS_ACK_WAIT:  // 响应等待状态：等待发送完成
        {
            if(modbus_type == EN_MODBUS_RTU)
            {
                // RTU模式发送完成后，直接返回空闲状态（可根据硬件增加发送完成判断）
                sta = EN_MODBUS_REV_STATUS_IDLE;
            }
            else  // TCP模式：无需等待，直接返回空闲
            {
                /*后期增加MODBUS_TCP,可不用等待直接跳入IDLE*/
            }
            break;
        }

        default:  // 异常状态：返回空闲
            break;
    }
}

/*********************************************************************
 * 函数名：BMSReadRegUpdate
 * 功能：更新BMS只读寄存器数据（从全局状态结构体同步到Modbus寄存器）
 * 参数：
 *      pPackStatus：电池包状态结构体指针（包含电压、电流、SOC、温度等核心数据）
 * 返回值：无
 * 说明：
 *      1. 只读寄存器数据来自BMS实时采集和计算结果，不可通过Modbus写入
 *      2. 数据同步包括：设备信息、电气参数、状态标志、故障码、时间戳等
 *      3. 部分参数需进行单位换算（如电流/电压放大10倍存储，适配16位寄存器精度）
 *********************************************************************/
static void BMSReadRegUpdate(PackStatus_t *pPackStatus)
{
    uint32_t rtc_times = 0;  // RTC时间戳（用于同步实时时间）

    /************************** 设备基础信息 **************************/
    garrBMSReg.bms_only_read_para.model_id = gSysInfo.model_id;  // 设备型号ID
    memcpy(garrBMSReg.bms_only_read_para.sn, gSysInfo.sn, 20);   // 设备序列号（20字节）
    garrBMSReg.bms_only_read_para.hardware_version = gSysInfo.hw_version;  // 硬件版本号

    garrBMSReg.bms_only_read_para.software_version_low = gSysInfo.sw_version;  // 软件版本号（低16位）
    garrBMSReg.bms_only_read_para.software_version_high = gSysInfo.sw_version << 16;  // 软件版本号（低16位）

    memcpy((uint8_t*)&garrBMSReg.bms_only_read_para.version_extend, (uint8_t*)&gSysInfo.ex_version, 12);  // 扩展版本信息（12字节）

    /************************** 电气核心参数（单位换算：适配寄存器存储精度）**************************/
    garrBMSReg.bms_only_read_para.pack_voltage = gstCellInfo.total_cell_voltage;  // 电池包总电压（单位：0.01V）
    garrBMSReg.bms_only_read_para.cell_current = (pPackStatus->pack_current / 100);  // 充放电电流（原单位：mA → 换算为0.1A，存储为整数）
    garrBMSReg.bms_only_read_para.current_power = (abs(garrBMSReg.bms_only_read_para.cell_current / 10) * (garrBMSReg.bms_only_read_para.pack_voltage / 100));  // 实时功率（单位：W）

    // SOC（State of Charge）状态：限制最大值为1000（对应100.0%，精度0.1%）
    if(pPackStatus->sox_info.soc_real * 10 >= 1000)
    {
        pPackStatus->sox_info.soc_real = 100;
    }
    garrBMSReg.bms_only_read_para.soc_real = pPackStatus->sox_info.soc_real * 10;  // 真实SOC（0.1%精度）
    if(pPackStatus->sox_info.soc_disp * 10 >= 1000)
    {
        pPackStatus->sox_info.soc_disp = 100;
    }
    garrBMSReg.bms_only_read_para.soc_rpt = pPackStatus->sox_info.soc_disp * 10;  // 显示SOC（0.1%精度）

    garrBMSReg.bms_only_read_para.pack_health = pPackStatus->soh_rpt;  // SOH（State of Health）健康度（0.1%精度）
    // 最大充放电电流（原单位：mA → 换算为0.01A）
    garrBMSReg.bms_only_read_para.pack_max_chg_current = (pPackStatus->cur_max_chg_current / 100);
    garrBMSReg.bms_only_read_para.pack_max_dsg_current = (pPackStatus->cur_max_dsg_current / 100);
    // 容量参数（单位：0.01Ah）
    garrBMSReg.bms_only_read_para.cell_std_capa_ah = CAPACITY_STD * 10;  // 标准容量（CAPACITY_STD×10 → 0.01Ah精度）
    garrBMSReg.bms_only_read_para.capacity_ah = pPackStatus->cr / 100;  // 额定容量
    garrBMSReg.bms_only_read_para.pack_res_capa_ah = pPackStatus->res_capa / 100;  // 剩余容量
    garrBMSReg.bms_only_read_para.pack_cycles = pPackStatus->pack_cycles;  // 充放电循环次数

    /************************** 累计充放电能量（32位数据拆分为2个16位寄存器）**************************/
    garrBMSReg.bms_only_read_para.charging_energy_high = (uint32_t)pPackStatus->chg_accumulate_capa_wh >> 16;  // 累计充电能量（高16位，单位：mWh）
    garrBMSReg.bms_only_read_para.charging_energy_low = (uint16_t)pPackStatus->chg_accumulate_capa_wh;         // 累计充电能量（低16位）
    garrBMSReg.bms_only_read_para.discharge_energy_high = (uint32_t)pPackStatus->dsg_accumulate_capa_wh >> 16; // 累计放电能量（高16位）
    garrBMSReg.bms_only_read_para.discharge_energy_low = (uint16_t)pPackStatus->dsg_accumulate_capa_wh;        // 累计放电能量（低16位）

    /************************** 状态标志 **************************/
    // MOS管状态：bit0=放电MOS使能，bit1=充电MOS使能（通过位或运算组合）
    garrBMSReg.bms_only_read_para.mos_status = gFaultAnalysisFlag.dischargemos_enable_flag | (gFaultAnalysisFlag.chargemos_enable_flag << 1);

    /************************** 电压/温度参数（多电芯均衡状态）**************************/
    garrBMSReg.bms_only_read_para.cell_max_voltage = gstCellInfo.cell_max_voltage;  // 电芯最高电压（mV）
    garrBMSReg.bms_only_read_para.cell_min_voltage = gstCellInfo.cell_min_voltage;  // 电芯最低电压（mV）
    garrBMSReg.bms_only_read_para.cell_average_voltage = gstCellInfo.cell_ave_voltage;  // 电芯平均电压（mV）
    // 温度参数（单位：0.1℃）
    garrBMSReg.bms_only_read_para.cell_max_temp = gstCellInfo.cell_max_temperature;  // 电芯最高温度
    garrBMSReg.bms_only_read_para.cell_min_temp = gstCellInfo.cell_min_temperature;  // 电芯最低温度
    garrBMSReg.bms_only_read_para.cell_ave_temp = gstCellInfo.cell_ave_temperature;  // 电芯平均温度
    garrBMSReg.bms_only_read_para.mos_max_temp = gstCellInfo.mos_max_temperature;    // MOS管最高温度
    garrBMSReg.bms_only_read_para.mos_min_temp = gstCellInfo.mos_min_temperature;    // MOS管最低温度
    garrBMSReg.bms_only_read_para.mos_ave_temp = gstCellInfo.mos_ave_temperature;    // MOS管平均温度
    garrBMSReg.bms_only_read_para.pack_max_temp = gstCellInfo.pack_max_temperature;  // 电池包最高温度
    garrBMSReg.bms_only_read_para.pack_min_temp = gstCellInfo.pack_min_temperature;  // 电池包最低温度
    garrBMSReg.bms_only_read_para.pack_ave_temp = gstCellInfo.pack_ave_temperature;  // 电池包平均温度

    /************************** 电芯均衡状态（低8位）**************************/
    // 组合8个电芯的均衡状态（arrCellBalSts[0]→bit0，arrCellBalSts[1]→bit1，...，arrCellBalSts[7]→bit7）
    garrBMSReg.bms_only_read_para.balance_status_low = ((uint16_t)(gstCellInfo.arrCellBalSts[0]) |
            (uint16_t)(gstCellInfo.arrCellBalSts[1]) << 1 |
            (uint16_t)(gstCellInfo.arrCellBalSts[2]) << 2 |
            (uint16_t)(gstCellInfo.arrCellBalSts[3]) << 3 |
            (uint16_t)(gstCellInfo.arrCellBalSts[4]) << 4 |
            (uint16_t)(gstCellInfo.arrCellBalSts[5]) << 5 |
            (uint16_t)(gstCellInfo.arrCellBalSts[6]) << 6 |
            (uint16_t)(gstCellInfo.arrCellBalSts[7]) << 7);



    /************************** 故障/报警/通知码 **************************/
    garrBMSReg.bms_only_read_para.fltLevel = error_level;  // 故障等级（全局变量，由故障分析模块更新）
    // 电芯电压数组（16个电芯，单位：mV）
    memcpy((uint8_t*)&garrBMSReg.bms_only_read_para.cell_vdc, (uint8_t*)&gstCellInfo.cell_voltage, 16);
    // 电芯温度数组（18个温度点，单位：0.1℃）
    memcpy((uint8_t*)&garrBMSReg.bms_only_read_para.cell_temp, (uint8_t*)&gstCellInfo.cell_temperature, 18);
    // 故障码（MOS/温度/电流/电压/SOX/其他）
    /************************** 上传给监控卡的故障/报警/通知码 做处理  只上传最高级故障 三级优先二级优先一级**************************/

    /************************** 充电电流故障**************************/
    if(gPackErrorInfo.curr_error_code.b.charge_overcurr3)
    {
        gPackErrorInfo.curr_error_code.b.charge_overcurr2 = 0;
        gPackErrorInfo.curr_alarm_code.b.charge_overcurr1 = 0;
    }
    else if(gPackErrorInfo.curr_error_code.b.charge_overcurr2)
    {
        gPackErrorInfo.curr_alarm_code.b.charge_overcurr1 = 0;
    }
    /************************** 放电电流故障**************************/
    if(gPackErrorInfo.curr_error_code.b.discharge_overcurr3)
    {
        gPackErrorInfo.curr_error_code.b.discharge_overcurr2 = 0;
        gPackErrorInfo.curr_alarm_code.b.discharge_overcurr1 = 0;
    }
    else if(gPackErrorInfo.curr_error_code.b.discharge_overcurr2)
    {
        gPackErrorInfo.curr_alarm_code.b.discharge_overcurr1 = 0;
    }
    /************************** 过压故障**************************/
    if(gPackErrorInfo.cell1_error_code.b.cell_over_vol3)
    {
        gPackErrorInfo.cell1_error_code.b.cell_over_vol2 = 0;
        gPackErrorInfo.cell1_alarm_code.b.cell_over_vol1 = 0;
    }
    else if(gPackErrorInfo.cell1_error_code.b.cell_over_vol2)
    {
        gPackErrorInfo.cell1_alarm_code.b.cell_over_vol1 = 0;
    }
    /************************** 欠压故障**************************/
    if(gPackErrorInfo.cell1_error_code.b.cell_under_vol3)
    {
        gPackErrorInfo.cell1_error_code.b.cell_under_vol2 = 0;
        gPackErrorInfo.cell1_alarm_code.b.cell_under_vol1 = 0;
    }
    else if(gPackErrorInfo.cell1_error_code.b.cell_under_vol2)
    {
        gPackErrorInfo.cell1_alarm_code.b.cell_under_vol1 = 0;
    }

    /************************** 过温故障**************************/
    if(gPackErrorInfo.temp_error_code.b.cell_high_temper3)
    {
        gPackErrorInfo.temp_error_code.b.cell_high_temper2 = 0;
        gPackErrorInfo.temp_alarm_code.b.cell_high_temper1 = 0;
    }
    else if(gPackErrorInfo.temp_error_code.b.cell_high_temper2)
    {
        gPackErrorInfo.temp_alarm_code.b.cell_high_temper1 = 0;
    }
    /************************** 欠温故障**************************/
    if(gPackErrorInfo.temp_error_code.b.cell_low_temper3)
    {
        gPackErrorInfo.temp_error_code.b.cell_low_temper2 = 0;
        gPackErrorInfo.temp_alarm_code.b.cell_low_temper1 = 0;
    }
    else if(gPackErrorInfo.temp_error_code.b.cell_low_temper2)
    {
        gPackErrorInfo.temp_alarm_code.b.cell_low_temper1 = 0;
    }

    /************************** SOH故障**************************/
    if(gPackErrorInfo.sox_error_code.b.pack_low_soh3)
    {
        gPackErrorInfo.sox_error_code.b.pack_low_soh2 = 0;
        gPackErrorInfo.sox_alarm_code.b.pack_low_soh1 = 0;
    }
    if(gPackErrorInfo.sox_error_code.b.pack_low_soh2)
    {
        gPackErrorInfo.sox_alarm_code.b.pack_low_soh1 = 0;
    }

    garrBMSReg.bms_only_read_para.mos_error = gPackErrorInfo.mos_error_code.value;
    garrBMSReg.bms_only_read_para.temp_error = gPackErrorInfo.temp_error_code.value;
    garrBMSReg.bms_only_read_para.curr_error = gPackErrorInfo.curr_error_code.value;
    garrBMSReg.bms_only_read_para.volt_error = gPackErrorInfo.cell1_error_code.value;
    garrBMSReg.bms_only_read_para.sox_error = gPackErrorInfo.sox_error_code.value;
    garrBMSReg.bms_only_read_para.other_error = gPackErrorInfo.other_error_code.value;
    // 报警码
    garrBMSReg.bms_only_read_para.mos_alarm = gPackErrorInfo.mos_alarm_code.value;
    garrBMSReg.bms_only_read_para.temp_alarm = gPackErrorInfo.temp_alarm_code.value;
    garrBMSReg.bms_only_read_para.curr_alarm = gPackErrorInfo.curr_alarm_code.value;
    garrBMSReg.bms_only_read_para.volt_alarm = gPackErrorInfo.cell1_alarm_code.value;
    garrBMSReg.bms_only_read_para.sox_alarm = gPackErrorInfo.sox_alarm_code.value;
    garrBMSReg.bms_only_read_para.other_alarm = gPackErrorInfo.other_alarm_code.value;
    // 通知码
    garrBMSReg.bms_only_read_para.mos_notice = gPackErrorInfo.mos_notice_code.value;
    garrBMSReg.bms_only_read_para.temp_notice = gPackErrorInfo.temp_notice_code.value;
    garrBMSReg.bms_only_read_para.curr_notice = gPackErrorInfo.curr_notice_code.value;
    garrBMSReg.bms_only_read_para.volt_notice = gPackErrorInfo.cell1_notice_code.value;
    garrBMSReg.bms_only_read_para.sox_notice = gPackErrorInfo.sox_notice_code.value;
    garrBMSReg.bms_only_read_para.other_notice = gPackErrorInfo.other_notice_code.value;



    /************************** RTC时间戳（32位拆分存储）**************************/
    rtc_times = RtcGetTime();  // 获取RTC时间戳（单位：秒，自1970年1月1日起）
    garrBMSReg.bms_hold_para.rtc_times_high = rtc_times >> 16;      // 时间戳高16位
    garrBMSReg.bms_hold_para.rtc_times_low = (uint16_t)rtc_times;   // 时间戳低16位
}

/*********************************************************************
 * 函数名：UpdateDebugReg
 * 功能：更新调试寄存器数据（用于UART调试输出，10ms更新一次）
 * 参数：
 *      pPackStatus：电池包状态结构体指针
 * 返回值：无
 * 说明：
 *      1. 调试寄存器数据来源于BMS只读寄存器，部分参数需二次换算（如温度÷10转为℃）
 *      2. 固定10ms更新周期（通过TimeDiffFromNow判断时间差）
 *      3. 用于调试工具实时查看核心参数，不影响Modbus通信
 *********************************************************************/
void UpdateDebugReg(PackStatus_t *pPackStatus)
{
    static uint32_t update_debugreg_time = 0;  // 上次更新时间戳（静态变量，保持周期）

    // 10ms更新一次调试寄存器（TimeDiffFromNow返回当前与上次的时间差，单位：ms）
    if(TimeDiffFromNow(update_debugreg_time) > 10)
    {
        update_debugreg_time = TimGetSystime();  // 更新当前时间戳

        /************************** 调试寄存器数据同步 **************************/
        memcpy(&garrUartDebugReg.cell_vlt[0], &gstCellInfo.cell_voltage, 16);  // 电芯电压数组（mV）
        garrUartDebugReg.cell_max_vlt = garrBMSReg.bms_only_read_para.cell_max_voltage;  // 电芯最高电压（mV）
        garrUartDebugReg.cell_min_vlt = garrBMSReg.bms_only_read_para.cell_min_voltage;  // 电芯最低电压（mV）
        garrUartDebugReg.cell_sum_vlt = garrBMSReg.bms_only_read_para.pack_voltage * 10;  // 总电压0.01v

        // 电芯温度数组（0.1℃→℃，取整数）
        for(uint8_t i = 0; i < 10; i++)
        {
            garrUartDebugReg.cell_temp[i] = (garrBMSReg.bms_only_read_para.cell_temp[i]) / 10;
        }
        garrUartDebugReg.cell_max_temp = garrBMSReg.bms_only_read_para.cell_max_temp / 10;  // 最高温度（℃）
        garrUartDebugReg.cell_min_temp = garrBMSReg.bms_only_read_para.cell_min_temp / 10;  // 最低温度（℃）
        garrUartDebugReg.afe_temp = gstCellInfo.afe_net_temperature / 10;  // AFE芯片温度（℃）
        garrUartDebugReg.mos_temp = 25;  // MOS管温度（暂固定为25℃，实际需替换为实时采集值）
        garrUartDebugReg.ambient_temp = 25;  // 环境温度（暂固定为25℃，实际需替换为实时采集值）
        garrUartDebugReg.pack_current = (garrBMSReg.bms_only_read_para.cell_current * 100);  // 充放电电流（0.01A→mA）
        garrUartDebugReg.current_power = garrBMSReg.bms_only_read_para.current_power;  // 实时功率（mW）
        garrUartDebugReg.heat_status = garrBMSReg.bms_only_read_para.heat_status;// 加热膜状态
        garrUartDebugReg.cell_soc_max_rlt = pPackStatus->sox_info.soc_max * 10;  // 最大SOC（0.1%精度）
        garrUartDebugReg.cell_soc_min_rlt = pPackStatus->sox_info.soc_min;        // 最小SOC
        garrUartDebugReg.pack_soc_rlt = garrBMSReg.bms_only_read_para.soc_real;    // 真实SOC（0.1%精度）
        garrUartDebugReg.pack_soc_dsp = garrBMSReg.bms_only_read_para.soc_rpt;     // 显示SOC（0.1%精度）
        garrUartDebugReg.pack_health = garrBMSReg.bms_only_read_para.pack_health;  // SOH健康度（0.1%精度）
        garrUartDebugReg.pack_status = pPackStatus->mode;  // 电池包工作模式（充电/放电/静置等）
        garrUartDebugReg.recharge_time = pPackStatus->recharge_times * 3600;  // 累计充电时间（秒）
        garrUartDebugReg.mos_status = garrBMSReg.bms_only_read_para.mos_status;  // MOS管状态
        garrUartDebugReg.balance_status = garrBMSReg.bms_only_read_para.balance_status_low;  // 均衡状态
        garrUartDebugReg.flt_lvl = error_level;  // 故障等级
        memcpy(&garrUartDebugReg.flt_dtc[0], &gBMSErrorNumCode.cell_under_vol1_error_num, 38);  // 故障DTC码数组（38字节）
        garrUartDebugReg.max_chg_current = garrBMSReg.bms_only_read_para.pack_max_chg_current * 100;  // 最大充电电流（0.01A→mA）
        garrUartDebugReg.max_dsg_current = garrBMSReg.bms_only_read_para.pack_max_dsg_current * 100;  // 最大放电电流（0.01A→mA）
        garrUartDebugReg.max_dsg_30s = (pPackStatus->cur_max_dsg_current_30s);  // 30秒最大放电电流（mA）
        garrUartDebugReg.max_dsg_60s = (pPackStatus->cur_max_dsg_current_60s);  // 60秒最大放电电流（mA）
    }
}

/*********************************************************************
 * 函数名：UpdateModbusReg
 * 功能：Modbus寄存器更新主函数（包含调试模式切换、系统控制逻辑）
 * 返回值：bool类型，始终返回true（表示更新成功）
 * 核心逻辑：
 *      1. 根据调试模式选择更新的状态结构体（正常模式/调试模式）
 *      2. 处理系统控制指令：MCU复位、SOC校准、恢复出厂、关机
 *      3. 控制指令执行后需清零标志位，避免重复执行
 *********************************************************************/
bool UpdateModbusReg(void)
{
    static uint8_t poweroffcount = 0;    // 预留：关机计数器（未使用）
    static uint8_t sys_reset_count = 0;  // 系统复位计数器（限制最大5次，防止死循环）
    static uint8_t turnoff_count = 0;    // 关机计数器（限制最大5次，防止死循环）

    // 根据SOX调试模式，选择更新的电池包状态结构体
    if(gSoxDebugParam.sox_debug_mode)
    {
        BMSReadRegUpdate(&gPackStatus_debug);  // 调试模式：更新调试状态结构体
        UpdateDebugReg(&gPackStatus_debug);     // 更新调试寄存器
    }
    else
    {
        BMSReadRegUpdate(&gPackStatus);         // 正常模式：更新实际状态结构体
        UpdateDebugReg(&gPackStatus);           // 更新调试寄存器
    }

    /************************** 系统复位控制（保持寄存器sys_reset置1触发）**************************/
    if((garrBMSReg.bms_hold_para.sys_reset) && (sys_reset_count < 5))
    {
        if(gFirst_AFE_Coved_Flag == 3)
        {
            StopAFESample();
            SoxFlashInfoSave();  // 关机前保存SOX参数
            SoxEepromInfoSave();
            DevFeedExtWdg();       // 喂外部看门狗
            soft_delayms(2000);            
            ResetProcess();      // 执行MCU复位（关闭中断+系统复位）
            CloseAllMOS();       // 关闭所有MOS管（停止充放电）
            sys_reset_count++;   // 计数器自增（限制5次复位）
        }
        garrBMSReg.bms_hold_para.sys_reset = 0;  // 清零复位标志位（避免重复触发）
    }

    /************************** SOC校准控制（保持寄存器force_ocv置1触发）**************************/
    if(garrBMSReg.bms_hold_para.force_ocv)
    {
        OCV_Test();          // 执行OCV（开路电压）测试，校准SOC
        SoxFlashInfoSave();  // 保存校准后的SOX参数
        SoxEepromInfoSave();
        garrBMSReg.bms_hold_para.force_ocv = 0;  // 清零校准标志位
    }

    /************************** 恢复出厂设置（保持寄存器reset_factory=0x5AA5触发）**************************/
    if(garrBMSReg.bms_hold_para.reset_factory == 0x5AA5)
    {
        ResetFactory(&gPackStatus);  // 执行恢复出厂操作
        garrBMSReg.bms_hold_para.reset_factory = 0;  // 清零出厂设置标志位
    }

    /************************** BMS关机控制（保持寄存器bms_shutdown位3=1、位2=0触发）**************************/
    if((BITCHECK(garrBMSReg.bms_hold_para.bms_shutdown, 3) == 1) &&
            (BITCHECK(garrBMSReg.bms_hold_para.bms_shutdown, 2) == 0) &&
            (turnoff_count < 5))
    {
        StopAFESample();
        SoxFlashInfoSave();  // 关机前保存SOX参数
        SoxEepromInfoSave();
        CloseAllMOS();       // 关闭所有MOS管（停止充放电）
        BMSBoardPowerOff();  // 切断BMS主板电源
        turnoff_count++;     // 计数器自增（限制5次关机）
    }

    return true;  // 返回更新成功
}

/*********************************************************************
 * 函数名：ResetProcess
 * 功能：MCU系统复位实现函数
 * 返回值：无
 * 说明：
 *      1. 复位前喂狗（避免复位过程中看门狗超时）
 *      2. 关闭总中断（防止复位过程中被中断打断）
 *      3. 调用NVIC_SystemReset()执行系统复位（CMSIS标准函数）
 *********************************************************************/
void ResetProcess()
{
    __set_PRIMASK(1);      // 关闭总中断（PRIMASK=1，禁止所有可屏蔽中断）
    NVIC_SystemReset();    // 触发MCU系统复位
}

/*********************************************************************
 * 函数名：ModbusRegInit
 * 功能：Modbus寄存器初始化（设置默认阈值参数）
 * 返回值：无
 * 说明：
 *      1. 清零BMS核心寄存器结构体和历史故障状态结构体
 *      2. 设置电压/温度/电流/SOH等保护阈值（默认值，可通过Modbus修改）
 *      3. 阈值单位说明：电压(mV)、温度(0.1℃)、电流(0.01A)、差值(0.1℃/mV)
 *********************************************************************/
void ModbusRegInit(void)
{
    // 清零BMS核心寄存器和历史故障状态结构体
    memset((uint8_t *)&garrBMSReg, 0, sizeof(BMSPara_t));
    memset((uint8_t *)&gHistoryErrorStatus, 0, sizeof(HistoryErrorStatus_t));

    /************************** 电池包电压保护阈值**************************/
    garrBMSReg.bms_only_read_para.pack_ov_lvl_low = 2960;   // 包过压低等级阈值（29.6V）
    garrBMSReg.bms_only_read_para.pack_ov_lvl_high = 0;    // 包过压高等级阈值（预留，未启用）
    garrBMSReg.bms_only_read_para.pack_uv_lvl_low = 2080;   // 包欠压低等级阈值（20.8V）
    garrBMSReg.bms_only_read_para.pack_uv_lvl_high = 0;    // 包欠压高等级阈值（预留）

    /************************** 电芯电压保护阈值（mV）**************************/
    garrBMSReg.bms_only_read_para.cell_ov_lvl1 = 3650;     // 电芯过压1级（3.65V）
    garrBMSReg.bms_only_read_para.cell_ov_lvl2 = 3700;     // 电芯过压2级（3.70V）
    garrBMSReg.bms_only_read_para.cell_ov_lvl3 = 3750;     // 电芯过压3级（3.75V）
    garrBMSReg.bms_only_read_para.cell_uv_lvl1 = 2600;     // 电芯欠压1级（2.60V）
    garrBMSReg.bms_only_read_para.cell_uv_lvl2 = 2550;     // 电芯欠压2级（2.55V）
    garrBMSReg.bms_only_read_para.cell_uv_lvl3 = 2000;     // 电芯欠压3级（2.00V）

    /************************** 温度保护阈值（0.1℃）**************************/
    garrBMSReg.bms_only_read_para.cell_ot_lvl1 = 580;      // 电芯过温1级（58.0℃）
    garrBMSReg.bms_only_read_para.cell_ot_lvl2 = 600;      // 电芯过温2级（60.0℃）
    garrBMSReg.bms_only_read_para.cell_ot_lvl3 = 650;      // 电芯过温3级（65.0℃）
    garrBMSReg.bms_only_read_para.cell_ut_lvl = (-100);    // 电芯欠温1级（-10.0℃）
    garrBMSReg.bms_only_read_para.cell_ut_lv2 = (-200);    // 电芯欠温2级（-20.0℃）
    garrBMSReg.bms_only_read_para.cell_ut_lv3 = (-300);    // 电芯欠温3级（-30.0℃）
    garrBMSReg.bms_only_read_para.mos_ot_lvl1 = 850;       // MOS管过温1级（85.0℃）
    garrBMSReg.bms_only_read_para.mos_ot_lvl2 = 900;       // MOS管过温2级（90.0℃）
    garrBMSReg.bms_only_read_para.mos_ot_lvl3 = 1000;      // MOS管过温3级（100.0℃）

    /************************** 过流保护阈值（0.01A）**************************/
    garrBMSReg.bms_only_read_para.dsg_oc_lvl1 = 110;       // 放电过流1级（1.10）
    garrBMSReg.bms_only_read_para.dsg_oc_lvl2 = 115;       // 放电过流2级（1.15）
    garrBMSReg.bms_only_read_para.dsg_oc_lvl3 = 120;       // 放电过流3级（1.20）
    garrBMSReg.bms_only_read_para.chg_oc_lvl1 = 110;       // 充电过流1级（1.10）
    garrBMSReg.bms_only_read_para.chg_oc_lvl2 = 115;       // 充电过流2级（1.15）
    garrBMSReg.bms_only_read_para.chg_oc_lvl3 = 120;       // 充电过流3级（1.20）

    /************************** 均衡阈值（mV/0.1℃）**************************/
    garrBMSReg.bms_only_read_para.cell_voltage_diff = 400; // 电芯电压差均衡阈值（400mV）
    garrBMSReg.bms_only_read_para.cell_temp_diff = 100;    // 电芯温度差阈值（10.0℃）

    /************************** SOH保护阈值（0.1%）**************************/
    garrBMSReg.bms_only_read_para.soh_under_lvl1 = 800;    // SOH低于80.0%（1级报警）
    garrBMSReg.bms_only_read_para.soh_under_lvl2 = 750;    // SOH低于75.0%（2级报警）
    garrBMSReg.bms_only_read_para.soh_under_lvl3 = 700;    // SOH低于70.0%（3级报警）

}