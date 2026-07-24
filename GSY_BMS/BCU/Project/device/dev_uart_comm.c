
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_modbus_s.c
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
  * <tr><td>2023-6-9      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "dev_uart_comm.h"


#define DEV_UART_REV_QUEUE_DEPTH    2
#define DEV_UART_RX_SIZE            512
#define DEV_UART_TX_SIZE            512

static uint8_t  _gDevUartCommSendCompleteFlag[EN_DEV_UART_TYPE_NUM] = {0};/*1表示发送完成*/
static uint32_t _gDevUartCommSendTimeout[EN_DEV_UART_TYPE_NUM] = {0};/*用于触发DMA发送后，长时间未产生DMA完成中断的情况*/
static uint32_t _gDevUartCommSendMaxTime[EN_DEV_UART_TYPE_NUM] = {0};/*DMA最大发送时间，由每次发送的数据量决定，增加一些冗余时间*/


/*说明：针对发送是否完成，可采用DMA完成中断+串口发送寄存器空的状态判断，也可在发送前作发送时间预计算，轮询时间到了后恢复接收态，前者时间控制更精准些*/

//

static uint8_t _UartTxBuf[EN_DEV_UART_TYPE_NUM][DEV_UART_TX_SIZE];
static uint8_t _UartRxBuf[EN_DEV_UART_TYPE_NUM][DEV_UART_RX_SIZE];//DMA接收提前把BUF定义好
    
static DevUartRevData_t _gDevUartData[EN_DEV_UART_TYPE_NUM][DEV_UART_REV_QUEUE_DEPTH];
static uint8_t _RevData[EN_DEV_UART_TYPE_NUM][DEV_UART_REV_QUEUE_DEPTH][DEV_UART_RX_SIZE];

Queue_t	gDevUartRevQueue[EN_DEV_UART_TYPE_NUM];

static void _Dev4851IrqRxProcess(void);
static void _Dev4852IrqRxProcess(void);
static void _DevTTLIrqRxProcess(void);

static void _Dev4851DMATxCompleteIrqProcess(void);
static void _Dev4852DMATxCompleteIrqProcess(void);

static Uart_HandleType *_pUartHandle[EN_DEV_UART_TYPE_NUM];
static Gpio_HandleType *_pGpioHandle[EN_DEV_UART_TYPE_NUM];

/** 
* @brief    串口初始化
* @param    none
* @param    none
* @return   none
* @note:
*/
void DevUartCommInit()
{
    for(uint8_t k = 0; k < EN_DEV_UART_TYPE_NUM; k++)
    {
        QueueInit(&gDevUartRevQueue[k], DEV_UART_REV_QUEUE_DEPTH, sizeof(DevUartRevData_t));
        for(uint8_t i = 0; i < DEV_UART_REV_QUEUE_DEPTH; i++)
        {
            _gDevUartData[k][i].pData = _RevData[k][i];
        }
    }
    gUart1ModbusHandle.pTxBuf = _UartTxBuf[EN_DEV_UART_TYPE_485_1];
    gUart1ModbusHandle.pRxBuf = _UartRxBuf[EN_DEV_UART_TYPE_485_1];
    gUart1ModbusHandle.rx_size = DEV_UART_RX_SIZE;
    gUart1ModbusHandle.irq_rx_process = _Dev4851IrqRxProcess;
    gUart1ModbusHandle.dma_tx_complete_process = _Dev4851DMATxCompleteIrqProcess;
    
    gUart2ModbusHandle.pTxBuf = _UartTxBuf[EN_DEV_UART_TYPE_485_2];
    gUart2ModbusHandle.pRxBuf = _UartRxBuf[EN_DEV_UART_TYPE_485_2];
    gUart2ModbusHandle.rx_size = DEV_UART_RX_SIZE;
    gUart2ModbusHandle.irq_rx_process = _Dev4852IrqRxProcess;
    gUart2ModbusHandle.dma_tx_complete_process = _Dev4852DMATxCompleteIrqProcess;
    
    gUart0TTLHandle.pTxBuf = _UartTxBuf[EN_DEV_UART_TYPE_TTL];
    gUart0TTLHandle.pRxBuf = _UartRxBuf[EN_DEV_UART_TYPE_TTL];
    gUart0TTLHandle.rx_size = DEV_UART_RX_SIZE;
    gUart0TTLHandle.irq_rx_process = _DevTTLIrqRxProcess;
    
    _pUartHandle[EN_DEV_UART_TYPE_485_1] = &gUart1ModbusHandle;
    _pUartHandle[EN_DEV_UART_TYPE_485_2] = &gUart2ModbusHandle;
    _pUartHandle[EN_DEV_UART_TYPE_TTL] = &gUart0TTLHandle;
    
    _pGpioHandle[EN_DEV_UART_TYPE_485_1] = &gGpioDE1ModbusHandle;
    _pGpioHandle[EN_DEV_UART_TYPE_485_2] = &gGpioDE2ModbusHandle;
    
    UartCommBspInit();
       
}


