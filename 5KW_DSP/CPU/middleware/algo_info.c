#include "algo_info.h"

#include "CountTrigger.h"
#include "dev_sensor.h"
#include "rms.h"
#include <voltagesourceinvlcfltr.h>
#include "algo_err_check.h"
#include "llc_dcdc_cntl.h"
#include "pll.h"
#include <Filter.h>
#include "pwr_compute.h"
#include "generator.h"
#include "modbus_analysis.h"

#include "cla_algo.h"
#pragma DATA_SECTION(gAlgoMsg,"CpuToCla1MsgRAM");
AlgoMsg_t gAlgoMsg;
AlgoInfo_t _gState_algo;
#define VAR_LOAD_OFFSET 0.0f //负载端口增加X电容，所加无功功率补偿
// static SPLL_1ph_F _gGridVolPll;
// static SPLL_1ph_F _gInvVolPll;
// static SPLL_1ph_F _gGridCurPll;
// static SPLL_1ph_F _gInvCurPll;
// static SPLL_1ph_F _gCT_CurPll; //
// static SPLL_1ph_F _gLoad_CurPll; //

//static InverterState_t* _gInvState;

static Rms_t _gGridVolPllAnlz;          //电流环给定信号分析器
// static Rms_t _gGridVolAnlz;          //电流环给定信号分析器
// static Rms_t _gInvVolAnlz;        //电流环给定信号分析器
// static Rms_t _gLoadVolAnlz;        //电流环给定信号分析器
// static Rms_t _gGridCurAnlz;          //电流环给定信号分析器
// static Rms_t _gInvCurAnlz;        //电流环给定信号分析器
// static Rms_t _gLoadCurAnlz;        //电流环给定信号分析器
// static Rms_t _gCTCurAnlz;        //电流环给定信号分析器

static CountTrigger_t _gRmsCalcTrigger;

static NOTCH_COEFF_F _gCheckPhaseNotchCoeff;    //相位偏差陷波器配置
static NOTCH_VARS_F _gCheckPhaseNotchVars;      //相位偏差陷波器参数

static NOTCH_COEFF_F _gPwrDegNotchCoeff;    //相位偏差陷波器配置
static NOTCH_VARS_F _gPwrDegNotchVars;      //相位偏差陷波器参数

static float _gPwrErrSum = 0;
static float _gBatCurSum = 0;
static uint16_t _gEnDischarge = 1;
static uint8_t _gEnCharge = 1;
static FilterComplementary_t _gGridVolFilter;
static FilterComplementary_t _gGridVolFreqFilter;
static FilterComplementary_t _gBusVolRmsFilter;
static FilterComplementary_t _gGridVolRmsFilter;
static FilterComplementary_t _gstGridRFilter;
float gGridRFilter;
float gGridR;
float gDeltaP;
float Inv_voltage;

float WATCH_CT_Power = 0;
float WATCH_INV_AC_Port_Power = 0;
float WATCH_INV_Total_Power = 0; // INV_AC_Port + INV_Backup_Port

