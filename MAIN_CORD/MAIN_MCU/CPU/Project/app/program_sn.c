#include "program_sn.h"
#include "sys_info_storage.h"
static UniformProtocolHead_t _gCommHead;
static UniformProtocolHead_t _gAckHead;
static UniformProtocolHead_t _gActiveReportHead;
static uint8_t _garrTxRequestBodyBuf[100];
static const uint16_t _garrCrcTable16[] =
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
        crc = (crc >> 8) ^ _garrCrcTable16[(crc ^ *data) & 0xFF];
        length--;
        data++;
    }
    return  ~crc; //  取反
}


static void _CommFrameHeadGet(uint8_t *p)
{
    _gCommHead.delimiter = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    _gCommHead.verion = p[4] ;
    _gCommHead.frame_type = p[5];
    _gCommHead.dst = p[6] | (p[7] << 8) | (p[8] << 16) | (p[9] << 24);
    _gCommHead.src = p[10] | (p[11] << 8) | (p[12] << 16) | (p[13] << 24);
    _gCommHead.tunnel_id = p[14] | (p[15] << 8);
    _gCommHead.frame_Length = p[16] | (p[17] << 8);
    _gCommHead.body_checksum = p[18] | (p[19] << 8);
    _gCommHead.head_checksum = p[20] | (p[21] << 8);
    _gCommHead.seq_id = p[22] | (p[23] << 8) | (p[24] << 16) | (p[25] << 24);
    _gCommHead.packet_num = p[26] | (p[27] << 8);
    _gCommHead.packet_index = p[28] | (p[29] << 8);
    _gCommHead.request_body_type = 0;
}


static void _AckHead()
{
    _gAckHead.delimiter          = _gCommHead.delimiter;
    _gAckHead.verion             = _gCommHead.verion;
    _gAckHead.frame_type         = _gCommHead.frame_type;
    _gAckHead.dst                = _gCommHead.src;
    _gAckHead.src                = _gCommHead.dst;
    _gAckHead.tunnel_id          = _gCommHead.tunnel_id;
    _gAckHead.request_body_type  = _gCommHead.request_body_type;
    _gAckHead.seq_id             = _gCommHead.seq_id;
}


