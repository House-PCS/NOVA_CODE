/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cpu_timers.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __CPU_TIMERS_H__
#define __CPU_TIMERS_H__

#ifdef __cplusplus
extern "C" {
#endif

void CpuTimersInit(void);
uint32_t TimGetSystime(void);
uint32_t TimDifftimeFromNow(uint32_t time);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
