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
#ifndef __DEV_SYSTEM_POWER_H_
#define __DEV_SYSTEM_POWER_H_


#include "bsp.h"
#include "systick.h"

void SystemPowerBspInit(void);
void PowerHoldOn(void);
void PowerHoldOff(void);
void CCEnOn(void);
void CCEnOff(void);


/*Ð¹·ÅµçÂ·*/
void ReleaseOn(void);
void ReleaseOff(void);
        
void OCStatusClear(void);

/*¼ì²â*/
bool OVCheck(void);
bool ChargerCheck(void);
bool DsgOCCheck(void);
bool HeatshortCheck(void);

#endif