static void _SensorUpdate()
{
    _gState_algo.sensor.bat_cur = gstSensorData.bat_current;
    _gState_algo.sensor.bat_vol = gstSensorData.bat_voltage;
    _gState_algo.sensor.bus_vol = gstSensorData.bus_voltage;
    _gState_algo.sensor.grid_cur = gstSensorData.power_grid_current;
    _gState_algo.sensor.grid_vol = gstSensorData.power_grid_voltage;
    _gState_algo.sensor.inv_cur = gstSensorData.inductor_current;
    _gState_algo.sensor.inv_vol = gstSensorData.invert_voltage;

    Inv_voltage = gstSensorData.invert_voltage;

    _gState_algo.sensor.load_cur = gstSensorData.load_current;
    _gState_algo.sensor.load_vol = gstSensorData.load_voltage;
    _gState_algo.sensor.pv1_cur = gstSensorData.pv1_current;
    _gState_algo.sensor.pv1_vol = gstSensorData.pv1_voltage;
    _gState_algo.sensor.pv2_cur = gstSensorData.pv2_current;
    _gState_algo.sensor.pv2_vol = gstSensorData.pv2_voltage;
    _gState_algo.sensor.CT_cur = gstSensorData.CT_current;

    _gState_algo.grid_pwr = _gState_algo.grid_cur_rms * _gState_algo.grid_vol_rms;
    _gState_algo.inv_pwr = _gState_algo.inv_cur_rms * _gState_algo.inv_vol_rms;
    _gState_algo.load_pwr = _gState_algo.load_cur_rms * _gState_algo.load_vol_rms;
    _gState_algo.CT_pwr = _gState_algo.ct_cur_rms * _gState_algo.grid_vol_rms;
    gAlgoMsg.grid_vol = _gState_algo.sensor.grid_vol;
    gAlgoMsg.inv_vol = _gState_algo.sensor.inv_vol;
    gAlgoMsg.load_vol = _gState_algo.sensor.load_vol;
    gAlgoMsg.grid_cur = _gState_algo.sensor.grid_cur;
    gAlgoMsg.inv_cur = _gState_algo.sensor.inv_cur;
    gAlgoMsg.load_cur = _gState_algo.sensor.load_cur;
    gAlgoMsg.ct_cur = _gState_algo.sensor.CT_cur;

    WATCH_CT_Power          = _gState_algo.CT_active_power;
    WATCH_INV_AC_Port_Power = _gState_algo.grid_active_power;
    WATCH_INV_Total_Power   = _gState_algo.inv_active_power;

    // static uint32_t Algo_j = 0;
    // static uint32_t Algo_i = 0;
    // Algo_j++;
    // if (Algo_j >= 10)gSysSetInfo.ct_or_meter
    // {
    //     Algo_j = 0;
    //     WatchData1[Algo_i] = _gState_algo.sensor.CT_cur; 
    //     Algo_i++;
    //     if (Algo_i>=200)
    //         Algo_i = 0;


    // }


}

const AlgoInfo_t* AlgoStateGet()
{
    return &_gState_algo;
}

