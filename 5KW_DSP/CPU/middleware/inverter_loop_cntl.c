/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        inverter_loop_cntl.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-4-10
  * @brief       逆变器控制算法文件
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-4-10    <td>V1.0.0    <td>dengjiahui    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "inverter_loop_cntl.h"
#include "cla_algo.h"

#include <IQmathlib.h>
#include <math.h>

#include "dev_sensor.h"
#include "voltagesourceinvlcfltr.h"

#include "Solar_F.h"
#include "CNTL_PI_F.h"
#include <SFRA_F_Include.h>

#include "algo_err_check.h"
#include "algo_info.h"
#include "CountTrigger.h"
#include "llc_dcdc_cntl.h"
#include "pll.h"
#include  "SINEANALYZER_DIFF_F.h"
#include "generator.h"

#include "sci.h"

#include "../cntl/TimeTrigger.h"
#include "FSM.h"
#include "inverter_state_def.h"
#include "pwm_cntl.h"
#include "CNTL_2P2Z_F.h"
#include "Filter.h"

#include "activation.h"
#include "inverter_func_def.h"
#include "algorithm_interface.h"
#include "inverter_func_def.h"
#include "continus_operation.h"
//TODO:操作并网继电器的接口实现
#define BLINKY_LED_GPIO    75
#define DSP_CRASH       86
#define RELAY0          87
#define RELAY1          88
#define RELAY2          89
#define RELAY3          90
#define RELAY4          91
#define RELAY5          92
#define ANALY_DELTA     45
#define ANALY_FREQ      (ISR_CONTROL_FREQUENCY/10)

CNTL_PI_F gGiPi;                        //电流环PI控制器
CNTL_PI_F gGvPi;                        //电压环PI控制器
CNTL_2P2Z_F_COEFFS gGvPrCoeffs;
CNTL_2P2Z_F_VARS gGvPrVars;

SINEANALYZER_DIFF_F gGvOutAnlz;         //电流环给定信号分析器
Pll_t gPll, gPllOffset;                             //电网锁相环
DelayTrigger_t gGridConnectTrig;
HoldTrigger_t gIsletRunTrig;
TimeTrigger_t gEnIsletTrig;
DelayTrigger_t gIsletCheckTrig;
DelayTrigger_t gDisConnectGridPhaseTrigger;
CountTrigger_t gGridRlyDelayTrigger;
DelayTrigger_t gIsletCurCheckTrigger;


float32 garrSignSin[2] = { 0,0 };       //内部sin信号
float32 garrSignCos[2] = { 1,1 };       //内部cos信号
float32 garrStdSignSin[2] = { 0,0 };       //内部标准sin信号
float32 garrStdSignCos[2] = { 1,1 };       //内部标准cos信号
float gSignStamp = 0;                   //内部斜坡信号
float gStampSpeed = 0.000005;            //斜坡信号自增速率
float gAcVolOpenLoopRatio = OPEN_LOOP_RADIO;        //逆变电压开环系数
float gAcCurSet = 125;                   //逆变电流峰峰值最大限制设置
float gAcVolSet = INV_VOL_SET;                  //逆变电压峰峰值设置
float gAvVolRef = 0;
float gGridConnectedCurRef = 0;         //并网电流给定
InverterState_t gState;                 //逆变器状态
static float32 _garrSciBuf[16];                  //Sci发送缓冲
uint8_t gEnGridConnect = 0;             //使能并网
uint16_t gGridConnectedTime = 0;
uint32_t gGridDisConnectedTime = 0;

EN_GRID_CONNECT_STA gStaGridConnect = EN_GRID_CONNECT_STA_UNCONNECT;//并网状态
float gGridPhaseErr = 0;                //相位偏差
float gVolPwrOffset = 0;
float gAcCurRefK;

