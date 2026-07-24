#include "dev_communication.h"
#include "math.h"
#include "mid_shell.h"
#define LOG_TAG     "dev_communication"

//#define UartComHandle  Uart3_Debug_Handle
//#define UartComHandle  Uart1_ModS_Handle

uint8_t garrComRxBuf[COM_RX_SIZE]; 
uint8_t garrComTxBuf[COM_TX_SIZE];
//static ComRevData_t	_gComRevData[COM_REV_QUEUE_DEPTH];
//static uint8_t _RevData[COM_REV_QUEUE_DEPTH][_COM_RX_SIZE];
//static uint8_t _RevDataBufIndex = 0;
//static uint16_t	_DataRevSize = 0;
static protocal_request_packet_t     _RequestPacketData         = {0};
static protocal_response_packet_t    _ResponsePacketData        = {0};
ProtocalReadResponsePacket_t  gstReadResponsePacketData         = {0};
static uint8_t  _ProtocolBuf[COM_RX_SIZE] = {0};
static uint32_t _MPPTFirmwareUpgradeEndTimes = 0;
static uint8_t _MPPTFirmwareUpgradeEnd = 0;
uint8_t gMPPTHardVersion[3] = {0};
uint32_t gMPPTHardwareLen;
uint32_t gMPPTbmsFirmwareChecksum;
uint8_t gBMSFirmwareType[4] = {0x0A, 0x0A, 0x0A, 0x03};
uint8_t gMPPTFirmwareType[4] = {0x0A, 0x0A, 0x0A, 0x05};
Queue_t	gComRevQueue;

ComRevUpdateData_t com_revupgradedata;
static void IrqRxProcess(void);

/**
* @brief CheckSumFun
* @param *pdata: data
* @param len: len
* @param mode: 0-checksum16     1-checksum32
* @return
* - checksum
* @note 
*/
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

/////////***********************外移****************************////////
//static void _RevDataAnalysis()
//{
//    uint8_t *p = UartComHandle.pRxBuf;
//    if((*UartComHandle.pRxBuf == REQUST_PACKET_HEAD) && (*(UartComHandle.pRxBuf + 1) == SEND_ADDR) && \
//       (*(UartComHandle.pRxBuf + 2) == RECV_ADDR))
//	{
//		if(QueueIsFull() == true)
//		{
//			return;//扔掉此包数据
//		}

//		_gComRevData[_RevDataBufIndex].data_type = COM_TYPE_UART_DATA;

//		_gComRevData[_RevDataBufIndex].data_size = _DataRevSize;
//		memcpy(_gComRevData[_RevDataBufIndex].pData, p, _DataRevSize);
//		if(QueuePut(&gComRevQueue, &_gComRevData[_RevDataBufIndex]) == false)
//		{
//			return;
//		}
//		
//		_RevDataBufIndex++;
//		_RevDataBufIndex %= COM_REV_QUEUE_DEPTH;
//	}
//}



//static void IrqRxProcess()
//{
//	/*DMA方式，空闲中断机制*/
//	UartDMARxDisable(&UartComHandle);
//	_DataRevSize = UartDMARxDataLength(&UartComHandle);
//	
//    _RevDataAnalysis();
//      
//	UartDMARxEnable(&UartComHandle);
//}


//bool CommunicationInit()
//{
//    bool ret;
//    uint8_t times = 0;
//    uint8_t i = 0;
//    
//	QueueInit(&gComRevQueue, COM_REV_QUEUE_DEPTH, sizeof(ComRevData_t));
//	for(i = 0; i < COM_REV_QUEUE_DEPTH; i++)
//	{
//		_gComRevData[i].pData = _RevData[i];
//	}

