/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        event_log.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-29
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-29    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __EVENT_LOG_H_
#define __EVENT_LOG_H_


#include "alarm.h"

typedef enum
{
    EN_EVENT_TYPE_START_UP,//开机事件
    EN_EVENT_TYPE_ALARM_OCCUR, //故障事件
    EN_EVENT_TYPE_ALARM_DISPEL,//故障消除
    EN_EVENT_TYPE_UPG,//升级事件
}EN_EVENT_TYPE;




typedef struct _event_log_info_stru_t
{
    uint32_t            event_valid_flag;//故障数据标记，用于读FLASH判断是否为故障数据，恒定为0x5A5A5A5A
    uint32_t            event_time;//事件日志发生时间，时间戳
    EN_EVENT_TYPE       event_type;//事件类型
    union
    {
        uint32_t            event_content_u32[5];//裸数据形式，由不同事件类型区分
        char                event_content_char[20];//字符串形式，由不同事件类型区分
    }event_content;//事件内容，总共预留32个字节
}EventLogInfo_t;


//每个事件占用32个字节，一天最多可存储128条日志
//存储在SPI FLASH中，一天一个SECTOR,两年循环，从2025年1月1日开始


#endif




