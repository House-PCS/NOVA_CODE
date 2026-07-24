
#include "dev_rs485.h"


#define COMM485_RX_SIZE       512

static uint8_t _Comm485RxBuf[COMM485_RX_SIZE];//DMA接收提前把BUF定义好

DevComm485_t gDevComm485;


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
    
    /*包头判断*/
    if(p_data[0] != 0x4e || p_data[1] != 0x4f || p_data[2] != 0x56 || p_data[3] != 0x41)
    {
        return;
    }
    
    gDevComm485.data_received = true;
    gDevComm485.p_data = p_data;
    gDevComm485.data_len = len;
    
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
  * @brief   发送函数
  * @param   p_data:数据指针
  * @param   len:数据长度
  * @return  None
  * @note    None
  */
void Comm485Send(uint8_t *p_data, uint16_t len)
{
    ModbusHostUartHandle.pTxBuf = p_data;
    ModbusHostUartHandle.tx_size = len;
    gDevComm485.send(&ModbusHostUartHandle);
}



/**
  * @brief   Comm初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevComm485Init()
{
    ModbusHostUartHandle.pRxBuf = _Comm485RxBuf;
    ModbusHostUartHandle.rx_size =COMM485_RX_SIZE;
    ModbusHostUartHandle.irq_rx_process = _IrqRxProcess;
    
    gDevComm485.send = UartSend;
    
    ModbusHostBspInit();
}






