/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * <tr><td>2024-2-4    <td>V1.0.1    <td>xingxiao    <td>modbus_reg LSB Save
  * </table>
  ******************************************************************************
  */

#include "modbus_analysis.h"
#include "private_analysis.h"
BMSPara_t       gSlaveBMSPara;
DCDCPara_t      gTempSlaveDCDCPara;
char  extend_version[24];
void BmsRegInit(void)
{
    uint16_t i = 0;
    memset(gstBmsReg, 0, sizeof(BMSPara_t));
    memset(&gSlaveBMSPara, 0, sizeof(BMSPara_t));
    for(i = 0; i < sizeof(BMSPara_t); i++)
    {
        *((uint16_t*)gstBmsReg + i) = i;
        *((uint16_t*)&gSlaveBMSPara + i) = i;
    }
}

void DCDCRegInit(void)
{
    uint16_t i = 0;
    memset(gstDCDCReg, 0, sizeof(DCDCPara_t));
    memset(&gTempSlaveDCDCPara, 0, sizeof(DCDCPara_t));
    for(i = 0; i < sizeof(DCDCPara_t); i++)
    {
        *((uint16_t*)gstDCDCReg + i) = 0;
    }
    SpiFlashReadAllData(FPGA_FIRMWARE_VERSION, (uint8_t *)extend_version, 24);//读出扩展的字段 上电状态不稳定容易导致flash读取数据异常
    SpiFlashReadAllData(FPGA_FIRMWARE_VERSION, (uint8_t *)extend_version, 24);//读出扩展的字段
    SpiFlashReadAllData(FPGA_FIRMWARE_VERSION, (uint8_t *)extend_version, 24);//读出扩展的字段

    memcpy(&gTempSlaveDCDCPara.dc_dc_only_read_para.version_extend, &extend_version, 24);

    memcpy(((uint8_t *) & (gstDCDCReg->dc_dc_only_read_para)), ((uint8_t *) & (gTempSlaveDCDCPara.dc_dc_only_read_para)), sizeof(DCDCPara_t));

}

//void BmsModBusHostWrite10(uint8_t slave_addr, uint16_t reg_start_addr, uint16_t reg_num, uint8_t data_num, uint16_t *pReg_value)
//{
//    uint16_t crc;
//    uint16_t i;
//    uint16_t tx_size;
//    uint8_t *p = NULL;

//    p[0] = slave_addr;
//    p[1] = 0x10;
//    p[2] = (reg_start_addr >> 8) & 0xFF;
//    p[3] = reg_start_addr & 0xFF;
//    p[4] = (reg_num >> 8) & 0xFF;
//    p[5] = reg_num & 0xFF;
//    p[6] = data_num;
//    for(i = 0; i < (data_num >> 1); i++)
//    {
//        p[2 * i + 7] = (pReg_value[i] >> 8) & 0xFF;
//        p[2 * i + 8] = pReg_value[i] & 0xFF;
//    }
//    crc = CrcModbus(p, 7 + data_num);
//    p[2 * i + 7] = crc & 0xFF;
//    p[2 * i + 8] = (crc >> 8) & 0xFF;
//    tx_size = 7 + data_num + 2;
//    Uart0Send(p, tx_size);
//}

//static void _BmsSlaveDisorderDeal(uint8_t disorder_slave_addr)
//{
//    if(disorder_slave_addr == SLAVE_BMS_ADDR)
//    {

//    }
//}

//static void _BmsSlaveNoAckDeal(uint8_t slave_addr)
//{
//    if(slave_addr == SLAVE_PCS_ADDR)
//    {

//    }
//}

//static void _BmsSlaveReadErrorDeal(uint8_t slave_addr, EN_SLAVE_ERROR_CODE err_code)
//{

//    if(slave_addr == SLAVE_PCS_ADDR)
//    {
//    }
//}

//static void _BmsSlaveReadAllDeal(ModbusRevData_t *p)
//{
//    uint8_t data_num = p->rev_03.data_num;
//    uint8_t i = 0;
//    uint8_t *pRev;
//    if(p->slave_addr == SLAVE_PCS_ADDR)
//    {

//    }
//    for(i = 0; i < (data_num >> 1); i++)
//    {
//        pRev[2 * i] = p->rev_03.pRev[2 * i + 1];
//        pRev[2 * i + 1] = p->rev_03.pRev[2 * i];
//    }
//}

//static void _BmsSlaveWriteDeal(ModbusRevData_t *p)
//{
//    /*暂且认为回复正确格式的包即表示成功*/
//}

