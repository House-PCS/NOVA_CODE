/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        atmos_led.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-03-07
  * @brief       ²àµÆ°å¿ØÖÆÒµÎñ
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-03-07    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __ATMOS_SIDE_LED_H_
#define __ATMOS_SIDE_LED_H_

#include "bsp.h"


#define  SIDE_LED_NUM      (21)

typedef enum 
{
    EN_ATMOS_SIDE_OFF = 0,      /*Ï¨Ãð*/
    EN_ATMOS_SIDE_ON,           /*³£ÁÁ*/
    EN_ATMOS_SIDE_BLINK,        /*ÉÁË¸*/
    EN_ATMOS_SIDE_BREATH,       /*ºôÎü*/
    EN_ATMOS_SIDE_SOC = 0xFF,   /*SOCÏÔÊ¾*/
} EN_ATMOS_SEID_STATUS;



void DevSideLedProcess(void);
void DevSideAtmosInit(void);
#endif





