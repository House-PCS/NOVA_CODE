/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        high_volt_detect.h
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
  * <tr><td>2024-09-02    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __HIGH_VOLT_DETECT_H_
#define __HIGH_VOLT_DETECT_H_

#include "dev_volt_detect.h"

typedef struct _high_volt_info_stru_t
{
    uint32_t        v_bp;//B+电压,单位mV
    uint32_t        v_pp;//P+电压,单位mV
}HighVoltInfo_t;

extern HighVoltInfo_t gHighVoltInfo;

void HighVoltDetectProcess(void);



#endif
