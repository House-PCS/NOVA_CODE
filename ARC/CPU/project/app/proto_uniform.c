#include "proto_uniform.h"
#include "bsp.h"
#include "modbus_analysis.h"

//static uint8_t _TxAckBuf[512];
uint8_t garrComRxBuf[COM_RX_SIZE];
uint8_t garrComTxBuf[COM_TX_SIZE];
static UniformProtocolHead_t _gCommHead;
static UniformProtocolHead_t _gAckHead;
uint8_t gARCFirmwareType[4] = {0x0C, 0x0C, 0x0C, 0x02};
uint8_t gARCHardVersion[3] = {0};
uint8_t gARCVersionExtend[12] = {0};
uint32_t gARCHardwareLen;
uint32_t gARCbmsFirmwareChecksum;
static uint8_t  _ProtocolBuf[COM_RX_SIZE] = {0};
static uint32_t _ARCFirmwareUpgradeEndTimes = 0;
static uint8_t _ARCFirmwareUpgradeEnd = 0;
static protocal_request_packet_t     _RequestPacketData         = {0};
static protocal_response_packet_t    _ResponsePacketData        = {0};
ProtocalReadResponsePacket_t  gstReadResponsePacketData         = {0};
ComRevUpdateData_t com_revupgradedata;
static const uint16_t _CrcTable16[] =
{
    0X0000, 0X1189, 0X2312, 0X329B, 0X4624, 0X57AD, 0X6536, 0X74BF,
    0X8C48, 0X9DC1, 0XAF5A, 0XBED3, 0XCA6C, 0XDBE5, 0XE97E, 0XF8F7,
    0X1081, 0X0108, 0X3393, 0X221A, 0X56A5, 0X472C, 0X75B7, 0X643E,
    0X9CC9, 0X8D40, 0XBFDB, 0XAE52, 0XDAED, 0XCB64, 0XF9FF, 0XE876,
    0X2102, 0X308B, 0X0210, 0X1399, 0X6726, 0X76AF, 0X4434, 0X55BD,
    0XAD4A, 0XBCC3, 0X8E58, 0X9FD1, 0XEB6E, 0XFAE7, 0XC87C, 0XD9F5,
    0X3183, 0X200A, 0X1291, 0X0318, 0X77A7, 0X662E, 0X54B5, 0X453C,
    0XBDCB, 0XAC42, 0X9ED9, 0X8F50, 0XFBEF, 0XEA66, 0XD8FD, 0XC974,
    0X4204, 0X538D, 0X6116, 0X709F, 0X0420, 0X15A9, 0X2732, 0X36BB,
    0XCE4C, 0XDFC5, 0XED5E, 0XFCD7, 0X8868, 0X99E1, 0XAB7A, 0XBAF3,
    0X5285, 0X430C, 0X7197, 0X601E, 0X14A1, 0X0528, 0X37B3, 0X263A,
    0XDECD, 0XCF44, 0XFDDF, 0XEC56, 0X98E9, 0X8960, 0XBBFB, 0XAA72,
    0X6306, 0X728F, 0X4014, 0X519D, 0X2522, 0X34AB, 0X0630, 0X17B9,
    0XEF4E, 0XFEC7, 0XCC5C, 0XDDD5, 0XA96A, 0XB8E3, 0X8A78, 0X9BF1,
    0X7387, 0X620E, 0X5095, 0X411C, 0X35A3, 0X242A, 0X16B1, 0X0738,
    0XFFCF, 0XEE46, 0XDCDD, 0XCD54, 0XB9EB, 0XA862, 0X9AF9, 0X8B70,
    0X8408, 0X9581, 0XA71A, 0XB693, 0XC22C, 0XD3A5, 0XE13E, 0XF0B7,
    0X0840, 0X19C9, 0X2B52, 0X3ADB, 0X4E64, 0X5FED, 0X6D76, 0X7CFF,
    0X9489, 0X8500, 0XB79B, 0XA612, 0XD2AD, 0XC324, 0XF1BF, 0XE036,
    0X18C1, 0X0948, 0X3BD3, 0X2A5A, 0X5EE5, 0X4F6C, 0X7DF7, 0X6C7E,
    0XA50A, 0XB483, 0X8618, 0X9791, 0XE32E, 0XF2A7, 0XC03C, 0XD1B5,
    0X2942, 0X38CB, 0X0A50, 0X1BD9, 0X6F66, 0X7EEF, 0X4C74, 0X5DFD,
    0XB58B, 0XA402, 0X9699, 0X8710, 0XF3AF, 0XE226, 0XD0BD, 0XC134,
    0X39C3, 0X284A, 0X1AD1, 0X0B58, 0X7FE7, 0X6E6E, 0X5CF5, 0X4D7C,
    0XC60C, 0XD785, 0XE51E, 0XF497, 0X8028, 0X91A1, 0XA33A, 0XB2B3,
    0X4A44, 0X5BCD, 0X6956, 0X78DF, 0X0C60, 0X1DE9, 0X2F72, 0X3EFB,
    0XD68D, 0XC704, 0XF59F, 0XE416, 0X90A9, 0X8120, 0XB3BB, 0XA232,
    0X5AC5, 0X4B4C, 0X79D7, 0X685E, 0X1CE1, 0X0D68, 0X3FF3, 0X2E7A,
    0XE70E, 0XF687, 0XC41C, 0XD595, 0XA12A, 0XB0A3, 0X8238, 0X93B1,
    0X6B46, 0X7ACF, 0X4854, 0X59DD, 0X2D62, 0X3CEB, 0X0E70, 0X1FF9,
    0XF78F, 0XE606, 0XD49D, 0XC514, 0XB1AB, 0XA022, 0X92B9, 0X8330,
    0X7BC7, 0X6A4E, 0X58D5, 0X495C, 0X3DE3, 0X2C6A, 0X1EF1, 0X0F78,
};
//  计算给定长度数据的 16 位 CRC。
uint16_t calculateCRC16(const uint8_t* data, uint16_t length)
{
    uint16_t crc = 0xFFFF;         //  初始化
    while (length > 0)
    {
        crc = (crc >> 8) ^ _CrcTable16[(crc ^ *data) & 0xFF];
        length--;
        data++;
    }
    return  ~crc; //  取反
}
uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode)
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

