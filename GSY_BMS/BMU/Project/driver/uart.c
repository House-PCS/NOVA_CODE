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


//static dma_parameter_struct    _DMA_UART_Tx_InitStructure[UART_NUM];
//static dma_parameter_struct    _DMA_UART_Rx_InitStructure[UART_NUM];

//static bool _dma_tx_first_flag[UART_NUM];





/**
  * @brief  UART GPIO init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _UartGpioInit(Uart_HandleType *pUartHandle)
{
    if(pUartHandle->uart_cfg->en_gpio_af)
    {
        rcu_periph_clock_enable(RCU_AF);
        gpio_pin_remap_config(pUartHandle->uart_cfg->gpio_remap,ENABLE);       
    }
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
    dma_parameter_struct	dma_init_struct;
    
    rcu_periph_clock_enable(pUartHandle->uart_cfg->rcu_dma);
    dma_deinit(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_tx_channel);
    dma_struct_para_init(&dma_init_struct);  

	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_addr              = (uint32_t)(pUartHandle->pTxBuf);
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 0;
	  dma_init_struct.periph_addr	= (uint32_t)&USART_DATA(pUartHandle->uart_cfg->uart_peri);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
	dma_init(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_tx_channel, &dma_init_struct);
    /* configure DMA mode */
	dma_circulation_disable(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_tx_channel);
    
      /* enable DMA1 channel7 transfer complete interrupt */
    dma_interrupt_enable(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_tx_channel, DMA_CHXCTL_FTFIE);
   nvic_irq_enable(pUartHandle->uart_cfg->nvic_dma_irq, pUartHandle->uart_cfg->irq_dma_prio, pUartHandle->uart_cfg->irq_dma_sub_prio);
  
    /* USART DMA enable for transmission and reception */
    usart_dma_transmit_config(pUartHandle->uart_cfg->uart_peri, USART_DENT_ENABLE);

		/* enable DMA channel3 */
//    dma_channel_enable(pUartHandle->uart_cfg->dma_peri, pUartHandleType->uart_cfg->dma_tx_channel);

}


/**
  * @brief  UART DMA RX init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void UartDMARxCfg(Uart_HandleType *pUartHandle)
{
    dma_parameter_struct	dma_init_struct;
    
    rcu_periph_clock_enable(pUartHandle->uart_cfg->rcu_dma);
    dma_deinit(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_rx_channel);
    dma_struct_para_init(&dma_init_struct);
    
	dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
	dma_init_struct.memory_addr = (uint32_t)pUartHandle->pRxBuf;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
	dma_init_struct.number = pUartHandle->rx_max;
	dma_init_struct.periph_addr = (uint32_t)&USART_DATA(pUartHandle->uart_cfg->uart_peri);
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
	dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
	dma_init(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_rx_channel, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_rx_channel); 
    usart_dma_receive_config(pUartHandle->uart_cfg->uart_peri, USART_DENR_ENABLE);
	/* enable DMA channel4 */
	dma_channel_enable(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_rx_channel);
	
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

    
    usart_transmit_config(pUartHandle->uart_cfg->uart_peri,             USART_TRANSMIT_ENABLE);
    usart_receive_config(pUartHandle->uart_cfg->uart_peri,              USART_RECEIVE_ENABLE);

    nvic_irq_enable(pUartHandle->uart_cfg->nvic_irq, pUartHandle->uart_cfg->irq_prio, pUartHandle->uart_cfg->irq_sub_prio);    
    /*RX DMA config*/
    UartDMATxCfg(pUartHandle);
	UartDMARxCfg(pUartHandle);
    
    
    /*irq config*/
	nvic_irq_enable(pUartHandle->uart_cfg->nvic_irq, pUartHandle->uart_cfg->irq_prio, pUartHandle->uart_cfg->irq_sub_prio);    
	usart_interrupt_enable(pUartHandle->uart_cfg->uart_peri, USART_INT_RT);
    usart_receiver_timeout_threshold_config(pUartHandle->uart_cfg->uart_peri,35);//bit数，与波特率无关 
	usart_receiver_timeout_enable(pUartHandle->uart_cfg->uart_peri);
    
    usart_enable(pUartHandle->uart_cfg->uart_peri); /* Enable USART */
}


/*
DMA发送完成中断方式
DMA传输完成后需要在对应的中断函数中处理发送状态
*/
void UartSendDMA(const Uart_HandleType *pUartHandle,uint16_t len){	
	dma_transfer_number_config(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_tx_channel, len);
	dma_channel_enable(pUartHandle->uart_cfg->dma_peri, pUartHandle->uart_cfg->dma_tx_channel);
}

