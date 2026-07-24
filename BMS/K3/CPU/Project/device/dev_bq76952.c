/**
******************************************************************************
* @copyright (C) COPYRIGHT Nova
* @file        dev_bq76952.c
* @author      Xi'an NovaStar Tech Co.,LTD.
* @version     V1.2.0
* @date        2025-06-1
* @brief       BQ76952电池管理芯片驱动实现
*              包含I2C通信、寄存器配置、状态读取、电压/电流/温度测量、OTP编程等功能
******************************************************************************
* @attention
* 1. 芯片通信：支持硬件I2C(BQHWI2C)和模拟I2C(SimI2C)两种模式，通过宏定义切换
* 2. 数据校验：支持CRC8校验(CRC_Mode)，确保通信可靠性
* 3. 配置模式：修改RAM寄存器需先进入CONFIG_UPDATE模式，修改后需写校验和与长度
* 4. OTP编程：需先解锁至全访问模式，编程后需验证状态位
* @relation    依赖头文件：dev_bq76952.h、systick.h（延时函数）
* @par 修改记录:
* <table>
* <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
* <tr><td>2024-08-16    <td>V1.0.0    <td>Xingwen Xu  <td>初始版本，实现基础功能
* <tr><td>2024-08-28    <td>V1.1.0    <td>Xingwen Xu  <td>对齐代码规范，优化注释
* <tr><td>2024-09-30    <td>V1.2.0    <td>Xingwen Xu  <td>新增OTP编程相关代码
* </table>
******************************************************************************
*/

/* Private includes ----------------------------------------------------------*/
#include "dev_bq76952.h"  // BQ76952芯片专用头文件（包含寄存器地址、宏定义、数据结构）

/* USER CODE BEGIN PV */
/**
 * @brief  2字节接收缓冲区
 * @details 用于存储从BQ769x2读取的短数据（如电压、状态字等）
 */
uint8_t RX_data[2] = {0x00, 0x00};

/**
 * @brief  32字节接收缓冲区
 * @details 用于存储长数据（如DASTATUS寄存器组、OTP数据等）
 */
uint8_t RX_32Byte[32] = {0x00};  // 初始化所有元素为0

/* Private function prototypes -----------------------------------------------*/
static void _DevAfeCopyArray(uint8_t *source, uint8_t *dest, uint8_t count);    // 数组拷贝
static unsigned char _DevAfeChecksum(unsigned char *ptr, unsigned char len);    // 计算校验和（RAM写操作专用）
static unsigned char _DevAfeCRC8(unsigned char *ptr, unsigned char len);         // 计算CRC8校验码
static bool _DevAfeI2CWriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count); // I2C写寄存器
static bool _DevAfeI2CReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count);  // I2C读寄存器
static bool _DevAfeSetRegister(uint8_t dev_addr, uint16_t reg_addr, uint32_t reg_data, uint8_t datalen); // 配置寄存器（含校验和）
static uint16_t _DevAfeReadVoltage(uint8_t dev_addr, uint8_t command, bool *com_sts); // 读取电压（电芯/堆叠/ PACK/LD）
static void _DevAfeChange2FullAccess(uint8_t dev_addr, bool *comm_sts);          // 切换至全访问模式（OTP编程前置）

/* Function implement */
/**
*******************************************************************************
* Function:  DevAfedelaymS
* @brief:    毫秒级延时函数
* @warning   函数设计用于毫秒级延时，不适合微秒级高精度延时
* @details:  调用systick.c中的soft_delayms实现，依赖系统滴答定时器配置
* Parameters:
* @param [in]  ms: 延时时间（单位：毫秒）
* @return:     None
*******************************************************************************
*/
void DevAfedelaymS(uint32_t ms)
{
    soft_delayms(ms);  // 外部延时函数，需确保systick已初始化
}

/**
*******************************************************************************
* Function:  _DevAfeCopyArray
* @brief:    数组拷贝（字节级）
* @details:  将源数组的前count个字节复制到目标数组，无内存重叠检查
* Parameters:
* @param [in]  *source: 源数组指针（输入）
* @param [out] *dest:   目标数组指针（输出）
* @param [in]  count:   拷贝字节数（需确保目标数组有足够空间）
* @return:     None
*******************************************************************************
*/
static void _DevAfeCopyArray(uint8_t *source, uint8_t *dest, uint8_t count)
{
    uint8_t copyIndex = 0;
    // 循环拷贝每个字节
    for(copyIndex = 0; copyIndex < count; copyIndex++)
    {
        dest[copyIndex] = source[copyIndex];
    }
}

/**
*******************************************************************************
* Function:  _DevAfeChecksum
* @brief:    计算校验和（RAM寄存器写操作专用）
* @details:  校验和算法：字节累加和的按位取反（仅低8位）
*            用于BQ76952 RAM寄存器写操作时的完整性校验
* Parameters:
* @param [in]  *ptr: 待校验数据数组指针
* @param [in]  len:  待校验数据长度（字节数）
* @return:     8位校验和（0x00~0xFF）
*******************************************************************************
*/
static unsigned char _DevAfeChecksum(unsigned char *ptr, unsigned char len)
{
    unsigned char i;
    unsigned char checksum = 0;
    // 累加所有字节
    for(i = 0; i < len; i++)
    {
        checksum += ptr[i];
    }
    checksum = 0xff & ~checksum;  // 按位取反（低8位）
    return (checksum);
}

/**
*******************************************************************************
* Function:  _DevAfeCRC8
* @brief:    计算CRC8校验码
* @details:  多项式：0x107（等效于x^8 + x^2 + x + 1），初始值：0x00
*            用于I2C通信数据的完整性校验（CRC_Mode开启时）
* Parameters:
* @param [in]  *ptr: 待校验数据数组指针
* @param [in]  len:  待校验数据长度（字节数）
* @return:     8位CRC校验码（0x00~0xFF）
*******************************************************************************
*/
static unsigned char _DevAfeCRC8(unsigned char *ptr, unsigned char len)
{
    unsigned char i;
    unsigned char crc = 0;  // 初始值
    while(len-- != 0)
    {
        for(i = 0x80; i != 0; i /= 2)  // 按位处理（从最高位到最低位）
        {
            if((crc & 0x80) != 0)      // 最高位为1时移位并异或多项式
            {
                crc *= 2;
                crc ^= 0x107;
            }
            else                        // 最高位为0时仅移位
                crc *= 2;
            
            if((*ptr & i) != 0)         // 数据位为1时异或多项式
                crc ^= 0x107;
        }
        ptr++;  // 处理下一个字节
    }
    return (crc);
}

