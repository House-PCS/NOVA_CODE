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
    if(pCanHandle->can_cfg->alt_rmp_mode == 3)
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
    if(pCanHandle->can_cfg->en_data_filter == true)
	{
		can_filter.filter_list_high = (pCanHandle->can_cfg->filter_id >> 13) & 0xFFFF;
		can_filter.filter_list_low = ((pCanHandle->can_cfg->filter_id << 3) | (1 << 2)) & 0xFFFF;
		can_filter.filter_mask_high = (pCanHandle->can_cfg->filter_mask >> 13) & 0xFFFF;
		can_filter.filter_mask_low = ((pCanHandle->can_cfg->filter_mask << 3) | (1 << 2)) & 0xFFFF;
	}
	else
	{
		can_filter.filter_list_high = 0x0000;
		can_filter.filter_list_low = 0x0000;
		can_filter.filter_mask_high = 0x0000;
		can_filter.filter_mask_low = 0x0000;
	}
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    can_filter_init(&can_filter);
    /*irq config*/
    if(pCanHandle->can_cfg->en_irq_tx == true)
	{
		nvic_irq_enable(pCanHandle->can_cfg->nvic_irq_tx, pCanHandle->can_cfg->irq_prio_tx, pCanHandle->can_cfg->irq_sub_prio_tx);  
        can_interrupt_enable(pCanHandle->can_cfg->can_peri, pCanHandle->can_cfg->irq_type_tx);    
	}
	
	if(pCanHandle->can_cfg->en_irq_rx0 == true)
	{
		nvic_irq_enable(pCanHandle->can_cfg->nvic_irq_rx0, pCanHandle->can_cfg->irq_prio_rx0, pCanHandle->can_cfg->irq_sub_prio_rx0);  
        can_interrupt_enable(pCanHandle->can_cfg->can_peri, pCanHandle->can_cfg->irq_type_rx0);  	
		
	}
	
	if(pCanHandle->can_cfg->en_irq_rx1 == true)
	{
		nvic_irq_enable(pCanHandle->can_cfg->nvic_irq_rx1, pCanHandle->can_cfg->irq_prio_rx1, pCanHandle->can_cfg->irq_sub_prio_rx1);  
        can_interrupt_enable(pCanHandle->can_cfg->can_peri, pCanHandle->can_cfg->irq_type_rx1);  	
	}
	
}


bool CanSend(Can_HandleType *pCanHandle)
{
    uint8_t ret = 0;
	can_trasnmit_message_struct transmit_message;
	
	if(pCanHandle->tx_frame_type == EN_CAN_FRAME_TYPE_STANDARD)
	{
		transmit_message.tx_sfid = pCanHandle->tx_id;
		transmit_message.tx_ff	 = CAN_FF_STANDARD;
	}
	else
	{
		transmit_message.tx_efid = pCanHandle->tx_id;
		transmit_message.tx_ff	 = CAN_FF_EXTENDED;
	}
	
	if(pCanHandle->tx_frame_kind == EN_CAN_FRAME_KIND_DATA)
	{
		transmit_message.tx_ft	 = CAN_FT_DATA;
		transmit_message.tx_dlen = pCanHandle->tx_len;
	}
	else
	{
		transmit_message.tx_ft	 = CAN_FT_REMOTE;
		transmit_message.tx_dlen = 0;
	}
	memcpy(transmit_message.tx_data, pCanHandle->tx_buf, transmit_message.tx_dlen);
	
	ret = can_message_transmit(pCanHandle->can_cfg->can_peri, &transmit_message);
	if(ret == CAN_NOMAILBOX)
	{
		return false;
	}
  	return true;
   
}


