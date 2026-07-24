/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_host.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-12-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-12-16    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "modbus_host.h"

#define LOG_TAG     "modbus_host"
#define  Uart_modbus_Handle  Uart_fc41d_Handle
#define TIMEOUT		100		/* 接收命令超时时间, 单位ms */
#define NUM			1		/* 循环发送次数 */

static uint8_t          _RevDataBufIndex = 0;
static uint8_t          _RevData[MODBUS_REV_QUEUE_DEPTH][_MODBUS_RX_SIZE];
static ModbusRevData_t	_gModbusRevData[MODBUS_REV_QUEUE_DEPTH];
static MODH_T   _gModH;
static EN_MODBUS_STATE_TYPE _CurrentModbusStateType = MODBUS_STATE_TYPE_IDLE;



    // CRC高字节
static const uint8_t s_CRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
// CRC 低字节
const uint8_t s_CRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

Queue_t	gModbusRevQueue;

uint16_t mudbus_test_num = 0x55AA;
/*******************************************************************************
  * @FunctionName: BEBufToUint16
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:08:24+0800
  * @Purpose:      大端数据转u16
  * @param:        _pBuf              
  * @return:
*******************************************************************************/
uint16_t BEBufToUint16(uint8_t *_pBuf)
{
    return (((uint16_t)_pBuf[0] << 8) | _pBuf[1]);
}
/*******************************************************************************
  * @FunctionName: CRC16_Modbus
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:09:58+0800
  * @Purpose:      
  * @param:        _pBuf              
  * @param:        _usLen             
  * @return:
*******************************************************************************/
uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen)
{
	uint8_t ucCRCHi = 0xFF; /* 高CRC字节初始化 */
	uint8_t ucCRCLo = 0xFF; /* 低CRC 字节初始化 */
	uint16_t usIndex;  /* CRC循环中的索引 */

    while (_usLen--)
    {
		usIndex = ucCRCHi ^ *_pBuf++; /* 计算CRC */
		ucCRCHi = ucCRCLo ^ s_CRCHi[usIndex];
		ucCRCLo = s_CRCLo[usIndex];
    }
    return ((uint16_t)ucCRCHi << 8 | ucCRCLo);

}

/*******************************************************************************
  * @FunctionName: RevDataAnalysis
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:11:45+0800
  * @Purpose:      
  * @param:                           
  * @return:
*******************************************************************************/
static void RevDataAnalysis()
{
    uint8_t *p = Uart_modbus_Handle.pRxBuf;
    
    _gModbusRevData[_RevDataBufIndex].data_size = _gModH.RxCount;
    memcpy(_gModbusRevData[_RevDataBufIndex].pData, p, _gModH.RxCount);
    if(QueuePut(&gModbusRevQueue, &_gModbusRevData[_RevDataBufIndex]) == false)
    {
        return;
    }
    _RevDataBufIndex++;
    _RevDataBufIndex %= MODBUS_REV_QUEUE_DEPTH;
}

/*******************************************************************************
  * @FunctionName: IrqRxProcess
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:12:19+0800
  * @Purpose:      
  * @param:                           
  * @return:
*******************************************************************************/
static void IrqRxProcess()
{
	UartDMARxDisable(&Uart_modbus_Handle);
	_gModH.RxCount = UartDMARxDataLength(&Uart_modbus_Handle);
	
    RevDataAnalysis();
      
	UartDMARxEnable(&Uart_modbus_Handle);
}

/*******************************************************************************
  * @FunctionName: ModbusInit
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:12:22+0800
  * @Purpose:      
  * @param:        void               
  * @return:
*******************************************************************************/
bool ModHInit(void)
{
    bool ret = true;
    
	QueueInit(&gModbusRevQueue, MODBUS_REV_QUEUE_DEPTH, sizeof(ModbusRevData_t));
	for(uint8_t i = 0; i < MODBUS_REV_QUEUE_DEPTH; i++)
	{
		_gModbusRevData[i].pData = _RevData[i];
	}
	
	Uart_fc41d_Handle.pRxBuf = _gModH.RxBuf;
	Uart_fc41d_Handle.rx_size = _MODBUS_RX_SIZE;
    Uart_fc41d_Handle.irq_rx_process = IrqRxProcess;
    
    UartDMARxDisable(&Uart_modbus_Handle);
    UartDMARxEnable(&Uart_modbus_Handle);
    
    return ret;
}


