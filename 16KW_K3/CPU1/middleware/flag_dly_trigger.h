/**
 * @file flag_dly_trigger.h
 * @brief Debounced flag trigger macros (DECLARE/RUN/RESET style)
 */

#ifndef FLAG_DLY_TRIGGER_H
#define FLAG_DLY_TRIGGER_H

#include "F28x_Project.h"
#include "cond_cum_countTrigger.h"

void FlagDly_SetHook(void* param);
void FlagDly_ClearHook(void* param);
void FlagDly_SetResetHook(void* param);
void FlagDly_ClearResetHook(void* param);

#define FLAG_DLY_SET_DECLARE(id, p_flag, threshold) \
static CondCum_CountTrigger_t _flag_dly_set_##id = { \
    .trigged = 0, \
    .count = 0, \
    .count_interval = 1, \
    .hook_param = (p_flag), \
    .trig_hook = FlagDly_SetHook, \
    .trig_threshold = (Uint32)(threshold), \
};

#define FLAG_DLY_SET_RUN(id, cmd) \
{cmd} \
CondCum_CountTrigger_Run(&_flag_dly_set_##id, rst);

#define FLAG_DLY_CLEAR_DECLARE(id, p_flag, threshold) \
static CondCum_CountTrigger_t _flag_dly_clear_##id = { \
    .trigged = 0, \
    .count = 0, \
    .count_interval = 1, \
    .hook_param = (p_flag), \
    .trig_hook = FlagDly_ClearHook, \
    .trig_threshold = (Uint32)(threshold), \
};

#define FLAG_DLY_CLEAR_RUN(id, cmd) \
{cmd} \
CondCum_CountTrigger_Run(&_flag_dly_clear_##id, rst);

#define FLAG_DLY_PAIR_DECLARE(id, p_flag, threshold) \
    FLAG_DLY_CLEAR_DECLARE(id, p_flag, threshold) \
    FLAG_DLY_SET_DECLARE(id##_recover, p_flag, threshold)

#define FLAG_DLY_SET_RESET(id, p_flag) \
    do { \
        CondCum_CountTrigger_Reset(&_flag_dly_set_##id); \
        FlagDly_SetResetHook(p_flag); \
    } while(0)

#define FLAG_DLY_CLEAR_RESET(id, p_flag) \
    do { \
        CondCum_CountTrigger_Reset(&_flag_dly_clear_##id); \
        FlagDly_ClearResetHook(p_flag); \
    } while(0)

#endif /* FLAG_DLY_TRIGGER_H */
