# 附录 B：算法层源码（algo/）

> 本附录收录三相电量计算算法与 CLA 预留代码。不含 TI 芯片库。


---

## algo/ph3_calc/ph3_calc_cfg.h

```c
/*
 * ph3_calc_cfg.h
 *
 * 三相电量计算模块配置参数（CPU 实现，算法参数与 CPU1 控制环一致）
 */
#ifndef ALGO_PH3_CALC_CFG_H_
#define ALGO_PH3_CALC_CFG_H_

#define PH3_CALC_PI                 (3.1415926f)
#define PH3_CALC_M_2PI              (6.2831853f)
#define PH3_CALC_TS                 (0.00005f)   /* 控制周期 1/20kHz */
#define PH3_CALC_PLL_KP             (1.43f)
#define PH3_CALC_PLL_KI             (317.0f)
#define PH3_CALC_PLL_W_NOMINAL      (100.0f * PH3_CALC_PI)
#define PH3_CALC_PLL_W_LIMIT        (50.0f * PH3_CALC_PI)
#define PH3_CALC_PLL_FREQ_MIN       (40.0f)
#define PH3_CALC_PLL_FREQ_MAX       (70.0f)
#define PH3_CALC_RMS_COUNT_MIN      (250.0f)     /* 对应约 80Hz 上限 */
#define PH3_CALC_RMS_COUNT_MAX      (666.0f)     /* 对应约 30Hz 下限 */
#define PH3_CALC_POSNEG_BP_FREQ     (50.0f)      /* 正负序分离带通滤波中心频率 (Hz) */

#define PH3_CALC_VALUE_LIMIT(x, max_v, min_v) \
    (((x) > (max_v)) ? (max_v) : (((x) < (min_v)) ? (min_v) : (x)))

#endif /* ALGO_PH3_CALC_CFG_H_ */

```

---

## algo/ph3_calc/ph3_calc_def.h

```c
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

```

---

## algo/ph3_calc/ph3_calc_math.h

```c
/*
 * ph3_calc_math.h
 *
 * CPU FPU 数学接口，替代 CLAmath（CLAcos / CLAsin / CLAsqrt / CLAdiv）
 */
#ifndef ALGO_PH3_CALC_MATH_H_
#define ALGO_PH3_CALC_MATH_H_

#include <math.h>

#define Ph3Calc_Cos(x)    cosf(x)
#define Ph3Calc_Sin(x)    sinf(x)
#define Ph3Calc_Sqrt(x)   sqrtf(x)
#define Ph3Calc_Div(n, d) ((n) / (d))

#endif /* ALGO_PH3_CALC_MATH_H_ */

```

---

## algo/ph3_calc/ph3_calc_channel.h

```c
/*
 * ph3_calc_channel.h
 *
 * 三相计算通道：采样输入与调度运行
 */
#ifndef ALGO_PH3_CALC_CHANNEL_H_
#define ALGO_PH3_CALC_CHANNEL_H_

#include "ph3_calc_def.h"

void Ph3Calc_ChannelInit(Ph3Calc_Channel_t* ch,
                         Ph3Calc_PfMode_e pf_mode,
                         Ph3Calc_PllMode_e pll_mode);
void Ph3Calc_ChannelSetSample(Ph3Calc_Channel_t* ch,
                              float32 va, float32 vb, float32 vc,
                              float32 ia, float32 ib, float32 ic);
void Ph3Calc_ChannelRun(Ph3Calc_Channel_t* ch);

#endif /* ALGO_PH3_CALC_CHANNEL_H_ */

```

---

## algo/ph3_calc/ph3_calc_channel.c

