/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        temp_detect.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-09
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __TEMP_DETECT_H_
#define __TEMP_DETECT_H_

#include "dev_sgm58031.h"
#include "common.h"

typedef struct _temp_info_stru_t
{
    float        temp_relay_p;//总正继电器温度
    float        temp_relay_n;//总负继电器温度
    float        temp_relay_pre;//预充继电器温度
    float        temp_hvbox;
}TempInfo_t;



typedef enum
{
    EN_SGM_TEMP_GET_CMD,
    EN_SGM_TEMP_GET_STAT,
}EN_SGM_TEMP_GET_OP;


extern TempInfo_t  gTempInfo;
void TempDetectProcess(void);




#endif