/**
*******************************************************************************
* Function:  _DevAfeI2CWriteReg
* @brief:    I2C写寄存器（底层通信接口）
* @details:  支持CRC校验模式和普通模式，自动适配硬件/模拟I2C
*            注意：驱动层返回true表示成功，本函数返回值取反（true=失败，false=成功）
* Parameters:
* @param [in]  dev_addr:  从机地址（7位地址，不含读写位）
* @param [in]  reg_addr:  寄存器地址（8位）
* @param [in]  *reg_data: 待写数据数组指针
* @param [in]  count:     待写数据长度（字节数）
* @return:     true:  写操作失败
*              false: 写操作成功
*******************************************************************************
*/
static bool _DevAfeI2CWriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
    uint8_t TX_Buffer[MAX_BUFFER_SIZE] = {0x00};  // 发送缓冲区（MAX_BUFFER_SIZE需在头文件定义）
    bool return_value = false;                    // 返回值（默认失败）
    DevFeedExtWdg();                              // 喂外部看门狗（防止系统复位）
    
#if CRC_Mode  // 开启CRC校验模式
    uint8_t crc1stByteBuffer[3] = {0x10, reg_addr, reg_data[0]};  // 第一个字节的CRC计算缓冲区
    unsigned int j;
    unsigned int i;
    uint8_t temp_crc_buffer[3];
    
    // 第一个数据字节 + 其CRC
    TX_Buffer[0] = reg_data[0];
    TX_Buffer[1] = _DevAfeCRC8(crc1stByteBuffer, 3);
    j = 2;  // 缓冲区指针偏移
    
    // 后续数据字节：每个字节后跟其CRC
    for(i = 1; i < count; i++)
    {
        TX_Buffer[j] = reg_data[i];
        j++;
        temp_crc_buffer[0] = reg_data[i];
        TX_Buffer[j] = _DevAfeCRC8(temp_crc_buffer, 1);
        j++;
    }
    
    // 配置I2C句柄（BQ76952专用句柄）
    gI2CBq76952HandleType.pTxBuf = &TX_Buffer;    // 发送缓冲区指针
    gI2CBq76952HandleType.reg_addr = reg_addr;    // 寄存器地址
    gI2CBq76952HandleType.tx_size = count;        // 数据长度（不含CRC字节，驱动层自动处理）
    
    // 执行I2C写操作（硬件/模拟I2C切换）
#if BQHWI2C
    return_value = !I2CRegWrite(&gI2CBq76952HandleType);  // 硬件I2C写
#else
    return_value = !SimI2CRegWrite(&gI2CBq76952HandleType); // 模拟I2C写
#endif

#else  // 普通模式（无CRC校验）
    // 直接配置I2C句柄
    gI2CBq76952HandleType.pTxBuf = reg_data;
    gI2CBq76952HandleType.reg_addr = reg_addr;
    gI2CBq76952HandleType.tx_size = count;
    
    // 执行I2C写操作
#if BQHWI2C
    return_value = !I2CRegWrite(&gI2CBq76952HandleType);
#else
    return_value = !SimI2CRegWrite(&gI2CBq76952HandleType);
#endif
#endif

    return return_value;
}

/**
*******************************************************************************
* Function:  _DevAfeI2CReadReg
* @brief:    I2C读寄存器（底层通信接口）
* @details:  支持CRC校验模式和普通模式，自动适配硬件/模拟I2C
*            注意：驱动层返回true表示成功，本函数返回值取反（true=失败，false=成功）
* Parameters:
* @param [in]  dev_addr:   从机地址（7位地址，不含读写位）
* @param [in]  reg_addr:   寄存器地址（8位）
* @param [out] *reg_data:  读取数据存储数组指针
* @param [in]  count:      待读数据长度（字节数）
* @return:     true:  读操作失败
*              false: 读操作成功
*******************************************************************************
*/
static bool _DevAfeI2CReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
    unsigned int RX_CRC_Fail = 0;  // CRC校验失败计数器
    uint8_t RX_Buffer[MAX_BUFFER_SIZE] = {0x00};  // 接收缓冲区
    bool return_value = false;                    // 返回值（默认失败）
    
#if CRC_Mode  // 开启CRC校验模式
    uint8_t crc1stByteBuffer[4] = {0x10, reg_addr, 0x11, 0};  // 第一个字节的CRC计算缓冲区
    unsigned int j;
    unsigned int i;
    unsigned char CRCc = 0;
    uint8_t temp_crc_buffer[3];
    uint8_t crc_count = count * 2;  // 接收总长度（数据+CRC，每个数据字节对应1字节CRC）
    
    // 配置I2C句柄
    gI2CBq76952HandleType.pRxBuf = reg_data;
    gI2CBq76952HandleType.reg_addr = reg_addr;
    gI2CBq76952HandleType.rx_size = count;
    
    // 执行I2C读操作
#if BQHWI2C
    return_value = !I2CRegRead(&gI2CBq76952HandleType);  // 硬件I2C读
#else
    return_value = !SimI2CRegRead(&gI2CBq76952HandleType); // 模拟I2C读
#endif
    
    // 校验第一个数据字节的CRC
    crc1stByteBuffer[3] = *(uint8_t *)gI2CBq76952HandleType.pRxBuf;  // 第一个数据字节
    CRCc = _DevAfeCRC8(crc1stByteBuffer, 4);                         // 计算CRC
    if(CRCc != *(uint8_t *)(gI2CBq76952HandleType.pRxBuf + 1))       // 对比接收的CRC字节
    {
        RX_CRC_Fail++;  // CRC校验失败
    }
    RX_Buffer[0] = *(uint8_t *)gI2CBq76952HandleType.pRxBuf;  // 存储有效数据
    j = 2;  // 跳过第一个CRC字节
    
    // 校验后续数据字节的CRC
    for(i = 1; i < count; i++)
    {
        RX_Buffer[i] = *(uint8_t *)(gI2CBq76952HandleType.pRxBuf + j);  // 读取数据字节
        temp_crc_buffer[0] = RX_Buffer[i];
        j++;
        CRCc = _DevAfeCRC8(temp_crc_buffer, 1);                         // 计算CRC
        if(CRCc != *(uint8_t *)(gI2CBq76952HandleType.pRxBuf + j))       // 对比CRC字节
        {
            RX_CRC_Fail++;
        }
        j++;  // 跳过当前CRC字节
    }
    
    // 将有效数据拷贝到输出缓冲区（剔除CRC字节）
    _DevAfeCopyArray(RX_Buffer, reg_data, crc_count);