//	UartComHandle.pRxBuf = _gComRxBuf;
//	UartComHandle.rx_size = _COM_RX_SIZE;
//    UartComHandle.irq_rx_process = IrqRxProcess;
//    Uart3DebugBspInit();
//    
//    if(UartComHandle.uart_cfg->en_dma_rx == true)
//    {    
//        /* 重新设置DMA传输(初始化DMA还未映射rx_buf,上面映射完后重新初始化一下DMA)*/		 
//        UartDMARxDisable(&UartComHandle);
//        UartDMARxEnable(&UartComHandle);
//    }
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
                
        Uart2Handle.pTxBuf = (uint8_t*)&garrComTxBuf;
        Uart2Handle.tx_size = gstReadResponsePacketData.data_field_len + sizeof(ProtocalReadResponsePacket_t) + sizeof(check_sum);
        ModbusRTUSend(Uart2Handle.pTxBuf, Uart2Handle.tx_size);
    }
    else
    {
        _ResponsePacketData.check_sum = CheckSumFun((uint8_t*)&_ResponsePacketData, sizeof(_ResponsePacketData) - 2, 0);    
				Uart2Handle.pTxBuf = (uint8_t*)&_ResponsePacketData;
				Uart2Handle.tx_size = sizeof(protocal_response_packet_t);
        ModbusRTUSend(Uart2Handle.pTxBuf, Uart2Handle.tx_size);
    } 
    
    return;                       
}

static bool _VerifyBmsFirmware(void)
{
    uint16_t i = 0;
    uint16_t block_num;
    uint16_t block_num_behind;
    uint8_t rdata[BLOCK_SIZE] = {0};
    uint32_t checksum = 0;
    uint32_t mppt_firmware_addr = BMS_FIRMWARE_ADDR;
    
    block_num = gMPPTHardwareLen / BLOCK_SIZE;
    block_num_behind = gMPPTHardwareLen % BLOCK_SIZE;
    
    for(i = 0; i < block_num; i ++)
    {
        spi_flash_read_bytes(mppt_firmware_addr, rdata, BLOCK_SIZE);
        checksum += CheckSumFun(rdata, BLOCK_SIZE, 1);
        mppt_firmware_addr += BLOCK_SIZE;
    }
    
    spi_flash_read_bytes(mppt_firmware_addr, rdata, block_num_behind);
    checksum += CheckSumFun(rdata, block_num_behind, 1);
    
    if(checksum != gMPPTbmsFirmwareChecksum)    
    {
        return false;
    }
    
    return true;
}