```c
/*
 * ph3_calc_channel.c
 *
 * 单通道三相计算调度（CPU 运行）：正负序分离 + PLL + RMS + 功率
 */
#include "ph3_calc_channel.h"
#include "ph3_calc_pll.h"
#include "ph3_calc_posneg.h"
#include "ph3_calc_rms.h"
#include "ph3_calc_power.h"
#include "ph3_calc_math.h"

#pragma CODE_SECTION(Ph3Calc_ChannelUpdateFreq, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_ChannelSetSample, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_ChannelRunVoltPll, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_ChannelRunCurrPll, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_ChannelRun, ".TI.ramfunc");

static void Ph3Calc_ChannelUpdateFreq(float32 cycle_time, float32* freq)
{
    float32 freq_temp;

    if((freq == NULL) || (cycle_time <= 0.0f))
    {
        return;
    }

    freq_temp = Ph3Calc_Div(1.0f, cycle_time);
    *freq = PH3_CALC_VALUE_LIMIT(freq_temp,
                                 PH3_CALC_PLL_FREQ_MAX,
                                 PH3_CALC_PLL_FREQ_MIN);
}

void Ph3Calc_ChannelInit(Ph3Calc_Channel_t* ch,
                         Ph3Calc_PfMode_e pf_mode,
                         Ph3Calc_PllMode_e pll_mode)
{
    if(ch == NULL)
    {
        return;
    }

    ch->phase_a.vol.inst = 0.0f;
    ch->phase_a.vol.rms = 0.0f;
    ch->phase_a.cur.inst = 0.0f;
    ch->phase_a.cur.rms = 0.0f;
    ch->phase_b = ch->phase_a;
    ch->phase_c = ch->phase_a;

    ch->total_pwr_act = 0.0f;
    ch->total_pwr_rea = 0.0f;
    ch->total_pwr_app = 0.0f;
    ch->power_factor = 0.0f;
    ch->power_refactor = 0.0f;
    ch->theta_diff = 0.0f;
    ch->freq_a = 0.0f;
    ch->freq_b = 0.0f;
    ch->freq_c = 0.0f;
    ch->pf_mode = pf_mode;
    ch->pll_mode = pll_mode;
    ch->enable_volt_pll = 1U;
    ch->enable_curr_pll = 1U;
    ch->enable_power_calc = 1U;

    Ph3Calc_PosNegInit(&ch->volt_posneg);
    Ph3Calc_PosNegInit(&ch->curr_posneg);
    Ph3Calc_RmsStateInit(&ch->rms_a);
    Ph3Calc_RmsStateInit(&ch->rms_b);
    Ph3Calc_RmsStateInit(&ch->rms_c);
    Ph3Calc_PllInit(&ch->volt_pll, PH3_CALC_TS);
    Ph3Calc_PllInit(&ch->curr_pll, PH3_CALC_TS);
}

void Ph3Calc_ChannelSetSample(Ph3Calc_Channel_t* ch,
                              float32 va, float32 vb, float32 vc,
                              float32 ia, float32 ib, float32 ic)
{
    if(ch == NULL)
    {
        return;
    }

    ch->phase_a.vol.inst = va;
    ch->phase_b.vol.inst = vb;
    ch->phase_c.vol.inst = vc;
    ch->phase_a.cur.inst = ia;
    ch->phase_b.cur.inst = ib;
    ch->phase_c.cur.inst = ic;
}

static void Ph3Calc_ChannelRunVoltPll(Ph3Calc_Channel_t* ch)
{
    if(ch->pll_mode == PH3_CALC_PLL_MODE_POSSEQ)
    {
        Ph3Calc_PosNegRun(&ch->volt_posneg,
                          ch->phase_a.vol.inst,
                          ch->phase_b.vol.inst,
                          ch->phase_c.vol.inst,
                          ch->volt_pll.theta_cos);
        Ph3Calc_PllRun(&ch->volt_pll,
                       ch->volt_posneg.alfa_pos,
                       ch->volt_posneg.beta_pos);
    }
    else
    {
        Ph3Calc_PllRunAbc(&ch->volt_pll,
                          ch->phase_a.vol.inst,
                          ch->phase_b.vol.inst,
                          ch->phase_c.vol.inst);
    }
}

static void Ph3Calc_ChannelRunCurrPll(Ph3Calc_Channel_t* ch)
{
    if(ch->pll_mode == PH3_CALC_PLL_MODE_POSSEQ)
    {
        Ph3Calc_PosNegRun(&ch->curr_posneg,
                          ch->phase_a.cur.inst,
                          ch->phase_b.cur.inst,
                          ch->phase_c.cur.inst,
                          ch->volt_pll.theta_cos);
        Ph3Calc_PllRun(&ch->curr_pll,
                       ch->curr_posneg.alfa_pos,
                       ch->curr_posneg.beta_pos);
    }
    else
    {
        Ph3Calc_PllRunAbc(&ch->curr_pll,
                          ch->phase_a.cur.inst,
                          ch->phase_b.cur.inst,
                          ch->phase_c.cur.inst);
    }
}

void Ph3Calc_ChannelRun(Ph3Calc_Channel_t* ch)
{
    if(ch == NULL)
    {
        return;
    }

    if(ch->enable_volt_pll != 0U)
    {
        Ph3Calc_ChannelRunVoltPll(ch);
    }
    else
    {
        Ph3Calc_PllInit(&ch->volt_pll, PH3_CALC_TS);
        Ph3Calc_PosNegInit(&ch->volt_posneg);
    }

    if(ch->enable_curr_pll != 0U)
    {
        Ph3Calc_ChannelRunCurrPll(ch);
    }
    else
    {
        Ph3Calc_PllInit(&ch->curr_pll, PH3_CALC_TS);
        Ph3Calc_PosNegInit(&ch->curr_posneg);
    }

    if(ch->enable_power_calc != 0U)
    {
        Ph3Calc_PowerUpdateFactor(ch);

        Ph3Calc_RmsStateRun(&ch->rms_a, &ch->phase_a);
        Ph3Calc_RmsStateRun(&ch->rms_b, &ch->phase_b);
        Ph3Calc_RmsStateRun(&ch->rms_c, &ch->phase_c);
        Ph3Calc_PowerUpdateReactive(ch);

        Ph3Calc_ChannelUpdateFreq(ch->rms_a.freq_cycle_time, &ch->freq_a);
        Ph3Calc_ChannelUpdateFreq(ch->rms_b.freq_cycle_time, &ch->freq_b);
        Ph3Calc_ChannelUpdateFreq(ch->rms_c.freq_cycle_time, &ch->freq_c);
        ch->volt_pll.freq = ch->freq_a;
    }
    else
    {
        ch->phase_a.vol.rms = 0.0f;
        ch->phase_a.cur.rms = 0.0f;
        ch->phase_b.vol.rms = 0.0f;
        ch->phase_b.cur.rms = 0.0f;
        ch->phase_c.vol.rms = 0.0f;
        ch->phase_c.cur.rms = 0.0f;
        ch->phase_a.pwr.pwr_act = 0.0f;
        ch->phase_a.pwr.pwr_rea = 0.0f;
        ch->phase_a.pwr.pwr_app = 0.0f;
        ch->phase_a.pwr.pf = 0.0f;
        ch->phase_b.pwr = ch->phase_a.pwr;
        ch->phase_c.pwr = ch->phase_a.pwr;
        ch->total_pwr_act = 0.0f;
        ch->total_pwr_rea = 0.0f;
        ch->total_pwr_app = 0.0f;
        ch->power_factor = 0.0f;
        ch->power_refactor = 0.0f;
        ch->theta_diff = 0.0f;
        ch->freq_a = 0.0f;
        ch->freq_b = 0.0f;
        ch->freq_c = 0.0f;
        ch->volt_pll.freq = 0.0f;
        Ph3Calc_RmsStateInit(&ch->rms_a);
        Ph3Calc_RmsStateInit(&ch->rms_b);
        Ph3Calc_RmsStateInit(&ch->rms_c);
    }
}

```

