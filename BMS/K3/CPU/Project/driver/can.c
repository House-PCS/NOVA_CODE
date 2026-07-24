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
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */



#include "can.h"

/**
  * @brief  CAN GPIO init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _CanGpioInit(Can_HandleType *pCanHandle)
{
    if(pCanHandle->can_cfg->en_gpio_af)
    {
        rcu_periph_clock_enable(RCU_AF);
        gpio_pin_remap_config(GPIO_CAN_PARTIAL_REMAP,ENABLE);       
    }
    rcu_periph_clock_enable(pCanHandle->can_cfg->rcu_tx_gpio);
	rcu_periph_clock_enable(pCanHandle->can_cfg->rcu_rx_gpio);
    gpio_init(pCanHandle->can_cfg->tx_gpiox,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,pCanHandle->can_cfg->tx_gpio_pinx);   /* Connect PXx to USARTx_Tx*/
    gpio_init(pCanHandle->can_cfg->rx_gpiox,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,pCanHandle->can_cfg->rx_gpio_pinx);     /* Connect PXx to USARTx_Rx*/
    
}


/**
  * @brief  CAN init.
  * @param  Can_HandleType: can需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanInit(Can_HandleType *pCanHandle)
{   
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;
    
    rcu_periph_clock_enable(pCanHandle->can_cfg->rcu_can); /* Enable CAN clock */
    
        /*IO configure*/
        _CanGpioInit(pCanHandle);

    /* CAN configure */
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_INIT_STRUCT, &can_filter);
    /* initialize CAN register */
    can_deinit(pCanHandle->can_cfg->can_peri);
    
    /* initialize CAN parameters */
     can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.no_auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width     = pCanHandle->can_cfg->resync_jump_width;
    can_parameter.time_segment_1        = pCanHandle->can_cfg->time_segment_1;
    can_parameter.time_segment_2        = pCanHandle->can_cfg->time_segment_2;
    can_parameter.prescaler             = pCanHandle->can_cfg->prescaler;  
    /* initialize CAN */
    can_init(pCanHandle->can_cfg->can_peri, &can_parameter);
    
    /* initialize filter */ 
    can_filter.filter_number=0;
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    can_filter_init(&can_filter);
    /*irq config*/
    if(pCanHandle->can_cfg->en_irq_rx == true)
    {
        nvic_irq_enable(pCanHandle->can_cfg->nvic_irq_rx, 1, 0);  
        /* enable CAN receive FIFO1 not empty interrupt  */ 
        can_interrupt_enable(pCanHandle->can_cfg->can_peri, CAN_INT_RFNE0);        
    }
}


bool CanSend(Can_HandleType *pCanHandle)
{
//    can_trasnmit_message_struct transmit_message;
//    uint32_t timeout = 0xFFFF;
//    uint8_t transmit_mailbox = 0;
//    
//    /* initialize transmit message */
//    transmit_message.tx_sfid = pCanHandle->tx_sfid;
//    transmit_message.tx_efid = pCanHandle->tx_efid;
//    transmit_message.tx_ff = pCanHandle->tx_ff;
//    transmit_message.tx_ft = pCanHandle->tx_ft;
//    transmit_message.tx_dlen = pCanHandle->tx_dlen;
//    memcpy(transmit_message.tx_data, pCanHandle->tx_data, transmit_message.tx_dlen);
//    /* transmit a message */
//    transmit_mailbox = can_message_transmit(pCanHandle->can_cfg->can_peri, &transmit_message);

//    /* waiting for transmit completed */
//    while((CAN_TRANSMIT_OK != can_transmit_states(pCanHandle->can_cfg->can_peri, transmit_mailbox)) && (0 != timeout)){
//        timeout--;
//    }
    uint8_t transmit_mailbox;
    
    transmit_mailbox = can_message_transmit(pCanHandle->can_cfg->can_peri, &pCanHandle->transmit_message);
    if(transmit_mailbox<3)
    {
        return true;
    }
    else
    {
        return false;
    }
   
}

/**
  * @brief  UART irq process.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanRxIrqProcess(Can_HandleType *pCanHandle)
{
//    can_receive_message_struct receive_message = {0};   
    /* check the receive message */
    can_message_receive(pCanHandle->can_cfg->can_peri, CAN_FIFO0, &pCanHandle->receive_message);
//    pCanHandle->rx_sfid = receive_message.rx_sfid; 
//    pCanHandle->rx_efid = receive_message.rx_efid; 
//    pCanHandle->rx_ff   = receive_message.rx_ff;   
//    pCanHandle->rx_dlen = receive_message.rx_dlen; 
//    pCanHandle->rx_fi   = receive_message.rx_fi;
//    memcpy(pCanHandle->rx_data, receive_message.rx_data, 8);     
    pCanHandle->irq_rx_process();
}
