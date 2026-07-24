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


#ifndef __UART_H_
#define __UART_H_

#include "gd32f30x_usart.h"
#include "gd32f30x_gpio.h"
#include "stdbool.h"
//#include "stdio.h"
//#include "drv_opt.h"



//typedef enum
//{
//#if USE_UART0
//    UART0_PORT,
//#endif
//#if USE_UART1
//    UART1_PORT,
//#endif
//#if USE_UART2
//    UART2_PORT,
//#endif
//#if USE_UART3  
//    UART3_PORT,
//#endif
//#if USE_UART4
//    UART4_PORT,
//#endif
//#if USE_UART5
//    UART5_PORT,
//#endif
//#if USE_UART6
//    UART6_PORT,
//#endif
//#if USE_UART7  
//    UART7_PORT,
//#endif
//    UART_NUM
//}EN_UARTx_PORT;





typedef struct _uart_config_stru_t
{
    uint32_t                uart_peri;	//USARTx
	rcu_periph_enum         rcu_uart;		//时钟选择
    
    uint32_t                tx_gpiox;
    uint32_t                tx_gpio_pinx;
    rcu_periph_enum         tx_gpio_rcu;		//时钟选择  
    uint32_t                rx_gpiox;
    uint32_t                rx_gpio_pinx;
    rcu_periph_enum         rx_gpio_rcu;		//时钟选择    
    bool                    en_gpio_af;
    uint32_t                gpio_remap;
    
    uint32_t                baud_rate;
    uint32_t                wlen; 
    uint32_t                stblen; 
    uint32_t                parity_cfg;
                
    /*中断相关，使用DMA发送中断方式+DMA接收超时中断方式*/
    uint32_t                nvic_irq;
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
//    usart_interrupt_enum    irq_type;
    
    /*DMA相关参数*/
    uint32_t                dma_peri;   
	rcu_periph_enum         rcu_dma;
    
    dma_channel_enum        dma_tx_channel;
    
    uint8_t                nvic_dma_irq;		//中断函数索引
    uint8_t                 irq_dma_prio;		
    uint8_t                 irq_dma_sub_prio;	
    dma_channel_enum        dma_rx_channel;
    
}Uart_Config;
typedef enum
{
    IDLE=0,
    SEND_ING,
    SEND_STOP,
}EN_SEND_STATE;
typedef struct {
	EN_SEND_STATE   Sending;		//发送状态，0:空闲，1:发送中，2:发送结束中（485延时切换状态）
	bool            ReceiveFlg;	//接收完成标志
	uint16_t        ReceiveNum;
    uint32_t        send_time;
}UART_RXTX_STATE;
typedef struct _uart_handle_stru_t
{
    uint8_t       uartx;
    Uart_Config         *uart_cfg;
    
    uint8_t             *pTxBuf;
//    uint32_t            tx_size;
    uint8_t             *pRxBuf;
    uint16_t            rx_max;
    UART_RXTX_STATE     UartRxTxState;
//    void                (*irq_rx_process)();
    bool            (*tx_process)(uint16_t);
}Uart_HandleType;


void UartInit(Uart_HandleType *pUartHandle);
void UartSendDMA(const Uart_HandleType *pUartHandle,uint16_t len);

#endif


