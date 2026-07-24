/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_it7c4337.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-22
  * @brief       GD32F450
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __DEV_IT7C4337_H_
#define __DEV_IT7C4337_H_

#include "bsp.h"
#include "queue.h"


#define IT7C4337_SEC_REG_ADDR        (0x00U)
#define IT7C4337_MIN_REG_ADDR        (0x01U)
#define IT7C4337_HOUR_REG_ADDR       (0x02U)
#define IT7C4337_WEEK_REG_ADDR       (0X03U)
#define IT7C4337_DAY_REG_ADDR        (0x04U)
#define IT7C4337_MONTH_REG_ADDR      (0x05U)
#define IT7C4337_YEAR_REG_ADDR       (0x06U)

#define IT7C4337_CONTROL_REG_ADDR    (0x0EU)
#define IT7C4337_STATUS_REG_ADDR     (0x0FU)


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


void RtcSecConvert2Date(DevRtc_t *pDate);
void RtcDat2Covert2Sec(DevRtc_t *pDate);

void DevRtcReadTime(DevRtc_t *pDate);
void DevRtcWriteTime(DevRtc_t *pDate);
void DevRtcInit(void);
void CalibrationRtcTime(uint32_t stamp);
uint32_t GetRtcTimeStamp(void);

#endif















