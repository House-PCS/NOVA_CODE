/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sys_run.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
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
#include "sys_run.h"
#include "systick.h"

SysRunInfo_t gSysRunInfo;




/** 
* @brief  模块运行状态大轮询处理
* @param  none
* @return none
* @note:
*/
void SysRunProcess()
{
    static uint32_t time_count = 0;
	
    switch(gSysRunInfo.sys_stat)
    {
        case EN_SYS_RUN_STAT_IDLE:
        {
            /*确保继电器是断开的*/
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
            gBMSDataInfo.relay_pre_stat = false;
            RelayPreCtl(gBMSDataInfo.relay_pre_stat);
            time_count = tim_get_systime();
            gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_WAIT;
            break;
        }
        case EN_SYS_RUN_STAT_WAIT:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 1000)//1S等待
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_BMU_ADDRESSING;
            }
            break;
        }
        case EN_SYS_RUN_STAT_BMU_ADDRESSING://上电BMU编址
        {
            EN_OP_RES res;
            res = BMUCfgPolling();
            if(res == EN_OP_RES_COMPLETE)
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_WAIT_FULL_REPORT;
                time_count = tim_get_systime();
            }
            else if(res == EN_OP_RES_FAIL)
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
            }
            break;
        }
        case EN_SYS_RUN_STAT_WAIT_FULL_REPORT://上电等待BMU全量上报
        {
            if(gBMSDataInfo.bmu_calc_info.valid_flag == true)
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_HV_EXAM;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 2000)//2S超时
                {
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
                }
            }
            break;
        }
        case EN_SYS_RUN_STAT_HV_EXAM://上电总压检测值和BMU上报的总压对比
        {
            if(abs((int)(gHighVoltInfo.v_bp - gBMSDataInfo.bmu_calc_info.cell_total_volt)) <= gBMSDataInfo.bcu_func_threshold.hv_bmu_total_diff_max_volt)
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_INS_RES_DETECT;
                InsResDetectStart();
            }
            else
            {
                //要么高压采样不准，要么BMU上报的电压信息有误
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
            }
            break;
        }
        case EN_SYS_RUN_STAT_INS_RES_DETECT://上电绝缘阻抗检测
        {
            if(gInsResInfo.detect_stat == EN_INS_RES_DETECT_STAT_COMPLETE)
            {
                //判断绝缘阻抗值是否满足要求
                if(gInsResInfo.bp_res >= gAlarmThreshold.ins_res_low_threshold[EN_ALARM_LEVEL_1] && gInsResInfo.bn_res >= gAlarmThreshold.ins_res_low_threshold[EN_ALARM_LEVEL_1])
                {
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_RELAY_QF_FB_JUDGE;
                }
                else
                {
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
                }
            }
            else if(gInsResInfo.detect_stat == EN_INS_RES_DETECT_STAT_ERROR)
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
            }
            break;
        }
        case EN_SYS_RUN_STAT_RELAY_QF_FB_JUDGE://继电器状态判断，此时确保处于断开状态
        {
            if(RelayPFeedback() == true || RelayPreFeedback() == true)//有继电器处于闭合状态
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
            }
            else if(BreakerFeedback() == true)//如果脱扣触发，不能往下走
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
            }
            else
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_WAIT_CLUSTER_CMD;
            }
            break;
        }
        case EN_SYS_RUN_STAT_WAIT_CLUSTER_CMD://等待PCS并簇指令
        {
            if(gSysRunInfo.cluster_cmd == 1)
            {
                gSysRunInfo.cluster_cmd = 0;
				InsResDetectStop();
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_RELAY_PRE_ON;
                gBMSDataInfo.relay_pre_stat = true;
                RelayPreCtl(gBMSDataInfo.relay_pre_stat);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_SYS_RUN_STAT_RELAY_PRE_ON://进行预充,判断B+和P+电压差是否满足并簇要求
        {
            if((uint32_t)(tim_get_systime() - time_count) > 1000)//确保继电器打开完全
            {
                if(abs((int)(gHighVoltInfo.v_bp - gHighVoltInfo.v_pp)) <= gBMSDataInfo.bcu_func_threshold.pre_charge_diff_volt && abs(gPackInfo.current) < gBMSDataInfo.bcu_func_threshold.pre_charge_complete_max_current)//压差小于一定值，电流小于2A
                {
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_RELAY_P_ON;
                    gBMSDataInfo.relay_p_stat = true;
                    RelayPCtl(gBMSDataInfo.relay_p_stat);
                    time_count = tim_get_systime();
                }
            }
            if(gSysRunInfo.cluster_cmd == 2)//收到退簇指令
            {
                gSysRunInfo.cluster_cmd = 0;
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_QUIT_CLUSTER_CMD;
            }
            if((uint32_t)(tim_get_systime() - time_count) > 10000)//预充时间过长
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
            }
            break;
        }
        case EN_SYS_RUN_STAT_RELAY_P_ON://打开总正
        {
            if((uint32_t)(tim_get_systime() - time_count) > 1000)//确保继电器打开完全
            {
                if(RelayPFeedback() == false)//总正继电器反馈不对
                {
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
                }
                else
                {
                    //关闭预充继电器
                    gBMSDataInfo.relay_pre_stat = false;
                    RelayPreCtl(gBMSDataInfo.relay_pre_stat);
                    //进入正常运行状态
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_NORMAL_RUN;
                }
            }
            break;
        }
        case EN_SYS_RUN_STAT_NORMAL_RUN://正常运行，此处主要是等待关机、重启、退簇等动作
        {
            if(gSysRunInfo.cluster_cmd == 2)//收到退簇指令
            {
                gSysRunInfo.cluster_cmd = 0;
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_QUIT_CLUSTER_CMD;
                gBMSDataInfo.current_forbid_flag = true;//停止电流
                time_count = tim_get_systime();
            }
            if(BreakerFeedback() == true)//正常运行过程中出现断路器脱扣
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_BREAKER;
                time_count = tim_get_systime();
            }
            if(RelayPFeedback() == false)//正常运行过程中主正继电器断开(大部分情况是发生了部分三级故障，小部分情况是继电器损坏)
            {
                gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
            }
//			if(gPackInfo.mode == EN_PACK_CURRENT_CHARGE_MODE || gPackInfo.mode == EN_PACK_CURRENT_DISCHARGE_MODE)
			if(gBMSDataInfo.relay_p_stat == true)
			{
				InsResDetectStop();
			}
			else
			{
				InsResDetectStart();
			}
			//绝缘检测手动开关
//            if(gBMSDataInfo.running_ins_res_cmd == true)
//            {
//                InsResDetectStart();
//            }
//            else
//            {
//                InsResDetectStop();
//            }
            break;
        }
        case EN_SYS_RUN_STAT_QUIT_CLUSTER_CMD:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 1000)//确保PCS完成电流的停止
            {
                if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)//电流没停止或达到阈值以下
                {
                    //触发断路器保护
                    gBMSDataInfo.breaker_stat = true;
                    BreakerCtl(gBMSDataInfo.breaker_stat);
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_BREAKER;
                    time_count = tim_get_systime();
                }
                else
                {
                    //切断主正继电器
                    gBMSDataInfo.relay_p_stat = false;
                    RelayPCtl(gBMSDataInfo.relay_p_stat);
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_RELAY;
                    time_count = tim_get_systime();
                }
            }
            break;
        }
        case EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_BREAKER:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 1000)
            {
                if(BreakerFeedback() == true)//确认断路器已经切断
                {
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_RELAY;
                    gBMSDataInfo.breaker_stat = false;
                    BreakerCtl(gBMSDataInfo.breaker_stat);
                    //切断主正继电器
                    gBMSDataInfo.relay_p_stat = false;
                    RelayPCtl(gBMSDataInfo.relay_p_stat);
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_RELAY;
                    time_count = tim_get_systime();
                }
                else//断路器存在故障
                {
                    //切断主正继电器
                    gBMSDataInfo.relay_p_stat = false;
                    RelayPCtl(gBMSDataInfo.relay_p_stat);
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_RELAY;
                    time_count = tim_get_systime();
                }
            }
            break;
        }
        case EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_RELAY:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 1000)
            {
                if(RelayPFeedback() == false)//确认总正继电器已经切断
                {
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_WAIT_CLUSTER_CMD;//等待下次并簇指令
                    gBMSDataInfo.current_forbid_flag = false;//恢复电流
					InsResDetectStart();
                    time_count = tim_get_systime();
                }
                else
                {
                    gSysRunInfo.sys_stat = EN_SYS_RUN_STAT_FAIL;
                }
            }
            break;
        }
        case EN_SYS_RUN_STAT_FAIL:
        {
            //等待人为处理
            
            break;
        }
        default:break;
    }
    
    if(gBMSDataInfo.bcu_reset_flag == true)
    {
        if((uint32_t)(tim_get_systime() - gBMSDataInfo.bcu_reset_delay_count) > 3000)//延时3S
        {
            //系统复位
            gBMSDataInfo.bcu_reset_flag = false;
            __set_FAULTMASK(1);
            NVIC_SystemReset();
        }
    }
    else
    {
        gBMSDataInfo.bcu_reset_delay_count = tim_get_systime();
    }
    
    //脱扣需要特殊处理
    if(BreakerFeedback() == true)//如果断路器已经切断
    {
        gBMSDataInfo.breaker_stat = false;
        BreakerCtl(gBMSDataInfo.breaker_stat);
    }
}








