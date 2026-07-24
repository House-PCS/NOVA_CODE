/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        rtc.h
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
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __RTC_H_
#define __RTC_H_

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "drv_opt.h"
#include "gd32f30x_libopt.h"
typedef struct _Rtc_handle_stru_t
{
    uint16_t        year;            
    uint8_t         month;               
    uint8_t         day;                
    uint8_t         week;         
    uint8_t         hour;                
    uint8_t         minute;              
    uint8_t         second;

    int8_t          time_zone;
    uint32_t        time_stamp;
}RtcDateInfo_t;

void RtcInit(void);
bool RtcNormalJudge(void);
void RtcSetTime(uint32_t sec);
uint32_t RtcGetTime(void);

void RtcSecCovert2Date(RtcDateInfo_t *pDate);
uint32_t RtcDat2Covert2Sec(RtcDateInfo_t *pDate);

void RtcIrqProcess(RtcDateInfo_t *pDataInfo);
#endif