float gPwm = 0;        //逆变器输出PWM占空比
float gPwmK = 0;       //逆变器控制器输出比例
uint8_t gInvStartOk = 0, gInvReleaseOk = 0;

uint8_t gEnFreqAdjust = 0;

uint8_t gInvErr = 0;

const AlgoInfo_t* gAlgoInfo;
float gDeltaWn;

static Fsm_t _gFsm;
uint8_t gIsLet = 0;
uint8_t gGridRlyActived = 0;
float gDegOffset = -0.03;
float gGridFreqLast = 0;
uint16_t gIsletTimeCount = 0, gEnIsletCompare = 0, gIsletErrCount = 0;
float gSignSinWithDegOffset = 0;
uint8_t gIsletTrueFlag = 0;
float gIsLetSum = 0, gIsLetAve = 0, gIsLetAveLast = 0, gIsLetAveErr = 0, gIsLetAveErrLast = 0;
float gIsLetErr, gIsLetErrReal;
float gSignSin, gSignCos;
float gSignTheta, gSignThetaDeg, gSignDegOffset, gSignDegOutput;

DelayTrigger_t gstGridVolLowTrigger;
static void _EnableAdjustFreq(void* param)
{
    gEnFreqAdjust = 1;
}

static void _DisEnableAdjustFreq(void* param)
{
    gEnFreqAdjust = 0;

}

static void _CheckIslet(void* param)
{
    if(gEnIsletCompare == 1)
        gIsletErrCount++;
}

static void _ConnectReady(void* param)
{
    ScidSendMsgToBuff("ready to connect\n");
}

static void _GridRlyActived(void* param)
{
    gGridRlyActived = 1;
}

/**
 * @brief 初始化逆变器控制
 */
void InverterLoopInit()
{
    CNTL_PI_F_init(&gGvPi);
    gGvPi.Kp = 0.12;
    gGvPi.Ki = 0.2;
    gGvPi.Umax = 40;
    gGvPi.Umin = -40;
    CNTL_PI_F_init(&gGiPi);
#ifdef LOW_VOL_TEST
    gGiPi.Kp = 0.015;
#else
#ifdef MID_VOL_TEST
    gGiPi.Kp = 0.005;
#else
    gGiPi.Kp = 0.002;
#endif
#endif
    gGiPi.Ki = 0;
    gGiPi.Umax = 1;
    gGiPi.Umin = -1;

    CNTL_2P2Z_F_COEFFS_init(&gGvPrCoeffs);
    float32 w2 = 314.15 * 314.15 * 0.00005 * 0.00005;
    float32 ki = 10;
    float32 kp = 0.1;
    float32 wc = 3.14 * 12;
    float32 wct = wc * 0.00005;
    float32 a0 = 1 + wct + w2 / 4.0;
    gGvPrCoeffs.Coeff_A1 = -1 * (-2 + w2 / 2) / a0;
    gGvPrCoeffs.Coeff_A2 = -1 * (1 - wct + w2 / 4.0) / a0;
    gGvPrCoeffs.Coeff_B0 = (kp + wct * (kp + ki) + w2 / 4 * kp) / a0;
    gGvPrCoeffs.Coeff_B1 = (-2 * kp + w2 / 2 * kp) / a0;
    gGvPrCoeffs.Coeff_B2 = (kp - wct * (kp + ki) + w2 / 4 * kp) / a0;
    //gGvPrCoeffs.Coeff_A1 = (-2.0 * (w2 - 4.0) / (w2 + 4.0));
    //gGvPrCoeffs.Coeff_A2 = (-1.0);
    //gGvPrCoeffs.Coeff_B0 = (kp + 2.0 * ki * 0.00005 / (4.0 + w2));
    //gGvPrCoeffs.Coeff_B1 = (2.0 * kp * (w2 - 4.0) / (w2 + 4.0));
    //gGvPrCoeffs.Coeff_B2 = (kp - 2.0 * ki * 0.00005 / (4.0 + w2));
    gGvPrCoeffs.Max = 125;
    gGvPrCoeffs.Min = -125;
    gGvPrCoeffs.IMin = -125;
    CNTL_2P2Z_F_VARS_init(&gGvPrVars);

    SINEANALYZER_DIFF_F_init(&gGvOutAnlz);
    gGvOutAnlz.SampleFreq = ISR_CONTROL_FREQUENCY;                    //采样频率
    gGvOutAnlz.Threshold = 0;                                      //信号过零阈值
    /*信号采样数范围，采样频率 / （电网频率容许范围）*/
    gGvOutAnlz.nsamplesMax = ISR_CONTROL_FREQUENCY / (50 - ANALY_DELTA);
    gGvOutAnlz.nsamplesMin = ISR_CONTROL_FREQUENCY / (50 + ANALY_DELTA);

    PllInit(&gPll);
    PllInit(&gPllOffset);

    DelayTriggerInit(&gGridConnectTrig, 2000);
    DelayTriggerInit(&gIsletCheckTrig, 35);
    HoldTriggerInit(&gIsletRunTrig, 15, 0);
    TimeTriggerInit(&gEnIsletTrig, 500);
    DelayTriggerInit(&gDisConnectGridPhaseTrigger, 500);
    DelayTriggerInit(&gIsletCurCheckTrigger, 1000);
    CountTriggerInit(&gGridRlyDelayTrigger, 400);
    gEnIsletTrig.trig_hook = _EnableAdjustFreq;
    gIsletRunTrig.trig_hook = _DisEnableAdjustFreq;
    gGridConnectTrig.trig_hook = _ConnectReady;
    gGridRlyDelayTrigger.trig_hook = _GridRlyActived;
    gIsletCheckTrig.trig_hook = _CheckIslet;

    gAlgoInfo = AlgoStateGet();

    _gFsm.current_state = &gStateClose;
    DelayTriggerInit(&gstGridVolLowTrigger, 10);

}

