/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_sd2506.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-19
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-02    <td>V1.0.0    <td>           <td>InitialVersion 
  * </table>
  ******************************************************************************
  */
#ifndef __DEV_SD2506_H_
#define __DEV_SD2506_H_

#include "mcu_bsp.h"


typedef struct _dev_rtc_stru_t
{
    /*非实时信息，用时转换，每次调用获取时，需要先调用_RtcSecConvert2Date*/
    uint16_t        year;            
    uint8_t         month;               
    uint8_t         day;                
    uint8_t         week;         
    uint8_t         hour;                
    uint8_t         minute;              
    uint8_t         second;
    int8_t          time_zone;/*时区，暂时不用*/
    /*实时信息*/
    uint32_t        time_stamp;/*时间戳，换算成秒，基于1970年1月1日0点0分0秒*/
}DevRtc_t;


extern DevRtc_t gDevRtc;

void RtcSecConvert2Date(DevRtc_t *pDate);
void RtcDat2Covert2Sec(DevRtc_t *pDate);
uint32_t GetRtcTimeStamp(void);

void DevRtcWriteTime(DevRtc_t *pDate);
void DevRtcInit(void);
void DevRtcProcess(void);

#endif







