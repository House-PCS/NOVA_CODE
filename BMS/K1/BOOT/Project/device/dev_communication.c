
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
  
#include "dev_communication.h"
#define LOG_TAG "dev_communication"         

/*****************************************宏***************************************/
#define _UART_REV_QUEUE_DEPTH           (2)
#define _UART_RX_SIZE                   (2100)

#define COMMON_PACK_SEND_ADDR           (0x00)            /* 主控 */



/**************************************全局变量*************************************/
typedef struct
{
    Uart_HandleType* p_ModbusUart               ;   /*串口指针*/
    Gpio_HandleType* p_Modbus485En              ;   /*使能引脚*/
    
    uint8_t          n_ModbusSendCompleteFlag   ;   /*1表示发送完成*/
    uint32_t         n_ModbusSendTimeout        ;   /*用于触发DMA发送后，长时间未产生DMA完成中断的情况*/
    uint32_t         n_ModbusSendMaxTime        ;   /*DMA最大发送时间，由每次发送的数据量决定，增加一些冗余时间*/
}_ModbusSet_t;

_ModbusSet_t _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_NUM];   /*主机与从机的modbus设置，主机与从机各占一个结构体*/

DevUartPackInfo_t gUartPackInfo;

uint8_t gBMSSleepFlag                    = 0;

/*说明：针对发送是否完成，可采用DMA完成中断+串口发送寄存器空的状态判断，也可在发送前作发送时间预计算，轮询时间到了后恢复接收态，前者时间控制更精准些*/

static ModbusRTURevData_t   _gModbusData[_UART_REV_QUEUE_DEPTH];
static uint8_t              _RevData[_UART_REV_QUEUE_DEPTH][_UART_RX_SIZE];

static uint8_t              _Uart1RxBuf[_UART_RX_SIZE];//DMA接收提前把BUF定义好
static uint8_t              _Uart2RxBuf[_UART_RX_SIZE];//DMA接收提前把BUF定义好

Queue_t	gModbusRTURevQueue;             /*MODBUS从机队列*/
Queue_t	gModbusHostQueue;               /*MODBUS主机队列*/
Queue_t	gUpCommonPackQueue;             /*普通协议包队列（与上一级相连）*/
Queue_t	gDownCommonPackQueue;           /*普通协议包队列（与下一级相连）*/
Queue_t	gModbusRTUPackForwardQueue;     /*MODBUS数据包转发队列（不是自己的包，向下转发）*/

/******************************局部函数****************************************/
static void _Uart1IrqRxProcess(void);                                       /*串口中断处理*/
static void _Uart2IrqRxProcess(void);                                       /*串口中断处理*/
static void _Uart1DMATxCompleteIrqProcess(void);                            /*DMA发送完成中断*/
static void _Uart2DMATxCompleteIrqProcess(void);                            /*DMA发送完成中断*/
static void _RevDataAnalysis(uint16_t len, Uart_HandleType* uarthandle);    /*数据解析*/


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
//




/**
 * @brief  : CRC校验计算
 * @para   : p    : 数据地址
           : size : 数据长度
 * @note   : None
 * @return : CRC校验结果
 */
static uint16_t _CrcModbus(uint8_t *p, uint16_t len)
{
    uint8_t ucCRCHi = 0xFF;         /* 高CRC字节初始化 */
	uint8_t ucCRCLo = 0xFF;         /* 低CRC 字节初始化 */
	uint16_t usIndex;               /* CRC循环中的索引 */
    while (len--)
    {
		usIndex = ucCRCLo ^ *p++;   /* 计算CRC */
		ucCRCLo = ucCRCHi ^ _CrcHi[usIndex];
		ucCRCHi = _CrcLo[usIndex];
    }
    return ((uint16_t)ucCRCHi << 8 | ucCRCLo);
}


/**
 * @brief  : CRC校验判断
 * @para   : p    : 数据地址
           : size : 数据长度
 * @note   : None
 * @return : None
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
 * @brief  : modus及串口初始化
 * @para   : None
 * @note   : None
 * @return : None
 */
