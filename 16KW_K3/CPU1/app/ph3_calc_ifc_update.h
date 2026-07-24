/*
 * ph3_calc_ifc_update.h
 *
 * CPU1 read-side ph3 calc interface (data produced by CPU2, GS4 shared RAM).
 * Ph3CalcInfo_t layout mirrors algo/cla_cpu_share.h AlgoInfo_t for application use.
 * Raw shared layout must match CPU2 algo/ph3_calc/ph3_calc_interface.h.
 */
#ifndef APP_PH3_CALC_IFC_UPDATE_H_
#define APP_PH3_CALC_IFC_UPDATE_H_

#include <stdbool.h>
#include <stdint.h>
#include "F28x_Project.h"

/** 电压信息（对应 ClaVol_t） */
typedef struct ph3_calc_info_vol_stru_t
{
    float32 a_phase_rms;
    float32 b_phase_rms;
    float32 c_phase_rms;
    float32 freq;
    float32 theta;
} Ph3CalcInfoVol_t;

/** 电流信息（对应 ClaCurr_t） */
typedef struct ph3_calc_info_curr_stru_t
{
    float32 a_phase_rms;
    float32 b_phase_rms;
    float32 c_phase_rms;
    float32 freq;
    float32 theta;
} Ph3CalcInfoCurr_t;

/** 功率信息（对应 ClaPower_t） */
typedef struct ph3_calc_info_power_stru_t
{
    float32 a_phase_active;
    float32 a_phase_reactive;
    float32 a_phase_apperant;

    float32 b_phase_active;
    float32 b_phase_reactive;
    float32 b_phase_apperant;

    float32 c_phase_active;
    float32 c_phase_reactive;
    float32 c_phase_apperant;

    float32 total_active;
    float32 total_reactive;
    float32 total_apperant;

    float32 power_factor;
} Ph3CalcInfoPower_t;

/** 母线滤波量（对应 AlgoInfo_t 中 bus 相关字段） */
typedef struct ph3_calc_info_bus_stru_t
{
    float32 vol_filter;
    float32 curr_filter;
    float32 power_filter;
    float32 pos_vol_filter;
    float32 neg_vol_filter;
} Ph3CalcInfoBus_t;

/** CPU2 ph3 计算数据信息（对应 AlgoInfo_t，供 CPU1 应用层使用） */
typedef struct ph3_calc_info_stru_t
{
    Ph3CalcInfoVol_t inv_vol;
    Ph3CalcInfoVol_t grid_vol;
    Ph3CalcInfoVol_t gen_vol;
    Ph3CalcInfoVol_t load_vol;
    Ph3CalcInfoVol_t mid_vol;

    Ph3CalcInfoCurr_t inv_curr;
    Ph3CalcInfoCurr_t grid_curr;
    Ph3CalcInfoCurr_t gen_curr;
    Ph3CalcInfoCurr_t load_curr;
    Ph3CalcInfoCurr_t ct_curr;

    Ph3CalcInfoPower_t inv_power;
    Ph3CalcInfoPower_t grid_power;
    Ph3CalcInfoPower_t gen_power;
    Ph3CalcInfoPower_t load_power;
    Ph3CalcInfoPower_t ct_power;

    Ph3CalcInfoBus_t bus;

    float32 grid_a_phase_count;
    float32 grid_b_phase_count;
    float32 grid_c_phase_count;

    float32 gen_a_phase_count;
    float32 gen_b_phase_count;
    float32 gen_c_phase_count;

    float32 rcmu_curr_rms;
    uint32_t ferq_devision;
} Ph3CalcInfo_t;

extern Ph3CalcInfo_t gstPh3CalcInfo;

void Ph3CalcInfo_Init(void);
bool Ph3CalcInfo_Update(void);

#endif /* APP_PH3_CALC_IFC_UPDATE_H_ */
