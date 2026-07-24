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


extern Uart_HandleType WirelessUartHandle;
extern Gpio_HandleType WirelessResetIOHandle;
extern Gpio_HandleType WirelessPowerEnIOHandle;
extern Uart_HandleType ModbusHostUartHandle;
extern Gpio_HandleType ModbusHostCtlIOHandle;
extern Spi_HandleType SpiFlashHandle;
extern Gpio_HandleType SpiFlashCsHandle;
extern Gpio_HandleType LedSysHandle;
extern Gpio_HandleType LedPcsHandle;
extern Gpio_HandleType LedConnHandle;
extern Gpio_HandleType KeyRstHandle;
extern Gpio_HandleType BoardIDGpioHandle;

void BaseBspInit();
void WirelessBspInit();
void ModbusHostBspInit();
void SpiFlashBspInit();
void LedBspInit();
void KeyBspInit();
void BoardIDBspInit();


extern Uart_HandleType DbgUartHandle;
void DbgUartBspInit();
