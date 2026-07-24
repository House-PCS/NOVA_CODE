/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-4-24
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-4-24    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-4-24
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-4-24    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
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
#include "ethernet_analysis.h"
#include "firmware_upgrate.h"
#include "sci.h"
ArcPara_t      gstArcReg;
CoproPara_t    gstCoproReg;
SlaveRead_t    gstModbusReadFrame;
SlaveWrite_t   gstModbusWriteFrame;
ModBusPara_t   gstModBusPara;
meter_info_t   gstMeterInfo;
static ModbusAckStatus_t _gstModbusAckStatus;
Queue_t gstModbusSendQueue;
static uint8_t _ModbusAckData[300];
static uint16_t _ModbusAckSize;
static EN_MODBUS_REV_STATUS _eSlaveModSta = EN_MODBUS_REV_STATUS_IDLE;




const ReadDealFram_t  garrReadDealFram[EN_READ_EVENT_NUM] =
{
    {EN_COMSERVICE_COM_INDEX0, MODBUS_SLAVE0_ADDR, EN_ARC_READ_INPUT_EVENT, READ_INPUT_TYPE,   ARC_MODBUS_READ_START, (uint16_t *)&(gstModBusPara.arc_para.arc_only_read_para), (int)sizeof(ArcOnlyReadPara_t), &_gstModbusAckStatus.read_arc_only_read},
    {EN_COMSERVICE_COM_INDEX0, MODBUS_SLAVE0_ADDR, EN_ARC_READ_HOLD_EVENT,  READ_HOLD_TYPE,    ARC_MODBUS_HOLD_START, (uint16_t *)&(gstModBusPara.arc_para.arc_hold_para),      (int)sizeof(ArcHoldPara_t), &_gstModbusAckStatus.read_arc_hold},
    {EN_COMSERVICE_COM_INDEX2, MODBUS_SLAVE6_ADDR, EN_COPRO_READ_INPUT_EVENT, READ_INPUT_TYPE,   COPRO_MODBUS_READ_START, (uint16_t *)&(gstModBusPara.copro_para.copro_only_read_para), (int)sizeof(CoproOnlyReadPara_t), &_gstModbusAckStatus.read_copro_only_read},
    {EN_COMSERVICE_COM_INDEX2, MODBUS_SLAVE6_ADDR, EN_COPRO_READ_HOLD_EVENT,  READ_HOLD_TYPE,    COPRO_MODBUS_HOLD_START, (uint16_t *)&(gstModBusPara.copro_para.copro_hold_para),      (int)sizeof(CoproHoldPara_t), &_gstModbusAckStatus.read_copro_hold},
};


static void _ModbusAckSuccessSet(EN_READ_EVENT e_index);
static void _ModbusAckReset(EN_READ_EVENT e_index);
static bool _ModbusAckGet(EN_READ_EVENT e_index);

/**
* @brief: _HexFloatActPowerDirection
* @param: hex_float                   
* @return: 0:功率为负   1：功率为正
* @note: 
*
*/
static int _HexFloatActPowerDirection(uint32_t hex_float)
{
    float f = *(float*)&hex_float;  // 按浮点数解析十六进制

    if(f < 0)
    {
        return 0;
    }
    return 1;
}


/**
* @brief: _HexFloatCurrToInt
* @param: hex_float: read meter data
* @param: meter_type:   0:CHNT_three  1:CHNT_single
* @param: r_curr_direction 0:负方向（流出为负） 1：正方向（流入为正）
* @note:
*
*/
static int _HexFloatCurrToInt(uint32_t hex_float, uint16_t meter_type, uint16_t curr_direction)
{
    float f = *(float*)&hex_float;  // 按浮点数解析十六进制
    int value = 0;

    if(meter_type == 0)  //0.001A
    {
        value = (int)(f / 100);
    }
    else if(meter_type == 1)    // 1A
    {
        value = (int)(f * 10);
    }

    if(!curr_direction)
    {
        value = -value;
    }

    // 返回整数部分
    return (int)value;
}

void ModbusRegInit(void)
{
    memset((uint16_t*)&gstModBusPara.arc_para, 0, (int)sizeof(ArcPara_t));
    memset((uint16_t*)&gstModBusPara.copro_para, 0, (int)sizeof(CoproPara_t));
    memset((uint16_t*)&_gstModbusAckStatus, 0xFF, (int)sizeof(ModbusAckStatus_t));
    memset((uint16_t*)&gstCoproReg, 0, (int)sizeof(CoproPara_t));
    QueueInit(&gstModbusSendQueue, MODBUS_WRITE_QUEUE_DEPTH, sizeof(SlaveWrite_t));  
}

