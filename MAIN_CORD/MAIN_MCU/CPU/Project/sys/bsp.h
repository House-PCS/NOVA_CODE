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
#include "can.h"

#define  CONNECT_GPIO_NUM		(8)
#define SWITCH_LED_HIGH_ENABLE  (1)

#define TEST_MODE               (0)

#define UNKNOWN_TYPE                            (0)
#define DEVICE_TYPE_LCD                         (1)
#define DEVICE_TYPE_LED                         (2)

extern int32_t  gDeviceVersionType;

extern Exmc_HandleType ExmcHandle;
extern Gpio_HandleType LcdRstnHandle;
extern Gpio_HandleType LcdBLHandle;

extern Spi_HandleType SpiFlashHandle;
extern Gpio_HandleType SpiFlashCsHandle;
extern Gpio_HandleType SpiFlashWpHandle;

extern Uart_HandleType ModbusSlaveCollectorUartHandle;
extern Gpio_HandleType ModbusSlaveCollectorDEHandle;
extern Uart_HandleType ModbusSlaveVoltmeterArcUartHandle;
extern Gpio_HandleType ModbusSlaveVoltmeterArcDEHandle;
extern Uart_HandleType ModbusHostCoprocessorUartHandle;
extern Uart_HandleType Uart0_Test_Handle;
extern Uart_HandleType ModbusHostPCSUartHandle;
extern Gpio_HandleType ModbusHostPCSDEHandle;


extern Gpio_HandleType KeyLeftHandle;
extern Gpio_HandleType KeyRightHandle;
extern Gpio_HandleType KeyConfirmHandle;
extern Gpio_HandleType KeyEscHandle;
//extern I2C_HandleType RtcHandle;
extern SimI2C_HandleType RtcHandle;
extern Gpio_HandleType RtcIntHandle;

extern Gpio_HandleType AtmosChxHandle[];
extern Gpio_HandleType SwitchLedHandle;
extern Timer_HandleType TimerPollingHandle;
extern Timer_HandleType CommTimerPollingHandle;
extern Gpio_HandleType KeyLeftLedHandle;
extern Gpio_HandleType KeyLeftHandle;

extern Gpio_HandleType MBI502xLedClkHandle;
extern Gpio_HandleType MBI502xLedDataHandle;
extern Gpio_HandleType MBI502xLedLatchHandle;
extern Gpio_HandleType MBI502xLedOEHandle;
extern Gpio_HandleType McuVerID1Handle;
extern Gpio_HandleType McuVerID2Handle;

extern Gpio_HandleType ConnectGPIOHandle[CONNECT_GPIO_NUM];
extern Uart_HandleType DebugUart0Handle;
extern Gpio_HandleType DRM0ConnectIOHandle;
extern Gpio_HandleType McuWorkLedHandle;
extern Gpio_HandleType DRM1ConnectIOHandle;
extern Gpio_HandleType DRM2ConnectIOHandle;
extern Gpio_HandleType DRM3ConnectIOHandle;
extern Gpio_HandleType DRM4ConnectIOHandle;
extern Gpio_HandleType OutputGPIOHandle[2];



//void ExmcBspInit();
void SdramPreInit(void);//用于main函数之前的初始化
void SdramBspInit(void);
void SramBspInit(void);
void ExmcBspInit(void);
void LcdCtlBspInit(void);

void SpiFlashBspInit(void);

void ModbusHostCollectorBspInit(void);
void ModbusHostVoltmeterArcBspInit(void);
void ModbusHostCoprocessorBspInit(void);
void ModbusHostPCSBspInit(void);
void Uart0testBspInit(void);
void KeyBspInit(void);
void BspSwitchLedInit(void);
void TimerPollingBspInit(void);
void CommTimerPollingBspInit(void);
void RtcBspInit(void);

void BaseBspInit(void);
void MBI502xLedBspInit(void);
void McuVersionBspInit(void);

void ConnectGPIOBspInit(void);
void DebugUart0BspInit(void);
void DRMConnectIOInit(void);
void McuWorkLedBspInit(void);

void OutputGPIOBspInit(void);
#endif








