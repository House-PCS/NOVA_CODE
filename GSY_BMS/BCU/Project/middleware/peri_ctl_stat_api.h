/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        peri_ctl_stat_api.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-04 
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-04    <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __PERI_CTL_STAT_API_H_
#define __PERI_CTL_STAT_API_H_


#include "dev_ioctl.h"

void RelayPCtl(bool value);
bool RelayPFeedback(void);
void RelayPreCtl(bool value);
bool RelayPreFeedback(void);
void BreakerCtl(bool value);
bool BreakerFeedback(void);
void RelayFanCtl(bool value);
void BMUAddressingMainOutCtl(bool value);
bool BMUAddressingMainInStat(void);
void BMUAddressingBackupOutCtl(bool value);
bool BMUAddressingBackupInStat(void);
void InsImpUpMosCtl(bool value);
void InsImpDownMosCtl(bool value);
void InsImpRelayCtl(bool value);

extern bool test_tool_flag;

#endif