//static void _CommFrameHeadGet(uint8_t *p)
//{
//  _gCommHead.delimiter = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
//  _gCommHead.verion = p[4] ;
//  _gCommHead.frame_type = p[5];
//  _gCommHead.dst = p[6] | (p[7] << 8) | (p[8] << 16) | (p[9] << 24);
//  _gCommHead.src = p[10] | (p[11] << 8) | (p[12] << 16) | (p[13] << 24);
//  _gCommHead.tunnel_id = p[14] | (p[15] << 8);
//  _gCommHead.frame_Length = p[16] | (p[17] << 8);
//  _gCommHead.body_checksum = p[18] | (p[19] << 8);
//  _gCommHead.head_checksum = p[20] | (p[21] << 8);
//  _gCommHead.seq_id = p[22] | (p[23] << 8) | (p[24] << 16) | (p[25] << 24);
//  _gCommHead.packet_num = p[26] | (p[27] << 8);
//  _gCommHead.packet_index = p[28] | (p[29] << 8);
//  _gCommHead.request_body_type = 0;
//}

//static void _AckHead()
//{
//    _gAckHead.delimiter          = _gCommHead.delimiter;
//    _gAckHead.verion             = _gCommHead.verion;
//    _gAckHead.frame_type         = _gCommHead.frame_type;
//    _gAckHead.dst                = _gCommHead.src;
//    _gAckHead.src                = _gCommHead.dst;
//    _gAckHead.tunnel_id          = _gCommHead.tunnel_id;
//    _gAckHead.request_body_type  = _gCommHead.request_body_type;
//    _gAckHead.seq_id             = _gCommHead.seq_id;
//}

//void CmdErrorLen(uint8_t *p, uint32_t cmd, uint16_t error, uint16_t len)
//{
//    /*cmd*/
//    p[0] = cmd & 0xFF;
//    p[1] = (cmd >> 8) & 0xFF;
//    p[2] = (cmd >> 16) & 0xFF;
//    p[3] = (cmd >> 24) & 0xFF;
//    /*error*/
//    p[4] = error & 0xFF;
//    p[5] = (error >> 8) & 0xFF;
//
//    p[6] = len & 0xFF;
//    p[7] = (len >> 8) & 0xFF;
//}

///*SN烧写*/
//bool SnInfoSet(uint8_t *p_src, uint16_t rev_len)
//{
//    uint8_t sn[24];
//    uint8_t i = 0;
//    char *psn = (char *)&p_src[2];
//    uint8_t sn_len = 0;
//
//    if(rev_len > 22 + 2)
//    {
//        return false;
//    }
//    else if(rev_len > 20 + 2)
//    {
//        if(psn[20] != '\r' && psn[20] != '\n' && psn[20] != 0x00)
//        {
//            return false;
//        }
//    }
//    for(i = 0; (psn[i] != '\r' && psn[i] != '\n' && psn[i] != 0x00 && i < 20); i++)
//    {
//
//    }
//    sn_len = i;
//
//    memcpy(sn, psn, sn_len);
//    sn[sn_len] = 0x00;
//
//    /*保存至FLASH*/
//    SNWrite(sn);
//
//    return true;
//}

