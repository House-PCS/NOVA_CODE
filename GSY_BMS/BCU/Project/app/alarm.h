/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        alarm.h
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
#ifndef __ALARM_H_
#define __ALARM_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "dev_sd2506.h"

/*测量告警，1~3级通用*/
typedef enum
{
    EN_MEASURE_ALARM_CELL_VOLT_HIGH,//单体电压高
    EN_MEASURE_ALARM_CELL_VOLT_LOW,//单体电压低
    EN_MEASURE_ALARM_CELL_TEMP_HIGH,//单体温度高
    EN_MEASURE_ALARM_CELL_TEMP_LOW,//单体温度低
    EN_MEASURE_ALARM_MOS_TEMP_HIGH,//MOS温度高
    EN_MEASURE_ALARM_MOS_TEMP_LOW,//MOS温度低
    EN_MEASURE_ALARM_RESERVED1,
    EN_MEASURE_ALARM_RESERVED2,
    EN_MEASURE_ALARM_BOX1_TEMP_HIGH,//箱体1过温
    EN_MEASURE_ALARM_BOX2_TEMP_HIGH,//箱体2过温
    EN_MEASURE_ALARM_BOX3_TEMP_HIGH,//箱体3过温
    EN_MEASURE_ALARM_BOX4_TEMP_HIGH,//箱体4过温
    EN_MEASURE_ALARM_BOX1_TEMP_LOW,//箱体1低温
    EN_MEASURE_ALARM_BOX2_TEMP_LOW,//箱体2低温
    EN_MEASURE_ALARM_BOX3_TEMP_LOW,//箱体3低温
    EN_MEASURE_ALARM_BOX4_TEMP_LOW,//箱体4低温
    EN_MEASURE_ALARM_BP_OVER,//B+过压
    EN_MEASURE_ALARM_BP_UNDER,//B+欠压
    EN_MEASURE_ALARM_PP_OVER,//P+过压
    EN_MEASURE_ALARM_PP_UNDER,//P+欠压
    EN_MEASURE_ALARM_TOTAL_VOLT_DIFF_HIGH,//总压压差过大
    EN_MEASURE_ALARM_SOC_LOW,//SOC低
    EN_MEASURE_ALARM_SOH_LOW,//SOH低
    EN_MEASURE_ALARM_CHG_CURRENT_HIGH,//充电电流大
    EN_MEASURE_ALARM_DSG_CURRENT_HIGH,//放电电流大
    EN_MEASURE_ALARM_INS_RES_LOW,//绝缘低
    EN_MEASURE_ALARM_CELL_VOLT_DIFF_HIGH,//电芯压差大
    EN_MEASURE_ALARM_CELL_TEMP_DIFF_HIGH,//电芯温差大
    EN_MEASURE_ALARM_RESERVED3,
    EN_MEASURE_ALARM_RESERVED4,
    EN_MEASURE_ALARM_RESERVED5,
    EN_MEASURE_ALARM_RESERVED6,
}EN_MEASURE_ALARM_TYPE;