static void MODH_SendAckWithCRC(void)
{
	uint16_t crc;
	
	crc = CRC16_Modbus(_gModH.TxBuf, _gModH.TxCount);
	_gModH.TxBuf[_gModH.TxCount++] = crc >> 8;
	_gModH.TxBuf[_gModH.TxCount++] = crc;	
    
    Uart_modbus_Handle.pTxBuf = _gModH.TxBuf;
	Uart_modbus_Handle.tx_size = _gModH.TxCount;	
	UartSend(&Uart_modbus_Handle);
	
}

void MODH_Send03H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	_gModH.TxCount = 0;
	_gModH.TxBuf[_gModH.TxCount++] = _addr;		/* 从站地址 */
	_gModH.TxBuf[_gModH.TxCount++] = 0x03;		/* 功能码 */	
	_gModH.TxBuf[_gModH.TxCount++] = _reg >> 8;	/* 寄存器编号 高字节 */
	_gModH.TxBuf[_gModH.TxCount++] = _reg;		/* 寄存器编号 低字节 */
	_gModH.TxBuf[_gModH.TxCount++] = _num >> 8;	/* 寄存器个数 高字节 */
	_gModH.TxBuf[_gModH.TxCount++] = _num;		/* 寄存器个数 低字节 */
	
	MODH_SendAckWithCRC();		/* 发送数据，自动加CRC */
	_gModH.fAck03H = 0;		/* 清接收标志 */
	_gModH.RegNum = _num;		/* 寄存器个数 */
	_gModH.Reg03H = _reg;		/* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */	
}

void MODH_Send10H(uint8_t _addr, uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
	uint16_t i;
	
	_gModH.TxCount = 0;
	_gModH.TxBuf[_gModH.TxCount++] = _addr;		/* 从站地址 */
	_gModH.TxBuf[_gModH.TxCount++] = 0x10;		/* 从站地址 */	
	_gModH.TxBuf[_gModH.TxCount++] = _reg >> 8;	/* 寄存器编号 高字节 */
	_gModH.TxBuf[_gModH.TxCount++] = _reg;		/* 寄存器编号 低字节 */
	_gModH.TxBuf[_gModH.TxCount++] = _num >> 8;	/* 寄存器个数 高字节 */
	_gModH.TxBuf[_gModH.TxCount++] = _num;		/* 寄存器个数 低字节 */
	_gModH.TxBuf[_gModH.TxCount++] = 2 * _num;	/* 数据字节数 */
	
	for (i = 0; i < 2 * _num; i++)
	{
		if (_gModH.TxCount > _MODBUS_RX_SIZE - 3)
		{
			return;		/* 数据超过缓冲区超度，直接丢弃不发送 */
		}
		_gModH.TxBuf[_gModH.TxCount++] = _buf[i];		/* 后面的数据长度 */
	}
	
	MODH_SendAckWithCRC();	/* 发送数据，自动加CRC */
    _gModH.fAck10H = 0;     /* 清接收标志 */
}
/*******************************************************************************
  * @FunctionName: MODH_Read_03H
  * @Author:       xingxiao
  * @DateTime:     2022年12月16日T16:56:10+0800
  * @Purpose:      
  * @param:        void               
*******************************************************************************/
void MODH_Read_03H(ModbusRevData_t *p)
{
	uint8_t bytes;
	
	if (p->data_size > 0)
	{
		bytes = p->pData[2];	/* 数据长度 字节数 */		  
        _gModH.fAck03H = 1;        
		switch (_gModH.Reg03H)
		{
			case 2001:
                LOG_DEBUG(LOG_TAG, "MODH Recive data:%04X\r\n", BEBufToUint16(&p->pData[3]));                               
				break;
		}
	}
}