void DevModbusRTUInit()
{
    //数据队列初始化
    //modbus队列
    QueueInit(&gModbusRTURevQueue         , _UART_REV_QUEUE_DEPTH   , sizeof(ModbusRTURevData_t));  
    QueueInit(&gModbusHostQueue           , _UART_REV_QUEUE_DEPTH   , sizeof(ModbusRTURevData_t));  
    QueueInit(&gModbusRTUPackForwardQueue , _UART_REV_QUEUE_DEPTH   , sizeof(ModbusRTURevData_t));
    //普通协议队列
    QueueInit(&gUpCommonPackQueue         , _UART_REV_QUEUE_DEPTH   , sizeof(ModbusRTURevData_t));
    QueueInit(&gDownCommonPackQueue       , _UART_REV_QUEUE_DEPTH   , sizeof(ModbusRTURevData_t));

    
    for(uint8_t i = 0; i < _UART_REV_QUEUE_DEPTH; i++)
	{
		_gModbusData[i].pData = _RevData[i];
	}
   
    //串口初始化
    Uart1_ModS_Handle.pRxBuf                       = _Uart1RxBuf;
    Uart1_ModS_Handle.rx_size                      = _UART_RX_SIZE;
    Uart1_ModS_Handle.irq_rx_process               = _Uart1IrqRxProcess;
    Uart1_ModS_Handle.dma_tx_complete_process      = _Uart1DMATxCompleteIrqProcess;
    Uart1ModsBspInit();
    
    Uart2_ModS_Handle.pRxBuf                       = _Uart2RxBuf;
    Uart2_ModS_Handle.rx_size                      = _UART_RX_SIZE;
    Uart2_ModS_Handle.irq_rx_process               = _Uart2IrqRxProcess;
    Uart2_ModS_Handle.dma_tx_complete_process      = _Uart2DMATxCompleteIrqProcess;
    Uart2ModsBspInit();
    
    //默认主机结构体初始化
    _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_SLAVE].p_ModbusUart     = &Uart2_ModS_Handle;
    _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_SLAVE].p_Modbus485En    = &Modbus485En2;
    
    //默认从机结构体初始化
    _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST].p_ModbusUart    = &Uart1_ModS_Handle;
    _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST].p_Modbus485En   = &Modbus485En1;
}


/**
 * @brief  : 接收数据解析
 * @para   : size : 接收数据长度
 * @note   : None
 * @return : None
 */
static void _RevDataAnalysis(uint16_t len, Uart_HandleType* p_uarthandle)
{
    static uint16_t rev_index = 0;

    uint8_t *p = p_uarthandle->pRxBuf;
    Queue_t *p_queue = NULL;
    
    if(len < 5)
    {
        return;
    }

    //根据数据头选择队列
    //与上一级相连的串口收到的一定是5A（请求包）, 监控卡发出,BMS接收
    if((* p == COMMON_PACK_REQUST_PACKET_HEAD) && (*(p + 1) == MONITORING_CARD_ID) && (*(p + 2)  == BMS_ID))
    {
        p_queue = &gUpCommonPackQueue;
        gBMSSleepFlag = 0;
        //更新最后通讯的时间
        gUartPackInfo.final_com_time = tim_get_systime();

    }//与下一级相连的串口收到的一定是A5（相应包），发送地址是BMS
    else if((*p == COMMON_PACK_RESPONSE_PACKET_HEAD) && (*(p + 1) == BMS_ID) && (*(p + 2)  == MONITORING_CARD_ID)) 
    {
        p_queue = &gDownCommonPackQueue;
        gBMSSleepFlag = 0;

    }//modbus数据包
    else if(gUartPackInfo.ModbusAddr == p[0] || (p[0] >= MODBUS_ADDR_PACK0 && p[0] <= MODBUS_ADDR_PACK_MAX_ADDR))
    {
        if(p_uarthandle == _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST].p_ModbusUart)
        {
            p_queue = &gModbusHostQueue;
        }
        else
        {
            p_queue = &gModbusRTURevQueue;
            //更新最后通讯的时间
            gUartPackInfo.final_com_time = tim_get_systime();
        }
        
        if(_CrcJudge(p, len) == false)
        {
            return;     //和校验失败，扔掉
        }
        gBMSSleepFlag = 0;
    }
    else 
    {
        return;
    }
    
    //该数据不属于任何队列，扔掉
    if(NULL == p_queue)
    {
        return;
    }
    
    //队列已满
    if(QueueIsFull(p_queue) == true)
    {
        return;//扔掉此包数据
    }
    
    //接收数据
    _gModbusData[rev_index].slave_addr = p[0];
    _gModbusData[rev_index].p_uarthandle = p_uarthandle;
    
    _gModbusData[rev_index].data_size = len;
    memcpy(_gModbusData[rev_index].pData, p, len);
    //判断是主机还是从机接收
    if(p_uarthandle->uart_cfg->rcu_uart == _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST].p_ModbusUart->uart_cfg->rcu_uart)
    {
        _gModbusData[rev_index].en_modbus_host_or_slave = EN_MODBUS_HOST_SLAVE_TYPE_HOST;
    }
    else
    {
        _gModbusData[rev_index].en_modbus_host_or_slave = EN_MODBUS_HOST_SLAVE_TYPE_SLAVE;
    }
         
    
    //放入对应数据队列中
    if(QueuePut(p_queue, &_gModbusData[rev_index]) == false)
    {
        return;
    }
    
    rev_index++;
    rev_index %= _UART_REV_QUEUE_DEPTH;
    
}


