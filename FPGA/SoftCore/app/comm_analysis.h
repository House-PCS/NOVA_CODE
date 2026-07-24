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

#ifndef __COMM_ANALYSIS_H_
#define __COMM_ANALYSIS_H_

#include "systick.h"
#include "private_analysis.h"
#include "mid_comservice.h"

#define HOST_COMM_REV_MAX_TIME          1000
#define HOST_COMM_POLLING_TIME          300
#define FPGA_COMM_POLLING_TIME          300
typedef struct _slave_error_para_stru_t
{   
    uint8_t                         bms_com_time_out;
    uint8_t                         dcdc_com_time_out;       
} SlaveErrorPara_t;
typedef enum
{
    EN_HOST_POLLING_STATUS_IDLE = 0,
    EN_HOST_POLLING_STATUS_WAITING_BMS_REV,
    EN_HOST_POLLING_STATUS_WAITING_DCDC_REV,    
    EN_HOST_POLLING_STATUS_REV_BMS_OVERTIME,
    EN_HOST_POLLING_STATUS_REV_DCDC_OVERTIME,    
    EN_HOSTE_POLLING_STATUS_REV_OK,
    EN_HOST_POLLING_STATUS_REV_ERROR,
} EN_HOST_POLLING_STATUS;



void HostCommProcess(void);
void SlaveCommProcess(void);
void CommBMSSend(uint8_t *p, uint16_t size);


















#endif
