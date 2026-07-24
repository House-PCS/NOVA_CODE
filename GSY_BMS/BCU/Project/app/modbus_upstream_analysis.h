/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_upstream_analysis.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-18
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-18     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __MODBUS_UPSTREAM_ANALYSIS_H_
#define __MODBUS_UPSTREAM_ANALYSIS_H_

#include "bmu_ctl.h"
#include "bms_data_manage.h"
#include "bcu_sox.h"
#include "high_volt_detect.h"
#include "ins_res_detect.h"
#include "temp_detect.h"
#include "alarm.h"
#include "peri_ctl_stat_api.h"
#include "sys_run.h"
#include "systick.h"
#include "crc.h"
#include "dev_sd2506.h"
#include "current_detect.h"
#include "ins_res_detect.h"
#include "high_volt_detect.h"
#include "temp_detect.h"
#include "data_storage_manage.h"

typedef enum
{
    EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS,
    EN_MODBUS_UPSTREAM_OP_STAT_ADDR_ERROR,
    EN_MODBUS_UPSTREAM_OP_STAT_LEN_ERROR,
    EN_MODBUS_UPSTREAM_OP_STAT_BUSY,
    EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA,//只针对写操作
}EN_MODBUS_UPSTREAM_OP_STAT;


#pragma pack(2)
typedef struct _mod_bmu_addressing_info_stru_t
{
    uint16_t            addressing_res;//0:尚未编址，1:编址中，2:编址成功，3:编址失败
    uint16_t            bmu_num;
    uint16_t            per_bmu_cell_num;
    uint16_t            per_bmu_temp_num;
    uint16_t            per_bmu_pole_temp_num;
    uint16_t            per_bmu_other_temp_num;
}ModBMUAddressingInfo_t;
#pragma pack()

#pragma pack(2)
typedef struct _mod_bmu_io_output_stat_stru_t
{
    uint16_t            io_output_stat;
    uint16_t            pwm1_output_duty;//0~100
    uint16_t            pwm2_output_duty;//0~100
    uint16_t            reserved[5];
}ModBMUIOOutputStat_t;
#pragma pack()

#pragma pack(2)
typedef struct _mod_bmu_io_input_stat_stru_t
{
    uint16_t            io_input_stat;
    uint16_t            pwm1_input_duty;//0~100
    uint16_t            pwm2_input_duty;//0~100
    uint16_t            reserved[5];
}ModBMUIOInputStat_t;
#pragma pack()

#pragma pack(2)
typedef struct _mod_pack_statis_Info_stru_t
{
    uint16_t            total_volt;//0.1V
    int16_t             current;//0.1A
    int16_t             mesure_current_big;//0.1A
    int16_t             mesure_current_small;//0.1A
    uint16_t            soc_real;//0.1%
    uint16_t            soc_disp;//0.1%
    uint16_t            soh;//0.1%
    uint16_t            chg_cycle;
    uint16_t            dsg_cycle;
    uint16_t            bp;//0.1V
    uint16_t            pp;//0.1V
    uint16_t            chg_cur_limit;//0.1A
    uint16_t            dsg_cur_limit;//0.1A
    uint16_t            ins_res_p;//kΩ
    uint16_t            ins_res_n;//kΩ
    uint16_t            cell_max_volt_num;
    uint16_t            cell_max_volt;//mV
    uint16_t            cell_min_volt_num;
    uint16_t            cell_min_volt;//mV
    uint16_t            cell_max_temp_num;
    int16_t             cell_max_temp;//℃
    uint16_t            cell_min_temp_num;
    int16_t             cell_min_temp;//℃
    uint16_t            cell_max_volt_diff;//mV
    uint16_t            cell_max_temp_diff;//℃
    uint16_t            cell_max_pole_temp_num;
    int16_t             cell_max_pole_temp;//℃
    uint16_t            cell_min_pole_temp_num;
    int16_t             cell_min_pole_temp;//℃
    uint16_t            cell_max_other_temp_num;
    int16_t             cell_max_other_temp;//℃
    uint16_t            cell_min_other_temp_num;
    int16_t             cell_min_other_temp;//℃
    uint16_t            cell_ave_volt;//mV
    int16_t             cell_ave_temp;//℃
    uint16_t            reserved[15];
    uint16_t            chg_cur_day_accum_e_high;//0.1kWh
    uint16_t            chg_cur_day_accum_e_low;//0.1kWh
    uint16_t            dsg_cur_day_accum_e_high;//0.1kWh
    uint16_t            dsg_cur_day_accum_e_low;//0.1kWh
    uint16_t            chg_cur_month_accum_e_high;//0.1kWh
    uint16_t            chg_cur_month_accum_e_low;//0.1kWh
    uint16_t            dsg_cur_month_accum_e_high;//0.1kWh
    uint16_t            dsg_cur_month_accum_e_low;//0.1kWh
    uint16_t            chg_cur_year_accum_e_high;//0.1kWh
    uint16_t            chg_cur_year_accum_e_low;//0.1kWh
    uint16_t            dsg_cur_year_accum_e_high;//0.1kWh
    uint16_t            dsg_cur_year_accum_e_low;//0.1kWh
    uint16_t            chg_total_accum_e_high;//0.1kWh
    uint16_t            chg_total_accum_e_low;//0.1kWh
    uint16_t            dsg_total_accum_e_high;//0.1kWh
    uint16_t            dsg_total_accum_e_low;//0.1kWh
    uint16_t            chg_total_accum_q_high;//0.1Ah
    uint16_t            chg_total_accum_q_low;//0.1Ah
    uint16_t            dsg_total_accum_q_high;//0.1Ah
    uint16_t            dsg_total_accum_q_low;//0.1Ah
}ModPackStatisInfo_t;
#pragma pack()


