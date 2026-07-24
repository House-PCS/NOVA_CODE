/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-20
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * <tr><td>2023-06-15    <td>V1.0.1    <td>           <td>增加同一串口可使用多种中断并存的机制
  * </table>
  ******************************************************************************
  */


#include "uart.h"

static DMA_InitType    _DMA_UART_Tx_InitStructure[UART_NUM];
static DMA_InitType    _DMA_UART_Rx_InitStructure[UART_NUM];


static bool _dma_tx_first_flag[UART_NUM];


/*国民技术的垃圾库*/

static void _get_dma_peri_channel(Uart_HandleType *pUartHandle)
{
    /*Tx*/
    if(pUartHandle->uart_cfg->en_dma_tx == true)
    {
        if(pUartHandle->uart_cfg->dma_tx_peri == DMA1)
        {
            pUartHandle->uart_cfg->dma_tx_peri_channel = (DMA_ChannelType *)(DMA1_CH1_BASE + 0x14 * pUartHandle->uart_cfg->dma_tx_channel);
        }
        else if(pUartHandle->uart_cfg->dma_tx_peri == DMA2)
        {
            pUartHandle->uart_cfg->dma_tx_peri_channel = (DMA_ChannelType *)(DMA2_CH1_BASE + 0x14 * pUartHandle->uart_cfg->dma_tx_channel);
        }
    }
    
    /*Rx*/
    if(pUartHandle->uart_cfg->en_dma_rx == true)
    {
        if(pUartHandle->uart_cfg->dma_rx_peri == DMA1)
        {
            pUartHandle->uart_cfg->dma_rx_peri_channel = (DMA_ChannelType *)(DMA1_CH1_BASE + 0x14 * pUartHandle->uart_cfg->dma_rx_channel);
        }
        else if(pUartHandle->uart_cfg->dma_rx_peri == DMA2)
        {
            pUartHandle->uart_cfg->dma_rx_peri_channel = (DMA_ChannelType *)(DMA2_CH1_BASE + 0x14 * pUartHandle->uart_cfg->dma_rx_channel);
        }
    }
}


/**
  * @brief  UART GPIO init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _UartGpioInit(Uart_HandleType *pUartHandle)
{
    GPIO_InitType GPIO_InitStructure;
    
    if(pUartHandle->uart_cfg->alt_rmp_mode == 3)
    {
        GPIO_ConfigPinRemap(pUartHandle->uart_cfg->alt_rmp_function, ENABLE);     
    }
    
    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.Pin        = pUartHandle->uart_cfg->tx_gpio_pinx;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitPeripheral(pUartHandle->uart_cfg->tx_gpiox, &GPIO_InitStructure);
    
    /* Configure USART Rx as input floating */
    GPIO_InitStructure.Pin        = pUartHandle->uart_cfg->rx_gpio_pinx;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitPeripheral(pUartHandle->uart_cfg->rx_gpiox, &GPIO_InitStructure);
}


/**
  * @brief  UART DMA TX init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartDMATxCfg(Uart_HandleType *pUartHandle)
{
    DMA_DeInit(pUartHandle->uart_cfg->dma_tx_peri_channel);
    USART_EnableDMA(pUartHandle->uart_cfg->uart_peri, USART_DMAREQ_TX, ENABLE);

    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].BufSize                  = pUartHandle->tx_size;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].MemAddr                  = (uint32_t)pUartHandle->pTxBuf;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].MemDataSize              = DMA_MemoryDataSize_Byte;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].DMA_MemoryInc            = DMA_MEM_INC_ENABLE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].PeriphAddr               = (uint32_t)&(pUartHandle->uart_cfg->uart_peri->DAT);//((uint32_t)pUartHandle->uart_cfg->uart_peri + 0x04)
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_BYTE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].PeriphInc                = DMA_PERIPH_INC_DISABLE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].Priority                 = DMA_PRIORITY_HIGH;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].Direction                = DMA_DIR_PERIPH_DST;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].CircularMode             = DMA_MODE_NORMAL;
    
    DMA_Init(pUartHandle->uart_cfg->dma_tx_peri_channel, &_DMA_UART_Tx_InitStructure[pUartHandle->uartx]);
    
    if(pUartHandle->uart_cfg->en_dma_tx == true)
    {
        if(pUartHandle->uart_cfg->en_dma_tx_irq == true)
        {
            NVIC_InitType NVIC_InitStructure;
            
            NVIC_InitStructure.NVIC_IRQChannel                   = pUartHandle->uart_cfg->nvic_dma_tx_irq;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pUartHandle->uart_cfg->irq_dma_tx_prio;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = pUartHandle->uart_cfg->irq_dma_tx_sub_prio;
            NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            DMA_ConfigInt(pUartHandle->uart_cfg->dma_tx_peri_channel, DMA_INT_TXC, ENABLE);/*当前驱动版本DMA仅支持完成中断，后续可增加其他中断种类*/
        }
    }
}



