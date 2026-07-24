/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-20
  * @brief       N32G4FR
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

#include "stdbool.h"
#include "stdio.h"
#include "drv_opt.h"
#include "dma.h"
#include "n32g4fr_usart.h"

typedef enum
{
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
}EN_UARTx_PORT;

//typedef enum
//{
//    EN_UART_INT_TYPE_PERR       = 0x01,
//    EN_UART_INT_TYPE_TBE        = 0x02,
//    EN_UART_INT_TYPE_TC         = 0x04,
//    EN_UART_INT_TYPE_RBNE       = 0x08,
//    EN_UART_INT_TYPE_IDLE       = 0x10,
//    EN_UART_INT_TYPE_LBD        = 0x20,
//    EN_UART_INT_TYPE_ERR        = 0x40,    
//    EN_UART_INT_TYPE_CTS        = 0x80, 
//    EN_UART_INT_TYPE_RT         = 0x100, 
//    EN_UART_INT_TYPE_EB         = 0x200, 
//    EN_UART_INT_TYPE_NUM        = 10
//}EN_UART_INT_TYPE;

typedef enum
{
    EN_UART_INT_TYPE_PEF        = 0x01,
    EN_UART_INT_TYPE_TXDE       = 0x02,
    EN_UART_INT_TYPE_TXC        = 0x04,
    EN_UART_INT_TYPE_RXDNE      = 0x08,
    EN_UART_INT_TYPE_IDLEF      = 0x10,
    EN_UART_INT_TYPE_LINBD      = 0x20,
    EN_UART_INT_TYPE_ERRF       = 0x40,    
    EN_UART_INT_TYPE_CTSF       = 0x80, 
    EN_UART_INT_TYPE_OREF       = 0x100, 
    EN_UART_INT_TYPE_NEF        = 0x200, 
    EN_UART_INT_TYPE_FEF        = 0x400, 
    EN_UART_INT_TYPE_NUM        = 11
}EN_UART_INT_TYPE;

typedef struct _uart_config_stru_t
{
    GPIO_Module*            tx_gpiox;
    uint32_t                tx_gpio_pinx;
    
    GPIO_Module*            rx_gpiox;
    uint32_t                rx_gpio_pinx;
    
    uint32_t                rcc_uart;
    USART_Module*           uart_peri;
    
    uint8_t                 alt_rmp_mode;/*1:Alternate1, 2:Alternate2, 3:Remap*/
    uint32_t                alt_rmp_function;/*alt_rmp_mode=1时表示afio_function， alt_rmp_mode=3时表示remap function, alt_rmp_mode为其他值，该字段无意义*/
    
    uint32_t                baud_rate;
    uint32_t                wlen; 
    uint32_t                stblen; 
    uint32_t                parity_cfg;
                
    bool                    en_irq;
    uint8_t                 nvic_irq;
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
    EN_UART_INT_TYPE        irq_type;
    
    /*DMA相关参数*/
    bool                    en_dma_tx;
    bool                    en_dma_rx;
    bool                    en_dma_tx_nowait;
    
    DMA_ChannelType*        dma_tx_peri_channel;//bsp无需配置，驱动自行计算
    DMA_Module*             dma_tx_peri;
    dma_channel_enum        dma_tx_channel;
	bool                    en_dma_tx_irq;
    uint8_t                 nvic_dma_tx_irq;
    uint8_t                 irq_dma_tx_prio;
    uint8_t                 irq_dma_tx_sub_prio;
    
    DMA_ChannelType*        dma_rx_peri_channel;//bsp无需配置，驱动自行计算
    DMA_Module*             dma_rx_peri;
    dma_channel_enum        dma_rx_channel;
    bool                    en_dma_rx_irq;
    uint8_t                 nvic_dma_rx_irq;
    uint8_t                 irq_dma_rx_prio;
    uint8_t                 irq_dma_rx_sub_prio;
    
}Uart_Config;


typedef struct _uart_handle_stru_t
{
    EN_UARTx_PORT       uartx;
    Uart_Config         *uart_cfg;
    
    uint8_t             *pTxBuf;
    uint32_t            tx_size;
    uint8_t             *pRxBuf;
    uint32_t            rx_size;
    
    void                (*irq_rx_process)();
    void                (*dma_tx_complete_process)();
}Uart_HandleType;






void UartInit(Uart_HandleType *pUartHandle);
uint16_t UartDMARxDataLength(Uart_HandleType *pUartHandle);
void UartDMARxEnable(Uart_HandleType *pUartHandle);
void UartDMASend(Uart_HandleType *pUartHandle);
void UartNormalSend(Uart_HandleType *pUartHandle);
void UartSend(Uart_HandleType *pUartHandle);
void UartIrqProcess(Uart_HandleType *pUartHandle);
void UartDMATxIrqProcess(Uart_HandleType *pUartHandle);
void UartDMARxDisable(Uart_HandleType *pUartHandle);
void UartDMARxEnable(Uart_HandleType *pUartHandle);
void UartDMARxEnable(Uart_HandleType *pUartHandle);
bool UartSendCompleteFlag(Uart_HandleType *pUartHandle);


bool UartSendCompleteFlag(Uart_HandleType *pUartHandle);

#endif


