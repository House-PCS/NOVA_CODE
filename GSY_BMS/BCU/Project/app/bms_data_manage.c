/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bms_data_manage.c
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
#include "bms_data_manage.h"


BMSDataInfo_t gBMSDataInfo;




/** 
* @brief    BMS需要计算的数据处理大轮询
* @param    none
* @return   none
* @note:
*/
void BMSCalcDataProcess()
{
    if(gBMSDataInfo.bmu_adressing_res != EN_OP_RES_COMPLETE)
    {
        return;
    }
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        if(gBMSDataInfo.bmu_statis_volt_info[i].valid_flag != true || 
           gBMSDataInfo.bmu_statis_temp_info[i].valid_flag != true || 
           gBMSDataInfo.bmu_statis_other_info[i].valid_flag != true)//需要等到所有从控相关数据都上报上来后才可以
        {
            return;
        }
    }
    
    uint16_t cell_max_volt_num = gBMSDataInfo.bmu_statis_volt_info[0].cell_max_volt_num;
    uint16_t cell_max_volt = gBMSDataInfo.bmu_statis_volt_info[0].cell_max_volt;
    uint16_t cell_min_volt_num = gBMSDataInfo.bmu_statis_volt_info[0].cell_min_volt_num;
    uint16_t cell_min_volt = gBMSDataInfo.bmu_statis_volt_info[0].cell_min_volt;
    uint16_t cell_max_temp_num = gBMSDataInfo.bmu_statis_temp_info[0].cell_max_temp_num;
    int8_t   cell_max_temp = gBMSDataInfo.bmu_statis_temp_info[0].cell_max_temp;
    uint16_t cell_min_temp_num = gBMSDataInfo.bmu_statis_temp_info[0].cell_min_temp_num;
    int8_t   cell_min_temp = gBMSDataInfo.bmu_statis_temp_info[0].cell_min_temp;
    uint32_t cell_ave_volt = 0;
    int32_t  cell_ave_temp = 0;
    uint32_t cell_total_volt = 0;
    uint16_t pole_max_temp_num = gBMSDataInfo.bmu_statis_temp_info[0].pole_max_temp_num;
    uint16_t pole_min_temp_num = gBMSDataInfo.bmu_statis_temp_info[0].pole_min_temp_num;
    int8_t   pole_max_temp = gBMSDataInfo.bmu_statis_temp_info[0].pole_max_temp;
    int8_t   pole_min_temp = gBMSDataInfo.bmu_statis_temp_info[0].pole_min_temp;
    
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        //最高单体电压
        if(gBMSDataInfo.bmu_statis_volt_info[i].cell_max_volt > cell_max_volt)
        {
            cell_max_volt = gBMSDataInfo.bmu_statis_volt_info[i].cell_max_volt;
            cell_max_volt_num = i * gBMSDataInfo.per_bmu_cell_num + gBMSDataInfo.bmu_statis_volt_info[i].cell_max_volt_num;
        }
        //最低单体电压
        if(gBMSDataInfo.bmu_statis_volt_info[i].cell_min_volt < cell_min_volt)
        {
            cell_min_volt = gBMSDataInfo.bmu_statis_volt_info[i].cell_min_volt;
            cell_min_volt_num = i * gBMSDataInfo.per_bmu_cell_num + gBMSDataInfo.bmu_statis_volt_info[i].cell_min_volt_num;
        }
        //最高单体温度
        if(gBMSDataInfo.bmu_statis_temp_info[i].cell_max_temp > cell_max_temp)
        {
            cell_max_temp = gBMSDataInfo.bmu_statis_temp_info[i].cell_max_temp;
            cell_max_temp_num = i * gBMSDataInfo.per_bmu_cell_temp_num + gBMSDataInfo.bmu_statis_temp_info[i].cell_max_temp_num;
        }
        //最低单体温度
        if(gBMSDataInfo.bmu_statis_temp_info[i].cell_min_temp < cell_min_temp)
        {
            cell_min_temp = gBMSDataInfo.bmu_statis_temp_info[i].cell_min_temp;
            cell_min_temp_num = i * gBMSDataInfo.per_bmu_cell_temp_num + gBMSDataInfo.bmu_statis_temp_info[i].cell_min_temp_num;
        }
        //电芯平均电压，先求和
        cell_ave_volt += gBMSDataInfo.bmu_statis_other_info[i].cell_ave_volt;
        //电芯平均温度，先求和
        cell_ave_temp += gBMSDataInfo.bmu_statis_other_info[i].cell_ave_temp;
        //电芯总压
        cell_total_volt += gBMSDataInfo.bmu_statis_volt_info[i].total_volt;
        //最高极柱温度
        if(gBMSDataInfo.bmu_statis_temp_info[i].pole_max_temp > pole_max_temp)
        {
            pole_max_temp = gBMSDataInfo.bmu_statis_temp_info[i].pole_max_temp;
            pole_max_temp_num = i * gBMSDataInfo.per_bmu_pole_temp_num + gBMSDataInfo.bmu_statis_temp_info[i].pole_max_temp_num;
        }
        //最低极柱温度
        if(gBMSDataInfo.bmu_statis_temp_info[i].pole_min_temp < pole_min_temp)
        {
            pole_min_temp = gBMSDataInfo.bmu_statis_temp_info[i].pole_min_temp;
            pole_min_temp_num = i * gBMSDataInfo.per_bmu_pole_temp_num + gBMSDataInfo.bmu_statis_temp_info[i].pole_min_temp_num;
        }
    }
    //电芯平均电压，再取平均
    cell_ave_volt /= gBMSDataInfo.bmu_num;
    //电芯平均温度，再取平均
    cell_ave_temp /= gBMSDataInfo.bmu_num;
    
    gBMSDataInfo.bmu_calc_info.cell_max_volt_num = cell_max_volt_num;
    gBMSDataInfo.bmu_calc_info.cell_max_volt = cell_max_volt;
    gBMSDataInfo.bmu_calc_info.cell_min_volt_num = cell_min_volt_num;
    gBMSDataInfo.bmu_calc_info.cell_min_volt = cell_min_volt;
    gBMSDataInfo.bmu_calc_info.cell_max_temp_num = cell_max_temp_num;
    gBMSDataInfo.bmu_calc_info.cell_max_temp = cell_max_temp;
    gBMSDataInfo.bmu_calc_info.cell_min_temp_num = cell_min_temp_num;
    gBMSDataInfo.bmu_calc_info.cell_min_temp = cell_min_temp;
    gBMSDataInfo.bmu_calc_info.cell_ave_volt = cell_ave_volt;
    gBMSDataInfo.bmu_calc_info.cell_ave_temp = cell_ave_temp;
    gBMSDataInfo.bmu_calc_info.cell_total_volt = cell_total_volt;
    gBMSDataInfo.bmu_calc_info.pole_max_temp_num = pole_max_temp_num;
    gBMSDataInfo.bmu_calc_info.pole_max_temp = pole_max_temp;
    gBMSDataInfo.bmu_calc_info.pole_min_temp_num = pole_min_temp_num;
    gBMSDataInfo.bmu_calc_info.pole_min_temp = pole_min_temp;
    
    gBMSDataInfo.bmu_calc_info.valid_flag = true;
}




