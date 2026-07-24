/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_upgrate.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-6-20      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "can_tx_rx_process.h"

//升级BCU自身

//#define UPGRATE_COM      0xB88B
//#define UPGRATE_CORRECT  0xB99B
//#define UPGRATE_ERR      0xB77B

#define UPGRATE_COM      0xF7B7
#define UPGRATE_CORRECT  0xF8B8
#define UPGRATE_ERR      0xF6B6

typedef enum
{
    EN_CAN_UPG_CAN0,
    EN_CAN_UPG_CAN1,
	EN_CAN_UPG_NUM
}EN_CAN_PROTO_UPG_IDX;

void CanUpgradeCAN0TxPrep(uint8_t idx);
void CanUpgradeCAN1TxPrep(uint8_t idx);
void CanUpgradeCAN0RxProcess(uint8_t *p_buf);
void CanUpgradeCAN1RxProcess(uint8_t *p_buf);


static CanProtoTxInfo_t _gCanProtoInfoUpgradeTx[] = 
{
	{0x10008BB4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, CanUpgradeCAN0TxPrep,   NULL}, 
    {0x10008BB4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, CanUpgradeCAN1TxPrep,   NULL}, 
};

CanProtoListTxInfo_t gCanProtoListInfoUpgradeTx = 
{
    .p_proto_info = _gCanProtoInfoUpgradeTx,
    .num = EN_CAN_UPG_NUM
};

void CanUpgrdeRxPolling(uint8_t idx);
static CanProtoRxInfo_t _gCanProtoInfoUpgradeRx[] = 
{
    {0x1000B48B, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, CanUpgradeCAN0RxProcess,   CanUpgrdeRxPolling}, 
    {0x1000B48B, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, CanUpgradeCAN1RxProcess,   CanUpgrdeRxPolling}, 
};


CanProtoListRxInfo_t gCanProtoListInfoUpgradeRx = 
{
    .p_proto_info = _gCanProtoInfoUpgradeRx,
    .num = EN_CAN_UPG_NUM
};




typedef enum
{
	EN_UPG_STATUS_IDLE,
	EN_UPG_STATUS_RECVING,
	EN_UPG_STATUS_WAIT_END,
	EN_UPG_STATUS_COMPLETE,
	EN_UPG_STATUS_ERROR,
    EN_UPG_STATUS_RESTART
}EN_UPG_STATUS;

#define EX_FLASH_UPGRADE_FLAG_START_ADDR    (0)
#define EX_FLASH_UPGRADE_START_ADDR    		(0x1000)

#define UPG_TIMEOUT		3000 //升级超时时间
static EN_UPG_STATUS _gUpgStatus;
static uint32_t _gUpgTimeCount;
__align(4) static uint8_t  _gRxTempBuf[1048];//用于缓存升级数据
static bool _gUpgErrorFlag = 0;//升级错误标志，用于调试
static uint8_t _gRecvAckSerialNum = 0;


