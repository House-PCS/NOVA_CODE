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


#include "private_analysis.h"
#include "firmware_upgrate.h"

#define PRIVATE_SEND_BUF_SIZE 1100

WiteResAckStatus_t  garrWriteResAckState[EN_COMSERVICE_INDEX_NUM];
static protocal_request_packet_t     _RequestPacketData;
static protocal_response_packet_t    _ResponsePacketData;
ProtocalReadResponsePacket_t  gstReadResponsePacketData;


static uint16_t  _ProtocolBuf[2100];
uint16_t garrComTxBuf[2100]; 
static uint16_t _gPrivateUpgradeFlag = 0;

/**
* @brief: PrivateParamInit
* @param: void                        
* @note: 
*
*/
void PrivateParamInit(void)
{
    memset((uint16_t*)&garrWriteResAckState, 0xFF, (int)sizeof(WiteResAckStatus_t));
}
/**
* @brief: PrivateDataSend
* @param: sci_index                   
* @param: data                        
* @param: len                         
* @note: 
*
*/
void PrivateDataSend(EN_COMSERVICE_INDEX sci_index, uint8_t* data, uint16_t len)
{
    if(sci_index == EN_COMSERVICE_COM_INDEX0)
    {
        ScibSend(data, len);
    }
    else if(sci_index == EN_COMSERVICE_COM_INDEX1)
    {
        ScicSend(data, len);
    }
}

/**
* @brief: WriteResponseAnalysis
* @param: p_com_cmd                   
* @note: 
*
*/
void WriteResponseAnalysis(SciRevData_t *p_com_cmd)
{
    uint16_t checksum;
    uint16_t cmd;

    checksum = ((uint16_t)p_com_cmd->pData[p_com_cmd->data_size - 1] << 8) | p_com_cmd->pData[p_com_cmd->data_size - 2];
    /*校验不通过，不解析*/
    if(CheckSum(p_com_cmd->pData, p_com_cmd->data_size - 2) != checksum)
    {
        return;
    }

    cmd = ((uint16_t)p_com_cmd->pData[4] << 8) | p_com_cmd->pData[3];
    switch(cmd)
    {
        case CMD_UPGRATE_START:
        {
            garrWriteResAckState[p_com_cmd->sci_index].cmd_upgrate_start = p_com_cmd->pData[5];
            break;
        }
        case CMD_UPGRATE_BUSY:
        {
            garrWriteResAckState[p_com_cmd->sci_index].cmd_upgrate_busy = p_com_cmd->pData[5];
            break;
        }
        case CMD_UPGRATE_END:
        {
            garrWriteResAckState[p_com_cmd->sci_index].cmd_upgrate_end = p_com_cmd->pData[5];
            break;
        }  
        default:
            break;
    }
        
}

void ResetWriteResAckState(EN_COMSERVICE_INDEX sci_index, uint16_t cmd)
{
    switch(cmd)
    {
        case CMD_UPGRATE_START:
        {
            garrWriteResAckState[sci_index].cmd_upgrate_start = 0xFF;
            break;
        }
        case CMD_UPGRATE_BUSY:
        {
            garrWriteResAckState[sci_index].cmd_upgrate_busy = 0xFF;
            break;
        }
        case CMD_UPGRATE_END:
        {
            garrWriteResAckState[sci_index].cmd_upgrate_end = 0xFF;
            break;
        }  
        default:
            break;
    }
}

