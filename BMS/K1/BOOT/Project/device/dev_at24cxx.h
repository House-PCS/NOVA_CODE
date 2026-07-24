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
#ifndef __DEV_AT24CXX_H_
#define __DEV_AT24CXX_H_

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

void DevEepromCtlInit(void);
bool EepromTest(void);
void EepromWrite(uint8_t *pBuf, uint16_t addr, uint16_t size);
void EepromRead(uint8_t *pBuf, uint16_t addr, uint16_t size);
#ifdef __cplusplus
}
#endif
#endif /* __SPI_FLASH */

