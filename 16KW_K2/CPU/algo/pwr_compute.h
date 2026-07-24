#ifndef __PWR_COMPUTE_H_
#define __PWR_COMPUTE_H_
#include "F28x_Project.h"
#include "CLAmath.h"
typedef struct pwr_cla_compute_stru_t
{
    float vol_in;                // Input: Sine Signal
    float cur_in;                // Input: Sine Signal

    float vol_rms;            // Output: RMS Value
    float vol_avg;            // Output: Average Value
    float cur_rms;            // Output: RMS Value
    float cur_avg;            // Output: Average Value
    float pwr_active_out;
    float pwr_reactive_out;
    float pwr_apperant_out;

    /* internal variables (trimmed: unused vol/cur_ema, *_acc_ema, inv_nsamples, max, zcd, max_freq) */
    float vol_acc_avg;
    float vol_acc_rms;
    float vol_acc_rms_sqrt;
    float cur_acc_avg;
    float cur_acc_rms;
    float cur_acc_rms_sqrt;
    float pwr_acc_rms;
    float pwr_acc_reactive;
    float pwr_acc_reactive_square;
    float vol_sample_norm;
    float cur_sample_norm;
    float pwr_sample_norm;

    float sample_freq;
    float threshold;
    float sig_freq;
    float inv_sqrt_nsamples;

    float zcd_count;
    float prev_sign;
    float curr_sign;
    float nsamples;
    float nsamples_min;
    float nsamples_max;
    float compute_complete;
    float sign;
} PwrClaCompute_t;

void RmsPwrClaCalcStep(PwrClaCompute_t* data, float32 vol_input, float cur_input, float power_refactor);
void RmsPwrClaInit(PwrClaCompute_t* data, float compute_freq, float low_freq, float high_freq, float ac_period);

/* CPU mirror of pwr_compute.cla ¡ª see pwr_compute_cpu.c */
#if !defined(__CLA__)
void RmsPwrCpuCalcStep(PwrClaCompute_t* data, float32 vol_input, float cur_input, float power_refactor);
void RmsPwrCpuInit(PwrClaCompute_t* data, float compute_freq, float low_freq, float high_freq, float ac_period);
#endif

#endif /* __PWR_COMPUTE_H_ */
