/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sys_run.h
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
#ifndef __SYS_RUN_H_
#define __SYS_RUN_H_

#include "bms_data_manage.h"
#include "bmu_ctl.h"
#include "common.h"
#include "high_volt_detect.h"
#include "ins_res_detect.h"
#include "alarm.h"
#include "peri_ctl_stat_api.h"
#include "bcu_sox.h"

typedef enum
{
    EN_SYS_RUN_STAT_IDLE,
    EN_SYS_RUN_STAT_WAIT,//初始Wait
    EN_SYS_RUN_STAT_BMU_ADDRESSING,//从控编址
    EN_SYS_RUN_STAT_WAIT_FULL_REPORT,//等待从控已经上报了全量数据
    EN_SYS_RUN_STAT_HV_EXAM,//总压采集检验,主要是和BMU上报的总压之和对比
    EN_SYS_RUN_STAT_INS_RES_DETECT,//绝缘阻抗检测
    EN_SYS_RUN_STAT_RELAY_QF_FB_JUDGE,//继电器及脱扣状态判断，是否都是关闭状态
    EN_SYS_RUN_STAT_WAIT_CLUSTER_CMD,//等待并簇指令
    EN_SYS_RUN_STAT_RELAY_PRE_ON,//进行预充
    EN_SYS_RUN_STAT_RELAY_P_ON,//打开总正继电器
    EN_SYS_RUN_STAT_NORMAL_RUN,//正常运行
    EN_SYS_RUN_STAT_QUIT_CLUSTER_CMD,//退簇指令处理
    EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_BREAKER,//退簇断开断路器(只有退簇过程中电流未按流程降低至阈值才触发)
    EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_RELAY,//退簇断开继电器
    EN_SYS_RUN_STAT_FAIL,
}EN_SYS_RUN_STAT;



typedef struct _sys_run_info_stru_t
{
    EN_SYS_RUN_STAT             sys_stat;
    uint8_t                     cluster_cmd;//0代表无动作，1代表并簇，2代表退簇
}SysRunInfo_t;

extern SysRunInfo_t gSysRunInfo;

void SysRunProcess(void);





#endif





