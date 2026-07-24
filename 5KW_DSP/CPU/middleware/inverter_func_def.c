#include "inverter_func_def.h"
#include "algo_info.h"
#include "CountTrigger.h"
#include "inverter_loop_cntl.h"
#include "pll.h"
#include "TimeTrigger.h"
#include "voltagesourceinvlcfltr.h"
#include "dead_compensation.h"
#include "pwm_cntl.h"
#include "generator.h"
#include "dc_injection.h"
#include "cla_algo.h"
#include "modbus_analysis.h"
#define VOL_RMS_CNTL_I  (1.0/ISR_CONTROL_FREQUENCY)
#define KCURRENT_EXTRA 0.01f
#define CURRENT_EXTRA_MAX 0.05f 
#define CURRENT_EXTRA_MIN -0.05f

extern CNTL_PI_F gGiPi;                        //电流环PI控制器
extern CNTL_PI_F gGvPi;                        //电压环PI控制器
extern SINEANALYZER_DIFF_F gGvOutAnlz;         //电流环给定信号分析器
extern Pll_t gPll, gPllOffset;                             //电网锁相环
extern DelayTrigger_t gGridConnectTrig;
extern DelayTrigger_t gIsletCheckTrig;
extern DelayTrigger_t gIsletCurCheckTrigger;
extern HoldTrigger_t gIsletRunTrig;
extern TimeTrigger_t gEnIsletTrig;
extern CountTrigger_t gGridRlyDelayTrigger;

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

extern EN_GRID_CONNECT_STA gStaGridConnect;//并网状态
extern float gGridPhaseErr;                //相位偏差
extern float gVolPwrOffset;

extern float gPwm;        //逆变器输出PWM占空比
extern float gPwmK;       //逆变器控制器输出比例
extern uint8_t gInvStartOk;

extern uint8_t gEnFreqAdjust;

extern uint8_t gInvErr;

extern const AlgoInfo_t* gAlgoInfo;
extern float gDeltaWn;
extern float gAcCurRefK;
extern uint8_t gIsLet;

extern uint16_t gGridConnectedTime;
extern uint32_t gGridDisConnectedTime;
extern uint8_t gGridRlyActived;
extern float gDegOffset;
extern float gGridFreqLast;
extern uint16_t gIsletTimeCount, gEnIsletCompare, gIsletErrCount;
extern float gSignSinWithDegOffset;
extern uint8_t gIsletTrueFlag;
extern float gIsLetSum, gIsLetAve, gIsLetAveLast, gIsLetAveErr, gIsLetAveLast, gIsLetAveErrLast;
extern float gIsLetErr, gIsLetErrReal;
extern CNTL_2P2Z_F_COEFFS gGvPrCoeffs;
extern CNTL_2P2Z_F_VARS gGvPrVars;
float gDutyCom;
float gLimitK;

extern float gSignSin, gSignCos, gSignTheta;
extern float gSignDegOutput;
extern float gSignThetaDeg, gSignDegOffset;

float GvLoopCntl(float ref, float fbk);
float GiLoopCntl(float ref, float fbk);
uint32_t gReleaseCount = 0;
uint16_t gUnConnectRlyCount = 0;
uint16_t gInvRunstate = 0;
static float _gOpenradio, _gDegLast;

void SignThetaUpdate(float delta_wn)
{
    gSignTheta += (314.15926 + delta_wn) * 0.00005;
    if(gSignTheta > 6.2831853)
    {
        gSignTheta -= 6.2831853;
    }
    gSignSin = sin(gSignTheta);
    gSignCos = cos(gSignTheta);
}

