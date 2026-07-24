/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.h
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

#ifndef __PRIVATE_ANALYSIS_H_
#define __PRIVATE_ANALYSIS_H_
#include "mid_comservice.h"


#define SLAVE_WRITE_QUEUE_DEPTH         2

#define CMD_UPGRATE_START           0x80F0
#define CMD_UPGRATE_BUSY            0x80F1
#define CMD_UPGRATE_END             0x80FF

#define  ACK_OK                 0x00             /* ack success */
#define  ACK_ERROR              0x01             /* ack error */
#define  ACK_INVALID            0xFF             /* ack invalid */


typedef struct _write_response_ack_stru_t
{
    uint8_t                 cmd_upgrate_start;      // 0xFF 无效 0x00 成功      0x01 错误                        
    uint8_t                 cmd_upgrate_busy;       // 0xFF 无效 0x00 成功      0x01 错误      
    uint8_t                 cmd_upgrate_end;        // 0xFF 无效 0x00 成功      0x01 错误                    
} WiteResAckStatus_t;

extern WiteResAckStatus_t  garrWriteResAckState[EN_COMSERVICE_INDEX_NUM];


void WriteResponseAnalysis(SciRevData_t *p_com_cmd);
void PrivateDataSend(EN_COMSERVICE_INDEX   sci_index, uint8_t* data, uint16_t len);
void ResetWriteResAckState(EN_COMSERVICE_INDEX sci_index, uint16_t cmd);
void PrivateParamInit(void);
bool WriteRequestAnalysis(SciRevData_t *p_com_cmd);
bool PrivateUpgradeRunning(void);

#endif