/**
* @brief: DataSendPolling
* @param: read_responsed              
* @note: 
*
*/
void DataSendPolling(SciRevData_t *p_com_cmd)
{
    uint16_t check_sum = 0;
    uint16_t data_filed_len = 0;

    if(p_com_cmd == NULL || p_com_cmd->pData == NULL)
    {
        return;
    }

    /* 组包 */
    _ResponsePacketData.packet_head = 0xA5;
    _ResponsePacketData.send_addr   = _RequestPacketData.recv_addr;
    _ResponsePacketData.recv_addr   = _RequestPacketData.send_addr;
    _ResponsePacketData.cmd_low     = _RequestPacketData.cmd_low;
    _ResponsePacketData.cmd_high    = _RequestPacketData.cmd_high;
    if(p_com_cmd->data_type == TYPE_COM_RREQUEST_DATA)
    {
        memcpy((uint16_t*)&gstReadResponsePacketData, (uint16_t*)&_ResponsePacketData, 5);
        memcpy(garrComTxBuf, (uint16_t*)&gstReadResponsePacketData, sizeof(ProtocalReadResponsePacket_t));

        if(gstReadResponsePacketData.error_code == ACK_ERROR)
        {
            /* ack error data_len 后跟 check_sum */
            check_sum = CheckSumFun((uint8_t*)&garrComTxBuf, sizeof(ProtocalReadResponsePacket_t), 0);
            *(garrComTxBuf + sizeof(ProtocalReadResponsePacket_t)) = check_sum & 0x00FF;
            *(garrComTxBuf + sizeof(ProtocalReadResponsePacket_t) + 1) = (check_sum >> 8);
        }
        else
        {
            data_filed_len = (gstReadResponsePacketData.data_field_len_low + \
                              gstReadResponsePacketData.data_field_len_high << 8);
            /* ack ok data_len 后跟 data + check_sum */
            check_sum = CheckSumFun((uint8_t*)&garrComTxBuf, \
                                    (data_filed_len + sizeof(ProtocalReadResponsePacket_t)), 0);
            *(garrComTxBuf + sizeof(ProtocalReadResponsePacket_t) + data_filed_len) = check_sum & 0x00FF;
            *(garrComTxBuf + sizeof(ProtocalReadResponsePacket_t) + data_filed_len + 1) = (check_sum >> 8);
        }

        PrivateDataSend(p_com_cmd->sci_index, (uint8_t*)&garrComTxBuf, (data_filed_len + sizeof(ProtocalReadResponsePacket_t) + 2));
    }
    else if(p_com_cmd->data_type == TYPE_COM_WREQUEST_DATA)
    {
        check_sum = CheckSumFun((uint8_t*)&_ResponsePacketData, sizeof(_ResponsePacketData) - 2, 0);
        _ResponsePacketData.check_sum_low = check_sum & 0x00FF;
        _ResponsePacketData.check_sum_high = (check_sum >> 8);
        PrivateDataSend(p_com_cmd->sci_index, (uint8_t*)&_ResponsePacketData, sizeof(protocal_response_packet_t));
    }

    return;
}