/**
 * @brief  : 串口1接收中断
 * @para   : None
 * @note   : None
 * @return : None
 */
static void _Uart1IrqRxProcess()
{
    uint16_t rev_size = 0;

	/*DMA方式，空闲中断机制*/
	UartDMARxDisable(&Uart1_ModS_Handle);
	rev_size = UartDMARxDataLength(&Uart1_ModS_Handle);
    _RevDataAnalysis(rev_size, &Uart1_ModS_Handle);
	UartDMARxEnable(&Uart1_ModS_Handle);
}

/**
 * @brief  : 串口2接收中断
 * @para   : None
 * @note   : None
 * @return : None
 */
static void _Uart2IrqRxProcess()
{
    uint16_t rev_size = 0;
    
	/*DMA方式，空闲中断机制*/
	UartDMARxDisable(&Uart2_ModS_Handle);
	rev_size = UartDMARxDataLength(&Uart2_ModS_Handle);	
    _RevDataAnalysis(rev_size, &Uart2_ModS_Handle);      
	UartDMARxEnable(&Uart2_ModS_Handle);
}

/**
 * @brief  : 485串口发送
 * @para   : p          : 发送数据地址
             size       : 发送数据长度
             en_modbus  : 主机或从机
                          MODBUS_TYPE_HOST  ,
                          MODBUS_TYPE_SLAVE ,
 * @note   : None
 * @return : None
 */
void ModbusRTUSend(uint8_t *p, uint16_t size, EN_MODBUS_HOST_SLAVE_TYPE en_modbustype, uint8_t is_need_add_crc)
{
    uint16_t crc;
 
    if(geBMSType == EN_BMS_A0)
    {
        //打开串口1或者串口2发送使能引脚
        GpioBitSet(_gModbusUart[en_modbustype].p_Modbus485En);
    }  
    
    _gModbusUart[en_modbustype].p_ModbusUart->pTxBuf               = p;
    _gModbusUart[en_modbustype].p_ModbusUart->tx_size              = size;
    /*modbus末尾添加CRC校验，普通串口不添加*/
    if(1 == is_need_add_crc)
    {
        crc = _CrcModbus(p, size);
        _gModbusUart[en_modbustype].p_ModbusUart->pTxBuf[size]     = crc & 0xFF;
        _gModbusUart[en_modbustype].p_ModbusUart->pTxBuf[size + 1] = (crc >> 8) & 0xFF;
        
        _gModbusUart[en_modbustype].p_ModbusUart->tx_size          = size + 2;
    }
  
    _gModbusUart[en_modbustype].n_ModbusSendCompleteFlag           = 0;
    _gModbusUart[en_modbustype].n_ModbusSendMaxTime                = 1000 * (8 + 4) * _gModbusUart[en_modbustype].p_ModbusUart->tx_size / _gModbusUart[en_modbustype].p_ModbusUart->uart_cfg->baud_rate + 2;//单位ms,8表示数据位时长，4表示起始+停止+GAP时长, +2表示增加一些冗余
    
    UartSend(_gModbusUart[en_modbustype].p_ModbusUart);  
    _gModbusUart[en_modbustype].n_ModbusSendTimeout                = tim_get_systime();
}