///*SN读取*/
//uint16_t SnInfoGet(uint8_t *p_dst)
//{
//    char sn[24];
//    uint16_t len = 0;
//    uint16_t offset = 0;
//    uint32_t cmd = UPPER_CMD_EVENT_READ_SN;
//
//    SNRead((char *)sn);
//    /*User Data Length*/
//    len = 2 + strlen(sn) + 1;
//
//    CmdErrorLen(&p_dst[offset], cmd, 0, len);
//    offset += 8;
//
//    /*User Data*/
//    //modelid
//    p_dst[offset] = 0xC1;
//    offset++;
//    p_dst[offset] = 0x00;
//    offset++;
//    //SN
//    strcpy((char *)&p_dst[offset], sn);
//    offset += strlen(sn);
//    p_dst[offset] = 0x00;
//    offset++;
//
//    return (4 + 2 + 2 + len);
//}
static bool _CmdAnalysis(protocal_request_packet_t *p_request_packet)   //升级数据分析
{
    uint16_t cmd = p_request_packet->cmd;
    static uint32_t arc_firmware_addr;
    static uint32_t arc_firmware_checksum;
    uint16_t i = 0;
    _ResponsePacketData.error_code = ACK_OK;
    gstReadResponsePacketData.error_code = ACK_OK;
    static uint32_t seek = 0;
    static uint32_t file_tatol_len = 0;
    static uint8_t Protocol_extension_len = 0;
    static uint16_t seqid = 0;
    static uint16_t old_seqid = 0;
    static uint16_t cnt = 0;
    uint8_t upgrade_flag = 0;
    switch(cmd)
    {
        case UPGRADE_START:
        {
            /* 初始化程序存放地址 */
            arc_firmware_addr = ARC_FIRMWARE_ADDR;
            arc_firmware_checksum = 0;
            old_seqid = 0xFFFF;
            /*文件总大小 不清楚用在何处*/
            file_tatol_len = p_request_packet->data_field[0] << 24 | p_request_packet->data_field[1] << 16 | p_request_packet->data_field[2] << 8 | p_request_packet->data_field[3];
            /*扩展大小*/
            Protocol_extension_len = p_request_packet->data_field[4] ;
            break;
        }
        case UPGRADE_BUSY:
        {
            seqid = (p_request_packet->data_field[1] << 8) | p_request_packet->data_field[0];
            if(seqid == old_seqid)
            {
                upgrade_flag = 0;
            }
            else
            {
                upgrade_flag = 1;
                old_seqid = seqid;
            }
            if(upgrade_flag)
            {
                /* 第一包数据 */
                if(arc_firmware_addr == ARC_FIRMWARE_ADDR)
                {
                    if(memcmp(&p_request_packet->data_field[Protocol_extension_len], gARCFirmwareType, 4) != 0)
                    {
                        _ResponsePacketData.error_code = ACK_ERROR;
                        return false;
                    }
                    /* 固件版本 */
                    memcpy(gARCHardVersion, &p_request_packet->data_field[4], 3);
                    /* 固件长度 */
                    gARCHardwareLen = *((uint32_t*)&p_request_packet->data_field[7]);
                    /* 固件校验和 */
                    gARCbmsFirmwareChecksum = *((uint32_t*)&p_request_packet->data_field[252 + Protocol_extension_len]);
//                  /* 版本扩展号 */
//                  memcpy(gARCVersionExtend, &p_request_packet->data_field[11], 12);
                    /*固件数据包第一包*/
                    InternalFlashUnlock();
                    InternalFlashWrite(arc_firmware_addr, (uint32_t*)&p_request_packet->data_field[Protocol_extension_len], p_request_packet->data_field_len  - Protocol_extension_len);
                    InternalFlashLock();
                    /*第一包数据校验  可要可不要*/
//                    arc_firmware_checksum += 0;    //校验第一包
                    /*flash存放地址递增*/
                    arc_firmware_addr += p_request_packet->data_field_len  - Protocol_extension_len;
                }
                else
                {
                    InternalFlashUnlock();
                    InternalFlashWrite(arc_firmware_addr, (uint32_t*)&p_request_packet->data_field[Protocol_extension_len], p_request_packet->data_field_len  - Protocol_extension_len);
                    InternalFlashLock();
                    InternalFlashRead(arc_firmware_addr, _ProtocolBuf, (uint32_t)p_request_packet->data_field_len - Protocol_extension_len);
                    /* 计算 checksum */
                    arc_firmware_checksum += CheckSumFun(_ProtocolBuf, p_request_packet->data_field_len - Protocol_extension_len , 1);               //校验其他的数据包
                    arc_firmware_addr += p_request_packet->data_field_len  - Protocol_extension_len;
//                    cnt++;
                }
            }
            break;
        }
        case UPGRADE_END:
        {
            if(arc_firmware_checksum != gARCbmsFirmwareChecksum)
            {
                _ResponsePacketData.error_code = ACK_ERROR;
            }
            else
            {
                uint8_t firmware_upgrade[256];
                memset(firmware_upgrade, 0xA0, 256);
                InternalFlashUnlock();
                InternalFlashWrite(ARC_UPGRADE_FLAG_ADDR, (uint32_t*)firmware_upgrade, 256);
                InternalFlashLock();
                _ARCFirmwareUpgradeEndTimes = tim_get_systime();
                _ARCFirmwareUpgradeEnd = 1;
            }
            break;
        }

        default:
            break;
    }

    return true;

}