//void ModbusCommProcess()
//{
//    static EN_SLAVE_POLLING_STATUS  _BmsSlavePollingStatus;
//    static uint32_t timeout;
//    static ModbusRevData_t *p;
//    static SlaveBmsWrite_t *pBmsSlaveWrite;
//    static uint8_t slave_addr;/*需要回复的设备地址*/
//    static uint8_t func_code;
//    static uint32_t bms_slave_read_polling_cnt = 0;

//    switch(_BmsSlavePollingStatus)
//    {
//        case EN_SLAVE_POLLING_STATUS_IDLE:
//        {
//            /*空闲时段也要执行Get gModbusRevQueue, 并舍弃，防止队列满导致无法再接收数据*/
//            QueueGet(&gBMSModebusRevQueue, (void *)&p);
//            if(QueueGet(&gBmsModbusSlaveCommSendQueue, (void *)&pBmsSlaveWrite) == true)/*如果有主动写操作*/
//            {
//                if(pBmsSlaveWrite->data_type == 0xAA)
//                {
//                    _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_REV_ADD_BMS_INTERVAL_WAIT;
//                    timeout = TimGetSystime();
//                }
//                else
//                {
//                    func_code = 0x10;
//                    BmsModBusHostWrite10(pBmsSlaveWrite->slave_addr, pBmsSlaveWrite->reg_addr, pBmsSlaveWrite->reg_num, (pBmsSlaveWrite->reg_num << 1), pBmsSlaveWrite->word_data);
//                    _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_WAITING_REV;
//                    timeout = TimGetSystime();
//                    slave_addr = pBmsSlaveWrite->slave_addr;
//                }
//            }
//            else
//            {
//                /*轮询回读*/
//                if((uint32_t)(TimGetSystime() - bms_slave_read_polling_cnt) > SLAVE_COMM_POLLING_TIME)
//                {
//                    func_code = 0x03;
//                    bms_slave_read_polling_cnt = TimGetSystime();
//                    _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_WAITING_REV;
//                    timeout = TimGetSystime();
//                }
//            }
//            break;
//        }
//        case EN_SLAVE_POLLING_STATUS_WAITING_REV:
//        {
//            if(QueueGet(&gBMSModebusRevQueue, (void *)&p) == true)
//            {
//                if(slave_addr == p->slave_addr)
//                {
//                    if(func_code == p->func_code)
//                    {
//                        timeout = TimGetSystime();
//                        _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_REV_OK;

//                    }
//                    else if((func_code | 0x80) == p->func_code)
//                    {
//                        timeout = TimGetSystime();
//                        _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_REV_ERROR;
//                    }
//                    switch(slave_addr)
//                    {
//                        default:
//                            break;
//                    }
//                    break;
//                }
//                else/*非目标设备的回复*/
//                {
//                    _BmsSlaveDisorderDeal(p->slave_addr);
//                }
//            }
//            if((uint32_t)(TimGetSystime() - timeout) > SLAVE_COMM_REV_MAX_TIME)
//            {
//                _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_REV_OVERTIME;
//            }
//            break;
//        }
//        case EN_SLAVE_POLLING_STATUS_REV_OVERTIME:
//        {
//            /*做传输超时的处理*/
//            _BmsSlaveNoAckDeal(slave_addr);
//            _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;

//            break;
//        }

//        case EN_SLAVE_POLLING_STATUS_REV_OK:
//        {
//            if(func_code == 0x03)
//            {
//                /*更新数据*/
//                _BmsSlaveReadAllDeal(p);

//            }
//            else if(func_code == 0x10)
//            {
//                _BmsSlaveWriteDeal(p);
//            }

//            timeout = TimGetSystime();
//            _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_REV_INTERVAL_WAIT;
//            break;
//        }
//        case EN_SLAVE_POLLING_STATUS_REV_INTERVAL_WAIT:
//        {

//            if((uint32_t)(TimGetSystime() - timeout) > SLAVE_COMM_BMS_INTERVAL_TIME)
//            {
//                _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
//                bms_slave_read_polling_cnt = TimGetSystime();
//                timeout = TimGetSystime();
//            }
//            break;
//        }

//        case  EN_SLAVE_POLLING_STATUS_REV_ADD_BMS_INTERVAL_WAIT:
//        {
//            if((uint32_t)(TimGetSystime() - timeout) > 2000)
//            {
//                _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
//                bms_slave_read_polling_cnt = TimGetSystime();
//                timeout = TimGetSystime();
//            }
//            break;
//        }
//        case EN_SLAVE_POLLING_STATUS_REV_ERROR:
//        {
//            /*错误信息处理*/
//            _BmsSlaveReadErrorDeal(p->slave_addr, (EN_SLAVE_ERROR_CODE)p->rev_error.err_code);
//            timeout = TimGetSystime();
//            _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_REV_INTERVAL_WAIT;
//            break;
//        }
//        default:
//            _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
//            break;
//    }
//}
