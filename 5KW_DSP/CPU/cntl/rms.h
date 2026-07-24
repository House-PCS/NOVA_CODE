//
// Created by Nova004693 on 24-6-21.
//

#ifndef RTTHREAD_RMS_H
#define RTTHREAD_RMS_H
#include "F2837xD_device.h"

typedef struct rms_stru_t
{
    float vin;                // Input: Sine Signal
    float vin_last;
    float sample_freq;        // Input: Signal Sampling Freq
    float threshold;        // Input: Voltage level corresponding to zero i/p
    float vrms;            // Output: RMS Value
    float vavg;            // Output: Average Value
    float vema;            // Output: Exponential Moving Average Value
    float sig_freq;            // Output: Signal Freq
    uint8_t zcd;                // Output: Zero Cross detected
    uint8_t zcd_count;                // Output: Zero Cross detected
    // internal variables
    float vacc_avg;
    float vacc_rms;
    float vacc_ema;
    float curr_sample_norm;    // normalized value of current sample
    uint8_t prev_sign;
    uint8_t curr_sign;
    uint32_t nsamples;            // samples in half cycle input waveform
    uint32_t nsamples_min;
    uint32_t nsamples_max;
    float inv_nsamples;
    float inv_sqrt_nsamples;
    float max;
    uint32_t max_freq;
} Rms_t;

void RmsCalc(Rms_t* v);
void RmsInit50Hz(Rms_t* v, uint16_t prescale);
float32 RmsCalcStep(Rms_t* v, float32 input);
#endif //RTTHREAD_RMS_H
