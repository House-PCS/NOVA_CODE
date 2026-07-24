/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        comm_analysis.c
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
  * </table>
  ******************************************************************************
  */
#include "comm_analysis.h"
#include "modbus_analysis.h"

static protocal_request_packet_t     _RequestPacketData;
static protocal_response_packet_t    _ResponsePacketData;
ProtocalReadResponsePacket_t  gstReadResponsePacketData;

uint16_t garrComRxBuf[COM_RX_SIZE];
uint16_t garrComTxBuf[COM_TX_SIZE];
static uint16_t  _ProtocolBuf[COM_RX_SIZE];

uint16_t gPCSFirmwareType[4] = {0x0C, 0x0C, 0x0C, 0x01};
uint16_t gPCSHardVersion[3];
uint32_t gPCSHardwareLen;
uint32_t gPCSbmsFirmwareChecksum;
uint8_t gReadResponsed;
/**
* @brief CheckSumFun
* @param *pdata: data
* @param len: len
* @param mode: 0-checksum16     1-checksum32
* @return
* - checksum
* @note
*/
uint32_t CheckSumFun(uint16_t *pdata, uint16_t len, uint8_t mode)
{
    uint16_t checksum16 = 0;
    uint32_t checksum32 = 0;
    uint16_t i = 0;

    if(mode)
    {
        for(i = 0; i < len; i ++)
        {
            checksum32 += pdata[i];
        }

        return checksum32;
    }
    else
    {
        for(i = 0; i < len; i ++)
        {
            checksum16 += pdata[i];
        }

        return checksum16;
    }
}

