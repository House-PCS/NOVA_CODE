/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_24c32.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-19
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-06-19    <td>V1.0.0    <td>           <td>InitialVersion 
  * </table>
  ******************************************************************************
  */
#ifndef __DEV_24C32_H_
#define __DEV_24C32_H_



//#include "bsp.h"
#include "stdint.h"
#include "mcu_bsp.h"


void DevEepromCtlInit(void);
void EepromWrite(uint8_t *pBuf, uint16_t addr, uint16_t size);
void EepromRead(uint8_t *pBuf, uint16_t addr, uint16_t size);


#endif