void ReadMenterCurrentProcess(void)
{
    uint16_t data[10];
    uint16_t len = 0;
    uint16_t crc;
    static uint16_t status = 0;
    static uint32_t timeout = 0;
    uint16_t MeterBrand = 0;
    uint16_t readaddr = 0;
    uint16_t funcode = 0;
    uint16_t read_reg_num = 0;
    static uint16_t timeout_count = 0;

    // if(MeterModeGet() == false)
    // {
    //     return;
    // }
    
    if(status == 0)
    {
        _gstModbusAckStatus.read_memter_current = 0xFF;
        MeterBrand = MeterBrandGet();
        //MeterBrand = 1;

        /*根据不同电表品牌确定对应读取电流地址*/
        if(MeterBrand == 0)     //正泰电表
        {
            //单相逆变器
            if(gstVersionInfo.pcs_version_info.device_type <= 0x7F)  //正泰DDSU666.010.0002型
            {
                funcode = 0x03;
                read_reg_num = 0x04;
                readaddr = 0x2002;
            }
            else if(gstVersionInfo.pcs_version_info.device_type <= 0xFF) //三相逆变器 正泰DRSU666型
            {
                funcode = 0x03;
                read_reg_num = 0x0D;
                readaddr = 0x200C;
            }
            else
            {
                return;
            }
        }
        else if(MeterBrand == 1)    //安科瑞电表 暂不支持
        {
            //单相逆变器
            if(gstVersionInfo.pcs_version_info.device_type <= 0x7F)
            {
                funcode = 0x03;
                read_reg_num = 0x06;
                readaddr = 0x2014;
            }
            else if(gstVersionInfo.pcs_version_info.device_type <= 0xFF) //三相逆变器
            {
                funcode = 0x04;
                read_reg_num = 0x06;
                readaddr = 0x200C;
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
        
        /*发送读取电流指令*/
        data[len++] = MODBUS_SLAVE3_ADDR;
        data[len++] = funcode;
        data[len++] = (readaddr >> 8) & 0xFF;
        data[len++] = readaddr & 0xFF;
        data[len++] = (read_reg_num >> 8) & 0xFF;;
        data[len++] = read_reg_num & 0xFF;

        crc = CrcModbus((uint8_t*)&data, len);
        data[len++] = crc & 0xFF;
        data[len++] = (crc >> 8) & 0xFF;

        PrivateDataSend(EN_COMSERVICE_COM_INDEX1, (uint8_t*)&data, len);   
        timeout = TimGetSystime();
        status = 1;
    }
    else if(status == 1)
    {
        if(_gstModbusAckStatus.read_memter_current == 0x00)
        {
//            printf("ok\r\n");
            timeout = TimGetSystime();
            gstMeterInfo.reset_sci_count = 0;
            if(timeout_count)
            {
                timeout_count--;
            }
            status = 2;
        }

        if(TimDifftimeFromNow(timeout) > 90)
        {
            timeout = TimGetSystime();
      //      printf("read curr timeout\r\n");
            timeout_count++;

            if(timeout_count > 10)
            {
                ScicFifoInit();
                timeout_count = 0;
                gstMeterInfo.reset_sci_count ++;
           //     printf("reset_sci_count:%d\r\n", gstMeterInfo.reset_sci_count);
            }

            status = 2;
        }
    }
    else if(status == 2)
    {
//        if(TimDifftimeFromNow(timeout) > 10)
//        {
            status = 0;
//        }
    }    
}

/**
* @brief: ModBusHostWrite10
* @param: index: which sci                       
* @param: slave_addr                  
* @param: reg_start_addr              
* @param: reg_num                     
* @param: data_num                    
* @param: pReg_value                  
* @note: 
*
*/
void ModBusHostWrite10(EN_COMSERVICE_INDEX index, uint8_t slave_addr, uint16_t reg_start_addr, uint16_t reg_num, uint8_t data_num, uint16_t *pReg_value)
{
    uint16_t crc;
    uint16_t i;
    uint16_t tx_size;
    uint8_t *p = NULL;

    p[0] = slave_addr;
    p[1] = 0x10;
    p[2] = (reg_start_addr >> 8) & 0xFF;
    p[3] = reg_start_addr & 0xFF;
    p[4] = (reg_num >> 8) & 0xFF;
    p[5] = reg_num & 0xFF;
    p[6] = data_num;
    for(i = 0; i < (data_num >> 1); i++)
    {
        p[2 * i + 7] = (pReg_value[i] >> 8) & 0xFF;
        p[2 * i + 8] = pReg_value[i] & 0xFF;
    }
    crc = CrcModbus(p, 7 + data_num);
    p[2 * i + 7] = crc & 0xFF;
    p[2 * i + 8] = (crc >> 8) & 0xFF;
    tx_size = 7 + data_num + 2;
//    Uart0Send(p, tx_size);
    PrivateDataSend(index, p, tx_size);
}

void ModbusHostRead(SlaveRead_t *p_slave_read)
{
    uint16_t len = 0;
    uint16_t crc;

    p_slave_read->data[len++] = p_slave_read->slave_addr;
    p_slave_read->data[len++] = p_slave_read ->func_code;
    p_slave_read->data[len++] = (p_slave_read->reg_addr >> 8) & 0xFF;
    p_slave_read->data[len++] = (p_slave_read->reg_addr) & 0xFF;
    p_slave_read->data[len++] = (p_slave_read->reg_num >> 8) & 0xFF;
    p_slave_read->data[len++] = (p_slave_read->reg_num) & 0xFF;

    crc = CrcModbus((uint8_t*)&p_slave_read->data, len);
    p_slave_read->data[len++] = crc & 0xFF;
    p_slave_read->data[len++] = (crc >> 8) & 0xFF;

    PrivateDataSend(p_slave_read->e_sci_index, (uint8_t*)&p_slave_read->data, len);
}

//static void _BmsSlaveWriteDeal(ModbusRevData_t *p)
//{
//    /*暂且认为回复正确格式的包即表示成功*/
//}

static void _SlaveReadPolling(SlaveRead_t *p_slave_frame)
{
    p_slave_frame->e_sci_index = garrReadDealFram[p_slave_frame->e_event_index].e_sci_index;
    p_slave_frame->slave_addr  = garrReadDealFram[p_slave_frame->e_event_index].slave_addr;
    p_slave_frame->func_code   = garrReadDealFram[p_slave_frame->e_event_index].func_code;
    p_slave_frame->reg_addr    = garrReadDealFram[p_slave_frame->e_event_index].reg_addr;
    p_slave_frame->reg_num     = garrReadDealFram[p_slave_frame->e_event_index].reg_num;
    ModbusHostRead(p_slave_frame);  
}


void UpdataCoproReg(void)
{

    static uint16_t cop_run_state = 0;
    static uint32_t cop_timeout = 0;

    if (memcmp(&(gstCoproReg.copro_only_read_para), &(gstModBusPara.copro_para.copro_only_read_para), sizeof(CoproOnlyReadPara_t)) ==0)
    {
        
    }
    else //如果轮询回读的输入寄存器值有变化，则更新本地寄存器值
    {
        memcpy(&(gstCoproReg.copro_only_read_para), &(gstModBusPara.copro_para.copro_only_read_para), sizeof(CoproOnlyReadPara_t));
    }

    if (memcmp(&(gstCoproReg.copro_hold_para), &(gstModBusPara.copro_para.copro_hold_para), sizeof(CoproHoldPara_t))==0)
    {

    }
    else 
    {

        if (gstCoproReg.copro_hold_para.sys_upgrade_status != gstModBusPara.copro_para.copro_hold_para.sys_upgrade_status)
        {
            gstModbusWriteFrame.reg_addr = 0x64D0;
            gstModbusWriteFrame.data[0] = gstCoproReg.copro_hold_para.sys_upgrade_status;
        }
        else if (gstCoproReg.copro_hold_para.dsp_heart_beat != gstModBusPara.copro_para.copro_hold_para.dsp_heart_beat)
        {
            gstModbusWriteFrame.reg_addr = 0x64D1;
            gstModbusWriteFrame.data[0] = gstCoproReg.copro_hold_para.dsp_heart_beat;
        }
        else if (gstCoproReg.copro_hold_para.mcu_rly_cntl != gstModBusPara.copro_para.copro_hold_para.mcu_rly_cntl)
        {
            gstModbusWriteFrame.reg_addr = 0x64D2;
            gstModbusWriteFrame.data[0] = gstCoproReg.copro_hold_para.mcu_rly_cntl;
        }
        else if (gstCoproReg.copro_hold_para.mcu_rly_state != gstModBusPara.copro_para.copro_hold_para.mcu_rly_state)
        {
            gstModbusWriteFrame.reg_addr = 0x64D3;
            gstModbusWriteFrame.data[0] = gstCoproReg.copro_hold_para.mcu_rly_state;
        }

        if(cop_run_state == 0)
        {
            /*清除Ack标志*/
            _gstModbusAckStatus.write_copro_hold = 0xFF;
            
            gstModbusWriteFrame.e_sci_index = EN_COMSERVICE_COM_INDEX2;
            gstModbusWriteFrame.func_code = WRITE_HOLD_TYPE;
            gstModbusWriteFrame.reg_num = 1;
            gstModbusWriteFrame.slave_addr = MODBUS_SLAVE6_ADDR;
            gstModbusWriteFrame.data_num = 2;   
            QueuePut(&gstModbusSendQueue, &gstModbusWriteFrame);    
            //if(QueuePut(&gstModbusSendQueue, &gstModbusWriteFrame) == false)
            // {
            //     return false;
            // }
            cop_timeout = TimGetSystime();
            cop_run_state = 1;
        }
        else if(cop_run_state == 1)
        {
            if(_gstModbusAckStatus.write_copro_hold == 0)
            {
    //            printf("Set Copro Reset ok!\r\n");
                if (_ModbusAckGet(EN_COPRO_READ_HOLD_EVENT) == true)
                {
                    cop_run_state = 0;
                }
                //return true;
            }

            if(TimDifftimeFromNow(cop_timeout) > 500)
            {
    //            printf("Set Copro Reset Timeout!\r\n");
                // QueueClear(&gstModbusSendQueue);
                // cop_run_state = 0;
                //return false;
            }
        }    
    }
    
    // memcmp(&(gstCoproReg.copro_only_read_para), &(gstModBusPara.copro_para.copro_only_read_para), sizeof(CoproOnlyReadPara_t));
    // memcmp(&(gstCoproReg.copro_hold_para), &(gstModBusPara.copro_para.copro_hold_para), sizeof(CoproHoldPara_t));

}




bool WriteArcHoldReg(uint16_t reg_addr, uint16_t value)
{
    static uint16_t arc_run_state = 0;
    static uint32_t arc_timeout = 0;

    if (arc_run_state == 0)
    {
        _gstModbusAckStatus.write_arc_hold = 0xFF;

        gstModbusWriteFrame.e_sci_index = EN_COMSERVICE_COM_INDEX0;
        gstModbusWriteFrame.func_code = WRITE_HOLD_TYPE;
        gstModbusWriteFrame.reg_addr = reg_addr;
        gstModbusWriteFrame.reg_num = 1;
        gstModbusWriteFrame.slave_addr = MODBUS_SLAVE0_ADDR;
        gstModbusWriteFrame.data_num = 2;
        gstModbusWriteFrame.data[0] = value;
        if (QueuePut(&gstModbusSendQueue, &gstModbusWriteFrame) == false)
        {
            return false;
        }
        arc_timeout = TimGetSystime();
        arc_run_state = 1;
    }
    else if (arc_run_state == 1)
    {
        if (_gstModbusAckStatus.write_arc_hold == 0)
        {
            arc_run_state = 0;
            return true;
        }

        if (TimDifftimeFromNow(arc_timeout) > 500)
        {
            printf("WriteArcHoldReg Timeout!\r\n");
            arc_run_state = 0;
            return false;
        }
    }
    return false;
}


void ModbusProcess(void)
{
    static EN_SLAVE_POLLING_STATUS  e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
    static EN_READ_EVENT e_event_count = EN_ARC_READ_INPUT_EVENT;
    static uint32_t timeout;
//    static ModbusRevData_t *p;
    static SlaveWrite_t *p_slave_write_frame;
//    static uint8_t slave_addr;/*需要回复的设备地址*/
//    static uint8_t func_code;
    static uint32_t slave_read_polling_cnt = 0;

    gstModBusPara.arc_para.arc_hold_para.arc_detect_enable = 1;   //默认使能弧光检测功能

    /*读取电表电流*/
//    ReadMenterCurrentProcess();
    UpdataCoproReg();
    WriteArcHoldReg(0x6450, 1);
//    SetCoproSysReset();

    switch(e_slave_pollingStatus)
    {
        case EN_SLAVE_POLLING_STATUS_IDLE:
        {
            if((GetEthSysUpgradeState() == true) || (PrivateUpgradeRunning() == true) || (ArcFirmwareSending() == true) || (CoproFirmwareSending() == true))
                {
                    return;
                }

            /*如果有主动写操作*/
            if(QueueGet(&gstModbusSendQueue, (void *)&p_slave_write_frame) == true)
            {
                if(p_slave_write_frame->func_code == 0X00)
                {//私有协议 
//                    ModbusPCSRTUSend((uint8_t *)pPcsArcSlaveWrite->data, 10); 
//                    _gePcsArcSlavePollingStatus = EN_PCS_ARC_POLLING_STATUS_REV_INTERVAL_WAIT;
//                    pcs_arc_read_polling_cnt = tim_get_systime();                    
                }
                else
                {
                    /*写modbus数据*/
                    gstModbusReadFrame.func_code = p_slave_write_frame->func_code;
                    gstModbusReadFrame.slave_addr = p_slave_write_frame->slave_addr;
                    ModBusHostWrite10(p_slave_write_frame->e_sci_index, p_slave_write_frame->slave_addr, \
                                      p_slave_write_frame->reg_addr, p_slave_write_frame->reg_num, \
                                      p_slave_write_frame->data_num, p_slave_write_frame->data);
                    e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_WAIT_WRITE_ACK;
                    _ModbusAckReset(EN_COPRO_READ_HOLD_EVENT);
                    timeout = TimGetSystime();                        
                }
            }
            else
            {

                /*轮询回读*/
                if(TimDifftimeFromNow(slave_read_polling_cnt) > SLAVE_COMM_POLLING_TIME)
                {
                    gstModbusReadFrame.e_event_index = e_event_count;
                    _ModbusAckReset(e_event_count);
                    _SlaveReadPolling(&gstModbusReadFrame);
                    slave_read_polling_cnt = TimGetSystime();
                    e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_WAITING_REV;
                    timeout = TimGetSystime();
//                    printf("IDLE:%d\r\n", e_event_count);
                }
            }
            break;
        }
        case EN_SLAVE_POLLING_STATUS_WAITING_REV:
        {
            /*回读ok*/
            if(_ModbusAckGet(e_event_count) == true)
            {
                e_event_count++;
                e_event_count %= EN_READ_EVENT_NUM;
                e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
            }
            
            if(TimDifftimeFromNow(timeout) > SLAVE_COMM_REV_MAX_TIME)
            {
          //      printf("WAIT Timeout:%d\r\n", e_event_count);
                e_event_count++;
                e_event_count %= EN_READ_EVENT_NUM;
                
                e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_REV_OVERTIME;
            }
            break;
        }
        case EN_SLAVE_POLLING_STATUS_REV_OVERTIME:
        {
            e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;

            break;
        }
        case EN_SLAVE_POLLING_STATUS_WAIT_WRITE_ACK:
        {
            if(p_slave_write_frame->e_sci_index == EN_COMSERVICE_COM_INDEX0)
            {
                /*等待ARC回应ok*/
               if(_gstModbusAckStatus.write_arc_hold == 0)
               {
                    e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
               }
            }
            else if(p_slave_write_frame->e_sci_index == EN_COMSERVICE_COM_INDEX1)
            {
                /*等待电表回应ok*/
            }
            else if(p_slave_write_frame->e_sci_index == EN_COMSERVICE_COM_INDEX2)
            {
                /*等待协处理器回应ok*/
               if(_gstModbusAckStatus.write_copro_hold == 0)
               {
                    e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
               }
            }

            if(TimDifftimeFromNow(timeout) > SLAVE_COMM_REV_MAX_TIME)
            {
         //       printf("arc write ack  Timeout\r\n");
                e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_REV_OVERTIME;
            }
            break;
        }
        default:
            e_slave_pollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
            break;
    }
}

/**
* @brief: _ModbusAckSuccessSet
* @param: e_index                     
* @return: 
* @note: 
*
*/
static void _ModbusAckSuccessSet(EN_READ_EVENT e_index)
{
    if(e_index >= EN_READ_EVENT_NUM)
    {
        return;
    }

    *garrReadDealFram[e_index].ack_result = 0;
}

/**
* @brief: _ModbusAckReset
* @param: e_index                     
* @return: 
* @note: 
*
*/
static void _ModbusAckReset(EN_READ_EVENT e_index)
{
    if(e_index >= EN_READ_EVENT_NUM)
    {
        return;
    }

    *garrReadDealFram[e_index].ack_result = 0xFF;
}

static bool _ModbusAckGet(EN_READ_EVENT e_index)
{
    if(e_index >= EN_READ_EVENT_NUM)
    {
        return false;
    }

    if(*garrReadDealFram[e_index].ack_result == 0)
    {
        return true;
    }

    return false;
}
void ModbusHostAnalysis(SciRevData_t *p_com_cmd)
{
    uint16_t cmd;
    uint32_t current = 0;
    uint32_t power = 0;

    cmd = p_com_cmd->pData[1];

    switch(cmd)
    {
        case READ_INPUT_TYPE:
        {
            if(p_com_cmd->sci_index == EN_COMSERVICE_COM_INDEX0)
            {
                MemcpyU8toU16(garrReadDealFram[EN_ARC_READ_INPUT_EVENT].pdata, (uint16_t*)&p_com_cmd->pData[3], garrReadDealFram[EN_ARC_READ_INPUT_EVENT].reg_num);
                _ModbusAckSuccessSet(EN_ARC_READ_INPUT_EVENT);
//                printf("read only ok\r\n");
//                memcpy(garrReadDealFram[0].pdata, &p_com_cmd->pData[2], garrReadDealFram[0].reg_num);
            }
            else if(p_com_cmd->sci_index == EN_COMSERVICE_COM_INDEX1)
            {
                //TODO
            }
            else if(p_com_cmd->sci_index == EN_COMSERVICE_COM_INDEX2)
            {
                MemcpyU8toU16(garrReadDealFram[EN_COPRO_READ_INPUT_EVENT].pdata, (uint16_t*)&p_com_cmd->pData[3], garrReadDealFram[EN_COPRO_READ_INPUT_EVENT].reg_num);
                _ModbusAckSuccessSet(EN_COPRO_READ_INPUT_EVENT);
            }
            break;
        }
        case READ_HOLD_TYPE:
        {
            if(p_com_cmd->sci_index == EN_COMSERVICE_COM_INDEX0)
            {
                MemcpyU8toU16(garrReadDealFram[EN_ARC_READ_HOLD_EVENT].pdata, (uint16_t*)&p_com_cmd->pData[3], garrReadDealFram[EN_ARC_READ_HOLD_EVENT].reg_num);
                _ModbusAckSuccessSet(EN_ARC_READ_HOLD_EVENT);
//                printf("read hold ok\r\n");
//                memcpy(garrReadDealFram[0].pdata, &p_com_cmd->pData[2], garrReadDealFram[0].reg_num);
            }
            else if(p_com_cmd->sci_index == EN_COMSERVICE_COM_INDEX1)
            {
                //单相逆变器
                if(gstVersionInfo.pcs_version_info.device_type <= 0x7F)
                {
                    if (MeterBrandGet()==0)//CHNT 电表
                    {
                        current = ((((uint32_t)p_com_cmd->pData[3] << 24) & 0xFF000000) | (((uint32_t)p_com_cmd->pData[4] << 16) & 0x00FF0000) | (((uint32_t)p_com_cmd->pData[5] << 8) & 0x0000FF00) | ((uint32_t)p_com_cmd->pData[6] & 0x000000FF));
                        power = ((((uint32_t)p_com_cmd->pData[7] << 24) & 0xFF000000) | (((uint32_t)p_com_cmd->pData[8] << 16) & 0x00FF0000) | (((uint32_t)p_com_cmd->pData[9] << 8) & 0x0000FF00) | ((uint32_t)p_com_cmd->pData[10] & 0x000000FF));
    //                    printf("current:0x%04X%04X\r\n", (uint16_t)(current >> 16), (uint16_t)current);
                        gstMeterInfo.r_phase_meter_curr = _HexFloatCurrToInt(current, 1, _HexFloatActPowerDirection(power));
                        gstMeterInfo.r_phase_meter_pwr = _HexFloatCurrToInt(power, 1, 1);
                    }
                    else if(MeterBrandGet()==1)//Acrel电表
                    {
                        power = ((((uint32_t)p_com_cmd->pData[3] << 24) & 0xFF000000) | (((uint32_t)p_com_cmd->pData[4] << 16) & 0x00FF0000) | (((uint32_t)p_com_cmd->pData[5] << 8) & 0x0000FF00) | ((uint32_t)p_com_cmd->pData[6] & 0x000000FF));
                        gstMeterInfo.r_phase_meter_pwr = _HexFloatCurrToInt(power, 1, 1);
                    }
//                    gstUdpPcsPara.udp_pcs_upload.r_phase_meter_curr = _HexFloatCurrToInt(current, 1, _HexFloatActPowerDirection(power));
//                    printf("meter_curr:0x%04X\r\n", gstUdpPcsPara.udp_pcs_upload.r_phase_meter_curr);
                }
                else if(gstVersionInfo.pcs_version_info.device_type <= 0xFF) //三相逆变器
                {
                    /*R相*/
                    current = ((((uint32_t)p_com_cmd->pData[3] << 24) & 0xFF000000) | (((uint32_t)p_com_cmd->pData[4] << 16) & 0x00FF0000) | (((uint32_t)p_com_cmd->pData[5] << 8) & 0x0000FF00) | ((uint32_t)p_com_cmd->pData[6] & 0x000000FF));
                    power = ((((uint32_t)p_com_cmd->pData[19] << 24) & 0xFF000000) | (((uint32_t)p_com_cmd->pData[20] << 16) & 0x00FF0000) | (((uint32_t)p_com_cmd->pData[21] << 8) & 0x0000FF00) | ((uint32_t)p_com_cmd->pData[22] & 0x000000FF));
                    gstMeterInfo.r_phase_meter_curr = _HexFloatCurrToInt(current, 1, _HexFloatActPowerDirection(power));
                    gstMeterInfo.r_phase_meter_pwr = _HexFloatCurrToInt(power, 1, 1);
//                    printf("current:0x%04X%04X\r\n", (uint16_t)(current >> 16), (uint16_t)current);
//                    gstMeterInfo.r_phase_meter_curr = _HexFloatCurrToInt(current, 0, _HexFloatActPowerDirection(power));
//                    gstUdpPcsPara.udp_pcs_upload.r_phase_meter_curr = _HexFloatCurrToInt(current, 0, _HexFloatActPowerDirection(power));
//                    printf("r_meter_curr:0x%04X\r\n", gstUdpPcsPara.udp_pcs_upload.r_phase_meter_curr);
                    /*S相*/
                    current = ((((uint32_t)p_com_cmd->pData[7] << 24) & 0xFF000000) | (((uint32_t)p_com_cmd->pData[8] << 16) & 0x00FF0000) | (((uint32_t)p_com_cmd->pData[9] << 8) & 0x0000FF00) | ((uint32_t)p_com_cmd->pData[10] & 0x000000FF));
                    power = ((((uint32_t)p_com_cmd->pData[23] << 24) & 0xFF000000) | (((uint32_t)p_com_cmd->pData[24] << 16) & 0x00FF0000) | (((uint32_t)p_com_cmd->pData[25] << 8) & 0x0000FF00) | ((uint32_t)p_com_cmd->pData[26] & 0x000000FF));
                    gstMeterInfo.s_phase_meter_curr = _HexFloatCurrToInt(current, 1, _HexFloatActPowerDirection(power));
                    gstMeterInfo.s_phase_meter_pwr = _HexFloatCurrToInt(power, 1, 1);
//                    printf("current:0x%04X%04X\r\n", (uint16_t)(current >> 16), (uint16_t)current);
//                    gstMeterInfo.s_phase_meter_curr = _HexFloatCurrToInt(current, 0, _HexFloatActPowerDirection(power));
//                    printf("s_meter_curr:0x%04X\r\n", gstUdpPcsPara.udp_pcs_upload.s_phase_meter_curr);
                    /*T相*/
                    current = ((((uint32_t)p_com_cmd->pData[11] << 24) & 0xFF000000) | (((uint32_t)p_com_cmd->pData[12] << 16) & 0x00FF0000) | (((uint32_t)p_com_cmd->pData[13] << 8) & 0x0000FF00) | ((uint32_t)p_com_cmd->pData[14] & 0x000000FF));
                    power = ((((uint32_t)p_com_cmd->pData[27] << 24) & 0xFF000000) | (((uint32_t)p_com_cmd->pData[28] << 16) & 0x00FF0000) | (((uint32_t)p_com_cmd->pData[29] << 8) & 0x0000FF00) | ((uint32_t)p_com_cmd->pData[30] & 0x000000FF));
                    gstMeterInfo.t_phase_meter_curr = _HexFloatCurrToInt(current, 1, _HexFloatActPowerDirection(power));
//                    gstMeterInfo.t_phase_meter_pwr = _HexFloatCurrToInt(power, 1, 1);
//                    printf("current:0x%04X%04X\r\n", (uint16_t)(current >> 16), (uint16_t)current);
                    gstMeterInfo.t_phase_meter_curr = _HexFloatCurrToInt(current, 0, _HexFloatActPowerDirection(power));
//                    printf("t_meter_curr:0x%04X\r\n", gstUdpPcsPara.udp_pcs_upload.t_phase_meter_curr);
                }
                _gstModbusAckStatus.read_memter_current = 0;
            }
            else if(p_com_cmd->sci_index == EN_COMSERVICE_COM_INDEX2)
            {
                MemcpyU8toU16(garrReadDealFram[EN_COPRO_READ_HOLD_EVENT].pdata, (uint16_t*)&p_com_cmd->pData[3], garrReadDealFram[EN_COPRO_READ_HOLD_EVENT].reg_num);
                _ModbusAckSuccessSet(EN_COPRO_READ_HOLD_EVENT);
            }
            break;
        }
        case WRITE_HOLD_TYPE:
        {
            if(p_com_cmd->sci_index == EN_COMSERVICE_COM_INDEX0)
            {
                _gstModbusAckStatus.write_arc_hold = 0;
//                printf("write hold  ok\r\n");
            }
            else if(p_com_cmd->sci_index == EN_COMSERVICE_COM_INDEX1)
            {
                //TODO
            }
            else if(p_com_cmd->sci_index == EN_COMSERVICE_COM_INDEX2)
            {
                _gstModbusAckStatus.write_copro_hold = 0;    
            }        
            break;
        }
        default:
            break;
    }
}

bool SetArcSysReset(void)
{
    static uint16_t run_state = 0;
    static uint32_t timeout = 0;

    if(run_state == 0)
    {
        /*清除Ack标志*/
        _gstModbusAckStatus.write_arc_hold = 0xFF;
        
        gstModbusWriteFrame.func_code = WRITE_HOLD_TYPE;
        gstModbusWriteFrame.reg_addr = 0x113;
        gstModbusWriteFrame.reg_num = 1;
        gstModbusWriteFrame.slave_addr = MODBUS_SLAVE0_ADDR;
        gstModbusWriteFrame.data[0] = gstModBusPara.arc_para.arc_hold_para.sys_upgrade_status | (0x01 << 1);
        if(QueuePut(&gstModbusSendQueue, &gstModbusWriteFrame) == false)
        {
            return false;
        }
        timeout = TimGetSystime();
        run_state = 1;
    }
    else if(run_state == 1)
    {
        if(_gstModbusAckStatus.write_arc_hold == 0)
        {
//            printf("Set Arc Reset ok!\r\n");
            run_state = 0;
            return true;
        }

        if(TimDifftimeFromNow(timeout) > 200)
        {
      //      printf("Set Arc Reset Timeout!\r\n");
            run_state = 0;
            return false;
        }
    }    
    return false;
}

bool SetCoproSysReset(void)
{

    gstCoproReg.copro_hold_para.sys_upgrade_status = gstCoproReg.copro_hold_para.sys_upgrade_status | (0x01 << 1);

    if ((gstModBusPara.copro_para.copro_hold_para.sys_upgrade_status >> 1) & 0x01 == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
    

//     static uint16_t cop_run_state = 0;
//     static uint32_t cop_timeout = 0;

//     if(cop_run_state == 0)
//     {
//         /*清除Ack标志*/
//         _gstModbusAckStatus.write_copro_hold = 0xFF;
        
//         gstModbusWriteFrame.e_sci_index = EN_COMSERVICE_COM_INDEX2;
//         gstModbusWriteFrame.func_code = WRITE_HOLD_TYPE;
//         gstModbusWriteFrame.reg_addr = 0x64D0;
//         gstModbusWriteFrame.reg_num = 1;
//         gstModbusWriteFrame.slave_addr = MODBUS_SLAVE6_ADDR;
//         gstModbusWriteFrame.data_num = 2;
//         gstModbusWriteFrame.data[0] = gstModBusPara.copro_para.copro_hold_para.SysUpgradeStatus | (0x01 << 1);
//         if(QueuePut(&gstModbusSendQueue, &gstModbusWriteFrame) == false)
//         {
//             return false;
//         }
//         cop_timeout = TimGetSystime();
//         cop_run_state = 1;
//     }
//     else if(cop_run_state == 1)
//     {
//         if(_gstModbusAckStatus.write_copro_hold == 0)
//         {
// //            printf("Set Copro Reset ok!\r\n");
//             cop_run_state = 0;
//             return true;
//         }

//         if(TimDifftimeFromNow(cop_timeout) > 500)
//         {
// //            printf("Set Copro Reset Timeout!\r\n");
//             QueueClear(&gstModbusSendQueue);
//             cop_run_state = 0;
//             return false;
//         }
//     }    
//     return false;
}

/************************Slave*********************************/

void ModbusRTUSend(EN_COMSERVICE_INDEX index, uint8_t *p, uint16_t size)
{
    uint16_t crc;

    crc = CrcModbus(p, size);
    _ModbusAckData[size] = crc & 0xFF;
    _ModbusAckData[size + 1] = (crc >> 8) & 0xFF;

    PrivateDataSend(index, p, size + 2);
//    SciaSend(_ModbusAckData, size + 2);
}

static void _ModbusReadInput(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint16_t reg_type = 0;

    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = slave_addr;

    /*做数据正确性判断*/
    if(reg_start_addr < MODBUS_INPUT_REG_START_ADDR || reg_start_addr > MODBUS_INPUT_REG_END_ADDR)
    {
        if(reg_start_addr < MODBUS_INPUT_REG_START_ADDR_TEST || reg_start_addr > MODBUS_INPUT_REG_END_ADDR_TEST)
        {
            error = MOD_REG_ADDR_ERROR;
        }
        reg_type = 1;
    }
    /*
     *  error判断，根据业务添加代码
     */
//    ModBusReadRegUpdate();
    if(error == 0)
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x04;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        if(reg_type)
        {
//            MemcpyU16toU8SwapByte(&_ModbusAckData[offset + 3], (uint16_t*)&ModbusReadRegTest + (reg_start_addr - MODBUS_INPUT_REG_START_ADDR_TEST), reg_num * 2);
        }
        else
        {
            MemcpyU16toU8SwapByte(&_ModbusAckData[offset + 3], (uint16_t*)&gstUdpPcsPara.udp_pcs_upload + (reg_start_addr - MODBUS_INPUT_REG_START_ADDR), reg_num * 2);
        }
        _ModbusAckSize = reg_num * 2 + 3 + offset;
    }
    else
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x84;
        /*错误码*/
        _ModbusAckData[offset + 2] = error;
        _ModbusAckSize = 3 + offset;
    }

}

/**
* @brief: _ModbusReadRemain
* @param: modbus_type
* @param: reg_start_addr
* @param: reg_num
* @param: slave_addr
* @param: error_code
* @return:
* @note:
*
*/
static void _ModbusReadRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint16_t reg_type = 0;

    error = error_code;
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = slave_addr;


    /*做数据正确性判断*/
    if(reg_start_addr < MODBUS_HOLD_REG_START_ADDR || reg_start_addr > MODBUS_HOLD_REG_END_ADDR)
    {
        if(reg_start_addr < MODBUS_HOLD_REG_START_ADDR_TEST || reg_start_addr > MODBUS_HOLD_REG_END_ADDR_TEST)
        {
            error = MOD_REG_ADDR_ERROR;
        }
        reg_type = 1;
    }
    /*
     *  error判断，根据业务添加代码
     */

//    ModBusReadRegUpdate();
    if(error == 0)
    {

        /*功能码*/
        _ModbusAckData[offset + 1] = 0x03;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        if(reg_type)
        {
//            MemcpyU16toU8SwapByte(&_ModbusAckData[offset + 3], (uint16_t*)&ModbusRegTest + (reg_start_addr - MODBUS_HOLD_REG_START_ADDR_TEST), reg_num * 2);
        }
        else
        {
            MemcpyU16toU8SwapByte(&_ModbusAckData[offset + 3], (uint16_t*)&gstUdpPcsPara.udp_pcs_hold + (reg_start_addr - MODBUS_HOLD_REG_START_ADDR), reg_num * 2);
        }

        _ModbusAckSize = reg_num * 2 + 3 + offset;
    }
    else
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x83;
        /*错误码*/
        _ModbusAckData[offset + 2] = error;
        _ModbusAckSize = 3 + offset;
    }

}

