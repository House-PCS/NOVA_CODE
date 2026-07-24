/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        ntc_convert.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-8-20    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __NTC_CONVERT_H_
#define __NTC_CONVERT_H_


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef enum
{
    EN_NTC_B_3450 = 3450,
    EN_NTC_B_3435 = 3435
}EN_NTC_B;


typedef enum
{
    EN_NTC_T25_5K = 5000,
    EN_NTC_T25_10K = 10000,
    EN_NTC_T25_50K = 50000,
    EN_NTC_T25_100K = 100000,
}EN_NTC_T25;


float TemperatureGet(uint16_t adc_volt, uint16_t volt_max, uint32_t r_up, EN_NTC_B ntc_b, EN_NTC_T25 ntc_t25);


#endif
