
#ifndef __BMS_DATA_MANAGE_H_
#define __BMS_DATA_MANAGE_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "common.h"


#define PACK_NUM_MAX                15  //最大支持的从控数量，暂不涉及
#define CELL_VOLT_SAMPLE_MAX        64  //BCU当前最大支持单个BMU采样64串电芯电压
#define PACK_TEMP_SAMPLE_MAX        68  //BCU当前最大支持单个BMU采样64串电芯温度(包含电芯温度和极柱温度和正负极温度)
#define BALANCE_TEMP_SAMPLE_MAX     8   //BCU当前最大支持单个BMU均衡温度数量



typedef struct _bmu_io_ctl_info_stru_t
{
    uint8_t         report_enable;//上报报文使能，当前只用前两个bit，分别代表详细电压信息和详细温度信息
    uint8_t         io_out;
    uint8_t         pwm_duty_1;
    uint8_t         pwm_duty_2;
}BMUIOCtlInfo_t;





typedef struct _bmu_io_info_stru_t
{
    bool            valid_flag;//必须由对应从控上报后，数据才有效
    uint8_t         io_input_stat;//8个输入IO的状态
    uint8_t         io_output_stat;//8个输出IO的状态
    uint8_t         pwm_1_output_duty;//PWM1输出的占空比
    uint8_t         pwm_2_output_duty;//PWM1输出的占空比
    uint8_t         pwm_1_input_duty;//PWM1输入的占空比
    uint8_t         pwm_2_input_duty;//PWM1输入的占空比
}BMUIOInfo_t;


typedef struct _bmu_statis_volt_info_stru_t
{
    bool            valid_flag;//必须由对应从控上报后，数据才有效
    uint32_t        total_volt;//总压,mV
    uint8_t         cell_max_volt_num;//最高单体电压序号,从0开始
    uint16_t        cell_max_volt;//最高单体电压,mV
    uint8_t         cell_min_volt_num;//最低单体电压序号，从0开始
    uint16_t        cell_min_volt;//最低单体电压,mV
}BMUStatisVoltInfo_t;


typedef struct _bmu_statis_temp_info_stru_t
{
    bool            valid_flag;//必须由对应从控上报后，数据才有效
    uint8_t         cell_max_temp_num;//最高单体温度序号,从0开始
    int8_t          cell_max_temp;//最高单体温度值，℃
    uint8_t         cell_min_temp_num;//最低单体温度序号，从0开始
    int8_t          cell_min_temp;//最低单体温度值，℃
    uint8_t         pole_max_temp_num;//最高极柱温度序号,从0开始
    int8_t          pole_max_temp;//最高极柱温度值，℃
    uint8_t         pole_min_temp_num;//最低极柱温度序号，从0开始
    int8_t          pole_min_temp;//最低极柱温度值，℃
}BMUStatisTempInfo_t;


typedef struct _bmu_statis_other_info_stru_t
{
    bool            valid_flag;//必须由对应从控上报后，数据才有效
    uint16_t        cell_ave_volt;//单体平均电压,mV
    int8_t          cell_ave_temp;//单体平均温度
    int8_t          equalizing_plate_max_temp;//均衡板最高温度
    uint16_t        fault_info;//故障信息
}BMUStatisOtherInfo_t;

typedef struct _bmu_detail_volt_info_stru_t
{
    bool            valid_flag;//必须由对应从控上报后，数据才有效
//    uint8_t         cell_num;//有效电芯数量
    uint16_t        cell_volt[CELL_VOLT_SAMPLE_MAX];//单体电压,mV
    bool            cell_balance_stat[CELL_VOLT_SAMPLE_MAX];//单体被动均衡状态
}BMUDetailVoltInfo_t;

typedef struct _bmu_detail_temp_info_stru_t
{
    bool            valid_flag;//必须由对应从控上报后，数据才有效
//    uint8_t         temp_num;//有效温度数量
    int8_t          cell_temp[PACK_TEMP_SAMPLE_MAX];//单体温度
}BMUDetailTempInfo_t;


