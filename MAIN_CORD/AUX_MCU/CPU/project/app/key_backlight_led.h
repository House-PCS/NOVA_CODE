/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        key_backlight_led.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-28
  * @brief       按键背光控制业务
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-02-28    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __KEY_BACKLIGHT_LED_H_
#define __KEY_BACKLIGHT_LED_H_

#include "dev_atmos_sk68xx.h"
//#include "dev_key.h"
#include "systick.h"
#include "slave_ledctl.h"
#include "dev_atmos_sy_adbic.h"


typedef enum
{
    EN_KEY_LED_STATUS_START_UP_FIRST,
    EN_KEY_LED_STATUS_START_UP_FIRST_WAIT,
    EN_KEY_LED_STATUS_START_UP_SECOND,
    EN_KEY_LED_STATUS_START_UP_SECOND_WAIT,
    EN_KEY_LED_STATUS_STAND_BY,
    EN_KEY_LED_STATUS_WAKE_UP,
}EN_KEY_LED_STATUS;


typedef enum
{
    EN_KEY_LED_1,
    EN_KEY_LED_2,
    EN_KEY_LED_3,
    EN_KEY_LED_4,
    EN_KEY_LED_NUM
}EN_KEY_LED_NUMBER;


void KeyLedProcess(void);

#endif


