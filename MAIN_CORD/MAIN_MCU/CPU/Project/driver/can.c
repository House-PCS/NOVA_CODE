/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-27
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-06-27    <td>V1.0.0    <td>           <td>InitialVersion，基础参数固定，不支持时间触发、支持自动重发、仅支持单FIFO、掩码过滤模式、32位宽
  * </table>
  ******************************************************************************
  */

#include "can.h"

/**
  * @brief  CAN GPIO init.
  * @param  Uart_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _CanGpioInit(Can_HandleType *pHandle)
{
    /* Configure CAN Tx as alternate function  */
    gpio_mode_set(pHandle->can_cfg->tx_gpiox, GPIO_MODE_AF, GPIO_PUPD_NONE, pHandle->can_cfg->tx_gpio_pinx);
    gpio_output_options_set(pHandle->can_cfg->tx_gpiox, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->can_cfg->tx_gpio_pinx);
    gpio_af_set(pHandle->can_cfg->tx_gpiox, pHandle->can_cfg->tx_alt_num, pHandle->can_cfg->tx_gpio_pinx);	 /* Connect PXx to USARTx_Tx*/

    /* Configure CAN Rx as alternate function  */            
    gpio_mode_set(pHandle->can_cfg->rx_gpiox, GPIO_MODE_AF, GPIO_PUPD_NONE, pHandle->can_cfg->rx_gpio_pinx);
    gpio_output_options_set(pHandle->can_cfg->rx_gpiox, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->can_cfg->rx_gpio_pinx);
    gpio_af_set(pHandle->can_cfg->rx_gpiox, pHandle->can_cfg->rx_alt_num, pHandle->can_cfg->rx_gpio_pinx);	 /* Connect PXx to USARTx_Rx*/

}


/**
  * @brief  CAN  init.
  * @param  Can_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanInit(Can_HandleType *pHandle)
{
	can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;
	
	_CanGpioInit(pHandle);
	rcu_periph_clock_enable(pHandle->can_cfg->rcu_can);
	
	/* initialize CAN structures */
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
    /* initialize CAN register */
    can_deinit(pHandle->can_cfg->can_peri);
	
	/* initialize CAN */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = ENABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;//CAN_SILENT_LOOPBACK_MODE//CAN_SILENT_MODE
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_7TQ; //CAN_BT_BS1_7TQ
    can_parameter.time_segment_2 = CAN_BT_BS2_2TQ;
    /* baudrate ?bps */
    can_parameter.prescaler = pHandle->can_cfg->prescaler;
    can_init(pHandle->can_cfg->can_peri, &can_parameter);
	
	/* initialize filter */ 
	/*用几个过滤器就配置几个过滤器(can_filter更换filter_number重复配置几次)，
	  当前版本的驱动，仅支持每个CAN配置一个过滤器(过滤器多个CAN是复用的), 暂固定CAN0配置过滤器0，CAN1配置过滤器15，均关联FIFO0
	  具体关联哪个FIFO，就得开启对应的RX中断
	  必须至少开启一个过滤器，否则无法接收数据
	  CAN1的过滤单元BANK默认起始位置为14,可修改*/
    if(pHandle->can_cfg->can_peri == CAN0)
    {
        can_filter.filter_number = 0;
    }
    else
    {
        can_filter.filter_number = 15;
    }
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
	if(pHandle->can_cfg->en_data_filter == true)
	{
		can_filter.filter_list_high = (pHandle->can_cfg->filter_id >> 13) & 0xFFFF;
		can_filter.filter_list_low = ((pHandle->can_cfg->filter_id << 3) | (1 << 2)) & 0xFFFF;
		can_filter.filter_mask_high = (pHandle->can_cfg->filter_mask >> 13) & 0xFFFF;
		can_filter.filter_mask_low = ((pHandle->can_cfg->filter_mask << 3) | (1 << 2)) & 0xFFFF;
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
	
	if(pHandle->can_cfg->en_irq_tx == true)
	{
		nvic_irq_enable(pHandle->can_cfg->nvic_irq_tx, pHandle->can_cfg->irq_prio_tx, pHandle->can_cfg->irq_sub_prio_tx);  
        can_interrupt_enable(pHandle->can_cfg->can_peri, pHandle->can_cfg->irq_type_tx);    
	}
	
	if(pHandle->can_cfg->en_irq_rx0 == true)
	{
		nvic_irq_enable(pHandle->can_cfg->nvic_irq_rx0, pHandle->can_cfg->irq_prio_rx0, pHandle->can_cfg->irq_sub_prio_rx0);  
        can_interrupt_enable(pHandle->can_cfg->can_peri, pHandle->can_cfg->irq_type_rx0);  	
		
	}
	
	if(pHandle->can_cfg->en_irq_rx1 == true)
	{
		nvic_irq_enable(pHandle->can_cfg->nvic_irq_rx1, pHandle->can_cfg->irq_prio_rx1, pHandle->can_cfg->irq_sub_prio_rx1);  
        can_interrupt_enable(pHandle->can_cfg->can_peri, pHandle->can_cfg->irq_type_rx1);  	
	}
	
}


