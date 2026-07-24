//
// Created by Nova007156 on 24-11-22.
//

#ifndef PWR_COMPUTE_H
#define PWR_COMPUTE_H
#include "F2837xD_device.h"

typedef struct pwr_compute_stru_t
{
    float vol_in;                // Input: Sine Signal
    float vol_in_last;
    float cur_in;                // Input: Sine Signal
    float cur_in_last;

    float vol_rms;            // Output: RMS Value
    float vol_avg;            // Output: Average Value
    float vol_ema;            // Output: Exponential Moving Average Value
    float cur_rms;            // Output: RMS Value
    float cur_avg;            // Output: Average Value
    float cur_ema;            // Output: Exponential Moving Average Value
    float pwr_active_out;
    float pwr_reactive_out;
    float pwr_apperant_out;

    // internal variables
    float vol_acc_avg;
    float vol_acc_rms;
    float vol_acc_ema;
    float vol_acc_rms_sqrt;
    float cur_acc_avg;
    float cur_acc_rms;
    float cur_acc_ema;
    float cur_acc_rms_sqrt;
    float pwr_acc_avg;
    float pwr_acc_rms;
    float pwr_acc_ema;
    float pwr_acc_reactive;
    float pwr_acc_reactive_square;
    float vol_sample_norm;    // normalized value of current sample
    float cur_sample_norm;    // normalized value of current sample
    float pwr_sample_norm;    // normalized value of current sample

    float sample_freq;        // Input: Signal Sampling Freq
    float threshold;        // Input: Voltage level corresponding to zero i/p
    float sig_freq;            // Output: Signal Freq
    float inv_nsamples;
    float inv_sqrt_nsamples;
    float max;

    uint8_t zcd;                // Output: Zero Cross detected
    uint8_t zcd_count;                // Output: Zero Cross detected
    uint8_t prev_sign;
    uint8_t curr_sign;
    uint32_t nsamples;            // samples in half cycle input waveform
    uint32_t nsamples_min;
    uint32_t nsamples_max;
    uint32_t max_freq;
    uint32_t compute_complete;
    int32_t sign; //超前为-1.滞后为1
} PwrCompute_t;

void RmsPwrInit50Hz(PwrCompute_t* data, uint16_t prescale);
#endif //RTTHREAD_RMS_H

