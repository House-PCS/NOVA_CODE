/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        data_storage_manage.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-18
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-18     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __DATA_STORAGE_MANAGE_H_
#define __DATA_STORAGE_MANAGE_H_


#include "dev_spiflash.h"
#include "dev_24c32.h"
#include "bcu_sox.h"
#include "current_detect.h"
#include "alarm.h"
#include "bmu_ctl.h"
#include "systick.h"
#include "flash.h"


extern uint8_t buf_Version[3];
  
void SoxInfoStorageClear(void);
void ElectStatistStorageClear(void);
void BaseInfoStorage(uint8_t *p_buf, uint8_t size);
void BMUSampleInfoStorge(BMUSampleInfo_t *p_sample_info);
void PackFactInfoStorge(PackFactPara_t *p_pack_fact_info);
void AlarmThresholdInfoStorge(AlarmThreshold_t *p_alarm_threshold_info);
void BCUFuncThresholdInfoStorge(BCUFuncThreshold_t *p_func_threshold_info);
void BCUCurrCorrCoefStorge(void);
void BMUSampleInfoClear(void);
void PackFactInfoClear(void);
void AlarmThresholdInfoClear(void);
void BCUFuncThresholdInfoClear(void);
void BCUCurrCorrCoefClear(void);

void DataStorageInfoInit(void);
void ScheStorageDataProcess(void);

#endif

