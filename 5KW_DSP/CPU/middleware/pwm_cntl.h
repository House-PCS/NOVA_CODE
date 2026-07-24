#ifndef PWM_CNTL_H
#define PWM_CNTL_H
#include "algo_info.h"
#include "llc_dcdc_cntl.h"

extern float gLlcPwmDuty[2], gLlcDegDuty, gInvPwmDuty, _gPv1PwmDuty, _gPv2PwmDuty;
extern uint16_t gIsInvCurLimitOverCount;



inline void PwmSetLlcDuty(float duty0, float duty1)
{
    if(duty0 >= 0 && duty0 <= LLC_MAX_PWM && duty1 >= 0 && duty1 <= LLC_MAX_PWM)
    {
        gLlcPwmDuty[0] = duty0;
        gLlcPwmDuty[1] = duty1;
    }
}

inline void PwmSetLlcDegDuty(float duty)
{
    if(duty >= 0 && duty <= 1)
    {
        gLlcDegDuty = duty;
    }
}

inline void PwmSetInvDuty(float duty)
{
    if(duty >= -1 && duty < 1)
    {
        gInvPwmDuty = duty;
    }
}

inline void PwmSetpv1Duty(float duty)
{
    if(duty >= 0 && duty < 1)
    {
        _gPv1PwmDuty = duty;
    }
}
inline void PwmSetpv2Duty(float duty)
{
    if(duty >= 0 && duty < 1)
    {
        _gPv2PwmDuty = duty;
    }
}

void LlcDegUpdate(float duty);
void PwmUpdate();
void PwmCntlInit();
void InvPwmStop();
void LlcPwmStop();
void Pv1PwmStop();
float InvDutyLimit(float duty_inv);
extern void ResetInvPwmCurrLimit(void);
#endif
