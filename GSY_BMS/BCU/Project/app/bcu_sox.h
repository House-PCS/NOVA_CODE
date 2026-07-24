/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bcu_sox.h
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
  * <tr><td>2024-8-20    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#ifndef __BCU_SOX_H_
#define __BCU_SOX_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "dev_sd2506.h"

typedef enum
{
    EN_PACK_CURRENT_IDLE_MODE = 0,
    EN_PACK_CURRENT_DISCHARGE_MODE,
    EN_PACK_CURRENT_CHARGE_MODE
} EN_PACK_CURRENT_MODE;


#pragma pack(1)
/*上电从EEPROM中加载，定时转存(如能收到关机指令，则在关机前转存)*/
typedef struct _sox_stru_t
{
    float           soc_real;/*真实SOC*/
//    float           soc_actl;/*计算续航SOC*/
    float           soc_disp;/*表显SOC*/
    float           soh;/*电池健康度*/
    uint32_t        chg_cycle;//充电循环次数
    uint32_t        dsg_cycle;//放电循环次数
    uint32_t        ocv_time;//OCV计时起始时间
}SOXInfo_t; 
#pragma pack()

#pragma pack(1)
/*充放电统计信息，上电从EEPROM中加载，定时转存(如能收到关机指令，则在关机前转存)*/
typedef struct _elect_statist_stru_t
{
    uint32_t                accum_time_stamp;//保存数据对应的时间戳(主要针对当天、当月、当年的数据，时间发生跨度变化后数据需要清零操作)
    double                  chg_cur_day_accum_e;//当天累计充电量，单位:Wh
    double                  dsg_cur_day_accum_e;//当天累计放电量，单位:Wh
    double                  chg_cur_month_accum_e;//当月累计充电量，单位:Wh
    double                  dsg_cur_month_accum_e;//当月累计放电量，单位:Wh
    double                  chg_cur_year_accum_e;//当年累计充电量，单位:Wh
    double                  dsg_cur_year_accum_e;//当年累计放电量，单位:Wh
    double                  chg_total_accum_e;//总累计充电量，单位:Wh
    double                  dsg_total_accum_e;//总累计放电量，单位:Wh
    
    double                  chg_total_accum_q;//总累计充电容量，单位:mAh
    double                  dsg_total_accum_q;//总累计放电容量，单位:mAh
}ElectStatist_t;
#pragma pack()


#pragma pack(1)
/*出厂参数信息，上电从FLASH中加载，可通过生产工具修改，如未写入过，则使用默认值*/
typedef struct _pack_fact_para_stru_t
{
    float                   soc_max;//SOC最大值
    float                   soc_min;//SOC最小值
    uint32_t                q_max;//电池出厂满电容量，单位mAh
    uint16_t                cell_max_chg_voltage;//充电电压上限，单位mV,默认3.62V
    uint16_t                cell_min_dsg_voltage;//放电电压下限，单位mV,默认2.8V
    uint16_t                chg_reduce_cur_voltage;//充电降流的电压阈值,默认3.5V
    uint16_t                dsg_reduce_cur_voltage;//放电降流的电压阈值,默认3V
    uint8_t                 chg_max_rate;//最大充电倍率，单位0.1C，工商业默认值通常为0.5C
    uint8_t                 dsg_max_rate;//最大放电倍率，单位0.1C，工商业默认值通常为0.5C
    uint8_t                 chg_reduce_cur_rate;//充电限流后的最大倍率，单位0.1C
    uint8_t                 dsg_reduce_cur_rate;//放电限流后的最大倍率，单位0.1C
}PackFactPara_t;
#pragma pack()




#pragma pack(1)
/*电池簇实时信息*/
typedef struct _pack_info_stru_t
{
    /*定时需要保存的数据*/
    SOXInfo_t               sox_info;    
    /*以下参数暂定使用默认值，也可由上位机进行配置*/
    PackFactPara_t          pack_fact_para;
    
    /*每种类型(当天、当月、当年)数据单独存储，并且存储需要带上对应的时间，每次上电后读取，需要判断时间，如时间一致则累加，如时间不一致则清零重新统计*/
    ElectStatist_t          elect_statist;
    
    /*以下参数为实时采集信息*/
    /*需要从data_manage模块获取相关信息*/
    bool                    rt_info_valid;//实时信息是否有效
    int32_t                 current;//实时电流，单位mA，充电为负，放电为正(和新艾华塑协议相反)
    uint16_t                cell_max_voltage;//最大的电芯电压，用于充电模式下的计算，单位mV
    uint16_t                cell_min_voltage;//最小的电芯电压，用于放电模式下的计算，单位mV
    uint32_t                cell_total_voltage;//电芯总压，单位mV
    int8_t                  cell_max_temperature;//电芯最高温度
    int8_t                  cell_min_temperature;//电芯最低温度
    int8_t                  cell_ave_temperature;//电芯平均温度
    
//    /*故障及限流处理逻辑判断并填入*/
//    uint8_t                 fault_cur_limit_flag;//因故障导致的限流标志,0代表无故障，数字越大越严重，影响chg_max_current,和dsg_max_current
    
    /*模块内计算，即输出信息*/
    uint32_t                q_real;//电池实际满电容量，工商业暂不考虑温度对容量影响因素，所以q_real = q_max * soh
    uint32_t                chg_max_current;//最大充电电流，单位mA，正常情况下为Q_real/chg_max_rate
    uint32_t                dsg_max_current;//最大放电电流，单位mA，正常情况下为Q_real/dsg_max_rate
    uint32_t                chg_cur_current_limit;//最大充电电流限制，实时根据电压情况调节
    uint32_t                dsg_cur_current_limit;//最大放电电流限制，实时根据电压情况调节
    bool                    full_flag;//满充标志
    bool                    empty_flag;//放空标志
    EN_PACK_CURRENT_MODE    mode;//当前充放电模式
    uint16_t                soc_disp_covert;//强制将加入放电深度机制的soc_disp转换为0~100      
    
    /*安时积分周期计算标志，定时器更新*/
    uint16_t                calc_period;//安时积分计算周期，单位ms，通常为10ms的倍数
    bool                    calc_flag;
}PackInfo_t;
#pragma pack()

extern PackInfo_t gPackInfo;


void BCUSoxInit(void);
void OCVCalibration(uint8_t type);
void FullChargeCalibration(void);
void EmptyDischargeCalibration(void);

void SOXProcess(void);

#endif