/*设备告警，1~3级通用*/
typedef enum
{
    EN_DEV_ALARM_CELL_NTC_SHORT_FAULT,//电芯温度传感器短路
    EN_DEV_ALARM_CELL_NTC_BREAK_FAULT,//电芯温度传感器断开
    EN_DEV_ALARM_MOS_NTC_SHORT_FAULT,//MOS温度传感器短路
    EN_DEV_ALARM_MOS_NTC_BREAK_FAULT,//MOS温度传感器断开
    EN_DEV_ALARM_AFE_FAULT,//AFE故障
    EN_DEV_ALARM_BALANCE_MOS_FAULT,//均衡MOS故障
    EN_DEV_ALARM_BMU_COMM_FAULT,//和BMU通信故障
    EN_DEV_ALARM_BMU_POWER_LOW,//BMU供电欠压
    EN_DEV_ALARM_BMU_FAN_FAULT,//BMU风扇故障
    EN_DEV_ALARM_BMU_FW_UPG_FAULT,//BMU固件升级异常
    EN_DEV_ALARM_BMU_PARA_SET_FAULT,//BMU参数设置异常
    EN_DEV_ALARM_BMU_POWER_HIGH_1,//BMU供电过压异常轻
    EN_DEV_ALARM_BMU_POWER_HIGH_2,//BMU供电过压异常中
    EN_DEV_ALARM_BMU_POWER_HIGH_3,//BMU供电过压异常重
    EN_DEV_ALARM_CAN_CUR_SAMP_FAULT,//CAN电流采集设备异常
    EN_DEV_ALARM_CELL_VOLT_SAMP_FAULT,//电芯电压采集故障
    EN_DEV_ALARM_RELAY_P_ADH_FAULT,//主正继电器粘连故障
    EN_DEV_ALARM_RELAY_N_ADH_FAULT,//主负继电器粘连故障
    EN_DEV_ALARM_RELAY_PRE_ADH_FAULT,//预充继电器粘连故障
    EN_DEV_ALARM_RELAY_FAN_ADH_FAULT,//风扇继电器粘连故障
    EN_DEV_ALARM_485_1_COMM_FAULT,//485-1通信异常
    EN_DEV_ALARM_485_2_COMM_FAULT,//485-2通信异常
    EN_DEV_ALARM_EEPROM_FAULT,//EEPROM故障
    EN_DEV_ALARM_ADRESSING_FAIL,//编址失败
    EN_DEV_ALARM_BCU_POWER_LOW,//主控供电欠压
    EN_DEV_ALARM_BCU_FW_UPG_FAULT,//BCU固件升级异常
    EN_DEV_ALARM_BCU_POWER_HIGH_1,//BCU供电过压异常轻
    EN_DEV_ALARM_BCU_POWER_HIGH_2,//BCU供电过压异常中
    EN_DEV_ALARM_BCU_POWER_HIGH_3,//BCU供电过压异常重
    EN_DEV_ALARM_BREAKER_ADH_FAULT,//断路器继电器粘连故障
    EN_DEV_ALARM_RESERVED1,
    EN_DEV_ALARM_RESERVED2,
}EN_DEV_ALARM_TYPE;


/*其他告警，1~3级通用*/
typedef enum
{
    EN_OTHER_ALARM_POLE_P_TEMP_HIGH,//正极柱过温
    EN_OTHER_ALARM_POLE_P_TEMP_LOW,//正极柱欠温
    EN_OTHER_ALARM_POLE_N_TEMP_HIGH,//负极柱过温
    EN_OTHER_ALARM_POLE_N_TEMP_LOW,//负极柱欠温
    EN_OTHER_ALARM_RESERVED1,
    EN_OTHER_ALARM_RESERVED2,
    EN_OTHER_ALARM_RESERVED3,
    EN_OTHER_ALARM_RESERVED4,
    EN_OTHER_ALARM_RESERVED5,
    EN_OTHER_ALARM_RESERVED6,
    EN_OTHER_ALARM_RESERVED7,
    EN_OTHER_ALARM_RESERVED8,
    EN_OTHER_ALARM_RESERVED9,
    EN_OTHER_ALARM_RESERVED10,
    EN_OTHER_ALARM_RESERVED11,
    EN_OTHER_ALARM_COPPER_BAR_TEMP_HIGH,//铜排过温
    EN_OTHER_ALARM_CELL_CHG_TEMP_HIGH,//单体充电过温
    EN_OTHER_ALARM_CELL_DSG_TEMP_HIGH,//单体放电过温
    EN_OTHER_ALARM_CELL_CHG_TEMP_LOW,//单体充电欠温
    EN_OTHER_ALARM_CELL_DSG_TEMP_LOW,//单体放电欠温
    EN_OTHER_ALARM_CELL_TEMP_CHA_RATE_HIGH,//单体温度变化率大
    EN_OTHER_ALARM_POLE_TEMP_HIGH,//极柱过温
    EN_OTHER_ALARM_POLE_TEMP_LOW,//极柱欠温
    EN_OTHER_ALARM_CELL_VOLT_CHA_RATE_HIGH,//单体电压变化率大
    EN_OTHER_ALARM_SOE_LOW,//SOE低
    EN_OTHER_ALARM_CHG_MAP_OVER,//充电MAP过流
    EN_OTHER_ALARM_DSG_MAP_OVER,//放电MAP过流
    EN_OTHER_ALARM_FIRE_FIGHT_FAULT,//消防故障
    EN_OTHER_ALARM_PCS_LOST,//PCS失联
    EN_OTHER_ALARM_RESERVED12,
    EN_OTHER_ALARM_RESERVED13,
    EN_OTHER_ALARM_RESERVED14,
}EN_OTHER_ALARM_TYPE;

