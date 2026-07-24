#include "inverter_state_def.h"

#include <NOTCH_FLTR_F.h>
#include <SINEANALYZER_DIFF_F.h>

#include "algo_info.h"
#include "FSM.h"
#include "inverter_loop_cntl.h"
#include "pll.h"
#include "TimeTrigger.h"
#include <voltagesourceinvlcfltr.h>

#include "llc_dcdc_cntl.h"
#include <algo_err_check.h>
#include <stdio.h>

#include "CountTrigger.h"
#include "inverter_func_def.h"
#include "generator.h"
#include "hard_cmpss.h"
#include "pwm_cntl.h"
#include "modbus_analysis.h"
#include "fault_diagnosis.h"
#include"dc_injection.h"

extern FaultInfo_t gstFaultInfo;

#define RELEASE_OVER_TIME 1200000  //对应20s，release超时阈值

extern CNTL_PI_F gGiPi;                        //电流环PI控制器
extern CNTL_PI_F gGvPi;                        //电压环PI控制器
extern SINEANALYZER_DIFF_F gGvOutAnlz;         //电流环给定信号分析器
extern Pll_t gPll;                             //电网锁相环
extern DelayTrigger_t gGridConnectTrig;
extern HoldTrigger_t gIsletRunTrig;
extern TimeTrigger_t gEnIsletTrig;
extern DelayTrigger_t gDisConnectGridPhaseTrigger;
extern DelayTrigger_t gIsletCurCheckTrigger;
extern CountTrigger_t gGridRlyDelayTrigger;
extern DelayTrigger_t gIsletCheckTrig;

extern float32 garrSignSin[2];       //内部sin信号
extern float32 garrSignCos[2];       //内部cos信号
extern float32 garrStdSignSin[2];       //内部标准sin信号
extern float32 garrStdSignCos[2];       //内部标准cos信号
extern float gSignStamp;                   //内部斜坡信号
extern float gStampSpeed;            //斜坡信号自增速率
extern float gAcVolOpenLoopRatio;        //逆变电压开环系数
extern float gAcCurSet;                   //逆变电流峰峰值最大限制设置
extern float gAcVolSet;                  //逆变电压峰峰值设置
extern float gAvVolRef;
extern float gOverAcVolThreshold;        //逆变电压过压阈值
extern float gLowDcVolThreshold;         //直流母线电压欠压阈值+
extern float gOverDcVolThreshold;         //直流母线电压过压阈值
extern float gGridConnectedCurRef;         //并网电流给定
extern InverterState_t gState;                 //逆变器状态
extern uint8_t gEnGridConnect;             //使能并网

extern EN_GRID_CONNECT_STA gStaGridConnect;//并网状态
extern float gGridPhaseErr;                //相位偏差
extern float gVolPwrOffset;
extern float gAcCurRefK;

extern float gPwm;        //逆变器输出PWM占空比
extern float gPwmK;       //逆变器控制器输出比例
extern uint8_t gInvStartOk, gInvReleaseOk;

extern uint8_t gEnFreqAdjust;

extern uint8_t gInvErr;

extern const AlgoInfo_t* gAlgoInfo;
extern float gDeltaWn;
extern uint8_t gIsLet;

extern uint16_t gGridConnectedTime;
extern uint32_t gGridDisConnectedTime;
extern uint8_t gGridRlyActived;
extern uint16_t gIsletTimeCount, gEnIsletCompare, gIsletErrCount;
extern float gGridCurCheckThreshold, gGridCurCheckThresholdSum;
extern float gSignThetaDeg;
extern DelayTrigger_t gstGridVolLowTrigger;
static char _garrScidBuff[256];
uint16_t gErrResetFlag = 0;
extern CNTL_2P2Z_F_VARS gGvPrVars;

static uint16_t _ErrCheck()
{
    return gAlgoErr;
}

static void _ScidSendMsgToBuffErr(const char* format, ...)
{
    //va_list args;
    //va_start(args, format);
    //int length = strlen(_garrScidBuff);
    //vsprintf(_garrScidBuff + length, format, args);
    //va_end(args);
}

