/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bmu_ctl.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-09
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "bmu_ctl.h"
#include "systick.h"
#include "peri_ctl_stat_api.h"

/*
 * 该模块根据BMU的数据状态，控制BMU的编址、掩码信息(电芯、温度等)、风扇转速、均衡功能等
 */
 

//BMUSampleInfo_t gBMSDataInfo.bmu_sample_info;
BMUCfgInfo_t gBMUCfgInfo;
BMUCtlInfo_t gBMUCtlInfo;

/*********初始化地址分配和掩码发送部分*********/


/** 
* @brief  BMU初始化配置，包括地址分配和掩码发送，由sys_run调用，需要通过状态机轮询调用，直到获取到结果
* @param  none
* @return 配置结果
* @note:
*/
EN_OP_RES BMUCfgPolling()
{
    static EN_BMU_CTL_CFG_STAT cfg_stat = EN_BMU_CTL_CFG_STAT_START;
    EN_OP_RES res = EN_OP_RES_ING;
    static uint32_t time_count = 0;
    
    switch(cfg_stat)
    {
        case EN_BMU_CTL_CFG_STAT_START:
        {
            //打开风扇继电器
            RelayFanCtl(true);
            
            gBMUCfgInfo.cur_addr_offset = 0;//分配地址偏移清零
//            BMUAddressingMainOutCtl(false);//当前先仅支持双向IO的主通道
//            BMUAddressingBackupOutCtl(true);//暂不使用备份通道
			BMUAddressingMainOutCtl(true);//当前先仅支持双向IO的主通道
            BMUAddressingBackupOutCtl(false);//暂不使用备份通道
            //下发编址清除指令标志置位
            gBMUCfgInfo.start_flag = true;
            cfg_stat = EN_BMU_CTL_CFG_STAT_START_WAIT;
            time_count = tim_get_systime();
            break;
        }
        case EN_BMU_CTL_CFG_STAT_START_WAIT:
        {
            /*start没有回包，隔50ms后开始编址*/
            if((uint32_t)(tim_get_systime() - time_count) > 200)
            {
                cfg_stat = EN_BMU_CTL_CFG_STAT_ADDR_DISTR;
                gBMUCfgInfo.cur_addr_offset = 0;
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_BMU_CTL_CFG_STAT_ADDR_DISTR:
        {
			if((uint32_t)(tim_get_systime() - time_count) > 80)
			{
				gBMUCfgInfo.distr_flag = true;
				gBMUCfgInfo.distr_ack = false;
				cfg_stat = EN_BMU_CTL_CFG_STAT_ADDR_DISTR_WAIT;
				time_count = tim_get_systime();
			}
            
            break;
        }
        case EN_BMU_CTL_CFG_STAT_ADDR_DISTR_WAIT:
        {
            if(gBMUCfgInfo.distr_ack == true)
            {
                gBMUCfgInfo.cur_addr_offset++;
                if(gBMUCfgInfo.cur_addr_offset >= gBMSDataInfo.bmu_sample_info.pack_num)//所有从控均已编址
                {
                    cfg_stat = EN_BMU_CTL_CFG_STAT_ADDR_LATCH;
                }
                else
                {
                    cfg_stat = EN_BMU_CTL_CFG_STAT_ADDR_DISTR;
					time_count = tim_get_systime();
                }
                time_count = tim_get_systime();
            }
            if((uint32_t)(tim_get_systime() - time_count) > 1000)//1000ms超时
            {
                cfg_stat = EN_BMU_CTL_CFG_STAT_ERROR;
            }
            break;
        }          
        case EN_BMU_CTL_CFG_STAT_ADDR_LATCH:
        {
            gBMUCfgInfo.latch_flag = true;
            memset(gBMUCfgInfo.latch_ack, false, gBMSDataInfo.bmu_sample_info.pack_num);
            cfg_stat = EN_BMU_CTL_CFG_STAT_ADDR_LATCH_WAIT;
            time_count = tim_get_systime();
            break;
        }
        case EN_BMU_CTL_CFG_STAT_ADDR_LATCH_WAIT:
        {
            uint8_t i = 0;
            for(i = 0; i < gBMSDataInfo.bmu_sample_info.pack_num; i++)
            {
                if(gBMUCfgInfo.latch_ack[i] == false)
                {
                    break;
                }
            }
            if(i == gBMSDataInfo.bmu_sample_info.pack_num)
            {
                cfg_stat = EN_BMU_CTL_CFG_STAT_MASK_SEND;
                time_count = tim_get_systime();
            }
            if((uint32_t)(tim_get_systime() - time_count) > 100)//100ms超时
            {
                cfg_stat = EN_BMU_CTL_CFG_STAT_ERROR;
            }
            break;
        }
        case EN_BMU_CTL_CFG_STAT_MASK_SEND:
        {
            gBMUCfgInfo.mask_flag = true;
            memset(gBMUCfgInfo.mask_ack, false, gBMSDataInfo.bmu_sample_info.pack_num);
            cfg_stat = EN_BMU_CTL_CFG_STAT_MASK_SEND_WAIT;
            time_count = tim_get_systime();
            break;
        }
        case EN_BMU_CTL_CFG_STAT_MASK_SEND_WAIT:
        {
            uint8_t i = 0;
            for(i = 0; i < gBMSDataInfo.bmu_sample_info.pack_num; i++)
            {
                if(gBMUCfgInfo.mask_ack[i] == false)
                {
                    break;
                }
            }
            if(i == gBMSDataInfo.bmu_sample_info.pack_num)
            {
                cfg_stat = EN_BMU_CTL_CFG_STAT_OK;
                time_count = tim_get_systime();
            }
            if((uint32_t)(tim_get_systime() - time_count) > 100)//100ms超时
            {
                cfg_stat = EN_BMU_CTL_CFG_STAT_ERROR;
            }
            break;
        }
        case EN_BMU_CTL_CFG_STAT_OK:
        {
            /*地址分配+掩码发送完成*/
            gBMSDataInfo.bmu_num = gBMSDataInfo.bmu_sample_info.pack_num;
            
            uint16_t per_bmu_cell_act_num = 0;//每个bmu实际电芯数量
            uint16_t per_bmu_cell_temp_act_num = 0;//每个bmu实际的电芯温度采样数量
            uint16_t per_bmu_pole_temp_act_num = 0;//每个bmu实际的极柱温度采样数量
            uint16_t per_bmu_other_temp_act_num = 0;//每个bmu实际的正负极温度采样数量
            /**/
            for(uint16_t i = 0; i < sizeof(gBMSDataInfo.bmu_sample_info.cell_volt_mask)/sizeof(uint8_t); i++)
            {
                for(uint8_t j = 0; j < 8; j++)
                {
                    if(((gBMSDataInfo.bmu_sample_info.cell_volt_mask[i] >> j) & 0x01) == 0x01)
                    {
                        per_bmu_cell_act_num++;
                    }
                }
            }
            for(uint16_t i = 0; i < sizeof(gBMSDataInfo.bmu_sample_info.cell_temp_mask)/sizeof(uint8_t); i++)
            {
                for(uint8_t j = 0; j < 8; j++)
                {
                    if(((gBMSDataInfo.bmu_sample_info.cell_temp_mask[i] >> j) & 0x01) == 0x01)
                    {
                        per_bmu_cell_temp_act_num++;
                    }
                }
            }
            for(uint16_t i = 0; i < sizeof(gBMSDataInfo.bmu_sample_info.pole_temp_mask)/sizeof(uint8_t); i++)
            {
                for(uint8_t j = 0; j < 8; j++)
                {
                    if(((gBMSDataInfo.bmu_sample_info.pole_temp_mask[i] >> j) & 0x01) == 0x01)
                    {
                        per_bmu_pole_temp_act_num++;
                    }
                }
            }
            for(uint16_t i = 0; i < sizeof(gBMSDataInfo.bmu_sample_info.other_temp_mask)/sizeof(uint8_t); i++)
            {
                for(uint8_t j = 0; j < 8; j++)
                {
                    if(((gBMSDataInfo.bmu_sample_info.other_temp_mask[i] >> j) & 0x01) == 0x01)
                    {
                        per_bmu_other_temp_act_num++;
                    }
                }
            }
            gBMSDataInfo.per_bmu_cell_num = per_bmu_cell_act_num;
            gBMSDataInfo.per_bmu_cell_temp_num = per_bmu_cell_temp_act_num;
            gBMSDataInfo.per_bmu_pole_temp_num = per_bmu_pole_temp_act_num;
            gBMSDataInfo.per_bmu_other_temp_num = per_bmu_other_temp_act_num;
            
            cfg_stat = EN_BMU_CTL_CFG_STAT_START;
            res = EN_OP_RES_COMPLETE;
            break;
        }
        case EN_BMU_CTL_CFG_STAT_ERROR:
        {
            cfg_stat = EN_BMU_CTL_CFG_STAT_START;
            res = EN_OP_RES_FAIL;
            break;
        }
        default:break;
    }
    gBMSDataInfo.bmu_adressing_res = res;
    return res;
}



#define BMU_CTL_PERIOD              1000
#define BMU_BALANCE_CTL_PERIOD      1000



/** 
* @brief  BMU控制大轮询
* @param  none
* @return none
* @note:
*/
void BMUCtlProcess()
{
    static uint32_t ctl_time_count = 0;
    static uint32_t passive_balance_time_count = 0;
    //未编址完成，不控制从控
    if(gBMSDataInfo.bmu_adressing_res != EN_OP_RES_COMPLETE)
    {
        return;
    }

    //控制报文
    if((uint32_t)(tim_get_systime() - ctl_time_count) >= BMU_CTL_PERIOD)
    {
        ctl_time_count = tim_get_systime();
        
        for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
        {
            uint8_t *p_fan_pwm = &gBMSDataInfo.bmu_io_ctl_info[i].pwm_duty_1;//方便后续如果使用别的管脚，代码改动小
            
            //先确保所有数据都上报一遍后再执行
            if(gBMSDataInfo.bmu_io_info[i].valid_flag != true || gBMSDataInfo.bmu_statis_volt_info[i].valid_flag != true || 
               gBMSDataInfo.bmu_statis_temp_info[i].valid_flag != true || gBMSDataInfo.bmu_statis_other_info[i].valid_flag != true)
            {
                continue;
            }
            gBMUCtlInfo.ctl_flag[i] = true;
            //上报报文使能
            gBMSDataInfo.bmu_io_ctl_info[i].report_enable = 0x3;
            //DO输出, 暂时不用
            
            //风扇策略
            if(gBMSDataInfo.bmu_statis_temp_info[i].cell_max_temp >= gBMSDataInfo.bcu_func_threshold.fan_full_temp)
            {
                *p_fan_pwm = 100;
            }
            else if(gBMSDataInfo.bmu_statis_temp_info[i].cell_max_temp >= gBMSDataInfo.bcu_func_threshold.fan_start_temp)
            {
                *p_fan_pwm = (float)(100 - gBMSDataInfo.bcu_func_threshold.fan_start_duty) * (gBMSDataInfo.bmu_statis_temp_info[i].cell_max_temp - gBMSDataInfo.bcu_func_threshold.fan_start_temp) /
                             (gBMSDataInfo.bcu_func_threshold.fan_full_temp - gBMSDataInfo.bcu_func_threshold.fan_start_temp) + gBMSDataInfo.bcu_func_threshold.fan_start_duty;
            }
            else
            {
                if(*p_fan_pwm != 0)//风扇原本在转
                {
                    if(gBMSDataInfo.bmu_statis_temp_info[i].cell_max_temp >= gBMSDataInfo.bcu_func_threshold.fan_stop_temp)
                    {
                        *p_fan_pwm = gBMSDataInfo.bcu_func_threshold.fan_start_duty;
                    }
                    else
                    {
                        *p_fan_pwm = 0;
                    }
                }
            }
        }
    }
    
    //均衡报文
    if((uint32_t)(tim_get_systime() - passive_balance_time_count) >= BMU_BALANCE_CTL_PERIOD)
    {
        passive_balance_time_count = tim_get_systime();
        
        for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
        {
            //先确保所有数据都上报一遍后再执行
            if(gBMSDataInfo.bmu_calc_info.valid_flag != true)//未完成一次完整的计算，不处理均衡
            {
                continue;
            }
            
            gBMUCtlInfo.passive_balance_flag[i] = true;
            //需要判断每节电压和总的最低电芯电压的压差，主控只下发要不要均衡标志，具体是奇偶均衡还是连续均衡，取决于从控
            for(uint8_t j = 0; j < gBMSDataInfo.per_bmu_cell_num; j++)
            {
                if(gBMSDataInfo.bmu_detail_volt_info[i].cell_volt[j] - gBMSDataInfo.bmu_calc_info.cell_min_volt > gBMSDataInfo.bcu_func_threshold.balance_start_diff_volt)
                {
                    if(gBMSDataInfo.bmu_detail_volt_info[i].cell_volt[j] > gBMSDataInfo.bcu_func_threshold.balance_min_volt)
                    {
                        gBMSDataInfo.bmu_passive_balance_info[i][j / 8] |= (1 << (j % 8));
                    }
                    else
                    {
                        gBMSDataInfo.bmu_passive_balance_info[i][j / 8] &= ~(1 << (j % 8));
                    }
                }
                else
                {
                    gBMSDataInfo.bmu_passive_balance_info[i][j / 8] &= ~(1 << (j % 8));
                }
            }
        }
    }
}




