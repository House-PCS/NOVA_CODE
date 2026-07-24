/*
 * ph3_calc_interface.h
 *
 * 三相电量计算对外接口数据结构（供应用层/双核通信读取）
 */
#ifndef ALGO_PH3_CALC_INTERFACE_H_
#define ALGO_PH3_CALC_INTERFACE_H_

#include "ph3_calc_def.h"

/** 三相电压接口量 */
typedef struct ph3_calc_ifc_vol_stru_t
{
    float32 a_phase_instant;
    float32 b_phase_instant;
    float32 c_phase_instant;
    float32 a_phase_rms;
    float32 b_phase_rms;
    float32 c_phase_rms;
    float32 freq_a;
    float32 freq_b;
    float32 freq_c;
    float32 theta;          /* 电压锁相角 (rad)，取自 volt_pll.theta_cos */
} Ph3Calc_IfcVol_t;

/** 三相电流接口量 */
typedef struct ph3_calc_ifc_curr_stru_t
{
    float32 a_phase_instant;
    float32 b_phase_instant;
    float32 c_phase_instant;
    float32 a_phase_rms;
    float32 b_phase_rms;
    float32 c_phase_rms;
    float32 freq_a;
    float32 freq_b;
    float32 freq_c;
    float32 theta;          /* 电流锁相角 (rad)，取自 curr_pll.theta_cos */
} Ph3Calc_IfcCurr_t;

/** 三相功率接口量 */
typedef struct ph3_calc_ifc_power_stru_t
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
} Ph3Calc_IfcPower_t;

/** 正负序分离接口量（电压环/电流环） */
typedef struct ph3_calc_ifc_posneg_stru_t
{
    float32 alfa_pos;
    float32 beta_pos;
    float32 alfa_neg;
    float32 beta_neg;
    float32 d_pos;
    float32 q_pos;
    float32 d_neg;
    float32 q_neg;
} Ph3Calc_IfcPosNeg_t;

/** 单通道三相电量对外接口 */
typedef struct ph3_calc_interface_stru_t
{
    Ph3Calc_IfcVol_t vol;
    Ph3Calc_IfcCurr_t curr;
    Ph3Calc_IfcPower_t power;
    float32 vol_a_phase_count;  /* A 相电压 RMS 周期采样点数 */
    float32 vol_b_phase_count;
    float32 vol_c_phase_count;
    Ph3Calc_IfcPosNeg_t volt_posneg;
    Ph3Calc_IfcPosNeg_t curr_posneg;
} Ph3Calc_Interface_t;

void Ph3Calc_InterfaceInit(Ph3Calc_Interface_t* ifc);
void Ph3Calc_InterfaceUpdate(Ph3Calc_Interface_t* ifc, const Ph3Calc_Channel_t* ch);
void Ph3Calc_InterfaceAngleUpdate(Ph3Calc_Interface_t* ifc, const Ph3Calc_Channel_t* ch);

#endif /* ALGO_PH3_CALC_INTERFACE_H_ */