//static bool _DataAnalysis(uint8_t *p, uint16_t len)
//{
//    uint8_t is_require;
//    uint16_t frame_len;
//    uint16_t body_checksum, head_checksum;
//
//    uint32_t cmd;
//    uint16_t index = 0;
//    uint16_t ack_index = 0;
//    uint8_t *p_tlv = p + 32;
//
//    if(p[0] != 'N' || p[1] != 'O' || p[2] != 'V' || p[3] != 'A')
//  {
//      return false;
//  }
//
//    if(len < 40)
//    {
//        return false;
//    }
//
//    frame_len = p[16] | (p[17] << 8);
//    body_checksum = p[18] | (p[19] << 8);
//  head_checksum = p[20] | (p[21] << 8);
//    if(body_checksum != calculateCRC16(&p[22], frame_len))
//  {
//      /*Frame Body校验不对*/
//      return false;
//  }
//    if(head_checksum != calculateCRC16(p, 20))
//    {
//        /*包头校验不对*/
//        return false;
//    }
//
//    _CommFrameHeadGet(p);
//
//    if(is_require == 0)
//    {
//        uint16_t user_data_len;
//        uint16_t cmd_data_len;
//        while(index + 32 < len)
//        {
//            cmd = p_tlv[index] | (p_tlv[index + 1] << 8) | (p_tlv[index + 2] << 16) | (p_tlv[index + 3] << 24);
//            user_data_len = p_tlv[index + 6] | (p_tlv[index + 7] << 8);
//            switch(cmd)
//            {
//                /*READ*/
//                case UPPER_CMD_EVENT_READ_SN:
//                {
//                    cmd_data_len = SnInfoGet(&_TxAckBuf[32 + ack_index]);
//                    break;
//                }
//
//                /*WRITE*/
//                case UPPER_CMD_EVENT_WRITE_SN:
//                {
//                    CmdErrorLen(&_TxAckBuf[32 + ack_index], cmd, 0, 0);
//                    if(SnInfoSet(&p_tlv[index + 8], user_data_len) == false)
//                    {
//                        return false;
//                    }
//                    cmd_data_len = 8;
//                    break;
//                }
//                default:
//                {
//                    return false;
//                }
//            }
//            ack_index += cmd_data_len;
//            index += (8 + user_data_len);
//        }
//
//        _AckHead();
//        _gAckHead.seq_id |= (1 << 31);
//        _gAckHead.packet_num = 1;
//        _gAckHead.packet_index = 0;
//        _gAckHead.frame_Length = ack_index + 10;
//
//        memcpy(_TxAckBuf, (uint8_t *)&_gAckHead, 32);
//        /*计算校验*/
//        body_checksum = calculateCRC16(&_TxAckBuf[22], _gAckHead.frame_Length);
//        _TxAckBuf[18] = body_checksum & 0xff;
//        _TxAckBuf[19] = (body_checksum >> 8) & 0xff;
//        /*Head Checksum*/
//        head_checksum = calculateCRC16(_TxAckBuf, 20);
//        _TxAckBuf[20] = head_checksum & 0xff;
//        _TxAckBuf[21] = (head_checksum >> 8) & 0xff;
//
//        Comm485Send(_TxAckBuf, ack_index + 32);
//    }
//
//    return true;
//}

