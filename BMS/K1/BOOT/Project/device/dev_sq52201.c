/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT Nova
  * @file    dev_SQ52201.c
  * @author  Xi'an NovaStar Tech Co.,LTD.()
  * @version V1.0.0
  * @date    2023.05.05
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date        <th>Version  <th>Author      <th>Description
  * <tr><td>2021/08/04  <td>1.0.0    <td>XingXiao    <td>CBB Version
  * </table>
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
/******************************************************************************/

#include "stddef.h"
#include "dev_sq52201.h"

#define LOG_TAG     "dev_SQ52201"

#define USE_SIM_IIC                      (1)

#define SQ52201_CALIB_VAL                (1024)
#define SQ52201_CURRENTLSB               (0.5F) // mA/bit
#define SQ52201_CURRENTLSB_INV           (1/(SQ52201_CURRENTLSB)) // bit/mA
#define SQ52201_POWERLSB_INV             (1/(SQ52201_CURRENTLSB*25)) // bit/mW

#define SQ52201_CONFIG                   (0x00) // Configuration Register (R/W)初始值4127
#define SQ52201_SHUNTV                   (0x01) // Shunt Voltage (R)初始值0，分流电压测量值
#define SQ52201_BUSV                     (0x02) // Bus Voltage (R)初始值0，总线电压测量值
#define SQ52201_POWER                    (0x03) // Power (R)初始值0，输出功率测量值
#define SQ52201_CURRENT                  (0x04) // Current (R)初始值0，分流电阻电流计算值
#define SQ52201_CALIB                    (0x05) // Calibration (R/W)，设置全量程和电流LSB
#define SQ52201_MASK                     (0x06) // Mask/Enable (R/W)，报警设置和转换准备标志
#define SQ52201_ALERTL                   (0x07) // Alert Limit (R/W)，报警阈值
#define SQ52201_MANUF_ID                 (0xFE) // Manufacturer ID (R)，0x5449
#define SQ52201_DIE_ID                   (0xFF) // Die ID (R),0x2260

#define SQ52201_MODE_POWER_DOWN          (0<<0) // Power-Down
#define SQ52201_MODE_TRIG_SHUNT_VOLTAGE  (1<<0) // Shunt Voltage, Triggered
#define SQ52201_MODE_TRIG_BUS_VOLTAGE    (2<<0) // Bus Voltage, Triggered
#define SQ52201_MODE_TRIG_SHUNT_AND_BUS  (3<<0) // Shunt and Bus, Triggered
#define SQ52201_MODE_POWER_DOWN2         (4<<0) // Power-Down
#define SQ52201_MODE_CONT_SHUNT_VOLTAGE  (5<<0) // Shunt Voltage, Continuous
#define SQ52201_MODE_CONT_BUS_VOLTAGE    (6<<0) // Bus Voltage, Continuous
#define SQ52201_MODE_CONT_SHUNT_AND_BUS  (7<<0) // Shunt and Bus, Continuous

// Shunt Voltage Conversion Time
#define SQ52201_VSH_140uS                (0<<3)
#define SQ52201_VSH_204uS                (1<<3)
#define SQ52201_VSH_332uS                (2<<3)
#define SQ52201_VSH_588uS                (3<<3)
#define SQ52201_VSH_1100uS               (4<<3)
#define SQ52201_VSH_2116uS               (5<<3)
#define SQ52201_VSH_4156uS               (6<<3)
#define SQ52201_VSH_8244uS               (7<<3)

// Bus Voltage Conversion Time (VBUS CT Bit Settings[6-8])
#define SQ52201_VBUS_140uS               (0<<6)
#define SQ52201_VBUS_204uS               (1<<6)
#define SQ52201_VBUS_332uS               (2<<6)
#define SQ52201_VBUS_588uS               (3<<6)
#define SQ52201_VBUS_1100uS              (4<<6)
#define SQ52201_VBUS_2116uS              (5<<6)
#define SQ52201_VBUS_4156uS              (6<<6)
#define SQ52201_VBUS_8244uS              (7<<6)

