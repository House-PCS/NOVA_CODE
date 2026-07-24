/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT Nova
  * @file    dev_spi_flash.h
  * @author  Xi'an NovaStar Tech Co.,LTD.()
  * @version V1.0.0
  * @date    2021.08.04
  * @brief   CBB:this file provides the functions of dev_spi_flash.
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date        <th>Version  <th>Author      <th>Description
  * <tr><td>2021/08/04  <td>1.0.0    <td>Changshuai  <td>CBB Version
  * </table>
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEV_SQ52201_H_
#define __DEV_SQ52201_H_

/* Inculdes ------------------------------------------------------------------*/
#include "bsp.h"
#include "systick.h"
#include "stdint.h"
#include "fault_analysis.h"
#include "sim_i2c.h"
#include "i2c.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Exported macro -----------------------------------------------------------*/


void DevSQ52201Init(void);
float SQ52201_GetBusV(void);
float SQ52201_GetCurrent(void);
float SQ52201_GetPower(void);
    
uint16_t SQ52201_GetBoardManufID(void);
#ifdef __cplusplus
}
#endif
#endif /* __SPI_FLASH */

