
#include "wireless_protocol.h"
#include "common.h"

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
//  计算给定长度数据�?16 �?CRC�?
uint16_t calculateCRC16(const uint8_t* data, uint16_t length)
{
    uint16_t crc = 0xFFFF;         //  初始�?
    while (length > 0) 
	{
        crc = (crc >> 8) ^ _CrcTable16[(crc ^ *data) & 0xFF];
        length--;
        data++;
    }
    return  ~crc; //  取反
}


// UpperCommFrame_t	gUpperCommFrame;

static WifiBleSendData_t _gWifiBleSendData;


/*临时变量*/
static EN_COM_REV_DATA_TYPE _CommType;
static char					     _sSocketId[5];
static char                      _sRemoteIP[20];
static char                      _sRemotePort[8];





static uint8_t	_ProtoUnPackRedepositBuf[4096];/*对于分包传输过来的数据，先转存，待接收完整后，再处理*/
static uint8_t	_AppUnPackRedepositBuf[8192];/*对于分包传输过来的数据，先转存，待接收完整后，再处理*/

/*0：不分包�?：分包，2：分包完�?/
static uint8_t _CommUnpackFlag = 0;/*TCP/UDP协议栈分�?/
static uint8_t _UpperAppUnackFlag = 0;/*应用层分包，通过uniform protocol包头可判�?/



static UniformProtocolHead_t _gCommHead;
static UniformProtocolHead_t _gAckHead;
static UniformProtocolHead_t _gActiveReportHead;

static uint8_t _WifiScanCmdFlag = 0;

static void _ActiveReportHead();
static void _AckHead();

static uint16_t _TxRequestBodyBufSize = 0;
static uint8_t _TxRequestBodyBuf[4096];

//static EN_UPLOAD_STATUS _UpLoadStatus = EN_UPLOAD_STATUS_IDLE;
EN_UPLOAD_STATUS _UpLoadStatus = EN_UPLOAD_STATUS_IDLE;
static bool _DataUpload(uint8_t func, uint8_t *p, uint16_t size, void (*send)(uint8_t, WifiBleSendData_t *), uint8_t mode);

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






/*针对读请求，先组包，再发�?/
static bool _CmdAnalysis(uint8_t *pBuf, uint16_t len)
{
	uint16_t index = 0;
    uint16_t ack_index = 0;
	uint8_t is_require;
	uint32_t cmd;
	uint8_t *pData;
	uint16_t tlv_data_len;
    uint16_t max_mtu;
	
	is_require = (_gCommHead.seq_id >> 31);
	
    /*上一次数据没处理完，新数据不做处�?/
    if(_UpLoadStatus != EN_UPLOAD_STATUS_IDLE)
    {
        return false;
    }
	
    _gWifiBleSendData.data_type = _CommType;
    strcpy(_gWifiBleSendData.s_socket_id, _sSocketId);
    strcpy(_gWifiBleSendData.remote_ip, _sRemoteIP);
    strcpy(_gWifiBleSendData.remote_port, _sRemotePort);
    
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
				case UPPER_CMD_EVENT_READ_WIFI_AP_INFO:/*特殊�?/
				{
					/*需要先从模块获取再响应*/
//                    WIFIScan();
                    _WifiScanCmdFlag = 1;
					return true;
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
        _DataUpload(1, _TxRequestBodyBuf, ack_index, DataSendPolling, 0);
        

		
		
	}
	else
	{
		
	}
    
    return true;
}





