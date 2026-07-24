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
  * <tr><td>2023-06-15    <td>V1.0.1    <td>           <td>增加同一串口可使用多种中断并存的机制
  * </table>
  ******************************************************************************
  */


#ifndef __UART_H_
#define __UART_H_

#include "gd32f4xx_gpio.h"
#include "gd32f4xx_usart.h"
#include "gd32f4xx_dma.h"
#include "stdbool.h"
#include "stdio.h"
#include "drv_opt.h"



typedef enum
{
#if USE_UART0
    UART0_PORT,
#endif
#if USE_UART1
    UART1_PORT,
#endif
#if USE_UART2
    UART2_PORT,
#endif
#if USE_UART3  
    UART3_PORT,
#endif
#if USE_UART4
    UART4_PORT,
#endif
#if USE_UART5
    UART5_PORT,
#endif
#if USE_UART6
    UART6_PORT,
#endif
#if USE_UART7  
    UART7_PORT,
#endif
    UART_NUM
} EN_UARTx_PORT;

typedef enum
{
    EN_UART_INT_TYPE_PERR       = 0x01,
    EN_UART_INT_TYPE_TBE        = 0x02,
    EN_UART_INT_TYPE_TC         = 0x04,
    EN_UART_INT_TYPE_RBNE       = 0x08,
    EN_UART_INT_TYPE_IDLE       = 0x10,
    EN_UART_INT_TYPE_LBD        = 0x20,
    EN_UART_INT_TYPE_ERR        = 0x40,    
    EN_UART_INT_TYPE_CTS        = 0x80, 
    EN_UART_INT_TYPE_RT         = 0x100, 
    EN_UART_INT_TYPE_EB         = 0x200, 
    EN_UART_INT_TYPE_NUM        = 10
} EN_UART_INT_TYPE;



typedef struct _uart_config_stru_t
{
    uint32_t                tx_gpiox;
    uint32_t                tx_gpio_pinx;
    uint32_t                tx_alt_num;

    uint32_t                rx_gpiox;
    uint32_t                rx_gpio_pinx;
    uint32_t                rx_alt_num;
     
    rcu_periph_enum         rcu_uart;
    uint32_t                uart_peri;


    uint32_t                baud_rate;
    uint32_t                wlen; 
    uint32_t                stblen; 
    uint32_t                parity_cfg;
                
    bool                    en_irq;
    uint32_t                nvic_irq;
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
    uint32_t                irq_type;

    /*DMA相关参数*/
    bool                    en_dma_tx;
    bool                    en_dma_rx;
    bool                    en_dma_tx_nowait;

    uint32_t                dma_tx_peri;
    dma_channel_enum        dma_tx_channel;
    dma_subperipheral_enum  dma_tx_subperi;
    bool                    en_dma_tx_irq;
    uint32_t                nvic_dma_tx_irq;
    uint8_t                 irq_dma_tx_prio;
    uint8_t                 irq_dma_tx_sub_prio;

    uint32_t                dma_rx_peri;
    dma_channel_enum        dma_rx_channel;
    dma_subperipheral_enum  dma_rx_subperi;
    bool                    en_dma_rx_irq;
    uint32_t                nvic_dma_rx_irq;
    uint8_t                 irq_dma_rx_prio;
    uint8_t                 irq_dma_rx_sub_prio;
    

} Uart_Config;


typedef struct _uart_handle_stru_t
{
    uint8_t             uartx;
    Uart_Config         *uart_cfg;

    uint8_t             *pTxBuf;
    uint16_t            tx_size;
    uint8_t             *pRxBuf;
    uint16_t            rx_size;
    void                (*irq_rx_process)();
    void                (*dma_tx_complete_process)();
} Uart_HandleType;






void UartInit(Uart_HandleType *pUartHandle);
uint16_t UartDMARxDataLength(Uart_HandleType *pUartHandle);
void UartDMASend(Uart_HandleType *pUartHandle);
void UartNormalSend(Uart_HandleType *pUartHandle);
void UartSend(Uart_HandleType *pUartHandle);
void UartIrqProcess(Uart_HandleType *pUartHandle);
void UartDMATxIrqProcess(Uart_HandleType *pUartHandle);
void UartDMARxDisable(Uart_HandleType *pUartHandle);
void UartDMARxEnable(Uart_HandleType *pUartHandle);
bool UartSendCompleteFlag(Uart_HandleType *pUartHandle);
bool UartSendBufEmptyFlag(Uart_HandleType *pUartHandle);
void UartClearCompleteFlag(Uart_HandleType *pUartHandle);
void UartClearSendBufEmptyFlag(Uart_HandleType *pUartHandle);

#endif