typedef struct _bmu_calc_info_stru_t
{
    bool            valid_flag;//必须完成至少一次计算，数据才有效
    
    uint16_t        cell_max_volt_num;//结合所有从控，最大单体电压序号
    uint16_t        cell_max_volt;//结合所有从控，最大单体电压,mV
    uint16_t        cell_min_volt_num;//结合所有从控，最小单体电压序号
    uint16_t        cell_min_volt;//结合所有从控，最小单体电压,mV
    uint16_t        cell_max_temp_num;//结合所有从控，最高单体温度序号
    int8_t          cell_max_temp;//结合所有从控，最高单体温度
    uint16_t        cell_min_temp_num;//结合所有从控，最低单体温度序号
    int8_t          cell_min_temp;//结合所有从控，最低单体温度
    uint16_t        cell_ave_volt;//结合所有从控，平均电芯电压,mV
    int8_t          cell_ave_temp;//结合所有从控，平均电芯温度
    uint32_t        cell_total_volt;//结合所有从控，总压,mV
    uint16_t        pole_max_temp_num;//结合所有从控，最高极柱温度序号
    int8_t          pole_max_temp;//结合所有从控，最高极柱温度
    uint16_t        pole_min_temp_num;//结合所有从控，最低极柱温度序号
    int8_t          pole_min_temp;//结合所有从控，最低极柱温度
    int8_t          pole_ave_temp;//结合所有从控，平均极柱温度，暂不使用
}BMUCalcInfo_t;





#pragma pack(1) 
typedef struct _bmu_sample_info_stru_t
{
    uint8_t         pack_num; //该参数只是主控期望的从控数量,后续可能支持不同PACK采样通道数不一样的情况，基于此，下方的成员均要增加从控数量数组下标
    uint8_t         cell_volt_sample_num; //对应从控最大支持的电芯电压采样数(不是实际用到的，实际用到的可能小于此值)
    uint8_t         pack_temp_sample_num; //对应从控最大支持的温度采样数，包含电芯温度和极柱温度和正负极温度(不是实际用到的，实际用到的可能小于此值)
    uint8_t         balance_temp_sample_num; //实际从控最大支持的均衡温度采样数(依据PCB板实际设计的数量)
    uint8_t         cell_volt_mask[(CELL_VOLT_SAMPLE_MAX + 7) / 8];
    uint8_t         cell_temp_mask[(PACK_TEMP_SAMPLE_MAX + 7) / 8];//单纯电芯，可以包含极柱和正负极，也可不包含
    uint8_t         pole_temp_mask[(PACK_TEMP_SAMPLE_MAX + 7) / 8];//极柱，与other_temp_mask值对应bit不可同时为1
    uint8_t         other_temp_mask[(PACK_TEMP_SAMPLE_MAX + 7) / 8];//正负极,与pole_temp_mask值对应bit不可同时为1
    uint8_t         balance_temp_mask[(BALANCE_TEMP_SAMPLE_MAX + 7) / 8];
}BMUSampleInfo_t;
#pragma pack()


#pragma pack(1) 
typedef struct _bcu_func_threshold_stru_t
{
    uint16_t        balance_start_diff_volt;//均衡开启的最小压差(与单节最小电压相比),单位mV
    uint16_t        balance_min_volt;//均衡要求的最小电压,单位mV
    int8_t          fan_start_temp;//风扇起转温度
    int8_t          fan_full_temp;//风扇满转温度
    int8_t          fan_stop_temp;//风扇停转温度
    int8_t          fan_start_duty;//风扇起转占空比
    uint16_t        pre_charge_diff_volt;//预充压差(B+和P+的压差不大于该值表示预充完成)，单位mV
    uint16_t        hv_bmu_total_diff_max_volt;//高压采集和BMU上报的总压之间的最大压差, 单位mV
    uint32_t        pre_charge_complete_max_current;//完成预充切入主正继电器的最大电流，电流大于此值时，不能闭合主正继电器，单位mA
    uint32_t        cut_off_relay_p_max_current;//退簇或者其他断开继电器操作前，需要判断电流绝对值是否大于此值，如大于，不能直接切断继电器，要先切断路器
}BCUFuncThreshold_t;
#pragma pack()