/**
 * @brief 电压环调节
 * @param ref 电压环给定
 * @param fbk 电压环反馈
 * @return 控制器输出
 */
float GvLoopCntl(float ref, float fbk)
{
    /*gGvPi.Ref = ref;
    gGvPi.Fbk = fbk;

    CNTL_PI_F_FUNC(&gGvPi);*/

    gGvPrVars.Ref = ref;
    gGvPrVars.Fdbk = fbk;
    CNTL_2P2Z_F_ASM(&gGvPrCoeffs, &gGvPrVars);

    return gGvPrVars.Out;

}

/**
 * @brief 电流环调节
 * @param ref 电流环给定
 * @param fbk 电流环反馈
 * @return 控制器输出
 */
float GiLoopCntl(float ref, float fbk)
{
    gGiPi.Ref = ref;
    gGiPi.Fbk = fbk;

    CNTL_PI_F_FUNC(&gGiPi);

    return gGiPi.Out;

}

/**
 * @brief 更新逆变器当前状态
 */
static void _StateUpdate()
{
    gState.sign_sin = garrSignSin[0];
    gState.sign_cos = garrSignCos[0];
    gState.sign_stamp = gSignStamp;
    gState.sta_gv_ref = gGvPi.Ref;
    gState.sta_gv_out = gGvPi.Out;
    gState.sta_gi_ref = gGiPi.Ref;
    gState.sta_gi_out = gGiPi.Out;
    gState.sta_gi_ref_rms = gGvOutAnlz.Vrms;
    gState.sta_grid_phase_err = gGridPhaseErr;
    gState.sta_grid_connect = gStaGridConnect;
    gState.sta_grid_vol_rms = gAlgoInfo->grid_vol_rms;
    gState.sta_inv_vol_rms = gAlgoInfo->inv_vol_rms;
    gState.sta_load_vol_rms = gAlgoInfo->load_vol_rms;
    gState.sta_grid_cur_rms = gAlgoInfo->grid_cur_rms;
    gState.sta_inv_cur_rms = gAlgoInfo->inv_cur_rms;
    gState.sta_load_cur_rms = gAlgoInfo->load_cur_rms;
    gState.sta_grid_vol_phase = gAlgoInfo->grid_vol_phase;
    gState.sta_grid_cur_phase = gAlgoInfo->grid_cur_phase;
    gState.sta_grid_ct_phase = gAlgoInfo->CT_cur_phase;
    gState.sta_inv_vol_phase = gAlgoInfo->inv_vol_phase;
    gState.sta_inv_cur_phase = gAlgoInfo->inv_cur_phase;
    gState.sta_grid_vol_phase_cos = gAlgoInfo->grid_vol_phase_cos;
    gState.sta_grid_cur_phase_cos = gAlgoInfo->grid_cur_phase_cos;
    gState.sta_inv_vol_phase_cos = gAlgoInfo->inv_vol_phase_cos;
    gState.sta_inv_cur_phase_cos = gAlgoInfo->inv_cur_phase_cos;
    gState.sta_inv_active_pwr = gAlgoInfo->inv_active_power;
    gState.sta_inv_reactive_pwr = gAlgoInfo->inv_reactive_power;
    gState.sta_inv_freq = gAlgoInfo->inv_freq;
}

