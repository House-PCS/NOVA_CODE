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