---

## algo/ph3_calc/ph3_calc_pll.h

```c
/*
 * ph3_calc_pll.h
 *
 * SRF 锁相环（CPU FPU 实现）
 */
#ifndef ALGO_PH3_CALC_PLL_H_
#define ALGO_PH3_CALC_PLL_H_

#include "ph3_calc_def.h"

void Ph3Calc_PllInit(Ph3Calc_Pll_t* pll, float32 step);
void Ph3Calc_PllRun(Ph3Calc_Pll_t* pll, float32 alfa, float32 beta);
void Ph3Calc_PllRunAbc(Ph3Calc_Pll_t* pll, float32 va, float32 vb, float32 vc);

#endif /* ALGO_PH3_CALC_PLL_H_ */

```

---

## algo/ph3_calc/ph3_calc_pll.c

```c
/*
 * ph3_calc_pll.c
 *
 * 正序 SRF-PLL（CPU FPU 实现，对应 ThreePhasePLL / ThreePhasePLL1）
 */
#include "ph3_calc_pll.h"
#include "ph3_calc_math.h"

#pragma CODE_SECTION(Ph3Calc_WrapTheta, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_Clarke, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_PllRun, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_PllRunAbc, ".TI.ramfunc");

static void Ph3Calc_PllResetState(Ph3Calc_Pll_t* pll)
{
    pll->theta_temp = 0.0f;
    pll->theta_cos = 0.0f;
    pll->theta_sin = 0.0f;
    pll->integral = 0.0f;
    pll->vd = 0.0f;
    pll->vq = 0.0f;
    pll->error = 0.0f;
    pll->w = 0.0f;
    pll->freq = 0.0f;
}

static float32 Ph3Calc_WrapTheta(float32 theta)
{
    if(theta > (2.0f * PH3_CALC_PI))
    {
        theta -= 2.0f * PH3_CALC_PI;
    }
    if(theta < (-2.0f * PH3_CALC_PI))
    {
        theta += 2.0f * PH3_CALC_PI;
    }
    return theta;
}

static void Ph3Calc_Clarke(float32 va, float32 vb, float32 vc,
                           float32* alfa, float32* beta)
{
    *alfa = (va - 0.5f * vb - 0.5f * vc) * 0.66666667f;
    *beta = (0.866025f * vb - 0.866025f * vc) * 0.66666667f;
}

void Ph3Calc_PllInit(Ph3Calc_Pll_t* pll, float32 step)
{
    if(pll == NULL)
    {
        return;
    }

    pll->kp = PH3_CALC_PLL_KP;
    pll->ki = PH3_CALC_PLL_KI;
    pll->step = step;
    Ph3Calc_PllResetState(pll);
}

void Ph3Calc_PllRun(Ph3Calc_Pll_t* pll, float32 alfa, float32 beta)
{
    if(pll == NULL)
    {
        return;
    }

    pll->vd = alfa * Ph3Calc_Cos(pll->theta_temp) + beta * Ph3Calc_Sin(pll->theta_temp);
    pll->vq = (-1.0f * alfa * Ph3Calc_Sin(pll->theta_temp)) + beta * Ph3Calc_Cos(pll->theta_temp);

    pll->error = pll->vq;
    pll->integral += pll->step * pll->error;
    pll->integral = PH3_CALC_VALUE_LIMIT(pll->integral,
                                           PH3_CALC_PLL_W_LIMIT,
                                           -PH3_CALC_PLL_W_LIMIT);

    pll->w = pll->kp * pll->error + pll->ki * pll->integral;
    pll->w = PH3_CALC_VALUE_LIMIT(pll->w, PH3_CALC_PLL_W_LIMIT, -PH3_CALC_PLL_W_LIMIT);

    pll->theta_temp += (pll->w + PH3_CALC_PLL_W_NOMINAL) * pll->step;
    pll->theta_temp = Ph3Calc_WrapTheta(pll->theta_temp);

    pll->theta_cos = pll->theta_temp;
    pll->theta_sin = pll->theta_cos + 0.5f * PH3_CALC_PI;
}

void Ph3Calc_PllRunAbc(Ph3Calc_Pll_t* pll, float32 va, float32 vb, float32 vc)
{
    float32 alfa;
    float32 beta;

    if(pll == NULL)
    {
        return;
    }

    Ph3Calc_Clarke(va, vb, vc, &alfa, &beta);
    Ph3Calc_PllRun(pll, alfa, beta);
}

```

