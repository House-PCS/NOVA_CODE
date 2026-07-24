#include "pwm_cntl.h"

#include "algo_info.h"
#include "F2837xD_device.h"
#include <voltagesourceinvlcfltr.h>
#include <algo_err_check.h>
#include "activation.h"
#include "mppt_process.h"
#include "pwm_cntl.h"
#include "generator.h"

#define RELAY_MAIN              87
#define RELAY_LOAD              88
#define RELAY_GRID              89
#define RELAY_ENGINE            90
#define RELAY_GRID_RECTIFY      91
#define RELAY_ENGINE_RECTIFY    92
#define INV_DUTY_MAX 0.98
#define INV_DUTY_MIN 0.01

#define RELAY_ACTIVE(RLY,value) GPIO_WritePin((RLY), value)

static const AlgoInfo_t* _gAlgoInfo;
float gLlcPwmDuty[2], gLlcDegDuty, gInvPwmDuty, _gPv1PwmDuty, _gPv2PwmDuty;
uint16_t gIsInvCurLimitOverCount = 0;
static uint16_t _gCurLimitCount = 0;

void LlcPwmStop()
{
    EALLOW;

    // if((GetGridToGenFinish() == 1)//发电机模式下忽略这两个故障封波
    //     &&((gAlgoErrDefGetInvVoltOver() == 1) || (gAlgoErrDefGetCmpssCurOver() == 1)))
    // {
    //     //发电机模式下这个不封波切换状态
    // }
    // else
    // {
        EPwm6Regs.CMPA.bit.CMPA = 0.98 * LLCDutyMax;
        EPwm6Regs.CMPB.bit.CMPB = 0.02 * LLCDutyMax;
        EPwm7Regs.CMPB.bit.CMPB = 0.98 * LLCDutyMax;
        EPwm7Regs.CMPA.bit.CMPA = 0.02 * LLCDutyMax;

        EPwm8Regs.CMPA.bit.CMPA = 0.98 * LLCDutyMax;
        EPwm8Regs.CMPB.bit.CMPB = 0.02 * LLCDutyMax;
        EPwm9Regs.CMPB.bit.CMPB = 0.98 * LLCDutyMax;
        EPwm9Regs.CMPA.bit.CMPA = 0.02 * LLCDutyMax;

        EPwm6Regs.TZFRC.bit.OST = 1;
        EPwm7Regs.TZFRC.bit.OST = 1;
        EPwm8Regs.TZFRC.bit.OST = 1;
        EPwm9Regs.TZFRC.bit.OST = 1;
    // }
    EDIS;
}

void LlcPwmUpdate(float duty0, float duty1)
{
    if(duty0 == 0 || duty1 == 0)
    {
        LlcPwmStop();
        EPwm6Regs.CMPA.bit.CMPA = 100;
        EPwm7Regs.CMPA.bit.CMPA = 100;
        EPwm8Regs.CMPA.bit.CMPA = 100;
        EPwm9Regs.CMPA.bit.CMPA = 100;
        return;
    }

    EALLOW;
    EPwm6Regs.CMPA.bit.CMPA = LLCDutyMax * duty0;
    EPwm7Regs.CMPA.bit.CMPA = LLCDutyMax * duty0;
    EPwm8Regs.CMPA.bit.CMPA = LLCDutyMax * duty1;
    EPwm9Regs.CMPA.bit.CMPA = LLCDutyMax * duty1;
    EDIS;

    EALLOW;
    EPwm6Regs.TZCLR.bit.OST = 1;
    EPwm7Regs.TZCLR.bit.OST = 1;
    EPwm8Regs.TZCLR.bit.OST = 1;
    EPwm9Regs.TZCLR.bit.OST = 1;
    EDIS;
}

void LlcDegUpdate(float duty)
{
    EPwm7Regs.TBPHS.bit.TBPHS = LLCDutyMax * duty;
    EPwm9Regs.TBPHS.bit.TBPHS = LLCDutyMax * duty;
}

