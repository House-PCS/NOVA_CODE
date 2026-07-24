
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        continus_operation.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-12-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>zhangqi    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef _CONTINUS_OPERATION_H_
#define _CONTINUS_OPERATION_H_
#include "F28x_Project.h"
#include "CountTrigger.h"

extern void ContinusOperationInit();
extern void ContinusOperationCheck(float32 vrms, float32 vrms_threshold, uint16_t check_en);
extern void ContinusOperationRecover(float32 vrms, float32 vrms_threshold, uint16_t check_en);
extern uint16_t ReadContinusOperationState(void);
extern uint32_t ReadContinusOperationCount(void);
extern uint32_t ReadRecoverCount(void);
#endif
