/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_modbus_h.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-8-8
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-8-8      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_modbus_h.h"

#define MODBUS_RTU_REV_QUEUE_DEPTH   2
#define MODBUS_RTU_RX_SIZE       512

static const uint8_t _CrcHi[] = {
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
static const uint8_t _CrcLo[] = {
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

static uint8_t _ModbusRxBuf[MODBUS_RTU_RX_SIZE];//DMA接收提前把BUF定义好
static uint8_t _RevData[MODBUS_RTU_REV_QUEUE_DEPTH][MODBUS_RTU_RX_SIZE];//从串口接收缓存拷贝到该数组

Queue_t	gModbusMTURevQueue;
DevModbusHost_t gDevModbus;

static uint16_t _CheckSum(uint8_t *p, uint16_t size)
{
    uint16_t i;
    uint16_t ret = 0;
    for(i = 0; i < size; i++)
    {
        ret += p[i];
    }
    return ret;
}

static uint16_t _CrcModbus(uint8_t *p, uint16_t len)
{
    uint8_t ucCRCHi = 0xFF; /* 高CRC字节初始化 */
	uint8_t ucCRCLo = 0xFF; /* 低CRC 字节初始化 */
	uint16_t usIndex;  /* CRC循环中的索引 */
    while (len--)
    {
		usIndex = ucCRCLo ^ *p++; /* 计算CRC */
		ucCRCLo = ucCRCHi ^ _CrcHi[usIndex];
		ucCRCHi = _CrcLo[usIndex];
    }
    return ((uint16_t)ucCRCHi << 8 | ucCRCLo);
}


/**
  * @brief   判断CRC校验是否正确
  * @param   p: data pointer
  * @param   len: data len 
  * @return  true or false
  * @note    None
  */
static bool _CrcJudge(uint8_t *p, uint16_t len)
{
    uint16_t calc_crc, act_crc;
    
    act_crc = (p[len - 1] << 8) | (p[len - 2]);
    calc_crc = _CrcModbus(p, len - 2);
    if(act_crc == calc_crc)
    {
        return true;
    }
    return false;
}


/**
  * @brief   串口接收数据解析
  * @param   len：数据长度
  * @param   p_data:数据指针
  * @return  None
  * @note    None
  */
static void _RevDataAnalysis(uint8_t *p_data, uint16_t len)
{
    static uint16_t rev_index = 0;
    ModbusRTURevData_t  rev_data;

	static int8_t debug_flag = 0;
	if (p_data[0] == 0xDE && p_data[1] == 0xED && len == 2)
	{
		debug_flag = 1;
	}
	else if (p_data[0] == 0xDE && p_data[1] == 0xEF && len == 2)
	{
		debug_flag = 0;
	}
	else if (debug_flag)
	{

	}
	else
	{
		if(len < 5)/*不完整的Modbus数据*/
		{
			return;
		}

		if (p_data[0] == 0x5A || p_data[0] == 0xA5)
		{
			// Nova PCS 使用 累加和校验
			uint16_t check_sum = _CheckSum(p_data, len-sizeof(uint16_t));
			if (check_sum != *(uint16_t *)&p_data[len-sizeof(uint16_t)])
			{
				return;
			}
		}
		else 
		{
			if(_CrcJudge(p_data, len) == false)
			{
				return;
			}
		}
	}
    
    memcpy(_RevData[rev_index], p_data, len);
    rev_data.data_size = len;
    rev_data.p_data = _RevData[rev_index];
    if(QueuePut(&gModbusMTURevQueue, &rev_data) == false)
    {
        return;
    }
    rev_index++;
    rev_index %= MODBUS_RTU_REV_QUEUE_DEPTH;
    
}


/**
  * @brief   中断处理函数
  * @param   None
  * @return  None
  * @note    None
  */
static void _IrqRxProcess()
{
    uint16_t rev_size = 0;
    
	/*DMA方式，空闲中断机制*/
	UartDMARxDisable(&ModbusHostUartHandle);
	rev_size = UartDMARxDataLength(&ModbusHostUartHandle);	
    _RevDataAnalysis(ModbusHostUartHandle.pRxBuf, rev_size);      
	UartDMARxEnable(&ModbusHostUartHandle);
}

/**
  * @brief   DMA中断处理函数
  * @param   None
  * @return  None
  * @note    None
  */
static void _DMATxCompleteIrqProcess()
{
    gDevModbus.flag_send_complete = true;
    
}

/**
  * @brief   Modbus发送函数
  * @param   p_data:数据指针
  * @param   len:数据长度
  * @return  None
  * @note    None
  */
void ModbusRTUSend(uint8_t *p_data, uint16_t len)
{
    uint16_t crc;
    
    ModbusHostUartHandle.pTxBuf = p_data;
    crc = _CrcModbus(p_data, len);
    ModbusHostUartHandle.pTxBuf[len] = crc & 0xFF;
    ModbusHostUartHandle.pTxBuf[len + 1] = (crc >> 8) & 0xFF;
    ModbusHostUartHandle.tx_size = len + 2;
    
    if(gDevModbus.dir_manual == true)
    {
        GpioBitSet(&ModbusHostCtlIOHandle);
        gDevModbus.flag_send_complete = false;
        gDevModbus.send_time_out = tim_get_systime();
        gDevModbus.send_max_time = 1000 * (8 + 4) * ModbusHostUartHandle.tx_size / ModbusHostUartHandle.uart_cfg->baud_rate + 2;
    }
    
    gDevModbus.send(&ModbusHostUartHandle);
}

void ModbusRTUSendWithoutCheckSum(uint8_t *p_data, uint16_t len)
{
    uint16_t crc;
    
    ModbusHostUartHandle.pTxBuf = p_data;
    ModbusHostUartHandle.tx_size = len;
    
    if(gDevModbus.dir_manual == true)
    {
        GpioBitSet(&ModbusHostCtlIOHandle);
        gDevModbus.flag_send_complete = false;
        gDevModbus.send_time_out = tim_get_systime();
        gDevModbus.send_max_time = 1000 * (8 + 4) * ModbusHostUartHandle.tx_size / ModbusHostUartHandle.uart_cfg->baud_rate + 2;
    }
    
    gDevModbus.send(&ModbusHostUartHandle);
}

void ModbusRTUSendData(uint8_t *p_data, uint16_t len)
{
    uint16_t crc;
    
    ModbusHostUartHandle.pTxBuf = p_data;
    ModbusHostUartHandle.tx_size = len;
    
    if(gDevModbus.dir_manual == true)
    {
        GpioBitSet(&ModbusHostCtlIOHandle);
        gDevModbus.flag_send_complete = false;
        gDevModbus.send_time_out = tim_get_systime();
        gDevModbus.send_max_time = 1000 * (8 + 4) * ModbusHostUartHandle.tx_size / ModbusHostUartHandle.uart_cfg->baud_rate + 2;
    }
    
    gDevModbus.send(&ModbusHostUartHandle);
}




/**
  * @brief   modbus主模式初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevModbusHostRTUInit(uint32_t baud_rate)
{
    QueueInit(&gModbusMTURevQueue, MODBUS_RTU_REV_QUEUE_DEPTH, sizeof(ModbusRTURevData_t));
    
    ModbusHostUartHandle.pRxBuf = _ModbusRxBuf;
    ModbusHostUartHandle.rx_size =MODBUS_RTU_RX_SIZE;
    ModbusHostUartHandle.irq_rx_process = _IrqRxProcess;
    ModbusHostUartHandle.dma_tx_complete_process = _DMATxCompleteIrqProcess;
    
    gDevModbus.dir_manual = false;
    gDevModbus.flag_send_complete = false;
    gDevModbus.send = UartSend;
    
    ModbusHostBspInit(baud_rate);
}

/**
  * @brief   modbus主模式反初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevModbusHostRTUDeInit()
{
    UartDeInit(&ModbusHostUartHandle);
}


/**
  * @brief   modbus主模式重新初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevModbusHostRTUReInit(uint32_t baud_rate)
{
    ModbusHostBspInit(baud_rate);
}


/*大轮询调用*/
void DevModbusDirCtlProcess()
{
    if(gDevModbus.flag_send_complete == true)
    {
        if(UartSendCompleteFlag(&ModbusHostUartHandle) == true)/*串口寄存器已经发送完成，为空*/
        {
            GpioBitReset(&ModbusHostCtlIOHandle);
            gDevModbus.flag_send_complete = false;
        }
        else
        {
            if((uint32_t)(tim_get_systime() - gDevModbus.send_time_out) > gDevModbus.send_max_time)/*发送异常，恢复接收状态*/
            {
                GpioBitReset(&ModbusHostCtlIOHandle);
            }
        }
    }
    else/*处理超时*/
    {
        if(GpioBitGet(&ModbusHostCtlIOHandle) != 0)
        {
            if((uint32_t)(tim_get_systime() - gDevModbus.send_time_out) > gDevModbus.send_max_time)/*发送超时，恢复接收状态*/
            {
                GpioBitReset(&ModbusHostCtlIOHandle);
            }
        }
    }
}