void DataSendPolling(uint8_t read_responsed)
{
    uint16_t check_sum = 0;

    /* 组包 */
    _ResponsePacketData.packet_head = RESPONSE_PACKET_HEAD;
    _ResponsePacketData.send_addr   = _RequestPacketData.recv_addr;
    _ResponsePacketData.recv_addr   = _RequestPacketData.send_addr;
    _ResponsePacketData.cmd         = _RequestPacketData.cmd;
    if(read_responsed)
    {
        memcpy((uint8_t*)&gstReadResponsePacketData, (uint8_t*)&_ResponsePacketData, 5);
        memcpy(garrComTxBuf, (uint8_t*)&gstReadResponsePacketData, sizeof(ProtocalReadResponsePacket_t));
        if(gstReadResponsePacketData.error_code == ACK_ERROR)
        {
            /* ack error data_len 后跟 check_sum */
            check_sum = CheckSumFun(garrComTxBuf, sizeof(ProtocalReadResponsePacket_t), 0);
            memcpy((garrComTxBuf + sizeof(ProtocalReadResponsePacket_t)), (uint8_t*)&check_sum, sizeof(check_sum));
        }
        else
        {
            /* ack ok data_len 后跟 data + check_sum */
            check_sum = CheckSumFun(garrComTxBuf, gstReadResponsePacketData.data_field_len + sizeof(ProtocalReadResponsePacket_t), 0);
            memcpy((garrComTxBuf + sizeof(ProtocalReadResponsePacket_t) + gstReadResponsePacketData.data_field_len), (uint8_t*)&check_sum, sizeof(check_sum));
        }

//        gUart1ModbusHandle.pTxBuf = (uint8_t*)&garrComTxBuf;
//        gUart1ModbusHandle.tx_size = gstReadResponsePacketData.data_field_len + sizeof(ProtocalReadResponsePacket_t) + sizeof(check_sum);
//        ModbusRTUSend(_ModbusAckData, _ModbusAckSize);
        ModbusRTUSend(garrComTxBuf, gstReadResponsePacketData.data_field_len + sizeof(ProtocalReadResponsePacket_t) + sizeof(check_sum));
    }
    else
    {
        _ResponsePacketData.check_sum = CheckSumFun((uint8_t*)&_ResponsePacketData, sizeof(_ResponsePacketData) - 2, 0);
//      gUart1ModbusHandle.pTxBuf = (uint8_t*)&;
//      gUart1ModbusHandle.tx_size = ;
        ModbusRTUSend((uint8_t*)&_ResponsePacketData, sizeof(protocal_response_packet_t));

    }

    return;
}

//void ProtoUniformRevProcess()
//{
//    if(gDevComm485.data_received == true)
//    {
//        _DataAnalysis(gDevComm485.p_data, gDevComm485.data_len);
//        gDevComm485.data_received = false;
//    }
//}

bool ProtoUnpackJudge(ComRevData_t *p)
{
    uint16_t checksum        = 0;
    uint8_t read_responsed   = 0;

    if(p == NULL || p->pData == NULL)
    {
        return false;
    }
    /* 解包 */
    memcpy(&_RequestPacketData, p->pData, 7);
    _RequestPacketData.data_field = _ProtocolBuf;
    _RequestPacketData.check_sum = (uint16_t)p->pData[p->data_size - 2] + (((uint16_t)p->pData[p->data_size - 1]) << 8);   /* 最后两个字节为校验位 */
    memcpy(_ProtocolBuf, (uint8_t*)(p->pData + 7), _RequestPacketData.data_field_len);
    checksum = CheckSumFun(p->pData, (p->data_size - 2), 0);
//  _RequestPacketData.data_field = _ProtocolBuf;
    if(_RequestPacketData.check_sum != checksum)
    {
        _ResponsePacketData.error_code = ACK_ERROR;
    }
    else
    {
        if(!(_RequestPacketData.cmd & 0x8000))   /* 升级cmd高位第一位全为0，此处为1即要读取BMS电量值 */
        {
            read_responsed = 1;
        }
        _CmdAnalysis(&_RequestPacketData);
    }

    /* 主机信息加电包信息立即上报 */
    if((_RequestPacketData.cmd == HISTORY_ENERGY) && (*_RequestPacketData.data_field != 0) && (*_RequestPacketData.data_field < 4))
    {

    }
    else
    {
        DataSendPolling(read_responsed);
    }

    return true;
}

void SysRestart(void)
{
    __set_PRIMASK(1);//关闭总中断
    __NVIC_SystemReset();
}



void UpgradeCompletehandle(void)
{
    static uint32_t restart_time = 0;
    if(gUpgradeRestartFlag == 1)
    {
        if(tim_difftime_from_now(restart_time) > 12000)
        {
            SysRestart();
        }
    }
    else
    {
        restart_time = tim_get_systime();
    }
}



