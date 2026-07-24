/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        crc.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __CRC_H_
#define __CRC_H_

#include "F28x_Project.h"

uint16_t CrcModbus(uint8_t *p, uint16_t len);
bool CrcJudge(uint8_t *p, uint16_t len);
uint16_t BEBufToUint16(uint8_t *_pBuf);
void TabLsbToMsb(uint8_t *str, uint16_t len);
uint8_t MemcpyU16toU8SwapByte(uint8_t* buf, uint16_t* data, uint16_t len);
uint8_t MemcpyU16toU8(uint8_t* buf, uint16_t* data, uint16_t len);

#endif
