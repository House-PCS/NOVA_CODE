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

#include "dev_led.h"
#define  LED_TIMES      100
void LedProcess(void)
{
    static uint32_t sn_time = 0;
    if(tim_difftime_from_now(sn_time) > LED_TIMES)
    {
        sn_time = tim_get_systime();
        GpioBitToogle((&LED_Handle));
    }
}