#pragma pack(2)
typedef struct _mod_bcu_box_info_stru_t
{
    uint16_t            relay_p_stat;
    uint16_t            relay_n_stat;
    uint16_t            relay_pre_stat;
    uint16_t            qf_stat;
    uint16_t            relay_fan_stat;
}ModBCUBoxInfo_t;
#pragma pack()

#pragma pack(2)
typedef struct _mod_bcu_sys_info_stru_t
{
    uint16_t            sys_run_stat;
    uint16_t            sys_mark;
}ModBCUSysInfo_t;
#pragma pack()




#pragma pack(2)
typedef struct _mod_upstream_read_info_stru_t
{
    ModBMUAddressingInfo_t      bmu_addressing_info;
    ModBMUIOOutputStat_t        bmu_io_output_stat[15];
    ModBMUIOInputStat_t         bmu_io_input_stat[15];
    ModPackStatisInfo_t         pack_statis_info;
    ModBCUBoxInfo_t             box_info;
    ModBCUSysInfo_t             bcu_sys_info;
}ModUpstreamReadInfo_t;
#pragma pack()


typedef struct _mod_upstream_op_list_stru_t
{
    uint16_t                    region_reg_start_addr;
    uint16_t                    region_reg_min_len;
    uint16_t                    region_reg_max_len;
    EN_MODBUS_UPSTREAM_OP_STAT  (*analysis_fun)(uint16_t ,uint16_t, uint16_t *);
}ModUpstreamOpList_t;





//下述长度指的是寄存器数量，不是字节数
#define MODBUS_UPSTREAM_READ_BMU_ADDRESSING_INFO_START_ADDR     1000
#define MODBUS_UPSTREAM_READ_BMU_ADDRESSING_INFO_MAX_LEN        6
#define MODBUS_UPSTREAM_READ_BMU_IOOUT_INFO_START_ADDR          1010
#define MODBUS_UPSTREAM_READ_BMU_IOOUT_INFO_MAX_LEN             120
#define MODBUS_UPSTREAM_READ_BMU_IOIN_INFO_START_ADDR           1300
#define MODBUS_UPSTREAM_READ_BMU_IOIN_INFO_MAX_LEN              120
#define MODBUS_UPSTREAM_READ_PACK_STATIS_START_ADDR             1600
#define MODBUS_UPSTREAM_READ_PACK_STATIS_MAX_LEN                70
#define MODBUS_UPSTREAM_READ_BOX_INFO_START_ADDR                1700
#define MODBUS_UPSTREAM_READ_BOX_INFO_MAX_LEN                   5
#define MODBUS_UPSTREAM_READ_RTC_START_ADDR                     1720
#define MODBUS_UPSTREAM_READ_RTC_MAX_LEN                        8
#define MODBUS_UPSTREAM_READ_BCU_STAT_CTL_START_ADDR            1730
#define MODBUS_UPSTREAM_READ_BCU_STAT_CTL_MAX_LEN               2
#define MODBUS_UPSTREAM_READ_ALARM_START_ADDR                   1800
#define MODBUS_UPSTREAM_READ_ALARM_MAX_LEN                      18
#define MODBUS_UPSTREAM_READ_DETAIL_VOLT_START_ADDR             1900
#define MODBUS_UPSTREAM_READ_DETAIL_VOLT_MAX_LEN                500
#define MODBUS_UPSTREAM_READ_DETAIL_TEMP_START_ADDR             2400
#define MODBUS_UPSTREAM_READ_DETAIL_TEMP_MAX_LEN                500

