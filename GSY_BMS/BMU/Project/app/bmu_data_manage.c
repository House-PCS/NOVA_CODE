
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bmu_data_manage.c
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
#include "bmu_data_manage.h"

BMUDataInfo_t gBMUDataInfo;





/** 
* @brief    BMS需要计算的数据处理大轮询
* @param    none
* @return   none
* @note:
*/
void BMSCalcDataProcess()
{
	
	 uint32_t total_volt = 0;
    uint16_t ave_volt = 0;
    uint8_t  max_volt_num = 1;
    uint16_t max_volt = gBMUDataInfo.bmu_detail_volt_info.cell_volt[0];
    uint8_t  min_volt_num = 1;
    uint16_t min_volt = gBMUDataInfo.bmu_detail_volt_info.cell_volt[0];
    int16_t  total_temp = 0;
    int8_t   ave_temp = 0;
    uint8_t  max_temp_num = 1;
    int8_t   max_temp = gBMUDataInfo.bmu_detail_temp_info.cell_temp[0];
    uint8_t  min_temp_num = 1;
    int8_t   min_temp = gBMUDataInfo.bmu_detail_temp_info.cell_temp[0];
    uint8_t  max_pole_temp_num = 1;
    int8_t   max_pole_temp = gBMUDataInfo.bmu_detail_pole_temp_info.pole_temp[0];
    uint8_t  min_pole_temp_num = 1;
    int8_t   min_pole_temp = gBMUDataInfo.bmu_detail_pole_temp_info.pole_temp[0];
    int8_t   max_balance_temp = gBMUDataInfo.bmu_detail_balance_temp_info.balance_temp[0];
	
    if(gAFESampleInfo.volt_sample_flag == true)
    {
        gBMUDataInfo.bmu_statis_volt_info.valid_flag = true;
        gBMUDataInfo.bmu_detail_volt_info.valid_flag = true;
    }
    if(gAFESampleInfo.temp_sample_flag == true)
    {
        uint8_t i = 0;
        
        gBMUDataInfo.bmu_statis_temp_info.valid_flag = true;
        gBMUDataInfo.bmu_detail_temp_info.valid_flag = true;
        gBMUDataInfo.bmu_detail_pole_temp_info.valid_flag = true;
        gBMUDataInfo.bmu_detail_other_temp_info.valid_flag = true;
        //温度检测故障判断
        for(i = 0; i < AFE_NUM; i++)
        {
            if(gAFESampleInfo.temp_ow_stat[i] > 0x1C00)//有断线情况
            {
                break;
            }
        }
        if(i == AFE_NUM)//无断线情况
        {
            gBMUDataInfo.bmu_statis_other_info.fault_info &= ~(1 << 2);
        }
        else//有断线情况
        {
            gBMUDataInfo.bmu_statis_other_info.fault_info |= (1 << 2);
        }
    }
    if(gAFESampleInfo.volt_sample_flag == true && gAFESampleInfo.temp_sample_flag == true &&gAFESampleInfo.balance_temp_sample_flag == true)
    {
        uint8_t i = 0;
        
        gBMUDataInfo.bmu_statis_other_info.valid_flag = true;
        gBMUDataInfo.bmu_detail_balance_temp_info.valid_flag = true;
        //均衡温度检测故障判断
        for(i = 0; i < AFE_NUM; i++)
        {
            if(gAFESampleInfo.balance_temp_ow_stat[i] != 0)//有断线情况
            {
                break;
            }
        }
        if(i == AFE_NUM)//无断线情况
        {
            gBMUDataInfo.bmu_statis_other_info.fault_info &= ~(1 << 4);
        }
        else//有断线情况
        {
            gBMUDataInfo.bmu_statis_other_info.fault_info |= (1 << 4);
        }
    }
    if(gAFESampleInfo.ow_sample_flag == true)
    {
        uint8_t i = 0;
        //断线检测故障判断(电压)
        for(i = 0; i < AFE_NUM; i++)
        {
            if(gAFESampleInfo.cell_ow_stat[i] > 0x8000)//有断线情况
            {
                break;
            }
        }
        if(i == AFE_NUM)//无断线情况
        {
            gBMUDataInfo.bmu_statis_other_info.fault_info &= ~(1 << 0);
        }
        else//有断线情况
        {
            gBMUDataInfo.bmu_statis_other_info.fault_info |= (1 << 0);
        }
    }
    
    //风扇故障判断
    //to do 先获取fan_rd_duty，再和fan_duty对比，如差值大于10，则认为风扇有故障
    DevPwmInGet(&gBMUDataInfo.fan_rd_freq, &gBMUDataInfo.fan_rd_duty);
    if(abs((int)gBMUDataInfo.fan_rd_duty - (int)gBMUDataInfo.fan_duty) > 10)
    {
        gBMUDataInfo.bmu_statis_other_info.fault_info |= (1 << 9);
    }
    else
    {
        gBMUDataInfo.bmu_statis_other_info.fault_info &= ~(1 << 9);
    }
    
    //AFE故障判断
//    if(gAFESampleInfo.cell_v_error_num != 0 || gAFESampleInfo.cell_t_error_num != 0 || 
//       gAFESampleInfo.bt_error_num != 0 || gAFESampleInfo.balance_error_num != 0 ||
//       gAFESampleInfo.ow_error_num != 0)
	uint8_t  error_num_temp = 50;
	if(gAFESampleInfo.cell_v_error_num > error_num_temp || gAFESampleInfo.cell_t_error_num > error_num_temp || 
	gAFESampleInfo.bt_error_num > error_num_temp || gAFESampleInfo.balance_error_num > error_num_temp ||
	gAFESampleInfo.ow_error_num > error_num_temp)
    {
        gBMUDataInfo.bmu_statis_other_info.fault_info |= (1 << 8);   //暂时屏蔽
    }
    else
    {
        gBMUDataInfo.bmu_statis_other_info.fault_info &= ~(1 << 8);  //暂时屏蔽
    }
    
    //数据计算
    //根据mask计算实际的电芯和温度数量
    uint16_t per_bmu_cell_act_num = 0;//每个bmu实际电芯数量
    uint16_t per_bmu_cell_temp_act_num = 0;//每个bmu实际的电芯温度采样数量
    uint16_t per_bmu_pole_temp_act_num = 0;//每个bmu实际的极柱温度采样数量
    uint16_t per_bmu_other_temp_act_num = 0;//每个bmu实际的正负极温度采样数量
    uint16_t per_bmu_balance_temp_act_num = 0;//每个bmu实际的均衡温度采样数量
    
    uint16_t *p_calc_volt = &gBMUDataInfo.bmu_detail_volt_info.cell_volt[0];
    uint16_t *p_afe_volt = &gAFESampleInfo.cell_voltage[0][0];
    for(uint16_t i = 0; i < sizeof(gBMUDataInfo.bmu_addr_mask_info.cell_volt_mask) / sizeof(uint8_t); i++)
    {
        for(uint8_t j = 0; j < 8; j++)
        {
            if(((gBMUDataInfo.bmu_addr_mask_info.cell_volt_mask[i] >> j) & 0x01) == 0x01)
            {
                per_bmu_cell_act_num++;
                //获取详细电压，根据mask信息
                *p_calc_volt = *p_afe_volt;
                p_calc_volt++;
                p_afe_volt++;
            }
            else
            {
                p_afe_volt++;
            }
        }
    }
    
    int8_t *p_calc_temp = &gBMUDataInfo.bmu_detail_temp_info.cell_temp[0];
    float *p_afe_temp = &gAFESampleInfo.cell_temp[0][0];
	int a = 0;
    for(uint16_t i = 0; i < sizeof(gBMUDataInfo.bmu_addr_mask_info.cell_temp_mask) / sizeof(uint8_t); i++)
    {
        for(uint8_t j = 0; j < 8; j++)
        {
            if(((gBMUDataInfo.bmu_addr_mask_info.cell_temp_mask[i] >> j) & 0x01) == 0x01)
            {
                per_bmu_cell_temp_act_num++;
                //获取详细温度(包含单体、极柱和正负极)，根据mask信息
                *p_calc_temp = (int8_t)(*p_afe_temp + 0.5);
                p_calc_temp++;
                p_afe_temp++;
            }
            else
            {
                p_afe_temp++;
            }
        }
    }
    
    int8_t *p_calc_pole_temp = &gBMUDataInfo.bmu_detail_pole_temp_info.pole_temp[0];
    float *p_afe_pole_temp = &gAFESampleInfo.cell_temp[0][0];
    for(uint16_t i = 0; i < sizeof(gBMUDataInfo.bmu_addr_mask_info.pole_temp_mask) / sizeof(uint8_t); i++)
    {
        for(uint8_t j = 0; j < 8; j++)
        {
            if(((gBMUDataInfo.bmu_addr_mask_info.pole_temp_mask[i] >> j) & 0x01) == 0x01)
            {
                per_bmu_pole_temp_act_num++;
                //获取详细极柱温度，根据mask信息
                *p_calc_pole_temp = (int8_t)(*p_afe_pole_temp + 0.5);
                p_calc_pole_temp++;
                p_afe_pole_temp++;
            }
            else
            {
                p_afe_pole_temp++;
            }
        }
    }
    
    int8_t *p_calc_other_temp = &gBMUDataInfo.bmu_detail_other_temp_info.other_temp[0];
    float *p_afe_other_temp = &gAFESampleInfo.cell_temp[0][0];
    for(uint16_t i = 0; i < sizeof(gBMUDataInfo.bmu_addr_mask_info.other_temp_mask) / sizeof(uint8_t); i++)
    {
        for(uint8_t j = 0; j < 8; j++)
        {
            if(((gBMUDataInfo.bmu_addr_mask_info.other_temp_mask[i] >> j) & 0x01) == 0x01)
            {
                per_bmu_other_temp_act_num++;
                //获取详细正负极温度，根据mask信息
                *p_calc_other_temp = (int8_t)(*p_afe_other_temp + 0.5);
                p_calc_other_temp++;
                p_afe_other_temp++;
            }
            else
            {
                p_afe_other_temp++;
            }
        }
    }
    
    int8_t *p_calc_balance_temp = &gBMUDataInfo.bmu_detail_balance_temp_info.balance_temp[0];
    float *p_afe_balance_temp = &gAFESampleInfo.balance_temp[0][0];
    for(uint16_t i = 0; i < sizeof(gBMUDataInfo.bmu_addr_mask_info.balance_temp_mask) / sizeof(uint8_t); i++)
    {
        for(uint8_t j = 0; j < 8; j++)
        {
            if(((gBMUDataInfo.bmu_addr_mask_info.balance_temp_mask[i] >> j) & 0x01) == 0x01)
            {
                per_bmu_balance_temp_act_num++;
                //获取详细均衡温度，根据mask信息
                *p_calc_balance_temp = (int8_t)(*p_afe_balance_temp + 0.5);
                p_calc_balance_temp++;
                p_afe_balance_temp++;
            }
            else
            {
                p_afe_balance_temp++;
            }
        }
    }
    
    uint8_t calc_balance_cnt = 0;
    uint8_t afe_balance_cnt = 0;
    for(uint16_t i = 0; i < sizeof(gBMUDataInfo.bmu_addr_mask_info.cell_volt_mask) / sizeof(uint8_t); i++)
    {
        for(uint8_t j = 0; j < 8; j++)
        {
            if(((gBMUDataInfo.bmu_addr_mask_info.cell_volt_mask[i] >> j) & 0x01) == 0x01)
            {
                //获取均衡状态，根据mask信息
                if(((gAFESampleInfo.balance_stat_flag[afe_balance_cnt / 16] >> afe_balance_cnt) & 0x01) == 0x01)
                {
                    gBMUDataInfo.bmu_detail_volt_info.cell_balance_stat[calc_balance_cnt] = true;
                }
                else
                {
                    gBMUDataInfo.bmu_detail_volt_info.cell_balance_stat[calc_balance_cnt] = false;
                }
                calc_balance_cnt++;
                afe_balance_cnt++;
            }
            else
            {
                
                afe_balance_cnt++;
            }
        }
        
    }

    gBMUDataInfo.per_bmu_cell_num = per_bmu_cell_act_num;
    gBMUDataInfo.per_bmu_cell_temp_num = per_bmu_cell_temp_act_num;
    gBMUDataInfo.per_bmu_pole_temp_num = per_bmu_pole_temp_act_num;
    gBMUDataInfo.per_bmu_other_temp_num = per_bmu_other_temp_act_num;
    gBMUDataInfo.per_bmu_balance_temp_num = per_bmu_balance_temp_act_num;

   
    
    for(uint16_t i = 0; i < per_bmu_cell_act_num; i++)
    {
        total_volt += gBMUDataInfo.bmu_detail_volt_info.cell_volt[i];
        if(gBMUDataInfo.bmu_detail_volt_info.cell_volt[i] > max_volt)
        {
            max_volt_num = i;
            max_volt = gBMUDataInfo.bmu_detail_volt_info.cell_volt[i];
        }
        if(gBMUDataInfo.bmu_detail_volt_info.cell_volt[i] < min_volt)
        {
            min_volt_num = i;
            min_volt = gBMUDataInfo.bmu_detail_volt_info.cell_volt[i];
        }
    }
    ave_volt = total_volt / per_bmu_cell_act_num;
    
    for(uint16_t i = 0; i < per_bmu_cell_temp_act_num; i++)
    {
        total_temp += gBMUDataInfo.bmu_detail_temp_info.cell_temp[i];
        if(gBMUDataInfo.bmu_detail_temp_info.cell_temp[i] > max_temp)
        {
            max_temp_num = i;
            max_temp = gBMUDataInfo.bmu_detail_temp_info.cell_temp[i];
        }
        if(gBMUDataInfo.bmu_detail_temp_info.cell_temp[i] < min_temp)
        {
            min_temp_num = i;
            min_temp = gBMUDataInfo.bmu_detail_temp_info.cell_temp[i];
        }
    }
    ave_temp = total_temp / per_bmu_cell_temp_act_num;
    
    for(uint16_t i = 0; i < per_bmu_pole_temp_act_num; i++)
    {
        total_temp += gBMUDataInfo.bmu_detail_pole_temp_info.pole_temp[i];
        if(gBMUDataInfo.bmu_detail_pole_temp_info.pole_temp[i] > max_pole_temp)
        {
            max_pole_temp_num = i;
            max_pole_temp = gBMUDataInfo.bmu_detail_pole_temp_info.pole_temp[i];
        }
        if(gBMUDataInfo.bmu_detail_pole_temp_info.pole_temp[i] < min_pole_temp)
        {
            min_pole_temp_num = i;
            min_pole_temp = gBMUDataInfo.bmu_detail_pole_temp_info.pole_temp[i];
        }
    }
    
    for(uint16_t i = 0; i < per_bmu_balance_temp_act_num; i++)
    {
        if(gBMUDataInfo.bmu_detail_balance_temp_info.balance_temp[i] > max_balance_temp)
        {
            max_balance_temp = gBMUDataInfo.bmu_detail_balance_temp_info.balance_temp[i];
        }
    }
    
    gBMUDataInfo.bmu_statis_volt_info.total_volt = total_volt;
    gBMUDataInfo.bmu_statis_volt_info.cell_max_volt_num = max_volt_num;
    gBMUDataInfo.bmu_statis_volt_info.cell_max_volt = max_volt;
    gBMUDataInfo.bmu_statis_volt_info.cell_min_volt_num = min_volt_num;
    gBMUDataInfo.bmu_statis_volt_info.cell_min_volt = min_volt;
    
    gBMUDataInfo.bmu_statis_temp_info.cell_max_temp_num = max_temp_num;
    gBMUDataInfo.bmu_statis_temp_info.cell_max_temp = max_temp;
    gBMUDataInfo.bmu_statis_temp_info.cell_min_temp_num = min_temp_num;
    gBMUDataInfo.bmu_statis_temp_info.cell_min_temp = min_temp;
    gBMUDataInfo.bmu_statis_temp_info.pole_max_temp_num = max_pole_temp_num;
    gBMUDataInfo.bmu_statis_temp_info.pole_max_temp = max_pole_temp;
    gBMUDataInfo.bmu_statis_temp_info.pole_min_temp_num = min_pole_temp_num;
    gBMUDataInfo.bmu_statis_temp_info.pole_min_temp = min_pole_temp;

    gBMUDataInfo.bmu_statis_other_info.cell_ave_volt = ave_volt;
    gBMUDataInfo.bmu_statis_other_info.cell_ave_temp = ave_temp;
    gBMUDataInfo.bmu_statis_other_info.equalizing_plate_max_temp = max_balance_temp;
}























