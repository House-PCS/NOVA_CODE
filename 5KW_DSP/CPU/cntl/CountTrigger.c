//
// Created by Nova004693 on 24-6-20.
//

#include "CountTrigger.h"

void CountTriggerInit(CountTrigger_t* p_trigger, uint32_t threshold)
{
    if(p_trigger == 0)
    {
        return;
    }
    p_trigger->trig_threshold = threshold;
    p_trigger->trig_hook = 0;
    p_trigger->trigged = 0;
    p_trigger->count_interval = 1;
}

void CountTriggerReset(CountTrigger_t* p_trigger)
{
    if(p_trigger == 0)
    {
        return;
    }
    p_trigger->trigged = 0;
    p_trigger->count = 0;
}

#pragma CODE_SECTION(CountTriggerRun,".TI.ramfunc");
void CountTriggerRun(CountTrigger_t* p_trigger, uint8_t value)
{
    if(p_trigger == 0)
    {
        return;
    }
    if(value == 1)
        p_trigger->count += p_trigger->count_interval;
    else if(value == 0)
        p_trigger->count -= p_trigger->count_interval;

    if(p_trigger->count < 0)
        p_trigger->count = 0;
    if(p_trigger->count >= p_trigger->trig_threshold)
    {
        p_trigger->trigged = 1;
        if(p_trigger->trig_hook != 0)
        {
            p_trigger->trig_hook(p_trigger->hook_param);
        }
        p_trigger->count = 0;
    }
}