---

## algo/ph3_calc/ph3_calc_posneg.h

```c
/*
 * ph3_calc_posneg.h
 *
 * 三相正负序分离（Clarke + 90° 带通 + 正负序分解 + Park）
 */
#ifndef ALGO_PH3_CALC_POSNEG_H_
#define ALGO_PH3_CALC_POSNEG_H_

#include "ph3_calc_def.h"

void Ph3Calc_PosNegInit(Ph3Calc_PosNegData_t* data);
void Ph3Calc_PosNegRun(Ph3Calc_PosNegData_t* data,
                       float32 va, float32 vb, float32 vc,
                       float32 theta);

#endif /* ALGO_PH3_CALC_POSNEG_H_ */

```

---

## algo/ph3_calc/ph3_calc_posneg.c

```c
/*
 * ph3_calc_posneg.c
 *
 * 三相正负序分离（CPU FPU 实现，对应 ThreePhaseTransformPosNegOrder）
 */
#include "ph3_calc_posneg.h"
#include "ph3_calc_math.h"

#pragma CODE_SECTION(Ph3Calc_PosNegRun, ".TI.ramfunc");

void Ph3Calc_PosNegInit(Ph3Calc_PosNegData_t* data)
{
    if(data == NULL)
    {
        return;
    }

//    data->a_instant = 0.0f;
//    data->b_instant = 0.0f;
//    data->c_instant = 0.0f;
//    data->alfa = 0.0f;
//    data->beta = 0.0f;
    data->alfa_90 = 0.0f;
    data->beta_90 = 0.0f;
    data->alfa_90_1 = 0.0f;
    data->beta_90_1 = 0.0f;
    data->alfa_1 = 0.0f;
    data->beta_1 = 0.0f;
    data->alfa_pos = 0.0f;
    data->alfa_neg = 0.0f;
    data->beta_pos = 0.0f;
    data->beta_neg = 0.0f;
    data->d_pos = 0.0f;
    data->d_neg = 0.0f;
    data->q_pos = 0.0f;
    data->q_neg = 0.0f;
}

void Ph3Calc_PosNegRun(Ph3Calc_PosNegData_t* data,
                       float32 va, float32 vb, float32 vc,
                       float32 theta)
{
    float32 w;
    float32 a1;
    float32 b0;
    float32 b1;
    float32 cos_theta;
    float32 sin_theta;
    float32 cos_neg_theta;
    float32 sin_neg_theta;

    if(data == NULL)
    {
        return;
    }

    data->a_instant = va;
    data->b_instant = vb;
    data->c_instant = vc;

    w = 2.0f * PH3_CALC_PI * PH3_CALC_POSNEG_BP_FREQ;
    a1 = Ph3Calc_Div((w * PH3_CALC_TS - 2.0f), (w * PH3_CALC_TS + 2.0f));
    b0 = Ph3Calc_Div((-2.0f + w * PH3_CALC_TS), (w * PH3_CALC_TS + 2.0f));
    b1 = 1.0f;

    data->alfa = (va - 0.5f * vb - 0.5f * vc) * 0.66666667f;
    data->beta = (0.866025f * vb - 0.866025f * vc) * 0.66666667f;

    data->alfa_90 = (-1.0f * a1 * data->alfa_90_1) + (b0 * data->alfa) + (b1 * data->alfa_1);
    data->beta_90 = (-1.0f * a1 * data->beta_90_1) + (b0 * data->beta) + (b1 * data->beta_1);

    data->alfa_90_1 = data->alfa_90;
    data->beta_90_1 = data->beta_90;
    data->alfa_1 = data->alfa;
    data->beta_1 = data->beta;

    data->alfa_pos = 0.5f * data->alfa - 0.5f * data->beta_90;
    data->beta_pos = 0.5f * data->beta + 0.5f * data->alfa_90;
    data->alfa_neg = 0.5f * data->alfa + 0.5f * data->beta_90;
    data->beta_neg = 0.5f * data->beta - 0.5f * data->alfa_90;

    cos_theta = Ph3Calc_Cos(theta);
    sin_theta = Ph3Calc_Sin(theta);
    cos_neg_theta = Ph3Calc_Cos(-theta);
    sin_neg_theta = Ph3Calc_Sin(-theta);

    data->d_pos = data->alfa_pos * cos_theta + data->beta_pos * sin_theta;
    data->q_pos = (-1.0f * data->alfa_pos * sin_theta) + data->beta_pos * cos_theta;
    data->d_neg = data->alfa_neg * cos_neg_theta + data->beta_neg * sin_neg_theta;
    data->q_neg = (-1.0f * data->alfa_neg * sin_neg_theta) + data->beta_neg * cos_neg_theta;
}

```

---

## algo/ph3_calc/ph3_calc_rms.h

```c
/*
 * ph3_calc_rms.h
 *
 * 三相有效值计算
 */
#ifndef ALGO_PH3_CALC_RMS_H_
#define ALGO_PH3_CALC_RMS_H_

#include "ph3_calc_def.h"

void Ph3Calc_RmsStateInit(Ph3Calc_RmsState_t* state);
void Ph3Calc_RmsStateRun(Ph3Calc_RmsState_t* rms_state,
                         Ph3Calc_Phase_t* phase);

#endif /* ALGO_PH3_CALC_RMS_H_ */

```