/**
* @brief: _ModbusWriteSingleRemain
* @param: modbus_type
* @param: reg_start_addr
* @param: reg_data
* @param: slave_addr
* @return:
* @note:
*
*/
static void _ModbusWriteSingleRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_data, uint8_t slave_addr)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint16_t reg_type = 0;

    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }

    /*地址*/
    _ModbusAckData[offset] = slave_addr;
    /*做数据正确性判断*/
    if(reg_start_addr < MODBUS_HOLD_REG_START_ADDR || reg_start_addr > MODBUS_HOLD_REG_END_ADDR)
    {
        if(reg_start_addr < MODBUS_HOLD_REG_START_ADDR_TEST || reg_start_addr > MODBUS_HOLD_REG_END_ADDR_TEST)

        {
            error = MOD_REG_ADDR_ERROR;
        }
        reg_type = 1;
    }
    /*
     *  error判断，根据业务添加代码
     */
    /*如Error为0执行写操作的任务*/
    if(error == 0)
    {
        if(reg_type)
        {
//            ((uint16_t*)&ModbusRegTest)[reg_start_addr - MODBUS_HOLD_REG_START_ADDR_TEST] = reg_data;
        }
        else
        {
            ((uint16_t*)&gstUdpPcsPara.udp_pcs_hold)[reg_start_addr - MODBUS_HOLD_REG_START_ADDR] = reg_data;
        }



        /*功能码*/
        _ModbusAckData[offset + 1] = 0x06;
        /*寄存器地址*/
        _ModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;
        _ModbusAckData[offset + 3] = reg_start_addr & 0xFF;
        _ModbusAckData[offset + 4] = (reg_data >> 8) & 0xFF;
        _ModbusAckData[offset + 5] = reg_data & 0xFF;
        _ModbusAckSize = 4 + 2 + offset;
    }
    else
    {
        /*功能码*/
        _ModbusAckData[1] = 0x86;
        /*错误码*/
        _ModbusAckData[2] = error;
        _ModbusAckSize = 3 + offset;
    }
}

