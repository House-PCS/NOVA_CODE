
//
// Created by Nova004693 on 24-6-21->
//

#include <math.h>
#include "pwr_compute.h"

#include "voltagesourceinvlcfltr.h"

#define ANALY_FREQ      (ISR_CONTROL_FREQUENCY)
#define ANALY_DELTA     10

#pragma CODE_SECTION(RmsPwrCalcStep,".TI.ramfunc");
void RmsPwrCalcStep(PwrCompute_t* data, float32 vol_input, float cur_input, float power_refactor)
{
    int32_t sign = 1;
    data->vol_in = vol_input;
    data->cur_in = cur_input;
    if(power_refactor < 0)
    {
        sign = -1;
    }
    
    data->sign = sign;

    data->vol_sample_norm = data->vol_in * data->vol_in;
    data->cur_sample_norm = data->cur_in * data->cur_in;
    data->pwr_sample_norm = data->vol_in * data->cur_in;
    if(data->vol_in > data->threshold)
    {
        data->curr_sign = 1;
    }
    else
    {
        data->curr_sign = 0;
    }
    
    if((data->nsamples > data->nsamples_max) || (data->compute_complete == 1))
    {
        data->pwr_acc_reactive = sqrtf(data->pwr_acc_reactive_square);
        data->sig_freq = 0;
        data->vol_acc_avg = 0;
        data->vol_acc_rms = 0;
        data->cur_acc_avg = 0;
        data->cur_acc_rms = 0;
        data->pwr_acc_avg = 0;
        data->pwr_acc_rms = 0;
        data->nsamples = 0;
        data->zcd_count= 0;
        data->compute_complete = 0;
        data->pwr_reactive_out = data->sign * sqrtf(data->pwr_acc_reactive_square);
    }
    if((data->prev_sign != data->curr_sign) && (data->curr_sign == 1) || (data->nsamples >= data->nsamples_max))
    {
        data->zcd_count++;
        if(data->nsamples_min < data->nsamples && data->nsamples <= data->nsamples_max)
        {
            data->inv_sqrt_nsamples = (1.0) * __eisqrtf32(data->nsamples);
            data->vol_acc_rms_sqrt = sqrtf(data->vol_acc_rms); 
            data->cur_acc_rms_sqrt = sqrtf(data->cur_acc_rms);
            data->sig_freq = (data->sample_freq / data->nsamples) * data->zcd_count;
            data->prev_sign = data->curr_sign;
            data->vol_rms = data->inv_sqrt_nsamples * data->vol_acc_rms_sqrt;
            data->cur_rms = data->inv_sqrt_nsamples * data->cur_acc_rms_sqrt;
            data->pwr_active_out = data->pwr_acc_rms / data->nsamples;
            data->pwr_apperant_out = data->vol_rms * data->cur_rms;
            data->pwr_acc_reactive_square = data->pwr_apperant_out * data->pwr_apperant_out - data->pwr_active_out * data->pwr_active_out;
            data->compute_complete = 1;
        }
    }
    data->nsamples = data->nsamples + 1;
    data->vol_acc_rms = data->vol_acc_rms + data->vol_sample_norm;
    data->cur_acc_rms = data->cur_acc_rms + data->cur_sample_norm;
    data->pwr_acc_rms = data->pwr_acc_rms + data->pwr_sample_norm;
    data->prev_sign = data->curr_sign;
}

void RmsPwrInit50Hz(PwrCompute_t* data, uint16_t prescale)
{
    data->sample_freq = ANALY_FREQ / prescale;
    data->threshold = 0;
    data->nsamples_max = data->sample_freq / (50 - ANALY_DELTA) * 5;
    data->nsamples_min = data->sample_freq / (50 + ANALY_DELTA) * 5;
}
