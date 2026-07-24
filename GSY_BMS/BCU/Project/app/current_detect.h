/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        current_detect.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-05
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-05    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __CURRENT_DETECT_H_
#define __CURRENT_DETECT_H_

#include "dev_volt_detect.h"


typedef struct _current_info_stru_t
{
    int32_t        rt_small_range_current;//实时小量程电流(做了多次采样中值滤波)，单位mA
    int32_t        rt_big_range_current;//实时大量程电流(做了多次采样中值滤波)，单位mA
    int32_t        filter_small_range_current;//再次滤波后小量程电流，单位mA
    int32_t        filter_big_range_current;//再次滤波后大量程电流，单位mA
    bool           small_corr_coef_new_flag;//当校正系数下发给BCU后，该标志置1，再接收到锁存指令时，才存储
    bool           big_corr_coef_new_flag;//当校正系数下发给BCU后，该标志置1，再接收到锁存指令时，才存储
    float          small_corr_coef_k;
    float          small_corr_coef_b;//单位mA
    float          big_corr_coef_k;
    float          big_corr_coef_b;//单位mA
}CurrentInfo_t;

extern CurrentInfo_t gCurrentInfo;
void CurrentDetectProcess(void);


#endif



