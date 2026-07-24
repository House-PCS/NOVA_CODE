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
  * </table>
  ******************************************************************************
  */



#include "uart.h"


static dma_parameter_struct    _DMA_UART_Tx_InitStructure[UART_NUM];
static dma_parameter_struct    _DMA_UART_Rx_InitStructure[UART_NUM];

static bool _dma_tx_first_flag[UART_NUM];





/**
  * @brief  UART GPIO init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _UartGpioInit(Uart_HandleType *pUartHandle)
{
    /* connect port to USARTx_Tx */
    gpio_init(pUartHandle->uart_cfg->tx_gpiox, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, pUartHandle->uart_cfg->tx_gpio_pinx);

    /* connect port to USARTx_Rx */
    gpio_init(pUartHandle->uart_cfg->rx_gpiox, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, pUartHandle->uart_cfg->rx_gpio_pinx);

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
    usart_dma_transmit_config(pUartHandle->uart_cfg->uart_peri, USART_DENT_ENABLE);

    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].number                   = pUartHandle->tx_size;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].memory_width             = DMA_PERIPHERAL_WIDTH_8BIT;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].memory_inc               = DMA_MEMORY_INCREASE_ENABLE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].periph_addr              = (uint32_t)&USART_DATA(pUartHandle->uart_cfg->uart_peri);
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].periph_inc               = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].priority                 = DMA_PRIORITY_HIGH;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].periph_inc               = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].direction                = DMA_MEMORY_TO_PERIPHERAL;
    _DMA_UART_Tx_InitStructure[pUartHandle->uartx].memory_addr              = (uint32_t)pUartHandle->pTxBuf;
    dma_init(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, &_DMA_UART_Tx_InitStructure[pUartHandle->uartx]);
    dma_circulation_disable(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel);
    
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
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].memory_width             = DMA_MEMORY_WIDTH_8BIT;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].memory_inc               = DMA_MEMORY_INCREASE_ENABLE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].periph_addr              = (uint32_t)&USART_DATA(pUartHandle->uart_cfg->uart_peri);
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].periph_inc               = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].priority                 = DMA_PRIORITY_HIGH;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].periph_inc               = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].direction                = DMA_PERIPHERAL_TO_MEMORY;
    _DMA_UART_Rx_InitStructure[pUartHandle->uartx].memory_addr              = (uint32_t)pUartHandle->pRxBuf;
    dma_init(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel, &_DMA_UART_Rx_InitStructure[pUartHandle->uartx]);
    dma_circulation_disable(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel);
    
    dma_channel_enable(pUartHandle->uart_cfg->dma_rx_peri, pUartHandle->uart_cfg->dma_rx_channel);
    usart_dma_receive_config(pUartHandle->uart_cfg->uart_peri, USART_DENR_ENABLE);
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
        usart_interrupt_enable(pUartHandle->uart_cfg->uart_peri, pUartHandle->uart_cfg->irq_type);  
        
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
//        while(dma_flag_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_FLAG_FTF)==RESET)
        while(dma_transfer_number_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel) != 0)
        {
            timeout--;
            if(timeout == 0)
            {
                break;
            }
        }
    }
    usart_flag_clear(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TC);
    dma_flag_clear(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_FLAG_FTF);
    UartDMATxCfg(pUartHandle);
    dma_channel_enable(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel);
    
    if(pUartHandle->uart_cfg->en_dma_tx_nowait == false)
    {
        timeout = 0xffff;
//        while(dma_flag_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_FLAG_FTF) == RESET);
        while(dma_transfer_number_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel) != 0)
        {
            timeout--;
            if(timeout == 0)
            {
                break;
            }
        }
        timeout = 0xffff;
        while(usart_flag_get(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TC) == RESET)
        {
            timeout--;
            if(timeout == 0)
            {
                break;
            }
        }
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
        if(usart_interrupt_flag_get(pUartHandle->uart_cfg->uart_peri, USART_INT_FLAG_IDLE) != RESET)
        {
            USART_STAT0(pUartHandle->uart_cfg->uart_peri);                      //读SR
            usart_data_receive(pUartHandle->uart_cfg->uart_peri);               //读DR
            pUartHandle->irq_rx_process();            
        }    
    }
    else
    {
        /*接收非空中断*/
        if(usart_interrupt_flag_get(pUartHandle->uart_cfg->uart_peri, USART_INT_FLAG_RBNE) != RESET)
        {
            *pUartHandle->pRxBuf = (uint8_t)usart_data_receive(pUartHandle->uart_cfg->uart_peri);            
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
    if(dma_interrupt_flag_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_INT_FLAG_FTF);
        pUartHandle->dma_tx_complete_process();
    }
    /*DMA传输错误*/
    else if(dma_interrupt_flag_get(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_INT_FLAG_ERR))
    {
        dma_interrupt_flag_clear(pUartHandle->uart_cfg->dma_tx_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_INT_FLAG_ERR);
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


void UartClearCompleteFlag(Uart_HandleType *pUartHandle)
{
    usart_flag_clear(pUartHandle->uart_cfg->uart_peri, USART_FLAG_TC);
}



















