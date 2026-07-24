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
