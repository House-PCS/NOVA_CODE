//
// Created by Nova004693 on 24-6-21->
//

#include <math.h>
#include "rms.h"

#include "voltagesourceinvlcfltr.h"

#define ANALY_FREQ      (ISR_CONTROL_FREQUENCY)
#define ANALY_DELTA     10

#define RMSCALCDEF(v)\
{\
    if(v->vin > v->threshold)\
    {\
        v->curr_sample_norm = v->vin;\
        v->curr_sign = 1;\
    }\
    else\
    {\
        v->curr_sample_norm = fabsf(v->vin);\
        v->curr_sign = 0;\
    }\
    \
    if((v->prev_sign != v->curr_sign) && (v->curr_sign == 1))\
    {\
        v->zcd_count++;\
        if(v->nsamples_min < v->nsamples && v->nsamples < v->nsamples_max)\
        {\
            v->inv_sqrt_nsamples = (1.0) * __eisqrtf32(v->nsamples);\
            v->vrms = (sqrt(v->vacc_rms) * v->inv_sqrt_nsamples);\
            v->sig_freq = (v->sample_freq / v->nsamples) * v->zcd_count;\
            v->prev_sign = v->curr_sign;\
            v->vacc_rms = 0;\
            v->nsamples = 0;\
            v->zcd_count=0;\
        }\
    }\
    if(v->nsamples > v->nsamples_max)\
    {\
        v->vrms = 0;\
        v->sig_freq = 0;\
        v->vacc_avg = 0;\
        v->vacc_rms = 0;\
        v->nsamples = 0;\
    }\
    v->nsamples = v->nsamples + 1;\
    v->vacc_rms = v->vacc_rms + (v->curr_sample_norm * v->curr_sample_norm);\
    v->prev_sign = v->curr_sign;\
}

#pragma CODE_SECTION(RmsCalcStep,".TI.ramfunc");
float32 RmsCalcStep(Rms_t* v, float32 input)
{
    v->vin = input;
    RMSCALCDEF(v);
    return v->vrms;
}

void RmsInit50Hz(Rms_t* v, uint16_t prescale)
{
    v->sample_freq = ANALY_FREQ / prescale;
    v->threshold = 0;
    v->nsamples_max = v->sample_freq / (50 - ANALY_DELTA) * 5;
    v->nsamples_min = v->sample_freq / (50 + ANALY_DELTA) * 5;
}