/**
 * @brief 更新内部信号源
 * @param delta_wn 信号源更新频率相对于基频的偏差
 */
static void _SignUpdate(float delta_wn)
{
    float wn = 314.15 + delta_wn;
    if(wn > 376.98)
    {
        wn = 376.98;
    }
    if(wn < 251.32)
    {
        wn = 251.32;
    }
    garrSignSin[0] = garrSignSin[1] + wn / ISR_CONTROL_FREQUENCY * garrSignCos[1];    //sin[n]=sin[n-1]+wT*cos[n-1],wT=2*Pi*50*(1/20K)=0.0157079632675
    garrSignCos[0] = garrSignCos[1] - wn / ISR_CONTROL_FREQUENCY * garrSignSin[1];    //cos[n]=cos[n-1]-wT*sin[n-1],wT=2*Pi*50*(1/20K)=0.0157079632675
    if(garrSignSin[0] > 1)
    {
        garrSignSin[0] = 1;
    }
    else if(garrSignSin[0] < -1)
    {
        garrSignSin[0] = -1;
    }
    if(garrSignCos[0] > 1)
    {
        garrSignCos[0] = 1;
    }
    else if(garrSignCos[0] < -1)
    {
        garrSignCos[0] = -1;
    }
    garrSignSin[1] = garrSignSin[0];
    garrSignCos[1] = garrSignCos[0];

    garrStdSignSin[0] = garrStdSignSin[1] + 0.0157 * garrStdSignCos[1];    //sin[n]=sin[n-1]+wT*cos[n-1],wT=2*Pi*50*(1/20K)=0.0157079632675
    garrStdSignCos[0] = garrStdSignCos[1] - 0.0157 * garrStdSignSin[1];    //cos[n]=cos[n-1]-wT*sin[n-1],wT=2*Pi*50*(1/20K)=0.0157079632675
    if(garrStdSignSin[0] > 1)
    {
        garrStdSignSin[0] = 1;
    }
    else if(garrStdSignSin[0] < -1)
    {
        garrStdSignSin[0] = -1;
    }
    if(garrStdSignCos[0] > 1)
    {
        garrStdSignCos[0] = 1;
    }
    else if(garrStdSignCos[0] < -1)
    {
        garrStdSignCos[0] = -1;
    }
    garrStdSignSin[1] = garrStdSignSin[0];
    garrStdSignCos[1] = garrStdSignCos[0];
}

/**
 * @brief 逆变器双环控制
 * @param
 */
