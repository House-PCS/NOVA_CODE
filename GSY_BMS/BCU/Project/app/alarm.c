/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        alarm.c
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
  * <tr><td>2024-09-02    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "alarm.h"
#include "systick.h"
#include "bcu_sox.h"
#include "bms_data_manage.h"
#include "temp_detect.h"
#include "high_volt_detect.h"
#include "ins_res_detect.h"
#include "peri_ctl_stat_api.h"

/*
 * 该模块只做报警标志的置位，不做处理动作
 */


//MeasureAlarm
static bool _MeasureAlarmCellVoltHighJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmCellVoltLowJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmCellTempHighJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmCellTempLowJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmBox1TempHighJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmBox2TempHighJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmBox1TempLowJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmBox2TempLowJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmBPVoltHighJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmBPVoltLowJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmPPVoltHighJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmPPVoltLowJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmSOCLowJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmSOHLowJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmChgCurHighJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmDsgCurHighJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmInsImpLowJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmCellVoltDiffHighJudge(EN_ALARM_LEVEL level);
static bool _MeasureAlarmCellTempDiffHighJudge(EN_ALARM_LEVEL level);
//DevAlarm
static bool _DevAlarmCellNTCShortJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmCellNTCBreakJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmAFEFaultJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmBalanceFaultJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmBMUCommFaultJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmBMUFanFaultJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmCellVoltSampFaultJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmRelayPAdhFaultJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmRelayPreAdhFaultJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmRelayFanAdhFaultJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmBMUAdressingFailJudge(EN_ALARM_LEVEL level);
static bool _DevAlarmRelayBreakerAdhFaultJudge(EN_ALARM_LEVEL level);
//OtherAlarm
static bool _OtherAlarmPoleTempHighJudge(EN_ALARM_LEVEL level);
static bool _OtherAlarmPoleTempLowJudge(EN_ALARM_LEVEL level);
static bool _OtherAlarmPCSLost(EN_ALARM_LEVEL level);