#define MODBUS_UPSTREAM_READ_CORR_COEF_ADDR                     903
#define MODBUS_UPSTREAM_READ_CORR_COEF_MAX_LEN                  8

#define MODBUS_UPSTREAM_WRITE_BOX_ADDR                          1700    //长度1~5
#define MODBUS_UPSTREAM_WRITE_BOX_MAX_LEN                       5
#define MODBUS_UPSTREAM_WRITE_BOX_MIN_LEN                       1

#define MODBUS_UPSTREAM_WRITE_TIMESTAMP_ADDR                    1720    //长度只能为2
#define MODBUS_UPSTREAM_WRITE_TIMESTAMP_LEN                     2
#define MODBUS_UPSTREAM_WRITE_TIME_ADDR                         1722    //长度只能为6
#define MODBUS_UPSTREAM_WRITE_TIME_LEN                          6
#define MODBUS_UPSTREAM_WRITE_RELATIVE_TIMESTAMP_ADDR           1728    //长度只能为2
#define MODBUS_UPSTREAM_WRITE_RELATIVE_TIMESTAMP_LEN            2
#define MODBUS_UPSTREAM_WRITE_CLUSTER_ADDR                      1732    //长度只能为1
#define MODBUS_UPSTREAM_WRITE_CLUSTER_LEN                       1
#define MODBUS_UPSTREAM_WRITE_RESET_ADDR                        1733    //长度只能为1
#define MODBUS_UPSTREAM_WRITE_RESET_LEN                         1
#define MODBUS_UPSTREAM_WRITE_INS_RES_ADDR                      1734    //长度只能为1
#define MODBUS_UPSTREAM_WRITE_INS_RES_LEN                       1

#define MODBUS_UPSTREAM_WRITE_CLEAR_STORAGE_ADDR                900
#define MODBUS_UPSTREAM_WRITE_CLEAR_STORAGE_LEN                 1
#define MODBUS_UPSTREAM_WRITE_LATCH_CORR_COEF_ADDR              901
#define MODBUS_UPSTREAM_WRITE_LATCH_CORR_COEF_LEN               1
#define MODBUS_UPSTREAM_WRITE_CLEAR_CORR_COEF_ADDR              902
#define MODBUS_UPSTREAM_WRITE_CLEAR_CORR_COEF_LEN               1
#define MODBUS_UPSTREAM_WRITE_HALL_BIG_CORR_COEF_ADDR           903
#define MODBUS_UPSTREAM_WRITE_HALL_BIG_CORR_COEF_LEN            4
#define MODBUS_UPSTREAM_WRITE_HALL_SMALL_CORR_COEF_ADDR         907
#define MODBUS_UPSTREAM_WRITE_HALL_SMALL_CORR_COEF_LEN          4


EN_MODBUS_UPSTREAM_OP_STAT ModbusUpstreamReadJudgeAndAnalysis(uint16_t reg_addr, uint16_t reg_len, uint16_t *p_reg_value);
EN_MODBUS_UPSTREAM_OP_STAT ModbusUpstreamWriteJudgeAndAnalysis(uint16_t reg_addr, uint16_t reg_len, uint16_t *p_reg_value);
void ModbusUpstreamReadInfoProcess();

#endif