void InvPwmStop()
{
    invDuty = 0;
    EALLOW;
    ePWM[2]->TZFRC.bit.OST = 1;
    ePWM[2 + 1]->TZFRC.bit.OST = 1;
    EDIS;
}
void InvPwmCurrLimit(void)
{
    if((Read_cmpss_status()) && (_gAlgoInfo->inv_state != EN_INV_STATE_DEF_CONNECTED) && (_gAlgoInfo->inv_state != EN_INV_STATE_DEF_DISCONNECTING))
    {
        if(_gCurLimitCount < 20000)//100000/50 = 2000次  100ms
        {
            _gCurLimitCount += 10;
            //开波
            Clear_cmpss_ost();
            //EPwm2Regs.TZCLR.bit.OST = 1;
            //EPwm3Regs.TZCLR.bit.OST = 1;
        }
        else
        {
            //封波不做处理
        }
    }
    else
    {
        if(_gCurLimitCount > 1)
        {
            _gCurLimitCount --;
        }
    }
    if(_gCurLimitCount >= 20000)
    {
        gIsInvCurLimitOverCount = 1;
    }
}
void ResetInvPwmCurrLimit(void)
{
    gIsInvCurLimitOverCount = 0;
    _gCurLimitCount = 0;
}

void InvPwmUpdate(float duty)
{
    if(duty == 0)
    {
        InvPwmStop();
        return;
    }

    int16_t inv_duty = _IQ24mpy(ePWM[2]->TBPRD, (1 - _IQ24abs(duty)));

    if(inv_duty >= InvDutyMax)
    {
        inv_duty = InvDutyMax;
    }
    if(inv_duty <= PWM_DUTY_CUT)
    {
        inv_duty = PWM_DUTY_CUT;
    }
    EALLOW;

    ePWM[2]->CMPA.bit.CMPA = inv_duty;
    ePWM[2]->CMPB.bit.CMPB = inv_duty;
    ePWM[2 + 1]->CMPA.bit.CMPA = inv_duty;
    ePWM[2 + 1]->CMPB.bit.CMPB = inv_duty;

    // wait for the PWM to start counting down
    // if(EPwm1Regs.TBSTS.bit.CTRDIR == 0) // make sure the PWM is counting down
    // {
        if(duty < 0)
        {
            ePWM[2]->AQCTLA.bit.CAU = AQ_SET; // CTR = CMPA@UP , set to 1
            ePWM[2]->AQCTLA.bit.CAD = AQ_CLEAR; // CTR = CMPA@Down , toggle
            ePWM[2]->AQCSFRC.bit.CSFA = AQ_NO_ACTION;
            ePWM[2 + 1]->AQCSFRC.bit.CSFA = AQ_CLEAR;
        }
        else
        {
            ePWM[2]->AQCSFRC.bit.CSFA = AQ_CLEAR;
            ePWM[2 + 1]->AQCSFRC.bit.CSFA = AQ_NO_ACTION;
            ePWM[2 + 1]->AQCTLA.bit.CAU = AQ_SET; // CTR = CMPA@UP , set to 1
            ePWM[2 + 1]->AQCTLA.bit.CAD = AQ_CLEAR; // CTR = CMPA@Down , toggle
        }
    // }
    if(!Read_cmpss_status())
    {
        ePWM[2]->TZCLR.bit.OST = 1;
        ePWM[2 + 1]->TZCLR.bit.OST = 1;
    }
    EDIS;

}

void Pv1PwmStop()
{
    EALLOW;
    EPwm11Regs.TZFRC.bit.OST = 1;
    EDIS;
}

void Pv1PwmUpdate(float pv1_duty, float pv2_duty)
{
    if((pv1_duty == 0) && (pv2_duty == 0))
    {
        Pv1PwmStop();
        return;
    }

    EPwm11Regs.CMPA.bit.CMPA = LLC_PWM_PERIOD / 2 * pv1_duty;
    EPwm11Regs.CMPB.bit.CMPB = LLC_PWM_PERIOD / 2 * (1 - pv2_duty);

    if(gPvOvrCurRecoverCount >= PV_RECOVER_COUNT)
    {
        EALLOW;
        EPwm11Regs.TZCLR.bit.OST = 1;
        EDIS;
    }
}

void RelayUpdate()
{
    RELAY_ACTIVE(RELAY_MAIN, _gAlgoInfo->relay.rly_main);
    RELAY_ACTIVE(RELAY_LOAD, _gAlgoInfo->relay.rly_load);
    RELAY_ACTIVE(RELAY_GRID, _gAlgoInfo->relay.rly_grid);
    RELAY_ACTIVE(RELAY_ENGINE, _gAlgoInfo->relay.rly_engine);
    RELAY_ACTIVE(RELAY_GRID_RECTIFY, _gAlgoInfo->relay.rly_grid_rectify);
    RELAY_ACTIVE(RELAY_ENGINE_RECTIFY, _gAlgoInfo->relay.rly_engine_rectify);
}