static AlarmInfo_t _gMeasureAlarmInfo[] = 
{
    {EN_MEASURE_ALARM_CELL_VOLT_HIGH,           EN_ALARM_LEVEL_1,  _MeasureAlarmCellVoltHighJudge,          0,   0},
    {EN_MEASURE_ALARM_CELL_VOLT_HIGH,           EN_ALARM_LEVEL_2,  _MeasureAlarmCellVoltHighJudge,          0,   0},
    {EN_MEASURE_ALARM_CELL_VOLT_HIGH,           EN_ALARM_LEVEL_3,  _MeasureAlarmCellVoltHighJudge,          0,   0},
    {EN_MEASURE_ALARM_CELL_VOLT_LOW,            EN_ALARM_LEVEL_1,  _MeasureAlarmCellVoltLowJudge,           0,   0},
    {EN_MEASURE_ALARM_CELL_VOLT_LOW,            EN_ALARM_LEVEL_2,  _MeasureAlarmCellVoltLowJudge,           0,   0},
    {EN_MEASURE_ALARM_CELL_VOLT_LOW,            EN_ALARM_LEVEL_3,  _MeasureAlarmCellVoltLowJudge,           0,   0},
    {EN_MEASURE_ALARM_CELL_TEMP_HIGH,           EN_ALARM_LEVEL_1,  _MeasureAlarmCellTempHighJudge,          0,   0},
    {EN_MEASURE_ALARM_CELL_TEMP_HIGH,           EN_ALARM_LEVEL_2,  _MeasureAlarmCellTempHighJudge,          0,   0},
    {EN_MEASURE_ALARM_CELL_TEMP_HIGH,           EN_ALARM_LEVEL_3,  _MeasureAlarmCellTempHighJudge,          0,   0},
    {EN_MEASURE_ALARM_CELL_TEMP_LOW,            EN_ALARM_LEVEL_1,  _MeasureAlarmCellTempLowJudge,           0,   0},
    {EN_MEASURE_ALARM_CELL_TEMP_LOW,            EN_ALARM_LEVEL_2,  _MeasureAlarmCellTempLowJudge,           0,   0},
    {EN_MEASURE_ALARM_CELL_TEMP_LOW,            EN_ALARM_LEVEL_3,  _MeasureAlarmCellTempLowJudge,           0,   0},
    {EN_MEASURE_ALARM_MOS_TEMP_HIGH,            EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_MOS_TEMP_HIGH,            EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_MOS_TEMP_HIGH,            EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_MOS_TEMP_LOW,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_MOS_TEMP_LOW,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_MOS_TEMP_LOW,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
                                                        
    {EN_MEASURE_ALARM_BOX1_TEMP_HIGH,           EN_ALARM_LEVEL_1,  _MeasureAlarmBox1TempHighJudge,          0,   0},
    {EN_MEASURE_ALARM_BOX1_TEMP_HIGH,           EN_ALARM_LEVEL_2,  _MeasureAlarmBox1TempHighJudge,          0,   0},
    {EN_MEASURE_ALARM_BOX1_TEMP_HIGH,           EN_ALARM_LEVEL_3,  _MeasureAlarmBox1TempHighJudge,          0,   0},
    {EN_MEASURE_ALARM_BOX2_TEMP_HIGH,           EN_ALARM_LEVEL_1,  _MeasureAlarmBox2TempHighJudge,          0,   0},
    {EN_MEASURE_ALARM_BOX2_TEMP_HIGH,           EN_ALARM_LEVEL_2,  _MeasureAlarmBox2TempHighJudge,          0,   0},
    {EN_MEASURE_ALARM_BOX2_TEMP_HIGH,           EN_ALARM_LEVEL_3,  _MeasureAlarmBox2TempHighJudge,          0,   0},
    {EN_MEASURE_ALARM_BOX3_TEMP_HIGH,           EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX3_TEMP_HIGH,           EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX3_TEMP_HIGH,           EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX4_TEMP_HIGH,           EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX4_TEMP_HIGH,           EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX4_TEMP_HIGH,           EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX1_TEMP_LOW,            EN_ALARM_LEVEL_1,  _MeasureAlarmBox1TempLowJudge,           0,   0},
    {EN_MEASURE_ALARM_BOX1_TEMP_LOW,            EN_ALARM_LEVEL_2,  _MeasureAlarmBox1TempLowJudge,           0,   0},
    {EN_MEASURE_ALARM_BOX1_TEMP_LOW,            EN_ALARM_LEVEL_3,  _MeasureAlarmBox1TempLowJudge,           0,   0},
    {EN_MEASURE_ALARM_BOX2_TEMP_LOW,            EN_ALARM_LEVEL_1,  _MeasureAlarmBox2TempLowJudge,           0,   0},
    {EN_MEASURE_ALARM_BOX2_TEMP_LOW,            EN_ALARM_LEVEL_2,  _MeasureAlarmBox2TempLowJudge,           0,   0},
    {EN_MEASURE_ALARM_BOX2_TEMP_LOW,            EN_ALARM_LEVEL_3,  _MeasureAlarmBox2TempLowJudge,           0,   0},
    {EN_MEASURE_ALARM_BOX3_TEMP_LOW,            EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX3_TEMP_LOW,            EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX3_TEMP_LOW,            EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX4_TEMP_LOW,            EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX4_TEMP_LOW,            EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_BOX4_TEMP_LOW,            EN_ALARM_LEVEL_3,  NULL,                                    0,   0},  
    {EN_MEASURE_ALARM_BP_OVER,                  EN_ALARM_LEVEL_1,  _MeasureAlarmBPVoltHighJudge,            0,   0},
    {EN_MEASURE_ALARM_BP_OVER,                  EN_ALARM_LEVEL_2,  _MeasureAlarmBPVoltHighJudge,            0,   0},
    {EN_MEASURE_ALARM_BP_OVER,                  EN_ALARM_LEVEL_3,  _MeasureAlarmBPVoltHighJudge,            0,   0},
    {EN_MEASURE_ALARM_BP_UNDER,                 EN_ALARM_LEVEL_1,  _MeasureAlarmBPVoltLowJudge,             0,   0},
    {EN_MEASURE_ALARM_BP_UNDER,                 EN_ALARM_LEVEL_2,  _MeasureAlarmBPVoltLowJudge,             0,   0},
    {EN_MEASURE_ALARM_BP_UNDER,                 EN_ALARM_LEVEL_3,  _MeasureAlarmBPVoltLowJudge,             0,   0},
    {EN_MEASURE_ALARM_PP_OVER,                  EN_ALARM_LEVEL_1,  _MeasureAlarmPPVoltHighJudge,            0,   0},
    {EN_MEASURE_ALARM_PP_OVER,                  EN_ALARM_LEVEL_2,  _MeasureAlarmPPVoltHighJudge,            0,   0},
    {EN_MEASURE_ALARM_PP_OVER,                  EN_ALARM_LEVEL_3,  _MeasureAlarmPPVoltHighJudge,            0,   0},
    {EN_MEASURE_ALARM_PP_UNDER,                 EN_ALARM_LEVEL_1,  _MeasureAlarmPPVoltLowJudge,             0,   0},
    {EN_MEASURE_ALARM_PP_UNDER,                 EN_ALARM_LEVEL_2,  _MeasureAlarmPPVoltLowJudge,             0,   0},
    {EN_MEASURE_ALARM_PP_UNDER,                 EN_ALARM_LEVEL_3,  _MeasureAlarmPPVoltLowJudge,             0,   0},
    {EN_MEASURE_ALARM_TOTAL_VOLT_DIFF_HIGH,     EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_TOTAL_VOLT_DIFF_HIGH,     EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_TOTAL_VOLT_DIFF_HIGH,     EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_MEASURE_ALARM_SOC_LOW,                  EN_ALARM_LEVEL_1,  _MeasureAlarmSOCLowJudge,                0,   0},
    {EN_MEASURE_ALARM_SOC_LOW,                  EN_ALARM_LEVEL_2,  _MeasureAlarmSOCLowJudge,                0,   0},
    {EN_MEASURE_ALARM_SOC_LOW,                  EN_ALARM_LEVEL_3,  _MeasureAlarmSOCLowJudge,                0,   0},
    {EN_MEASURE_ALARM_SOH_LOW,                  EN_ALARM_LEVEL_1,  _MeasureAlarmSOHLowJudge,                0,   0},
    {EN_MEASURE_ALARM_SOH_LOW,                  EN_ALARM_LEVEL_2,  _MeasureAlarmSOHLowJudge,                0,   0},
    {EN_MEASURE_ALARM_SOH_LOW,                  EN_ALARM_LEVEL_3,  _MeasureAlarmSOHLowJudge,                0,   0},
    {EN_MEASURE_ALARM_CHG_CURRENT_HIGH,         EN_ALARM_LEVEL_1,  _MeasureAlarmChgCurHighJudge,            0,   0},
    {EN_MEASURE_ALARM_CHG_CURRENT_HIGH,         EN_ALARM_LEVEL_2,  _MeasureAlarmChgCurHighJudge,            0,   0},
    {EN_MEASURE_ALARM_CHG_CURRENT_HIGH,         EN_ALARM_LEVEL_3,  _MeasureAlarmChgCurHighJudge,            0,   0},
    {EN_MEASURE_ALARM_DSG_CURRENT_HIGH,         EN_ALARM_LEVEL_1,  _MeasureAlarmDsgCurHighJudge,            0,   0},
    {EN_MEASURE_ALARM_DSG_CURRENT_HIGH,         EN_ALARM_LEVEL_2,  _MeasureAlarmDsgCurHighJudge,            0,   0},
    {EN_MEASURE_ALARM_DSG_CURRENT_HIGH,         EN_ALARM_LEVEL_3,  _MeasureAlarmDsgCurHighJudge,            0,   0},
    {EN_MEASURE_ALARM_INS_RES_LOW,              EN_ALARM_LEVEL_1,  _MeasureAlarmInsImpLowJudge,             0,   0},
    {EN_MEASURE_ALARM_INS_RES_LOW,              EN_ALARM_LEVEL_2,  _MeasureAlarmInsImpLowJudge,             0,   0},
    {EN_MEASURE_ALARM_INS_RES_LOW,              EN_ALARM_LEVEL_3,  _MeasureAlarmInsImpLowJudge,             0,   0},
    {EN_MEASURE_ALARM_CELL_VOLT_DIFF_HIGH,      EN_ALARM_LEVEL_1,  _MeasureAlarmCellVoltDiffHighJudge,      0,   0},
    {EN_MEASURE_ALARM_CELL_VOLT_DIFF_HIGH,      EN_ALARM_LEVEL_2,  _MeasureAlarmCellVoltDiffHighJudge,      0,   0},
    {EN_MEASURE_ALARM_CELL_VOLT_DIFF_HIGH,      EN_ALARM_LEVEL_3,  _MeasureAlarmCellVoltDiffHighJudge,      0,   0},
    {EN_MEASURE_ALARM_CELL_TEMP_DIFF_HIGH,      EN_ALARM_LEVEL_1,  _MeasureAlarmCellTempDiffHighJudge,      0,   0},
    {EN_MEASURE_ALARM_CELL_TEMP_DIFF_HIGH,      EN_ALARM_LEVEL_2,  _MeasureAlarmCellTempDiffHighJudge,      0,   0},
    {EN_MEASURE_ALARM_CELL_TEMP_DIFF_HIGH,      EN_ALARM_LEVEL_3,  _MeasureAlarmCellTempDiffHighJudge,      0,   0},
};

static AlarmInfo_t _gDevAlarmInfo[] = 
{
    {EN_DEV_ALARM_CELL_NTC_SHORT_FAULT,         EN_ALARM_LEVEL_1,  _DevAlarmCellNTCShortJudge,              0,   0},
    {EN_DEV_ALARM_CELL_NTC_SHORT_FAULT,         EN_ALARM_LEVEL_2,  _DevAlarmCellNTCShortJudge,              0,   0},
    {EN_DEV_ALARM_CELL_NTC_SHORT_FAULT,         EN_ALARM_LEVEL_3,  _DevAlarmCellNTCShortJudge,              0,   0},
    {EN_DEV_ALARM_CELL_NTC_BREAK_FAULT,         EN_ALARM_LEVEL_1,  _DevAlarmCellNTCBreakJudge,              0,   0},
    {EN_DEV_ALARM_CELL_NTC_BREAK_FAULT,         EN_ALARM_LEVEL_2,  _DevAlarmCellNTCBreakJudge,              0,   0},
    {EN_DEV_ALARM_CELL_NTC_BREAK_FAULT,         EN_ALARM_LEVEL_3,  _DevAlarmCellNTCBreakJudge,              0,   0},
    {EN_DEV_ALARM_MOS_NTC_SHORT_FAULT,          EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_MOS_NTC_SHORT_FAULT,          EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_MOS_NTC_SHORT_FAULT,          EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_MOS_NTC_BREAK_FAULT,          EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_MOS_NTC_BREAK_FAULT,          EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_MOS_NTC_BREAK_FAULT,          EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_AFE_FAULT,                    EN_ALARM_LEVEL_1,  _DevAlarmAFEFaultJudge,                  0,   0},
    {EN_DEV_ALARM_AFE_FAULT,                    EN_ALARM_LEVEL_2,  _DevAlarmAFEFaultJudge,                  0,   0},
    {EN_DEV_ALARM_AFE_FAULT,                    EN_ALARM_LEVEL_3,  _DevAlarmAFEFaultJudge,                  0,   0},
    {EN_DEV_ALARM_BALANCE_MOS_FAULT,            EN_ALARM_LEVEL_1,  _DevAlarmBalanceFaultJudge,              0,   0},
    {EN_DEV_ALARM_BALANCE_MOS_FAULT,            EN_ALARM_LEVEL_2,  _DevAlarmBalanceFaultJudge,              0,   0},
    {EN_DEV_ALARM_BALANCE_MOS_FAULT,            EN_ALARM_LEVEL_3,  _DevAlarmBalanceFaultJudge,              0,   0},
    {EN_DEV_ALARM_BMU_COMM_FAULT,               EN_ALARM_LEVEL_1,  _DevAlarmBMUCommFaultJudge,              0,   0},
    {EN_DEV_ALARM_BMU_COMM_FAULT,               EN_ALARM_LEVEL_2,  _DevAlarmBMUCommFaultJudge,              0,   0},
    {EN_DEV_ALARM_BMU_COMM_FAULT,               EN_ALARM_LEVEL_3,  _DevAlarmBMUCommFaultJudge,              0,   0},
    {EN_DEV_ALARM_BMU_POWER_LOW,                EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_LOW,                EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_LOW,                EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_FAN_FAULT,                EN_ALARM_LEVEL_1,  _DevAlarmBMUFanFaultJudge,               0,   0},
    {EN_DEV_ALARM_BMU_FAN_FAULT,                EN_ALARM_LEVEL_2,  _DevAlarmBMUFanFaultJudge,               0,   0},
    {EN_DEV_ALARM_BMU_FAN_FAULT,                EN_ALARM_LEVEL_3,  _DevAlarmBMUFanFaultJudge,               0,   0},
    {EN_DEV_ALARM_BMU_FW_UPG_FAULT,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_FW_UPG_FAULT,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_FW_UPG_FAULT,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_PARA_SET_FAULT,           EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_PARA_SET_FAULT,           EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_PARA_SET_FAULT,           EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_HIGH_1,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_HIGH_1,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_HIGH_1,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_HIGH_2,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_HIGH_2,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_HIGH_2,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_HIGH_3,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_HIGH_3,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BMU_POWER_HIGH_3,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_CAN_CUR_SAMP_FAULT,           EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_CAN_CUR_SAMP_FAULT,           EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_CAN_CUR_SAMP_FAULT,           EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_CELL_VOLT_SAMP_FAULT,         EN_ALARM_LEVEL_1,  _DevAlarmCellVoltSampFaultJudge,         0,   0},
    {EN_DEV_ALARM_CELL_VOLT_SAMP_FAULT,         EN_ALARM_LEVEL_2,  _DevAlarmCellVoltSampFaultJudge,         0,   0},
    {EN_DEV_ALARM_CELL_VOLT_SAMP_FAULT,         EN_ALARM_LEVEL_3,  _DevAlarmCellVoltSampFaultJudge,         0,   0},
    {EN_DEV_ALARM_RELAY_P_ADH_FAULT,            EN_ALARM_LEVEL_1,  _DevAlarmRelayPAdhFaultJudge,            0,   0},
    {EN_DEV_ALARM_RELAY_P_ADH_FAULT,            EN_ALARM_LEVEL_2,  _DevAlarmRelayPAdhFaultJudge,            0,   0},
    {EN_DEV_ALARM_RELAY_P_ADH_FAULT,            EN_ALARM_LEVEL_3,  _DevAlarmRelayPAdhFaultJudge,            0,   0},
    {EN_DEV_ALARM_RELAY_N_ADH_FAULT,            EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_RELAY_N_ADH_FAULT,            EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_RELAY_N_ADH_FAULT,            EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_RELAY_PRE_ADH_FAULT,          EN_ALARM_LEVEL_1,  _DevAlarmRelayPreAdhFaultJudge,          0,   0},
    {EN_DEV_ALARM_RELAY_PRE_ADH_FAULT,          EN_ALARM_LEVEL_2,  _DevAlarmRelayPreAdhFaultJudge,          0,   0},
    {EN_DEV_ALARM_RELAY_PRE_ADH_FAULT,          EN_ALARM_LEVEL_3,  _DevAlarmRelayPreAdhFaultJudge,          0,   0},
    {EN_DEV_ALARM_RELAY_FAN_ADH_FAULT,          EN_ALARM_LEVEL_1,  _DevAlarmRelayFanAdhFaultJudge,          0,   0},
    {EN_DEV_ALARM_RELAY_FAN_ADH_FAULT,          EN_ALARM_LEVEL_2,  _DevAlarmRelayFanAdhFaultJudge,          0,   0},
    {EN_DEV_ALARM_RELAY_FAN_ADH_FAULT,          EN_ALARM_LEVEL_3,  _DevAlarmRelayFanAdhFaultJudge,          0,   0},
    {EN_DEV_ALARM_485_1_COMM_FAULT,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_485_1_COMM_FAULT,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_485_1_COMM_FAULT,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_485_2_COMM_FAULT,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_485_2_COMM_FAULT,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_485_2_COMM_FAULT,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_EEPROM_FAULT,                 EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_EEPROM_FAULT,                 EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_EEPROM_FAULT,                 EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_ADRESSING_FAIL,               EN_ALARM_LEVEL_1,  _DevAlarmBMUAdressingFailJudge,          0,   0},
    {EN_DEV_ALARM_ADRESSING_FAIL,               EN_ALARM_LEVEL_2,  _DevAlarmBMUAdressingFailJudge,          0,   0},
    {EN_DEV_ALARM_ADRESSING_FAIL,               EN_ALARM_LEVEL_3,  _DevAlarmBMUAdressingFailJudge,          0,   0},
    {EN_DEV_ALARM_BCU_POWER_LOW,                EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_LOW,                EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_LOW,                EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_FW_UPG_FAULT,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_FW_UPG_FAULT,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_FW_UPG_FAULT,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_HIGH_1,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_HIGH_1,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_HIGH_1,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_HIGH_2,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_HIGH_2,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_HIGH_2,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_HIGH_3,             EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_HIGH_3,             EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BCU_POWER_HIGH_3,             EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_DEV_ALARM_BREAKER_ADH_FAULT,            EN_ALARM_LEVEL_1,  _DevAlarmRelayBreakerAdhFaultJudge,      0,   0},
    {EN_DEV_ALARM_BREAKER_ADH_FAULT,            EN_ALARM_LEVEL_2,  _DevAlarmRelayBreakerAdhFaultJudge,      0,   0},
    {EN_DEV_ALARM_BREAKER_ADH_FAULT,            EN_ALARM_LEVEL_3,  _DevAlarmRelayBreakerAdhFaultJudge,      0,   0},
};

static AlarmInfo_t _gOtherAlarmInfo[] = 
{
    {EN_OTHER_ALARM_POLE_P_TEMP_HIGH,           EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_P_TEMP_HIGH,           EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_P_TEMP_HIGH,           EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_P_TEMP_LOW,            EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_P_TEMP_LOW,            EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_P_TEMP_LOW,            EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_N_TEMP_HIGH,           EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_N_TEMP_HIGH,           EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_N_TEMP_HIGH,           EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_N_TEMP_LOW,            EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_N_TEMP_LOW,            EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_N_TEMP_LOW,            EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
                                        
    {EN_OTHER_ALARM_COPPER_BAR_TEMP_HIGH,       EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_COPPER_BAR_TEMP_HIGH,       EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_COPPER_BAR_TEMP_HIGH,       EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_CHG_TEMP_HIGH,         EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_CHG_TEMP_HIGH,         EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_CHG_TEMP_HIGH,         EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_DSG_TEMP_HIGH,         EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_DSG_TEMP_HIGH,         EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_DSG_TEMP_HIGH,         EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_CHG_TEMP_LOW,          EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_CHG_TEMP_LOW,          EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_CHG_TEMP_LOW,          EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_DSG_TEMP_LOW,          EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_DSG_TEMP_LOW,          EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_DSG_TEMP_LOW,          EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_TEMP_CHA_RATE_HIGH,    EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_TEMP_CHA_RATE_HIGH,    EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_TEMP_CHA_RATE_HIGH,    EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_POLE_TEMP_HIGH,             EN_ALARM_LEVEL_1,  _OtherAlarmPoleTempHighJudge,            0,   0},
    {EN_OTHER_ALARM_POLE_TEMP_HIGH,             EN_ALARM_LEVEL_2,  _OtherAlarmPoleTempHighJudge,            0,   0},
    {EN_OTHER_ALARM_POLE_TEMP_HIGH,             EN_ALARM_LEVEL_3,  _OtherAlarmPoleTempHighJudge,            0,   0},
    {EN_OTHER_ALARM_POLE_TEMP_LOW,              EN_ALARM_LEVEL_1,  _OtherAlarmPoleTempLowJudge,             0,   0},
    {EN_OTHER_ALARM_POLE_TEMP_LOW,              EN_ALARM_LEVEL_2,  _OtherAlarmPoleTempLowJudge,             0,   0},
    {EN_OTHER_ALARM_POLE_TEMP_LOW,              EN_ALARM_LEVEL_3,  _OtherAlarmPoleTempLowJudge,             0,   0},
    {EN_OTHER_ALARM_CELL_VOLT_CHA_RATE_HIGH,    EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_VOLT_CHA_RATE_HIGH,    EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CELL_VOLT_CHA_RATE_HIGH,    EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_SOE_LOW,                    EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_SOE_LOW,                    EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_SOE_LOW,                    EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CHG_MAP_OVER,               EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CHG_MAP_OVER,               EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_CHG_MAP_OVER,               EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_DSG_MAP_OVER,               EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_DSG_MAP_OVER,               EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_DSG_MAP_OVER,               EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_FIRE_FIGHT_FAULT,           EN_ALARM_LEVEL_1,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_FIRE_FIGHT_FAULT,           EN_ALARM_LEVEL_2,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_FIRE_FIGHT_FAULT,           EN_ALARM_LEVEL_3,  NULL,                                    0,   0},
    {EN_OTHER_ALARM_PCS_LOST,                   EN_ALARM_LEVEL_1,  _OtherAlarmPCSLost,                      0,   0},
    {EN_OTHER_ALARM_PCS_LOST,                   EN_ALARM_LEVEL_2,  _OtherAlarmPCSLost,                      0,   0},
    {EN_OTHER_ALARM_PCS_LOST,                   EN_ALARM_LEVEL_3,  _OtherAlarmPCSLost,                      0,   0},
};

AlarmCode_t gAlarmCode;
AlarmThreshold_t gAlarmThreshold;

/********************measure alarm*********************/
/** 
* @brief    测量告警-单体电压高判断
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmCellVoltHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    uint16_t threshold;
    
    threshold = gAlarmThreshold.cell_volt_high_threshold[level];
    if(ret == false)
    {
        if(gPackInfo.cell_max_voltage > threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gPackInfo.cell_max_voltage <= threshold - 50)//带50mV的回差
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    测量告警-单体电压低判断
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmCellVoltLowJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    uint16_t threshold;
    
    threshold = gAlarmThreshold.cell_volt_low_threshold[level];
    if(ret == false)
    {
        if(gPackInfo.cell_min_voltage < threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gPackInfo.cell_min_voltage >= threshold + 50)//带50mV的回差
        {
            ret = false;
        }
    }
    return ret;
}

/** 
* @brief    测量告警-单体温度高判断
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmCellTempHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    int8_t threshold;
    
    threshold = gAlarmThreshold.cell_temp_high_threshold[level];
    if(ret == false)
    {
        if(gPackInfo.cell_max_temperature > threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gPackInfo.cell_max_temperature <= threshold - 2)//带2℃的回差
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    测量告警-单体温度低判断
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmCellTempLowJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    int8_t threshold;
    
    threshold = gAlarmThreshold.cell_temp_low_threshold[level];
    if(ret == false)
    {
        if(gPackInfo.cell_min_temperature < threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gPackInfo.cell_min_temperature >= threshold + 2)//带2℃的回差
        {
            ret = false;
        }
    }
    return ret;
}

/** 
* @brief    测量告警-箱体1过温--继电器
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmBox1TempHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    int8_t threshold;
    
    threshold = gAlarmThreshold.box1_temp_high_threshold[level];
    if(ret == false)
    {
        if(gTempInfo.temp_relay_p > threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gTempInfo.temp_relay_p <= threshold - 2)//带2℃的回差
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    测量告警-箱体2过温--环境-高压箱
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmBox2TempHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    int8_t threshold;
    
    threshold = gAlarmThreshold.box2_temp_high_threshold[level];
    if(ret == false)
    {
        if(gTempInfo.temp_hvbox > threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gTempInfo.temp_hvbox <= threshold - 2)//带2℃的回差
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    测量告警-箱体1低温--继电器
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmBox1TempLowJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    int8_t threshold;
    
    threshold = gAlarmThreshold.box1_temp_low_threshold[level];
    if(ret == false)
    {
        if(gTempInfo.temp_relay_p < threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gTempInfo.temp_relay_p >= threshold + 2)//带2℃的回差
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    测量告警-箱体2低温--环境-高压箱
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmBox2TempLowJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    int8_t threshold;
    
    threshold = gAlarmThreshold.box2_temp_low_threshold[level];
    if(ret == false)
    {
        if(gTempInfo.temp_hvbox < threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gTempInfo.temp_hvbox >= threshold + 2)//带2℃的回差
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    测量告警-B+过压
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmBPVoltHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    uint16_t threshold;
    
    threshold = gAlarmThreshold.bp_volt_high_threshold[level];
    if(ret == false)
    {
        if(gHighVoltInfo.v_bp / 1000 > threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gHighVoltInfo.v_bp / 1000 <= threshold - 5)//带5V的回差
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    测量告警-B+欠压
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmBPVoltLowJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    uint16_t threshold;
    
    threshold = gAlarmThreshold.bp_volt_low_threshold[level];
    if(ret == false)
    {
        if(gHighVoltInfo.v_bp / 1000 < threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gHighVoltInfo.v_bp / 1000 >= threshold + 5)//带5V的回差
        {
            ret = false;
        }
    }
    return ret;
}

/** 
* @brief    测量告警-P+过压
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmPPVoltHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    /*暂不处理*/
    
    return ret;
}


/** 
* @brief    测量告警-P+欠压
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmPPVoltLowJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    /*暂不处理*/
    
    return ret;
}


