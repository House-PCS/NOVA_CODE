
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        hard_cmpss.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-7-29
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-7-29    <td>V1.0.0    <td>zhangqi    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef HARD_CMPSS_H
#define HARD_CMPSS_H
#include "F28x_Project.h"
#include "algo_err_check.h"

extern void Cmpss_Init();
extern uint16_t Updata_cmpss_dac(float adcref_reg);
extern uint16_t Read_cmpss_status();
extern uint16_t Calibrate_cmpss_dac(float adcref_reg);
extern void Clear_cmpss_ost();

#endif