void MODH_Read_10H(ModbusRevData_t *p)
{
	/*
		10H指令的应答:
			从机地址                11
			功能码                  10
			寄存器起始地址高字节	00
			寄存器起始地址低字节    01
			寄存器数量高字节        00
			寄存器数量低字节        02
			CRC校验高字节           12
			CRC校验低字节           98
	*/
	if (p->data_size > 0)
	{
		if (p->pData[0] == SLAVEADDR)		
		{
			_gModH.fAck10H = 1;		/* 接收到应答 */
		}
	}
}

void MODHAnalyzeApp(ModbusRevData_t *p)
{
	uint16_t crc1;
    
    /* 计算CRC校验和 */
    crc1 = CRC16_Modbus(p->pData, (p->data_size));
    if (crc1 != 0)
    {
        return;
    }
    
    switch (p->pData[1])				/* 第2个字节 功能码 */
	{
		case 0x03:						/* 读取保持寄存器 */
        {
            MODH_Read_03H(p);
			break;
        }
		case 0x10:						/* 写多个保存寄存器*/
        {
            MODH_Read_10H(p);
            break;
        }
		default:
			break;
	}
    
}


void ModHProcess(void)
{
	ModbusRevData_t *p;
    static uint32_t timeout = 0;
    
	if(QueueGet(&gModbusRevQueue, (void *)&p) == true)
	{
        /* 分析应用层协议 */
        MODHAnalyzeApp(p);	
	} 

    switch(_CurrentModbusStateType)
    {
        case MODBUS_STATE_TYPE_IDLE:
        {
            timeout = tim_get_systime();
            break;
        }
        case MODBUS_STATE_TYPE_WAIT_ACK03H:
        {
            if(_gModH.fAck03H)
            {
                _CurrentModbusStateType = MODBUS_STATE_TYPE_IDLE;
                LOG_DEBUG(LOG_TAG, "Write 03 ok!\r\n");
            }
            else if(tim_difftime_from_now(timeout) > TIMEOUT)
            {
                _CurrentModbusStateType = MODBUS_STATE_TYPE_IDLE;
                LOG_ERROR(LOG_TAG, "Write 03 error!");
            }
            break;
        }
        case MODBUS_STATE_TYPE_WAIT_ACK10H:
        {
            if(_gModH.fAck10H)
            {
                _CurrentModbusStateType = MODBUS_STATE_TYPE_IDLE;
                LOG_DEBUG(LOG_TAG, "Write 10 ok!\r\n");
            }
            else if(tim_difftime_from_now(timeout) > TIMEOUT)
            {
                _CurrentModbusStateType = MODBUS_STATE_TYPE_IDLE;
                LOG_ERROR(LOG_TAG, "Write 10 error!");
            }
            break;
        }
        default:
            break;
        
    }
    
}

uint8_t MODH_ReadParam_03H(uint8_t _slaveaddr, uint16_t _reg, uint16_t _num)
{
	uint8_t i;
	
	for (i = 0; i < NUM; i++)
	{
		MODH_Send03H (_slaveaddr, _reg, _num);
        _CurrentModbusStateType = MODBUS_STATE_TYPE_WAIT_ACK03H;
	}

    return 1;
}

uint8_t MODH_WriteParam_10H(uint8_t _slaveaddr, uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
	uint8_t i;
	
	for (i = 0; i < NUM; i++)
	{	
		MODH_Send10H(_slaveaddr, _reg, _num, _buf);
		_CurrentModbusStateType = MODBUS_STATE_TYPE_WAIT_ACK10H;

	}
    
    return 1;
}

static void send_10(uint16_t data)
{
    uint8_t arr_data[2];
    arr_data[0] =  data >> 8;
    arr_data[1] =  (uint8_t)(data & 0x00FF);
    MODH_WriteParam_10H(1, 2001, 1, arr_data);
}

FINSH_CMD_EXPORT(MODH_ReadParam_03H, uint8_t _slaveaddr-uint16_t _reg-uint16_t _num);
FINSH_CMD_EXPORT(send_10, uint16_t _num);