static void _ActiveReportHead()
{
    static uint32_t id;

    _gActiveReportHead.delimiter = 0x41564f4e;
    _gActiveReportHead.verion    = 0x00;
    _gActiveReportHead.frame_type = 1;
    _gActiveReportHead.tunnel_id = 0;

    _gActiveReportHead.seq_id = id & (~((uint32_t)1 << 31));
    _gActiveReportHead.request_body_type = 0;
    id++;
}
static bool _DataUploadComm(uint8_t func, uint8_t *p, uint16_t size, void (*send)(uint8_t *, uint16_t ), uint8_t mode)
{
    static uint8_t *pData = NULL;

    static uint32_t timeout = 0;
    static uint16_t total_size = 0;
    static uint16_t max_size_per_packet = 0;
    static void (*sd)(uint8_t *, uint16_t );

    static uint8_t packet_num = 0;
    static uint8_t packet_index = 0;
    static UniformProtocolHead_t *p_ack_head;

    static uint8_t arr_tx_buf[100];

    uint16_t body_checksum, head_checksum;

    pData = p;
    sd = send;
    total_size = size;

    packet_num = 1;
    packet_index = 0;
    if(mode == 0)
    {
        p_ack_head = &_gAckHead;
        _AckHead();
        p_ack_head->seq_id |= ((uint32_t)1 << 31);
        p_ack_head->packet_num = packet_num;

    }
    else
    {
        p_ack_head = &_gActiveReportHead;
        _ActiveReportHead();
        p_ack_head->seq_id &= ~((uint32_t)1 << 31);
        p_ack_head->packet_num = packet_num;
    }

    p_ack_head->packet_index = packet_index;
    p_ack_head->frame_Length = total_size + 10;
    memcpy(arr_tx_buf, p_ack_head, 32);
    memcpy(&arr_tx_buf[32], pData, total_size);
    /*计算校验*/
    /*Body Checksum*/
    body_checksum = calculateCRC16(&arr_tx_buf[22], p_ack_head->frame_Length);
    arr_tx_buf[18] = body_checksum & 0xff;
    arr_tx_buf[19] = (body_checksum >> 8) & 0xff;
    /*Head Checksum*/
    head_checksum = calculateCRC16(arr_tx_buf, 20);
    arr_tx_buf[20] = head_checksum & 0xff;
    arr_tx_buf[21] = (head_checksum >> 8) & 0xff;
    sd(arr_tx_buf, total_size + 32);

    return true;
}
void CmdErrorLen(uint8_t *p, uint32_t cmd, uint16_t error, uint16_t len)
{
    /*cmd*/
    p[0] = cmd & 0xFF;
    p[1] = (cmd >> 8) & 0xFF;
    p[2] = (cmd >> 16) & 0xFF;
    p[3] = (cmd >> 24) & 0xFF;
    /*error*/
    p[4] = error & 0xFF;
    p[5] = (error >> 8) & 0xFF;

    p[6] = len & 0xFF;
    p[7] = (len >> 8) & 0xFF;
}
/*生产烧写*/
bool DeviceInfoWrite(uint8_t *pSrc, uint16_t rev_len)
{
    uint8_t i = 0;
    uint8_t arr_model_id[2];
    char *psn = (char *)&pSrc[2];
    uint8_t sn_len = 0;
    //gstCollectParaData.pcs_data.udp_pcs_input_para.mode_id
    arr_model_id[0] = (uint8_t)(gstCollectInfo.maincard_data.base_info.model_id >> 8);
    arr_model_id[1] = (uint8_t)(gstCollectInfo.maincard_data.base_info.model_id);
    if((pSrc[0] != 0x00 || pSrc[1] != 0x00) && (pSrc[0] != 0xFF || pSrc[1] != 0xFF))
    {
        arr_model_id[0] = pSrc[0];
        arr_model_id[1] = pSrc[1];
    }

    if(rev_len > 22 + 2)
    {
        return false;
    }
    else if(rev_len > 20 + 2)
    {
        if(psn[20] != '\r' && psn[20] != '\n' && psn[20] != 0x00)
        {
            return false;
        }
    }
    for(i = 0; (psn[i] != '\r' && psn[i] != '\n' && psn[i] != 0x00 && i < 20); i++)
    {

    }
    sn_len = i;

    gstCollectInfo.maincard_data.base_info.model_id = arr_model_id[0];
    gstCollectInfo.maincard_data.base_info.model_id <<= 8;
    gstCollectInfo.maincard_data.base_info.model_id += arr_model_id[1];


    memset(gstCollectInfo.maincard_data.base_info.sn, 0, sizeof(gstCollectInfo.maincard_data.base_info.sn));
    memcpy((uint8_t *)gstCollectInfo.maincard_data.base_info.sn, psn, sn_len);


    /*保存至FLASH*/
    DeviceBaseInfoWrite();

    /*重新获取控制信息，主要是更新SN带来的默认名称的变化*/
//    DeviceCtlRead();



    return true;
}

/*设备校时信息组包*/
uint16_t DeviceReadCalibrationTimePackaging(uint8_t *pDst, uint8_t ack_or_report)
{
    uint8_t i = 0;
    uint16_t len = 0;
    uint16_t offset = 0;
    uint32_t  time_data = 0;
    uint32_t cmd = UPPER_CMD_EVENT_READ_CALIBRATION_TIME;
    /*cmd*/
    if(ack_or_report == 1)
    {
        cmd |= ((uint32_t)1 << 31);
    }


//    offset += 8;

//    /*User Data*/
//    pDst[offset++] = gSysInfo.rtc_time.time_offset & 0xff;
//    pDst[offset++] = (gSysInfo.rtc_time.time_offset >> 8) & 0xff;
//    pDst[offset++] = (gSysInfo.rtc_time.time_offset >> 16) & 0xff;
//    pDst[offset++] = (gSysInfo.rtc_time.time_offset >> 24) & 0xff;


//    time_data = gSysInfo.rtc_time.time_stamp;
//    pDst[offset++] = time_data & 0xff;
//    pDst[offset++] = (time_data >> 8) & 0xff;
//    pDst[offset++] = (time_data >> 16) & 0xff;
//    pDst[offset++] = (time_data >> 24) & 0xff;

//    while((gSysInfo.rtc_time.zoneinfo[i]!='\0')&&(i<sizeof(gSysInfo.rtc_time.zoneinfo))){
//        pDst[offset++] = gSysInfo.rtc_time.zoneinfo[i];
//        i++;
//    }
//    pDst[offset++] = '\0';

//    len = offset-8;
    CmdErrorLen(&pDst[0], cmd, 0, len);
    return (4 + 2 + 2 + len);
}
/*设备名称信息组包*/
uint16_t DeviceReadNamePackaging(uint8_t *pDst, uint8_t ack_or_report)
{
    uint16_t len = 0;
    uint16_t offset = 0;
    uint32_t cmd = UPPER_CMD_EVENT_READ_DEVICE_NAME;
    /*cmd*/
    if(ack_or_report == 1)
    {
        cmd |= ((uint32_t)1 << 31);
    }

//    len = strlen(gSysInfo.name) + 1;
//    if(len >= sizeof(gSysInfo.name) - 1)
//    {
//        len = 1;
//    }
    CmdErrorLen(&pDst[offset], cmd, 0, len);
//    offset += 8;
//
//    memcpy(&pDst[offset], gSysInfo.name, len - 1);
//    offset += len - 1;
//    pDst[offset] = 0x00;
//    offset++;

    return (4 + 2 + 2 + len);
}