/**
 * @brief  : 普通数据包发送
 * @para   : p             : 发送数据地址
             size          : 发送数据长度
             p_uarthandle  : 串口结构体指针

 * @note   : None
 * @return : None
 */
void CommonPackSend(uint8_t *p, uint16_t size, Uart_HandleType* p_uarthandle)
{
    EN_MODBUS_HOST_SLAVE_TYPE en_modbustype = EN_MODBUS_HOST_SLAVE_TYPE_HOST;
    
    /*根据p_uarthandle选择串口*/
    if(p_uarthandle->uart_cfg->uart_peri == _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST].p_ModbusUart->uart_cfg->uart_peri)
    {
        en_modbustype = EN_MODBUS_HOST_SLAVE_TYPE_HOST;
    }
    else
    {
        en_modbustype = EN_MODBUS_HOST_SLAVE_TYPE_SLAVE;
    }
    /*发送普通数据包*/
    ModbusRTUSend(p, size, en_modbustype, 0);
}


uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};  

/**
 * @brief  : 串口发送测试
 * @para   : None
 * @note   : None
 * @return : None
 */
void UartSendTest(Uart_HandleType *pUartHandle)
{
    static uint32_t n_output_time = 0;
    
    //固定时间上发一次
    if(tim_difftime_from_now(n_output_time) <= 5000)
    {
        return;
    }
    
    n_output_time = tim_get_systime();
    
    pUartHandle->pTxBuf  = data;
    pUartHandle->tx_size = 10;
    
    CommonPackSend(pUartHandle->pTxBuf, pUartHandle->tx_size, pUartHandle); 
}

MSH_CMD_EXPORT(UartSendTest, ---);


/**
* @brief  : 发送完成中断，将485方向从发送转变为接收（暂未用到）
 * @para   : None
 * @note   : None
 * @return : None
 */
static void _Uart1DMATxCompleteIrqProcess()
{
    /*发送完成标志与串口本身绑定，不管主机从机*/
    if(USART1 == _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST].p_ModbusUart->uart_cfg->uart_peri)
    {
        _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST].n_ModbusSendCompleteFlag = 1;
    }
    else if(USART1 == _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_SLAVE].p_ModbusUart->uart_cfg->uart_peri)
    {
        _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_SLAVE].n_ModbusSendCompleteFlag = 1;
    }
}

/**
* @brief  : 发送完成中断，将485方向从发送转变为接收（暂未用到）
 * @para   : None
 * @note   : None
 * @return : None
 */
static void _Uart2DMATxCompleteIrqProcess()
{
    /*发送完成标志与串口本身绑定，不管主机从机*/
    if(USART2 == _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST].p_ModbusUart->uart_cfg->uart_peri)
    {
        _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST].n_ModbusSendCompleteFlag = 1;
    }
    else if(USART2 == _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_SLAVE].p_ModbusUart->uart_cfg->uart_peri)
    {
        _gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_SLAVE].n_ModbusSendCompleteFlag = 1;
    }
}


/**
 * @brief  : 获取485状态，判断当前是否发送完成
 * @para   : None
 * @note   : None
 * @return : None
 */