/** 
* @brief    升级数据接收解析
* @param    p_buf:接收到的协议数据内容(8byte), can_idx：数据来自哪个CAN总线
* @return   NULL
* @note:
*/
void _CanUpgradeDataAnalysis(uint8_t *p_buf, EN_DEV_CAN can_idx)
{
	_gUpgTimeCount = tim_get_systime();
	
	static uint32_t buf_offset = 0;
	static uint32_t flash_offset = 0;
	static uint32_t checksum = 0;//文件和校验
	static uint32_t packet_num = 0;//小包计数
    static uint32_t file_size;
    static EN_DEV_CAN _cur_can_idx;//升级来自哪个CAN
	
	switch(_gUpgStatus)
	{
		case EN_UPG_STATUS_IDLE:
		{
			if(((p_buf[0] << 8) | p_buf[1]) == UPGRATE_COM)
			{
                _cur_can_idx = can_idx;
                if(can_idx == EN_DEV_CAN0)
                {
                    CanUpgradeCAN0TxPrep(0);
                }
                else if(can_idx == EN_DEV_CAN1)
                {
                    CanUpgradeCAN1TxPrep(0);
                }
				
				_gUpgStatus = EN_UPG_STATUS_RECVING;
				buf_offset = 0;
				flash_offset = 0;
				packet_num = 0;
				checksum = 0;
                _gRecvAckSerialNum = 0;
			}
			break;
		}
		case EN_UPG_STATUS_RECVING:
		{
            if(_cur_can_idx != can_idx)
            {
                break;
            }
			if(p_buf[0] == 0xFF)//小包结束报文
			{
				uint16_t buf_data_num;
				uint16_t crc_calc;
				buf_data_num = buf_offset + p_buf[3] - 7 - 7;//去除最后一包不足7，多余的部分,以及固件以外的其他验证数据(2B总包数+2B小包序号+2B总字节数+1总帧数)
				
				/*进行数据正确性校验*/
				crc_calc = CRC16(_gRxTempBuf, buf_data_num + 7);//+7是包含了验证数据
				if(crc_calc != ((uint16_t)p_buf[2] << 8) + p_buf[1])
				{
					_gUpgStatus = EN_UPG_STATUS_ERROR;
					if(can_idx == EN_DEV_CAN0)
                    {
                        CanUpgradeCAN0TxPrep(2);
                    }
                    else if(can_idx == EN_DEV_CAN1)
                    {
                        CanUpgradeCAN1TxPrep(2);
                    }
					break;
				}
				/*包验证校验*/
				if(packet_num != ((_gRxTempBuf[buf_data_num + 2] << 8) | (_gRxTempBuf[buf_data_num + 3])))//小包序号判断
				{
					_gUpgStatus = EN_UPG_STATUS_ERROR;
					if(can_idx == EN_DEV_CAN0)
                    {
                        CanUpgradeCAN0TxPrep(2);
                    }
                    else if(can_idx == EN_DEV_CAN1)
                    {
                        CanUpgradeCAN1TxPrep(2);
                    }
					break;
				}
                /*如果是第一包，获取文件大小*/
                if(packet_num == 0)
                {
                    if((_gRxTempBuf[0] == 0x5A) || (_gRxTempBuf[0] == 0xA5) || (_gRxTempBuf[0] == 0x3C) || (_gRxTempBuf[0] == 0xC3))
                    {
                        file_size = _gRxTempBuf[4] | (_gRxTempBuf[5] << 8) | (_gRxTempBuf[6] << 16) | (_gRxTempBuf[7] << 24);
                    }
                    else
                    {
                        _gUpgStatus = EN_UPG_STATUS_ERROR;
                        if(can_idx == EN_DEV_CAN0)
                        {
                            CanUpgradeCAN0TxPrep(2);
                        }
                        else if(can_idx == EN_DEV_CAN1)
                        {
                            CanUpgradeCAN1TxPrep(2);
                        }
                        break;
                    }
                }
                
				/*数据存储*/
				SpiFlashWriteBytesWithErase(EX_FLASH_UPGRADE_START_ADDR + flash_offset, _gRxTempBuf, buf_data_num);
				
				/*累加和校验*/  
                for(uint32_t i = 0; i < buf_data_num; i++)
                {
                    if(flash_offset + i + 1 <= file_size - 4)
                    {
                        checksum += _gRxTempBuf[i];
                    }
                }
                flash_offset += buf_data_num;
				packet_num++;
                
				/*判断是否是最后一包*/
				if(packet_num == ((_gRxTempBuf[buf_data_num + 0] << 8) | (_gRxTempBuf[buf_data_num + 1])))
				{
					uint32_t file_checksum;
					
					SpiFlashReadData(EX_FLASH_UPGRADE_START_ADDR + flash_offset - 4, (uint8_t *)&file_checksum, 4);
					if(file_checksum != checksum)//文件和校验不对
					{
						_gUpgStatus = EN_UPG_STATUS_ERROR;
						if(can_idx == EN_DEV_CAN0)
                        {
                            CanUpgradeCAN0TxPrep(2);
                        }
                        else if(can_idx == EN_DEV_CAN1)
                        {
                            CanUpgradeCAN1TxPrep(2);
                        }
						break;
					}
					else
					{
						/*更新FLASH标志位*/
						uint8_t temp[256];
						memset(temp, 0x0A, 256);
						SpiFlashWriteBytesWithErase(EX_FLASH_UPGRADE_FLAG_START_ADDR, temp, 256);
						_gUpgStatus = EN_UPG_STATUS_WAIT_END;
					}
				}
				buf_offset = 0;//用完清零
				if(can_idx == EN_DEV_CAN0)
                {
                    CanUpgradeCAN0TxPrep(1);
                }
                else if(can_idx == EN_DEV_CAN1)
                {
                    CanUpgradeCAN1TxPrep(1);
                }
                _gRecvAckSerialNum++;
			}
			else
			{
				if(buf_offset >= sizeof(_gRxTempBuf) - 7)//数据越界
				{
					_gUpgStatus = EN_UPG_STATUS_ERROR;
				}
				else
				{
					memcpy(&_gRxTempBuf[buf_offset], &p_buf[1], 7);
					buf_offset += 7;
				}
			}
			break;
		}
		case EN_UPG_STATUS_WAIT_END:
		{
            if(_cur_can_idx != can_idx)
            {
                break;
            }
			if(((p_buf[0] << 8) | p_buf[1]) == UPGRATE_CORRECT)
			{
				if(can_idx == EN_DEV_CAN0)
                {
                    CanUpgradeCAN0TxPrep(1);
                }
                else if(can_idx == EN_DEV_CAN1)
                {
                    CanUpgradeCAN1TxPrep(1);
                }
                _gUpgStatus = EN_UPG_STATUS_COMPLETE;
                _gUpgTimeCount = tim_get_systime();
			}
			break;
		}
		case EN_UPG_STATUS_ERROR:
		{
            if(_cur_can_idx != can_idx)
            {
                break;
            }
			if(((p_buf[0] << 8) | p_buf[1]) == UPGRATE_ERR)
			{
				if(can_idx == EN_DEV_CAN0)
                {
                    CanUpgradeCAN0TxPrep(2);
                }
                else if(can_idx == EN_DEV_CAN1)
                {
                    CanUpgradeCAN1TxPrep(2);
                }
			}
			break;
		}
		default:break;
	}
}

