/**
 * @file flag_dly_trigger.c
 * @brief Generic debounced flag trigger hook
 */

#include "flag_dly_trigger.h"

void FlagDly_SetHook(void* param)
{
    uint8_t* flag = (uint8_t*)param;

    if(flag != 0) {
        *flag = 1U;
    }
}

void FlagDly_ClearHook(void* param)
{
    uint8_t* flag = (uint8_t*)param;

    if(flag != 0) {
        *flag = 0U;
    }
}

void FlagDly_SetResetHook(void* param)
{
    uint8_t* flag = (uint8_t*)param;

    if(flag != 0) {
        *flag = 0U;
    }
}

void FlagDly_ClearResetHook(void* param)
{
    uint8_t* flag = (uint8_t*)param;

    if(flag != 0) {
        *flag = 1U;
    }
}
