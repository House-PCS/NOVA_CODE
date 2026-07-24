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
#ifndef __BSP_H_
#define __BSP_H_


#include "gpio.h"
#include "sim_i2c.h"
#include "exti.h"
#include "spi.h"
#include "uart.h"
#include "adc.h"
#include "dma.h"
#include "timer.h"
#include "can.h"
#include "mid_shell.h"
#include "debug.h"
//#include "EventRecorder.h"

extern Gpio_HandleType LED_Handle;
extern Uart_HandleType Uart3_Debug_Handle;
extern Uart_HandleType Uart0_Debug_Handle;
extern Uart_HandleType Uart1_ModS_Handle;

extern Spi_HandleType  SpiFlashHandle;
extern Gpio_HandleType CsSpiFlashHandle;
extern Gpio_HandleType WpSpiFlashHandle;

extern Gpio_HandleType DisCharge_Handle;
extern Gpio_HandleType Charge_Handle;
extern Gpio_HandleType PreCharge_Handle;
extern Gpio_HandleType CP_Handle;
extern Gpio_HandleType CP1_Handle;
extern Gpio_HandleType PMON_Handle;
extern Gpio_HandleType ExDisCharge_Handle;
extern Gpio_HandleType ExCharge_Handle;
extern Gpio_HandleType PowerHoldHandle;
extern Gpio_HandleType ExtWdgInHandle;
extern Adc_HandleType Adc_Handle;

extern Timer_HandleType LampHandle;
extern Gpio_HandleType KeySwitchHandle;

void BspInit(void);
void SpiFlashBspInit(void);
void Uart0DebugBspInit(void);
void Uart1ModsBspInit(void);
void ADCBspInit(void);
void KeyBspInit(void);
void AFETimerInit(void);


#endif