/** 
* @brief    测量告警-SOC低
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmSOCLowJudge(EN_ALARM_LEVEL level)
{
    /*无需回差*/
    bool ret = false;
    uint8_t threshold;
    
    threshold = gAlarmThreshold.soc_low_threshold[level];
    if(gPackInfo.sox_info.soc_real <= threshold)
    {
        ret = true;
    }
    else
    {
        ret = false;
    }
    return ret;
}

/** 
* @brief    测量告警-SOH低
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmSOHLowJudge(EN_ALARM_LEVEL level)
{
    /*无需回差*/
    bool ret = false;
    uint8_t threshold;
    
    threshold = gAlarmThreshold.soh_low_threshold[level];
    if(gPackInfo.sox_info.soh < threshold)
    {
        ret = true;
    }
    else
    {
        ret = false;
    }
    return ret;
}

/** 
* @brief    测量告警-充电电流大
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmChgCurHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    uint16_t threshold;
    
    threshold = gAlarmThreshold.chg_current_high_threshold[level];
    if(ret == false)
    {
        if(gPackInfo.current < 0)
        {
            if(-gPackInfo.current / 1000 > threshold)
            {
                ret = true;
            }
        }
        else
        {
            ret = false;
        }
    }
    else
    {
        if(gPackInfo.current < 0)
        {
            if(-gPackInfo.current / 1000 <= threshold - 5)//带5A的回差
            {
                ret = false;
            }
        }
        else
        {
            ret = false;
        }
    }
    return ret;
}

/** 
* @brief    测量告警-放电电流大
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmDsgCurHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    uint16_t threshold;
    
    threshold = gAlarmThreshold.dsg_current_high_threshold[level];
    if(ret == false)
    {
        if(gPackInfo.current > 0)
        {
            if(gPackInfo.current / 1000 > threshold)
            {
                ret = true;
            }
        }
        else
        {
            ret = false;
        }
    }
    else
    {
        if(gPackInfo.current > 0)
        {
            if(gPackInfo.current / 1000 <= threshold - 5)//带5A的回差
            {
                ret = false;
            }
        }
        else
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    测量告警-绝缘阻抗低
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmInsImpLowJudge(EN_ALARM_LEVEL level)
{
    /*无需回差*/
    bool ret = false;
    uint32_t threshold;
    
    threshold = gAlarmThreshold.ins_res_low_threshold[level];
    if(gInsResInfo.detect_stat != EN_INS_RES_DETECT_STAT_COMPLETE)//绝缘检测要完成，值才有效
    {
        return ret;
    }
    
    if(gInsResInfo.bp_res < threshold || gInsResInfo.bn_res < threshold)
    {
        ret = true;
    }
    else
    {
        ret = false;
    }
    return ret;
}

