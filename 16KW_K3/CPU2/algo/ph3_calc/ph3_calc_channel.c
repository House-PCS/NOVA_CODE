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
