/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        drv_opt.h
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
#ifndef __DRV_OPT_H_
#define __DRV_OPT_H_

#include "gd32f4xx_gpio.h"
#include "gd32f4xx_usart.h"
#include "gd32f4xx_dma.h"
#include "gd32f4xx_i2c.h"
#include "gd32f4xx_spi.h"
#include "gd32f4xx_timer.h"
#include "gd32f4xx_exmc.h"
#include "gd32f4xx_fmc.h"

#include "stdbool.h"
#include "stdio.h"

/*UART*/
#define  USE_UART0      1
#define  USE_UART1      1
#define  USE_UART2      1
#define  USE_UART3      1
#define  USE_UART4      1
#define  USE_UART5      1
#define  USE_UART6      1
#define  USE_UART7      0

/*GPIO*/
#define  USE_GPIOA      1
#define  USE_GPIOB      1
#define  USE_GPIOC      1
#define  USE_GPIOD      1
#define  USE_GPIOE      1
#define  USE_GPIOF      1
#define  USE_GPIOG      1
#define  USE_GPIOH      0
#define  USE_GPIOI      0


/*EXTI*/
#define  USE_EXTI       1

/*SPI*/
#define  USE_SPI0       1
#define  USE_SPI1       1
#define  USE_SPI2       1
#define  USE_SPI3       1
#define  USE_SPI4       0
#define  USE_SPI5       0


/*I2C*/
#define  USE_I2C0       1
#define  USE_I2C1       0
#define  USE_I2C2       0

/*ADC*/
#define  USE_ADC0       1
#define  USE_ADC1       0

/*DMA*/
#define  USE_DMA0       1
#define  USE_DMA1       1


/*TIMER*/

#define  USE_TIMER0     1 
#define  USE_TIMER1     1 
#define  USE_TIMER2     1 
#define  USE_TIMER3     1 
#define  USE_TIMER4     1 
#define  USE_TIMER5     1 
#define  USE_TIMER6     1 
#define  USE_TIMER7     1 
/*RTC*/
#define RTC_CLOCK_SOURCE_IRC32K     1





#define __SDRAM __attribute__ ((section("SDRAM")))









#endif

















