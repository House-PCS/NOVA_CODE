/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        drv_opt.j
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-22
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-22    <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __DRV_OPT_H_
#define __DRV_OPT_H_

#include "n32g4fr_adc.h"
#include "n32g4fr_usart.h"
#include "n32g4fr_gpio.h"
#include "n32g4fr_exti.h"
#include "n32g4fr_i2c.h"
#include "n32g4fr_spi.h"
#include "n32g4fr_tim.h"
#include "n32g4fr_usart.h"

/*UART*/
#define  USE_UART1      1
#define  USE_UART2      1
#define  USE_UART3      1
#define  USE_UART4      0
#define  USE_UART5      0
#define  USE_UART6      0
#define  USE_UART7      0

/*GPIO*/
#define  USE_GPIOA      1
#define  USE_GPIOB      1
#define  USE_GPIOC      1
#define  USE_GPIOD      1
#define  USE_GPIOE      0
#define  USE_GPIOF      0
#define  USE_GPIOG      0
#define  USE_GPIOH      0
#define  USE_GPIOI      0


/*EXTI*/
#define  USE_EXTI       1

/*SPI*/
#define  USE_SPI1       1
#define  USE_SPI2       0
#define  USE_SPI3       0
#define  USE_SPI4       0
#define  USE_SPI5       0


/*I2C*/
#define  USE_I2C0       1
#define  USE_I2C1       0
#define  USE_I2C2       0

/*ADC*/
#define  USE_ADC0       0
#define  USE_ADC1       1
#define  USE_ADC2       1
#define  USE_ADC3       0

/*DMA*/
#define  USE_DMA1       1
#define  USE_DMA2       1


/*TIMER*/
#define  USE_TIMER0     1
#define  USE_TIMER1     1
#define  USE_TIMER7     1
#endif

