// Averaging Mode (AVG Bit Settings[9-11])
#define SQ52201_AVG_1                    (0<<9)
#define SQ52201_AVG_4                    (1<<9)
#define SQ52201_AVG_16                   (2<<9)
#define SQ52201_AVG_64                   (3<<9)
#define SQ52201_AVG_128                  (4<<9)
#define SQ52201_AVG_256                  (5<<9)
#define SQ52201_AVG_512                  (6<<9)
#define SQ52201_AVG_1024                 (7<<9)

// Reset Bit (RST bit [15])
#define SQ52201_RESET_ACTIVE             (1<<15)
#define SQ52201_RESET_INACTIVE           (0<<15)

// Mask/Enable Register
#define SQ52201_MER_SOL                  (1<<15) // Shunt Voltage Over-Voltage
#define SQ52201_MER_SUL                  (1<<14) // Shunt Voltage Under-Voltage
#define SQ52201_MER_BOL                  (1<<13) // Bus Voltagee Over-Voltage
#define SQ52201_MER_BUL                  (1<<12) // Bus Voltage Under-Voltage
#define SQ52201_MER_POL                  (1<<11) // Power Over-Limit
#define SQ52201_MER_CNVR                 (1<<10) // Conversion Ready
#define SQ52201_MER_AFF                  (1<<4) // Alert Function Flag
#define SQ52201_MER_CVRF                 (1<<3) // Conversion Ready Flag
#define SQ52201_MER_OVF                  (1<<2) // Math Overflow Flag
#define SQ52201_MER_APOL                 (1<<1) // Alert Polarity Bit
#define SQ52201_MER_LEN                  (1<<0) // Alert Latch Enable

#define SQ52201_MANUF_ID_DEFAULT         (0x5449)
#define SQ52201_DIE_ID_DEFAULT           (0x2260)

static uint8_t _SQ52201_SetConfig(uint16_t config_word);
static uint8_t _SQ52201_SetCalibrationReg(uint16_t config_word);
static void _SQ52201RegInit(void);

/**
 * @brief  : SQ52201初始化
 * @para   : None
 * @note   : None
 * @return : 
 */
void DevSQ52201Init()
{
#if USE_SIM_IIC == 0
    gI2CEepromHandleType.slave_addr0 = EEPROM_DEVICE_ADDR;
    gI2CEepromHandleType.reg_addr_width = EN_I2C_REG_ADDR_WIDTH_16;
    I2CEepromBspInit();
#else
    SQ52201Init();
    _SQ52201RegInit();
#endif
}

/**
 * @brief  : SQ52201寄存器初始化设置
 * @para   : None
 * @note   : None
 * @return : 
 */
static void _SQ52201RegInit(void)
{
    /*
    * 设置转换时间8.244ms,求平均值次数16，设置模式为分流和总线连续模式
    * 总数据转换时间 = 8.244*16 = 131.9ms
    */
    _SQ52201_SetConfig(0x45FF);//0100 0101 1111 1111

    /*
    * 分流电阻最大电压 = 32768 * 0.0000025V = 0.08192V
    * 设置分流电压转电流转换参数:电阻0.01R，分辨率0.2mA
    * 公式1
    * Current_LSB = 预期最大电流 / 2^15
    * Current_LSB = 5 / 32768 = 0.000152A ,选0.2ma
    * 公式2
    * CAL = 0.00512/(Current_LSB*R)
    * CAL = 0.00512/(0.0002*0.01)=2560 = 0x0a00
    */
    _SQ52201_SetCalibrationReg(0x0a00);//0000 1010 0000 0000

}

/**
 * @brief  : SQ52201设置
 * @para   : None
 * @note   : None
 * @return : 
 */
static uint8_t _SQ52201_SetConfig(uint16_t config_word)
{
    uint8_t send_table[2];
    send_table[0] = (config_word & 0xFF00) >> 8;
    send_table[1] = (config_word & 0x00FF);
    
    SQ52201Handle.pTxBuf = send_table;
    SQ52201Handle.reg_addr = SQ52201_CONFIG;
    SQ52201Handle.tx_size = 2;
    SimI2CRegWrite(&SQ52201Handle);
    
    return 0;
}

/**
 * @brief  : SQ52201校准寄存器设置
 * @para   : None
 * @note   : None
 * @return : 
 */
