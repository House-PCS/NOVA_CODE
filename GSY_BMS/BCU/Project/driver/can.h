/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can.h
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
#ifndef __CAN_H_
#define	__CAN_H_

#include "drv_opt.h"
#include "gd32f4xx_can.h"
#include <stdbool.h>
#include <String.h>

typedef enum
{
#if USE_CAN0
    CAN0_PORT,
#endif
#if USE_CAN1
    CAN1_PORT,
#endif
    CAN_PORT_NUM
} EN_CANx_PORT;



#define CAN_BPS_250K    24
#define CAN_BPS_500K    12

typedef enum
{
    EN_CAN_FRAME_TYPE_STANDARD,
    EN_CAN_FRAME_TYPE_EXTENDED
} EN_CAN_FRAME_TYPE;

typedef enum
{
    EN_CAN_FRAME_KIND_DATA,
    EN_CAN_FRAME_KIND_REMOTE
} EN_CAN_FRAME_KIND;

typedef struct _can_config_stru_t
{
    uint32_t                tx_gpiox;
    uint32_t                tx_gpio_pinx;
    uint32_t                tx_alt_num;

    uint32_t                rx_gpiox;
    uint32_t                rx_gpio_pinx;
    uint32_t                rx_alt_num;

    rcu_periph_enum         rcu_can;
    uint32_t                can_peri;


    /* baudrate = APB1CLK / ((SJW + BS1 + BS2) * prescaler)  */
    uint16_t                prescaler;                         /*!< baudrate prescaler 1M/500K/250K/125K/100K/50k/20k(5/20/20/40/50/100/250)*/
    // uint8_t                 resync_jump_width;                 /*!< CAN resynchronization jump width */
    // uint8_t                 time_segment_1;                    /*!< time segment 1 */
    // uint8_t                 time_segment_2;                    /*!< time segment 2 */

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
	
            
} Can_Config;

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


void CanInit(Can_HandleType *pHandle);
bool CanSend(Can_HandleType *pHandle);

void CanRx0IrqProcess(Can_HandleType *pHandle);
void CanRx1IrqProcess(Can_HandleType *pHandle);

#endif 










