/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        gpio.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-20
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * <tr><td>2023-06-15    <td>V1.0.1    <td>           <td>增加同一串口可使用多种中断并存的机制
  * </table>
  ******************************************************************************
  */
#ifndef __MDIO_H_
#define __MDIO_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include <stdbool.h>
//#include <math.h>
//#include "CMSDK_CM0.h"
//#include "core_cm0.h"
#include "system_CMSDK_CM0.h"

void SmiWriteBits(uint8_t *tx_data, uint16_t tx_bit_size);
void SmiReadBits(uint8_t *rx_data, uint16_t rx_bit_size);
bool SmiSendTa(void);
void SmiIdleState(void);
void MdioSet2Output(void);
void MdioSet2Input(void);
void SmiRstLow(void);
void SmiRstHigh(void);
#endif








