/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.h
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


#ifndef __CAN_H_
#define __CAN_H_

#include "gd32f30x_can.h"
#include "gd32f30x_gpio.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "drv_opt.h"



typedef enum
{
#if USE_CAN0
    CAN0_PORT,
#endif
#if USE_CAN1
    CAN1_PORT,
#endif
    CAN_NUM
}EN_CANx_PORT;


typedef enum
{
    EN_CAN_FRAME_TYPE_STANDARD,
    EN_CAN_FRAME_TYPE_EXTENDED,
} EN_CAN_FRAME_TYPE;

typedef enum
{
    EN_CAN_FRAME_KIND_DATA,
    EN_CAN_FRAME_KIND_REMOTE,
} EN_CAN_FRAME_KIND;

typedef struct _can_config_stru_t
{
    uint32_t                tx_gpiox;
    uint32_t                tx_gpio_pinx;
    rcu_periph_enum         rcu_tx_gpio;
    
    uint32_t                rx_gpiox;
    uint32_t                rx_gpio_pinx;
    rcu_periph_enum         rcu_rx_gpio;
	
    uint8_t                 alt_rmp_mode;/*1:Alternate1, 2:Alternate2, 3:Remap*/
    uint32_t                alt_rmp_function;/*alt_rmp_mode=1时表示afio_function， alt_rmp_mode=3时表示remap function, alt_rmp_mode为其他值，该字段无意义*/
                                             /*remap function包含GPIO_CAN_PARTIAL_REMAP, GPIO_CAN_FULL_REMAP*/
    
    rcu_periph_enum         rcu_can;
    uint32_t                can_peri;
    
//    uint8_t                 working_mode;                      /*!< CAN working mode */ 
    uint8_t                 resync_jump_width;                 /*!< CAN resynchronization jump width */
    uint8_t                 time_segment_1;                    /*!< time segment 1 */
    uint8_t                 time_segment_2;                    /*!< time segment 2 */

    uint16_t                prescaler;                         /*!< baudrate prescaler 1M/500K/250K/125K/100K/50k/20k(5/20/20/40/50/100/250)*/
    bool					en_data_filter;	/*如开启数据过滤，则非当前规定格式的数据不接收*/
	
	uint32_t				filter_id; /*低两个字节表示主从机地址*/
    uint32_t				filter_mask;/*低两个字节表示主从机地址，可通过地址进行掩码设置*/

    bool                    en_irq_tx;
    uint32_t                nvic_irq_tx;
    uint8_t                 irq_prio_tx;
    uint8_t                 irq_sub_prio_tx;
    uint32_t                irq_type_tx;

    bool                    en_irq_rx0;
    uint32_t                nvic_irq_rx0;
    uint8_t                 irq_prio_rx0;
    uint8_t                 irq_sub_prio_rx0;
    uint32_t                irq_type_rx0;

    bool                    en_irq_rx1;
    uint32_t                nvic_irq_rx1;
    uint8_t                 irq_prio_rx1;
    uint8_t                 irq_sub_prio_rx1;
    uint32_t                irq_type_rx1;    
}Can_Config;


typedef struct _can_handle_stru_t
{
    EN_CANx_PORT        	canx;
    Can_Config          	*can_cfg;

    uint32_t                tx_id;
    uint8_t                 tx_len;
    EN_CAN_FRAME_TYPE		tx_frame_type;
    EN_CAN_FRAME_KIND		tx_frame_kind;
    uint8_t                 tx_buf[8];
//    can_trasnmit_message_struct transmit_message;
    uint32_t                rx_id;
    uint8_t                 rx_len;
    EN_CAN_FRAME_TYPE       rx_frame_type;
    EN_CAN_FRAME_KIND       rx_frame_kind;
    uint8_t                 rx_buf[8];
//    can_receive_message_struct receive_message;
    void                	(*irq_tx_process)();
    void                	(*irq_rx0_process)();
    void                	(*irq_rx1_process)();
} Can_HandleType;

void CanInit(Can_HandleType *pCanHandle);
bool CanSend(Can_HandleType *pCanHandle);
void CanIrqProcess(Can_HandleType *pCanHandle);
void CanRx0IrqProcess(Can_HandleType *pCanHandle);
#endif