/**
* @brief: _CmdAnalysis
* @param: p_request_packet            
* @return: 
* @note: 
*
*/
static bool _CmdAnalysis(protocal_request_packet_t *p_request_packet)
{
    uint16_t cmd = p_request_packet->cmd_low + (p_request_packet->cmd_high << 8);
    static uint32_t pcs_firmware_addr;
    static uint32_t pcs_firmware_checksum;
    static uint32_t old_update_index ;
    uint32_t  data_field_len;
    uint16_t update_flag = 0;
    static uint16_t special_data_len = 0;
    uint32_t updata_index = 0;
    _ResponsePacketData.error_code = ACK_OK;
    gstReadResponsePacketData.error_code = ACK_OK;
    static uint32_t recv_firmware_len;
    static uint32_t recv_firmware_checksum = 0xFFFFFFFF;

    switch(cmd)
    {
        case UPGRADE_START:
        {
            _gPrivateUpgradeFlag = 1;
            /* 初始化程序存放地址 */
            pcs_firmware_addr = DSP_FIRMWARE_ADDR;
            pcs_firmware_checksum = 0;
            old_update_index = 0xFF;
            /* 取出特殊字段长度 */
            if(p_request_packet->data_field_len_low == 0x05)
            {
                special_data_len = p_request_packet->data_field[4];
            }
            else
            {
                special_data_len = 0;
            }
            break;
        }
        case UPGRADE_BUSY:
        {
            _gPrivateUpgradeFlag = 1;
            if(special_data_len == 2)
            {
                updata_index = (p_request_packet->data_field[0] & 0x00FF) + (p_request_packet->data_field[1] << 8);
            }
            if(updata_index == old_update_index)
            {
                update_flag = 0;
            }
            else
            {
                update_flag = 1;
                old_update_index = updata_index;
            }
            /* 第一包数据 */
            if((pcs_firmware_addr == DSP_FIRMWARE_ADDR) && (updata_index == 0))
            {
                if(memcmp(&p_request_packet->data_field[special_data_len], garrDspFirmwareType, 4) == 0)
                {
                    pcs_firmware_addr = DSP_FIRMWARE_ADDR;
                }
                else
                {
                    _ResponsePacketData.error_code = ACK_ERROR;
                    return false;
                }
                /* 固件版本 */
//                memcpy(gPCSHardVersion, &p_request_packet->data_field[4 + special_data_len], 3);
                /* 固件长度 */
                recv_firmware_len = (uint32_t)(*(p_request_packet->data_field + 7 + special_data_len)) + (((uint32_t)(*(p_request_packet->data_field + 8 + special_data_len))) << 8) + \
                                  (((uint32_t)(*(p_request_packet->data_field + 9 + special_data_len))) << 16) + (((uint32_t)(*(p_request_packet->data_field + 10 + special_data_len))) << 24);
                /* 固件校验和 */
                recv_firmware_checksum = (uint32_t)(*(p_request_packet->data_field + 252 + special_data_len)) + (((uint32_t)(*(p_request_packet->data_field + 253 + special_data_len))) << 8) + \
                                          (((uint32_t)(*(p_request_packet->data_field + 254 + special_data_len))) << 16) + (((uint32_t)(*(p_request_packet->data_field + 255 + special_data_len))) << 24);
            }

            data_field_len = (p_request_packet->data_field_len_low & 0x00FF) + (p_request_packet->data_field_len_high << 8) - special_data_len;
            SpiFlashWriteBytesWithErase(pcs_firmware_addr, &p_request_packet->data_field[special_data_len], data_field_len);
            SpiFlashReadBytes(pcs_firmware_addr, _ProtocolBuf, data_field_len);

            if(pcs_firmware_addr == DSP_FIRMWARE_ADDR)
            {
                if(data_field_len >= 512)
                {
                    /* 计算 checksum */
                    pcs_firmware_checksum += CheckSumFun((uint8_t*)&_ProtocolBuf[256], (data_field_len - 256), 1);
                }
            }
            else
            {
                /* 计算 checksum */
//                pcs_firmware_checksum += CheckSumFun(_ProtocolBuf, data_field_len, 1);
                if(update_flag)
                {
                    pcs_firmware_checksum += CheckSumFun((uint8_t*)&_ProtocolBuf, data_field_len, 1);
                }
            }
            if(update_flag)
            {
                pcs_firmware_addr +=  data_field_len;
            }
            break;
        }
        case UPGRADE_END:
        {
            if(pcs_firmware_checksum != recv_firmware_checksum)
            {
                _ResponsePacketData.error_code = ACK_ERROR;
            }
            else
            {
                uint16_t firmware_upgrade[256];
                memset(firmware_upgrade, 0xA0, 256);
                SpiFlashWriteBytesWithErase(DSP_FIRMWARE_FRAME_ADDR, firmware_upgrade, 256);
//                memset(firmware_upgrade, 0x00, 256);
//                SpiFlashReadBytes(UPGRADE_FLAG_ADDR, firmware_upgrade, 256);
            }
            _gPrivateUpgradeFlag = 0;
            break;
        }
        default:
            break;
    }

    return true;

}


bool WriteRequestAnalysis(SciRevData_t *p_com_cmd)
{
    uint16_t checksum   = 0;

    if(p_com_cmd == NULL || p_com_cmd->pData == NULL)
    {
        return false;
    }

    /* 解包 */
    memcpy((uint16_t*)&_RequestPacketData, p_com_cmd->pData, 7);
    _RequestPacketData.data_field = _ProtocolBuf;

    _RequestPacketData.check_sum_low = p_com_cmd->pData[p_com_cmd->data_size - 2];
    _RequestPacketData.check_sum_high = p_com_cmd->pData[p_com_cmd->data_size - 1];
    memcpy(_ProtocolBuf, (uint16_t*)(p_com_cmd->pData + 7), _RequestPacketData.data_field_len_low + (_RequestPacketData.data_field_len_high << 8));

    checksum = CheckSumFun(p_com_cmd->pData, (p_com_cmd->data_size - 2), 0);
    if((_RequestPacketData.check_sum_low + (_RequestPacketData.check_sum_high << 8)) != checksum)
    {
        _ResponsePacketData.error_code = ACK_ERROR;
    }
    else
    {
        _CmdAnalysis(&_RequestPacketData);
    }

    DataSendPolling(p_com_cmd);

    return true;
}

bool PrivateUpgradeRunning(void)
{
    static uint32_t upgrade_timeout = 0;
    static uint16_t sta = 0;

    if(sta == 0)
    {
        if(_gPrivateUpgradeFlag)
        {
            upgrade_timeout = TimGetSystime();
            _gPrivateUpgradeFlag = 0;
            sta = 1;
            return true;
        }
    }
    else if(sta == 1)
    {
        if(TimDifftimeFromNow(upgrade_timeout) > 5000)
        {
            sta = 0;
            if(_gPrivateUpgradeFlag)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        return true;
    }
    return false;
}
