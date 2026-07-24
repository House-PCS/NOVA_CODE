/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.h
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


#ifndef __BMS_SAMPLE_HV_BSP_H_
#define __BMS_SAMPLE_HV_BSP_H_

#include <stdint.h>

#define GPV_MAX     15750       //0.1V
#define GNV_MAX     15000
#define RTP_MAX     15000
typedef enum 
{
    GPV = 0,
    RTP,
    GNV,
    
    HV_NUM,
}EN_HV_TYPE;
typedef struct _bms_sample_hv_stru_t
{
    uint16_t        valid_cnt;      //采集有效次数
    uint16_t        error_cnt;      //采集错误次数
    uint16_t        hv_voltage[HV_NUM];
        
}BmsSampleHVType;


extern const BmsSampleHVType *const IR_BmsSampleHV;


void SampleHighVoltMain(void);

#endif
