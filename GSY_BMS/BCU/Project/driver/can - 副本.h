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

#include "gd32f4xx_gpio.h"
#include "gd32f4xx_can.h"
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
//    uint32_t                tx_alt_num;
    
    uint32_t                rx_gpiox;
    uint32_t                rx_gpio_pinx;
//    uint32_t                rx_alt_num;
//    uint8_t                 alt_rmp_mode;/*1:Alternate1, 2:Alternate2, 3:Remap*/
//    uint32_t                alt_rmp_function;/*alt_rmp_mode=1时表示afio_function， alt_rmp_mode=3时表示remap function, alt_rmp_mode为其他值，该字段无意义*/
    
    rcu_periph_enum         rcu_can;
    uint32_t                can_peri;
   
    bool                    en_irq_rx;
    uint32_t                nvic_irq_rx;
    bool                    en_irq_tx;
    uint32_t                nvic_irq_tx;
    bool                    en_irq_er;
    uint32_t                nvic_irq_er;
    
    
    uint8_t                 resync_jump_width;                 /*!< CAN resynchronization jump width */
    uint8_t                 time_segment_1;                    /*!< time segment 1 */
    uint8_t                 time_segment_2;                    /*!< time segment 2 */
    uint16_t                prescaler;                         /*!< baudrate prescaler 1M/500K/250K/125K/100K/50k/20k(5/20/20/40/50/100/250)*/
            
}Can_Config;


typedef struct _Can_handle_stru_t
{
    uint8_t             canx;
    Can_Config          *can_cfg;
    uint8_t             can_fifox;
    
    /* CAN transmit message structure */
    can_trasnmit_message_struct transmit_message;
    
    /* CAN receive message structure */
    can_receive_message_struct receive_message;
    
    void                (*irq_rx_process)();
}Can_HandleType;

void CanInit(Can_HandleType *pCanHandle);
bool CanSend(Can_HandleType *pCanHandle);
void CanIrqProcess(Can_HandleType *pCanHandle);
#endif















