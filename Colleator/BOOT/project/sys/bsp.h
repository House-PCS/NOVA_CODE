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
#include "flash.h"
#include "n32g4fr.h"

extern Uart_HandleType WirelessUartHandle;
extern Gpio_HandleType WirelessResetIOHandle;
extern Uart_HandleType ModbusHostUartHandle;
extern Gpio_HandleType ModbusHostCtlIOHandle;
extern Spi_HandleType SpiFlashHandle;
extern Gpio_HandleType SpiFlashCsHandle;
extern Gpio_HandleType LedSysHandle;
extern Gpio_HandleType LedPcsHandle;
extern Gpio_HandleType LedConnHandle;
extern Gpio_HandleType KeyRstHandle;

void BaseBspInit();
void WirelessModbusBspInit();
void SpiFlashBspInit();
void ModbusHostBspInit();
void LedBspInit();
void KeyBspInit();





