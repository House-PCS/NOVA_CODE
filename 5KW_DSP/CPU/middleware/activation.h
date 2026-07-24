
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        activation.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-9-24
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-3-19    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __ACTIVATION_H__
#define __ACTIVATION_H__

#ifdef __cplusplus
extern "C" {
#endif
#define PVACTIVAT_BUS_THRESHOLD 450 //PV激活Boost模式母线预充结束阈值
#include "stdint.h"
typedef enum
{
    EN_ACTIVATION_STATE_DEF_SELFTEST,
    EN_ACTIVATION_STATE_DEF_READY,
    EN_ACTIVATION_STATE_DEF_ACTIVATIONBOOST,
    EN_ACTIVATION_STATE_DEF_ACTIVATIONLLCRUN,
    EN_ACTIVATION_STATE_DEF_ACTIVATIONREADY,
    EN_ACTIVATION_STATE_DEF_NORMAL,
    EN_ACTIVATION_STATE_DEF_ERR,
} EN_ACTIVATION_STATE_DEF;

#define PVACTIVATION_PVSTART 1
#define PVACTIVATION_PVSTARTOK 2
#define PVACTIVATION_BATSTART 3
#define PVACTIVATION_PVSTARTERR 4

#define ACTIVATION_STATE_READY 1
#define ACTIVATION_STATE_ACTIVATING 2
#define ACTIVATION_STATE_ACTIVATIONREADY 3
#define ACTIVATION_STATE_ACTIVATIONOK 4
#define ACTIVATION_STATE_ERR 5
void ActivationFsmRun();
EN_ACTIVATION_STATE_DEF ActivationFsmStateRead();
uint16_t ActivationRunStateRead();
void ActivationRunStateSet(uint16_t activationrunstate);
#ifdef __cplusplus
}
#endif                                  /* extern "C" */
#endif