#else  // 普通模式（无CRC校验）
    // 配置I2C句柄
    gI2CBq76952HandleType.pRxBuf = reg_data;
    gI2CBq76952HandleType.reg_addr = reg_addr;
    gI2CBq76952HandleType.rx_size = count;
    
    // 执行I2C读操作
#if BQHWI2C
    return_value = !I2CRegRead(&gI2CBq76952HandleType);
#else
    return_value = !SimI2CRegRead(&gI2CBq76952HandleType);
#endif
#endif

    return return_value;
}

/**
*******************************************************************************
* Function:  _DevAfeSetRegister
* @brief:    配置BQ76952 RAM寄存器（含校验和与长度写入）
* @details:  遵循BQ76952寄存器写协议：
*            1. 将寄存器地址+数据写入0x3E寄存器
*            2. 将校验和+总长度写入0x60寄存器
*            支持1/2/4字节数据长度，数据采用小端格式存储
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [in]  reg_addr:  目标寄存器地址（16位）
* @param [in]  reg_data:  待写数据（32位，根据datalen截取有效字节）
* @param [in]  datalen:   数据长度（ONE_BYTE_LEN/TWO_BYTE_LEN/FOUR_BYTE_LEN）
* @return:     true:  配置失败
*              false: 配置成功
*******************************************************************************
*/
static bool _DevAfeSetRegister(uint8_t dev_addr, uint16_t reg_addr, uint32_t reg_data, uint8_t datalen)
{
    uint8_t TX_Buffer[2] = {0x00};  // 校验和+长度缓冲区
    uint8_t TX_RegData[6] = {0x00}; // 寄存器地址+数据缓冲区（最大6字节：16位地址+32位数据）
    bool return_value = false;
    DevFeedExtWdg();  // 喂看门狗
    
    // 寄存器地址按小端格式存储（低字节在前）
    TX_RegData[0] = reg_addr & 0xff;         // 地址低8位
    TX_RegData[1] = (reg_addr >> 8) & 0xff;  // 地址高8位
    TX_RegData[2] = reg_data & 0xff;         // 数据第1字节（低8位）
    
    // 根据数据长度填充剩余字节
    switch(datalen)
    {
        case ONE_BYTE_LEN:  // 1字节数据
            // 写入地址+1字节数据（共3字节）到0x3E
            return_value = _DevAfeI2CWriteReg(dev_addr, 0x3E, TX_RegData, 3);
            // 计算校验和（3字节：地址2字节+数据1字节）
            TX_Buffer[0] = _DevAfeChecksum(TX_RegData, 3);
            TX_Buffer[1] = 0x05;  // 总长度（地址2字节+数据1字节=3字节？此处按芯片要求填写0x05）
            // 写入校验和+长度到0x60
            return_value = _DevAfeI2CWriteReg(dev_addr, 0x60, TX_Buffer, 2);
            break;
            
        case TWO_BYTE_LEN:  // 2字节数据
            TX_RegData[3] = (reg_data >> 8) & 0xff;  // 数据第2字节（高8位）
            // 写入地址+2字节数据（共4字节）到0x3E
            return_value = _DevAfeI2CWriteReg(dev_addr, 0x3E, TX_RegData, 4);
            // 计算校验和（4字节）
            TX_Buffer[0] = _DevAfeChecksum(TX_RegData, 4);
            TX_Buffer[1] = 0x06;  // 总长度（地址2字节+数据2字节=4字节？按芯片要求填写0x06）
            return_value = _DevAfeI2CWriteReg(dev_addr, 0x60, TX_Buffer, 2);
            break;
            
        case FOUR_BYTE_LEN:  // 4字节数据
            TX_RegData[3] = (reg_data >> 8) & 0xff;   // 数据第2字节
            TX_RegData[4] = (reg_data >> 16) & 0xff;  // 数据第3字节
            TX_RegData[5] = (reg_data >> 24) & 0xff;  // 数据第4字节（最高8位）
            // 写入地址+4字节数据（共6字节）到0x3E
            return_value = _DevAfeI2CWriteReg(dev_addr, 0x3E, TX_RegData, 6);
            // 计算校验和（6字节）
            TX_Buffer[0] = _DevAfeChecksum(TX_RegData, 6);
            TX_Buffer[1] = 0x08;  // 总长度（地址2字节+数据4字节=6字节？按芯片要求填写0x08）
            return_value = _DevAfeI2CWriteReg(dev_addr, 0x60, TX_Buffer, 2);
            break;
    }
    return return_value;
}

/**
*******************************************************************************
* Function:  DevAfeCommandSubcommands
* @brief:    发送子命令（仅写操作，无数据）
* @details:  子命令为16位，通过0x3E寄存器间接发送（BQ76952命令协议）
*            特殊说明：DEEPSLEEP/SHUTDOWN子命令需连续调用2次才能生效
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [in]  command:   16位子命令（小端格式传输）
* @return:     true:  发送成功
*              false: 发送失败
*******************************************************************************
*/
bool DevAfeCommandSubcommands(uint8_t dev_addr, uint16_t command)
{
    DevFeedExtWdg();
    uint8_t TX_Reg[2] = {0x00};  // 子命令缓冲区（小端格式）
    bool return_value = false;
    
    // 子命令按小端格式存储
    TX_Reg[0] = command & 0xff;         // 子命令低8位
    TX_Reg[1] = (command >> 8) & 0xff;  // 子命令高8位
    
    // 通过0x3E寄存器发送子命令
    return_value = _DevAfeI2CWriteReg(dev_addr, 0x3E, TX_Reg, 2);
    return return_value;
}