---

## algo/ph3_calc/ph3_calc_rms.c

```c
/*
 * ph3_calc_rms.c
 *
 * 周期 RMS 计算（CPU FPU 实现，对应 RMS_Compute）
 */
#include "ph3_calc_rms.h"
#include "ph3_calc_math.h"

#pragma CODE_SECTION(Ph3Calc_RmsStateRun, ".TI.ramfunc");

void Ph3Calc_RmsStateInit(Ph3Calc_RmsState_t* state)
{
    if(state == NULL)
    {
        return;
    }

    state->count = 0.0f;
    state->vol_rms_sum = 0.0f;
    state->cur_rms_sum = 0.0f;
    state->vi_sum = 0.0f;
    state->vol_rms_input_1 = 0.0f;
    state->period_alpha_start = 1.0f;
    state->freq_cycle_time = 0.0f;
}

void Ph3Calc_RmsStateRun(Ph3Calc_RmsState_t* rms_state,
                         Ph3Calc_Phase_t* phase)
{
    if((rms_state == NULL) || (phase == NULL))
    {
        return;
    }

    rms_state->vol_rms_sum += phase->vol.inst * phase->vol.inst;
    rms_state->cur_rms_sum += phase->cur.inst * phase->cur.inst;
    rms_state->vi_sum += phase->vol.inst * phase->cur.inst;
    rms_state->count += 1.0f;

    uint16_t zero_cross = (uint16_t)((phase->vol.inst >= 0.0f) &&
                                     (rms_state->vol_rms_input_1 < 0.0f) &&
                                     (rms_state->count > PH3_CALC_RMS_COUNT_MIN));
    uint16_t timeout = (uint16_t)(rms_state->count > PH3_CALC_RMS_COUNT_MAX);

    if((zero_cross != 0U) || (timeout != 0U))
    {
        if(zero_cross != 0U)
        {
            float32 v_prev = rms_state->vol_rms_input_1;
            float32 v_curr = phase->vol.inst;
            float32 denominator = v_curr - v_prev;
            float32 alpha_end = 0.0f;
            uint16_t alpha_valid = 0U;

            if(fabsf(denominator) > 1e-6f)
            {
                alpha_end = -v_prev / denominator;
                if((alpha_end >= 0.0f) && (alpha_end <= 1.0f))
                {
                    alpha_valid = 1U;
                }
            }

            if(alpha_valid != 0U)
            {
                rms_state->count = (rms_state->count - 1.0f) + alpha_end
                                   + (1.0f - rms_state->period_alpha_start);
                rms_state->period_alpha_start = alpha_end;
            }
            else
            {
                /* 插值失败：保持整数 count，下周期不做起点修正 */
                rms_state->period_alpha_start = 1.0f;
            }
        }

        if(rms_state->count < 1.0f)
        {
            rms_state->count = 1.0f;
        }

        float32 inv_count = Ph3Calc_Div(1.0f, rms_state->count);
        phase->vol.rms = Ph3Calc_Sqrt(rms_state->vol_rms_sum * inv_count);
        phase->cur.rms = Ph3Calc_Sqrt(rms_state->cur_rms_sum * inv_count);
        phase->pwr.pwr_act = rms_state->vi_sum * inv_count;
        phase->pwr.pwr_app = phase->vol.rms * phase->cur.rms;

        rms_state->freq_cycle_time = rms_state->count * PH3_CALC_TS;
        rms_state->vol_rms_sum = 0.0f;
        rms_state->cur_rms_sum = 0.0f;
        rms_state->vi_sum = 0.0f;
        rms_state->count = 0.0f;
    }

    rms_state->vol_rms_input_1 = phase->vol.inst;
}

```

---

## algo/ph3_calc/ph3_calc_power.h

```c
/*
 * ph3_calc_power.h
 *
 * 三相有功/无功/视在功率及功率因数
 */
#ifndef ALGO_PH3_CALC_POWER_H_
#define ALGO_PH3_CALC_POWER_H_

#include "ph3_calc_def.h"

void Ph3Calc_PowerUpdateFactor(Ph3Calc_Channel_t* ch);
void Ph3Calc_PowerUpdateReactive(Ph3Calc_Channel_t* ch);

#endif /* ALGO_PH3_CALC_POWER_H_ */

```

---

## algo/ph3_calc/ph3_calc_power.c