typedef enum
{
    EN_ALARM_LEVEL_1,
    EN_ALARM_LEVEL_2,
    EN_ALARM_LEVEL_3,
    EN_ALARM_LEVEL_NUM
}EN_ALARM_LEVEL;





typedef struct _alarm_info_stru_t
{
    uint8_t             alarm_type;
    EN_ALARM_LEVEL      level;
    bool                (*p_alarm_judge)(EN_ALARM_LEVEL );//故障是否发生判断函数，因为同类型故障调用同一个接口，所以需要形参区分故障等级
    uint32_t            alarm_occur_delay_time;//故障发生延迟时间，单位ms
    uint32_t            alarm_dispel_delay_time;//故障消除延迟时间，单位ms
    uint32_t            alarm_occur_delay_count;//故障发生延迟时间计数
    uint32_t            alarm_dispel_delay_count;//故障消除延迟时间计数
    uint32_t            alarm_occur_dur_time;//故障持续时长，单位S
    uint32_t            alarm_occur_time_calibration;//故障发生时间标定
}AlarmInfo_t;

//typedef struct _alarm_list_info_stru_t
//{
//    AlarmInfo_t         *p_measure_alarm_info;
//    AlarmInfo_t         *p_dev_alarm_info;
//    AlarmInfo_t         *p_other_alarm_info;
//    uint32_t            measure_alarm_code[3];
//    uint32_t            dev_alarm_code[3];
//    uint32_t            other_alarm_code[3];
//}AlarmListInfo_t;

typedef struct _alarm_list_info_stru_t
{
    uint32_t            measure_alarm_code[EN_ALARM_LEVEL_NUM];
    uint32_t            dev_alarm_code[EN_ALARM_LEVEL_NUM];
    uint32_t            other_alarm_code[EN_ALARM_LEVEL_NUM];
}AlarmCode_t;