/**
  * @brief  CAN 错误清除，可轮询执行.
  * @param  Can_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanErrorClear(Can_HandleType *pCanHandle)
{
	if(can_flag_get(pCanHandle->can_cfg->can_peri, CAN_FLAG_MTE2) != RESET ||
	   can_flag_get(pCanHandle->can_cfg->can_peri, CAN_FLAG_MTE0) != RESET ||
	   can_flag_get(pCanHandle->can_cfg->can_peri, CAN_FLAG_MTE1) != RESET || 
	   can_flag_get(pCanHandle->can_cfg->can_peri, CAN_FLAG_PERR) != RESET ||
	   can_flag_get(pCanHandle->can_cfg->can_peri, CAN_FLAG_WERR) != RESET)
	{
		can_flag_clear(pCanHandle->can_cfg->can_peri, CAN_FLAG_MTE0);
		can_flag_clear(pCanHandle->can_cfg->can_peri, CAN_FLAG_MTE1);
		can_flag_clear(pCanHandle->can_cfg->can_peri, CAN_FLAG_MTE2);
		can_flag_clear(pCanHandle->can_cfg->can_peri, CAN_FLAG_PERR);
		can_flag_clear(pCanHandle->can_cfg->can_peri, CAN_FLAG_WERR);
		can_wakeup(pCanHandle->can_cfg->can_peri);
		CanInit(pCanHandle);
	}
}



/**
  * @brief  CAN tx irq process.
  * @param  Can_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanTxIrqProcess(Can_HandleType *pCanHandle)
{
	if(can_interrupt_flag_get(pCanHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF0) != RESET)
	{
		can_interrupt_flag_clear(pCanHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF0);
	}
	if(can_interrupt_flag_get(pCanHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF1) != RESET)
	{
		can_interrupt_flag_clear(pCanHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF1);
	}
	if(can_interrupt_flag_get(pCanHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF2) != RESET)
	{
		can_interrupt_flag_clear(pCanHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF2);
	}
	else
	{
		return;
	}
	
	if(pCanHandle->irq_tx_process != NULL)
	{
		pCanHandle->irq_tx_process();
	}
}



/**
  * @brief  CAN rx0 irq process.
  * @param  Can_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanRx0IrqProcess(Can_HandleType *pCanHandle)
{
	can_receive_message_struct receive_message;
	can_message_receive(pCanHandle->can_cfg->can_peri, CAN_FIFO0, &receive_message);
	
	if(receive_message.rx_ff == CAN_FF_STANDARD)
	{
		pCanHandle->rx_id = receive_message.rx_sfid;
		pCanHandle->rx_frame_type = EN_CAN_FRAME_TYPE_STANDARD;
	}
	else
	{
		pCanHandle->rx_id = receive_message.rx_efid;
		pCanHandle->rx_frame_type = EN_CAN_FRAME_TYPE_EXTENDED;
	}
	if(receive_message.rx_ft == CAN_FT_DATA)
	{
		pCanHandle->rx_frame_kind = EN_CAN_FRAME_KIND_DATA;
		pCanHandle->rx_len = receive_message.rx_dlen;
		memcpy(pCanHandle->rx_buf, receive_message.rx_data, receive_message.rx_dlen);
	}
	else
	{
		pCanHandle->rx_frame_kind = EN_CAN_FRAME_KIND_REMOTE;
		pCanHandle->rx_len = 0;
	}
    if(pCanHandle->irq_rx0_process != NULL)
	{
		pCanHandle->irq_rx0_process();
	}
}
  
  
  /**
  * @brief  CAN rx1 irq process.
  * @param  Can_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanRx1IrqProcess(Can_HandleType *pCanHandle)
{
	can_receive_message_struct receive_message;
	can_message_receive(pCanHandle->can_cfg->can_peri, CAN_FIFO1, &receive_message);
	
	if(receive_message.rx_ff == CAN_FF_STANDARD)
	{
		pCanHandle->rx_id = receive_message.rx_sfid;
		pCanHandle->rx_frame_type = EN_CAN_FRAME_TYPE_STANDARD;
	}
	else
	{
		pCanHandle->rx_id = receive_message.rx_efid;
		pCanHandle->rx_frame_type = EN_CAN_FRAME_TYPE_EXTENDED;
	}
	if(receive_message.rx_ft == CAN_FT_DATA)
	{
		pCanHandle->rx_frame_kind = EN_CAN_FRAME_KIND_DATA;
		pCanHandle->rx_len = receive_message.rx_dlen;
		memcpy(pCanHandle->rx_buf, receive_message.rx_data, receive_message.rx_dlen);
	}
	else
	{
		pCanHandle->rx_frame_kind = EN_CAN_FRAME_KIND_REMOTE;
		pCanHandle->rx_len = 0;
	}
    if(pCanHandle->irq_rx1_process != NULL)
	{
		pCanHandle->irq_rx1_process();
	}
}