/**
* @brief: DataSendPolling
* @param: read_responsed              
* @note: 
*
*/
void DataSendPolling(uint8_t read_responsed)
{
    uint16_t check_sum = 0;
    uint16_t data_filed_len = 0;

    /* 组包 */
    _ResponsePacketData.packet_head = RESPONSE_PACKET_HEAD;
    _ResponsePacketData.send_addr   = _RequestPacketData.recv_addr;
    _ResponsePacketData.recv_addr   = _RequestPacketData.send_addr;
    _ResponsePacketData.cmd_low     = _RequestPacketData.cmd_low;
    _ResponsePacketData.cmd_high    = _RequestPacketData.cmd_high;
    if(read_responsed)
    {
        memcpy((uint16_t*)&gstReadResponsePacketData, (uint16_t*)&_ResponsePacketData, 5);
        memcpy(garrComTxBuf, (uint16_t*)&gstReadResponsePacketData, sizeof(ProtocalReadResponsePacket_t));

        if(gstReadResponsePacketData.error_code == ACK_ERROR)
        {
            /* ack error data_len 后跟 check_sum */
            check_sum = CheckSumFun(garrComTxBuf, sizeof(ProtocalReadResponsePacket_t), 0);
            *(garrComTxBuf + sizeof(ProtocalReadResponsePacket_t)) = check_sum & 0x00FF;
            *(garrComTxBuf + sizeof(ProtocalReadResponsePacket_t) + 1) = (check_sum >> 8);
        }
        else
        {
            data_filed_len = (gstReadResponsePacketData.data_field_len_low + \
                              gstReadResponsePacketData.data_field_len_high << 8);
            /* ack ok data_len 后跟 data + check_sum */
            check_sum = CheckSumFun(garrComTxBuf, \
                                    (data_filed_len + sizeof(ProtocalReadResponsePacket_t)), 0);
            *(garrComTxBuf + sizeof(ProtocalReadResponsePacket_t) + data_filed_len) = check_sum & 0x00FF;
            *(garrComTxBuf + sizeof(ProtocalReadResponsePacket_t) + data_filed_len + 1) = (check_sum >> 8);
        }

        SciaSend((uint8_t*)&garrComTxBuf, (data_filed_len + sizeof(ProtocalReadResponsePacket_t) + 2));
    }
    else
    {
        check_sum = CheckSumFun((uint16_t*)&_ResponsePacketData, sizeof(_ResponsePacketData) - 2, 0);
        _ResponsePacketData.check_sum_low = check_sum & 0x00FF;
        _ResponsePacketData.check_sum_high = (check_sum >> 8);
        SciaSend((uint8_t*)&_ResponsePacketData, sizeof(protocal_response_packet_t));
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
    uint32_t  data_field_len;
    static uint32_t old_update_index;
    uint16_t update_flag = 0;
    static uint16_t special_data_len = 0;
    uint32_t updata_index = 0;
    _ResponsePacketData.error_code = ACK_OK;
    gstReadResponsePacketData.error_code = ACK_OK;

    switch(cmd)
    {
        case UPGRADE_START:
        {
            /* 初始化程序存放地址 */
            pcs_firmware_addr = PCS_FIRMWARE_ADDR;
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
            if((pcs_firmware_addr == PCS_FIRMWARE_ADDR) && (updata_index == 0))
            {
                if(memcmp(&p_request_packet->data_field[special_data_len], gPCSFirmwareType, 4) == 0)
                {
                    pcs_firmware_addr = PCS_FIRMWARE_ADDR;
                }
                else
                {
                    _ResponsePacketData.error_code = ACK_ERROR;
                    return false;
                }
                /* 固件版本 */
                memcpy(gPCSHardVersion, &p_request_packet->data_field[4 + special_data_len], 3);
                /* 固件长度 */
//                gBMSHardwareLen = *((uint32_t*)&p_request_packet->data_field[7]);
                gPCSHardwareLen = (uint32_t)(*(p_request_packet->data_field + 7 + special_data_len)) + (((uint32_t)(*(p_request_packet->data_field + 8 + special_data_len))) << 8) + \
                                  (((uint32_t)(*(p_request_packet->data_field + 9 + special_data_len))) << 16) + (((uint32_t)(*(p_request_packet->data_field + 10 + special_data_len))) << 24);

                /* 固件校验和 */
//                gBMSbmsFirmwareChecksum = *((uint32_t*)&p_request_packet->data_field[252]);
                gPCSbmsFirmwareChecksum = (uint32_t)(*(p_request_packet->data_field + 252 + special_data_len)) + (((uint32_t)(*(p_request_packet->data_field + 253 + special_data_len))) << 8) + \
                                          (((uint32_t)(*(p_request_packet->data_field + 254 + special_data_len))) << 16) + (((uint32_t)(*(p_request_packet->data_field + 255 + special_data_len))) << 24);


            }

            data_field_len = (p_request_packet->data_field_len_low & 0x00FF) + (p_request_packet->data_field_len_high << 8) - special_data_len;
            SpiFlashWriteBytesWithErase(pcs_firmware_addr, &p_request_packet->data_field[special_data_len], data_field_len);
            SpiFlashReadBytes(pcs_firmware_addr, _ProtocolBuf, data_field_len);

            if(pcs_firmware_addr == PCS_FIRMWARE_ADDR)
            {
                if(data_field_len >= 512)
                {
                    /* 计算 checksum */
                    pcs_firmware_checksum += CheckSumFun(&_ProtocolBuf[256], (data_field_len - 256), 1);
                }
            }
            else
            {
                /* 计算 checksum */
//                pcs_firmware_checksum += CheckSumFun(_ProtocolBuf, data_field_len, 1);
                if(update_flag)
                {
                    pcs_firmware_checksum += CheckSumFun(_ProtocolBuf, data_field_len, 1);
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
            if(pcs_firmware_checksum != gPCSbmsFirmwareChecksum)
            {
                _ResponsePacketData.error_code = ACK_ERROR;
            }
            else
            {
                uint16_t arr_firmware_upgrade_state[256];
                memset(arr_firmware_upgrade_state, 0xA0, 256);
                SpiFlashWriteBytesWithErase(UPGRADE_FLAG_ADDR, arr_firmware_upgrade_state, 256);
            }
            break;
        }
        default:
            break;
    }

    return true;

}

/**
* @brief: ProtoUnpackJudge
* @param: p                           
* @return: 
* @note: 
*
*/
bool ProtoUnpackJudge(SciRevData_t *p)
{
    uint16_t checksum      = 0;

    if(p == NULL || p->pData == NULL)
    {
        return false;
    }

    /* 解包 */
    memcpy((uint16_t*)&_RequestPacketData, p->pData, 7);
    _RequestPacketData.data_field = _ProtocolBuf;

    _RequestPacketData.check_sum_low = p->pData[p->data_size - 2];
    _RequestPacketData.check_sum_high = p->pData[p->data_size - 1];
    memcpy(_ProtocolBuf, (uint16_t*)(p->pData + 7), _RequestPacketData.data_field_len_low + (_RequestPacketData.data_field_len_high << 8));

    checksum = CheckSumFun((uint16_t*)p->pData, (p->data_size - 2), 0);
    if((_RequestPacketData.check_sum_low + (_RequestPacketData.check_sum_high << 8)) != checksum)
    {
        _ResponsePacketData.error_code = ACK_ERROR;
    }
    else
    {
        if(!((_RequestPacketData.cmd_low + (_RequestPacketData.cmd_high << 8)) & 0x8000))
        {
            gReadResponsed = 1;
        }
        _CmdAnalysis(&_RequestPacketData);
    }

//  DataSendPolling(gReadResponsed);

    return true;
}


void SysRestart(void)
{
    EALLOW;

    WdRegs.WDCR.all = 0x00af;

    WdRegs.WDCR.bit.WDCHK = 2;
    EDIS;
}



void UpgradeProcesshandle(void)
{
    static uint32_t restart_time = 0;
    if(gUpgraderRestartFlag == 1)
    {
        if(TimDifftimeFromNow(restart_time) > 12000)
        {
            SysRestart();
        }
    }
    else
    {
        restart_time = TimGetSystime();
    }
}
