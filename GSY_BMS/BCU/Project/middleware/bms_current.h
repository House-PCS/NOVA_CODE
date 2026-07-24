/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can.c
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
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __BMS_CURRENT_H_
#define __BMS_CURRENT_H_

#include <stdint.h>




typedef struct _bms_current_stru_t
{
//    uint16_t        valid_cnt;      //采集有效次数
//    uint16_t        error_cnt;      //采集错误次数
    int32_t         current;    //mA
    uint32_t        charge_integral;    //mAms
    uint32_t        discharge_integral;
}BmsCurrentType;

extern const BmsCurrentType *const IR_BmsCurrent;



void OtherCurrSet(int32_t curr);

void BmsCurrentMain(void);
void CurrIntegralClear(void);

#endif