/** 
* @brief    测量告警-电芯压差大
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmCellVoltDiffHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    uint16_t threshold;
    
    threshold = gAlarmThreshold.cell_volt_diff_high_threshold[level];
    if(ret == false)
    {
        if((gPackInfo.cell_max_voltage - gPackInfo.cell_min_voltage) > threshold)
        {
            ret = true;
        }
    }
    else
    {
        if((gPackInfo.cell_max_voltage - gPackInfo.cell_min_voltage) <= threshold - 50)//带50mV的回差
        {
            ret = false;
        }
    }
    return ret;
}



/** 
* @brief    测量告警-电芯温差大
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _MeasureAlarmCellTempDiffHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    uint8_t threshold;
    
    threshold = gAlarmThreshold.cell_volt_diff_high_threshold[level];
    if(ret == false)
    {
        if((gPackInfo.cell_max_temperature - gPackInfo.cell_min_temperature) > threshold)
        {
            ret = true;
        }
    }
    else
    {
        if((gPackInfo.cell_max_temperature - gPackInfo.cell_min_temperature) <= threshold - 2)//带2℃的回差
        {
            ret = false;
        }
    }
    return ret;
}



/********************dev alarm*********************/
/** 
* @brief    设备告警-电芯温度传感器短路
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmCellNTCShortJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    //不分级
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        if((gBMSDataInfo.bmu_statis_other_info[i].fault_info & (1 << 3)) != 0)
        {
            return true;
        }
    }
    return ret;
}


/** 
* @brief    设备告警-电芯温度传感器断开
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmCellNTCBreakJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    //不分级
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        if((gBMSDataInfo.bmu_statis_other_info[i].fault_info & (1 << 2)) != 0)
        {
            return true;
        }
    }
    return ret;
}


/** 
* @brief    设备告警-AFE故障
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmAFEFaultJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    //不分级
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        if((gBMSDataInfo.bmu_statis_other_info[i].fault_info & (1 << 8)) != 0)
        {
            return true;
        }
    }
    return ret;
}


/** 
* @brief    设备告警-均衡故障
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmBalanceFaultJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    //不分级
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        if((gBMSDataInfo.bmu_statis_other_info[i].fault_info & (0xf << 4)) != 0)
        {
            return true;
        }
    }
    return ret;
}


/** 
* @brief    设备告警-和BMU通信故障
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmBMUCommFaultJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    //不分级
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        if((uint32_t)(tim_get_systime() - gBMSDataInfo.bmu_comm_last_time[i]) > 10 * 1000)//通信超时10S
        {
            return true;
        }
    }
    return ret;
}


/** 
* @brief    设备告警-BMU风扇故障
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmBMUFanFaultJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    //不分级
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        if((gBMSDataInfo.bmu_statis_other_info[i].fault_info & (1 << 9)) != 0)
        {
            return true;
        }
    }
    return ret;
}


/** 
* @brief    设备告警-电芯电压采集故障
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmCellVoltSampFaultJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    //不分级
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        if((gBMSDataInfo.bmu_statis_other_info[i].fault_info & (0x3)) != 0)
        {
            return true;
        }
    }
    return ret;
}


/** 
* @brief    设备告警-主正继电器粘连故障
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmRelayPAdhFaultJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    
    if(gBMSDataInfo.relay_p_stat == false)//继电器逻辑上是断开的
    {
        if(RelayPFeedback() == true)//继电器反馈又是闭合状态
        {
            ret = true;
        }
    }
    return ret;
}

/** 
* @brief    设备告警-预充继电器粘连故障
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmRelayPreAdhFaultJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    
    if(gBMSDataInfo.relay_pre_stat == false)//继电器逻辑上是断开的
    {
        if(RelayPreFeedback() == true)//继电器反馈又是闭合状态
        {
            ret = true;
        }
    }
    return ret;
}


/** 
* @brief    设备告警-风扇继电器粘连故障
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmRelayFanAdhFaultJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    
    //风扇继电器没有反馈信号，暂不处理
    return ret;
}


/** 
* @brief    设备告警-从控编址失败
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmBMUAdressingFailJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;

    if(gBMSDataInfo.bmu_adressing_res == EN_OP_RES_FAIL)
    {
        ret = true;
    }
    return ret;
}


/** 
* @brief    设备告警-断路器继电器粘连故障
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _DevAlarmRelayBreakerAdhFaultJudge(EN_ALARM_LEVEL level)
{
    bool ret = false;
    
    if(gBMSDataInfo.breaker_stat == true)//断路器逻辑上是脱扣的
    {
        if(BreakerFeedback() == false)//断路器反馈又是未脱扣状态
        {
            ret = true;
        }
    }
    return ret;
}

/********************other alarm*********************/
/** 
* @brief    其他告警-极柱过温
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _OtherAlarmPoleTempHighJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    int8_t threshold;
    
    threshold = gAlarmThreshold.pole_temp_high_threshold[level];
    if(gBMSDataInfo.bmu_calc_info.valid_flag == false)
    {
        return ret;
    }
    if(ret == false)
    {
        if(gBMSDataInfo.bmu_calc_info.pole_max_temp > threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gBMSDataInfo.bmu_calc_info.pole_max_temp <= threshold - 2)//带2℃的回差
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    其他告警-极柱欠温
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _OtherAlarmPoleTempLowJudge(EN_ALARM_LEVEL level)
{
    /*对于有回差的情况，ret需要定义成数组(不同等级分开)，以及static类型，作为记录上一次的状态*/
    static bool ret = false;
    int8_t threshold;
    
    threshold = gAlarmThreshold.pole_temp_low_threshold[level];
    if(gBMSDataInfo.bmu_calc_info.valid_flag == false)
    {
        return ret;
    }
    if(ret == false)
    {
        if(gBMSDataInfo.bmu_calc_info.pole_min_temp < threshold)
        {
            ret = true;
        }
    }
    else
    {
        if(gBMSDataInfo.bmu_calc_info.pole_min_temp >= threshold + 2)//带2℃的回差
        {
            ret = false;
        }
    }
    return ret;
}