```c
/*
 * ph3_calc_power.c
 *
 * 功率因数、无功功率及合相功率（CPU FPU 实现）
 */
#include "ph3_calc_power.h"
#include "ph3_calc_math.h"

#pragma CODE_SECTION(Ph3Calc_PowerUpdateFactor, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_PowerUpdateReactive, ".TI.ramfunc");

static float32 Ph3Calc_WrapThetaDiff(float32 theta_diff)
{
    if(theta_diff > PH3_CALC_PI)
    {
        theta_diff -= 2.0f * PH3_CALC_PI;
    }
    else if(theta_diff < (-PH3_CALC_PI))
    {
        theta_diff += 2.0f * PH3_CALC_PI;
    }
    return theta_diff;
}

static float32 Ph3Calc_CalcReactive(float32 pwr_app, float32 pwr_act, float32 power_refactor)
{
    float32 pwr_rea;

    pwr_rea = Ph3Calc_Sqrt((pwr_app * pwr_app) - (pwr_act * pwr_act));
    if(power_refactor < 0.0f)
    {
        pwr_rea = -pwr_rea;
    }
    return pwr_rea;
}

void Ph3Calc_PowerUpdateFactor(Ph3Calc_Channel_t* ch)
{
    float32 theta_diff;

    if(ch == NULL)
    {
        return;
    }

    if(ch->pf_mode == PH3_CALC_PF_MODE_INV)
    {
        theta_diff = (ch->volt_pll.theta_cos - (ch->curr_pll.theta_cos + PH3_CALC_PI)) * -1.0f;
    }
    else
    {
        theta_diff = ch->volt_pll.theta_cos - ch->curr_pll.theta_cos;
    }

    theta_diff = Ph3Calc_WrapThetaDiff(theta_diff);
    ch->theta_diff = theta_diff;
    ch->power_factor = Ph3Calc_Cos(theta_diff);
    ch->power_refactor = Ph3Calc_Sin(theta_diff);
}

void Ph3Calc_PowerUpdateReactive(Ph3Calc_Channel_t* ch)
{
    if(ch == NULL)
    {
        return;
    }

    /*
     * 无功 Q 计算方式一（备选）：
     *   Q = sin(theta_diff) * S = power_refactor * pwr_app
     *
     * 无功 Q 计算方式二（当前采用）：
     *   Q = sqrt(S^2 - P^2)
     *   再根据电压电流相位差定正负：power_refactor < 0 时取反
     */
    /* ch->phase_a.pwr.pwr_rea = ch->power_refactor * ch->phase_a.pwr.pwr_app; */
    /* ch->phase_b.pwr.pwr_rea = ch->power_refactor * ch->phase_b.pwr.pwr_app; */
    /* ch->phase_c.pwr.pwr_rea = ch->power_refactor * ch->phase_c.pwr.pwr_app; */
    ch->phase_a.pwr.pwr_rea = Ph3Calc_CalcReactive(ch->phase_a.pwr.pwr_app,
                                                   ch->phase_a.pwr.pwr_act,
                                                   ch->power_refactor);
    ch->phase_b.pwr.pwr_rea = Ph3Calc_CalcReactive(ch->phase_b.pwr.pwr_app,
                                                   ch->phase_b.pwr.pwr_act,
                                                   ch->power_refactor);
    ch->phase_c.pwr.pwr_rea = Ph3Calc_CalcReactive(ch->phase_c.pwr.pwr_app,
                                                   ch->phase_c.pwr.pwr_act,
                                                   ch->power_refactor);

    ch->phase_a.pwr.pf = ch->power_factor;
    ch->phase_b.pwr.pf = ch->power_factor;
    ch->phase_c.pwr.pf = ch->power_factor;

    ch->total_pwr_act = ch->phase_a.pwr.pwr_act + ch->phase_b.pwr.pwr_act + ch->phase_c.pwr.pwr_act;
    ch->total_pwr_rea = ch->phase_a.pwr.pwr_rea + ch->phase_b.pwr.pwr_rea + ch->phase_c.pwr.pwr_rea;
    ch->total_pwr_app = ch->phase_a.pwr.pwr_app + ch->phase_b.pwr.pwr_app + ch->phase_c.pwr.pwr_app;
}

```

---

## algo/ph3_calc/ph3_calc_filter.h

```c
/*
 * ph3_calc_filter.h
 *
 * 块累加平均滤波（满窗后输出均值并清零累加器）
 */
#ifndef ALGO_PH3_CALC_FILTER_H_
#define ALGO_PH3_CALC_FILTER_H_

#include "ph3_calc_def.h"

#define PH3_CALC_FILTER_DEFAULT_WIN  400U

typedef struct ph3_calc_filter_stru_t
{
    float32 sum;
    uint32_t count;
    uint32_t window_size;
    float32 output;
} Ph3Calc_Filter_t;

void Ph3Calc_Filter_Init(Ph3Calc_Filter_t *filter, uint32_t window_size);
void Ph3Calc_Filter_Reset(Ph3Calc_Filter_t *filter);
float32 Ph3Calc_Filter_Run(Ph3Calc_Filter_t *filter, float32 sample);
float32 Ph3Calc_Filter_GetOutput(const Ph3Calc_Filter_t *filter);

#endif /* ALGO_PH3_CALC_FILTER_H_ */

```

---

## algo/ph3_calc/ph3_calc_filter.c

