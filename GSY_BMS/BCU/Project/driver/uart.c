/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
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


static dma_single_data_parameter_struct    _DMA_UART_Tx_InitStructure[UART_NUM];
static dma_single_data_parameter_struct    _DMA_UART_Rx_InitStructure[UART_NUM];

static bool _dma_tx_first_flag[UART_NUM];





/**
  * @brief  UART GPIO init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _UartGpioInit(Uart_HandleType *pUartHandle)
{
    gpio_af_set(pUartHandle->uart_cfg->tx_gpiox, pUartHandle->uart_cfg->tx_alt_num, pUartHandle->uart_cfg->tx_gpio_pinx);	 /* Connect PXx to USARTx_Tx*/
    gpio_af_set(pUartHandle->uart_cfg->rx_gpiox, pUartHandle->uart_cfg->rx_alt_num, pUartHandle->uart_cfg->rx_gpio_pinx);	 /* Connect PXx to USARTx_Rx*/
    
    /* Configure USART Tx as alternate function  */
    gpio_mode_set(pUartHandle->uart_cfg->tx_gpiox, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pUartHandle->uart_cfg->tx_gpio_pinx);
    gpio_output_options_set(pUartHandle->uart_cfg->tx_gpiox, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pUartHandle->uart_cfg->tx_gpio_pinx);
    
    /* Configure USART Rx as alternate function  */            
    gpio_mode_set(pUartHandle->uart_cfg->rx_gpiox, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pUartHandle->uart_cfg->rx_gpio_pinx);
    gpio_output_options_set(pUartHandle->uart_cfg->rx_gpiox, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pUartHandle->uart_cfg->rx_gpio_pinx);

}


/**
  * @brief  UART DMA TX init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartDMATxCfg(Uart_HandleType *pUartHandle)
{
    dma_deinit(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel);
    usart_dma_transmit_config(pUartHandle->uart_cfg->uart_peri, USART_TRANSMIT_DMA_ENABLE);

    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].number                   = pUartHandle->tx_size;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].periph_memory_width      = DMA_PERIPH_WIDTH_8BIT;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].memory_inc               = DMA_MEMORY_INCREASE_ENABLE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].periph_addr              = (uint32_t)&USART_DATA(pUartHandle->uart_cfg->uart_peri);
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].periph_inc               = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].priority                 = DMA_PRIORITY_HIGH;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].circular_mode            = DMA_CIRCULAR_MODE_DISABLE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].periph_inc               = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].direction                = DMA_MEMORY_TO_PERIPH;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].memory0_addr             = (uint32_t)pUartHandle->pTxBuf;
    dma_single_data_mode_init(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, &_DMA_UART_Tx_InitStructure[pUartHandle->uartx]);
    dma_channel_subperipheral_select(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, pUartHandle->uart_cfg->dma_tx_subperi);
    
    if(pUartHandle->uart_cfg->en_dma_tx == true)
    {
        if(pUartHandle->uart_cfg->en_dma_tx_irq == true)
        {
            nvic_irq_enable(pUartHandle->uart_cfg->nvic_dma_tx_irq, pUartHandle->uart_cfg->irq_dma_tx_prio, pUartHandle->uart_cfg->irq_dma_tx_sub_prio);
            dma_interrupt_enable(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_CHXCTL_FTFIE);/*当前驱动版本DMA仅支持完成中断，后续可增加其他中断种类*/
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
    dma_deinit(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel);   

    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].number                   = pUartHandle->rx_size;/*如果是空闲中断，通常配置一个比较大的值，确保实际所需接收数据小于该值*/
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].periph_memory_width      = DMA_MEMORY_WIDTH_8BIT;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].memory_inc               = DMA_MEMORY_INCREASE_ENABLE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].periph_addr              = (uint32_t)&USART_DATA(pUartHandle->uart_cfg->uart_peri);
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].periph_inc               = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].priority                 = DMA_PRIORITY_HIGH;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].circular_mode            = DMA_CIRCULAR_MODE_DISABLE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].periph_inc               = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].direction                = DMA_PERIPH_TO_MEMORY;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].memory0_addr             = (uint32_t)pUartHandle->pRxBuf;
    
    dma_single_data_mode_init(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel, &_DMA_UART_Rx_InitStructure[pUartHandle->uartx]);
    dma_channel_subperipheral_select(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel, pUartHandle->uart_cfg->dma_rx_subperi);
    
    dma_channel_enable(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel);
    usart_dma_receive_config(pUartHandle->uart_cfg->uart_peri, USART_RECEIVE_DMA_ENABLE);
}




static usart_interrupt_enum _uart_irq_map[EN_UART_INT_TYPE_NUM] = {USART_INT_PERR, USART_INT_TBE, USART_INT_TC, USART_INT_RBNE, USART_INT_IDLE, USART_INT_LBD, USART_INT_ERR, USART_INT_CTS, USART_INT_RT, USART_INT_EB};
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
            usart_interrupt_enable(pUartHandle->uart_cfg->uart_peri, _uart_irq_map[i]);  
        }
    }
}