/**
* @brief: _ModbusWriteMultiRemain
* @param: modbus_type
* @param: reg_start_addr
* @param: p_reg_data
* @param: reg_num
* @param: slave_addr
* @param: error_code
* @return:
* @note:
*
*/
static void _ModbusWriteMultiRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint8_t *p_reg_data, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint8_t i = 0;
    uint16_t value = 0;
    uint16_t reg_type = 0;

    error = error_code;
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = slave_addr;

    /*做数据正确性判断*/
    if(reg_start_addr < MODBUS_HOLD_REG_START_ADDR || reg_start_addr > MODBUS_HOLD_REG_END_ADDR)
    {
        if(reg_start_addr < MODBUS_HOLD_REG_START_ADDR_TEST || reg_start_addr > MODBUS_HOLD_REG_END_ADDR_TEST)

        {
            error = MOD_REG_ADDR_ERROR;
        }
        reg_type = 1;
    }
    /*
     *  error判断，根据业务添加代码
     */
    /*如Error为0执行写操作的任务*/

    if(error == 0)
    {
        /* 写寄存器操作 */
        for (i = 0; i < reg_num; i++)
        {
            value = p_reg_data[i * 2 + 1] + (p_reg_data[i * 2] << 8);

            if(reg_type)
            {
//                ((uint16_t*)&ModbusRegTest)[reg_start_addr + i - MODBUS_HOLD_REG_START_ADDR_TEST] = value;
            }
            else
            {
                ((uint16_t*)&gstUdpPcsPara.udp_pcs_hold)[reg_start_addr + i - MODBUS_HOLD_REG_START_ADDR] = value;
            }
        }

        /*功能码*/
        _ModbusAckData[offset + 1] = 0x10;
        /*寄存器地址*/
        _ModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;
        _ModbusAckData[offset + 3] = reg_start_addr & 0xFF;
        _ModbusAckData[offset + 4] = (reg_num >> 8) & 0xFF;
        _ModbusAckData[offset + 5] = reg_num & 0xFF;
        _ModbusAckSize = 4 + 2 + offset;

    }
    else
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x90;
        /*错误码*/
        _ModbusAckData[offset + 2] = error;
        _ModbusAckSize = 3 + offset;
    }
}