void PwmUpdate()
{
    if(gAlgoErr == 1)
    {
        EALLOW;
        (*ePWM[2]).TZCTL.bit.TZB = TZ_FORCE_LO;
        (*ePWM[3]).TZCTL.bit.TZB = TZ_FORCE_LO;
        EDIS;

        // if((GetGridToGenFinish() == 1) && ((gAlgoErrDefGetInvVoltOver() == 1) || (gAlgoErrDefGetCmpssCurOver() == 1)))
        // {
        //     //发电机模式下这个不封波切换状态

        // }
        // else
        // {
            LlcPwmUpdate(0, 0);
            LlcDegUpdate(0);
            LlcPwmStop();
            EPwm6Regs.CMPA.bit.CMPA = 100;
            EPwm7Regs.CMPA.bit.CMPA = 100;
            EPwm8Regs.CMPA.bit.CMPA = 100;
            EPwm9Regs.CMPA.bit.CMPA = 100;
        // }
        InvPwmStop();
        Pv1PwmStop();
    }
    else if(ActivationFsmStateRead() == EN_ACTIVATION_STATE_DEF_NORMAL)
    {
        if(_gAlgoInfo->sys_enable == 0 || gAlgoErr == 1)
        {
            LlcPwmUpdate(0, 0);
            LlcDegUpdate(0);
            InvPwmStop();
            LlcPwmStop();
            Pv1PwmStop();
            EPwm6Regs.CMPA.bit.CMPA = 100;
            EPwm7Regs.CMPA.bit.CMPA = 100;
            EPwm8Regs.CMPA.bit.CMPA = 100;
            EPwm9Regs.CMPA.bit.CMPA = 100;
        }
        else
        {
            if(_gAlgoInfo->inv_state == EN_INV_STATE_DEF_CLOSE || _gAlgoInfo->inv_state == EN_INV_STATE_DEF_ERR)
            {
                EALLOW;
                (*ePWM[2]).TZCTL.bit.TZB = TZ_FORCE_LO;
                (*ePWM[3]).TZCTL.bit.TZB = TZ_FORCE_LO;
                EDIS;
                InvPwmStop();
            }
            else
            {
                InvPwmUpdate(gInvPwmDuty);
            }
            InvPwmCurrLimit();
            LlcPwmUpdate(gLlcPwmDuty[0], gLlcPwmDuty[1]);
            LlcDegUpdate(gLlcDegDuty);
//            Pv1PwmStop();
            Pv1PwmUpdate(_gPv1PwmDuty, _gPv2PwmDuty);
        }
    }
    else
    {
        Pv1PwmUpdate(_gPv1PwmDuty, _gPv2PwmDuty);
        if(LlcRunStateRead() == EN_LLC_STATE_DEF_READY)
        {
            LlcPwmUpdate(0, 0);
            LlcDegUpdate(0);
            LlcPwmStop();
            EPwm6Regs.CMPA.bit.CMPA = 100;
            EPwm7Regs.CMPA.bit.CMPA = 100;
            EPwm8Regs.CMPA.bit.CMPA = 100;
            EPwm9Regs.CMPA.bit.CMPA = 100;

        }
        else
        {
            LlcPwmUpdate(gLlcPwmDuty[0], gLlcPwmDuty[1]);
            LlcDegUpdate(gLlcDegDuty);
        }
    }

    RelayUpdate();
}

void PwmCntlInit()
{
    _gAlgoInfo = AlgoStateGet();
}
float InvDutyLimit(float duty_inv)
{
    float duty_out = 0;
    if(duty_inv > INV_DUTY_MAX)
    {
        duty_out = INV_DUTY_MAX;
    }
    else if(duty_inv < (0.0f - INV_DUTY_MAX))
    {
        duty_out = 0.0f - INV_DUTY_MAX;
    }
    else
    {
        if((duty_inv < INV_DUTY_MIN) && (duty_inv > (0.0f - INV_DUTY_MIN)))
        {
            if(duty_inv >= 0.0f)
            {
                duty_out = INV_DUTY_MIN;
            }
            else
            {
                duty_out = 0.0f - INV_DUTY_MIN;
            }
        }
        else
        {
            duty_out = duty_inv; 
        }
    }
    return duty_out;
}
