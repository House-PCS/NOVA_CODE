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

#include "comm_analysis.h"
SlaveErrorPara_t   gErrorSlavePara;
#define BITCHECK(data,number)           ((data >> number) & (0x01))
void SlaveCommProcess()
{
    static  uint32_t timeout = 0;
    static UdpRevData_t *p_udp_cmd;
    static PrivateBmsWrite_t *pSlaveWrite;
    static SciRevData_t *p_rtu_cmd;
    static EN_HOST_POLLING_STATUS   host_polling_status = EN_HOST_POLLING_STATUS_IDLE;
    static EN_BMS_READ_WRITE_STATE  read_write_state_type = EN_READ_DCDC_INFO;
    ComRevData_t        com_revdata ;
    FpgaUdpRevData_t    udp_revdata ;
    memset((uint8_t *)&com_revdata, 0, sizeof(ComRevData_t));
    memset((uint8_t *)&udp_revdata, 0, sizeof(FpgaUdpRevData_t));
    /*接收udp协议*/
    if(QueueGet(&gUdpSlaveCommSendQueue, (void *)&p_udp_cmd) == true)
    {
        udp_revdata.pData = p_udp_cmd->pData;
        udp_revdata.data_size = p_udp_cmd->data_size;
        UdpProtopackJudge(&udp_revdata);
    }
    switch(host_polling_status)
    {
        case EN_HOST_POLLING_STATUS_IDLE:
        {
            /*空闲时段也要执行Get gstSciRevQueue, 并舍弃，防止队列满导致无法再接收数据*/
            QueueGet(&gstSciRevQueue, (void *)&p_rtu_cmd);
            if(QueueGet(&gBmsSendQueue, (void *)&pSlaveWrite) == true)/*如果有主动写操作*/
            {
                WriteBmsData(pSlaveWrite->reg_addr, pSlaveWrite->reg_num, pSlaveWrite->p);
                host_polling_status = EN_HOST_POLLING_STATUS_WAITING_BMS_REV;
                timeout = TimGetSystime();
            }
            else
            {
                if(TimDifftimeFromNow(timeout) > HOST_COMM_POLLING_TIME)
                {
                    /*轮询回读*/
                    switch(read_write_state_type)
                    {
                        case EN_READ_DCDC_INFO:
                        {
                            ReadDCDCReg();
                            timeout = TimGetSystime();
                            host_polling_status = EN_HOST_POLLING_STATUS_IDLE;
                            break;
                        }
                        case EN_READ_WRITE_BMS_INFO:
                        {
                            if((gBMSFirmwareUpgrateInfo.bms_upgrating == 0) && (BITCHECK(gstDCDCReg->dc_dc_hold_para.updata_status, 0) == 0))//非升级
                            {
                                ReadBMSReg();
                                timeout = TimGetSystime();
                                host_polling_status = EN_HOST_POLLING_STATUS_WAITING_BMS_REV;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    read_write_state_type++;
                    read_write_state_type %= EN_READ_EVENT_NUM;
                }
            }
            break;
        }
		
        case EN_HOST_POLLING_STATUS_WAITING_BMS_REV:
        {
            if(TimDifftimeFromNow(timeout) > HOST_COMM_REV_MAX_TIME)
            {
                host_polling_status = EN_HOST_POLLING_STATUS_REV_BMS_OVERTIME;
            }

            else if(QueueGet(&gstSciRevQueue, (void *)&p_rtu_cmd) == true)
            {
                if(p_rtu_cmd->data_type == TYPE_MODBUS_DATA)
                {
                    /*Modbus协议*/
//                    ModbusCommProcess();
                }
                else if(p_rtu_cmd->data_type == TYPE_COM_RRESPONSE_DATA)
                {
                    /*私有协议*/
                    host_polling_status = EN_HOSTE_POLLING_STATUS_REV_OK;
                    timeout = TimGetSystime();
                }
                else if(p_rtu_cmd->data_type == TYPE_COM_WRESPONSE_DATA)
                {
                    host_polling_status = EN_HOSTE_POLLING_STATUS_REV_OK;
                    return;
                }
                else if(p_rtu_cmd->data_type == TYPE_COM_RREQUEST_DATA)
                {

                    return;
                }
                else if(p_rtu_cmd->data_type == TYPE_COM_WREQUEST_DATA)
                {

                    return;
                }
            }
            break;
        }

        case EN_HOST_POLLING_STATUS_REV_BMS_OVERTIME:
        {
            /*做传输超时的处理*/
            timeout = TimGetSystime();
            if(++gErrorSlavePara.bms_com_time_out >= 3)               //连续3次读取不到 则表示超时
            {
                gErrorSlavePara.dcdc_com_time_out = 0;
                memset((uint8_t*)&gSlaveBMSPara.bms_only_read_para, 0, sizeof(BMSOnlyReadPara_t));
                memset((uint8_t*)&gSlaveBMSPara.bms_hold_para, 0, sizeof(BMSHoldPara_t));
				memcpy(gstBmsReg, (uint8_t*)&gSlaveBMSPara, sizeof(BMSPara_t));				
                host_polling_status = EN_HOST_POLLING_STATUS_IDLE;
            }
            else
            {
                host_polling_status = EN_HOST_POLLING_STATUS_WAITING_BMS_REV;
            }
            break;
        }

        case EN_HOST_POLLING_STATUS_REV_DCDC_OVERTIME:
        {
            /*做传输超时的处理*/
            host_polling_status = EN_HOST_POLLING_STATUS_IDLE;
            timeout  = TimGetSystime();
            if(++gErrorSlavePara.dcdc_com_time_out >= 3)               //连续3次读取不到 则表示超时
            {
                gErrorSlavePara.dcdc_com_time_out = 0;
                memset((uint8_t*)&gTempSlaveDCDCPara.dc_dc_only_read_para, 0, sizeof(DCDCOnlyReadPara_t));
                memset((uint8_t*)&gTempSlaveDCDCPara.dc_dc_hold_para, 0, sizeof(DCDCHoldPara_t));
            }
            break;
        }

        case EN_HOSTE_POLLING_STATUS_REV_OK:
        {
            /*更新数据*/
            com_revdata.data_size = p_rtu_cmd->data_size;
            com_revdata.pData = p_rtu_cmd->pData;
            ProtoUnpackJudge(&com_revdata);
            timeout = TimGetSystime();
            host_polling_status = EN_HOST_POLLING_STATUS_IDLE;
            break;
        }

        case EN_HOST_POLLING_STATUS_REV_ERROR:
        {
            /*错误信息处理*/
            timeout = TimGetSystime();
            host_polling_status = EN_HOST_POLLING_STATUS_IDLE;
            break;
        }
        default:
            break;
    }
}

