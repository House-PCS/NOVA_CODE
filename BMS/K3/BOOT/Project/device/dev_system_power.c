/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
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

#include "dev_system_power.h"
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


