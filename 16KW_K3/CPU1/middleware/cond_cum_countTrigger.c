/**
 * @file cond_cum_countTrigger.c
 * @brief Conditional cumulative count trigger
 */

#include "cond_cum_countTrigger.h"

void CondCum_CountTrigger_Init(CondCum_CountTrigger_t* trigger, Uint32 threshold)
{
    if(trigger == 0) {
        return;
    }

    trigger->count = 0;
    trigger->count_interval = 1;
    trigger->trig_threshold = threshold;
    trigger->trigged = 0U;
    trigger->trig_hook = 0;
    trigger->hook_param = 0;
    trigger->mode = COND_CUM_COUNT_TRIGGER_MODE_DECREASE;
}

void CondCum_CountTrigger_Reset(CondCum_CountTrigger_t* trigger)
{
    if(trigger == 0) {
        return;
    }

    trigger->trigged = 0U;
    trigger->count = 0;
}

#pragma CODE_SECTION(CondCum_CountTrigger_Run, ".TI.ramfunc");
void CondCum_CountTrigger_Run(CondCum_CountTrigger_t* trigger, Uint16 value)
{
    if(trigger == 0) {
        return;
    }

    if(value == 1U) {
        trigger->count += trigger->count_interval;
    } else if(value == 0U) {
        if(trigger->mode == COND_CUM_COUNT_TRIGGER_MODE_CLEAR) {
            trigger->count = 0;
        } else {
            trigger->count -= trigger->count_interval;
        }
    }

    if(trigger->count < 0) {
        trigger->count = 0;
    }

    if(trigger->count >= (int32)trigger->trig_threshold) {
        trigger->trigged = 1U;
        if(trigger->trig_hook != 0) {
            trigger->trig_hook(trigger->hook_param);
        }
        trigger->count = 0;
    }
}