/*设备信息*/
uint16_t DeviceInfoPackaging(uint8_t *pDst, uint8_t ack_or_report)
{
    uint16_t len = 0;
    uint16_t offset = 0;
    uint16_t cnt = 0;
    uint32_t cmd = UPPER_CMD_EVENT_READ_CURRENT_DEVICE_INFO;
    char *p[13];
    /*cmd*/
    if(ack_or_report == 1)
    {
        cmd |= ((uint32_t)1 << 31);
    }

    /*User Data Length*/
//    len = 2 + strlen(gSysInfo.pdt_info.sn) + 2 + strlen(gSysInfo.pdt_info.sw_version) + 2 +                     \
//          strlen(gSysInfo.pdt_info.sw_main_ctl_version) + 2 + strlen(gSysInfo.pdt_info.sw_bms_version) + 2 +    \
//          strlen(gSysInfo.pdt_info.sw_pfc_version) + 2 + strlen(gSysInfo.pdt_info.sw_llc_version) + 2 +         \
//          strlen(gSysInfo.pdt_info.sw_dcdc_version) + 2 + strlen(gSysInfo.pdt_info.sw_mppt_version) + 2 +       \
//          strlen(gSysInfo.pdt_info.hw_main_ctl_version) + 2 + strlen(gSysInfo.pdt_info.hw_bms_version) + 2 +    \
//          strlen(gSysInfo.pdt_info.hw_pcs_version) + 2 + strlen(gSysInfo.pdt_info.hw_dcdc_version) + 2 + strlen(gSysInfo.pdt_info.hw_mppt_version) + 2;


    len = 2 + strlen((char *)gstCollectInfo.maincard_data.base_info.sn) + 2 ;


    CmdErrorLen(&pDst[offset], cmd, 0, len);
    offset += 8;

    /*User Data*/
    //modelid
    memcpy(&pDst[offset], &gstCollectInfo.maincard_data.base_info.model_id, 2);
    offset += 2;

    p[cnt++] = (char *)gstCollectInfo.maincard_data.base_info.sn;
//    p[cnt++] = gSysInfo.pdt_info.sw_version;
//    p[cnt++] = gSysInfo.pdt_info.sw_main_ctl_version;
//    p[cnt++] = gSysInfo.pdt_info.sw_bms_version;
//    p[cnt++] = gSysInfo.pdt_info.sw_pfc_version;
//    p[cnt++] = gSysInfo.pdt_info.sw_llc_version;
//    p[cnt++] = gSysInfo.pdt_info.sw_dcdc_version;
//    p[cnt++] = gSysInfo.pdt_info.sw_mppt_version;
//    p[cnt++] = gSysInfo.pdt_info.hw_main_ctl_version;
//    p[cnt++] = gSysInfo.pdt_info.hw_bms_version;
//    p[cnt++] = gSysInfo.pdt_info.hw_pcs_version;
//    p[cnt++] = gSysInfo.pdt_info.hw_dcdc_version;
//    p[cnt++] = gSysInfo.pdt_info.hw_mppt_version;

    for(int i = 0; i < 1; i++)
    {
        memcpy(&pDst[offset], p[i], strlen(p[i]));
        offset += strlen(p[i]);
        pDst[offset] = '\r';
        pDst[offset + 1] = '\n';
        offset += 2;
    }

    return (4 + 2 + 2 + len);
}

