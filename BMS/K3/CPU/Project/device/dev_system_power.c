/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_system_power.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "dev_system_power.h"
#include "dev_mos_control.h"
#define LOG_TAG "dev_system_power"

void BMSBoardPowerOn(void)
{
    GpioBitSet((&PowerHoldHandle));
}

void BMSBoardPowerOff(void)
{
    GpioBitReset((&PowerHoldHandle));
}

bool BMSBoardPowerGet(void)
{
    return (bool)GpioOutputBitGet((&PowerHoldHandle));
}


