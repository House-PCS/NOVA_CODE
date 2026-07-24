/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_volt_detect.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-6-9      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __DEV_VOLT_DETECT_H_
#define __DEV_VOLT_DETECT_H_


#include "mcu_bsp.h"


#define CONT_SAMPLE_NUM     64  //连续采样次数


typedef enum
{
    EN_DEV_VOLT_DETECT_BP,//B+
    EN_DEV_VOLT_DETECT_PP,//P+
    EN_DEV_VOLT_DETECT_CUR1,//小量程电流
    EN_DEV_VOLT_DETECT_CUR2,//大量程电流
    EN_DEV_VOLT_DETECT_NUM
}EN_DEV_VOLT_DETECT_IDX;




void DevVoltDetectInit(void);
uint16_t DevGetVoltValue(EN_DEV_VOLT_DETECT_IDX idx);
uint16_t *DevGetMultiContVoltValue(EN_DEV_VOLT_DETECT_IDX idx);
void DevVoltDetectProcess(void);
#endif











