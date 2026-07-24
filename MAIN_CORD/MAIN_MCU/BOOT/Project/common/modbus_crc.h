/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_crc.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-27
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-27    <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __MODBUS_CRC_H_
#define __MODBUS_CRC_H_

#include "drv_opt.h"

uint16_t CrcModbus(uint8_t *p, uint16_t len);
bool ModbusCrcJudge(uint8_t *p, uint16_t len);

#endif