void InverterLoopCntl(void)
{
    static uint16_t count = 0;
    _SignUpdate(gDeltaWn);

    FsmHandler(&_gFsm);

    PwmSetInvDuty(gPwm);

    _StateUpdate();

    if(gstSensorData.temp[4] > 850 || gstSensorData.temp[5] > 800)
    {
        gAcCurSet -= 0.00005;
    }
    else
    {
        gAcCurSet += 0.00005;
    }
    if(gAcCurSet > 125)
    {
        gAcCurSet = 125;
    }
    if(gAcCurSet < 1)
    {
        gAcCurSet = 1;
    }

    if(++count >= 800)
    {
        if(gAlgoInfo->inv_cur_rms != 0)
        {
            if(gAlgoInfo->inv_cur_rms > INV_PWR_LIMIT_CUR)
            {
                gGridConnectedCurRef *= INV_PWR_LIMIT_CUR / gAlgoInfo->inv_cur_rms;
            }
        }
        count = 0;
    }

    if(gAlgoInfo->inv_state == EN_INV_STATE_DEF_CONNECTED)
    {
        gStaGridConnect = EN_GRID_CONNECT_STA_CONNECTED;
    }
    else if(gAlgoInfo->inv_state == EN_INV_STATE_DEF_CONNECTING)
    {
        gStaGridConnect = EN_GRID_CONNECT_STA_CONNECTING;
    }
    else
    {
        gStaGridConnect = EN_GRID_CONNECT_STA_UNCONNECT;
    }
    if(GetGridToGenFinish() == 1)//已切换至发电机模式
    {
        RelaySetValue(EN_RELAY_DEF_ENGINE_RECTIFY, gAlgoInfo->sensor.bat_vol > ALGOERR_THRESHOLD_BAT_VOL_LOW);
    }
    else
    {
        RelaySetValue(EN_RELAY_DEF_GRID_RECTIFY, gAlgoInfo->sensor.bat_vol > ALGOERR_THRESHOLD_BAT_VOL_LOW);
    }
}

EN_INV_STATE_DEF InvGetState()
{
    return (EN_INV_STATE_DEF)_gFsm.current_state_id;
}

/**
 * @brief 获取当前逆变器波形数据
 * @return 数据缓冲地址
 */
void* InverterGetStateWave()
{
    extern float gLimitK;
    extern float gLeak3ma,gLeakCur,gLeakCmp;
    _garrSciBuf[0] = gAlgoInfo->sensor.grid_vol;
    _garrSciBuf[1] = gAlgoInfo->sensor.inv_vol;
    _garrSciBuf[2] = gAlgoInfo->inv_state;
    _garrSciBuf[3] = gLeak3ma;
    _garrSciBuf[4] = gLeakCur;
    _garrSciBuf[5] = gLeakCmp;
    _garrSciBuf[6] = gAlgoInfo->grid_vol_rms;
    _garrSciBuf[6] = gAlgoInfo->grid_cur_rms;
//    _garrSciBuf[6] = fabsf(_gState.sta_grid_vol_rms - _gState.sta_inv_vol_rms);
    _garrSciBuf[7] = gAlgoInfo->sensor.inv_cur;
    _garrSciBuf[8] = gstInvAnlz.cur_avg;
    _garrSciBuf[9] = gstInvAnlz.vol_avg;
    _garrSciBuf[10] = gAlgoInfo->sensor.grid_cur;
    _garrSciBuf[11] = gGridConnectedCurRef;
    _garrSciBuf[12] = gPwm;
    return _garrSciBuf;
}

/**
 * @brief 发送逆变器波形
 */
void InverterSendStateWave()
{
    InverterGetStateWave();
}
/**
 * @brief 发送逆变器运行状态
 */
EN_INV_STATE_DEF GetInvRunState(void)
{
    return gAlgoInfo->inv_state;
}

/**
 * @brief 获取逆变器控制器当前状态
 * @return
 */
