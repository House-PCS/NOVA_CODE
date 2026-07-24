
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        selftest.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-11-4
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

#ifndef __SELFTEST_H__
#define __SELFTEST_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#define ZERODRIFTING 0
#define ZERODRIFTOK  1
#define ZERODRIFTERR 2
#define SELFTESTING 0
#define SELFTESTOK  1
#define SELFTESTERR 2
extern uint16_t gZeroDriftState;
extern void ZeroDriftCorrect();
extern uint16_t ReadSelfTestState();
extern void SelfTest();

#ifdef __cplusplus
}
#endif                                  /* extern "C" */
#endif