static uint8_t _SQ52201_SetCalibrationReg(uint16_t config_word)
{
    uint8_t send_table[2];
    send_table[0] = (config_word & 0xFF00) >> 8;
    send_table[1] = (config_word & 0x00FF);
    
    SQ52201Handle.pTxBuf = send_table;
    SQ52201Handle.reg_addr = SQ52201_CALIB;
    SQ52201Handle.tx_size = 2;
    SimI2CRegWrite(&SQ52201Handle);
    
    return 0;
}

/**
 * @brief  : SQ52201总线电压寄存器数值读取
 * @para   : None
 * @note   : None
 * @return : 总线电压寄存器数值
 */
static uint16_t _SQ52201_GetBusVReg()
{
    uint8_t rece_table[2];
    
    SQ52201Handle.pRxBuf = rece_table;
    SQ52201Handle.reg_addr = SQ52201_BUSV;
    SQ52201Handle.rx_size = 2;
    SimI2CRegRead(&SQ52201Handle);

    return ((uint16_t)rece_table[0]<<8 | rece_table[1]);
}

/**
 * @brief  : SQ52201功率寄存器数值读取
 * @para   : None
 * @note   : None
 * @return : 功率寄存器数值
 */
static uint16_t _SQ52201_GetPowerReg()
{    
    uint8_t rece_table[2];
    
    SQ52201Handle.pRxBuf = rece_table;
    SQ52201Handle.reg_addr = SQ52201_POWER;
    SQ52201Handle.rx_size = 2;
    SimI2CRegRead(&SQ52201Handle);

    return ((uint16_t)rece_table[0]<<8 | rece_table[1]);
}

/**
 * @brief  : SQ52201电流寄存器数值读取
 * @para   : None
 * @note   : None
 * @return : 电流寄存器数值
 */
static uint16_t _SQ52201_GetCurrentReg()
{
    uint8_t rece_table[2];
    
    SQ52201Handle.pRxBuf = rece_table;
    SQ52201Handle.reg_addr = SQ52201_CURRENT;
    SQ52201Handle.rx_size = 2;
    SimI2CRegRead(&SQ52201Handle);

    return ((uint16_t)rece_table[0]<<8 | rece_table[1]);
}

/**
 * @brief  : SQ52201总线电压读取
 * @para   : None
 * @note   : None
 * @return : 总线电压
 */
float SQ52201_GetBusV(void)
{
    uint16_t regData;
    float fVoltage;
    regData = _SQ52201_GetBusVReg();
    fVoltage = regData * 0.00125f;/*电压的LSB = 1.25mV*/
    return fVoltage;
}

/**
 * @brief  : SQ52201电流读取
 * @para   : None
 * @note   : None
 * @return : 电流
 */
float SQ52201_GetCurrent()
{
    uint16_t regData;
    float fCurrent;
    regData = _SQ52201_GetCurrentReg();
    if(regData >= 0x8000)   regData = 0;
    fCurrent = regData * 0.0002f;/*电流的LSB = 0.2mA，由用户配置*/
    return fCurrent;
}

/**
 * @brief  : SQ52201功率读取
 * @para   : None
 * @note   : None
 * @return : 功率
 */
float SQ52201_GetPower()
{
    uint16_t regData;
    float fPower;
    regData = _SQ52201_GetPowerReg();
    fPower = regData * 0.005f;/*功率的LSB = 电流的LSB*25*/
    return fPower;
}

/**
 * @brief  : SQ52201ManufID读取
 * @para   : None
 * @note   : None
 * @return : ID
 */

uint16_t SQ52201_GetBoardManufID()
{
    uint8_t id_rece_table[2];
    
    SQ52201Handle.slave_addr0 = (0x40<<1);  //iic地址需查阅手册
    SQ52201Handle.pRxBuf = id_rece_table;
    SQ52201Handle.reg_addr = SQ52201_MANUF_ID;
    SQ52201Handle.rx_size = 2;
    SimI2CRegRead(&SQ52201Handle);

    return ((uint16_t)id_rece_table[0]<<8 | id_rece_table[1]);
    
}