void ScidSendMsgErr()
{
    if(_garrScidBuff[0] == 0)
        return;
    int length = strlen(_garrScidBuff);
    uint8_t sum = 0x55;
    ScidSendByte(0xAA);
    ScidSendByte(0xCC);
    ScidSendByte(length + 2);
    ScidSendByte(0x55);
    for(int i = 0; i < length; ++i)
    {
        ScidSendByte(_garrScidBuff[i]);
        sum += _garrScidBuff[i];
    }
    ScidSendByte(sum);
    _garrScidBuff[0] = 0;
}

static void _ErrAction()
{
    RelaySetValue(EN_RELAY_DEF_GRID, 0);
    if(GetGridToGenFinish() == 0)//发电机模式不断开并网和负载继电器
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, 0);
        RelaySetValue(EN_RELAY_DEF_ENGINE, 0);
    }
    RelaySetValue(EN_RELAY_DEF_MAIN, 0);
    gInvStartOk = 0;
    //_ScidSendMsgToBuffErr("\n->err %04X%04X: %d", gAlgoErrDef.byte[1], gAlgoErrDef.byte[0], (int32_t)(gAlgoErrData * 1000));
}

static void _ErrActionToClose()
{
    gPwm = 0;
    gInvReleaseOk = 0;
}

static void _ErrFunc()
{
    static uint32_t count = 0;
    if(++count >= 2400000) //2分钟后，开始故障复位
    {
        //_ScidSendMsgToBuffErr("err code %04X%04X", gAlgoErrDef.byte[1], gAlgoErrDef.byte[0]);
        count = 0;
        gErrResetFlag = 1; 
    }
    gInvReleaseOk = 0;
    gPwm = 0;
}

static uint16_t _ErrConditionToClose()
{
    if(gErrResetFlag == 1)
    {
        if(gAlgoErrDef.err.bit8 == 1) //预留不恢复故障接口
        {
            return 0;
        }
        else
        {
            Clear_cmpss_ost();
            ResetInvPwmCurrLimit();
            gAlgoErrDef.byte[0] = 0; //清除故障byte位
            gAlgoErrDef.byte[1] = 0; //清除故障byte位
            gErrResetFlag = 0;       //清除故障复位标志位
            gAlgoErr = 0;  //清除总故障位
            gstFaultInfo.worning_code.b.leak_curr_high = 0;
            return 1;
        }
    }
    return 0;
    // return gAlgoInfo->sys_enable == 1 && gAlgoInfo->llc_start_ok == 1 && gAlgoErr == 0 && GetGridToGenFinish() == 1;
}


#define TRANSITION_ERR {_ErrCheck, _ErrAction,&_gStateErr}

static FsmState_t _gStateErr = {
    .id = EN_INV_STATE_DEF_ERR,
    .transition_num = 1,
    .transitions = (FsmTransition_t[]){
    {
        .action = _ErrActionToClose,
        .condition = _ErrConditionToClose,
        .next_state = &gStateClose,
    }
    },
    .func = _ErrFunc,
};
static void _CloseActionToOpening()
{
    gPwm = 0;
    gPwmK = 0;
    gSignStamp = 0;
    if((GetGridToGenFinish() == 1) && (GetGenVoltNormal() == 1) && (GetGenLoadFault() == 0))//发电机模式,发电机电压正常时开发电机继电器和负载继电器,不开主机电器
    {
        RelaySetValue(EN_RELAY_DEF_MAIN, 0);
        RelaySetValue(EN_RELAY_DEF_ENGINE, 1);
    }
    else
    {
        RelaySetValue(EN_RELAY_DEF_ENGINE, 0);
        RelaySetValue(EN_RELAY_DEF_MAIN, 1);
    }
    if(GetGenLoadFault() == 0)
    {
//        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
    else
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, 0);
        ModbusReg.backup_output_enable = 0;
        gSysSetInfo.backup_output_enable = 0;
    }
    if(GetGridToGenFinish() == 0)
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
    ScidSendMsgToBuff("close -> opening");
}

static uint16_t _CloseConditionToOpening()
{
    return gAlgoInfo->sys_enable == 1 && gAlgoInfo->llc_start_ok == 1 && GetGenLoadFault() == 0;
}

static void _CloseActionToRelease()
{
    gPwm = 0;
    gPwmK = 0;
    gSignStamp = 0;
    gInvReleaseOk = 0;
    gReleaseCount = 0;
}

static uint16_t _CloseConditionToRelease()
{
    return gAlgoInfo->sys_enable == 1 && gInvReleaseOk == 0;
}