/**
* @brief: ModbusCmdProcess
* @param:
* @note: 轮询执行
*
*/
uint8_t ModbusCmdAnalysis(SciRevData_t *p_com_cmd)
{
    uint8_t ret = 0;
    static uint8_t func_code;
    static EN_MODBUS_TYPE   modbus_type = EN_MODBUS_RTU;
    static uint8_t   slave_addr = 0;
    uint8_t error_code = 0;
    static uint32_t mod_data_send_wait_time = 0;

    switch(_eSlaveModSta)
    {
        case EN_MODBUS_REV_STATUS_IDLE:
        {
            if(p_com_cmd->data_type == TYPE_MODBUS_SLAVE_DATA)
            {
                func_code = p_com_cmd->pData[1];
                modbus_type = EN_MODBUS_RTU;
                slave_addr = p_com_cmd->slave_addr;
                _eSlaveModSta = EN_MODBUS_REV_STATUS_ANALYSIS;
            }
            break;
        }
        case EN_MODBUS_REV_STATUS_ANALYSIS:
        {
            switch(func_code)
            {
                case EN_MODBUS_FUNC_CODE_READ_INPUT:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_num;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_com_cmd->pData[2] << 8) | p_com_cmd->pData[3];
                        reg_num = (p_com_cmd->pData[4] << 8) | p_com_cmd->pData[5];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusReadInput(modbus_type, reg_start_addr, reg_num, slave_addr);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_READ_REMAIN:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_num;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_com_cmd->pData[2] << 8) | p_com_cmd->pData[3];
                        reg_num = (p_com_cmd->pData[4] << 8) | p_com_cmd->pData[5];

                        if((p_com_cmd->data_size != 8) || (reg_num > 128))
                        {
                            error_code = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                        }
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusReadRemain(modbus_type, reg_start_addr, reg_num, slave_addr, error_code);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_WRITE_SINGLE:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_data;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_com_cmd->pData[2] << 8) | p_com_cmd->pData[3];
                        reg_data = (p_com_cmd->pData[4] << 8) | p_com_cmd->pData[5];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusWriteSingleRemain(modbus_type, reg_start_addr, reg_data, slave_addr);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_WRITE_MULTI:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_num;
                    uint8_t *p_reg_data = NULL;
                    uint8_t byte_num;

                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_com_cmd->pData[2] << 8) | p_com_cmd->pData[3];
                        reg_num = (p_com_cmd->pData[4] << 8) | p_com_cmd->pData[5];
                        p_reg_data = &p_com_cmd->pData[7];
                        byte_num = p_com_cmd->pData[6];
                        if((p_com_cmd->data_size < 11) || (byte_num != (2 * reg_num)))
                        {
                            error_code = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                        }
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusWriteMultiRemain(modbus_type, reg_start_addr, p_reg_data, reg_num, slave_addr, error_code);
                    _eSlaveModSta = EN_MODBUS_REV_STATUS_ACK;
                    break;
                }
                default:
                {
                    uint8_t offset;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        offset = 0;
                    }
                    else
                    {
                        offset = 6;
                    }
                    _ModbusAckData[offset] = slave_addr;
                    _ModbusAckData[offset + 1] = func_code | 0x80;
                    _ModbusAckData[offset + 2] = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                    _ModbusAckSize = 3 + offset;
                    break;
                }
            }
            mod_data_send_wait_time = TimGetSystime();
            _eSlaveModSta = EN_MODBUS_REV_STATUS_ACK_SEND_WAIT;