InverterState_t* InverterGetState()
{
    return &gState;
}

void InverterSetGridConnectedCur(float cur)
{
    if(fabsf(cur) < gAcCurSet)
    {
        gGridConnectedCurRef = cur;
    }
}

float InverterGetGridConnectedCur()
{
    return gGridConnectedCurRef;
}

void InverterSetDeg(float d)
{
    gSignThetaDeg = d;
    if(gSignThetaDeg > 0.2)
    {
        gSignThetaDeg = 0.2;
    }
    if(gSignThetaDeg < -0.2)
    {
        gSignThetaDeg = -0.2;
    }
}

float InverterGetDeg()
{
    return gSignThetaDeg;
}

/**
 * @brief 设置并网使能
 * @param state 设置值
 */
void InverterSetGridConnect(uint8_t state)
{
    if(state == 0 || state == 1)
    {
        gEnGridConnect = state;
    }
}

void InverterTest(uint8_t data)
{
    //if(data >= 20)
    //    return;
    ScidSendMsgToBuff("Test:0x%02X\n", data);
    if(data == 0x1C)
        gEnGridConnect = 1;
    else if(data == 0x4C)
        gEnGridConnect = 0;
    else if(data == 0x51)
    {
        gEnInvPwm = 1;
        gEnLlcPwm = 1;
        ActivationRunStateSet(PVACTIVATION_BATSTART);
    }
    else if(data == 0x3A)
    {
        ActivationRunStateSet(PVACTIVATION_PVSTART);
    }
    else if(data == 0x7D)
    {
        ActivationRunStateSet(PVACTIVATION_PVSTARTOK);
    }
    else if(data == 0xA4)
    {
        ActivationRunStateSet(PVACTIVATION_BATSTART);
    }
    /*else if(data == 0x5C)
    {
        gEnInvPwm = 0;
        gEnLlcPwm = 0;
    }
    else if(data == 0xA1)
    {
        gEnGridConnect = 1;
    }
    else if(data == 0xAC)
    {
        gEnGridConnect = 0;
    }
    else if(data == 0x06)
    {
        gDegOffset += 0.01;
        ScidSendMsgToBuff("->%d\n", (int)(gDegOffset * 100));
    }
    else if(data == 0x07)
    {
        gDegOffset -= 0.01;
        ScidSendMsgToBuff("->%d\n", (int)(gDegOffset * 100));
    }*/
    else if(data == 0x05)
    {
        EN_INV_STATE_DEF state = (EN_INV_STATE_DEF)_gFsm.current_state_id;
        switch(state)
        {
        case EN_INV_STATE_DEF_ERR:
            ScidSendMsgToBuff("state err");
            break;
        case EN_INV_STATE_DEF_CLOSE:
            ScidSendMsgToBuff("state close");
            break;
        case EN_INV_STATE_DEF_OPENING:
            ScidSendMsgToBuff("state opening");
            break;
        case EN_INV_STATE_DEF_DISCONNECTED:
            ScidSendMsgToBuff("state disconnected");
            break;
        case EN_INV_STATE_DEF_CONNECTING:
            ScidSendMsgToBuff("state connecting");
            break;
        case EN_INV_STATE_DEF_CONNECTED:
            ScidSendMsgToBuff("state connected");
            break;
        case EN_INV_STATE_DEF_DISCONNECTING:
            ScidSendMsgToBuff("state disconnecting");
            break;
        default:
            ScidSendMsgToBuff("state none");
        }
        ScidSendMsgToBuff("\nerr code %04X%04X\n", gAlgoErrDef.byte[1], gAlgoErrDef.byte[0]);
        ScidSendMsgToBuff("warning code %04X%04X\n", gAlgoWarningDef.byte[1], gAlgoWarningDef.byte[0]);
        ScidSendMsgToBuff("grid low:%d", gAlgoWarningDef.err.grid_vol_low);
    }
}