```c
/*
 * ph3_calc_filter.c
 *
 * 块累加平均滤波（满窗后输出均值并清零累加器）
 */
#include "ph3_calc_filter.h"
#include "ph3_calc_math.h"

void Ph3Calc_Filter_Init(Ph3Calc_Filter_t *filter, uint32_t window_size)
{
    if(filter == NULL)
    {
        return;
    }

    filter->sum = 0.0f;
    filter->count = 0U;
    filter->window_size = (window_size > 0U) ? window_size : PH3_CALC_FILTER_DEFAULT_WIN;
    filter->output = 0.0f;
}

void Ph3Calc_Filter_Reset(Ph3Calc_Filter_t *filter)
{
    if(filter == NULL)
    {
        return;
    }

    filter->sum = 0.0f;
    filter->count = 0U;
}

float32 Ph3Calc_Filter_Run(Ph3Calc_Filter_t *filter, float32 sample)
{
    if(filter == NULL)
    {
        return 0.0f;
    }

    filter->count++;
    filter->sum += sample;

    if(filter->count >= filter->window_size)
    {
        filter->output = Ph3Calc_Div(filter->sum, (float32)filter->count);
        filter->sum = 0.0f;
        filter->count = 0U;
    }

    return filter->output;
}

float32 Ph3Calc_Filter_GetOutput(const Ph3Calc_Filter_t *filter)
{
    if(filter == NULL)
    {
        return 0.0f;
    }

    return filter->output;
}

```

---

## algo/ph3_calc/ph3_calc_interface.h

```c
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

```

---

## algo/ph3_calc/ph3_calc_interface.c

```c
/*
 * ph3_calc_interface.c
 *
 *
 */
#include "ph3_calc_interface.h"

#pragma CODE_SECTION(Ph3Calc_InterfacePosNegUpdate, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_InterfaceAngleUpdate, ".TI.ramfunc");

static void Ph3Calc_InterfacePosNegUpdate(Ph3Calc_IfcPosNeg_t* ifc_posneg,
                                          const Ph3Calc_PosNegData_t* posneg)
{
    if((ifc_posneg == NULL) || (posneg == NULL))
    {
        return;
    }

    ifc_posneg->alfa_pos = posneg->alfa_pos;
    ifc_posneg->beta_pos = posneg->beta_pos;
    ifc_posneg->alfa_neg = posneg->alfa_neg;
    ifc_posneg->beta_neg = posneg->beta_neg;
    ifc_posneg->d_pos = posneg->d_pos;
    ifc_posneg->q_pos = posneg->q_pos;
    ifc_posneg->d_neg = posneg->d_neg;
    ifc_posneg->q_neg = posneg->q_neg;
}

void Ph3Calc_InterfaceInit(Ph3Calc_Interface_t* ifc)
{
    if(ifc == NULL)
    {
        return;
    }

    ifc->vol.a_phase_instant = 0.0f;
    ifc->vol.b_phase_instant = 0.0f;
    ifc->vol.c_phase_instant = 0.0f;
    ifc->vol.a_phase_rms = 0.0f;
    ifc->vol.b_phase_rms = 0.0f;
    ifc->vol.c_phase_rms = 0.0f;
    ifc->vol.freq_a = 0.0f;
    ifc->vol.freq_b = 0.0f;
    ifc->vol.freq_c = 0.0f;
    ifc->vol.theta = 0.0f;

    ifc->curr.a_phase_instant = 0.0f;
    ifc->curr.b_phase_instant = 0.0f;
    ifc->curr.c_phase_instant = 0.0f;
    ifc->curr.a_phase_rms = 0.0f;
    ifc->curr.b_phase_rms = 0.0f;
    ifc->curr.c_phase_rms = 0.0f;
    ifc->curr.freq_a = 0.0f;
    ifc->curr.freq_b = 0.0f;
    ifc->curr.freq_c = 0.0f;
    ifc->curr.theta = 0.0f;

    ifc->power.a_phase_active = 0.0f;
    ifc->power.a_phase_reactive = 0.0f;
    ifc->power.a_phase_apperant = 0.0f;
    ifc->power.b_phase_active = 0.0f;
    ifc->power.b_phase_reactive = 0.0f;
    ifc->power.b_phase_apperant = 0.0f;
    ifc->power.c_phase_active = 0.0f;
    ifc->power.c_phase_reactive = 0.0f;
    ifc->power.c_phase_apperant = 0.0f;
    ifc->power.total_active = 0.0f;
    ifc->power.total_reactive = 0.0f;
    ifc->power.total_apperant = 0.0f;
    ifc->power.power_factor = 0.0f;

    ifc->vol_a_phase_count = 0.0f;
    ifc->vol_b_phase_count = 0.0f;
    ifc->vol_c_phase_count = 0.0f;

    ifc->volt_posneg.alfa_pos = 0.0f;
    ifc->volt_posneg.beta_pos = 0.0f;
    ifc->volt_posneg.alfa_neg = 0.0f;
    ifc->volt_posneg.beta_neg = 0.0f;
    ifc->volt_posneg.d_pos = 0.0f;
    ifc->volt_posneg.q_pos = 0.0f;
    ifc->volt_posneg.d_neg = 0.0f;
    ifc->volt_posneg.q_neg = 0.0f;
    ifc->curr_posneg = ifc->volt_posneg;
}

static inline float normalize_angle(float theta)
{
    while (theta >= PH3_CALC_M_2PI)
    {
        theta -= PH3_CALC_M_2PI;
    }
    while (theta < 0.0f)
    {
        theta += PH3_CALC_M_2PI;
    }
    return theta;
}

void Ph3Calc_InterfaceAngleUpdate(Ph3Calc_Interface_t* ifc, const Ph3Calc_Channel_t* ch)
{
    if((ifc == NULL) || (ch == NULL))
    {
        return;
    }

//    ifc->vol.theta = ch->volt_pll.theta_cos;
//    ifc->curr.theta = ch->curr_pll.theta_cos;
    ifc->vol.theta = normalize_angle(ch->volt_pll.theta_sin);
    ifc->curr.theta = normalize_angle(ch->curr_pll.theta_sin);

}

void Ph3Calc_InterfaceUpdate(Ph3Calc_Interface_t* ifc, const Ph3Calc_Channel_t* ch)
{
    if((ifc == NULL) || (ch == NULL))
    {
        return;
    }

    ifc->vol.a_phase_instant = ch->phase_a.vol.inst;
    ifc->vol.b_phase_instant = ch->phase_b.vol.inst;
    ifc->vol.c_phase_instant = ch->phase_c.vol.inst;
    ifc->vol.a_phase_rms = ch->phase_a.vol.rms;
    ifc->vol.b_phase_rms = ch->phase_b.vol.rms;
    ifc->vol.c_phase_rms = ch->phase_c.vol.rms;
    ifc->vol.freq_a = ch->freq_a;
    ifc->vol.freq_b = ch->freq_b;
    ifc->vol.freq_c = ch->freq_c;

    ifc->curr.a_phase_instant = ch->phase_a.cur.inst;
    ifc->curr.b_phase_instant = ch->phase_b.cur.inst;
    ifc->curr.c_phase_instant = ch->phase_c.cur.inst;
    ifc->curr.a_phase_rms = ch->phase_a.cur.rms;
    ifc->curr.b_phase_rms = ch->phase_b.cur.rms;
    ifc->curr.c_phase_rms = ch->phase_c.cur.rms;
    ifc->curr.freq_a = ch->freq_a;
    ifc->curr.freq_b = ch->freq_b;
    ifc->curr.freq_c = ch->freq_c;

    ifc->power.a_phase_active = ch->phase_a.pwr.pwr_act;
    ifc->power.a_phase_reactive = ch->phase_a.pwr.pwr_rea;
    ifc->power.a_phase_apperant = ch->phase_a.pwr.pwr_app;
    ifc->power.b_phase_active = ch->phase_b.pwr.pwr_act;
    ifc->power.b_phase_reactive = ch->phase_b.pwr.pwr_rea;
    ifc->power.b_phase_apperant = ch->phase_b.pwr.pwr_app;
    ifc->power.c_phase_active = ch->phase_c.pwr.pwr_act;
    ifc->power.c_phase_reactive = ch->phase_c.pwr.pwr_rea;
    ifc->power.c_phase_apperant = ch->phase_c.pwr.pwr_app;
    ifc->power.total_active = ch->total_pwr_act;
    ifc->power.total_reactive = ch->total_pwr_rea;
    ifc->power.total_apperant = ch->total_pwr_app;
    ifc->power.power_factor = ch->power_factor;

    ifc->vol_a_phase_count = ch->rms_a.count;
    ifc->vol_b_phase_count = ch->rms_b.count;
    ifc->vol_c_phase_count = ch->rms_c.count;

    Ph3Calc_InterfacePosNegUpdate(&ifc->volt_posneg, &ch->volt_posneg);
    Ph3Calc_InterfacePosNegUpdate(&ifc->curr_posneg, &ch->curr_posneg);
}

```