/** 
* @brief    接收数据解析
* @param    接受数据长度
* @param    端口
* @return   none
* @note:
*/
static void _RevDataAnalysis(uint16_t len, EN_DEV_UART_TYPE type)
{
    static uint16_t rev_index[EN_DEV_UART_TYPE_NUM] = {0};
    uint8_t *p = _pUartHandle[type]->pRxBuf;
    
    _gDevUartData[type][rev_index[type]].data_size = len;
    memcpy(_gDevUartData[type][rev_index[type]].pData, p, len);
    
    if(QueuePut(&gDevUartRevQueue[type], &_gDevUartData[type][rev_index[type]]) == false)
    {
        return;
    }
    rev_index[type]++;
    rev_index[type] %= DEV_UART_REV_QUEUE_DEPTH;
    
}

/** 
* @brief    中断接收 
* @param    端口
* @return   none
* @note:
*/
static void _IrqRxProcess(EN_DEV_UART_TYPE type)
{
    /*DMA方式，空闲中断机制*/
    UartDMARxDisable(_pUartHandle[type]);
    _RevDataAnalysis(UartDMARxDataLength(_pUartHandle[type]), type);      
    UartDMARxEnable(_pUartHandle[type]);
}

/** 
* @brief    不同端口调用不同中断接收函数
* @param    none
* @return   none
* @note:
*/
static void _Dev4851IrqRxProcess()
{
    _IrqRxProcess(EN_DEV_UART_TYPE_485_1);
}

static void _Dev4852IrqRxProcess()
{
    _IrqRxProcess(EN_DEV_UART_TYPE_485_2);
}

static void _DevTTLIrqRxProcess()
{
    _IrqRxProcess(EN_DEV_UART_TYPE_TTL);
}

/** 
* @brief  485串口发送
* @param  端口
* @param  发送数据地址
* @param  发送数据长度
* @return  none
* @note:
*/
void DevUartCommSend(EN_DEV_UART_TYPE type, uint8_t *p, uint16_t size)
{
    _pUartHandle[type]->pTxBuf = p;
    _pUartHandle[type]->tx_size = size;
    if(type != EN_DEV_UART_TYPE_TTL)
    {
        GpioBitSet(_pGpioHandle[type]);
    }
    
    _gDevUartCommSendCompleteFlag[type] = 0;
    _gDevUartCommSendTimeout[type] = tim_get_systime();
    _gDevUartCommSendMaxTime[type] = 1000 * (8 + 4) * _pUartHandle[type]->tx_size / _pUartHandle[type]->uart_cfg->baud_rate + 2;//单位ms,8表示数据位时长，4表示起始+停止+GAP时长, +2表示增加一些冗余

    UartSend(_pUartHandle[type]);  
}

/** 
* @brief    发送完成中断
* @param    none
* @return   none
* @note:
*/
static void _Dev4851DMATxCompleteIrqProcess()
{
    _gDevUartCommSendCompleteFlag[EN_DEV_UART_TYPE_485_1] = 1;
    if(UartSendCompleteFlag(_pUartHandle[0]) == true)
    {
        UartClearCompleteFlag(_pUartHandle[0]);
    }
    
}

static void _Dev4852DMATxCompleteIrqProcess()
{
    _gDevUartCommSendCompleteFlag[EN_DEV_UART_TYPE_485_2] = 1;
    if(UartSendCompleteFlag(_pUartHandle[1]) == true)
    {
        UartClearCompleteFlag(_pUartHandle[1]);
    }
}

/** 
* @brief    发送完成判断
* @param    type:哪一路串口
* @return   true表示发送完成，false表示发送未完成
* @note:
*/
bool Dev485SendCompleteJudge(EN_DEV_UART_TYPE type)
{
    if(GpioBitGet(_pGpioHandle[type]) != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}




/*大轮询调用*/
void DevUartCommDirCtlProcess()
{
    for(EN_DEV_UART_TYPE i = EN_DEV_UART_TYPE_485_1; i < EN_DEV_UART_TYPE_NUM; i++)
    {
        if(i == EN_DEV_UART_TYPE_TTL)
        {
            continue;
        }
        if(_gDevUartCommSendCompleteFlag[i] == 1)
        {
            if(UartSendCompleteFlag(_pUartHandle[i]) == true)/*串口寄存器已经发送完成，为空*/
            {
//                soft_delayms(1);
                GpioBitReset(_pGpioHandle[i]);
                _gDevUartCommSendCompleteFlag[i] = 0;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - _gDevUartCommSendTimeout[i]) > _gDevUartCommSendMaxTime[i])/*发送异常，恢复接收状态*/
                {
                    GpioBitReset(_pGpioHandle[i]);
                }
            }
        }
        else/*处理超时*/
        {
            if(GpioBitGet(_pGpioHandle[i]) != 0)
            {
                if((uint32_t)(tim_get_systime() - _gDevUartCommSendTimeout[i]) > _gDevUartCommSendMaxTime[i])/*发送超时，恢复接收状态*/
                {
                    GpioBitReset(_pGpioHandle[i]);
                }
            }
        }
    }   
}





