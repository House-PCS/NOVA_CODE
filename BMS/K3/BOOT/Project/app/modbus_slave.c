/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_slave.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-12-16
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-12-16    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "modbus_slave.h"
#define Uart_modbus_Handle  Uart1_ModS_Handle
#define ModS_IRQHandler     USART1_IRQHandler
#define CAPACITY_STD    72000

static uint8_t          _RevDataBufIndex = 0;
static uint8_t          _RevData[MODBUS_REV_QUEUE_DEPTH][_MODBUS_RX_SIZE];
static ModbusRevData_t  _gModbusRevData[MODBUS_REV_QUEUE_DEPTH];
static MODS_T _gModS;
ModbusReg_t gMODBUS_Reg;
// CRC高字节
static const uint8_t s_CRCHi[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
// CRC 低字节
const uint8_t s_CRCLo[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

Queue_t gModbusRevQueue;

/*******************************************************************************
  * @FunctionName: BEBufToUint16
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:08:24+0800
  * @Purpose:      大端数据转u16
  * @param:        _pBuf
  * @return:
*******************************************************************************/
uint16_t BEBufToUint16(uint8_t *_pBuf)
{
    return (((uint16_t)_pBuf[0] << 8) | _pBuf[1]);
}

void TabLsbToMsb(uint8_t *str, uint16_t len)
{
    for(int j = 0; j < len / 2; j++)
    {
        str[j] ^= str[len - j - 1];
        str[len - j - 1] ^= str[j];
        str[j] ^= str[len - j - 1];
    }
}

/*******************************************************************************
  * @FunctionName: CRC16_Modbus
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:09:58+0800
  * @Purpose:
  * @param:        _pBuf
  * @param:        _usLen
  * @return:
*******************************************************************************/
uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen)
{
    uint8_t ucCRCHi = 0xFF; /* 高CRC字节初始化 */
    uint8_t ucCRCLo = 0xFF; /* 低CRC 字节初始化 */
    uint16_t usIndex;  /* CRC循环中的索引 */

    while (_usLen--)
    {
        usIndex = ucCRCHi ^ *_pBuf++; /* 计算CRC */
        ucCRCHi = ucCRCLo ^ s_CRCHi[usIndex];
        ucCRCLo = s_CRCLo[usIndex];
    }
    return ((uint16_t)ucCRCHi << 8 | ucCRCLo);

}

/*******************************************************************************
  * @FunctionName: RevDataAnalysis
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:11:45+0800
  * @Purpose:
  * @param:
  * @return:
*******************************************************************************/
static void RevDataAnalysis()
{
    uint8_t *p = Uart_modbus_Handle.pRxBuf;

    if(Uart_modbus_Handle.pRxBuf[0] != SADDRMASTER)
    {
        return;
    }
    _gModbusRevData[_RevDataBufIndex].data_size = _gModS.RxCount;
    memcpy(_gModbusRevData[_RevDataBufIndex].pData, p, _gModS.RxCount);
    if(QueuePut(&gModbusRevQueue, &_gModbusRevData[_RevDataBufIndex]) == false)
    {
        return;
    }
    _RevDataBufIndex++;
    _RevDataBufIndex %= MODBUS_REV_QUEUE_DEPTH;
}

/*******************************************************************************
  * @FunctionName: IrqRxProcess
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:12:19+0800
  * @Purpose:
  * @param:
  * @return:
*******************************************************************************/
static void IrqRxProcess()
{
    UartDMARxDisable(&Uart_modbus_Handle);
    _gModS.RxCount = UartDMARxDataLength(&Uart_modbus_Handle);

    RevDataAnalysis();

    UartDMARxEnable(&Uart_modbus_Handle);
}

/*******************************************************************************
  * @FunctionName: ModSInit
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:12:22+0800
  * @Purpose:
  * @param:        void
  * @return:
*******************************************************************************/
bool ModSInit(void)
{
    bool ret = true;

    Uart1ModsBspInit();
    QueueInit(&gModbusRevQueue, MODBUS_REV_QUEUE_DEPTH, sizeof(ModbusRevData_t));
    for(uint8_t i = 0; i < MODBUS_REV_QUEUE_DEPTH; i++)
    {
        _gModbusRevData[i].pData = _RevData[i];
    }

    Uart_modbus_Handle.pRxBuf = _gModS.RxBuf;
    Uart_modbus_Handle.rx_size = _MODBUS_RX_SIZE;
    Uart_modbus_Handle.irq_rx_process = IrqRxProcess;

    UartDMARxDisable(&Uart_modbus_Handle);
    UartDMARxEnable(&Uart_modbus_Handle);

    return ret;
}

/*******************************************************************************
  * @FunctionName: MODS_ReadRegValue
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:12:43+0800
  * @Purpose:
  * @param:        reg_addr
  * @param:        reg_value
  * @return:
*******************************************************************************/
static bool MODS_ReadRegValue(uint16_t reg_addr, uint8_t *reg_value)
{
    uint16_t value;

    switch (reg_addr)                                   /* 判断寄存器地址 */
    {
        /***base***/
        case MODBUS_HARDWARE_VERSION:
        {
            value = 1000;
            break;
        }
        case MODBUS_SOFTWARE_VERSION:
        {
            value = 1000;
            break;
        }
        case MODBUS_MODEL_ID:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_19_18:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_17_16:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_15_14:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_13_12:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_11_10:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_09_08:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_07_06:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_05_04:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_03_02:
        {
            value = 0xFFFF;
            break;
        }
        case MODBUS_SN_01_00:
        {
            value = 0xFFFF;
            break;
        }
        /***cell***/
        case MODBUS_CELL_00_VDC:
        {
            value = gCell_Info.cell_voltage[0];
            break;
        }
        case MODBUS_CELL_01_VDC:
        {
            value = gCell_Info.cell_voltage[1];
            break;
        }
        case MODBUS_CELL_02_VDC:
        {
            value = gCell_Info.cell_voltage[2];
            break;
        }
        case MODBUS_CELL_03_VDC:
        {
            value = gCell_Info.cell_voltage[3];
            break;
        }
        case MODBUS_CELL_04_VDC:
        {
            value = gCell_Info.cell_voltage[4];
            break;
        }
        case MODBUS_CELL_05_VDC:
        {
            value = gCell_Info.cell_voltage[5];
            break;
        }
        case MODBUS_CELL_06_VDC:
        {
            value = gCell_Info.cell_voltage[6];
            break;
        }
        case MODBUS_CELL_07_VDC:
        {
            value = gCell_Info.cell_voltage[7];
            break;
        }
        case MODBUS_CELL_08_VDC:
        {
            value = gCell_Info.cell_voltage[8];
            break;
        }
        case MODBUS_CELL_09_VDC:
        {
            value = gCell_Info.cell_voltage[9];
            break;
        }
        case MODBUS_CELL_10_VDC:
        {
            value = gCell_Info.cell_voltage[10];
            break;
        }
        case MODBUS_CELL_11_VDC:
        {
            value = gCell_Info.cell_voltage[11];
            break;
        }
        case MODBUS_CELL_12_VDC:
        {
            value = gCell_Info.cell_voltage[12];
            break;
        }
        case MODBUS_CELL_13_VDC:
        {
            value = gCell_Info.cell_voltage[13];
            break;
        }
        case MODBUS_CELL_14_VDC:
        {
            value = gCell_Info.cell_voltage[14];
            break;
        }
        case MODBUS_CELL_15_VDC:
        {
            value = gCell_Info.cell_voltage[15];
            break;
        }
        case MODBUS_CELL_16_VDC:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_17_VDC:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_18_VDC:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_19_VDC:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_MAX_VOLTAGE:
        {
            value = gCell_Info.cell_max_voltage;
            break;
        }
        case MODBUS_CELL_MIN_VOLTAGE:
        {
            value = gCell_Info.cell_min_voltage;
            break;
        }
        case MODBUS_CELL_AVERAGE_VOLTAGE:
        {
            value = gCell_Info.cell_ave_voltage;
            break;
        }
        case MODBUS_CELL_ALL_VDC_HIGH:
        {
            value = (uint16_t)(gCell_Info.total_cell_voltage >> 16);
            break;
        }
        case MODBUS_CELL_ALL_VDC_LOW:
        {
            value = (uint16_t)gCell_Info.total_cell_voltage;
            break;
        }
        case MODBUS_CELL_CURRENT_HIGH:
        {
            value = (uint16_t)(gCell_Info.cell_current >> 16);
            break;
        }
        case MODBUS_CELL_CURRENT_LOW:
        {
            value = gCell_Info.cell_current;
            break;
        }
        case MODBUS_CURRENT_POWER:
        {
            value = gPackStatus.current_power;
            break;
        }
        /***sox***/
        case MODBUS_SOC_REAL:
        {
            value = gPackStatus.soc_real_rpt;
            break;
        }
        case MODBUS_SOC_ACT:
        {
            value = gPackStatus.soc_actl_rpt;
            break;
        }
        case MODBUS_SOC_RPT:
        {
            value = gPackStatus.soc_disp_rpt;
            break;
        }
        case MODBUS_CAPACITY_STD_HIGH:
        {
            value = CAPACITY_STD >> 16;
            break;
        }
        case MODBUS_CAPACITY_STD_LOW:
        {
            value = (uint16_t)CAPACITY_STD;
            break;
        }
        case MODBUS_CURRENT_MAX_CAPA_HIGH:
        {
            value = gPackStatus.cr >> 16;
            break;
        }
        case MODBUS_CURRENT_MAX_CAPA_LOW:
        {
            value = gPackStatus.cr;
            break;
        }
        case MODBUS_PACK_HEALTH:
        {
            value = gPackStatus.soh_rpt;
            break;
        }
        case MODBUS_PACK_CYCLES:
        {
            value = gPackStatus.pack_cycles;
            break;
        }
        case MODBUS_CHARGING_CAPACITY_HIGH:
        {
            value = gPackStatus.chg_accumulate_capa >> 16;
            break;
        }
        case MODBUS_CHARGING_CAPACITY_LOW:
        {
            value = gPackStatus.chg_accumulate_capa;
            break;
        }
        case MODBUS_DISCHARGE_CAPACITY_HIGH:
        {
            value = gPackStatus.dsg_accumulate_capa >> 16;
            break;
        }
        case MODBUS_DISCHARGE_CAPACITY_LOW:
        {
            value = gPackStatus.dsg_accumulate_capa;
            break;
        }
        case MODBUS_CURRENT_MAX_CHG_CURRENT_HIGH:
        {
            value = gPackStatus.cur_max_chg_current >> 16;
            break;
        }
        case MODBUS_CURRENT_MAX_CHG_CURRENT_LOW:
        {
            value = gPackStatus.cur_max_chg_current;
            break;
        }
        case MODBUS_CURRENT_MAX_DSG_CURRENT_10_HIGH:
        {
            value = gPackStatus.cur_max_dsg_current_10s >> 16;
            break;
        }
        case MODBUS_CURRENT_MAX_DSG_CURRENT_10_LOW:
        {
            value = gPackStatus.cur_max_dsg_current_10s;
            break;
        }
        case MODBUS_CURRENT_MAX_DSG_CURRENT_30_HIGH:
        {
            value = gPackStatus.cur_max_dsg_current_30s >> 16;
            break;
        }
        case MODBUS_CURRENT_MAX_DSG_CURRENT_30_LOW:
        {
            value = gPackStatus.cur_max_dsg_current_30s;
            break;
        }
        case MODBUS_CURRENT_MAX_DSG_CURRENT_60_HIGH:
        {
            value = gPackStatus.cur_max_dsg_current_60s >> 16;
            break;
        }
        case MODBUS_CURRENT_MAX_DSG_CURRENT_60_LOW:
        {
            value = gPackStatus.cur_max_dsg_current_60s;
            break;
        }
        case MODBUS_PACK_SLEEP_TIME_HIGH:
        {
            value = gPackStatus.ocv_time_count >> 16;
            break;
        }
        case MODBUS_PACK_SLEEP_TIME_LOW:
        {
            value = gPackStatus.ocv_time_count;
            break;
        }
        case MODBUS_CHARGING_CAPACITY2_HIGH:
        {
            value = gPackStatus.chg_accumulate_capa_mwh >> 16;
            break;
        }
        case MODBUS_CHARGING_CAPACITY2_LOW:
        {
            value = gPackStatus.chg_accumulate_capa_mwh;
            break;
        }
        case MODBUS_DISCHARGE_CAPACITY2_HIGH:
        {
            value = gPackStatus.dsg_accumulate_capa_mwh >> 16;
            break;
        }
        case MODBUS_DISCHARGE_CAPACITY2_LOW:
        {
            value = gPackStatus.dsg_accumulate_capa_mwh;
            break;
        }
        case MODBUS_PACK_STATUS:
        {
            value = gPackStatus.mode;
            break;
        }
        case MODBUS_RECHARGE_TIME_HIGH:
        {
            value = gPackStatus.recharge_times >> 16;
            break;
        }
        case MODBUS_RECHARGE_TIME_LOW:
        {
            value = gPackStatus.recharge_times;
            break;
        }
        /***temp***/
        case MODBUS_CELL_TEMP_00:
        {
            value = gCell_Info.cell_temperature[0];
            break;
        }
        case MODBUS_CELL_TEMP_01:
        {
            value = gCell_Info.cell_temperature[1];
            break;
        }
        case MODBUS_CELL_TEMP_02:
        {
            value = gCell_Info.cell_temperature[2];
            break;
        }
        case MODBUS_CELL_TEMP_03:
        {
            value = gCell_Info.cell_temperature[3];
            break;
        }
        case MODBUS_CELL_TEMP_04:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_05:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_06:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_07:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_08:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_09:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_10:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_11:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_12:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_13:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_14:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_15:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_16:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_17:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_18:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_TEMP_19:
        {
            value = 0;
            break;
        }
        case MODBUS_CELL_MAX_TEMP:
        {
            value = gCell_Info.cell_max_temperature;
            break;
        }
        case MODBUS_CELL_MIN_TEMP:
        {
            value = gCell_Info.cell_min_temperature;
            break;
        }
        case MODBUS_CELL_AVE_TEMP:
        {
            value = gCell_Info.cell_ave_temperature;
            break;
        }
        case MODBUS_BALANCE_TEMP :
        {
            value = gCell_Info.afe_net_temperature;
            break;
        }
        case MODBUS_DSG_MOS_TEMP :
        {
            value = gCell_Info.dmos_temperature;
            break;
        }
        case MODBUS_CHG_MOS_TEMP :
        {
            value = gCell_Info.cmos_temperature;
            break;
        }
        case MODBUS_ADD_MOS_TEMP1:
        {
            value = gCell_Info.emos_temperature;
            break;
        }
        case MODBUS_ADD_MOS_TEMP2:
        {
            value = gCell_Info.emos_temperature;
            break;
        }
        case MODBUS_BALANCE_STATUS:
        {
            value = 0;
            break;
        }
        /***r w***/
        case MODBUS_DSG_MOS_CTL:
        {
            value = gMODBUS_Reg.dsg_mos_ctl;
            break;
        }
        case MODBUS_CHG_MOS_CTL:
        {
            value = gMODBUS_Reg.chg_mos_ctl;
            break;
        }
        case MODBUS_PRE_MOS_CTL:
        {
            value = gMODBUS_Reg.pre_mos_ctl;
            break;
        }
        case MODBUS_ADD_MOS_CTL1:
        {
            value = gMODBUS_Reg.add_mos_ctl1;
            break;
        }
        case MODBUS_ADD_MOS_CTL2:
        {
            value = gMODBUS_Reg.add_mos_ctl2;
            break;
        }
        case MODBUS_BMS_SHUTDOWN:
        {
            value = gMODBUS_Reg.bms_shutdown;
            break;
        }

        /***limit***/
        case MODBUS_CELL_UV_LVL1:
        {
            value = gMODBUS_Reg.cell_uv_lvl1;
            break;
        }
        case MODBUS_CELL_UV_LVL2:
        {
            value = gMODBUS_Reg.cell_uv_lvl2;
            break;
        }
        case MODBUS_CELL_UV_LVL3:
        {
            value = gMODBUS_Reg.cell_uv_lvl3;
            break;
        }
        case MODBUS_CELL_OV_LVL1:
        {
            value = gMODBUS_Reg.cell_ov_lvl1;
            break;
        }
        case MODBUS_CELL_OV_LVL2:
        {
            value = gMODBUS_Reg.cell_ov_lvl2;
            break;
        }
        case MODBUS_CELL_OV_LVL3:
        {
            value = gMODBUS_Reg.cell_ov_lvl3;
            break;
        }
        case MODBUS_PACK_OV_LVL_HIGH:
        {
            value = gMODBUS_Reg.pack_ov_lvl >> 16;
            break;
        }
        case MODBUS_PACK_OV_LVL_LOW:
        {
            value = gMODBUS_Reg.pack_ov_lvl;
            break;
        }
        case MODBUS_PACK_UV_LVL_HIGH:
        {
            value = gMODBUS_Reg.pack_uv_lvl >> 16;
            break;
        }
        case MODBUS_PACK_UV_LVL_LOW:
        {
            value = gMODBUS_Reg.pack_uv_lvl;
            break;
        }
        case MODBUS_PACK_OT_LVL1:
        {
            value = gMODBUS_Reg.pack_ot_lvl1;
            break;
        }
        case MODBUS_PACK_OT_LVL2:
        {
            value = gMODBUS_Reg.pack_ot_lvl2;
            break;
        }
        case MODBUS_PACK_OT_LVL3:
        {
            value = gMODBUS_Reg.pack_ot_lvl3;
            break;
        }
        case MODBUS_PACK_UT_LVL:
        {
            value = gMODBUS_Reg.pack_ut_lvl;
            break;
        }
        /***error***/
        case MODBUS_MOS_ERROR_CODE:
        {
            value = gBmsErrorInfo.mos_error_code.value;
            break;
        }
        case MODBUS_CURR_ERROR_CODE:
        {
            value = gBmsErrorInfo.curr_error_code.value;
            break;
        }
        case MODBUS_CELL1_ERROR_CODE:
        {
            value = gBmsErrorInfo.cell1_error_code.value;
            break;
        }
        case MODBUS_CELL2_ERROR_CODE:
        {
            value = gBmsErrorInfo.cell2_error_code.value;
            break;
        }
        case MODBUS_PREC_ERROR_CODE:
        {
            value = gBmsErrorInfo.prec_error_code.value;
            break;
        }
        case MODBUS_CELLSAMPLE_ERROR_CODE:
        {
            value = gBmsErrorInfo.cellsample_error_code.value;
            break;
        }
        case MODBUS_OTHER1_ERROR_CODE:
        {
            value = gBmsErrorInfo.other1_error_code.value;
            break;
        }

        default:
        {
            value = 0;
            break;
        }
    }

    reg_value[0] = value >> 8;
    reg_value[1] = value;

    return true;                                        /* 读取成功 */
}

/*******************************************************************************
  * @FunctionName: MODS_WriteRegValue
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:13:32+0800
  * @Purpose:
  * @param:        reg_addr
  * @param:        reg_value
  * @return:
*******************************************************************************/
static uint8_t MODS_WriteRegValue(uint16_t reg_addr, uint16_t reg_value)
{
    switch (reg_addr)                           /* 判断寄存器地址 */
    {
        case MODBUS_DSG_MOS_CTL:
        {
            gMODBUS_Reg.dsg_mos_ctl = reg_value;
            break;
        }
        case MODBUS_CHG_MOS_CTL:
        {
            gMODBUS_Reg.chg_mos_ctl = reg_value;
            break;
        }
        case MODBUS_PRE_MOS_CTL:
        {
            gMODBUS_Reg.pre_mos_ctl = reg_value;
            break;
        }
        case MODBUS_ADD_MOS_CTL1:
        {
            gMODBUS_Reg.add_mos_ctl1 = reg_value;
            break;
        }
        case MODBUS_ADD_MOS_CTL2:
        {
            gMODBUS_Reg.add_mos_ctl2 = reg_value;
            break;
        }
        case MODBUS_BMS_SHUTDOWN:
        {
            gMODBUS_Reg.bms_shutdown  = reg_value;
            break;
        }

        case MODBUS_CELL_UV_LVL1:
        {
            gMODBUS_Reg.cell_uv_lvl1 = reg_value;
            break;
        }
        case MODBUS_CELL_UV_LVL2:
        {
            gMODBUS_Reg.cell_uv_lvl2 = reg_value;
            break;
        }
        case MODBUS_CELL_UV_LVL3:
        {
            gMODBUS_Reg.cell_uv_lvl3 = reg_value;
            break;
        }
        case MODBUS_CELL_OV_LVL1:
        {
            gMODBUS_Reg.cell_ov_lvl1 = reg_value;
            break;
        }
        case MODBUS_CELL_OV_LVL2:
        {
            gMODBUS_Reg.cell_ov_lvl2 = reg_value;
            break;
        }
        case MODBUS_CELL_OV_LVL3:
        {
            gMODBUS_Reg.cell_ov_lvl3 = reg_value;
            break;
        }
        case MODBUS_PACK_OV_LVL_HIGH:
        {
            gMODBUS_Reg.pack_ov_lvl = (uint32_t)reg_value << 16;
            break;
        }
        case MODBUS_PACK_OV_LVL_LOW:
        {
            gMODBUS_Reg.pack_ov_lvl |= reg_value;
            break;
        }
        case MODBUS_PACK_UV_LVL_HIGH:
        {
            gMODBUS_Reg.pack_uv_lvl = (uint32_t)reg_value << 16;
            break;
        }
        case MODBUS_PACK_UV_LVL_LOW:
        {
            gMODBUS_Reg.pack_uv_lvl |= reg_value;
            break;
        }
        case MODBUS_PACK_OT_LVL1:
        {
            gMODBUS_Reg.pack_ot_lvl1 = reg_value;
            break;
        }
        case MODBUS_PACK_OT_LVL2:
        {
            gMODBUS_Reg.pack_ot_lvl2 = reg_value;
            break;
        }
        case MODBUS_PACK_OT_LVL3:
        {
            gMODBUS_Reg.pack_ot_lvl3 = reg_value;
            break;
        }
        case MODBUS_PACK_UT_LVL:
        {
            gMODBUS_Reg.pack_uv_lvl = reg_value;
            break;
        }

        default:
            return false;       /* 参数异常，返回 0 */
    }

    return true;        /* 读取成功 */
}

/*******************************************************************************
  * @FunctionName: MODS_SendWithCRC
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:14:25+0800
  * @Purpose:
  * @param:        pBuf
  * @param:        ucLen
  * @return:
*******************************************************************************/
static void MODS_SendWithCRC(uint8_t *pBuf, uint8_t ucLen)
{
    uint16_t crc;

    crc = CRC16_Modbus(pBuf, ucLen);
    pBuf[ucLen++] = crc >> 8;
    pBuf[ucLen++] = crc;

    Uart_modbus_Handle.pTxBuf = pBuf;
    Uart_modbus_Handle.tx_size = ucLen;
    UartSend(&Uart_modbus_Handle);

}

/*******************************************************************************
  * @FunctionName: MODS_SendAckErr
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:14:33+0800
  * @Purpose:
  * @param:        _ucErrCode
  * @return:
*******************************************************************************/
static void MODS_SendAckErr(uint8_t _ucErrCode)
{
    uint8_t txbuf[3];

    txbuf[0] = _gModS.TxBuf[0];                     /* 485地址 */
    txbuf[1] = _gModS.TxBuf[1] | 0x80;              /* 异常的功能码 */
    txbuf[2] = _ucErrCode;                          /* 错误代码(01,02,03,04) */

    Uart_modbus_Handle.pTxBuf = txbuf;
    Uart_modbus_Handle.tx_size = 3;
    UartSend(&Uart_modbus_Handle);
}

/*******************************************************************************
  * @FunctionName: MODS_SendAckOk
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:15:43+0800
  * @Purpose:
  * @param:        void
  * @return:
*******************************************************************************/
static void MODS_SendAckOk(void)
{
    MODS_SendWithCRC(_gModS.TxBuf, 6);
}


/*******************************************************************************
  * @FunctionName: MODS_03H
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:15:58+0800
  * @Purpose:
  * @param:        p
  * @return:
*******************************************************************************/
uint32_t c1, c2;
static void MODS_03H(ModbusRevData_t *p)
{
    /*
        从机地址为11H。保持寄存器的起始地址为006BH，结束地址为006DH。该次查询总共访问3个保持寄存器。

        主机发送:
            11 从机地址
            03 功能码
            00 寄存器地址高字节
            6B 寄存器地址低字节
            00 寄存器数量高字节
            03 寄存器数量低字节
            76 CRC高字节
            87 CRC低字节

        从机应答:   保持寄存器的长度为2个字节。对于单个保持寄存器而言，寄存器高字节数据先被传输，
                    低字节数据后被传输。保持寄存器之间，低地址寄存器先被传输，高地址寄存器后被传输。
            11 从机地址
            03 功能码
            06 字节数
            00 数据1高字节(006BH)
            6B 数据1低字节(006BH)
            00 数据2高字节(006CH)
            13 数据2 低字节(006CH)
            00 数据3高字节(006DH)
            00 数据3低字节(006DH)
            38 CRC高字节
            B9 CRC低字节

        例子:
            01 03 30 06 00 01  6B0B      ---- 读 3006H, 触发电流
            01 03 4000 0010 51C6         ---- 读 4000H 倒数第1条浪涌记录 32字节
            01 03 4001 0010 0006         ---- 读 4001H 倒数第1条浪涌记录 32字节

            01 03 F000 0008 770C         ---- 读 F000H 倒数第1条告警记录 16字节
            01 03 F001 0008 26CC         ---- 读 F001H 倒数第2条告警记录 16字节

            01 03 7000 0020 5ED2         ---- 读 7000H 倒数第1条波形记录第1段 64字节
            01 03 7001 0020 0F12         ---- 读 7001H 倒数第1条波形记录第2段 64字节

            01 03 7040 0020 5F06         ---- 读 7040H 倒数第2条波形记录第1段 64字节
    */


    uint16_t reg;
    uint16_t num;
    uint16_t i;
    uint8_t reg_value[255];
    _gModS.RspCode = RSP_OK;

    if (p->data_size != 8)                          /* 03H命令必须是8个字节 */
    {
        _gModS.RspCode = RSP_ERR_VALUE;                 /* 数据值域错误 */
        goto err_ret;
    }

    reg = BEBufToUint16(&p->pData[2]);              /* 寄存器号 */
    num = BEBufToUint16(&p->pData[4]);              /* 寄存器个数 */

    if (num > sizeof(reg_value) / 2)
    {
        _gModS.RspCode = RSP_ERR_VALUE;                 /* 数据值域错误 */
        goto err_ret;
    }

    for (i = 0; i < num; i++)
    {
        if (MODS_ReadRegValue(reg, &reg_value[2 * i]) == false) /* 读出寄存器值放入reg_value */
        {
            _gModS.RspCode = RSP_ERR_REG_ADDR;              /* 寄存器地址错误 */
            break;

        }
        reg++;
    }

err_ret:
    if (_gModS.RspCode == RSP_OK)                           /* 正确应答 */
    {
        _gModS.TxCount = 0;
        _gModS.TxBuf[_gModS.TxCount++] = p->pData[0];
        _gModS.TxBuf[_gModS.TxCount++] = p->pData[1];
        _gModS.TxBuf[_gModS.TxCount++] = num * 2;           /* 返回字节数 */

        for (i = 0; i < num; i++)
        {
            _gModS.TxBuf[_gModS.TxCount++] = reg_value[2 * i];
            _gModS.TxBuf[_gModS.TxCount++] = reg_value[2 * i + 1];
        }
        c1 = tim_get_systime();
        MODS_SendWithCRC(_gModS.TxBuf, _gModS.TxCount); /* 发送正确应答 */
        c2 = tim_get_systime() - c1;
    }
    else
    {
        _gModS.TxBuf[0] = p->pData[0];
        _gModS.TxBuf[1] = p->pData[1];
        MODS_SendAckErr(_gModS.RspCode);                    /* 发送错误应答 */
    }
}

/*******************************************************************************
  * @FunctionName: MODS_10H
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:17:15+0800
  * @Purpose:
  * @param:        p
  * @return:
*******************************************************************************/
static void MODS_10H(ModbusRevData_t *p)
{
    /*
        从机地址为11H。保持寄存器的其实地址为0001H，寄存器的结束地址为0002H。总共访问2个寄存器。
        保持寄存器0001H的内容为000AH，保持寄存器0002H的内容为0102H。

        主机发送:
            11 从机地址
            10 功能码
            00 寄存器起始地址高字节
            01 寄存器起始地址低字节
            00 寄存器数量高字节
            02 寄存器数量低字节
            04 字节数
            00 数据1高字节
            0A 数据1低字节
            01 数据2高字节
            02 数据2低字节
            C6 CRC校验高字节
            F0 CRC校验低字节

        从机响应:
            11 从机地址
            06 功能码
            00 寄存器地址高字节
            01 寄存器地址低字节
            00 数据1高字节
            01 数据1低字节
            1B CRC校验高字节
            5A  CRC校验低字节

        例子:
            01 10 30 00 00 06 0C  07 DE  00 0A  00 01  00 08  00 0C  00 00     389A    ---- 写时钟 2014-10-01 08:12:00
            01 10 30 00 00 06 0C  07 DF  00 01  00 1F  00 17  00 3B  00 39     5549    ---- 写时钟 2015-01-31 23:59:57

    */

    uint16_t reg_addr;
    uint16_t reg_num;
    uint8_t byte_num;
    uint8_t i;
    uint16_t value;

    _gModS.RspCode = RSP_OK;

    if (p->data_size < 11)
    {
        _gModS.RspCode = RSP_ERR_VALUE;         /* 数据值域错误 */
        goto err_ret;
    }

    reg_addr = BEBufToUint16(&p->pData[2]);     /* 寄存器号 */
    reg_num = BEBufToUint16(&p->pData[4]);      /* 寄存器个数 */
    byte_num = p->pData[6];                     /* 后面的数据体字节数 */

    if (byte_num != 2 * reg_num)
    {
        _gModS.RspCode = RSP_ERR_VALUE;         /* 数据值域错误 */
        goto err_ret;
    }

    for (i = 0; i < reg_num; i++)
    {
        value = BEBufToUint16(&p->pData[7 + 2 * i]);    /* 寄存器值 */

        if (MODS_WriteRegValue(reg_addr + i, value) == false)
        {
            _gModS.RspCode = RSP_ERR_REG_ADDR;      /* 寄存器地址错误 */
            break;
        }
    }

err_ret:
    if (_gModS.RspCode == RSP_OK)                   /* 正确应答 */
    {
        memcpy(_gModS.TxBuf, p->pData, 6);
        MODS_SendAckOk();
    }
    else
    {
        MODS_SendAckErr(_gModS.RspCode);            /* 告诉主机命令错误 */
    }
}

void MODSAnalyzeApp(ModbusRevData_t *p)
{
    uint16_t crc1;
    uint16_t addr;

    /* 计算CRC校验和 */
    crc1 = CRC16_Modbus(p->pData, (p->data_size));
    if (crc1 != 0)
    {
        return;
    }

    /* 站地址 (1字节） */
    addr = p->pData[0];                         /* 第1字节 站号 */
    if (addr != SADDRMASTER)                    /* 判断主机发送的命令地址是否符合 */
    {
        return;
    }

    switch (p->pData[1])                /* 第2个字节 功能码 */
    {
        case 0x03:                      /* 读取保持寄存器 */
        {
            MODS_03H(p);
            break;
        }
        case 0x10:                      /* 写多个保存寄存器*/
        {
            MODS_10H(p);
            break;
        }
        default:
            break;
    }

}


void ModSProcess(void)
{
    ModbusRevData_t *p;

    if(QueueGet(&gModbusRevQueue, (void *)&p) == true)
    {
        /* 分析应用层协议 */
        MODSAnalyzeApp(p);
    }
}

void ModS_IRQHandler(void)
{
    UartIrqProcess(&Uart_modbus_Handle);                                                                                                                           // 接收完成
}