/** 
* @brief    CAN0口升级数据接收处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void CanUpgradeCAN0RxProcess(uint8_t *p_buf)
{
    _CanUpgradeDataAnalysis(p_buf, EN_DEV_CAN0);
}


/** 
* @brief    CAN1口升级数据接收处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void CanUpgradeCAN1RxProcess(uint8_t *p_buf)
{
    _CanUpgradeDataAnalysis(p_buf, EN_DEV_CAN1);
}


/** 
* @brief    升级响应数据准备
* @param    idx:响应类型，0对应UPGRATE_COM，1对应UPGRATE_CORRECT，2对应UPGRATE_ERR
* @param    proto_idx:对应哪一条协议，主要是区分CAN口
* @return   NULL
* @note:
*/
static void _CanUpgradeDataPrep(uint8_t idx, EN_CAN_PROTO_UPG_IDX proto_idx)
{
    uint8_t buf[8];
	uint16_t cmd_type;
	switch(idx)
	{
		case 0:
		{
			cmd_type = UPGRATE_COM;
			break;
		}
		case 1:
		{
			cmd_type = UPGRATE_CORRECT;
			break;
		}
		case 2:
		{
			cmd_type = UPGRATE_ERR;
			break;
		}
		default:return;
	}
	buf[0] = (cmd_type >> 8) & 0xFF;
	buf[1] = cmd_type & 0xFF;
	buf[2] = _gCanProtoInfoUpgradeTx[proto_idx].can_id.ext_pdu.sa;
    buf[3] = _gRecvAckSerialNum;
	memset(&buf[4], 0x00, 4);
	
    CanProtoTxIntoQueue(&_gCanProtoInfoUpgradeTx[proto_idx], buf);
}




/** 
* @brief    CAN0口升级响应数据准备
* @param    idx:响应类型，0对应UPGRATE_COM，1对应UPGRATE_CORRECT，2对应UPGRATE_ERR
* @return   NULL
* @note:
*/
void CanUpgradeCAN0TxPrep(uint8_t idx)
{
    _CanUpgradeDataPrep(idx, EN_CAN_UPG_CAN0);
}

/** 
* @brief    CAN1口升级响应数据准备
* @param    idx:响应类型，0对应UPGRATE_COM，1对应UPGRATE_CORRECT，2对应UPGRATE_ERR
* @return   NULL
* @note:
*/
void CanUpgradeCAN1TxPrep(uint8_t idx)
{
    _CanUpgradeDataPrep(idx, EN_CAN_UPG_CAN1);
}



uint8_t restart_test;
/** 
* @brief    升级过程轮询处理，用于做超时判断
* @param    idx:该模块下第几条协议(第几个CAN ID),该模块用不到
* @return   NULL
* @note:
*/
void CanUpgrdeRxPolling(uint8_t idx)
{
	if((uint32_t)(tim_get_systime() - _gUpgTimeCount) > UPG_TIMEOUT)
	{
		_gUpgStatus = EN_UPG_STATUS_IDLE;
	}
	switch(_gUpgStatus)
	{
		case EN_UPG_STATUS_COMPLETE:
		{
			_gUpgErrorFlag = 0;
			if((uint32_t)(tim_get_systime() - _gUpgTimeCount) > UPG_TIMEOUT - 1000)
			{
                _gUpgStatus = EN_UPG_STATUS_RESTART;
            }
			break;
		}
		case EN_UPG_STATUS_ERROR:
		{
			_gUpgErrorFlag = 1;
			break;
		}
        case EN_UPG_STATUS_RESTART:
        {
            /*准备重启*/
            restart_test = 0;
            __set_FAULTMASK(1);
            NVIC_SystemReset();
            break;
        }
		default:return;
	}
}