---

## algo/cla_control.h

```c
/*
 * cla_control.h
 *
 * CLA 控制相关声明（算法在 cla_control.cla 中实现）
 */
#ifndef MIDDLEWARE_CLA_ALGO_H_
#define MIDDLEWARE_CLA_ALGO_H_

#include "F28x_Project.h"

void CLA_Task(void);

#endif /* MIDDLEWARE_CLA_ALGO_H_ */

```

---

## algo/cla_control.cla

```c
/**
  ******************************************************************************
  * @file        cla_control.cla
  * @brief       CLA 任务入口（空实现，预留后续算法）
  ******************************************************************************
  */
#include "F28x_Project.h"

static void ClearClaInterrupt(void)
{
//    EPwm10Regs.ETCLR.bit.INT = 1;
//    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

void CLA_Task(void)
{
    ClearClaInterrupt();
}

__attribute__((interrupt)) void Cla1Task1(void)
{
    CLA_Task();
}

__attribute__((interrupt)) void Cla1Task2(void)
{
}

__attribute__((interrupt)) void Cla1Task3(void)
{
}

__attribute__((interrupt)) void Cla1Task4(void)
{
}

__attribute__((interrupt)) void Cla1Task5(void)
{
}

__attribute__((interrupt)) void Cla1Task6(void)
{
}

__attribute__((interrupt)) void Cla1Task7(void)
{
}

__attribute__((interrupt)) void Cla1Task8(void)
{
}

```

---

## algo/cla_cpu_share.h

```c
/*
 * cla_cpu_share.h
 *
 * CPU �� CLA �������ݽӿڣ���ǰ�޹��������Ԥ����չ��
 */
#ifndef APP_CLA_CPU_SHARE_H_
#define APP_CLA_CPU_SHARE_H_

#include "F28x_Project.h"

#endif /* APP_CLA_CPU_SHARE_H_ */

```

---

## algo/cla_cpu_share.c

```c
/*
 * cla_cpu_share.c
 *
 * CPU �� CLA �������ݶ��壨��ǰ�޹��������
 */
#include "cla_cpu_share.h"

```