//            _eSlaveModSta = EN_MODBUS_REV_STATUS_ACK;
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK_SEND_WAIT:
        {
            _eSlaveModSta = EN_MODBUS_REV_STATUS_ACK;
            if(TimDifftimeFromNow(mod_data_send_wait_time) > 100)
            {
                _eSlaveModSta = EN_MODBUS_REV_STATUS_ACK;
            }
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK:
        {
            if(modbus_type == EN_MODBUS_RTU)
            {
                ModbusRTUSend(p_com_cmd->sci_index ,_ModbusAckData, _ModbusAckSize);
                _eSlaveModSta = EN_MODBUS_REV_STATUS_ACK_WAIT;
            }
            else
            {
                /*后期增加MODBUS_TCP*/
            }
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK_WAIT:
        {
            if(modbus_type == EN_MODBUS_RTU)
            {
                _eSlaveModSta = EN_MODBUS_REV_STATUS_IDLE;  
                /* 解析回包完成     */
                ret = 1;
            }
            else
            {
                /*后期增加MODBUS_TCP,可不用等待直接跳入IDLE*/
            }
            break;
        }
        default:
            break;
    }

    return ret;
}

int16_t GetMeterRCurInfo()
{
    return gstMeterInfo.r_phase_meter_curr;
}
int16_t GetMeterSCurInfo()
{
    return gstMeterInfo.s_phase_meter_curr;
}
int16_t GetMeterTCurInfo()
{
    return gstMeterInfo.t_phase_meter_curr;
}

int32_t GetMeterPowerInfo()
{
    return gstMeterInfo.r_phase_meter_pwr + gstMeterInfo.s_phase_meter_pwr + gstMeterInfo.t_phase_meter_pwr;
}

uint16_t GetMeterConnectState()
{
    return gstMeterInfo.reset_sci_count;
}



