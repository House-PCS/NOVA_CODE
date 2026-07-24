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
#ifndef __DEV_MOS_H_
#define __DEV_MOS_H_


#include "bsp.h"
#include "systick.h"

void OpenChargeMos(void);
void CloseChargeMos(void);
bool ChargeMosIsOpen(void);
void OpenDisChargeMos(void);
void CloseDisChargeMos(void);
bool DisChargeMosIsOpen(void);
void OpenPreChargeMos(void);
void ClosePreChargeMos(void);
bool PreChargeMosIsOpen(void);
void Open1ExpMos(void);
void Close1ExpMos(void);
bool ExternPack1MosIsOpen(void);
void Open2ExpMos(void);
void Close2ExpMos(void);
void CloseAllMOS(void);

void MosInit(void);
#endif