void InvStateClosedFunc()
{
    gInvRunstate = 2;
    gPwm = 0;
    gInvStartOk = 0;
    RelaySetValue(EN_RELAY_DEF_GRID, 0);
    if((GetGridToGenFinish() == 0) || (GetGenLoadFault() == 1))//已切换至发电机模式后不再断开发电机并网继电器
    {
//        RelaySetValue(EN_RELAY_DEF_LOAD, 0);
        RelaySetValue(EN_RELAY_DEF_ENGINE, 0);
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
    RelaySetValue(EN_RELAY_DEF_MAIN, 0);

    float delta_wn = PllHandler(&gPll, garrStdSignSin[0], garrSignCos[0] * -1);
    gDeltaWn = 0;
}

void InvStateReleaseFunc()
{
    gInvRunstate = 1;
    float delta_wn = PllHandler(&gPll, garrStdSignSin[0], garrSignCos[0] * -1);
    gDeltaWn = 0;

    SignThetaUpdate(gDeltaWn);
    gReleaseCount++;

    /*缓启动信号*/
    if(gSignStamp <= gAcVolOpenLoopRatio)
    {
        gSignStamp += gStampSpeed;
    }

    gPwm = gSignStamp * gSignSin;
    gInvReleaseOk = gAlgoInfo->sensor.bus_vol < 5;
}

void InvStateOpeningFunc()
{
    gInvRunstate = 3;
    static uint32_t rdy_count = 0;
    float delta_wn = PllHandler(&gPll, garrStdSignSin[0], garrSignCos[0] * -1);
    gDeltaWn = 0;
    gAvVolRef = gAcVolSet;

    gPwm = 0;

    SignThetaUpdate(gDeltaWn);

    /*缓启动信号*/
    if(gSignStamp <= gAcVolOpenLoopRatio)
    {
        gSignStamp += gStampSpeed;
    }
    else
    {
        float ac_vol_ref = gAvVolRef * gSignSin * (gSignStamp / gAcVolOpenLoopRatio);
        float ac_cur_ref = GvLoopCntl(ac_vol_ref, gAlgoInfo->sensor.inv_vol);

        gGvOutAnlz.Vin = ac_cur_ref;
        SINEANALYZER_DIFF_F_FUNC(&gGvOutAnlz);
        /*如果电流环给定的有效值超过了最大电流限制，则按比例缩减电流给定*/
        if(gGvOutAnlz.Vrms > gAcCurSet / 1.414 * 1.5)
        {
            ac_cur_ref /= gGvOutAnlz.Vrms / (gAcCurSet / 1.414 * 1.5);
        }
        gPwm = GiLoopCntl(ac_cur_ref, gAlgoInfo->sensor.inv_cur);
    }
    if(gPwmK < 1.0)
    {
        gPwmK += gStampSpeed;
    }
    if(gPwmK >= 1.0)
    {
        gPwmK = 1.0;
        rdy_count++;
    }
    else
    {
        rdy_count = 0;
    }
    gInvStartOk = (rdy_count >= 10000);

    gPwm = gPwm + gSignStamp * gSignSin;
}

static float32 _InvCurLimit()
{
    static float protect_i = 1;
    float cur_limit = HARDERR_THRESHOLD_IND_CUR_OVER;
    float limit_sign = cur_limit * (gAlgoInfo->inv_vol_phase);
    float limit_sign_abs = fabsf(limit_sign);
    float inv_cur_abs = fabsf(gAlgoInfo->sensor.inv_cur);
    float protect = 0;
    float err = limit_sign_abs - inv_cur_abs;
    float theta = gAlgoInfo->inv_vol_theta;
    if(inv_cur_abs > cur_limit / 2)
    {
        if(theta > 3.1415926 / 6 && theta < 3.1415926 / 3)
        {
            if(limit_sign_abs < inv_cur_abs - 0.5)
            {
                protect_i += (err) * 20.0 / 10 * 0.0005;
                if(protect_i < -1)
                {
                    protect_i = -1;
                }
                protect = err * 0.1 / 10;
            }
        }
    }
    if(limit_sign_abs > inv_cur_abs + 0.5)
    {
        if(protect_i < 1)
        {
            protect_i += 7.5 / 10 * 0.0005;
        }
        if(protect < 0)
        {
            protect += 0.05 / 10 * 0.0005;
        }
    }
    if(protect_i > 1)
    {
        protect_i = 1;
    }
    if(protect < 0)
    {
        protect = 0;
    }
    float k = protect_i + protect;
    if(k < 0.6)
    {
        k = 0.62;
//        gIsInvCurLimitOverCount = 1;
    }
    gLimitK = k;
    return k;
}

void InvStateDisConnectedFunc()
{
    gInvRunstate = 7;
    float delta_wn = PllHandler(&gPll, garrStdSignSin[0], garrSignCos[0] * -1);
    float cur_extra_add = KCURRENT_EXTRA * gAlgoInfo->current_extra;
    gDeltaWn = 0;
    //gSignSinWithDegOffset = PllOffsetHandler(&gPllOffset, garrSignSin[0], 0, 0);
    gAvVolRef = gAcVolSet;

    SignThetaUpdate(gDeltaWn);

    float k = _InvCurLimit();

    float ac_vol_ref = gAvVolRef * gSignSin * k;
    float ac_cur_ref = GvLoopCntl(ac_vol_ref, gAlgoInfo->sensor.inv_vol);

    gGvOutAnlz.Vin = ac_cur_ref;
    SINEANALYZER_DIFF_F_FUNC(&gGvOutAnlz);
    /*如果电流环给定的有效值超过了最大电流限制，则按比例缩减电流给定*/
    if(gGvOutAnlz.Vrms > gAcCurSet / 1.414 * 1.5)
    {
        ac_cur_ref /= gGvOutAnlz.Vrms / (gAcCurSet / 1.414 * 1.5);
    }

    gPwm = GiLoopCntl(ac_cur_ref, gAlgoInfo->sensor.inv_cur);
    if(cur_extra_add > CURRENT_EXTRA_MAX)
    {
        cur_extra_add = CURRENT_EXTRA_MAX;
    }
    else if(cur_extra_add < CURRENT_EXTRA_MIN)
    {
        cur_extra_add = CURRENT_EXTRA_MIN;
    }

    float32 duty_dc_reject = DcRejection(gstInvAnlz.vol_avg, gstInvAnlz.cur_avg);
    gPwm = gPwm + gAcVolOpenLoopRatio * gSignSin * k - cur_extra_add + duty_dc_reject;
    gPwm = InvDutyLimit(gPwm);


    gGridDisConnectedTime++;
    if(gGridDisConnectedTime > (uint32_t)20000 * 60)
    {
        gGridDisConnectedTime = (uint32_t)20000 * 60;
    }
    if(gAlgoWarningDef.err.grid_freq_low || gAlgoWarningDef.err.grid_freq_over || gAlgoWarningDef.err.grid_vol_low || gAlgoWarningDef.err.grid_vol_over)
    {
        gGridDisConnectedTime = 0;
    }
    if(GetGridToGenFinish() == 0)
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
//    else
//    {
//        if(GetGenVoltNormal() == 0)//发电机电压异常则闭合主路继电器维持负载输出
//        {
//            RelaySetValue(EN_RELAY_DEF_ENGINE, 0);
//            RelaySetValue(EN_RELAY_DEF_MAIN, 1);
//            RelaySetValue(EN_RELAY_DEF_LOAD, 1);
//        }

        if(GetGenLoadFault() == 1)//发电机模式下负载过流或过载断开负载继电器
        {
            RelaySetValue(EN_RELAY_DEF_LOAD, 0);
            RelaySetValue(EN_RELAY_DEF_ENGINE, 0);
            ModbusReg.backup_output_enable = 0;
            gSysSetInfo.backup_output_enable = 0;
        }
//    }
}

//float limit_vol_low = 0;
//float limit_vol_low_abs = 0;
//float limit_vol_high = 0;
//float limit_vol_high_abs = 0;
//float Watch_GridVoltage = 0; //20260122，增加电网电压观测变量
//uint32_t count_BadGrid = 0; //20260122，增加电网质量差计数器
//uint32_t flag_disconnect = 0; //20260122，增加脱网标志
//uint32_t limit_BadGrid = 0; //20260415，新增0ms脱网响应时间计数数值

float Watch_limit_vol_low = 0;
float Watch_limit_vol_high = 0;

static void _IsletCheckLow()
{
    float limit_vol_low = 150 * 1.414 * gAlgoInfo->grid_vol_phase; //20260206，扩大电网电压弃用下限到150V
    float limit_vol_low_abs = fabsf(limit_vol_low);
    float limit_vol_high = 273 * 1.414 * gAlgoInfo->grid_vol_phase; //20260206，扩大电网电压弃用上限到273V
    float limit_vol_high_abs = fabsf(limit_vol_high);
    float grid_vol_abs = fabsf(gAlgoInfo->sensor.grid_vol);
    float Watch_GridVoltage = 0; //20260122，增加电网电压观测变量
    static uint32_t count_BadGrid = 0; //20260122，增加电网质量差计数器
    static uint32_t flag_disconnect = 0; //20260122，增加脱网标志
    static uint32_t limit_BadGrid = 0; //20260415，新增0ms脱网响应时间计数数值
    float check_theta = gSignTheta > 3.1415926 ? (gSignTheta - 3.1415926) : gSignTheta;

    if(gSysSetInfo.off_grid_sensitivity == 6)
    {
        limit_vol_low  = 190 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增6级0ms脱网响应对应电网电压弃用下限190V
        limit_vol_high = 250 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增6级0ms脱网响应对应电网电压弃用上限250V
        limit_BadGrid  = 10;                                      //20260415，新增6级0ms脱网响应对应电网弃用计数10次
    }
    else if(gSysSetInfo.off_grid_sensitivity == 5)
    {
        limit_vol_low  = 180 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增5级0ms脱网响应对应电网电压弃用下限180V
        limit_vol_high = 255 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增5级0ms脱网响应对应电网电压弃用上限255V
        limit_BadGrid  = 20;                                      //20260415，新增5级0ms脱网响应对应电网弃用计数20次
    }
    else if(gSysSetInfo.off_grid_sensitivity == 4)
    {
        limit_vol_low  = 170 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增4级0ms脱网响应对应电网电压弃用下限170V
        limit_vol_high = 260 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增4级0ms脱网响应对应电网电压弃用上限260V
        limit_BadGrid  = 30;                                      //20260415，新增4级0ms脱网响应对应电网弃用计数30次
    }
    else if(gSysSetInfo.off_grid_sensitivity == 3)
    {
        limit_vol_low  = 160 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增3级0ms脱网响应对应电网电压弃用下限160V
        limit_vol_high = 265 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增3级0ms脱网响应对应电网电压弃用上限265V
        limit_BadGrid  = 40;                                      //20260415，新增3级0ms脱网响应对应电网弃用计数40次
    }
    else if(gSysSetInfo.off_grid_sensitivity == 2)
    {
        limit_vol_low  = 150 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增2级0ms脱网响应对应电网电压弃用下限150V
        limit_vol_high = 270 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增2级0ms脱网响应对应电网电压弃用上限270V
        limit_BadGrid  = 50;                                      //20260415，新增2级0ms脱网响应对应电网弃用计数50次
    }
    else if(gSysSetInfo.off_grid_sensitivity == 1)
    {
        limit_vol_low  = 140 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增1级0ms脱网响应对应电网电压弃用下限140V
        limit_vol_high = 275 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增1级0ms脱网响应对应电网电压弃用上限275V
        limit_BadGrid  = 60;                                      //20260415，新增1级0ms脱网响应对应电网弃用计数60次
    }
    else
    {
        limit_vol_low  = 130 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增0级0ms脱网响应对应电网电压弃用下限130V
        limit_vol_high = 280 * 1.414 * gAlgoInfo->grid_vol_phase; //20260415，新增0级0ms脱网响应对应电网电压弃用上限280V
        limit_BadGrid  = 70;                                      //20260415，新增0级0ms脱网响应对应电网弃用计数70次
    }

    limit_vol_low_abs  = fabsf(limit_vol_low);
    limit_vol_high_abs = fabsf(limit_vol_high);

    Watch_limit_vol_low  = limit_vol_low_abs  / (1.414 * gAlgoInfo->grid_vol_phase);
    Watch_limit_vol_high = limit_vol_high_abs / (1.414 * gAlgoInfo->grid_vol_phase);
    Watch_GridVoltage    = grid_vol_abs       / (1.414 * gAlgoInfo->grid_vol_phase);

    if(check_theta > 3.1415926 * 1 / 6 && check_theta < 3.1415926 * 5 / 6) //20260126，缩小相位可用区间从1/6pi到5/6pi
    {
        if(limit_vol_low_abs > grid_vol_abs || limit_vol_high_abs < grid_vol_abs)
        {
            count_BadGrid++; //20260122，增加电网弃用计数
            if(count_BadGrid >= limit_BadGrid)
            {
                count_BadGrid = 0;
                gIsLet |= 1; //控制继电器脱网
                flag_disconnect = 1; //20260122，增加电网脱网触发标志
            }
            else
            {
                gIsLet = 0; //保持并网
                flag_disconnect = 0; //20260122，增加电网脱网触发标志
            }
        }
        else
        {
            gIsLet = 0; //保持并网
            flag_disconnect = 0; //20260122，增加电网脱网触发标志
            if(count_BadGrid <= 1)
            {
                count_BadGrid = 1;
            }
            count_BadGrid--; //20260126，增加电网弃用计数
        }
    }
}

void InvStateConnectingFunc()
{
    gInvRunstate = 4;
    float delta_wn = PllHandler(&gPll, gAlgoInfo->grid_vol_phase, gSignCos * -1);
    //float delta_wn = PllHandler(&gPll, gAlgoInfo->grid_vol_phase, garrSignCos[0] * -1);
    gDeltaWn = delta_wn;
    SignThetaUpdate(gDeltaWn);
    //gDegOffset = 0;
    gSignThetaDeg += gAlgoInfo->grid_deg_err * 0.00005;
    gSignDegOffset = 2 * cos(gSignTheta + gSignThetaDeg * 0.5) * sin(gSignThetaDeg * 0.5);
    //gSignSinWithDegOffset = PllOffsetHandler(&gPllOffset, garrSignSin[0], gDegOffset, 0);
    //gDeltaWn = delta_wn;
    if(gVolPwrOffset < gAcVolSet * 0.2)
    {
        gVolPwrOffset += VOL_RMS_CNTL_I * (gAlgoInfo->grid_vol_rms - gAlgoInfo->inv_vol_rms);
    }
    gAvVolRef = (gAcVolSet + gVolPwrOffset);
    float sign = gSignDegOffset + gSignSin;
    float ac_vol_ref = gAvVolRef * sign;
    float ac_cur_ref = GvLoopCntl(ac_vol_ref, gAlgoInfo->sensor.inv_vol);

    gGvOutAnlz.Vin = ac_cur_ref;
    SINEANALYZER_DIFF_F_FUNC(&gGvOutAnlz);
    /*如果电流环给定的有效值超过了最大电流限制，则按比例缩减电流给定*/
    if(gGvOutAnlz.Vrms > gAcCurSet / 1.414 * 1.5)
    {
        ac_cur_ref /= gGvOutAnlz.Vrms / (gAcCurSet / 1.414 * 1.5);
    }

    gPwm = GiLoopCntl(ac_cur_ref, gAlgoInfo->sensor.inv_cur);

    gPwm = gPwm + gAcVolOpenLoopRatio * sign;
    //gSignThetaDeg = 0.06;
    gPwm = InvDutyLimit(gPwm);
    if(GetGridToGenFinish() == 0)//非发电机模式才判断，发电机模式下不断开
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
}

static void _IsletCheckCur()
{
    if(GetGridToGenFinish() == 1)//已切换至发电机模式
    {
        if(gAlgoInfo->grid_freq <= 40 || gAlgoInfo->grid_freq >= 60)
        {
            gIsLet = 1;
        }
    }
    else
    {
        if(gAlgoInfo->grid_freq <= 45 || gAlgoInfo->grid_freq >= 55)
        {
            gIsLet = 1;
        }
    }

    DelayTriggerRun(&gIsletCurCheckTrigger, gAlgoInfo->grid_cur_rms_ave < 0.2);
    gIsLet |= gIsletCurCheckTrigger.output;

    if(gIsLet == 1)
    {
        if(gAlgoInfo->grid_cur_rms >= 3)
        {
            gIsLet = 0;
        }
    }
}

void InvStateConnectedFunc()
{
    gInvRunstate = 5;
    float cur_extra_add = KCURRENT_EXTRA * gAlgoInfo->current_extra;
    float sign = 0;

    gUnConnectRlyCount = 0;
    if(gGridConnectedTime == 20000 && gAlgoInfo->inv_state == EN_INV_STATE_DEF_CONNECTED)
    {
        //_IsletCheck();
        //_IsletCheckCur();
        //gIsLet |= (gAlgoInfo->grid_omiga > 20);
        if(ModbusReg.ups_delay0ms == 1)
        {
            _IsletCheckLow();
        }

    }

    //gSignSinWithDegOffset = PllOffsetHandler(&gPllOffset, gAlgoInfo->grid_vol_phase, gDegOffset, gEnFreqAdjust);
    float delta_wn = PllHandler(&gPll, gAlgoInfo->grid_vol_phase, gSignCos * -1);
    gDeltaWn = delta_wn;

    SignThetaUpdate(gDeltaWn);

    //deg = 0;
    gSignDegOffset = 2 * cos(gSignTheta + gSignThetaDeg * 0.5) * sin(gSignThetaDeg * 0.5); //sin(gSignTheta+gSignThetaDeg) - sin(gSignTheta) gSignThetaDeg为无功控制角度
    float grid_sin = gAlgoInfo->sensor.grid_vol / gAlgoInfo->grid_vol_rms / 1.414;

//    if(GetGridToGenFinish() == 1)//已切换至发电机
//    {
//        sign = gSignDegOffset + gSignSin;
//    }
//    else//已切换至市电模式
//    {
        sign = gSignDegOffset + grid_sin; //sqrt(((sqrt(1+cos(gSignThetaDeg)*cos(B)))^2+1 )
//    }

    gGridConnectedTime++;
    if(gGridConnectedTime > 20000)
    {
        gGridConnectedTime = 20000;
    }

    if(gGridConnectedCurRef > gAcCurSet)
        gGridConnectedCurRef = gAcCurSet;
    if(gGridConnectedCurRef < -gAcCurSet)
        gGridConnectedCurRef = -gAcCurSet;

    float ac_cur_ref;
    if(gGridConnectedTime < 400)
    {
        float ac_vol_ref = gAlgoInfo->grid_vol_rms * 1.414 * sign;
        ac_cur_ref = GvLoopCntl(ac_vol_ref, gAlgoInfo->sensor.inv_vol);
    }
    else
    {
        ac_cur_ref = gGridConnectedCurRef * sign;
    }

    gGvOutAnlz.Vin = ac_cur_ref;
    SINEANALYZER_DIFF_F_FUNC(&gGvOutAnlz);
    /*如果电流环给定的有效值超过了最大电流限制，则按比例缩减电流给定*/
    if(gGvOutAnlz.Vrms > gAcCurSet / 1.414 * 1.5)
    {
        ac_cur_ref /= gGvOutAnlz.Vrms / (gAcCurSet / 1.414 * 1.5);
    }

    gPwm = GiLoopCntl(ac_cur_ref, gAlgoInfo->sensor.inv_cur);
    //gPwm = gPwm + gAcVolOpenLoopRatio * BUS_VOLTAGE_NORMAL / gAlgoInfo->sensor.bus_vol * gSignSinWithDegOffset;
    static float open_loop_ratio = OPEN_LOOP_RADIO;
    if(gAlgoInfo->grid_vol_rms_filter > ALGOWARNING_THRESHOLD_GRID_VOL_LOW_STOP && gAlgoInfo->grid_vol_rms_filter < ALGOWARNING_THRESHOLD_GRID_VOL_OVER_OFF)
    {
        //open_loop_ratio = gAcVolOpenLoopRatio * gAlgoInfo->grid_vol_rms * 1.414 / INV_VOL_SET;
    }
    open_loop_ratio = gAlgoInfo->grid_vol_rms * 1.414 / gAlgoInfo->sensor.bus_vol;
//if(gAlgoInfo->grid_vol_rms < ALGOWARNING_THRESHOLD_GRID_VOL_LOW_STOP)
//{
//    open_loop_ratio = gAlgoInfo->grid_vol_rms * 1.414 / gAlgoInfo->sensor.bus_vol;
//}
    if(cur_extra_add > CURRENT_EXTRA_MAX)
    {
        cur_extra_add = CURRENT_EXTRA_MAX;
    }
    else if(cur_extra_add < CURRENT_EXTRA_MIN)
    {
        cur_extra_add = CURRENT_EXTRA_MIN;
    }
    float32 duty_dc_reject = DcRejection(gstInvAnlz.vol_avg, gstInvAnlz.cur_avg);
    gPwm = gPwm + open_loop_ratio * sign + duty_dc_reject;

    gDutyCom = dead_compensation(gAlgoInfo->sensor.inv_cur, gPwm, gAlgoInfo->sensor.bus_vol, gAlgoInfo->sensor.inv_vol);
    gPwm = gPwm + gDutyCom;
    gPwm = InvDutyLimit(gPwm);
    if(GetGridToGenFinish() == 0)
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
    _gOpenradio = open_loop_ratio;
    _gDegLast = gSignThetaDeg;
}
int gGotoDisConnectingReset=0;

void InvStateDisConnectingFunc()
{
    gInvRunstate = 6;
    static uint16_t count = 0;
    gIsLet |= gAlgoWarningDef.err.grid_vol_over | gAlgoWarningDef.err.grid_vol_low | gAlgoWarningDef.err.grid_freq_over | gAlgoWarningDef.err.grid_freq_low;
    if(((fabsf(gAlgoInfo->grid_cur_rms) > 10 && gAlgoInfo->relay.rly_grid == 1) ||
       ((fabsf(gAlgoInfo->grid_cur_rms) > 10) && ((gAlgoInfo->relay.rly_engine == 1) && (GetGridToGenFinish() == 1) && (gAlgoInfo->relay.rly_main == 1)))) && (!gIsLet))
    {
        InvStateConnectedFunc();
        gGridRlyActived = 0;
        count = 0;
        CountTriggerReset(&gGridRlyDelayTrigger);
    }
    else
    {
        if(count <= 20000)
            count++;
        if((gAlgoInfo->relay.rly_grid == 1) || ((GetGridToGenFinish() == 1) && (gAlgoInfo->relay.rly_engine == 1) && (gAlgoInfo->relay.rly_main == 1)))
        {
            count = 0;
        }
        if(gGotoDisConnectingReset == 1)
        {
            CNTL_2P2Z_F_VARS_init(&gGvPrVars);
            gGvPrVars.Errn1=0;
            gGvPrVars.Errn2=0;
            gGvPrVars.Errn=0;
            gGvPrVars.Out1=0;
            gGvPrVars.Out2=0;
            gGvPrVars.Out=0;
            count = 0;
            gGotoDisConnectingReset = 0;
        }
        if(GetGridToGenFinish() == 1)
        {
            if(0)//发电机电压正常,断主继电器,异常则断发电机继电器
            {
                RelaySetValue(EN_RELAY_DEF_MAIN, 0);
                RelaySetValue(EN_RELAY_DEF_ENGINE, 1);
            }
            else
            {
                RelaySetValue(EN_RELAY_DEF_ENGINE, 0);
                RelaySetValue(EN_RELAY_DEF_MAIN, 1);
            }
        }
        else
        {
            RelaySetValue(EN_RELAY_DEF_ENGINE, 0);
        }
        RelaySetValue(EN_RELAY_DEF_GRID, 0);
        CountTriggerRun(&gGridRlyDelayTrigger, 1);

        float delta_wn = PllHandler(&gPll, gAlgoInfo->grid_vol_phase, gSignCos * -1);
        static float open_loop_ratio = OPEN_LOOP_RADIO;
        gGridConnectedCurRef = 0;

        float sign = gSignSin;
        float ac_cur_ref;
        float open_loop_ratio_inst = 0;
        float open_loop_ratio_rms = 0;
        float open_loop_ratio_k = (count) / 400.0f;
        float open_loop_ratio_k2 = (count - 400.0f) / 400.0f;
        if(open_loop_ratio_k > 1.0f)
            open_loop_ratio_k = 1.0f;
        if(open_loop_ratio_k2 > 1.0f)
            open_loop_ratio_k2 = 1.0f;
        open_loop_ratio_rms = gAlgoInfo->grid_vol_rms_filter * 1.414 / gAlgoInfo->bus_vol_filter;
        
        if(count < 400)
        {
            gSignDegOffset = 2 * cos(gSignTheta + gSignThetaDeg * 0.5) * sin(gSignThetaDeg * 0.5);
            sign = gSignSin + gSignDegOffset;
            gDeltaWn = delta_wn;
            ac_cur_ref = 0;
            //open_loop_ratio = gAcVolOpenLoopRatio * gAlgoInfo->grid_vol_rms * 1.414 / INV_VOL_SET;
            gAvVolRef = gAlgoInfo->grid_vol_rms_filter * 1.414213;
            open_loop_ratio_inst = gAlgoInfo->sensor.grid_vol / gAlgoInfo->bus_vol_filter;
            open_loop_ratio = (1.0f - open_loop_ratio_k) * open_loop_ratio_inst + (open_loop_ratio_k) * open_loop_ratio_rms * sign;
        }
        else
        {
            open_loop_ratio = ((1.0f - open_loop_ratio_k2) * open_loop_ratio_rms + (open_loop_ratio_k2) * OPEN_LOOP_RADIO) * sign;
            if(count < 800)
            {
                gDeltaWn = delta_wn;
                gAvVolRef = gAlgoInfo->grid_vol_rms_filter * 1.414213;
                // open_loop_ratio_rms = gAlgoInfo->grid_vol_rms_filter * 1.414 / gAlgoInfo->bus_vol_filter;
                // open_loop_ratio = _gOpenradio;
            }
            else
            {
                gDeltaWn = 0;
                gAvVolRef = gAcVolSet;
                // open_loop_ratio = OPEN_LOOP_RADIO;
            }
            float ac_vol_ref = gAvVolRef * sign;
            ac_cur_ref = GvLoopCntl(ac_vol_ref, gAlgoInfo->sensor.inv_vol);
        }
        

        SignThetaUpdate(gDeltaWn);

        //gSignDegOffset = 2 * cos(gSignTheta + _gDegLast * 0.5) * sin(_gDegLast * 0.5);


        if(count >= 800)
        {
            gAcCurRefK += 0.00025;
        }
        else
        {
            gAcCurRefK = 0;
        }
        if(gAcCurRefK >= 1)
        {
            //count = 0;
            gAcCurRefK = 1;
        }

        gGvOutAnlz.Vin = ac_cur_ref;
        SINEANALYZER_DIFF_F_FUNC(&gGvOutAnlz);
        /*如果电流环给定的有效值超过了最大电流限制，则按比例缩减电流给定*/
        if(gGvOutAnlz.Vrms > gAcCurSet / 1.414 * 1.5)
        {
            ac_cur_ref /= gGvOutAnlz.Vrms / (gAcCurSet / 1.414 * 1.5);
        }

        gPwm = GiLoopCntl(ac_cur_ref, gAlgoInfo->sensor.inv_cur);
        //gPwm = gPwm + gAcVolOpenLoopRatio * BUS_VOLTAGE_NORMAL / gAlgoInfo->sensor.bus_vol * gSignSinWithDegOffset;
        float cur_extra_add = KCURRENT_EXTRA * gAlgoInfo->current_extra;
        if(cur_extra_add > 0.1f)
        {
            cur_extra_add =  0.1f;
        }
        else if(cur_extra_add <  -0.1f)
        {
            cur_extra_add =  -0.1f;
        }
        gPwm = gPwm + open_loop_ratio - cur_extra_add;
        gPwm = InvDutyLimit(gPwm);
        gUnConnectRlyCount = count;
        if(GetGridToGenFinish() == 0)
        {
            RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
        }
    }
    if(GetGridToGenFinish() == 0)
    {
        RelaySetValue(EN_RELAY_DEF_LOAD, gAlgoInfo->discharge_enable);
    }
}
/**
 * @brief
 * @param GetGenLoadFault
 * @param 获取逆变运行状态
 * @return
 */
uint16_t GetInvRunstate(void)
{
    return (gInvRunstate);
}