typedef struct _bms_data_info_t
{
    EN_OP_RES               bmu_adressing_res;//编址结果
    uint8_t                 bmu_num;//从控数量(编址成功后实际获取到的，通过编址流程获取，不能超过BMU_MAX_SUPPORT_NUM
    uint8_t                 per_bmu_cell_num;//单个从控电芯实际数量
    uint8_t                 per_bmu_cell_temp_num;//单个从控电芯温度实际数量
    uint8_t                 per_bmu_pole_temp_num;//单个从控极柱温度实际数量
    uint8_t                 per_bmu_other_temp_num;//单个从控正负极温度实际数量
    /*需要下发给BMU的数据*/
    BMUIOCtlInfo_t          bmu_io_ctl_info[PACK_NUM_MAX];
    uint8_t                 bmu_passive_balance_info[PACK_NUM_MAX][(CELL_VOLT_SAMPLE_MAX + 7) / 8];
    /*从BMU获取到的信息*/
    BMUIOInfo_t             bmu_io_info[PACK_NUM_MAX];
    BMUStatisVoltInfo_t     bmu_statis_volt_info[PACK_NUM_MAX];
    BMUStatisTempInfo_t     bmu_statis_temp_info[PACK_NUM_MAX];
    BMUStatisOtherInfo_t    bmu_statis_other_info[PACK_NUM_MAX];
    BMUDetailVoltInfo_t     bmu_detail_volt_info[PACK_NUM_MAX];
    BMUDetailTempInfo_t     bmu_detail_temp_info[PACK_NUM_MAX];
    /*控制BMU的指令信息*/
//    uint8_t                 bmu_detail_report_enable[PACK_NUM_MAX];//从控详细信息上报使能，目前只用到bit0(详细电压信息)和bit1(详细温度信息)
//    uint8_t                 bmu_io_output_value[PACK_NUM_MAX];//各个从控IO输出的值
//    uint8_t                 bmu_pwm_1_output_duty;//各个从控PWM1输出占空比
//    uint8_t                 bmu_pwm_2_output_duty;//各个从控PWM2输出占空比
    /*需要计算的信息*/
    BMUCalcInfo_t           bmu_calc_info;
    /*BCU自身信息*/ 
    uint32_t                bmu_comm_last_time[PACK_NUM_MAX];//和从控上次通信时刻，用于判断通信超时
    uint32_t                pcs_comm_last_time;//和PCS上次通信时刻，用于判断通信超时
    bool                    relay_p_stat;//总正继电器操作值，true表示闭合，false表示断开
    bool                    relay_pre_stat;//预充继电器状态操作值，true表示闭合，false表示断开
    bool                    relay_fan_stat;//风扇继电器状态操作值，true表示闭合，false表示断开
    bool                    breaker_stat;//断路器状态，true表示触发了脱扣操作，false表示未触发脱扣操作
    bool                    current_forbid_flag;//禁止充放电，用于在并簇前或者即将退簇时，PCS禁止给电池充放电
    bool                    bcu_reset_flag;//系统复位标志
    uint32_t                bcu_reset_delay_count;//系统复位延时计数
    bool                    factory_test_mode;//工厂测试模式，可支持工装测试，在该模式下，部分业务停止运行
    bool                    running_ins_res_cmd;//运行过程中启动绝缘检测命令
    /*BMU采样信息*/
    BMUSampleInfo_t         bmu_sample_info;
    /*BCU其他阈值信息*/
    BCUFuncThreshold_t      bcu_func_threshold;
    
    /*设备基础信息*/
    char                    sn[32];//字符串形式，以'\0'结尾
    uint8_t                 model_id[2];
    char                    fw_version[16];//字符串形式，以'\0'结尾
}BMSDataInfo_t;

extern BMSDataInfo_t gBMSDataInfo;

void BMSCalcDataProcess(void);


#endif

