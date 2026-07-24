/*
 * pwr_compute_cpu.c
 *
 * CPU-side implementation matching algo/pwr_compute.cla (float math via math.h).
 * Used for gstMidRelay / gstBusData RMS and power effective-value calculation off CLA.
 */

#include "pwr_compute.h"
#include "math.h"

#ifndef QVAR_COMPUTE_FUN_FACTOR_CPU
#define QVAR_COMPUTE_FUN_FACTOR_CPU 0
#endif

void RmsPwrCpuCalcStep(PwrClaCompute_t* data, float32 vol_input, float cur_input, float power_refactor)
{
    float32 sign = 1.0f;
    float32 isnsamples = 0.0f;

    data->vol_in = vol_input;
    data->cur_in = cur_input;
    if (power_refactor < 0.0f)
    {
        sign = -1.0f;
    }

    data->sign = sign;

    data->vol_sample_norm = data->vol_in * data->vol_in;
    data->cur_sample_norm = data->cur_in * data->cur_in;
    data->pwr_sample_norm = data->vol_in * data->cur_in;
    if (data->vol_in > data->threshold)
    {
        data->curr_sign = 1.0f;
    }
    else
    {
        data->curr_sign = 0.0f;
    }

    if (data->compute_complete >= 1.0f)
    {
        data->pwr_acc_reactive = sqrtf(data->pwr_acc_reactive_square);
        data->vol_acc_avg = 0.0f;
        data->vol_acc_rms = 0.0f;
        data->cur_acc_avg = 0.0f;
        data->cur_acc_rms = 0.0f;
        data->pwr_acc_rms = 0.0f;
        data->nsamples = 0.0f;
        data->zcd_count = 0.0f;
        data->compute_complete = 0.0f;
#if QVAR_COMPUTE_FUN_FACTOR_CPU
        data->pwr_reactive_out = power_refactor * data->pwr_apperant_out;
#else
        if (power_refactor > 0.0f)
        {
            data->pwr_reactive_out = data->pwr_acc_reactive;
        }
        else
        {
            data->pwr_reactive_out = -data->pwr_acc_reactive;
        }
#endif
    }

    if ((((data->prev_sign != data->curr_sign) && (data->curr_sign == 1.0f))
            || (data->nsamples >= data->nsamples_max)))
    {
        data->zcd_count = data->zcd_count + 1.0f;
        if (data->nsamples_min < data->nsamples)
        {
            isnsamples = 1.0f / data->nsamples;
            data->inv_sqrt_nsamples = 1.0f / sqrtf(data->nsamples);
            data->vol_acc_rms_sqrt = sqrtf(data->vol_acc_rms);
            data->cur_acc_rms_sqrt = sqrtf(data->cur_acc_rms);
            data->sig_freq = (data->sample_freq * isnsamples) * data->zcd_count;
            data->prev_sign = data->curr_sign;
            data->vol_rms = data->inv_sqrt_nsamples * data->vol_acc_rms_sqrt;
            data->cur_rms = data->inv_sqrt_nsamples * data->cur_acc_rms_sqrt;
            data->pwr_active_out = data->pwr_acc_rms * isnsamples;
            data->pwr_apperant_out = data->vol_rms * data->cur_rms;
            data->pwr_acc_reactive_square = data->pwr_apperant_out * data->pwr_apperant_out
                    - data->pwr_active_out * data->pwr_active_out;
            data->vol_avg = data->vol_acc_avg * isnsamples;
            data->cur_avg = data->cur_acc_avg * isnsamples;
            data->compute_complete = 2.0f;
        }
    }
    data->nsamples = data->nsamples + 1.0f;
    data->vol_acc_rms = data->vol_acc_rms + data->vol_sample_norm;
    data->cur_acc_rms = data->cur_acc_rms + data->cur_sample_norm;
    data->pwr_acc_rms = data->pwr_acc_rms + data->pwr_sample_norm;
    data->vol_acc_avg = data->vol_acc_avg + data->vol_in;
    data->cur_acc_avg = data->cur_acc_avg + data->cur_in;
    data->prev_sign = data->curr_sign;
}

void RmsPwrCpuInit(PwrClaCompute_t* data, float compute_freq, float low_freq, float high_freq, float ac_period)
{
    data->sample_freq = compute_freq;
    data->threshold = 0.0f;
    data->nsamples_max = data->sample_freq / (low_freq) * ac_period;
    data->nsamples_min = data->sample_freq / (high_freq) * ac_period;
    data->nsamples = 0.0f;
    data->vol_acc_avg = 0.0f;
    data->vol_acc_rms = 0.0f;
    data->cur_acc_avg = 0.0f;
    data->cur_acc_rms = 0.0f;
    data->pwr_acc_rms = 0.0f;
    data->zcd_count = 0.0f;
    data->compute_complete = 0.0f;
}
