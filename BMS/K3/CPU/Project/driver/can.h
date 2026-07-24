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


typedef struct _can_config_stru_t
{
    uint32_t                tx_gpiox;
    uint32_t                tx_gpio_pinx;
    rcu_periph_enum         rcu_tx_gpio;
//    uint32_t                tx_alt_num;
    
    uint32_t                rx_gpiox;
    uint32_t                rx_gpio_pinx;
    rcu_periph_enum         rcu_rx_gpio;
//    uint32_t                rx_alt_num;
    bool                    en_gpio_af;
    
    rcu_periph_enum         rcu_can;
    uint32_t                can_peri;
    bool                    en_irq_rx;
    uint32_t                nvic_irq_rx;
    bool                    en_irq_tx;
    uint32_t                nvic_irq_tx;
    bool                    en_irq_er;
    uint32_t                nvic_irq_er;
//    uint8_t                 irq_prio;
//    uint8_t                 irq_sub_prio;
//    uint8_t                 irq_type;
    
//    uint8_t                 working_mode;                      /*!< CAN working mode */ 
    uint8_t                 resync_jump_width;                 /*!< CAN resynchronization jump width */
    uint8_t                 time_segment_1;                    /*!< time segment 1 */
    uint8_t                 time_segment_2;                    /*!< time segment 2 */
//    ControlStatus           time_triggered;                    /*!< time triggered communication mode */
//    ControlStatus           auto_bus_off_recovery;             /*!< automatic bus-off recovery */
//    ControlStatus           auto_wake_up;                      /*!< automatic wake-up mode */
//    ControlStatus           no_auto_retrans;                   /*!< automatic retransmission mode disable */
//    ControlStatus           rec_fifo_overwrite;                /*!< receive FIFO overwrite mode disable */
//    ControlStatus           trans_fifo_order;                  /*!< transmit FIFO order */
    uint16_t                prescaler;                         /*!< baudrate prescaler 1M/500K/250K/125K/100K/50k/20k(5/20/20/40/50/100/250)*/
    
//    uint16_t                filter_list_high;                  /*!< filter list number high bits */
//    uint16_t                filter_list_low;                   /*!< filter list number low bits */
//    uint16_t                filter_mask_high;                  /*!< filter mask number high bits */
//    uint16_t                filter_mask_low;                   /*!< filter mask number low bits */
//    uint16_t                filter_fifo_number;                /*!< receive FIFO associated with the filter */
//    uint16_t                filter_number;                     /*!< filter number */
//    uint16_t                filter_mode;                       /*!< filter mode, list or mask */
//    uint16_t                filter_bits;                       /*!< filter scale */
//    ControlStatus           filter_enable;                     /*!< filter work or not */
        
}Can_Config;


typedef struct _Can_handle_stru_t
{
    EN_CANx_PORT        canx;
    Can_Config          *can_cfg;
    uint8_t             can_fifox;
    
    /* CAN transmit message structure */
    can_trasnmit_message_struct transmit_message;
//    uint32_t                tx_sfid;                           /*!< standard format frame identifier */
//    uint32_t                tx_efid;                           /*!< extended format frame identifier */
//    uint8_t                 tx_ff;                             /*!< format of frame, standard or extended format */
//    uint8_t                 tx_ft;                             /*!< type of frame, data or remote */
//    uint8_t                 tx_dlen;                           /*!< data length */
//    uint8_t                 tx_data[8];                        /*!< transmit data */
    
    /* CAN receive message structure */
    can_receive_message_struct receive_message;
//    uint32_t                rx_sfid;                           /*!< standard format frame identifier */
//    uint32_t                rx_efid;                           /*!< extended format frame identifier */
//    uint8_t                 rx_ff;                             /*!< format of frame, standard or extended format */
//    uint8_t                 rx_ft;                             /*!< type of frame, data or remote */
//    uint8_t                 rx_dlen;                           /*!< data length */
//    uint8_t                 rx_data[8];                        /*!< receive data */
//    uint8_t                 rx_fi;                             /*!< filtering index */
    
    void                (*irq_rx_process)();
}Can_HandleType;

void CanInit(Can_HandleType *pCanHandle);
bool CanSend(Can_HandleType *pCanHandle);
void CanIrqProcess(Can_HandleType *pCanHandle);
#endif















