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
#include "F28x_Project.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EN_CLA_RUN_TIME_MEASURE (0)

/**
 * @brief 定时器状态结构体
 */
typedef struct timer_state_stru_t
{
    uint32_t Time;                  //计数
    uint16_t Timer_1ms_Count;       //定时器1ms
    uint16_t Timer_5ms_Count;       //定时器5ms
    uint16_t Timer_10ms_Count;      //定时器10ms
    uint16_t Timer_20ms_Count;      //定时器20ms
    uint16_t Timer_100ms_Count;     //定时器100ms
    uint16_t Timer_500ms_Count;
    uint16_t Timer_1000ms_Count;    //定时器1000ms

    uint16_t Timer_1ms_Flag;        //定时器1ms标志位 置1有效
    uint16_t Timer_5ms_Flag;        //定时器5ms标志位 置1有效
    uint16_t Timer_10ms_Flag;       //定时器10ms标志位 置1有效
    uint16_t Timer_20ms_Flag;       //定时器20ms标志位 置1有效
    uint16_t Timer_100ms_Flag;      //定时器100ms标志位 置1有效
    uint16_t Timer_500ms_Flag;      //定时器100ms标志位 置1有效
    uint16_t Timer_1000ms_Flag;     //定时器1000ms标志位 置1有效
} TimerState_t;

void CpuTimer_Init(void);
uint32_t TimGetSystime(void);
uint32_t TimDifftimeFromNow(uint32_t time);
void CpuTimerFlag(void);
uint32_t TimDifftime(uint32_t time, uint32_t new_time);
uint16_t Get_1ms_Flag(void);
uint16_t Get_5ms_Flag(void);
uint16_t Get_10ms_Flag(void);
uint16_t Get_20ms_Flag(void);
uint16_t Get_100ms_Flag(void);
uint16_t Get_500ms_Flag(void);
uint16_t Get_1s_Flag(void);

void Timers_1ms(void);
void Timers_5ms(void);
void Timers_10ms(void);
void Timers_20ms(void);
void Timers_100ms(void);
void Timers_500ms(void);
void Timers_1s(void);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
