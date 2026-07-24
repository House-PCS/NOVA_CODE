/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        protocol_process.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-3-14
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-3-14    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __PROTOCOL_PROCESS_H_
#define __PROTOCOL_PROCESS_H_

#include "F28x_Project.h"
#include "private_analysis.h"
#include "protocol_process.h"
#include "ethernet_analysis.h"
#include "firmware_upgrate.h"
#include "modbus_analysis.h"


typedef enum
{
    EN_PROTOCOL_IDLE,
    EN_PROTOCOL_MODBUS_HOST_ANALYSIS,
    EN_PROTOCOL_MODBUS_SLAVE_ANALYSIS,
    EN_PROTOCOL_COM_RRESPONSE_ANALYSIS,
    EN_PROTOCOL_COM_WRESPONSE_ANALYSIS,
    EN_PROTOCOL_COM_RREQUEST_ANALYSIS,
    EN_PROTOCOL_COM_WREQUEST_ANALYSIS,
} EN_PROTOCOL_STATUS;


void ProtocolProcess(void);
void ProtocolInit(void);



















#endif