/**
  * @brief  UART init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartInit(Uart_HandleType *pUartHandle)
{   
    rcu_periph_clock_enable(pUartHandle->uart_cfg->rcu_uart); /* Enable UART clock */
    
    /*IO configure*/
    _UartGpioInit(pUartHandle);
    /* USART configure */
    usart_deinit(pUartHandle->uart_cfg->uart_peri);
    usart_baudrate_set(pUartHandle->uart_cfg->uart_peri,                pUartHandle->uart_cfg->baud_rate);
    usart_parity_config(pUartHandle->uart_cfg->uart_peri,               pUartHandle->uart_cfg->parity_cfg);
    usart_word_length_set(pUartHandle->uart_cfg->uart_peri,             pUartHandle->uart_cfg->wlen);
    usart_stop_bit_set(pUartHandle->uart_cfg->uart_peri,                pUartHandle->uart_cfg->stblen);
    usart_hardware_flow_rts_config(pUartHandle->uart_cfg->uart_peri,    USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(pUartHandle->uart_cfg->uart_peri,    USART_CTS_DISABLE);

    usart_enable(pUartHandle->uart_cfg->uart_peri); /* Enable USART */
    usart_transmit_config(pUartHandle->uart_cfg->uart_peri,             USART_TRANSMIT_ENABLE);
    usart_receive_config(pUartHandle->uart_cfg->uart_peri,              USART_RECEIVE_ENABLE);

    nvic_irq_enable(pUartHandle->uart_cfg->nvic_irq, pUartHandle->uart_cfg->irq_prio, pUartHandle->uart_cfg->irq_sub_prio);    
    /*RX DMA config*/
    if(pUartHandle->uart_cfg->en_dma_rx == true)
    {
        UartDMARxEnable(pUartHandle);
    }
    /*irq config*/
    if(pUartHandle->uart_cfg->en_irq == true)
    {
//        usart_interrupt_enable(pUartHandle->uart_cfg->uart_peri, pUartHandle->uart_cfg->irq_type);  
        _UartIrqInit(pUartHandle);
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
    return (pUartHandle->rx_size - dma_transfer_number_get(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel));
}

/**
  * @brief  UART rx dma disable.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartDMARxDisable(Uart_HandleType *pUartHandle)
{
    dma_channel_disable(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel);
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
    dma_channel_enable(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel);
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
        while(dma_flag_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_FLAG_FTF)==RESET)
        {
            timeout--;
            if(timeout == 0)
            {
                break;
            }
        }
    }
    dma_flag_clear(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_FLAG_FTF);
    UartDMATxCfg(pUartHandle);
    dma_channel_enable(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel);
    
    if(pUartHandle->uart_cfg->en_dma_tx_nowait == false)
    {
        while(dma_flag_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_FLAG_FTF) == RESET);
        while(usart_flag_get(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TC) == RESET);
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
        while((USART_STAT0(pUartHandle->uart_cfg->uart_peri) & 0x40) == 0);
        usart_data_transmit(pUartHandle->uart_cfg->uart_peri, pUartHandle->pTxBuf[i] & 0x1ff);
    }
    while((USART_STAT0(pUartHandle->uart_cfg->uart_peri) & 0x40) == 0);
}


/**
  * @brief  UART send out, Whether to use DMA depends on the user configuration.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartSend(Uart_HandleType *pUartHandle)
{
    //uint16_t i = 0;
    //uint32_t timeout = 0xfff;
    
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
        if(usart_interrupt_flag_get(pUartHandle->uart_cfg->uart_peri, USART_INT_FLAG_IDLE) != RESET)
        {
            USART_STAT0(pUartHandle->uart_cfg->uart_peri);                      //读SR
            usart_data_receive(pUartHandle->uart_cfg->uart_peri);               //读DR
            pUartHandle->irq_rx_process(pUartHandle);            
        }    
    }
    else
    {
        /*接收非空中断*/
        if(usart_interrupt_flag_get(pUartHandle->uart_cfg->uart_peri, USART_INT_FLAG_RBNE) != RESET)
        {
            *pUartHandle->pRxBuf = (uint8_t)usart_data_receive(pUartHandle->uart_cfg->uart_peri);            
            pUartHandle->irq_rx_process(pUartHandle);
        } 
    }
    if(usart_interrupt_flag_get(pUartHandle->uart_cfg->uart_peri, USART_INT_FLAG_TC) != RESET)
    {
        usart_interrupt_flag_clear(pUartHandle->uart_cfg->uart_peri, USART_INT_FLAG_TC);            
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
    if(dma_interrupt_flag_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_INT_FLAG_FTF);
        pUartHandle->dma_tx_complete_process();
    }
    /*DMA传输错误*/
    else if(dma_interrupt_flag_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_INT_FLAG_TAE))
    {
        dma_interrupt_flag_clear(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_INT_FLAG_TAE);
    }
}




bool UartSendCompleteFlag(Uart_HandleType *pUartHandle)
{
    FlagStatus sta = usart_flag_get(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TC);
    if(sta == SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool UartSendBufEmptyFlag(Uart_HandleType *pUartHandle)
{
    FlagStatus sta = usart_flag_get(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TBE);
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
    usart_flag_clear(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TC);
}


void UartClearSendBufEmptyFlag(Uart_HandleType *pUartHandle)
{
    usart_flag_clear(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TBE);
}