static void _RmsCalc(void* param)
{
    static int count = 0;
    static float p_last = 0, v_last = 0;
    static int r_count = 0;
    if(++count >= 5)
    {
        count = 0;
    }
    RmsCalcStep(&_gGridVolPllAnlz, _gGridVolFilter.filter_base.output);
    switch(count) //分时计算
    {
        case 0:
        {

            _gState_algo.grid_cur_rms = gstGridAnlz.cur_rms;
            _gState_algo.grid_vol_rms = gstGridAnlz.vol_rms;
            _gState_algo.grid_pwr = gstGridAnlz.pwr_apperant_out;
            float v = _gState_algo.grid_vol_rms;
            float p = _gState_algo.grid_pwr;
            if(++r_count >= 80 * 10)
            {
                r_count = 0;
                float delta_v = (v - v_last);
                float delta_p = p - p_last;
                if(_gState_algo.inv_state != EN_INV_STATE_DEF_CONNECTED)
                {
                    gGridRFilter = FilterComplementaryUpdate(&_gstGridRFilter, 0);
                    p_last = p;
                    v_last = v;
                }
                else
                {
                    gDeltaP = delta_p;
                    if(fabsf(delta_p) > 1.0)
                    {
                        float r = p * 4 * delta_v * delta_v / delta_p / delta_p;
                        gGridR = r;
                        gGridRFilter = FilterComplementaryUpdate(&_gstGridRFilter, gGridR);
                        v_last = v;
                        p_last = p;
                        _gState_algo.grid_omiga = gGridRFilter;
                    }
                }
            }
            break;
        }
        case 1:
        {
            _gState_algo.inv_cur_rms = gstInvAnlz.cur_rms;
            _gState_algo.inv_vol_rms = gstInvAnlz.vol_rms;
            break;
        }
        case 2:
        {
            _gState_algo.load_cur_rms = gstLoadAnlz.cur_rms;
            _gState_algo.load_vol_rms = gstLoadAnlz.vol_rms;
            break;
        }
        case 3:
        {
            _gState_algo.ct_cur_rms = gstCtAnlz.cur_rms;
            break;
        }
        case 4:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    //_gState_algo.inv_power_factor = _gPwrErrSum / 10 * 2;
    _gPwrErrSum = 0;

}
extern SysInfoSet_t gSysSetInfo;
void AlgoPwrInfoCalc()
{
    _gState_algo.CT_power_factor = gGridVolPll.cos[0] * gCtCurPll.cos[0] + gGridVolPll.sin[0] * gCtCurPll.sin[0];
    _gState_algo.CT_power_refactor = gGridVolPll.sin[0] * gCtCurPll.cos[0] - gGridVolPll.cos[0] * gCtCurPll.sin[0];
    _gState_algo.grid_power_factor = gGridVolPll.cos[0] * gGridCurPll.cos[0] + gGridVolPll.sin[0] * gGridCurPll.sin[0];
    _gState_algo.grid_power_refactor = gGridVolPll.sin[0] * gGridCurPll.cos[0] - gGridVolPll.cos[0] * gGridCurPll.sin[0];
    _gState_algo.inv_power_factor = gInvVolPll.cos[0] * gInvCurPll.cos[0] + gInvVolPll.sin[0] * gInvCurPll.sin[0];
    _gState_algo.inv_power_refactor = gInvVolPll.sin[0] * gInvCurPll.cos[0] - gInvVolPll.cos[0] * gInvCurPll.sin[0];
    _gState_algo.load_power_factor = -(gInvVolPll.cos[0] * gLoadCurPll.cos[0] + gInvVolPll.sin[0] * gLoadCurPll.sin[0]);  //Load_Cur与电压反相
    _gState_algo.load_power_refactor = -(gInvVolPll.sin[0] * gLoadCurPll.cos[0] - gInvVolPll.cos[0] * gLoadCurPll.sin[0]); //Load_Cur与电压反相
    // _gState_algo.grid_active_power = _gState_algo.grid_pwr * _gState_algo.grid_power_factor;
    // _gState_algo.grid_reactive_power = _gState_algo.grid_pwr * _gState_algo.grid_power_refactor;
    // _gState_algo.inv_active_power = _gState_algo.inv_pwr * _gState_algo.inv_power_factor;
    // _gState_algo.inv_reactive_power = _gState_algo.inv_pwr * _gState_algo.inv_power_refactor;
    // _gState_algo.CT_active_power = _gState_algo.CT_pwr * _gState_algo.CT_power_factor;
    // _gState_algo.CT_reactive_power = _gState_algo.CT_pwr * _gState_algo.CT_power_refactor;
    // _gState_algo.load_active_power = _gState_algo.load_pwr * _gState_algo.load_power_factor;
    // _gState_algo.load_reactive_power = _gState_algo.load_pwr * _gState_algo.load_power_refactor + VAR_LOAD_OFFSET;
    _gState_algo.grid_active_power = gstGridAnlz.pwr_active_out;
    _gState_algo.grid_reactive_power = gstGridAnlz.pwr_reactive_out;
    _gState_algo.inv_active_power = gstInvAnlz.pwr_active_out;
    _gState_algo.inv_reactive_power = gstInvAnlz.pwr_reactive_out;
    _gState_algo.CT_active_power =  gstCtAnlz.pwr_active_out; // 20260519，补充备注：设备CT端实时并网功率，正数为买电，负数为卖电，例：-4000表示从电网买电流向CT端的功率为4000W
    _gState_algo.CT_reactive_power = gstCtAnlz.pwr_reactive_out;
    _gState_algo.load_active_power = gstLoadAnlz.pwr_active_out;
    _gState_algo.load_reactive_power = gstLoadAnlz.pwr_reactive_out;
    _gState_algo.norload_active_power = _gState_algo.CT_active_power - _gState_algo.grid_active_power;
    _gState_algo.norload_reactive_power = _gState_algo.CT_reactive_power - _gState_algo.grid_reactive_power;
    _gState_algo.norload_pwr = sqrtf(_gState_algo.norload_active_power * _gState_algo.norload_active_power + _gState_algo.norload_reactive_power * _gState_algo.norload_reactive_power);
}

float Inv_state;
float PV1_Current;
float PV2_Current;
float PV_set_power;
void AlgoStateUpdate()
{
    static int count = 0;
    _SensorUpdate();
    _gState_algo.current_extra = _gState_algo.sensor.inv_cur + _gState_algo.sensor.load_cur + _gState_algo.sensor.grid_cur;
    static float value_ave = 0, gird_cur_rms_ave = 0;
    static float ct_cur_rms_ave = 0;
    value_ave = value_ave * 0.9995 + _gState_algo.sensor.grid_cur * 0.0005;
    gird_cur_rms_ave = gird_cur_rms_ave * 0.9995 + _gState_algo.grid_cur_rms * 0.0005;
    ct_cur_rms_ave = ct_cur_rms_ave * 0.9995 + _gState_algo.ct_cur_rms * 0.0005;
    _gState_algo.grid_cur_ave = value_ave;
    _gState_algo.grid_cur_rms_ave = gird_cur_rms_ave;
    _gState_algo.ct_cur_rms_ave = ct_cur_rms_ave;

    //CountTriggerRun(&_gRmsCalcTrigger, 1);
    FilterComplementaryUpdate(&_gGridVolFilter, _gState_algo.sensor.grid_vol);
    _RmsCalc(0);
    if(++count >= 1000)
    {
        count = 0;
        _gState_algo.bus_vol_dt = _gState_algo.sensor.bus_vol - _gState_algo.bus_vol_last;
        _gState_algo.bus_vol_last = _gState_algo.sensor.bus_vol;
        _gState_algo.bat_cur_ave = _gBatCurSum / 1000.0;
        _gBatCurSum = 0;
    }

    _gState_algo.grid_vol_phase = gGridVolPll.sin[0];
    _gState_algo.grid_cur_phase = gGridCurPll.sin[0];
    _gState_algo.inv_vol_phase = gInvVolPll.sin[0];
    _gState_algo.inv_cur_phase = gInvCurPll.sin[0];
    _gState_algo.CT_cur_phase = gCtCurPll.sin[0];
    _gState_algo.load_cur_phase = gLoadCurPll.sin[0];

    _gState_algo.grid_phase_err = PllGetSignPhaseErr(_gState_algo.grid_vol_phase, _gState_algo.inv_vol_phase, &_gCheckPhaseNotchVars, &_gCheckPhaseNotchCoeff) * 2 - 1;
    //_gState_algo.power_deg_err = PllGetSignPhaseErr(_gInvVolPll.sin[0], _gInvCurPll.sin[0], &_gPwrDegNotchVars, &_gPwrDegNotchCoeff);
    //_gPwrErrSum += _gState_algo.power_deg_err;

    _gState_algo.grid_vol_phase_cos = gGridVolPll.cos[0];
    _gState_algo.grid_cur_phase_cos = gGridCurPll.cos[0];
    _gState_algo.inv_vol_phase_cos = gInvVolPll.cos[0];
    _gState_algo.inv_cur_phase_cos = gInvCurPll.cos[0];
    _gState_algo.CT_cur_phase_cos = gCtCurPll.cos[0];
    _gState_algo.grid_deg_err = _gState_algo.grid_vol_phase * _gState_algo.inv_vol_phase_cos - _gState_algo.grid_vol_phase_cos * _gState_algo.inv_vol_phase;

    _gState_algo.inv_vol_theta = gInvVolPll.theta[0];

    _gState_algo.inv_start_ok = gInvStartOk;
    _gState_algo.inv_release_ok = gInvReleaseOk;
    _gState_algo.inv_enable = gEnInvPwm;
    _gState_algo.llc_start_ok = gIsLlcStartupOk;
    _gState_algo.llc_enable = gEnLlcPwm;
    _gState_algo.sys_enable = (_gState_algo.inv_enable && _gState_algo.llc_enable);
    _gState_algo.pv1_vol_ave = gPv1_Voltage_Ave > 20 ? gPv1_Voltage_Ave : 0;
    _gState_algo.pv2_vol_ave = gPv2_Voltage_Ave > 20 ? gPv2_Voltage_Ave : 0;
    _gState_algo.pv1_cur_ave = gPv1_Current_Ave;
    _gState_algo.pv2_cur_ave = gPv2_Current_Ave;
    _gState_algo.pv1_pwr = gstPv1Data.pv_pwr_out;
    _gState_algo.pv2_pwr = gstPv2Data.pv_pwr_out;
    _gState_algo.pv_set_pwr = gPvPwrRef;
    _gState_algo.pv_state = Read_Mppt_state();
    _gState_algo.inv_state = InvGetState();

    PV1_Current = gPv1_Current_Ave;
    PV2_Current = gPv2_Current_Ave;
    PV_set_power = gPvPwrRef;

    Inv_state = InvGetState();

    if((_gState_algo.grid_vol_rms >= 180.0f) && (_gState_algo.grid_vol_rms <= 280.0f))
    {
        _gState_algo.grid_freq = FilterComplementaryUpdate(&_gGridVolFreqFilter, _gGridVolPllAnlz.sig_freq);
    }
    else
    {
        _gState_algo.grid_freq = 0.0;
    }

    _gState_algo.inv_freq = gstInvAnlz.sig_freq;

    _gBatCurSum += _gState_algo.sensor.bat_cur;
    _gState_algo.discharge_enable = _gEnDischarge;
    _gState_algo.charge_enable = _gEnCharge;
    _gState_algo.bus_vol_filter = FilterComplementaryUpdate(&_gBusVolRmsFilter, _gState_algo.sensor.bus_vol);
    _gState_algo.grid_vol_rms_filter = FilterComplementaryUpdate(&_gGridVolRmsFilter, _gState_algo.grid_vol_rms);
// _gState_algo.sys_enable = 1;//调试用启机
// _gState_algo.llc_start_ok = 1;
}

void RelaySetValue(EN_RELAY_DEF relay, uint16_t value)
{
    if(value != 0 && value != 1)
        return;
    switch(relay)
    {
        case EN_RELAY_DEF_MAIN:
            _gState_algo.relay.rly_main = value;
            break;
        case EN_RELAY_DEF_LOAD:
            _gState_algo.relay.rly_load = value;
            break;
        case EN_RELAY_DEF_GRID:
            _gState_algo.relay.rly_grid = value;
            break;
        case EN_RELAY_DEF_ENGINE:
            _gState_algo.relay.rly_engine = value;
            break;
        case EN_RELAY_DEF_GRID_RECTIFY:
            _gState_algo.relay.rly_grid_rectify = value;
            break;
        case EN_RELAY_DEF_ENGINE_RECTIFY:
            _gState_algo.relay.rly_engine_rectify = value;
            break;
        default:;
    }
}

void AlgoStateInit()
{
    RmsInit50Hz(&_gGridVolPllAnlz, 1);

    CountTriggerInit(&_gRmsCalcTrigger, 2);
    _gRmsCalcTrigger.trig_hook = _RmsCalc;

    NOTCH_FLTR_F_VARS_init(&_gCheckPhaseNotchVars);
    NOTCH_FLTR_F_COEFF_Update(1.0 / ISR_CONTROL_FREQUENCY, 200 * 3.1415, 0.00001, 1.0, &_gCheckPhaseNotchCoeff);

    NOTCH_FLTR_F_VARS_init(&_gPwrDegNotchVars);
    NOTCH_FLTR_F_COEFF_Update(1.0 / ISR_CONTROL_FREQUENCY, 200 * 3.1415, 0.00001, 1.0, &_gPwrDegNotchCoeff);

    FilterComplementaryInit(&_gGridVolFilter, 0.1);
    FilterComplementaryInit(&_gGridVolFreqFilter, 0.0005);
    FilterComplementaryInit(&_gGridVolRmsFilter, 0.0001);
    FilterComplementaryInit(&_gBusVolRmsFilter, 0.0001);
    FilterComplementaryInit(&_gstGridRFilter, 0.1);

}

void AlgoEnDischarge(uint16_t state)
{
    _gEnDischarge = state != 0;
}

void AlgoEnCharge(uint16_t state)
{
    _gEnCharge = state != 0;
}
uint16_t GetGridRlyState(void)
{
    return (_gState_algo.relay.rly_grid);
}
uint16_t GetEngineRlyState(void)
{
    return (_gState_algo.relay.rly_engine);
}
uint16_t GetMainRlyState(void)
{
    return (_gState_algo.relay.rly_main);
}
uint16_t GetLoadRlyState(void)
{
    return (_gState_algo.relay.rly_load);
}
