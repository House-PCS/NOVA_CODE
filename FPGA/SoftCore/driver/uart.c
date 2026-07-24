/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
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


#include "uart.h"

#define MCLK            (50*1000*1000)
#define UART_BUAD       (115200)
#define UART0           ((CMSDK_UART_TypeDef   *) 0x41001000   )

void (*Uart0IrqRxProcess)();

void Uart0SendByte(char dat)
{
    while ((UART0->STATE & 1));
    UART0->DATA = dat;
}

void Uart0Init(void)
{
    UART0->BAUDDIV = MCLK / UART_BUAD;
    UART0->CTRL    = 0x0B;
    
    NVIC_EnableIRQ(INT01_IRQn);
}

void Uart0Send(uint8_t *data, uint16_t len)
{
    uint16_t i = 0;
    
    for(i = 0; i < len; i++)
    {
        Uart0SendByte(data[i]);
    }
}

uint8_t Uart0DataGet(void)
{
    return UART0->DATA;
}

void INT01_Handler()
{
    if(((UART0->INTSTATUS & 2) >> 1) != 0)
    {
        while(!(UART0->STATE & 2));
        UART0->INTCLEAR = 2;       
        Uart0IrqRxProcess();        
    }        
}