/**
*******************************************************************************
* Function:  DevAfeSubcommands
* @brief:    发送子命令（支持读/写操作，带数据）
* @warning   安全密钥和厂商数据写入不支持此函数（读取支持）
* @details:  子命令协议：
*            读操作(R)：1. 写子命令到0x3E；2. 等待0x3E寄存器值非0xFF；3. 从0x40读取数据
*            写操作(W)：1. 写子命令+1字节数据到0x3E；2. 写校验和+长度到0x60
*            写操作(W2)：1. 写子命令+2字节数据到0x3E；2. 写校验和+长度到0x60
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [in]  command:   16位子命令
* @param [in]  data:      待写数据（读操作时忽略）
* @param [in]  type:      操作类型（R=读，W=写1字节，W2=写2字节）
* @param [in]  count:     读数据长度（写操作时忽略，最大32字节）
* @return:     true:  操作成功
*              false: 操作失败
*******************************************************************************
*/
bool DevAfeSubcommands(uint8_t dev_addr, uint16_t command, uint16_t data, uint8_t type, uint8_t count)
{
    uint8_t TX_Reg[4] = {0x00};  // 子命令+数据缓冲区（最大4字节：16位命令+16位数据）
    uint8_t TX_Buffer[2] = {0x00};  // 校验和+长度缓冲区
    bool return_value = false;
    
    // 子命令按小端格式存储
    TX_Reg[0] = command & 0xff;
    TX_Reg[1] = (command >> 8) & 0xff;
    
    if(type == R)  // 读操作
    {
        // 写子命令到0x3E
        return_value = _DevAfeI2CWriteReg(dev_addr, 0x3E, TX_Reg, 2);
        // 等待芯片接收完成（0x3E寄存器值非0xFF）
        do
        {
            return_value = _DevAfeI2CReadReg(dev_addr, 0x3E, RX_32Byte, 2);
        } while(RX_32Byte[0] == 0xFF);
        // 从0x40寄存器读取数据
        return_value = _DevAfeI2CReadReg(dev_addr, 0x40, RX_32Byte, count);
    }
    else if(type == W)  // 写1字节数据
    {
        TX_Reg[2] = data & 0xff;  // 1字节数据
        // 写子命令+数据到0x3E
        return_value = _DevAfeI2CWriteReg(dev_addr, 0x3E, TX_Reg, 3);
        // 计算校验和（3字节：命令2字节+数据1字节）
        TX_Buffer[0] = _DevAfeChecksum(TX_Reg, 3);
        TX_Buffer[1] = 0x05;  // 总长度（命令2+数据1=3字节，按芯片要求填写0x05）
        // 写校验和+长度到0x60
        return_value = _DevAfeI2CWriteReg(dev_addr, 0x60, TX_Buffer, 2);
    }
    else if(type == W2)  // 写2字节数据
    {
        TX_Reg[2] = data & 0xff;         // 数据低8位
        TX_Reg[3] = (data >> 8) & 0xff;  // 数据高8位
        // 写子命令+数据到0x3E
        return_value = _DevAfeI2CWriteReg(dev_addr, 0x3E, TX_Reg, 4);
        // 计算校验和（4字节：命令2字节+数据2字节）
        TX_Buffer[0] = _DevAfeChecksum(TX_Reg, 4);
        TX_Buffer[1] = 0x06;  // 总长度（命令2+数据2=4字节，按芯片要求填写0x06）
        // 写校验和+长度到0x60
        return_value = _DevAfeI2CWriteReg(dev_addr, 0x60, TX_Buffer, 2);
    }
    return return_value;
}

/**
*******************************************************************************
* Function:  DevAfeDirectCommands
* @brief:    发送直接命令（1字节命令，支持读/写）
* @details:  直接命令通过7位I2C地址空间直接访问，数据为16位（小端格式）
*            读操作：从命令地址读取2字节数据到RX_data
*            写操作：向命令地址写入2字节数据
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [in]  command:   8位直接命令
* @param [in]  data:      待写数据（读操作时忽略）
* @param [in]  type:      操作类型（R=读，W=写）
* @return:     true:  操作成功
*              false: 操作失败
*******************************************************************************
*/
bool DevAfeDirectCommands(uint8_t dev_addr, uint8_t command, uint16_t data, uint8_t type)
{
    uint8_t TX_data[2] = {0x00};  // 写数据缓冲区（小端格式）
    bool return_value = false;
    
    // 数据按小端格式存储
    TX_data[0] = data & 0xff;
    TX_data[1] = (data >> 8) & 0xff;
    
    if(type == R)  // 读操作
    {
        // 从命令地址读取2字节数据到全局缓冲区RX_data
        return_value = _DevAfeI2CReadReg(dev_addr, command, RX_data, 2);
    }
    if(type == W)  // 写操作
    {
        // 向命令地址写入2字节数据（适用于控制状态、报警状态等16位寄存器）
        return_value = _DevAfeI2CWriteReg(dev_addr, command, TX_data, 2);
    }
    return return_value;
}

/**
*******************************************************************************
* Function:  DevAfeI2CInit
* @brief:    初始化BQ76952 I2C通信参数
* @details:  配置I2C从机地址和寄存器地址宽度（固定8位）
* Parameters:
* @param [in]  dev_addr:  从机地址（7位地址，不含读写位）
* @return:     None
*******************************************************************************
*/
void DevAfeI2CInit(uint8_t dev_addr)
{
    gI2CBq76952HandleType.slave_addr0 = dev_addr;  // 设置从机地址
    gI2CBq76952HandleType.reg_addr_width = EN_I2C_REG_ADDR_WIDTH_8;  // 寄存器地址宽度8位
}

