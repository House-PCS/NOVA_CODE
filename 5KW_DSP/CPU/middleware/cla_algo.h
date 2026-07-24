/*
 * cla_init.h
 *
 *  Created on: 2024年8月27日
 *      Author: Nova007456
 */
#ifndef MIDDLEWARE_CLA_ALGO_H_
#define MIDDLEWARE_CLA_ALGO_H_


#include "F28x_Project.h"
#include "F2837xD_Cla_defines.h"
#include <SPLL_1ph_F.h>

__attribute__((interrupt)) void cla1Isr1();
__attribute__((interrupt)) void cla1Isr2();
__attribute__((interrupt)) void cla1Isr3();
__attribute__((interrupt)) void cla1Isr4();
__attribute__((interrupt)) void cla1Isr5();
__attribute__((interrupt)) void cla1Isr6();
__attribute__((interrupt)) void cla1Isr7();
__attribute__((interrupt)) void cla1Isr8();

__attribute__((interrupt)) void Cla1Task1();
__attribute__((interrupt)) void Cla1Task2();
__attribute__((interrupt)) void Cla1Task3();
__attribute__((interrupt)) void Cla1Task4();
__attribute__((interrupt)) void Cla1Task5();
__attribute__((interrupt)) void Cla1Task6();
__attribute__((interrupt)) void Cla1Task7();
__attribute__((interrupt)) void Cla1Task8();

//cla_init.c中调用的函数 → 给CPU使用的声明写在这下面
void ClaInit(void);

//control.cla中定义的函数 → 给CLA使用的声明写在这下面
inline void ClaTask(void);


void PllCla(SPLL_1ph_F* v, float32 input);
extern SPLL_1ph_F gGridVolPll;
extern SPLL_1ph_F gInvVolPll;
extern SPLL_1ph_F gGridCurPll;
extern SPLL_1ph_F gInvCurPll;
extern SPLL_1ph_F gCtCurPll;
extern SPLL_1ph_F gLoadCurPll;

typedef struct rms_cla_stru_t
{
    float32 vin;                // Input: Sine Signal
    float32 vin_last;
    float32 sample_freq;        // Input: Signal Sampling Freq
    float32 threshold;        // Input: Voltage level corresponding to zero i/p
    float32 vrms;            // Output: RMS Value
    float32 vavg;            // Output: Average Value
    float32 vema;            // Output: Exponential Moving Average Value
    float32 sig_freq;            // Output: Signal Freq
    uint16_t zcd;                // Output: Zero Cross detected
    uint16_t zcd_count;                // Output: Zero Cross detected
    // internal variables
    float32 vacc_avg;
    float32 vacc_rms;
    float32 vacc_ema;
    float32 curr_sample_norm;    // normalized value of current sample
    uint16_t prev_sign;
    uint16_t curr_sign;
    uint32_t nsamples;            // samples in half cycle input waveform
    uint32_t nsamples_min;
    uint32_t nsamples_max;
    float32 inv_nsamples;
    float32 inv_sqrt_nsamples;
    float32 max;
    uint32_t max_freq;
} RmsCla_t;
typedef struct pwr_cla_compute_stru_t
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

    uint16_t zcd;                // Output: Zero Cross detected
    uint16_t zcd_count;                // Output: Zero Cross detected
    uint16_t prev_sign;
    uint16_t curr_sign;
    uint32_t nsamples;            // samples in half cycle input waveform
    uint32_t nsamples_min;
    uint32_t nsamples_max;
    uint32_t max_freq;
    uint32_t compute_complete;
    int32_t sign; //无功功率符号
} PwrClaCompute_t;

// extern RmsCla_t gGridVolAnlz;
extern RmsCla_t gGridVolPllAnlz;
// extern RmsCla_t gInvVolAnlz;
// extern RmsCla_t gLoadVolAnlz;
// extern RmsCla_t gGridCurAnlz;
// extern RmsCla_t gInvCurAnlz;
// extern RmsCla_t gLoadCurAnlz;
// extern RmsCla_t gCtCurAnlz;
extern PwrClaCompute_t gstGridAnlz;
extern PwrClaCompute_t gstInvAnlz;
extern PwrClaCompute_t gstLoadAnlz;
extern PwrClaCompute_t gstCtAnlz;

#endif