/**Function: _CmdAnalysis
* @brief	 固件升级解析
 * @param	 p_request_packet：数据包地址
 * @return 失败/成功
*/
static bool _CmdAnalysis(protocal_request_packet_t *p_request_packet)
{
    uint16_t cmd = p_request_packet->cmd;
    static uint32_t mppt_firmware_addr;
    static uint32_t mppt_firmware_checksum;
    uint16_t i = 0;
    _ResponsePacketData.error_code = ACK_OK;
    gstReadResponsePacketData.error_code = ACK_OK;
    
    switch(cmd)
    {
        case UPGRADE_START:
        {
            /* 初始化程序存放地址 */
            mppt_firmware_addr = MPPT_FIRMWARE_ADDR;
            mppt_firmware_checksum = 0;
            LOG_INFO(LOG_TAG, "mppt_firmware_upgrade start!");
            /* 升级期间暂停AFE采集 */
//////             StopAFESample();
//            gFirst_AFE_Coved_Flag = 9;

            break;
        } 
        case UPGRADE_BUSY:
        {
            /* 第一包数据 */
            if(mppt_firmware_addr == MPPT_FIRMWARE_ADDR)
            {
                if(memcmp(p_request_packet->data_field, gMPPTFirmwareType, 4) != 0)
                {
                    _ResponsePacketData.error_code = ACK_ERROR;
                    return false;
                }
                /* 固件版本 */
                memcpy(gMPPTHardVersion, &p_request_packet->data_field[4], 3);
                /* 固件长度 */
                gMPPTHardwareLen = *((uint32_t*)&p_request_packet->data_field[7]);
                /* 固件校验和 */
                gMPPTbmsFirmwareChecksum = *((uint32_t*)&p_request_packet->data_field[252]);
 
            }  

            spi_flash_write_bytes_with_erase(mppt_firmware_addr, p_request_packet->data_field, p_request_packet->data_field_len);
            spi_flash_read_bytes(mppt_firmware_addr, _ProtocolBuf, p_request_packet->data_field_len);
					
            if(mppt_firmware_addr == MPPT_FIRMWARE_ADDR)
            {
                /* 计算 checksum */
                mppt_firmware_checksum += CheckSumFun(&_ProtocolBuf[256], p_request_packet->data_field_len - 256, 1);
//                LOG_INFO(LOG_TAG, "CheckSum:0x%02X", mppt_firmware_checksum);
            }
            else
            {
                /* 计算 checksum */
                mppt_firmware_checksum += CheckSumFun(_ProtocolBuf, p_request_packet->data_field_len, 1);    
//                LOG_INFO(LOG_TAG, "CheckSum:0x%02X", mppt_firmware_checksum);                
            }
   
            mppt_firmware_addr += p_request_packet->data_field_len;
            break;
        }
        case UPGRADE_END:
        {                        
            if(mppt_firmware_checksum != gMPPTbmsFirmwareChecksum)
            {
                _ResponsePacketData.error_code = ACK_ERROR;
                LOG_INFO(LOG_TAG, "mppt_firmware_checksum error!!!");
            }
            else
            {
                uint8_t firmware_upgrade[256];
                memset(firmware_upgrade, 0xA0, 256);
                spi_flash_write_bytes_with_erase(MPPT_UPGRADE_FLAG_ADDR, firmware_upgrade, 256);
                _MPPTFirmwareUpgradeEndTimes = tim_get_systime();
                _MPPTFirmwareUpgradeEnd = 1;
                LOG_INFO(LOG_TAG, "mppt_firmware_checksum ok!!!");
            }
            /* 升级结束继续AFE采集 */
///////             StartAFESample();
//            gFirst_AFE_Coved_Flag = 8;
            break;
        } 

        default: break;
    }
    
    return true;
    
}
bool ProtoUnpackJudge(ComRevData_t *p)
{
    uint16_t checksum      	 = 0;
    uint8_t read_responsed 	 = 0;
	
    if(p == NULL || p->pData == NULL)
    {
        return false;
    }
    
    /* 解包 */
    memcpy(&_RequestPacketData, p->pData, 7);
    _RequestPacketData.data_field = _ProtocolBuf;
    
    _RequestPacketData.check_sum = (uint16_t)p->pData[p->data_size - 2] + (((uint16_t)p->pData[p->data_size - 1]) << 8);
    memcpy(_ProtocolBuf, (uint8_t*)(p->pData + 7), _RequestPacketData.data_field_len);
    
    checksum = CheckSumFun(p->pData, (p->data_size - 2), 0);
    if(_RequestPacketData.check_sum != checksum)
    {
        _ResponsePacketData.error_code = ACK_ERROR;
    }
    else
    {
        if(!(_RequestPacketData.cmd & 0x8000))
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

void ComDataProcess()
{
//		ComRevData_t *p_rtu_cmd;
    static ModbusRTURevData_t *p_rtu_cmd;
	
//    if(QueueGet(&gComRevQueue, (void *)&pCom) == true)
//    {
//			_gcom_revdata_struct.data_size = pCom
//        if(pCom->data_type == COM_TYPE_UART_DATA)
//        {
//            /* 下位机串口通信协议解析 */
//            ProtoUnpackJudge(pCom);
//        } 
//    }
		      if(QueueGet(&gModbusMTURevQueue, (void *)&p_rtu_cmd) == true)
          {
							com_revupgradedata.com_revdata.data_size = p_rtu_cmd->data_size;
							com_revupgradedata.com_revdata.pData = p_rtu_cmd->pData;
							// 下位机串口通信协议解析 */
								ProtoUnpackJudge(&com_revupgradedata.com_revdata);							
					}
		
    if(_MPPTFirmwareUpgradeEnd)
    {
        if(tim_difftime_from_now(_MPPTFirmwareUpgradeEndTimes) > 1000) 
        {
//            __set_PRIMASK(1);   // 关闭总中断
//            NVIC_SystemReset();
        }    
    }

}