/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-22
  * @brief       GD32F450
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __BSP_H_
#define __BSP_H_


#include "adc.h"
#include "gpio.h"
#include "exti.h"
#include "spi.h"
#include "uart.h"
#include "dma.h"
#include "timer.h"
#include "string.h"
#include "stdint.h"
#include "exmc.h"
#include "i2c.h"
#include "sim_i2c.h"


#define  SWITCH_LED_HIGH_ENABLE   1

extern Exmc_HandleType ExmcHandle;
extern Gpio_HandleType LcdRstnHandle;
extern Gpio_HandleType LcdBLHandle;

extern Spi_HandleType SpiFlashHandle;
extern Gpio_HandleType SpiFlashCsHandle;
extern Gpio_HandleType SpiFlashWpHandle;

extern Uart_HandleType ModbusHostBMSUartHandle;
extern Gpio_HandleType ModbusHostBMSDEHandle;
extern Uart_HandleType ModbusHostPCSUartHandle;
extern Gpio_HandleType ModbusHostPCSDEHandle;
extern Uart_HandleType ModbusSlaveCollectorUartHandle;
extern Gpio_HandleType ModbusSlaveCollectorDEHandle;
extern Gpio_HandleType KeyLeftHandle;
extern Gpio_HandleType KeyRightHandle;
extern Gpio_HandleType KeyConfirmHandle;
extern Gpio_HandleType KeyEscHandle;
//extern I2C_HandleType RtcHandle;
extern SimI2C_HandleType RtcHandle;
extern Gpio_HandleType RtcIntHandle;

extern Gpio_HandleType AtmosChxHandle[];
extern Timer_HandleType TimerPollingHandle;

extern Gpio_HandleType KeyLeftLedHandle;
extern Gpio_HandleType KeyLeftHandle;

extern SimI2C_HandleType TCA9555I2CHandle;
extern Gpio_HandleType TCA9555IntHandle;

extern Gpio_HandleType MBI502xLedClkHandle;
extern Gpio_HandleType MBI502xLedDataHandle;
extern Gpio_HandleType MBI502xLedLatchHandle;
extern Gpio_HandleType MBI502xLedOEHandle;

//void ExmcBspInit();
void SdramPreInit(void);//用于main函数之前的初始化
void SdramBspInit(void);
void SramBspInit(void);
void ExmcBspInit(void);
void LcdCtlBspInit(void);

void SpiFlashBspInit(void);

void ModbusHostBMSBspInit(void);
void ModbusHostPCSBspInit(void);
void ModbusHostCollectorBspInit(void);
void KeyBspInit(void);

void AtmosBspInit(void);
void TimerPollingBspInit(void);
void TimerPollingBspDeInit(void);
void RtcBspInit(void);

void BaseBspInit(void);
void TCA9555BspInit(void);

void MBI502xLedBspInit(void);
#endif