/*针对读请求，先组包，再发送*/
static bool _CmdAnalysis(uint8_t *pBuf, uint16_t len)
{
    uint16_t index = 0;
    uint16_t ack_index = 0;
    uint8_t is_require;
    uint32_t cmd;
    uint8_t *pData;
    uint16_t tlv_data_len;
    uint16_t max_mtu;
    uint32_t start_time = 0;
    uint32_t end_time = 0;
    is_require = (_gCommHead.seq_id >> 31);
    if(is_require == 0)
    {


        uint16_t user_data_len;
        uint16_t cmd_data_len;
        while(index < len)
        {

            cmd = pBuf[index] | (pBuf[index + 1] << 8) | (pBuf[index + 2] << 16) | (pBuf[index + 3] << 24);
            user_data_len = pBuf[index + 6] | (pBuf[index + 7] << 8);
            switch(cmd)
            {
                /*READ*/
                case UPPER_CMD_EVENT_READ_CURRENT_DEVICE_INFO:
                {
                    cmd_data_len = DeviceInfoPackaging(&_garrTxRequestBodyBuf[ack_index], 0);
//cmd_data_len = 8;
                    break;
                }
                case UPPER_CMD_EVENT_READ_DEVICE_NAME:
                {
                    cmd_data_len = DeviceReadNamePackaging(&_garrTxRequestBodyBuf[ack_index], 0);
                    break;
                }
                case UPPER_CMD_EVENT_READ_CALIBRATION_TIME:
                {
                    cmd_data_len = DeviceReadCalibrationTimePackaging(&_garrTxRequestBodyBuf[ack_index], 0);
                    break;
                }


                /*WRITE*/
                case UPPER_CMD_EVENT_WRITE_DEVICE_INFO:
                {
                    CmdErrorLen(&_garrTxRequestBodyBuf[ack_index], cmd, 0, 0);
                    if(DeviceInfoWrite(&pBuf[index + 8], user_data_len) == false)
                    {
                        return false;
                    }
                    break;
                }
                case UPPER_CMD_EVENT_WRITE_DEVICE_NAME:
                {
                    CmdErrorLen(&_garrTxRequestBodyBuf[ack_index], cmd, 0, 0);
//                    if(DeviceWriteNameDataProcess(&pBuf[index + 8], user_data_len) == false)
//                    {
//                        return false;
//                    }
                    cmd_data_len = 8;
                    break;
                }
                case UPPER_CMD_EVENT_WRITE_CALIBRATION_TIME:
                {
                    CmdErrorLen(&_garrTxRequestBodyBuf[ack_index], cmd, 0, 0);
//                    if(DeviceWriteCalibrationTimeDataProcess(&pBuf[index + 8], user_data_len) == false)
//                    {
//                        return false;
//                    }
                    cmd_data_len = 8;
                    break;
                }

                default:
                {
                    return false;
//                    index += 8;
//                    break;
                }
            }
            ack_index += cmd_data_len;
            index += (8 + user_data_len);

        }
        _DataUploadComm(1, _garrTxRequestBodyBuf, ack_index, ModbusCollectorRTUSend, 0);
    }
    else
    {

    }

    return true;
}










uint8_t _WifiSameDataPacketFlag = 0;

static  uint8_t _ProgramSnProtocolProc(uint8_t *pBuf, uint16_t len)
{

    uint16_t frame_len;
    uint16_t body_checksum, head_checksum;
    uint16_t total_packet_num, packet_index;
    static uint16_t app_redeposit_offset = 0;
    static uint32_t old_seqid = 0xffff;
    static uint32_t seqid = 0;
    volatile uint16_t  test_body_checksum = 0;
    volatile uint16_t  test_head_checksum = 0;

    if(pBuf[0] != 'N' || pBuf[1] != 'O' || pBuf[2] != 'V' || pBuf[3] != 'A')
    {
        return false;
    }

    frame_len = pBuf[16] | (pBuf[17] << 8);
    body_checksum = pBuf[18] | (pBuf[19] << 8);
    head_checksum = pBuf[20] | (pBuf[21] << 8);

    test_body_checksum = calculateCRC16(&pBuf[22], frame_len);
    test_head_checksum = calculateCRC16(pBuf, 20);

    if(body_checksum != calculateCRC16(&pBuf[22], frame_len))
    {
        /*Frame Body校验不对*/
        return false;
    }
    if(head_checksum != calculateCRC16(pBuf, 20))
    {
        /*包头校验不对*/
        return false;
    }
    seqid   =  pBuf[22] | (pBuf[23] << 8) | (pBuf[24] << 16) | (pBuf[25] << 24);
    seqid   &= 0x7fffff;
    if(seqid == old_seqid)
    {
        _WifiSameDataPacketFlag = 1;            //两次包相同，在升级的时候，不需要处理，只返回即可
    }
    else
    {
        old_seqid = seqid;
        _WifiSameDataPacketFlag  = 0;
    }

    total_packet_num = pBuf[26] | (pBuf[27] << 8);
    packet_index = pBuf[28] | (pBuf[29] << 8);


    if((total_packet_num == 1) && (packet_index == 0))
    {
        _CommFrameHeadGet(pBuf);
        if(frame_len < 10)
        {
            return false;
        }
        _CmdAnalysis(&pBuf[32], frame_len - 10);

    }
    return true;
}
void ProgramFunInital(void)
{
    RegisterProgramSnFun(_ProgramSnProtocolProc);
}
