/*
 * Fan.c
 *
 *  Created on: 2024年8月12日
 *      Author: Nova007456
 */
#include <fan.h>
#include "F28x_Project.h"
#include "epwm.h"
#include "dev_sensor.h"
#include "fault_diagnosis.h"

Fan_Strut_t _gFan;

/**
* @brief: _SetupFanPWM
* @param:
* @param: pwm_period_ticks
* @param: pwm_dbred_ticks
* @note:
*
*/
static void _SetupFanPWM(uint16_t inv_pwm_no, uint16_t pwm_period_ticks, uint16_t pwm_dbred_ticks)
{
    EALLOW;

    // PWM clock on F2837x is divided by 2
    // ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV=1
    // Deadband needs to be 0.5us => 10ns*50=500ns
    // Time Base SubModule Registers
    (*ePWM[inv_pwm_no]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;                            //增减计数
    (*ePWM[inv_pwm_no]).TBPRD = pwm_period_ticks >> 1; // PWM frequency = 1 / period    //周期确定
    (*ePWM[inv_pwm_no]).TBCTL.bit.PHSEN = TB_DISABLE;
    (*ePWM[inv_pwm_no]).TBCTL.bit.PRDLD = TB_SHADOW;
    (*ePWM[inv_pwm_no]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // sync "down-stream"
    (*ePWM[inv_pwm_no]).TBPHS.bit.TBPHS = 0;
    (*ePWM[inv_pwm_no]).TBCTR = 0;
    (*ePWM[inv_pwm_no]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
    (*ePWM[inv_pwm_no]).TBCTL.bit.CLKDIV = TB_DIV1;

    (*ePWM[inv_pwm_no]).CMPA.bit.CMPA = 0;
    (*ePWM[inv_pwm_no]).CMPB.bit.CMPB = 0;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    // to start don'tonfigure the PWM to do anything
    (*ePWM[inv_pwm_no]).AQCTLA.all = 0;
    (*ePWM[inv_pwm_no]).AQCTLB.all = 0;
    (*ePWM[inv_pwm_no]).AQCTLA.bit.CAU = AQ_SET; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no]).AQCTLA.bit.CAD = AQ_CLEAR; // CTR = CMPA@Down , toggle
    (*ePWM[inv_pwm_no]).AQCTLB.bit.CBU = AQ_CLEAR; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no]).AQCTLB.bit.CBD = AQ_SET; // CTR = CMPA@Down , toggle

    // Active high complementary PWMs - Set up the deadband
    (*ePWM[inv_pwm_no]).DBCTL.bit.IN_MODE = DBA_ALL;
    (*ePWM[inv_pwm_no]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    (*ePWM[inv_pwm_no]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
    (*ePWM[inv_pwm_no]).DBRED.all =  pwm_dbred_ticks;
    (*ePWM[inv_pwm_no]).DBFED.all =  pwm_dbred_ticks;

//    (*ePWM[inv_pwm_no]).TZSEL.bit.OSHT1 = 1;
    (*ePWM[inv_pwm_no]).TZCTL.bit.TZA = TZ_FORCE_LO;
    (*ePWM[inv_pwm_no]).TZCTL.bit.TZB = TZ_FORCE_LO;
    (*ePWM[inv_pwm_no]).TZCLR.bit.OST = 1;      //2026年2月6日新增清除TZ保护
    EDIS;
}

/**
 * @brief
 * @param FanInit
 * @param 风扇初始化
 * @return
 */
void FanInit(void)
{
    _SetupFanPWM(5, FAN_PWM_PERIOD, 0);
    InitEPwm5Gpio();
    //30 -> GPIO0
    GPIO_SetupPinMux(30, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(30, GPIO_INPUT, GPIO_PULLUP);
    _gFan.State.Speed = EN_FAN_SPEED_0_PERCENT;
    _gFan.State.Self_Check_Finish = 0;
    _gFan.Value.DutyMax = FAN_PWM_MAX;
    _gFan.Value.DutyMin = FAN_PWM_MIN;
}

/**
 * @brief
 * @param _Fan_SetCMPA
 * @param 设置占空比
 * @return
 */
static void _Fan_SetCMPA(uint16_t Value)
{
    EPwm5Regs.CMPA.bit.CMPA = (uint16_t)_gFan.Value.Duty_CMPA;
}

/**
 * @brief
 * @param FanFault
 * @param 风扇故障判断
 * @return
 */
void FanFault(void)
{
    static uint32_t faultcnt,recovercnt1,recovercnt2;
    //风扇故障,暂定不恢复
    if((gstFaultInfo.worning_code.b.fan_error == 0)
            &&(_gFan.State.Speed != EN_FAN_SPEED_0_PERCENT))
    {
        if(GPIO_ReadPin(30) == 1)
        {
            faultcnt++;
            if(faultcnt > 40000)//2S判断
            gstFaultInfo.worning_code.b.fan_error = 1;
        }
        else
        {
            if(faultcnt > 10)
                faultcnt -= 10;
        }
    }
    else
    {
        faultcnt = 0;
        recovercnt1++;
        if(GPIO_ReadPin(30) == 0)
            recovercnt2++;
        if(recovercnt1 > 20000)//1S内存在大于5000次低电平，则认为风扇恢复（1/4）
        {
            if(recovercnt2 > 5000)
                gstFaultInfo.worning_code.b.fan_error = 0;
            recovercnt1 = 0;
            recovercnt2 = 0;
        }
    }
}

/**
 * @brief
 * @param _FanState
 * @param 风扇转速状态函数
 * @return
 */
static void _FanState(void)
{
    _gFan.TemperatureMax = Value_TWO_MAX(gstSensorData.temp[4],gstSensorData.temp[5]);
    if(_gFan.State.Self_Check_Finish == 1)
    {
        switch(_gFan.State.Speed)
        {
            case EN_FAN_SPEED_0_PERCENT:
                if(_gFan.TemperatureMax >= 800.0f)
                    _gFan.State.Speed = EN_FAN_SPEED_80_PERCENT;
            break;
            case EN_FAN_SPEED_80_PERCENT:
                if(_gFan.TemperatureMax <750.0f)
                    _gFan.State.Speed = EN_FAN_SPEED_0_PERCENT;
            break;
            default:
                _gFan.State.Speed = EN_FAN_SPEED_80_PERCENT;
            break;
        }
    }
}

/**
 * @brief
 * @param _FanDuty
 * @param 输出风扇占空比及限幅
 * @return
 */
static void _FanDuty(void)
{
    switch(_gFan.State.Speed)
    {
        case EN_FAN_SPEED_0_PERCENT:
            _gFan.Value.Duty = 0.0f;
            break;
        case EN_FAN_SPEED_40_PERCENT:
            _gFan.Value.Duty = 0.4f;
            break;
        case EN_FAN_SPEED_60_PERCENT:
            _gFan.Value.Duty = 0.6f;
            break;
        case EN_FAN_SPEED_80_PERCENT:
            _gFan.Value.Duty = 0.8f;
            break;
        case EN_FAN_SPEED_100_PERCENT:
            _gFan.Value.Duty = 1.0f;
            break;
        default:
            break;
    }

    _gFan.Value.Duty_CMPA = (uint16_t)((1.0f - _gFan.Value.Duty)*FAN_PWM_MAX);
    _gFan.Value.Duty_CMPA = Value_LIMIT(_gFan.Value.Duty_CMPA,_gFan.Value.DutyMax,_gFan.Value.DutyMin);
    _Fan_SetCMPA(_gFan.Value.Duty_CMPA);
}

/**
 * @brief
 * @param _Fan_SelfCheck
 * @param 风扇自检10秒
 * @return
 */
static void _Fan_SelfCheck(void)
{
    static uint16_t checkcnt;
    if(_gFan.State.Self_Check_Finish == 0)
    {
        _gFan.State.Speed = EN_FAN_SPEED_80_PERCENT;
        if(checkcnt++ > 100)//10秒自检
        {
            checkcnt = 0;
            _gFan.State.Self_Check_Finish = 1;
            _gFan.State.Speed = EN_FAN_SPEED_0_PERCENT;
        }
    }
    else
    {
        if(gstFaultInfo.worning_code.b.fan_error == 1)
        {
            _gFan.State.Self_Check_Finish = 0;
        }
    }
}

/**
 * @brief
 * @param FanState_GetFault
 * @param 输出风扇占故障
 * @return
 */
float FanState_GetFault()
{
    return(gstFaultInfo.worning_code.b.fan_error);
}

/**
 * @brief
 * @param FanValue_GetDuty
 * @param 输出风扇占空比
 * @return
 */
float FanValue_GetDuty()
{
    return(_gFan.Value.Duty);
}

/**
 * @brief
 * @param Fan_100ms
 * @param 风扇部分总函数
 * @return
 */
void FanCntl(void)
{
    _Fan_SelfCheck();
    _FanState();
    _FanDuty();
}