/**
*******************************************************************************
* Function:  DevAfeRegInit
* @brief:    初始化BQ76952所有RAM寄存器（唤醒后必须执行）
* @details:  初始化流程：
*            1. 进入CONFIG_UPDATE模式（子命令0x0090）
*            2. 配置电源、引脚、保护阈值、均衡等参数
*            3. 退出CONFIG_UPDATE模式
*            所有配置宏（如POWER_CFG）需在dev_bq76952.h中定义
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [in]  *pIC:      AFE芯片数据结构指针（存储状态和配置）
* @return:     None
*******************************************************************************
*/
void DevAfeRegInit(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    DevFeedExtWdg();  // 喂看门狗
    
    // 1. 进入CONFIG_UPDATE模式（修改RAM寄存器必需）
    pIC->FltSts.CommFlt = DevAfeCommandSubcommands(dev_addr, SET_CFGUPDATE);
    
    // 2. 配置核心参数
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, PowerConfig, POWER_CFG, TWO_BYTE_LEN);          // 电源配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, REG0Config, REG0_CFG, ONE_BYTE_LEN);           // REG0引脚配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, REG12Config, REG12_CFG, ONE_BYTE_LEN);         // REG12引脚配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, CFETOFFPinConfig, CFETOFFPIN_CFG, ONE_BYTE_LEN); // 充电FET关断引脚配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, DFETOFFPinConfig, DFETOFF_PIN_CFG, ONE_BYTE_LEN); // 放电FET关断引脚配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, ALERTPinConfig, ALERT_PIN_CFG, ONE_BYTE_LEN);   // 报警引脚配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, TS1Config, TS1_CFG, ONE_BYTE_LEN);               // 温度传感器1配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, TS2Config, TS2_CFG, ONE_BYTE_LEN);               // 温度传感器2配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, TS3Config, TS3_CFG, ONE_BYTE_LEN);               // 温度传感器3配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, HDQPinConfig, HDQ_PIN_CFG, ONE_BYTE_LEN);       // HDQ引脚配置（无热敏电阻时设0x00）
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, VCellMode, VCELL_MODE_CFG, TWO_BYTE_LEN);        // 电芯电压测量模式
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, DAConfiguration, DA_CFG, ONE_BYTE_LEN);          // 数据采集配置
    
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, EnabledProtectionsA, PROTC_A_CFG, ONE_BYTE_LEN); // 保护使能A（过充/过放等）
    DevFeedExtWdg();
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, EnabledProtectionsB, PROTC_B_CFG, ONE_BYTE_LEN); // 保护使能B（短路等）
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, DefaultAlarmMask, ALM_MASK_CFG, TWO_BYTE_LEN);   // 默认报警掩码
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, BalancingConfiguration, BAL_CFG, ONE_BYTE_LEN);   // 电芯均衡配置
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, CellBalanceMaxCells, BAL_MAX_NUM_CFG, ONE_BYTE_LEN); // 最大均衡电芯数
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, CUVThreshold, CUV_THRES_CFG, ONE_BYTE_LEN);      // 欠压阈值
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, COVThreshold, COV_THRES_CFG, ONE_BYTE_LEN);      // 过压阈值

    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, CheckTime, COV_THRES_CFG, ONE_BYTE_LEN);      // 断线检测开路电路检测间隔
    
    DevFeedExtWdg();
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, OCCThreshold, 0x10, ONE_BYTE_LEN);      // 充电过流阈值 
    
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, OCD1Threshold, 0x10, ONE_BYTE_LEN);     // 放电过流1阈值（0x0A为示例值）OCD1Threshold
    
    
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, SCDThreshold, 0x05, ONE_BYTE_LEN);               // 短路阈值（0x05为示例值）
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, SCDDelay, SCD_DELAY_CFG, ONE_BYTE_LEN);           // 短路延时
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, SCDLLatchLimit, SCD_LATCH_LMT_CFG, ONE_BYTE_LEN); // 短路锁存限制
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, ShutdownCellVoltage, SHUT_DOWN_CELL_VOL_CFG, TWO_BYTE_LEN); // 关机电芯电压
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, CCGain, CC_GAIN_CFG, FOUR_BYTE_LEN);             // 电流校准增益1
    pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, CapacityGain, CAP_GAIN_CFG, FOUR_BYTE_LEN);      // 电流校准增益2
    
    // 3. 退出CONFIG_UPDATE模式
    pIC->FltSts.CommFlt = DevAfeCommandSubcommands(dev_addr, EXIT_CFGUPDATE);
}

//  ********************************* FET Control Commands  ***************************************
/**
*******************************************************************************
* Function:  DevAfeReadFETStatus
* @brief:    读取FET状态（充电/放电/预充/预放）
* @details:  通过直接命令FETStatus（地址待定）读取16位状态字，解析各FET状态位
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储FET状态（FetSts结构体）
* @return:     None
*******************************************************************************
*/
void DevAfeReadFETStatus(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    // 读取FET状态寄存器（命令地址需在头文件定义）
    DevAfeDirectCommands(dev_addr, FETStatus, 0x00, R);
    
    // 解析状态字（小端格式：RX_data[0]低8位，RX_data[1]高8位）
    pIC->FetSts.FETSts = (RX_data[1] << 8) | RX_data[0];  // 完整状态字
    pIC->FetSts.DSG = ((RX_data[0] & 0x04) >> 2);         // 放电FET状态（bit2）
    pIC->FetSts.CHG = (RX_data[0] & 0x01);                // 充电FET状态（bit0）
    pIC->FetSts.PCHG = ((RX_data[0] & 0x02) >> 1);        // 预充FET状态（bit1）
    pIC->FetSts.PDSG = ((RX_data[0] & 0x08) >> 3);        // 预放FET状态（bit3）
}
// ********************************* End of FET Control Commands *********************************

// ********************************* BQ769x2 Power Commands   *****************************************
/**
*******************************************************************************
* Function:  DevAfeGpo
* @brief:    配置GPO引脚状态（控制芯片电源模式）
* @details:  通过设置GPO引脚电平控制芯片进入关机模式或电源模式
*            GPO_SET：置1，进入关机模式；GPO_UNSET：置0，进入电源模式
* Parameters:
* @param [in]  gpo:  GPO引脚编号（需在头文件定义枚举GPO）
* @param [in]  stat: 引脚状态（GPO_SET/GPO_UNSET）
* @return:     引脚配置值（1<<gpo 或 0）
*******************************************************************************
*/
uint16_t DevAfeGpo(GPO gpo, CFGA_GPO stat)
{
    uint16_t gpovalue;
    if(stat == GPO_SET)
    {
        gpovalue = (1 << gpo);  // GPO置1：关机模式
    }
    else
    {
        gpovalue = (0 << gpo);  // GPO置0：电源模式
    }
    return (gpovalue);
}
// ********************************* End of BQ769x2 Power Commands   *****************************************

// ********************************* BQ769x2 Status and Fault Commands   *****************************************
/**
*******************************************************************************
* Function:  DevAfeReadBatStatus
* @brief:    读取电池状态寄存器
* @details:  通过直接命令BatteryStatus（0x12）读取16位状态字
*            状态字包含芯片工作模式、访问权限等信息
* Parameters:
* @param [in]  dev_addr:  从机地址
* @return:     16位电池状态字（小端格式拼接）
*******************************************************************************
*/
uint16_t DevAfeReadBatStatus(uint8_t dev_addr)
{
    DevAfeDirectCommands(dev_addr, BatteryStatus, 0x00, R);
    return (RX_data[1] << 8) | RX_data[0];  // 小端转主机字节序
}

/**
*******************************************************************************
* Function:  DevAfeReadAlarmStatus
* @brief:    读取报警状态寄存器
* @details:  通过直接命令AlarmStatus读取16位报警状态字
*            用于判断ALERT引脚触发原因（如过压、欠压报警）
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *com_sts:  通信状态（true=成功，false=失败）
* @return:     16位报警状态字
*******************************************************************************
*/
uint16_t DevAfeReadAlarmStatus(uint8_t dev_addr, bool *com_sts)
{
    *com_sts = DevAfeDirectCommands(dev_addr, AlarmStatus, 0x00, R);
    return (RX_data[1] << 8) | RX_data[0];
}