/**
  * @brief  UART DMA RX init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartDMARxCfg(Uart_HandleType *pUartHandle)
{
    DMA_DeInit(pUartHandle->uart_cfg->dma_rx_peri_channel);   

    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].BufSize                  = pUartHandle->rx_size;/*如果是空闲中断，通常配置一个比较大的值，确保实际所需接收数据小于该值*/
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].MemAddr                  = (uint32_t)pUartHandle->pRxBuf;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].MemDataSize              = DMA_MemoryDataSize_Byte;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].DMA_MemoryInc            = DMA_MEM_INC_ENABLE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].PeriphAddr               = (uint32_t)&(pUartHandle->uart_cfg->uart_peri->DAT);//((uint32_t)pUartHandle->uart_cfg->uart_peri + 0x04)
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_BYTE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].PeriphInc                = DMA_PERIPH_INC_DISABLE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].Priority                 = DMA_PRIORITY_HIGH;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].Direction                = DMA_DIR_PERIPH_SRC;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].CircularMode             = DMA_MODE_NORMAL;
    
    DMA_Init(pUartHandle->uart_cfg->dma_rx_peri_channel, &_DMA_UART_Rx_InitStructure[pUartHandle->uartx]);
    
    DMA_EnableChannel(pUartHandle->uart_cfg->dma_rx_peri_channel, ENABLE);
    USART_EnableDMA(pUartHandle->uart_cfg->uart_peri, USART_DMAREQ_RX, ENABLE);
}



static uint16_t _uart_irq_map[EN_UART_INT_TYPE_NUM] = {USART_INT_PEF, USART_INT_TXDE, USART_INT_TXC, USART_INT_RXDNE, USART_INT_IDLEF, USART_INT_LINBD, USART_INT_ERRF, USART_INT_CTSF, USART_INT_OREF, USART_INT_NEF, USART_INT_FEF};

/**
  * @brief  UART IRQ Init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _UartIrqInit(Uart_HandleType *pUartHandle)
{
    uint8_t i = 0;
    for(i = 0; i < EN_UART_INT_TYPE_NUM; i++)
    {
        if(((pUartHandle->uart_cfg->irq_type >> i) & 0x01) != 0)
        {
            USART_ConfigInt(pUartHandle->uart_cfg->uart_peri, _uart_irq_map[i], ENABLE);  
        }
    }
}



USART_InitType USART_InitStructure;
/**
  * @brief  UART init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartInit(Uart_HandleType *pUartHandle)
{
    if(pUartHandle->uart_cfg->uart_peri == USART1 || pUartHandle->uart_cfg->uart_peri == UART6 || pUartHandle->uart_cfg->uart_peri == UART7)
    {
        RCC_EnableAPB2PeriphClk(pUartHandle->uart_cfg->rcc_uart, ENABLE); /* Enable UART clock */
    }
    else
    {
        RCC_EnableAPB1PeriphClk(pUartHandle->uart_cfg->rcc_uart, ENABLE); /* Enable UART clock */
    }
    
    _get_dma_peri_channel(pUartHandle);
    
    /* USART configure */
    USART_DeInit(pUartHandle->uart_cfg->uart_peri);
    USART_InitStructure.BaudRate            = pUartHandle->uart_cfg->baud_rate;
    USART_InitStructure.WordLength          = pUartHandle->uart_cfg->wlen;
    USART_InitStructure.StopBits            = pUartHandle->uart_cfg->stblen;
    USART_InitStructure.Parity              = pUartHandle->uart_cfg->parity_cfg;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;
    
    USART_Init(pUartHandle->uart_cfg->uart_peri, &USART_InitStructure);
    
    USART_Enable(pUartHandle->uart_cfg->uart_peri, ENABLE);
    /*IO configure*/
    _UartGpioInit(pUartHandle);
    /*irq config*/
    if(pUartHandle->uart_cfg->en_irq == true)
    {
        NVIC_InitType NVIC_InitStructure;

        /* Enable the USARTz Interrupt */
        NVIC_InitStructure.NVIC_IRQChannel                   = pUartHandle->uart_cfg->nvic_irq;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pUartHandle->uart_cfg->irq_prio;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = pUartHandle->uart_cfg->irq_sub_prio;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        
        _UartIrqInit(pUartHandle);
    }
    /*RX DMA config*/
    if(pUartHandle->uart_cfg->en_dma_rx == true)
    {
        UartDMARxEnable(pUartHandle);
    }
    
}



/**
  * @brief  UART rx dma data length.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return data length
  * @note   none
  */
uint16_t UartDMARxDataLength(Uart_HandleType *pUartHandle)
{
    return (pUartHandle->rx_size - DMA_GetCurrDataCounter(pUartHandle->uart_cfg->dma_rx_peri_channel));
}

