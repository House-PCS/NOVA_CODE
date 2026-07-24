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
#define BQHWI2C      1
#define USE_SIM_IIC  1

#include "gpio.h"
#include "sim_i2c.h"
#include "i2c.h"
#include "exti.h"
#include "spi.h"
#include "uart.h"
#include "adc.h"
#include "dma.h"
#include "timer.h"
#include "mid_shell.h"
#include "debug.h"
#include "dev_ext_wdg.h"
#include "afe_control.h"
#include "dev_ntc.h"

extern Adc_HandleType  Adc_Handle;
extern Uart_HandleType Uart1_Fpga_Sofcore_Handle;
extern Spi_HandleType  SpiFlashHandle;
extern Gpio_HandleType CsSpiFlashHandle;
extern Uart_HandleType Uart0_Handle;
extern Gpio_HandleType VBENHandle;
extern Gpio_HandleType Bq76952WpHandle;
extern Gpio_HandleType E2promWpHandle;
extern Gpio_HandleType MCU_LED_Handle;
extern Gpio_HandleType KEY_LED_EN_Handle;
extern Gpio_HandleType TempS2Handle;
extern Gpio_HandleType TempS1Handle;
extern Gpio_HandleType TempS0Handle;
extern Gpio_HandleType KeyCheckHandle;
extern Gpio_HandleType PowerHoldHandle;
extern Gpio_HandleType HeatCkHandle;
extern Gpio_HandleType HeatEnHandle;
extern Gpio_HandleType IDADCEnHandle;
extern Gpio_HandleType ErrorCkHandle;
extern Gpio_HandleType PmosEnHandle;
extern Gpio_HandleType CmosEnHandle;
extern Gpio_HandleType DmosEnHandle;
extern Timer_HandleType AFESampleTimeHandle;
extern Timer_HandleType OsSchTimeHandle;
extern Gpio_HandleType ExtWdgInHandle;
extern Gpio_HandleType ExtWdgOutHandle;
extern Gpio_HandleType AfeTs2Handle;

#if BQHWI2C
extern I2C_HandleType gI2CBq76952HandleType;
#else
extern SimI2C_HandleType gI2CBq76952HandleType;
#endif

#if USE_SIM_IIC == 0
extern I2C_HandleType I2CEepromHandle;
#else
extern SimI2C_HandleType   SimI2CEepromHandle;
#endif

void ADCBspInit(void);  
void Uart1FpgaSofcoreBspInit(void);
void SpiFlashBspInit(void);
void Uart0BspInit(void);
void ExtWdgOut1BspInit(void);
void ExtWdgOut2BspInit(void);
void VBENBspInit(void);
void I2CBq76952BspInit(void);
void I2CEepromBspInit(void);
void MCULEDBspInit(void);
void KeyLEDBspInit(void);
void TempS2BspInit(void);
void TempS1BspInit(void);
void TempS0BspInit(void);
void KeyCheckBspInit(void);
void PowerHoldBspInit(void);
void HeatCKBspInit(void);
void HeatEnBspInit(void);
void IDAdcEnBspInit(void);
void ErrorCkBspInit(void);
void PmosEnBspInit(void);
void CmosEnBspInit(void);
void DmosEnBspInit(void);
void BspInit(void);
void BspBaseInit(void);
void AFETimerInit(void);
void AfeTsBspInit(void);
#endif