/**
  * @brief  CAN Send.
  * @param  Can_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
bool CanSend(Can_HandleType *pHandle)
{
	uint8_t ret = 0;
	can_trasnmit_message_struct transmit_message;
	
	if(pHandle->tx_frame_type == EN_CAN_FRAME_TYPE_STANDARD)
	{
		transmit_message.tx_sfid = pHandle->tx_id;
		transmit_message.tx_ff	 = CAN_FF_STANDARD;
	}
	else
	{
		transmit_message.tx_efid = pHandle->tx_id;
		transmit_message.tx_ff	 = CAN_FF_EXTENDED;
	}
	
	if(pHandle->tx_frame_kind == EN_CAN_FRAME_KIND_DATA)
	{
		transmit_message.tx_ft	 = CAN_FT_DATA;
		transmit_message.tx_dlen = pHandle->tx_len;
	}
	else
	{
		transmit_message.tx_ft	 = CAN_FT_REMOTE;
		transmit_message.tx_dlen = 0;
	}
	memcpy(transmit_message.tx_data, pHandle->tx_buf, transmit_message.tx_dlen);
	
	ret = can_message_transmit(pHandle->can_cfg->can_peri, &transmit_message);
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
void CanErrorClear(Can_HandleType *pHandle)
{
	if(can_flag_get(pHandle->can_cfg->can_peri, CAN_FLAG_MTE2) != RESET ||
	   can_flag_get(pHandle->can_cfg->can_peri, CAN_FLAG_MTE0) != RESET ||
	   can_flag_get(pHandle->can_cfg->can_peri, CAN_FLAG_MTE1) != RESET || 
	   can_flag_get(pHandle->can_cfg->can_peri, CAN_FLAG_PERR) != RESET ||
	   can_flag_get(pHandle->can_cfg->can_peri, CAN_FLAG_WERR) != RESET)
	{
		can_flag_clear(pHandle->can_cfg->can_peri, CAN_FLAG_MTE0);
		can_flag_clear(pHandle->can_cfg->can_peri, CAN_FLAG_MTE1);
		can_flag_clear(pHandle->can_cfg->can_peri, CAN_FLAG_MTE2);
		can_flag_clear(pHandle->can_cfg->can_peri, CAN_FLAG_PERR);
		can_flag_clear(pHandle->can_cfg->can_peri, CAN_FLAG_WERR);
		can_wakeup(pHandle->can_cfg->can_peri);
		CanInit(pHandle);
	}
}




/**
  * @brief  CAN tx irq process.
  * @param  Can_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanTxIrqProcess(Can_HandleType *pHandle)
{
	if(can_interrupt_flag_get(pHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF0) != RESET)
	{
		can_interrupt_flag_clear(pHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF0);
	}
	if(can_interrupt_flag_get(pHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF1) != RESET)
	{
		can_interrupt_flag_clear(pHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF1);
	}
	if(can_interrupt_flag_get(pHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF2) != RESET)
	{
		can_interrupt_flag_clear(pHandle->can_cfg->can_peri, CAN_INT_FLAG_MTF2);
	}
	else
	{
		return;
	}
	
	if(pHandle->irq_tx_process != NULL)
	{
		pHandle->irq_tx_process();
	}
}



/**
  * @brief  CAN rx0 irq process.
  * @param  Can_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanRx0IrqProcess(Can_HandleType *pHandle)
{
	can_receive_message_struct receive_message;
	can_message_receive(pHandle->can_cfg->can_peri, CAN_FIFO0, &receive_message);
	
	if(receive_message.rx_ff == CAN_FF_STANDARD)
	{
		pHandle->rx_id = receive_message.rx_sfid;
		pHandle->rx_frame_type = EN_CAN_FRAME_TYPE_STANDARD;
	}
	else
	{
		pHandle->rx_id = receive_message.rx_efid;
		pHandle->rx_frame_type = EN_CAN_FRAME_TYPE_EXTENDED;
	}
	if(receive_message.rx_ft == CAN_FT_DATA)
	{
		pHandle->rx_frame_kind = EN_CAN_FRAME_KIND_DATA;
		pHandle->rx_len = receive_message.rx_dlen;
		memcpy(pHandle->rx_buf, receive_message.rx_data, receive_message.rx_dlen);
	}
	else
	{
		pHandle->rx_frame_kind = EN_CAN_FRAME_KIND_REMOTE;
		pHandle->rx_len = 0;
	}
    if(pHandle->irq_rx0_process != NULL)
	{
		pHandle->irq_rx0_process();
	}
}
  
  
  /**
  * @brief  CAN rx1 irq process.
  * @param  Can_HandleType: CAN需要用户配置的参数
  * @return NULL
  * @note   none
  */
void CanRx1IrqProcess(Can_HandleType *pHandle)
{
	can_receive_message_struct receive_message;
	can_message_receive(pHandle->can_cfg->can_peri, CAN_FIFO1, &receive_message);
	
	if(receive_message.rx_ff == CAN_FF_STANDARD)
	{
		pHandle->rx_id = receive_message.rx_sfid;
		pHandle->rx_frame_type = EN_CAN_FRAME_TYPE_STANDARD;
	}
	else
	{
		pHandle->rx_id = receive_message.rx_efid;
		pHandle->rx_frame_type = EN_CAN_FRAME_TYPE_EXTENDED;
	}
	if(receive_message.rx_ft == CAN_FT_DATA)
	{
		pHandle->rx_frame_kind = EN_CAN_FRAME_KIND_DATA;
		pHandle->rx_len = receive_message.rx_dlen;
		memcpy(pHandle->rx_buf, receive_message.rx_data, receive_message.rx_dlen);
	}
	else
	{
		pHandle->rx_frame_kind = EN_CAN_FRAME_KIND_REMOTE;
		pHandle->rx_len = 0;
	}
    if(pHandle->irq_rx1_process != NULL)
	{
		pHandle->irq_rx1_process();
	}
}