/**
*******************************************************************************
* Function:  DevAfeReadSafetyStatus
* @brief:    读取安全状态寄存器（A/B/C）并解析故障
* @details:  读取SafetyStatusA/B/C三个16位寄存器，解析关键故障位：
*            UVFault：欠压故障；OVFault：过压故障；SCDFault：短路故障；OCDFault：放电过流故障
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储安全状态和故障信息
* @return:     None
*******************************************************************************
*/
void DevAfeReadSafetyStatus(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    // 读取安全状态A
    pIC->FltSts.CommFlt = DevAfeDirectCommands(dev_addr, SafetyStatusA, 0x00, R);
    pIC->AlmSts.SafetyStatusAVal = (RX_data[1] << 8) | RX_data[0];
    // 解析故障位（根据芯片手册定义）
    pIC->FltSts.UVFault = ((RX_data[0] & 0x04) >> 2);  // 欠压故障（bit2）
    pIC->FltSts.OVFault = ((RX_data[0] & 0x08) >> 3);  // 过压故障（bit3）
    pIC->FltSts.SCDFault = ((RX_data[1] & 0x08) >> 3);  // 短路故障（bit11）
    pIC->FltSts.OCDFault = ((RX_data[1] & 0x02) >> 1);  // 放电过流故障（bit9）
    
    // 读取安全状态B
    pIC->FltSts.CommFlt = DevAfeDirectCommands(dev_addr, SafetyStatusB, 0x00, R);
    pIC->AlmSts.SafetyStatusBVal = (RX_data[1] << 8) | RX_data[0];
    
    // 读取安全状态C
    pIC->FltSts.CommFlt = DevAfeDirectCommands(dev_addr, SafetyStatusC, 0x00, R);
    pIC->AlmSts.SafetyStatusCVal = (RX_data[1] << 8) | RX_data[0];
    
    // 判断是否有保护机制触发（状态字总和>1表示至少有一个状态位被置1）
    pIC->FltSts.ProTriggered = (pIC->AlmSts.SafetyStatusAVal + pIC->AlmSts.SafetyStatusBVal + pIC->AlmSts.SafetyStatusCVal) > 1 ? 1 : 0;
}

/**
*******************************************************************************
* Function:  DevAfeReadPFStatus
* @brief:    读取永久故障状态寄存器（A/B/C）
* @details:  永久故障一旦触发需特殊复位才能清除，状态存储在PFStatusA/B/C
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储永久故障状态
* @return:     None
*******************************************************************************
*/
void DevAfeReadPFStatus(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    // 读取永久故障状态A
    DevAfeDirectCommands(dev_addr, PFStatusA, 0x00, R);
    pIC->AlmSts.PFStatusAVal = (RX_data[1] << 8) | RX_data[0];
    
    // 读取永久故障状态B
    DevAfeDirectCommands(dev_addr, PFStatusB, 0x00, R);
    pIC->AlmSts.PFStatusBVal = (RX_data[1] << 8) | RX_data[0];
    
    // 读取永久故障状态C
    DevAfeDirectCommands(dev_addr, PFStatusC, 0x00, R);
    pIC->AlmSts.PFStatusCVal = (RX_data[1] << 8) | RX_data[0];
}
// ********************************* End of BQ769x2 Status and Fault Commands   *****************************************

// ********************************* BQ769x2 Measurement Commands   *****************************************
/**
*******************************************************************************
* Function:  _DevAfeReadVoltage
* @brief:    读取电压（电芯/堆叠/PACK/LD）
* @details:  自动区分电压类型并转换单位：
*            电芯电压（0x14~0x32）：返回mV（16位原始值直接对应）
*            堆叠/PACK/LD电压（0x34/0x36/0x38）：返回0.01V单位转换后的mV（原始值*10）
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [in]  command:   电压读取命令（寄存器地址）
* @param [out] *com_sts:  通信状态（true=成功，false=失败）
* @return:     电压值（单位：mV）
*******************************************************************************
*/
uint16_t _DevAfeReadVoltage(uint8_t dev_addr, uint8_t command, bool *com_sts)
{
    *com_sts = DevAfeDirectCommands(dev_addr, command, 0x00, R);  // 读取16位电压数据
    
    if(command >= Cell1Voltage && command <= Cell16Voltage)  // 电芯电压（1~16节）
    {
        return (RX_data[1] << 8) | RX_data[0];  // 原始值=mV
    }
    else  // 堆叠/PACK/LD电压
    {
        return 10 * ((RX_data[1] << 8) | RX_data[0]);  // 0.01V -> mV（乘以10）
    }
}

/**
*******************************************************************************
* Function:  DevAfeReadCellVoltages
* @brief:    读取所有16节电芯电压
* @details:  遍历Cell1Voltage（0x14）~Cell16Voltage（0x32）命令，每节电芯地址间隔2
*            电压值存储在pIC->SampInfo.cell_voltage[0]~[15]（对应1~16节）
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储16节电芯电压（单位：mV）
* @return:     None
*******************************************************************************
*/
void DevAfeReadCellVoltages(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    uint8_t x;
    int cellvoltageholder = Cell1Voltage;  // 起始命令：第1节电芯电压（0x14）
    
    // 读取16节电芯电压（地址间隔2：0x14,0x16,...,0x32）
    for(x = 0; x < 16; x++)
    {
        pIC->SampInfo.cell_voltage[x] = _DevAfeReadVoltage(dev_addr, cellvoltageholder, &pIC->FltSts.CommFlt);
        cellvoltageholder += 2;  // 下一节电芯地址
    }
}

/**
*******************************************************************************
* Function:  DevAfeReadHighVol
* @brief:    读取高压（堆叠电压、PACK引脚电压、LD引脚电压）
* @details:  分别读取StackVoltage（0x34）、PACKPinVoltage（0x36）、LDPinVoltage（0x38）
*            电压值存储在pIC->SampInfo中（单位：mV）
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储高压测量值
* @return:     None
*******************************************************************************
*/
void DevAfeReadHighVol(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    pIC->SampInfo.StackVol = _DevAfeReadVoltage(dev_addr, StackVoltage, &pIC->FltSts.CommFlt);    // 堆叠电压
    pIC->SampInfo.PackVol = _DevAfeReadVoltage(dev_addr, PACKPinVoltage, &pIC->FltSts.CommFlt);  // PACK引脚电压
    pIC->SampInfo.LDVol = _DevAfeReadVoltage(dev_addr, LDPinVoltage, &pIC->FltSts.CommFlt);      // LD引脚电压
}

/**
*******************************************************************************
* Function:  DevAfeReadCurrent
* @brief:    读取实时电流（CC2寄存器）
* @details:  通过直接命令CC2Current读取16位电流值，单位：mA
*            电流传感器为1mΩ采样电阻，电流单位可通过DAConfiguration配置（默认1mA）
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储电流值（单位：mA）
* @return:     None
*******************************************************************************
*/
void DevAfeReadCurrent(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    pIC->FltSts.CommFlt = DevAfeDirectCommands(dev_addr, CC2Current, 0x00, R);
    pIC->SampInfo.cell_current = (RX_data[1] << 8) | RX_data[0];
}

