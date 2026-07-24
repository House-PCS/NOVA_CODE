/**
 * @file cond_cum_countTrigger.h
 * @brief Conditional cumulative count trigger
 */

#ifndef COND_CUM_COUNT_TRIGGER_H
#define COND_CUM_COUNT_TRIGGER_H

#include "F28x_Project.h"

typedef enum cond_cum_count_trigger_mode_e
{
    COND_CUM_COUNT_TRIGGER_MODE_DECREASE = 0,
    COND_CUM_COUNT_TRIGGER_MODE_CLEAR = 1
} CondCum_CountTriggerMode_e;

typedef struct cond_cum_count_trigger_stru_t
{
    int32 count;
    int32 count_interval;
    Uint32 trig_threshold;
    Uint16 trigged;
    void (*trig_hook)(void* param);
    void* hook_param;
    CondCum_CountTriggerMode_e mode;
} CondCum_CountTrigger_t;

void CondCum_CountTrigger_Init(CondCum_CountTrigger_t* trigger, Uint32 threshold);
void CondCum_CountTrigger_Reset(CondCum_CountTrigger_t* trigger);
void CondCum_CountTrigger_Run(CondCum_CountTrigger_t* trigger, Uint16 value);

#endif /* COND_CUM_COUNT_TRIGGER_H */