FsmState_t gStateClose = {
    .id = EN_INV_STATE_DEF_CLOSE,
    .transition_num = 3,
    .transitions = (FsmTransition_t[]) {
        TRANSITION_ERR,
        {
            .action = _CloseActionToRelease,
            .condition = _CloseConditionToRelease,
            .next_state = &gStateRelease,
        },
        {
            .action = _CloseActionToOpening,
            .condition = _CloseConditionToOpening,
            .next_state = &gStateOpening,
        }
    },
    .func = InvStateClosedFunc
};

static void _ReleaseActionToClose()
{
    gPwm = 0;
    gPwmK = 0;
    gSignStamp = 0;
    gReleaseCount = 0;
    gInvReleaseOk = gAlgoInfo->sys_enable;
}

static uint16_t _ReleaseConditionToClose()
{
    return ((gAlgoInfo->sensor.bus_vol < 5) || (gAlgoInfo->sys_enable == 0) || (gReleaseCount >= RELEASE_OVER_TIME));  //超时，跳出release，强制启机
}

FsmState_t gStateRelease = {
    .id = EN_INV_STATE_DEF_RELEASE,
    .transition_num = 2,
    .transitions = (FsmTransition_t[]) {
        TRANSITION_ERR,
        {
            .action = _ReleaseActionToClose,
            .condition = _ReleaseConditionToClose,
            .next_state = &gStateClose,
        }
    },
    .func = InvStateReleaseFunc
};

static void _OpeningActionToDisConnected()
{
    gSignStamp = gAcVolOpenLoopRatio;
    gPwmK = 1;
//    if(GetGridToGenFinish() == 1)//发电机模式
//    {
//        RelaySetValue(EN_RELAY_DEF_LOAD, 1);
//    }
//    else
//    {
//        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
//    }
    RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    ScidSendMsgToBuff("opening -> disconnected");
}
static uint16_t _OpeningConditionToDisConnected()
{
    return gInvStartOk == 1;
}
static void _OpeningActionToClose()
{
    if((GetGridToGenFinish() == 1) && (GetGenLoadFault() == 0))//发电机模式
    {
        RelaySetValue(EN_RELAY_DEF_ENGINE, 1);
//        RelaySetValue(EN_RELAY_DEF_LOAD, 1);
        RelaySetValue(EN_RELAY_DEF_MAIN, 0);
    }
    else
    {
        RelaySetValue(EN_RELAY_DEF_MAIN, 0);
//        RelaySetValue(EN_RELAY_DEF_LOAD, 0);
    }
    if(GetGenLoadFault() == 0)
    {
//        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
    else
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, 0);
        ModbusReg.backup_output_enable = 0;
        gSysSetInfo.backup_output_enable = 0;
    }
    if(GetGridToGenFinish() == 0)
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
    gPwm = 0;
    gPwmK = 0;
    gSignStamp = 0;
    gInvReleaseOk = 0;
    ScidSendMsgToBuff("opening -> close");
}
static uint16_t _OpeningConditionToClose()
{
    return gAlgoInfo->sys_enable == 0 || gAlgoInfo->llc_start_ok == 0;
}

FsmState_t gStateOpening = {
    .id = EN_INV_STATE_DEF_OPENING,
    .transition_num = 3,
    .transitions = (FsmTransition_t[]) {
        TRANSITION_ERR,
        {
            .action = _OpeningActionToDisConnected,
            .condition = _OpeningConditionToDisConnected,
            .next_state = &gStateDisConnected,
        },
        {
            .action = _OpeningActionToClose,
            .condition = _OpeningConditionToClose,
            .next_state = &gStateClose,
        }
    },
    .func = InvStateOpeningFunc,
};