bool DevModbusSendComplete(EN_MODBUS_HOST_SLAVE_TYPE en_modbustype)
{
    if(geBMSType == EN_BMS_A0)
    {   /*通过485控制方向引脚判断是否发送完成*/
        if(GpioBitGet(_gModbusUart[en_modbustype].p_Modbus485En) != 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        if(_gModbusUart[en_modbustype].n_ModbusSendCompleteFlag == 1 && (UartSendCompleteFlag(_gModbusUart[en_modbustype].p_ModbusUart) == true))
        {
            _gModbusUart[en_modbustype].n_ModbusSendCompleteFlag = 0;
            return true;        
        }
        else
        {
            return false;
        }
    }
}



/*大轮询调用*/
/**
 * @brief  : modbus方向处理（发送转接收）
 * @para   : en_modbustype:主机或从机
                           MODBUS_TYPE_HOST  ,
                           MODBUS_TYPE_SLAVE ,
 * @note   : None
 * @return : None
 */
void DevModbusDirCtlProcess(EN_MODBUS_HOST_SLAVE_TYPE en_modbustype)
{
    if(geBMSType == EN_BMS_A0)
    {
        if( _gModbusUart[en_modbustype].n_ModbusSendCompleteFlag == 1)
        {
            if(UartSendCompleteFlag(_gModbusUart[en_modbustype].p_ModbusUart) == true)/*串口寄存器已经发送完成，为空*/
            {
                  GpioBitReset(_gModbusUart[en_modbustype].p_Modbus485En);
                 _gModbusUart[en_modbustype].n_ModbusSendCompleteFlag = 0;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - _gModbusUart[en_modbustype].n_ModbusSendTimeout) > _gModbusUart[en_modbustype].n_ModbusSendMaxTime)/*发送异常，恢复接收状态*/
                {
                   GpioBitReset(_gModbusUart[en_modbustype].p_Modbus485En);
                }
            }
        }
        else/*处理超时*/
        {
            if(GpioBitGet((_gModbusUart[en_modbustype].p_Modbus485En)) != 0)
            {
                if((uint32_t)(tim_get_systime() - _gModbusUart[en_modbustype].n_ModbusSendTimeout) > _gModbusUart[en_modbustype].n_ModbusSendMaxTime)/*发送超时，恢复接收状态*/
                {
                    GpioBitReset(_gModbusUart[en_modbustype].p_Modbus485En);
                }
            }
        }
    }
}

/**
 * @brief  : 设置modbus从机与主机
 * @para   : p_uart        : 串口结构体指针
             en_modbustype : 主机或从机
                             MODBUS_TYPE_HOST  ,
                             MODBUS_TYPE_SLAVE ,
 * @note   : None
 * @return : None
 */
void ModbusHostOrSlaveSet(Uart_HandleType* p_uart, EN_MODBUS_HOST_SLAVE_TYPE en_modbustype)
{
    //如果当前主从机设置与期望一致，则保持原样
    if(p_uart->uart_cfg->uart_peri == _gModbusUart[en_modbustype].p_ModbusUart->uart_cfg->uart_peri)
    {
        return;
    }
    //如果当前主从机设置与期望不一致，进行调换
    _ModbusSet_t modbusset = {0};
    memcpy(&modbusset                                     , &_gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST]  , sizeof(_ModbusSet_t));
    memcpy(&_gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_HOST]  , &_gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_SLAVE] , sizeof(_ModbusSet_t));
    memcpy(&_gModbusUart[EN_MODBUS_HOST_SLAVE_TYPE_SLAVE] , &modbusset                                     , sizeof(_ModbusSet_t));
}

/**
 * @brief  : 最大响应设置时间
 * @para   :
 * @note   : None
 * @return : None
 */
uint32_t SetMaxWaitTime(uint32_t addr)
{
    /*最后一个pack等待1s，向前每个pack以此增加1s等待时间*/
    uint32_t n_max_wait_time = (1 + ((MODBUS_ADDR_PACK_MAX_ADDR - addr))) * 400;
//    if(addr == MODBUS_ADDR_DEFAULT_VALUE)
//    {
//        n_max_wait_time = 1000;
//    }
    return n_max_wait_time;
}

/**
 * @brief  : 8位转16位
 * @para   : _pBuf：待转换数据地址
 * @note   : None
 * @return : 转换完成的16位数据
 */
uint16_t BEBufToUint16(uint8_t *_pBuf)
{
    return (((uint16_t)_pBuf[0] << 8) | _pBuf[1]);
}


/**
 * @brief  : 小端模式转大端模式
 * @para   : str：待转换数据地址
             len：待转换数据长度
 * @note   : None
 * @return : None
 */
void TabLsbToMsb(uint8_t *str, uint16_t len)
{
    for(int j = 0; j < len / 2; j++)
    {
        str[j] ^= str[len - j - 1];
        str[len - j - 1] ^= str[j];
        str[j] ^= str[len - j - 1];
    }
}




