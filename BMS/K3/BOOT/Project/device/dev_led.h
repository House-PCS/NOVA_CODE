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
#ifndef __DEV_LED_H_
#define __DEV_LED_H_


#include "bsp.h"
#include "systick.h"

typedef enum
{
	LAMP_ON,
	LAMP_OFF,
	LAMP_BLINK,
    LAMP_BREATH,
    LAMP_INVALID,
}EN_LAMP_STATUS;

void LedProcess(void);
void AtmoLampProcess(void);
#endif