/** 
* @brief    其他告警-PCS失联
* @param    level：故障所属等级
* @return   故障是否发生
* @note:
*/
static bool _OtherAlarmPCSLost(EN_ALARM_LEVEL level)
{
    bool ret = false;
    //不分级
    if((uint32_t)(tim_get_systime() - gBMSDataInfo.pcs_comm_last_time) > 10 * 1000)//通信超时10S
    {
        return true;
    }
    return ret;
}



/**/

/** 
* @brief    ALARM状态解析
* @param    p_alarm_info:故障信息指针，p_alarm_code：故障码指针，alarm_num：当前处理的故障种类数量
* @return   none
* @note: 
*/
static void _AlarmStatAnalysis(AlarmInfo_t *p_alarm_info, uint32_t *p_alarm_code, uint8_t alarm_num)
{
    for(uint8_t i = 0; i < alarm_num; i++)
    {
        EN_ALARM_LEVEL level = p_alarm_info[i].level;
        if(p_alarm_info[i].p_alarm_judge == NULL)
        {
            continue;
        }
        bool res = p_alarm_info[i].p_alarm_judge(level);
        if(res == true)//检测到故障发生
        {
            if((p_alarm_code[level] & (1 << p_alarm_info[i].alarm_type)) == 0)//故障尚未置位
            {
                p_alarm_info[i].alarm_occur_dur_time = 0;
                if((uint32_t)(tim_get_systime() - p_alarm_info[i].alarm_occur_delay_count) >= p_alarm_info[i].alarm_occur_delay_time)
                {
                    p_alarm_code[level] |= (1 << p_alarm_info[i].alarm_type);
                    p_alarm_info[i].alarm_occur_time_calibration = GetRtcTimeStamp();
                }
            }
            else//开始统计故障持续时长
            {
                p_alarm_info[i].alarm_occur_dur_time = GetRtcTimeStamp() - p_alarm_info[i].alarm_occur_time_calibration;
            }
            p_alarm_info[i].alarm_dispel_delay_count = tim_get_systime();
        }
        else
        {
            if((p_alarm_code[level] & (1 << p_alarm_info[i].alarm_type)) != 0)//故障处于置位态
            {
                if((uint32_t)(tim_get_systime() - p_alarm_info[i].alarm_dispel_delay_count) >= p_alarm_info[i].alarm_dispel_delay_time)
                {
                    p_alarm_code[level] &= ~(1 << p_alarm_info[i].alarm_type);
                }
            }
            p_alarm_info[i].alarm_occur_delay_count = tim_get_systime();
        }
    }
}




/** 
* @brief    ALARM处理大轮询
* @param    none
* @return   none
* @note:    和从控相关的告警需要等待从控编址完成，且已有全量数据上报给主控后再执行，如分配失败，则和从控相关的告警只有通信故障
*/
void AlarmProcess()
{
    /*测量告警*/
    _AlarmStatAnalysis(_gMeasureAlarmInfo, gAlarmCode.measure_alarm_code, sizeof(_gMeasureAlarmInfo)/sizeof(_gMeasureAlarmInfo[0]));
    /*设备告警*/
    _AlarmStatAnalysis(_gDevAlarmInfo, gAlarmCode.dev_alarm_code, sizeof(_gDevAlarmInfo)/sizeof(_gDevAlarmInfo[0]));
    /*其他告警*/
    _AlarmStatAnalysis(_gOtherAlarmInfo, gAlarmCode.other_alarm_code, sizeof(_gOtherAlarmInfo)/sizeof(_gOtherAlarmInfo[0]));
}