static void _DisConnectedActionToClose()
{
    if((GetGridToGenFinish() == 0) || (GetGenLoadFault() == 1))//发电机模式下负载过流或过载断开负载继电器
    {
//        RelaySetValue(EN_RELAY_DEF_LOAD, 0);
    }
    if(GetGenLoadFault() == 0)
    {
//        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
    else
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, 0);
        ModbusReg.backup_output_enable = 0;
        gSysSetInfo.backup_output_enable = 0;
    }
    if(GetGridToGenFinish() == 0)
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
    RelaySetValue(EN_RELAY_DEF_MAIN, 0);
    gPwm = 0;
    gPwmK = 0;
    gSignStamp = 0;
    gInvReleaseOk = 0;
    ScidSendMsgToBuff("disconnected -> close");
}
static uint16_t _DisConnectedConditionToClose()
{
    return gAlgoInfo->sys_enable == 0 || GetGenLoadFault() == 1;
}
static void _DisConnectedActionToConnecting()
{
    gGridDisConnectedTime = 0;
    gVolPwrOffset = 0;
    gGridConnectedCurRef = 0;
    DelayTriggerInit(&gGridConnectTrig, 2000);
    DelayTriggerInit(&gstGridVolLowTrigger, 10);
    gIsLet = 0;
    gGridRlyActived = 0;
    CountTriggerReset(&gGridRlyDelayTrigger);
    ScidSendMsgToBuff("disconnected -> connecting");
}
static uint16_t _DisConnectedConditionToConnecting()
{
    //TODO: 判断电网频率
    uint8_t warning = 0;
    warning |= gAlgoWarningDef.err.grid_freq_low;
    warning |= gAlgoWarningDef.err.grid_freq_over;
    warning |= gAlgoWarningDef.err.grid_vol_low;
    warning |= gAlgoWarningDef.err.grid_vol_over;
    warning |= GetGenLoadFault();
    return (gEnGridConnect == 1) && (warning == 0) && (gAlgoInfo->sys_enable == 1) && (gGridDisConnectedTime >= (uint32_t)20000 * 10);
}

FsmState_t gStateDisConnected = {
    .id = EN_INV_STATE_DEF_DISCONNECTED,
    .transition_num = 3,
    .transitions = (FsmTransition_t[]) {
        TRANSITION_ERR,
        {
            .action = _DisConnectedActionToClose,
            .condition = _DisConnectedConditionToClose,
            .next_state = &gStateClose,
        },
        {
            .action = _DisConnectedActionToConnecting,
            .condition = _DisConnectedConditionToConnecting,
            .next_state = &gStateConnecting,
        }
    },
    .func = InvStateDisConnectedFunc
};

