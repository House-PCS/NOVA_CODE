/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.h
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
#include "gpio.h"
//#include "exti.h"
#include "spi.h"
#include "uart.h"
#include "dma.h"
#include "timer.h"

#define DBU_UPDATE_VERIFY   1


extern Gpio_HandleType AtmosLedHandle[6];
extern Gpio_HandleType SideAtmosLedHandle[2];
extern Timer_HandleType TimerPollingHandle;
extern Uart_HandleType ModbusHostUartHandle;
extern Gpio_HandleType ModbusHostCtlIOHandle;
extern Uart_HandleType DbgUartHandle;
extern Timer_HandleType TimerModbusPollingHandle;



void BaseBspInit(void);
void ModbusHostBspInit(void);
void AtmosLedBspInit(void);
void TimerPollingBspInit(void);
void DbgUartBspInit(void);
void SideLedBspInit(void);
void TimerModbusPollingBspInit(void);


extern Gpio_HandleType TESTIOHandle;
void TestIOInit(void);

void SetTestIo(void);
void ReSetTestIo(void);
