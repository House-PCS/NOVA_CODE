/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_can_test.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-6-20      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __CAN_MAIN_CARD_H_
#define __CAN_MAIN_CARD_H_  
#include "dev_can_comm.h"

typedef struct
{
    uint8_t bms_shut_down_signal;  //bms开关机信号（主机发送）
} CanRevData_t;

extern CanRevData_t CanRevData;

void SendShutDownSignal(uint8_t shutdown);

#endif