/**
  * @brief  UART rx dma disable.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartDMARxDisable(Uart_HandleType *pUartHandle)
{
    DMA_EnableChannel(pUartHandle->uart_cfg->dma_rx_peri_channel, DISABLE);
}


/**
  * @brief  UART rx dma enable.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartDMARxEnable(Uart_HandleType *pUartHandle)
{
    UartDMARxCfg(pUartHandle);
    DMA_EnableChannel(pUartHandle->uart_cfg->dma_rx_peri_channel, ENABLE);
}




/**
  * @brief  UART dma send out.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartDMASend(Uart_HandleType *pUartHandle)
{
    uint32_t timeout = 0xfff;
    if(_dma_tx_first_flag[pUartHandle->uartx] == 1)
    {
        uint8_t dma_channel_index;
        uint8_t dma_index;
        while(DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pUartHandle->uart_cfg->dma_tx_channel * 4)), pUartHandle->uart_cfg->dma_tx_peri) == RESET)//DMA1和DMA2的FLAG值一样
        {
            timeout--;
            if(timeout == 0)
            {
                break;
            }
        }
    }
    DMA_ClearFlag((DMA1_FLAG_TC1 << (pUartHandle->uart_cfg->dma_tx_channel * 4)), pUartHandle->uart_cfg->dma_tx_peri);
    UartDMATxCfg(pUartHandle);
    DMA_EnableChannel(pUartHandle->uart_cfg->dma_tx_peri_channel, ENABLE);
    
    if(pUartHandle->uart_cfg->en_dma_tx_nowait == false)
    {
        while(DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pUartHandle->uart_cfg->dma_tx_channel * 4)), pUartHandle->uart_cfg->dma_tx_peri) == RESET);
        while(USART_GetFlagStatus(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TXC) == RESET);
    }
    
    _dma_tx_first_flag[pUartHandle->uartx] = 1;
}


/**
  * @brief  UART normal send out.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartNormalSend(Uart_HandleType *pUartHandle)
{
    uint16_t i = 0;
    for(i = 0; i < pUartHandle->tx_size; i++)
    {
        while((pUartHandle->uart_cfg->uart_peri->STS & 0x40) == 0);
        USART_SendData(pUartHandle->uart_cfg->uart_peri, pUartHandle->pTxBuf[i] & 0x1ff);
    }
    while((pUartHandle->uart_cfg->uart_peri->STS & 0x40) == 0);
}



/**
  * @brief  UART send out, Whether to use DMA depends on the user configuration.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartSend(Uart_HandleType *pUartHandle)
{
    uint16_t i = 0;
    uint32_t timeout = 0xfff;
    
    if(pUartHandle->uart_cfg->en_dma_tx == true)
    {
        UartDMASend(pUartHandle);
    }
    else
    {
        UartNormalSend(pUartHandle);
    }
}




/**
  * @brief  UART irq process.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartIrqProcess(Uart_HandleType *pUartHandle)
{
 
    if(pUartHandle->uart_cfg->en_dma_rx == true)
    {
        /*接收空闲中断*/
        if(USART_GetFlagStatus(pUartHandle->uart_cfg->uart_peri, USART_FLAG_IDLEF) != RESET)
        {
            pUartHandle->uart_cfg->uart_peri->STS;                     //读SR
            USART_ReceiveData(pUartHandle->uart_cfg->uart_peri);               //读DR
            pUartHandle->irq_rx_process();            
        }    
    }
    else
    {
        /*接收非空中断*/
        if(USART_GetFlagStatus(pUartHandle->uart_cfg->uart_peri, USART_FLAG_RXDNE) != RESET)
        {
            *pUartHandle->pRxBuf = (uint8_t)USART_ReceiveData(pUartHandle->uart_cfg->uart_peri);            
            pUartHandle->irq_rx_process();
        } 
    }
}

/**
  * @brief  UART tx DMA Complete process.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartDMATxIrqProcess(Uart_HandleType *pUartHandle)
{
    /*DMA传输完成*/
    if(DMA_GetIntStatus(DMA1_INT_TXC1 << pUartHandle->uart_cfg->dma_tx_channel * 4, pUartHandle->uart_cfg->dma_rx_peri))
    {
        DMA_ClrIntPendingBit(DMA1_INT_TXC1 << pUartHandle->uart_cfg->dma_tx_channel * 4, pUartHandle->uart_cfg->dma_rx_peri);
        pUartHandle->dma_tx_complete_process();
    }
    /*DMA传输错误*/
    else if(DMA_GetIntStatus(DMA1_INT_ERR1 << pUartHandle->uart_cfg->dma_tx_channel * 4, pUartHandle->uart_cfg->dma_rx_peri))
    {
        DMA_ClrIntPendingBit(DMA1_INT_ERR1 << pUartHandle->uart_cfg->dma_tx_channel * 4, pUartHandle->uart_cfg->dma_rx_peri);
    }
}



/**
  * @brief  UART发送完成标志.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
bool UartSendCompleteFlag(Uart_HandleType *pUartHandle)
{
    FlagStatus sta = USART_GetFlagStatus(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TXC);
    if(sta == SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}



void UartClearCompleteFlag(Uart_HandleType *pUartHandle)
{
    USART_ClrFlag(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TXC);
}




