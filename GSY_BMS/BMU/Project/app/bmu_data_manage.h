/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bmu_data_manage.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-04 
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-04    <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __BMU_DATA_MANAGE_H_
#define __BMU_DATA_MANAGE_H_


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "afe_ctl.h"
#include "dev_pwm_detect_cfg.h"

#pragma pack(1)
typedef struct _bmu_addr_mask_info_stru_t
{
    uint8_t             can_sa;//CAN通信从机地址
    uint8_t             cell_volt_mask[(CELL_VOLT_SAMPLE_MAX + 7) / 8];
    uint8_t             cell_temp_mask[(PACK_TEMP_SAMPLE_MAX + 7) / 8];
    uint8_t             pole_temp_mask[(PACK_TEMP_SAMPLE_MAX + 7) / 8];
    uint8_t             other_temp_mask[(PACK_TEMP_SAMPLE_MAX + 7) / 8];
    uint8_t             balance_temp_mask[(BALANCE_TEMP_SAMPLE_MAX + 7) / 8];
}BMUAddrMaskInfo_t;
#pragma pack()

typedef struct _bmu_statis_volt_info_stru_t
{
    bool                valid_flag;//数据有效性，经过至少一次采样后才有效
    uint32_t            total_volt;//总压,mV
    uint8_t             cell_max_volt_num;//最高单体电压序号,从0开始
    uint16_t            cell_max_volt;//最高单体电压,mV
    uint8_t             cell_min_volt_num;//最低单体电压序号，从0开始
    uint16_t            cell_min_volt;//最低单体电压,mV
}BMUStatisVoltInfo_t;

typedef struct _bmu_statis_temp_info_stru_t
{
    bool                valid_flag;//数据有效性，经过至少一次采样后才有效
    uint8_t             cell_max_temp_num;//最高单体温度序号,从0开始
    int8_t              cell_max_temp;//最高单体温度值，℃
    uint8_t             cell_min_temp_num;//最低单体温度序号，从0开始
    int8_t              cell_min_temp;//最低单体温度值，℃
    uint8_t             pole_max_temp_num;//最高极柱温度序号,从0开始
    int8_t              pole_max_temp;//最高极柱温度值，℃
    uint8_t             pole_min_temp_num;//最低极柱温度序号，从0开始
    int8_t              pole_min_temp;//最低极柱温度值，℃
}BMUStatisTempInfo_t;

typedef struct _bmu_statis_other_info_stru_t
{
    bool                valid_flag;//数据有效性，经过至少一次采样后才有效
    uint16_t            cell_ave_volt;//单体平均电压,mV
    int8_t              cell_ave_temp;//单体平均温度
    int8_t              equalizing_plate_max_temp;//均衡板最高温度
    uint16_t            fault_info;//故障信息
}BMUStatisOtherInfo_t;


typedef struct _bmu_detail_volt_info_stru_t
{
    bool                valid_flag;//数据有效性，经过至少一次采样后才有效
    uint16_t            cell_volt[CELL_VOLT_SAMPLE_MAX];//单体电压,mV
    bool                cell_balance_stat[CELL_VOLT_SAMPLE_MAX];//单体被动均衡状态
}BMUDetailVoltInfo_t;

typedef struct _bmu_detail_temp_info_stru_t
{
    bool                valid_flag;//数据有效性，经过至少一次采样后才有效
    int8_t              cell_temp[PACK_TEMP_SAMPLE_MAX];//单体温度
}BMUDetailTempInfo_t;

typedef struct _bmu_detail_pole_temp_info_stru_t
{
    bool                valid_flag;//数据有效性，经过至少一次采样后才有效
    int8_t              pole_temp[PACK_TEMP_SAMPLE_MAX];//单体温度
}BMUDetailPoleTempInfo_t;

typedef struct _bmu_detail_other_temp_info_stru_t
{
    bool                valid_flag;//数据有效性，经过至少一次采样后才有效
    int8_t              other_temp[PACK_TEMP_SAMPLE_MAX];//单体温度
}BMUDetailOtherTempInfo_t;

typedef struct _bmu_detail_balance_temp_info_stru_t
{
    bool                valid_flag;//数据有效性，经过至少一次采样后才有效
    int8_t              balance_temp[BALANCE_TEMP_SAMPLE_MAX];//单体温度
}BMUDetailBalanceTempInfo_t;


#pragma pack(1)
typedef struct _bmu_data_info_stru_t
{
    BMUAddrMaskInfo_t               bmu_addr_mask_info;
    
    //风扇
    uint8_t                         fan_duty;
    uint8_t                         fan_rd_duty;//反馈的占空比
    uint32_t                        fan_rd_freq;
            
    uint8_t                         per_bmu_cell_num;//单个从控电芯实际数量
    uint8_t                         per_bmu_cell_temp_num;//单个从控电芯温度实际数量
    uint8_t                         per_bmu_pole_temp_num;//单个从控极柱温度实际数量
    uint8_t                         per_bmu_other_temp_num;//单个从控正负极温度实际数量
    uint8_t                         per_bmu_balance_temp_num;//单个从控均衡温度实际数量
            
    BMUStatisVoltInfo_t             bmu_statis_volt_info;
    BMUStatisTempInfo_t             bmu_statis_temp_info;
    BMUStatisOtherInfo_t            bmu_statis_other_info;
    BMUDetailVoltInfo_t             bmu_detail_volt_info;
    BMUDetailTempInfo_t             bmu_detail_temp_info;
    BMUDetailPoleTempInfo_t         bmu_detail_pole_temp_info;
    BMUDetailOtherTempInfo_t        bmu_detail_other_temp_info;
    BMUDetailBalanceTempInfo_t      bmu_detail_balance_temp_info;
	
	char                    		fw_version[16];  //版本号
}BMUDataInfo_t;
#pragma pack()

extern BMUDataInfo_t gBMUDataInfo;
void BMSCalcDataProcess(void);


#endif