/**
*******************************************************************************
* Function:  DevAfeReadCC3Current
* @brief:    读取平均电流（CC3寄存器，DASTATUS5子命令）
* @details:  通过子命令DASTATUS5（0x0075）读取，值为最近80次CC2采样的平均值
*            电流单位：mA，存储在pIC->SampInfo.cell_current
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储平均电流值
* @return:     None
*******************************************************************************
*/
void DevAfeReadCC3Current(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    // 读取DASTATUS5（32字节数据），电流值存储在第20~21字节（小端格式）
    pIC->FltSts.CommFlt = DevAfeSubcommands(dev_addr, DASTATUS5, 0x00, R, 32);
    pIC->SampInfo.cell_current = (RX_32Byte[21] << 8) | RX_32Byte[20];  // 小端转主机序
}

/**
*******************************************************************************
* Function:  DevAfeReadIntT
* @brief:    读取芯片内部温度
* @details:  通过直接命令IntTemperature读取16位原始值，转换为浮点型存储
*            温度转换公式需参考芯片手册（本函数仅存储原始值）
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储内部温度（原始浮点值）
* @return:     None
*******************************************************************************
*/
void DevAfeReadIntT(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    pIC->FltSts.CommFlt = DevAfeDirectCommands(dev_addr, IntTemperature, 0x00, R);
    // 原始值转换为浮点型（实际温度需按手册公式计算，如：T(°C) = (raw_value - 2048) / 4）
    pIC->SampInfo.arrTemperature[0] = (float)((RX_data[1] << 8) | RX_data[0]);
}

/**
*******************************************************************************
* Function:  DevAfeReadT1
* @brief:    读取外部温度（TS1引脚，热敏电阻）
* @details:  通过直接命令TS1Temperature读取16位原始值，存储在arrTemperature[1]
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储TS1温度（原始浮点值）
* @return:     None
*******************************************************************************
*/
void DevAfeReadT1(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    pIC->FltSts.CommFlt = DevAfeDirectCommands(dev_addr, TS1Temperature, 0x00, R);
    pIC->SampInfo.arrTemperature[1] = (float)((RX_data[1] << 8) | RX_data[0]);
}

/**
*******************************************************************************
* Function:  DevAfeReadT2
* @brief:    读取外部温度（TS2引脚，热敏电阻）
* @details:  通过直接命令TS2Temperature读取16位原始值，存储在arrTemperature[2]
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储TS2温度（原始浮点值）
* @return:     None
*******************************************************************************
*/
void DevAfeReadT2(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    pIC->FltSts.CommFlt = DevAfeDirectCommands(dev_addr, TS2Temperature, 0x00, R);
    pIC->SampInfo.arrTemperature[2] = (float)((RX_data[1] << 8) | RX_data[0]);
}

/**
*******************************************************************************
* Function:  DevAfeReadT3
* @brief:    读取外部温度（TS3引脚，热敏电阻）
* @details:  通过直接命令TS3Temperature读取16位原始值，存储在arrTemperature[3]
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储TS3温度（原始浮点值）
* @return:     None
*******************************************************************************
*/
void DevAfeReadT3(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    pIC->FltSts.CommFlt = DevAfeDirectCommands(dev_addr, TS3Temperature, 0x00, R);
    pIC->SampInfo.arrTemperature[3] = (float)((RX_data[1] << 8) | RX_data[0]);
}

/**
*******************************************************************************
* Function:  DevAfeReadPassQ
* @brief:    读取累计充电量和累计时间（DASTATUS6子命令）
* @details:  通过子命令DASTATUS6读取32字节数据，解析：
*            AccumCharFrac：累计充电量（Ah，分两部分存储：字节0-3和4-7）
*            AccumCharTime：累计时间（秒，字节8-11）
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储累计充电量和时间
* @return:     None
*******************************************************************************
*/
void DevAfeReadPassQ(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    // 读取DASTATUS6（32字节数据）
    pIC->FltSts.CommFlt = DevAfeSubcommands(dev_addr, DASTATUS6, 0x00, R, 32);
    
    // 累计充电量（字节0-3：低32位，字节4-7：高32位，小端格式）
    pIC->AccmChg.AccumCharFrac = ((uint32_t)RX_32Byte[3] << 24) | 
                                 ((uint32_t)RX_32Byte[2] << 16) | 
                                 ((uint32_t)RX_32Byte[1] << 8) | 
                                 RX_32Byte[0];
    pIC->AccmChg.AccumCharFrac |= ((uint64_t)((uint32_t)RX_32Byte[7] << 24) | 
                                   ((uint32_t)RX_32Byte[6] << 16) | 
                                   ((uint32_t)RX_32Byte[5] << 8) | 
                                   RX_32Byte[4]) << 32;
    
    // 累计时间（字节8-11，32位，小端格式）
    pIC->AccmChg.AccumCharTime = ((uint32_t)RX_32Byte[11] << 24) | 
                                 ((uint32_t)RX_32Byte[10] << 16) | 
                                 ((uint32_t)RX_32Byte[9] << 8) | 
                                 RX_32Byte[8];
}

/**
*******************************************************************************
* Function:  DevAfeHostWriteCBL
* @brief:    主机控制电芯均衡（开启/关闭）
* @details:  通过子命令CB_ACTIVE_CELLS（0x00xx）写入均衡状态字
*            CBLSts：16位状态字，每一位对应一节电芯的均衡使能（1=开启，0=关闭）
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [in]  CBLSts:    均衡状态字（小端格式）
* @param [in]  *pIC:      存储通信状态
* @return:     None
*******************************************************************************
*/
void DevAfeHostWriteCBL(uint8_t dev_addr, uint16_t CBLSts, AFECellAsic_t *pIC)
{
    // 写入均衡状态字（W2类型：2字节数据）
    pIC->FltSts.CommFlt = DevAfeSubcommands(dev_addr, CB_ACTIVE_CELLS, CBLSts, W2, 0);
}

/**
*******************************************************************************
* Function:  DevAfeHostReadCBL
* @brief:    读取电芯均衡状态
* @details:  通过子命令CB_ACTIVE_CELLS读取16位均衡状态字
*            存储在pIC->SampInfo.CBActiveCells（每一位对应一节电芯）
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储均衡状态字
* @return:     None
*******************************************************************************
*/
void DevAfeHostReadCBL(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    // 读取均衡状态字（R类型：2字节数据）
    pIC->FltSts.CommFlt = DevAfeSubcommands(dev_addr, CB_ACTIVE_CELLS, 0x00, R, 2);
    // 小端格式转主机序
    pIC->SampInfo.CBActiveCells = ((uint16_t)RX_32Byte[1] << 8) | RX_32Byte[0];
}
// ********************************* End of BQ769x2 Measurement Commands   *****************************************