static bool _PacketJudge(uint8_t *pBuf, uint16_t len)
{
	uint16_t frame_len;
	uint16_t body_checksum, head_checksum;
	uint16_t total_packet_num, packet_index;
	static uint16_t app_redeposit_offset = 0;

	
	
	if(pBuf[0] != 'N' || pBuf[1] != 'O' || pBuf[2] != 'V' || pBuf[3] != 'A')
	{
		return false;
	}
	
	frame_len = pBuf[16] | (pBuf[17] << 8);
	body_checksum = pBuf[18] | (pBuf[19] << 8);
	head_checksum = pBuf[20] | (pBuf[21] << 8);
	
	if(body_checksum != calculateCRC16(&pBuf[22], frame_len))
	{
		/*Frame Body校验不对*/
		
//		return false;
	}
	if(head_checksum != calculateCRC16(pBuf, 20))
	{
		/*包头校验不对*/
		
//		return false;
	}
	
	total_packet_num = pBuf[26] | (pBuf[27] << 8);
	packet_index = pBuf[28] | (pBuf[29] << 8);
	
	if(_UpperAppUnackFlag == 0)
	{
		app_redeposit_offset = 0;
		if(total_packet_num == 1)
		{
			/*应用未分包，直接解析*/
			_CommFrameHeadGet(pBuf);
			_CmdAnalysis(&pBuf[32], frame_len - 10);
		}
		else
		{
			if(packet_index != 0)
			{
				/*没有收到第一包，不接受后续的�?/
				return false;
			}
			else/*第一�?/
			{
				/*对于整合包不是很大的情况，部分特殊的Cmd Code不能用_AppUnPackRedepositBuf转存*/
				memcpy(&_AppUnPackRedepositBuf[app_redeposit_offset], &pBuf[32], frame_len - 10);
				app_redeposit_offset += frame_len - 10;
				_UpperAppUnackFlag = 1;
				
				/*对于部分特殊的Cmd Code，需要每个分包单独处�?/
				// TBD
				
			}
		}
	}
	else
	{
		if(packet_index == 0)/*抛弃之前的包，从第一包再次开�?/
		{
			_UpperAppUnackFlag = 0;
			if(total_packet_num > 1)/*第一�?/
			{
				app_redeposit_offset = 0;
				memcpy(&_AppUnPackRedepositBuf[app_redeposit_offset], &pBuf[32], frame_len - 10);
				app_redeposit_offset += frame_len - 10;
				_UpperAppUnackFlag = 1;
				
			}
			else
			{
				/*应用未分包，直接解析*/
				_CommFrameHeadGet(pBuf);
				
				
				_CmdAnalysis(&pBuf[32], frame_len - 10);
			}
		}
		else
		{
			memcpy(&_AppUnPackRedepositBuf[app_redeposit_offset], &pBuf[32], frame_len - 10);
			app_redeposit_offset += frame_len - 10;
			
			if(packet_index == total_packet_num - 1)/*最后一�?/
			{
				_UpperAppUnackFlag = 0;
				/*基于转存的合并包，解�?/
				_CommFrameHeadGet(pBuf);
				
				_CmdAnalysis(_AppUnPackRedepositBuf, app_redeposit_offset);
			}
		}
	}
	return true;
	
}


static bool _ProtoUnpackJudge(WifiBleRevData_t *p)
{
	static uint16_t frame_len;
	static uint16_t proto_redeposit_offset = 0;
	
	uint8_t *pBuf = p->pData;
	uint16_t len = p->data_size;
	
	_CommType = p->data_type;
    
    strcpy(_sSocketId, p->s_socket_id);
    strcpy(_sRemoteIP, p->s_remote_ip);
    strcpy(_sRemotePort, p->s_remote_port);
    
    
	
	if(pBuf[0] == 'N' && pBuf[1] == 'O' && pBuf[2] == 'V' && pBuf[3] == 'A' && _CommUnpackFlag == 0)/*第一�?/
	{
		frame_len = pBuf[16] | (pBuf[17] << 8);
		if(frame_len + 22 > len)/*协议栈分包了*/
		{
			_CommUnpackFlag = 1;
			proto_redeposit_offset = 0;
			memcpy(&_ProtoUnPackRedepositBuf[proto_redeposit_offset], pBuf, len);
			proto_redeposit_offset += len;
		}
		else/*协议栈没有分�?/
		{
			_CommUnpackFlag = 0;
			_PacketJudge(pBuf, len);
		}
	}
	else if(_CommUnpackFlag == 1)
	{
		if(frame_len + 22 <= proto_redeposit_offset + len)/*分包完成*/
		{
			_CommUnpackFlag = 0;
			memcpy(&_ProtoUnPackRedepositBuf[proto_redeposit_offset], pBuf, len);
			_PacketJudge(_ProtoUnPackRedepositBuf, proto_redeposit_offset + len);
			proto_redeposit_offset = 0;
		}
		else/*分包未完�?/
		{
			memcpy(&_ProtoUnPackRedepositBuf[proto_redeposit_offset], pBuf, len);
			proto_redeposit_offset += len;
		}
	}
	
	return true;
}











//typedef void (*FUNC_POINT)(uint8_t *, uint16_t);
/*正常轮询func = 0�?初始化func = 1, 查询状态func = 2,*/
/*除func外其余形参，只有在flag = 1的时候有效，其余flag值均无效*/
/*size包含了command code等字�?/
/*mode=0表示正常响应，mode=1表示主动上报*/

static bool _DataUpload(uint8_t func, uint8_t *p, uint16_t size, void (*send)(uint8_t, WifiBleSendData_t *), uint8_t mode)
{
    static uint8_t *pData = NULL;
    
    static uint32_t timeout = 0;
    static uint16_t total_size = 0;
    static uint16_t max_size_per_packet = 0;
    static void (*sd)(uint8_t ,WifiBleSendData_t *);
    
    static uint8_t packet_num = 0;
    static uint8_t packet_index = 0;
    static UniformProtocolHead_t *p_ack_head;
    
    
    if(func == 1)
    {
        if(_UpLoadStatus == EN_UPLOAD_STATUS_IDLE)
        {
            uint16_t max_mtu;
            if(_gWifiBleSendData.data_type == TYPE_WIFI_UDP_DATA || _gWifiBleSendData.data_type == TYPE_WIFI_TCP_DATA)
            {
                max_mtu = 1024;
            }
            else
            {
//                max_mtu = 512;
                max_mtu = 480;
            }
            
            pData = p;
            sd = send;
            max_size_per_packet = max_mtu;
            total_size = size;
            
            packet_num = (total_size + (max_size_per_packet - 33)) / (max_size_per_packet - 32);
            packet_index = 0;
            _UpLoadStatus = EN_UPLOAD_STATUS_SEND;
            
            if(mode == 0)
            {
                p_ack_head = &_gAckHead;
                _AckHead();
                p_ack_head->seq_id |= (1 << 31);
                p_ack_head->packet_num = packet_num;
                
            }
            else
            {
                p_ack_head = &_gActiveReportHead;
                _ActiveReportHead();
                p_ack_head->seq_id &= ~(1 << 31);
                p_ack_head->packet_num = packet_num;
            }
        }
        else
        {
            return false;
        }
        return true;
    }
    
    
    switch(_UpLoadStatus)
    {
        case EN_UPLOAD_STATUS_IDLE:
        {
            break;
        }
        case EN_UPLOAD_STATUS_SEND:
        {
            static uint8_t tx_buf[2048];
            uint16_t body_checksum, head_checksum;
            
//            if(DataSendStatusIdleJudge() == false)
//            {
//                return false;
//            }
            if(packet_index == packet_num - 1)
            {
                p_ack_head->packet_index = packet_index;
                p_ack_head->frame_Length = total_size + 10;
                memcpy(tx_buf, p_ack_head, 32);
                memcpy(&tx_buf[32], pData, total_size);
                /*计算校验*/
                /*Body Checksum*/
                body_checksum = calculateCRC16(&tx_buf[22], p_ack_head->frame_Length);
                tx_buf[18] = body_checksum & 0xff;
                tx_buf[19] = (body_checksum >> 8) & 0xff;
                /*Head Checksum*/
                head_checksum = calculateCRC16(tx_buf, 20);
                tx_buf[20] = head_checksum & 0xff;
                tx_buf[21] = (head_checksum >> 8) & 0xff;
                
                _gWifiBleSendData.pData = tx_buf;
                _gWifiBleSendData.data_size = total_size + 32;
                sd(1, &_gWifiBleSendData);
                _UpLoadStatus = EN_UPLOAD_STATUS_WAIT_LAST;
            }
            else
            {
                p_ack_head->packet_index = packet_index;
                p_ack_head->frame_Length = max_size_per_packet - 22;
                memcpy(tx_buf, p_ack_head, 32);
                memcpy(&tx_buf[32], pData, max_size_per_packet - 32);
                /*计算校验*/
                body_checksum = calculateCRC16(&tx_buf[22], p_ack_head->frame_Length);
                tx_buf[18] = body_checksum & 0xff;
                tx_buf[19] = (body_checksum >> 8) & 0xff;
                /*Head Checksum*/
                head_checksum = calculateCRC16(tx_buf, 20);
                tx_buf[20] = head_checksum & 0xff;
                tx_buf[21] = (head_checksum >> 8) & 0xff;
                
                _gWifiBleSendData.pData = tx_buf;
                _gWifiBleSendData.data_size = max_size_per_packet;
                sd(1, &_gWifiBleSendData);
                pData += max_size_per_packet - 32;
                packet_index++;
                total_size -= max_size_per_packet - 32;
                _UpLoadStatus = EN_UPLOAD_STATUS_WAIT;
            }           
            timeout = tim_get_systime();
            break;
        }
        case EN_UPLOAD_STATUS_WAIT:
        {
//            if(ModuleAckDetect() == true)
            {
                _UpLoadStatus = EN_UPLOAD_STATUS_SEND;
            }
            else if(tim_get_systime() - timeout > 3000)
            {
                _UpLoadStatus = EN_UPLOAD_STATUS_IDLE;
            }
            break;
        }
        case EN_UPLOAD_STATUS_WAIT_LAST:
        {
//            if((ModuleAckDetect() == true) || (tim_get_systime() - timeout > 300))
            {
                _UpLoadStatus = EN_UPLOAD_STATUS_IDLE;
            }
            break;
        }
        default:break;
    }
    return true;
}



static void _ActiveReportHead()
{
    static uint32_t id;
    
    _gActiveReportHead.delimiter = 0x41564f4e;
    _gActiveReportHead.verion    = 0x00;
    _gActiveReportHead.frame_type = 1;
    _gActiveReportHead.dst = 0xffffffff;
    _gActiveReportHead.src = 0;
    _gActiveReportHead.tunnel_id = 0;
    
    _gActiveReportHead.seq_id = id & (~(1 << 31));
    _gActiveReportHead.request_body_type = 0;
    id++;
}


void WirelessDataProcess(void)
{
	ComRevData_t *p;
    
    if(_UpLoadStatus == EN_UPLOAD_STATUS_IDLE)
    {
        if(QueueGet(&gComRevQueue, (void *)&p) == true)
        {
//            if(p->data_type == TYPE_WIFI_UDP_DATA || p->data_type == TYPE_WIFI_TCP_DATA || p->data_type == TYPE_BLE_DATA || p->data_type == TYPE_COM_DATA)
            {
                _ProtoUnpackJudge(p);
            }
        }
    }
	_DataUpload(0, NULL, 0, NULL, 0);
	DataSendPolling(0, NULL);
	/*_CommUnpackFlag需要增加超时机�?/

}





