/*触发各类故障的阈值定义如下*/
#pragma pack(1) 
typedef struct _alarm_threshold_stru_t
{
    //测量告警
    uint16_t            cell_volt_high_threshold[EN_ALARM_LEVEL_NUM];//单体过压，单位mV
    uint16_t            cell_volt_low_threshold[EN_ALARM_LEVEL_NUM];//单体欠压，单位mV
    int8_t              cell_temp_high_threshold[EN_ALARM_LEVEL_NUM];//单体过温，单位℃
    int8_t              cell_temp_low_threshold[EN_ALARM_LEVEL_NUM];//单体欠温，单位℃
    int8_t              mos_temp_high_threshold[EN_ALARM_LEVEL_NUM];//mos过温(均衡温度)，单位℃
    int8_t              mos_temp_low_threshold[EN_ALARM_LEVEL_NUM];//mos欠温(均衡温度)，单位℃
    int8_t              box1_temp_high_threshold[EN_ALARM_LEVEL_NUM];//箱体1过温，单位℃
    int8_t              box1_temp_low_threshold[EN_ALARM_LEVEL_NUM];//箱体1欠温，单位℃
    int8_t              box2_temp_high_threshold[EN_ALARM_LEVEL_NUM];//箱体2过温，单位℃
    int8_t              box2_temp_low_threshold[EN_ALARM_LEVEL_NUM];//箱体2欠温，单位℃
    int8_t              box3_temp_high_threshold[EN_ALARM_LEVEL_NUM];//箱体3过温，单位℃
    int8_t              box3_temp_low_threshold[EN_ALARM_LEVEL_NUM];//箱体3欠温，单位℃
    int8_t              box4_temp_high_threshold[EN_ALARM_LEVEL_NUM];//箱体4过温，单位℃
    int8_t              box4_temp_low_threshold[EN_ALARM_LEVEL_NUM];//箱体4欠温，单位℃
    uint16_t            bp_volt_high_threshold[EN_ALARM_LEVEL_NUM];//B+过压，单位V
    uint16_t            bp_volt_low_threshold[EN_ALARM_LEVEL_NUM];//B+欠压，单位V
    uint16_t            pp_volt_high_threshold[EN_ALARM_LEVEL_NUM];//P+过压，单位V
    uint16_t            pp_volt_low_threshold[EN_ALARM_LEVEL_NUM];//P+欠压，单位V
    uint16_t            total_volt_diff_high_threshold[EN_ALARM_LEVEL_NUM];//总压压差过大，单位V，单簇不涉及
    uint8_t             soc_low_threshold[EN_ALARM_LEVEL_NUM];//SOC低(0~100)
    uint8_t             soh_low_threshold[EN_ALARM_LEVEL_NUM];//SOH低(0~100)
    uint16_t            chg_current_high_threshold[EN_ALARM_LEVEL_NUM];//充电电流大，单位A
    uint16_t            dsg_current_high_threshold[EN_ALARM_LEVEL_NUM];//放电电流大，单位A
    uint32_t            ins_res_low_threshold[EN_ALARM_LEVEL_NUM];//绝缘阻抗低，单位KΩ
    uint16_t            cell_volt_diff_high_threshold[EN_ALARM_LEVEL_NUM];//电芯压差大，单位mV
    uint8_t             cell_temp_diff_high_threshold[EN_ALARM_LEVEL_NUM];//电芯温差大，单位℃
    
    //设备告警
    //不涉及阈值
    
    //其他告警
    int8_t              pole_p_temp_high_threshold[EN_ALARM_LEVEL_NUM];//正极柱过温，单位℃，暂不涉及
    int8_t              pole_p_temp_low_threshold[EN_ALARM_LEVEL_NUM];//正极柱欠温，单位℃，暂不涉及
    int8_t              pole_n_temp_high_threshold[EN_ALARM_LEVEL_NUM];//负极柱过温，单位℃，暂不涉及
    int8_t              pole_n_temp_low_threshold[EN_ALARM_LEVEL_NUM];//负极柱欠温，单位℃，暂不涉及
    int8_t              copper_temp_high_threshold[EN_ALARM_LEVEL_NUM];//铜排过温，单位℃，暂不涉及
    int8_t              copper_temp_low_threshold[EN_ALARM_LEVEL_NUM];//铜排欠温，单位℃，暂不涉及
    int8_t              cell_chg_temp_high_threshold[EN_ALARM_LEVEL_NUM];//单体充电过温，单位℃，暂不涉及
    int8_t              cell_dsg_temp_high_threshold[EN_ALARM_LEVEL_NUM];//单体放电过温，单位℃，暂不涉及
    int8_t              cell_chg_temp_low_threshold[EN_ALARM_LEVEL_NUM];//单体充电欠温，单位℃，暂不涉及
    int8_t              cell_dsg_temp_low_threshold[EN_ALARM_LEVEL_NUM];//单体放电欠温，单位℃，暂不涉及
    int8_t              cell_temp_change_rate_high_threshold[EN_ALARM_LEVEL_NUM];//单体温度变化率大，暂不涉及
    int8_t              pole_temp_high_threshold[EN_ALARM_LEVEL_NUM];//极柱过温，单位℃
    int8_t              pole_temp_low_threshold[EN_ALARM_LEVEL_NUM];//极柱欠温，单位℃
    int8_t              cell_volt_change_rate_high_threshold[EN_ALARM_LEVEL_NUM];//单体电压变化率大，暂不涉及
    uint8_t             soe_low_threshold[EN_ALARM_LEVEL_NUM];//SOE低,暂不涉及
    uint16_t            chg_current_map_high_threshold[EN_ALARM_LEVEL_NUM];//充电MAP过流，单位A，暂不涉及
    uint16_t            dsg_current_map_high_threshold[EN_ALARM_LEVEL_NUM];//放电MAP过流，单位A，暂不涉及
    //剩余其他告警没有阈值
}AlarmThreshold_t;
#pragma pack()




extern AlarmCode_t gAlarmCode;
extern AlarmThreshold_t gAlarmThreshold;


void AlarmProcess(void);



#endif










