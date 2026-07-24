/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        comm_analysis.h
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

#ifndef __COMM_ANALYSIS_H_
#define __COMM_ANALYSIS_H_

#include "F28x_Project.h"
#include "mid_comservice.h"
#include "sci.h"
#include "dev_flash.h"

#define REQUST_PACKET_HEAD      0x5A
#define RESPONSE_PACKET_HEAD    0xA5
#define SEND_ADDR               0x00 /* Ö÷¿Ø */
#define RECV_ADDR               0x03 /* BMS */

#define  ACK_OK                 0x00             /* ack success */
#define  ACK_ERROR              0x01             /* ack error */
#define  ACK_INVALID            0xFF             /* ack invalid */

#define COM_RX_SIZE     2100
#define COM_TX_SIZE     1024

#define UPGRADE_FLAG_ADDR 0x00000000
#define PCS_FIRMWARE_ADDR 0x00001000


#define ON_OFF_CMD              0x000E
#define UPGRADE_START           0x80F0
#define UPGRADE_BUSY            0x80F1
#define UPGRADE_END             0x80FF

extern uint8_t gReadResponsed;
extern uint16_t gPCSFirmwareType[4];

extern uint16_t gDeviceUpFlag;

bool ProtoUnpackJudge(SciRevData_t *p);
void DataSendPolling(uint8_t read_responsed);

void SysRestart(void);
void UpgradeProcesshandle(void);
#endif