static void _ConnectingActionToDisConnected()
{

    gVolPwrOffset = 0;
    gGridConnectedCurRef = 0;
    ScidSendMsgToBuff("connecting -> disconnected");
}
static uint16_t _ConnectingConditionToDisConnected()
{
    uint8_t warning = 0;
    warning |= gAlgoWarningDef.err.grid_freq_low;
    warning |= gAlgoWarningDef.err.grid_freq_over;
    warning |= gAlgoWarningDef.err.grid_vol_low;
    warning |= gAlgoWarningDef.err.grid_vol_over;
    return (gEnGridConnect == 0) || (warning == 1) || (gAlgoInfo->sys_enable == 0);
}
static void _ConnectingActionToConnected()
{

    gGridConnectedTime = 0;
    ResetDutyAvg();
    TimeTriggerClear(&gEnIsletTrig);
    DelayTriggerClear(&gIsletCheckTrig);
    DelayTriggerClear(&gIsletCurCheckTrigger);
    HoldTriggerClear(&gIsletRunTrig);
    CNTL_2P2Z_F_VARS_init(&gGvPrVars);
    gGvPrVars.Errn1=0;
    gGvPrVars.Errn2=0;
    gGvPrVars.Errn=0;
    gGvPrVars.Out1=0;
    gGvPrVars.Out2=0;
    gGvPrVars.Out=0;
    gIsletErrCount = 0;

    if(GetGridToGenFinish() == 1)//发电机模式
    {
        if(GetEngineRlyState() == 1)//发电机继电器已闭合,并网闭合主继电器
        {
            RelaySetValue(EN_RELAY_DEF_MAIN, 1);
            RelaySetValue(EN_RELAY_DEF_GRID, 0);
        }
        else
        {
            RelaySetValue(EN_RELAY_DEF_ENGINE, 1);
            RelaySetValue(EN_RELAY_DEF_GRID, 0);
        }
    }
    else
    {
        RelaySetValue(EN_RELAY_DEF_GRID, 1);
        RelaySetValue(EN_RELAY_DEF_ENGINE, 0);
    }

    EALLOW;
    (*ePWM[2]).TZCTL.bit.TZB = TZ_FORCE_LO;
    (*ePWM[3]).TZCTL.bit.TZB = TZ_FORCE_LO;
    EDIS;
    gSignThetaDeg += 0.03;
    ScidSendMsgToBuff("connecting -> connected");
}
static uint16_t _ConnectingConditionToConnected()
{
    uint8_t is_ok = 0;

    if(GetGridToGenFinish() == 1)//已切换至发电机模式
    {
        is_ok = (fabsf(gAlgoInfo->grid_vol_rms_filter - gAlgoInfo->inv_vol_rms) < gAcVolSet * 0.03) && (gAlgoInfo->grid_phase_err > -0.15);
    }
    else
    {
        is_ok = (fabsf(gAlgoInfo->grid_vol_rms_filter - gAlgoInfo->inv_vol_rms) < gAcVolSet * 0.03) && (gAlgoInfo->grid_phase_err > -0.075);
    }
    DelayTriggerRun(&gGridConnectTrig, is_ok);

    CountTriggerRun(&gGridRlyDelayTrigger, gGridConnectTrig.output);
    //return 1;
    return gGridRlyActived;
}
FsmState_t gStateConnecting = {
    .id = EN_INV_STATE_DEF_CONNECTING,
    .transition_num = 3,
    .transitions = (FsmTransition_t[]) {
        TRANSITION_ERR,
        {
            .action = _ConnectingActionToDisConnected,
            .condition = _ConnectingConditionToDisConnected,
            .next_state = &gStateDisConnected,
        },
        {
            .action = _ConnectingActionToConnected,
            .condition = _ConnectingConditionToConnected,
            .next_state = &gStateConnected,
        }
    },
    .func = InvStateConnectingFunc
};
extern int gGotoDisConnectingReset;
static void _ConnectedActionToDisConnecting()
{
    gAcCurRefK = 0;
    ResetDutyAvg();
    DelayTriggerInit(&gDisConnectGridPhaseTrigger, 1000);
    ScidSendMsgToBuff("connected -> disconnecting");
    gGotoDisConnectingReset = 1;
}
static uint16_t _ConnectedConditionToDisConnecting()
{
    DelayTriggerRun(&gstGridVolLowTrigger, fabsf(gAlgoInfo->sensor.grid_vol) < ALGOWARNING_THRESHOLD_GRID_VOL_LOW_OFF * 1.414);
    uint8_t warning = 0;
    warning |= gAlgoWarningDef.err.grid_freq_low;
    warning |= gAlgoWarningDef.err.grid_freq_over;
    warning |= gAlgoWarningDef.err.grid_vol_low;
    warning |= gAlgoWarningDef.err.grid_vol_over;
    warning |= gstGridVolLowTrigger.output;
    warning |= GetGenLoadFault();
    return ((gEnGridConnect == 0) && (fabsf(gAlgoInfo->grid_cur_rms) < 10)) || (gAlgoInfo->sys_enable == 0) || (gIsLet == 1) || (warning == 1);
}
FsmState_t gStateConnected = {
    .id = EN_INV_STATE_DEF_CONNECTED,
    .transition_num = 2,
    .transitions = (FsmTransition_t[]) {
        TRANSITION_ERR,
        {
            .action = _ConnectedActionToDisConnecting,
            .condition = _ConnectedConditionToDisConnecting,
            .next_state = &gStateDisConnecting,
        }
    },
    .func = InvStateConnectedFunc
};

static void _DisConnectingActionToDisConnected()
{
    EALLOW;
    (*ePWM[2]).TZCTL.bit.TZB = TZ_NO_CHANGE;
    (*ePWM[3]).TZCTL.bit.TZB = TZ_NO_CHANGE;
    EDIS;
    ScidSendMsgToBuff("disconnecting -> disconnected");
}
static uint16_t _DisConnectingConditionToDisConnected()
{
    DelayTriggerRun(&gDisConnectGridPhaseTrigger, gAcCurRefK == 1);
    return gDisConnectGridPhaseTrigger.output || (gAlgoInfo->sys_enable == 0) || (GetGridToGenFinish() == 1);
}
FsmState_t gStateDisConnecting = {
    .id = EN_INV_STATE_DEF_DISCONNECTING,
    .transition_num = 2,
    .transitions = (FsmTransition_t[]) {
        TRANSITION_ERR,
        {
            .action = _DisConnectingActionToDisConnected,
            .condition = _DisConnectingConditionToDisConnected,
            .next_state = &gStateDisConnected,
        }
    },
    .func = InvStateDisConnectingFunc
};