// ********************************* Start of BQ769x2 OTP Commands *************************************************
/**
*******************************************************************************
* Function:  _DevAfeChange2FullAccess
* @brief:    切换BQ76952至全访问模式（OTP编程必需）
* @details:  解锁流程（严格遵循芯片手册）：
*            1. 发送SEAL子命令：进入密封模式
*            2. 发送UNSEAL_KEYS_CFG和UNSEAL_KEYS_CFG_CRC：解锁至非密封模式
*            3. 发送FULL_ACCESS_KEYS_CFG和FULL_ACCESS_KEYS_CFG_CRC：解锁至全访问模式
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *comm_sts: 通信状态（true=成功，false=失败）
* @return:     None
*******************************************************************************
*/
static void _DevAfeChange2FullAccess(uint8_t dev_addr, bool *comm_sts)
{
    // 1. 进入密封模式
    *comm_sts = DevAfeCommandSubcommands(dev_addr, SEAL);
    
    // 2. 解锁至非密封模式（需发送正确的解锁密钥）
    *comm_sts = DevAfeSubcommands(dev_addr, UNSEAL_KEYS_CFG, 0x00, W2, 0);
    *comm_sts = DevAfeSubcommands(dev_addr, UNSEAL_KEYS_CFG_CRC, 0x00, W2, 0);
    
    // 3. 解锁至全访问模式（需发送正确的全访问密钥）
    *comm_sts = DevAfeSubcommands(dev_addr, FULL_ACCESS_KEYS_CFG, 0x00, W2, 0);
    *comm_sts = DevAfeSubcommands(dev_addr, FULL_ACCESS_KEYS_CFG_CRC, 0x00, W2, 0);
}

/**
*******************************************************************************
* Function:  DevAfeOTPWrite
* @brief:    OTP编程（以通信模式配置为例）
* @warning   OTP编程为一次性操作，写入后不可修改，需谨慎执行
* @details:  OTP编程流程：
*            1. 切换至全访问模式
*            2. 进入CONFIG_UPDATE模式，配置CommType寄存器（通信模式）
*            3. 验证配置是否成功
*            4. 检查OTP写入使能（bit7=0）
*            5. 检查OTP编程就绪（OTP_WR_CHECK=0x80）
*            6. 发送OTP_WRITE子命令，等待100ms
*            7. 验证编程结果，退出CONFIG_UPDATE模式
* Parameters:
* @param [in]  dev_addr:  从机地址
* @param [out] *pIC:      存储OTP编程状态和失败原因
* @return:     None
*******************************************************************************
*/
void DevAfeOTPWrite(uint8_t dev_addr, AFECellAsic_t *pIC)
{
    uint16_t otp_fail_reason = 0;  // OTP失败原因（0=成功，1~5=对应失败步骤）
    
    // 1. 切换至全访问模式
    _DevAfeChange2FullAccess(dev_addr, &pIC->FltSts.CommFlt);
    pIC->OtpSts.BattSts = DevAfeReadBatStatus(dev_addr);
    
    // 检查是否进入全访问模式（电池状态字bit8~9为11）
    if(pIC->OtpSts.BattSts & 0x300)
    {
        // 2. 进入CONFIG_UPDATE模式
        pIC->FltSts.CommFlt = DevAfeCommandSubcommands(dev_addr, SET_CFGUPDATE);
        
        // 配置OTP：通信模式（COMM_TYPE_CFG需在头文件定义，示例值0x07）
        pIC->FltSts.CommFlt = _DevAfeSetRegister(dev_addr, CommType, COMM_TYPE_CFG, ONE_BYTE_LEN);
        
        // 退出CONFIG_UPDATE模式
        pIC->FltSts.CommFlt = DevAfeCommandSubcommands(dev_addr, EXIT_CFGUPDATE);
        DevAfedelaymS(30);  // 等待配置生效
        
        // 3. 验证CommType配置是否成功（读取值应等于COMM_TYPE_CFG=0x07）
        pIC->FltSts.CommFlt = DevAfeSubcommands(dev_addr, CommType, 0x00, R, 1);
        if(RX_32Byte[0] == 0x07)
        {
            // 4. 再次进入CONFIG_UPDATE模式，检查OTP写入使能（电池状态字bit7=0）
            pIC->FltSts.CommFlt = DevAfeCommandSubcommands(dev_addr, SET_CFGUPDATE);
            pIC->OtpSts.BattSts = DevAfeReadBatStatus(dev_addr);
            if((pIC->OtpSts.BattSts & 0x80) == 0)
            {
                // 5. 检查OTP编程就绪（OTP_WR_CHECK=0x80表示可编程）
                pIC->FltSts.CommFlt = DevAfeSubcommands(dev_addr, OTP_WR_CHECK, 0x00, R, 1);
                if(RX_32Byte[0] == 0x80)
                {
                    // 6. 发送OTP_WRITE命令，开始编程
                    pIC->FltSts.CommFlt = DevAfeSubcommands(dev_addr, OTP_WRITE, 0x00, R, 1);
                    DevAfedelaymS(100);  // 等待编程完成（芯片要求至少100ms）
                    
                    // 读取编程结果（0x40寄存器）
                    pIC->FltSts.CommFlt = _DevAfeI2CReadReg(dev_addr, 0x40, RX_32Byte, 2);
                    
                    // 7. 验证编程成功（bit7仍为0表示编程完成且无错误）
                    if((pIC->OtpSts.BattSts & 0x80) == 0)
                    {
                        // 编程成功，退出CONFIG_UPDATE模式
                        pIC->FltSts.CommFlt = DevAfeCommandSubcommands(dev_addr, EXIT_CFGUPDATE);
                        pIC->OtpSts.OtpWriteSts = 1;  // 标记编程成功
                        otp_fail_reason = 0;
                    }
                    else
                    {
                        otp_fail_reason = 5;  // 编程失败（bit7置1）
                    }
                }
                else
                {
                    otp_fail_reason = 4;  // OTP编程未就绪（OTP_WR_CHECK≠0x80）
                }
            }
            else
            {
                otp_fail_reason = 3;  // OTP写入不允许（bit7=1）
            }
        }
        else
        {
            otp_fail_reason = 2;  // CommType配置失败（读取值≠0x07）
        }
    }
    else
    {
        otp_fail_reason = 1;  // 未进入全访问模式（bit8~9≠11）
    }
    
    // 存储失败原因
    pIC->OtpSts.OtpFailReason = otp_fail_reason;
}
