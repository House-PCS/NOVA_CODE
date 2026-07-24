/*
 * ph3_calc_def.h
 *
 * 三相电量计算数据结构（CPU 运行，参考 inverter_3ph/cntl_sign_def.h）
 */
#ifndef ALGO_PH3_CALC_DEF_H_
#define ALGO_PH3_CALC_DEF_H_

#include "F28x_Project.h"
#include "ph3_calc_cfg.h"

/** 交流信号（瞬时值 + 有效值） */
typedef struct ph3_calc_ac_stru_t
{
    float32 inst;
    float32 rms;
} Ph3Calc_Ac_t;

/** 单相功率 */
typedef struct ph3_calc_phase_pwr_stru_t
{
    float32 pwr_act;   /* 有功功率 (W) */
    float32 pwr_rea;   /* 无功功率 (VAR) */
    float32 pwr_app;   /* 视在功率 (VA) */
    float32 pf;        /* 功率因数 */
} Ph3Calc_PhasePwr_t;

/** 单相数据（电压/电流/功率） */
typedef struct ph3_calc_phase_stru_t
{
    Ph3Calc_Ac_t vol;
    Ph3Calc_Ac_t cur;
    Ph3Calc_PhasePwr_t pwr;

} Ph3Calc_Phase_t;

/** SRF-PLL 状态（对正序 alpha/beta 锁相） */
typedef struct ph3_calc_pll_stru_t
{
    float32 kp;
    float32 ki;
    float32 step;
    float32 theta_temp;
    float32 theta_cos;
    float32 theta_sin;
    float32 integral;
    float32 vd;
    float32 vq;
    float32 error;
    float32 w;
    float32 freq;
} Ph3Calc_Pll_t;

/** 正负序分离数据 */
typedef struct ph3_calc_posneg_data_stru_t
{
    float32 a_instant;
    float32 b_instant;
    float32 c_instant;
    float32 alfa;
    float32 beta;
    float32 alfa_90;
    float32 beta_90;
    float32 alfa_90_1;
    float32 beta_90_1;
    float32 alfa_1;
    float32 beta_1;
    float32 alfa_pos;
    float32 alfa_neg;
    float32 beta_pos;
    float32 beta_neg;
    float32 d_pos;
    float32 d_neg;
    float32 q_pos;
    float32 q_neg;
} Ph3Calc_PosNegData_t;

/** 功率因数计算模式 */
typedef enum
{
    PH3_CALC_PF_MODE_NORMAL = 0, /* theta_diff = volt_theta - curr_theta */
    PH3_CALC_PF_MODE_INV    = 1  /* theta_diff = (volt_theta - (curr_theta + PI)) * -1 */
} Ph3Calc_PfMode_e;

/** PLL 锁相模式 */
typedef enum
{
    PH3_CALC_PLL_MODE_ABC = 0,     /* ThreePhasePLL1：Clarke 变换后直接锁相（电网/负载） */
    PH3_CALC_PLL_MODE_POSSEQ = 1   /* 正负序分离后对正序 alpha/beta 锁相（逆变控制） */
} Ph3Calc_PllMode_e;

/** RMS 累加器（每通道独立，由模块内部维护） */
typedef struct ph3_calc_rms_state_stru_t
{
    float32 count;
    float32 vol_rms_sum;
    float32 cur_rms_sum;
    float32 vi_sum;
    float32 vol_rms_input_1;
    float32 period_alpha_start; /* 上周期过零线性插值系数，用于本周期 count 起点修正 */
    float32 freq_cycle_time;  /* 本相单周期采样时间 (s) */
} Ph3Calc_RmsState_t;

/** 单通道三相电量（电网/逆变/负载等） */
typedef struct ph3_calc_channel_stru_t
{
    Ph3Calc_Phase_t phase_a;
    Ph3Calc_Phase_t phase_b;
    Ph3Calc_Phase_t phase_c;
    Ph3Calc_Pll_t volt_pll;
    Ph3Calc_Pll_t curr_pll;
    Ph3Calc_PosNegData_t volt_posneg;
    Ph3Calc_PosNegData_t curr_posneg;
    Ph3Calc_RmsState_t rms_a;
    Ph3Calc_RmsState_t rms_b;
    Ph3Calc_RmsState_t rms_c;
    float32 total_pwr_act;
    float32 total_pwr_rea;
    float32 total_pwr_app;
    float32 power_factor;
    float32 power_refactor;   /* sin(theta_diff)，无功功率符号因子 */
    float32 theta_diff;
    float32 freq_a;             /* A 相电压频率 (Hz) */
    float32 freq_b;             /* B 相电压频率 (Hz) */
    float32 freq_c;             /* C 相电压频率 (Hz) */
    Ph3Calc_PfMode_e pf_mode;
    Ph3Calc_PllMode_e pll_mode;
    uint16_t enable_volt_pll;
    uint16_t enable_curr_pll;
    uint16_t enable_power_calc;
} Ph3Calc_Channel_t;

#endif /* ALGO_PH3_CALC_DEF_H_ */
