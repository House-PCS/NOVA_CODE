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
//#include "EventRecorder.h"
#include "queue.h"
#include "dev_can_comm.h"

//AFE
extern Spi_HandleType gSpiAdbms6830Handle;
extern Gpio_HandleType gCsAdbms6830Handle;
void SpiAdbms6830BspInit(void);

//SPIFLASH
extern Gpio_HandleType SpiFlashCsHandle;
extern Spi_HandleType SpiFlashHandle;
void SpiFlashBspInit(void);

//CAN	
extern Can_HandleType gCanHandle[];
void CanBspInit(void);

//PWM
extern Timer_HandleType gTimer2PwmFanHandle;
void PWMFanOutputBspInit(void);
extern Timer_HandleType gTimer0PwmFanRdHandle;
void PWMFanRdInputBspInit(void);


//JACK IO
extern Gpio_HandleType gJackIOHandle[];
void JackIOBspInit(void);

extern Gpio_HandleType gMcuLedHandle;
//void McuLedBspInit(void);

//TIMER5 ÂÖÑ¯
extern Timer_HandleType gTimerPollingHandle;
void TimerPollingBspInit(void);

//LEDµÆ
extern Gpio_HandleType gGpioLedHandle;
void GpioLedBspInit(void);


void BaseBspInit(void);

#endif
