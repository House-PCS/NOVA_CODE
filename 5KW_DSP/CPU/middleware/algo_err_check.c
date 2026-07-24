#include "algo_err_check.h"

#include <math.h>
#include <string.h>
#include "energy_management.h"
#include "algo_info.h"
#include "CountTrigger.h"
#include "dev_sensor.h"
#include "hard_cmpss.h"
#include "mid_comservice.h"
#include "generator.h"
#include "pwm_cntl.h"
#include "fault_diagnosis.h"
#include "mppt_process.h"

#define SCALER 5

extern int gTestErr;
int gAlgoErr = 0, gAlgoWarning = 0;
float gAlgoErrData = 0;
//uint16_t gAlgoErrCode[4] = { 0,0,0,0 }, gAlgoWarningCode[4] = { 0,0,0,0 };
uint16_t gCode = 0;
const AlgoInfo_t* _gAlgoInfo;
AlgoErrDefUn_t gAlgoErrDef;
AlgoWarningDefUn_t gAlgoWarningDef;
extern uint16_t gUnConnectRlyCount;
static EN_INV_PWR_LEVEL _geLevel;
extern FaultInfo_t gstFaultInfo;
extern uint16_t gArcFaultStandbyFlag;
extern uint16_t gSyncHostStandby;

bool GetCmpssCurOver(void)
{
    if(gAlgoErrDef.err.cmpss_cur_over == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void _SetErrCode(uint16_t code)
{
    gAlgoErrDef.byte[(code & 0xFFF0) >> 4] |= (1 << (code & 0x000F));
}

static void _SetWarningCode(uint16_t code)
{
    gAlgoWarningDef.byte[(code & 0xFFF0) >> 4] |= (1 << (code & 0x000F));
}

static void _ResetWarningCode(uint16_t code)
{
    gAlgoWarningDef.byte[(code & 0xFFF0) >> 4] &= ~(1 << (code & 0x000F));
}

uint8_t AlgoWarningGet(EN_ALGO_WARNING warning)
{
    return (gAlgoWarningDef.byte[(warning & 0xFFF0) >> 4] & (1 << warning)) != 0;
}

static void _ErroTrig(void* param)
{
    uint16_t* p_code = param;
    gAlgoErr |= 1;
    if(p_code != 0)
        _SetErrCode(*p_code);
}
static void _WarningTrig(void* param)
{
    uint16_t* p_code = param;
    gAlgoWarning |= 1;

    if(p_code != 0)
        _SetWarningCode(*p_code);
}
static void _ClearwarningTrig(void* param)
{
    uint16_t* p_code = param;
    if(p_code != 0)
        _ResetWarningCode(*p_code);
}

#define ERR_CHECK(cmd,code)  \
{cmd}                   \
if(rst==1){gAlgoErr|=1;_SetErrCode(code);}

#define WARNING_CHECK(cmd,code)  \
{cmd}                   \
if(rst==1){gAlgoWarning|=1;_SetWarningCode(code);}

#define WARNING_CHECK_CLEAR(cmd,code)  \
{cmd}                   \
if(rst==1){_ResetWarningCode(code);}

#define ERRDLY_CHECK_ALIGN(cmd,ID,threshold,code)                                         \
static CountTrigger_t errDlyTrigger_##ID=                                                 \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &gCode,                                                               \
    .trig_hook = _ErroTrig,                                                             \
    .trig_threshold = threshold / SCALER,                                                           \
};                                                                                         \
{cmd}{gCode=code;}                                                                      \
CountTriggerRun(&errDlyTrigger_##ID,rst);
#define ERRDLY_CHECK_ALIAS(cmd,ID,threshold,code) ERRDLY_CHECK_ALIGN(cmd,ID,threshold,code)
#define ERRDLY_CHECK(cmd,threshold,code) ERRDLY_CHECK_ALIAS(cmd,__LINE__,threshold,code)
#define WARNINGDLY_CHECK_ALIGN(cmd,ID,threshold,code)                                         \
static CountTrigger_t warningDlyTrigger##ID=                                                 \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &gCode,                                                               \
    .trig_hook = _WarningTrig,                                                             \
    .trig_threshold = threshold / SCALER,                                                           \
};                                                                                         \
{cmd}{gCode=code;}                                                                      \
CountTriggerRun(&warningDlyTrigger##ID,rst);
#define WARNINGDLY_CHECK_ALIAS(cmd,ID,threshold,code) WARNINGDLY_CHECK_ALIGN(cmd,ID,threshold,code)
#define WARNINGDLY_CHECK(cmd,threshold,code) WARNINGDLY_CHECK_ALIAS(cmd,__LINE__,threshold,code)

#define WARNINGDLY_CLEAR_ALIGN(cmd,ID,threshold,code)                                         \
static CountTrigger_t warningDlyClearTrigger##ID=                                                 \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &gCode,                                                               \
    .trig_hook = _ClearwarningTrig,                                                             \
    .trig_threshold = threshold / SCALER,                                                           \
};                                                                                         \
{cmd}{gCode=code;}                                                                      \
CountTriggerRun(&warningDlyClearTrigger##ID,rst);
#define WARNINGDLY_CLEAR_ALIAS(cmd,ID,threshold,code) WARNINGDLY_CLEAR_ALIGN(cmd,ID,threshold,code)
#define WARNINGDLY_CLEAR(cmd,threshold,code) WARNINGDLY_CLEAR_ALIAS(cmd,__LINE__,threshold,code)

#define TIMES_COMPARE_BUS_VOLTAGE 20000 // 20260518,新增母线电压上升过程计数器触发上限
int32_t count_bus_voltage_rise = 0; // 20260518,新增母线电压上升过程计数器
int16_t flag_bus_voltage_rise = 0; // 20260518,新增母线电压上升过程标志位
float bus_voltage_new = 0; // 20260518,新增母线电压实时值
float bus_voltage_old = 0; // 20260518,新增母线电压历史值

void AlgoErrHandler()
{
    static uint16_t count = 0;
    char rst = 0;
    float bat_cur_over;
    float bat_cur_over_dly;
    float load_pwr_over;
    float load_pwr_over_dly;
    float inv_pwr_over;
    float inv_pwr_over_dly;
    float bat_pwr_over;
    float bat_pwr_over_dly;
    float grid_volt_over;
    float grid_volt_lower;
    float grid_freq_over;
    float grid_freq_lower;
    _geLevel = geInvPwrLevel;

    if(_gAlgoInfo->sensor.bus_vol <= ALGOERR_THRESHOLD_BUS_VOL_LOW + 3) // 20260518，新增母线电压上升过程判断，在上升过程中不报母线电压低故障
    {
        bus_voltage_new = _gAlgoInfo->sensor.bus_vol;
        count_bus_voltage_rise++;
        if(count_bus_voltage_rise >= TIMES_COMPARE_BUS_VOLTAGE)
        {
            count_bus_voltage_rise = 0;
            if(bus_voltage_new >= bus_voltage_old + 3)
            {
                flag_bus_voltage_rise = 1;
            }
            else
            {
                flag_bus_voltage_rise = 0;
            }
            bus_voltage_old = bus_voltage_new;
        }
    }
    else
    {
        flag_bus_voltage_rise = 0;
    }


    switch(_geLevel)
    {
        //todo:后续作为可设置阈值判断变量，初次上电默认：欠频：45,过频：55;欠压：170，过压：265
//        grid_volt_over = gSysSetInfo.grid_voltage_upper_limit;
//        grid_volt_lower = gSysSetInfo.grid_voltage_lower_limit;
//        grid_freq_over = gSysSetInfo.grid_frequency_upper_limit;
//        grid_freq_lower = gSysSetInfo.grid_frequency_lower_limit;
        case EN_INV_PWR_LEVEL1:
        {
            bat_cur_over = ALGOERR_THRESHOLD_BAT_CUR_OVER_L1;
            bat_cur_over_dly = ALGOERRDLY_THRESHOLD_BAT_CUR_OVER_L1;
            load_pwr_over = ALGOERR_THRESHOLD_LOAD_PWR_OVER_L1;
            load_pwr_over_dly = ALGOERR_THRESHOLD_LOAD_PWR_DLY_OVER_L1;
            inv_pwr_over = ALGOERR_THRESHOLD_INV_PWR_OVER_L1;
            inv_pwr_over_dly = ALGOERR_THRESHOLD_INV_PWR_DLY_OVER_L1;
            bat_pwr_over = ALGOERR_THRESHOLD_BAT_PWR_OVER_L1;
            bat_pwr_over_dly = ALGOERR_THRESHOLD_BAT_PWR_DLY_OVER_L1;
            break;
        }
        case EN_INV_PWR_LEVEL2:
        {
            bat_cur_over = ALGOERR_THRESHOLD_BAT_CUR_OVER_L2;
            bat_cur_over_dly = ALGOERRDLY_THRESHOLD_BAT_CUR_OVER_L2;
            load_pwr_over = ALGOERR_THRESHOLD_LOAD_PWR_OVER_L2;
            load_pwr_over_dly = ALGOERR_THRESHOLD_LOAD_PWR_DLY_OVER_L2;
            inv_pwr_over = ALGOERR_THRESHOLD_INV_PWR_OVER_L2;
            inv_pwr_over_dly = ALGOERR_THRESHOLD_INV_PWR_DLY_OVER_L2;
            bat_pwr_over = ALGOERR_THRESHOLD_BAT_PWR_OVER_L2;
            bat_pwr_over_dly = ALGOERR_THRESHOLD_BAT_PWR_DLY_OVER_L2;
            break;
        }
        case EN_INV_PWR_LEVEL3:
        {
            bat_cur_over = ALGOERR_THRESHOLD_BAT_CUR_OVER_L3;
            bat_cur_over_dly = ALGOERRDLY_THRESHOLD_BAT_CUR_OVER_L3;
            load_pwr_over = ALGOERR_THRESHOLD_LOAD_PWR_OVER_L3;
            load_pwr_over_dly = ALGOERR_THRESHOLD_LOAD_PWR_DLY_OVER_L3;
            inv_pwr_over = ALGOERR_THRESHOLD_INV_PWR_OVER_L3;
            inv_pwr_over_dly = ALGOERR_THRESHOLD_INV_PWR_DLY_OVER_L3;
            bat_pwr_over = ALGOERR_THRESHOLD_BAT_PWR_OVER_L3;
            bat_pwr_over_dly = ALGOERR_THRESHOLD_BAT_PWR_DLY_OVER_L3;
            break;
        }
        default:
        {
            gAlgoErr |= 1;
            _SetErrCode(EN_ALGO_ERR_INV_OUTPUT_ERR);
            break;
        }
    }

    //ERR_CHECK({ rst = gTestErr == 1; }, EN_ALGO_ERR_BUS_VOL_LOW);
    //20260518,新增待机不报母线电压低故障,新增母线电压上升过程中不报母线电压低故障
    ERR_CHECK({ rst = (_gAlgoInfo->llc_start_ok == 1) && (_gAlgoInfo->sensor.bus_vol < ALGOERR_THRESHOLD_BUS_VOL_LOW) && (flag_bus_voltage_rise != 1) && (gSysSetInfo.remote_standby_cmd != 1) && (gArcFaultStandbyFlag != 1) && (gSyncHostStandby != EN_INV_STANDBY);
        gAlgoErrData = _gAlgoInfo->sensor.bus_vol; }, EN_ALGO_ERR_BUS_VOL_LOW);
    ERR_CHECK({ rst = (_gAlgoInfo->sensor.bus_vol > ALGOERR_THRESHOLD_BUS_VOL_OVER); }, EN_ALGO_ERR_BUS_VOL_OVER);
    ERR_CHECK(
        {
            rst =
            (fabsf(_gAlgoInfo->sensor.inv_vol) > ALGOERR_THRESHOLD_INV_VOL_OVER) &&
            _gAlgoInfo->inv_state != EN_INV_STATE_DEF_CONNECTED &&
            !(_gAlgoInfo->inv_state == EN_INV_STATE_DEF_DISCONNECTING && gUnConnectRlyCount < 200);
        }, EN_ALGO_ERR_INV_VOL_OVER);
    //ERR_CHECK({ rst = ((_gAlgoInfo->inv_state == EN_INV_STATE_DEF_CLOSE) && _gAlgoInfo->sensor.inv_vol > 10); }, EN_ALGO_ERR_INV_OUTPUT_ERR);
    ERR_CHECK({ rst = (fabsf(_gAlgoInfo->sensor.inv_cur) > ALGOERR_THRESHOLD_INV_CUR_OVER); }, EN_ALGO_ERR_INV_CUR_OVER);
    //ERR_CHECK({ rst = TestErr == 1; }, EN_ALGO_ERR_INV_VOL_DC_OFFSET_OVER);

    //20260518,新增待机不报电池电压低故障,新增母线电压上升过程中不报电池电压低故障
    ERR_CHECK({ rst = (_gAlgoInfo->llc_enable == 1) && (_gAlgoInfo->sensor.bat_vol < ALGOERR_THRESHOLD_BAT_VOL_LOW) && (flag_bus_voltage_rise != 1) && (gSysSetInfo.remote_standby_cmd != 1) && (gArcFaultStandbyFlag != 1) && (gSyncHostStandby != EN_INV_STANDBY); }, EN_ALGO_ERR_BAT_VOL_LOW);
//    ERR_CHECK({ rst = (_gAlgoInfo->sensor.bat_vol > ALGOERR_THRESHOLD_BAT_VOL_OVER); }, EN_ALGO_ERR_BAT_VOL_OVER);
    ERR_CHECK({ rst = ((_gAlgoInfo->inv_state == EN_INV_STATE_DEF_OPENING) && (_gAlgoInfo->relay.rly_load == 0) && (_gAlgoInfo->load_vol_rms > 10)); }, EN_ALGO_ERR_INV_OUTPUT_ERR);
    ERR_CHECK({ rst = gIsInvCurLimitOverCount; }, EN_HARDWARE_CMPSS_OVER_CUR);
    ERR_CHECK({ rst = Read_cmpss_status() && (_gAlgoInfo->inv_state == EN_INV_STATE_DEF_CONNECTED || _gAlgoInfo->inv_state == EN_INV_STATE_DEF_DISCONNECTING); }, EN_HARDWARE_CMPSS_OVER_CUR);
    ERR_CHECK({ rst = (_gAlgoInfo->sys_enable == 1) && (_gAlgoInfo->bat_cur_ave > bat_cur_over); }, EN_ALGO_ERR_BAT_CUR_DISCHARGE_OVER);
    ERR_CHECK({ rst = (_gAlgoInfo->sys_enable == 1) && (_gAlgoInfo->bat_cur_ave < bat_cur_over * -1); }, EN_ALGO_ERR_BAT_CUR_CHARGE_OVER);
    ERR_CHECK(
        {
            rst =
            (fabsf(_gAlgoInfo->grid_pwr) > ALGOERR_THRESHOLD_GRID_PWR_OVER) ||
            (fabsf(_gAlgoInfo->inv_pwr) > inv_pwr_over) ||
            (_gAlgoInfo->load_pwr > load_pwr_over) ||
            (fabsf(_gAlgoInfo->bat_pwr) > bat_pwr_over) ||
            (_gAlgoInfo->pv1_pwr + _gAlgoInfo->pv2_pwr > ALGOERR_THRESHOLD_PV_PWR_OVER);
        }, EN_ALGO_ERR_OVERLOAD_2);
     ERR_CHECK({ rst = gstFaultInfo.worning_code.b.leak_curr_high; }, EN_ALGO_ERR_LEAK_CUR);

    //ERRDLY_CHECK({ rst = (_gAlgoInfo->sys_enable == 1) && (_gAlgoInfo->sensor.bus_vol < 10); }, 15000, EN_ALGO_ERR_DCDC_START_ERR);
// WARNINGDLY_CHECK(
//     { 
//         rst = (gAlgoWarningDef.err.pv_loss_control == 0);
//     }, 10 , EN_ALGO_WARNING_PV_LOSS_CONTROL);

    if(++count >= SCALER)
    {
        count = 0;
    }
    switch(count)
    {
        case 0:
        {
            ERRDLY_CHECK(
                { rst =
                    (fabsf(_gAlgoInfo->sensor.inv_vol) > ALGOERRDLY_THRESHOLD_INV_VOL_OVER) &&
                    _gAlgoInfo->inv_state != EN_INV_STATE_DEF_CONNECTED &&
                    !(_gAlgoInfo->inv_state == EN_INV_STATE_DEF_DISCONNECTING && gUnConnectRlyCount < 200);
                }, 20, EN_ALGO_ERR_INV_VOL_OVER);
            ERRDLY_CHECK({ rst = (fabsf(_gAlgoInfo->sensor.inv_cur) > ALGOERRDLY_THRESHOLD_INV_CUR_OVER); }, 20, EN_ALGO_ERR_INV_CUR_OVER);
            ERRDLY_CHECK({ rst = (_gAlgoInfo->sys_enable == 1) && (_gAlgoInfo->bat_cur_ave > bat_pwr_over_dly); }, 20000, EN_ALGO_ERR_BAT_CUR_DISCHARGE_OVER);
            ERRDLY_CHECK({ rst = (_gAlgoInfo->sys_enable == 1) && (_gAlgoInfo->bat_cur_ave < bat_pwr_over_dly * -1); }, 20000, EN_ALGO_ERR_BAT_CUR_CHARGE_OVER);
            ERRDLY_CHECK({ rst = (_gAlgoInfo->sys_enable == 1) && (fabsf(_gAlgoInfo->bat_cur_ave) < 1) && (_gAlgoInfo->bus_vol_dt < -2.0) && (_gAlgoInfo->bus_vol_dt > -4.0) && (gAlgoWarningDef.err.bus_vol_fastover == 0); }, 6000, EN_ALGO_ERR_BAT_VOL_DOWN);
            ERRDLY_CHECK({ rst = (gstSensorData.temp[4] > 900) || (gstSensorData.temp[5] > 900); }, 20000, EN_ALGO_ERR_INV_RADIATOR_OVERHEAT);
            ERRDLY_CHECK(
                {
                    rst =
                    (fabsf(_gAlgoInfo->grid_pwr) > ALGOERR_THRESHOLD_GRID_PWR_DLY_OVER) ||
                    (fabsf(_gAlgoInfo->inv_pwr) > inv_pwr_over_dly) ||
                    (_gAlgoInfo->load_pwr > load_pwr_over_dly) ||
                    (fabsf(_gAlgoInfo->bat_pwr) > bat_pwr_over_dly) ||
                    (_gAlgoInfo->pv1_pwr + _gAlgoInfo->pv2_pwr > ALGOERR_THRESHOLD_PV_PWR_DLY_OVER);
                }, 1200000, EN_ALGO_ERR_OVERLOAD_1); //60s触发
            ERRDLY_CHECK({ rst = (_gAlgoInfo->inv_start_ok == 1 && _gAlgoInfo->inv_vol_rms < ALGOERR_THRESHOLD_INV_VOL_LOW && _gAlgoInfo->inv_state != EN_INV_STATE_DEF_CONNECTED); }, 20000, EN_ALGO_ERR_INV_VOL_LOW);
            ERRDLY_CHECK({ rst = (_gAlgoInfo->inv_start_ok == 1 && _gAlgoInfo->inv_vol_rms < ALGOERR_THRESHOLD_INV_VOL_LOW); }, 40000, EN_ALGO_ERR_INV_VOL_LOW);
            ERRDLY_CHECK({ rst = (_gAlgoInfo->sensor.bat_vol > ALGOERR_THRESHOLD_BAT_VOL_OVER); }, 4000, EN_ALGO_ERR_BAT_VOL_OVER);    //逆变器电池侧过压故障200mS触发
            break;
        }
        case 1:
        {
            WARNING_CHECK({
               rst = (_gAlgoInfo->sensor.bat_vol > 59) && (gAlgoWarningDef.err.pv_loss_control == 0) && ((_gAlgoInfo->pv1_cur_ave > 2) || (_gAlgoInfo->pv2_cur_ave > 2)) && ((gstPv1Data.duty_eff < 0.15) && (gstPv2Data.duty_eff < 0.15));
                }, EN_ALGO_WARNING_PV_LOSS_CONTROL); //电池电压大于59V关PV,报警告 //20260511,新增PV失控必须满足平均电流大于2A //20260511,新增PV失控必须满足占空比小于15%
            WARNING_CHECK({ rst = (_gAlgoInfo->grid_vol_rms < ALGOWARNING_THRESHOLD_GRID_VOL_LOW_OFF); }, EN_ALGO_WARNING_GRID_VOL_LOW);
            WARNING_CHECK({ rst = (fabsf(_gAlgoInfo->sensor.grid_vol) > ALGOWARNING_THRESHOLD_GRID_VOL_OVER_OFF * 1.414); }, EN_ALGO_WARNING_GRID_VOL_OVER);
            WARNING_CHECK({ rst = (_gAlgoInfo->grid_vol_rms > ALGOWARNING_THRESHOLD_GRID_VOL_OVER_STOP); }, EN_ALGO_WARNING_GRID_STOP);
            WARNING_CHECK({ rst = (_gAlgoInfo->grid_vol_rms < ALGOWARNING_THRESHOLD_GRID_VOL_LOW_STOP); }, EN_ALGO_WARNING_GRID_STOP);
            WARNING_CHECK({ rst = (_gAlgoInfo->pv1_vol_ave <= ALGOWARNING_THRESHOLD_PV_VOL_LOW); }, EN_ALGO_WARNING_PV1_VOL_LOW);
            WARNING_CHECK({ rst = (_gAlgoInfo->pv2_vol_ave <= ALGOWARNING_THRESHOLD_PV_VOL_LOW); }, EN_ALGO_WARNING_PV2_VOL_LOW);
            WARNING_CHECK({ rst = (_gAlgoInfo->pv1_vol_ave >= ALGOWARNING_THRESHOLD_PV_VOL_OVER); }, EN_ALGO_WARNING_PV1_VOL_OVER);
            WARNING_CHECK({ rst = (_gAlgoInfo->pv2_vol_ave >= ALGOWARNING_THRESHOLD_PV_VOL_OVER); }, EN_ALGO_WARNING_PV2_VOL_OVER);

            break;
        }
        case 2:
        {
            WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->grid_vol_rms_filter >= ALGOWARNING_THRESHOLD_GRID_VOL_LOW_RECOVER); }, EN_ALGO_WARNING_GRID_VOL_LOW);
            WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->grid_vol_rms_filter <= ALGOWARNING_THRESHOLD_GRID_VOL_OVER_RECOVER); }, EN_ALGO_WARNING_GRID_VOL_OVER);
            WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->grid_vol_rms >= ALGOWARNING_THRESHOLD_GRID_VOL_LOW_RECOVER && _gAlgoInfo->grid_vol_rms_filter <= ALGOWARNING_THRESHOLD_GRID_VOL_OVER_RECOVER); }, EN_ALGO_WARNING_GRID_STOP);
            WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->pv1_vol_ave > (ALGOWARNING_THRESHOLD_PV_VOL_LOW + 10)); }, EN_ALGO_WARNING_PV1_VOL_LOW); //滞环恢复
            WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->pv2_vol_ave > (ALGOWARNING_THRESHOLD_PV_VOL_LOW + 10)); }, EN_ALGO_WARNING_PV2_VOL_LOW); //滞环恢复
            WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->pv1_vol_ave < (ALGOWARNING_THRESHOLD_PV_VOL_OVER - 10)); }, EN_ALGO_WARNING_PV1_VOL_OVER); //滞环恢复
            WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->pv2_vol_ave < (ALGOWARNING_THRESHOLD_PV_VOL_OVER - 10)); }, EN_ALGO_WARNING_PV2_VOL_OVER); //滞环恢复
            if(GetGridToGenFinish() == 1)//已切换至发电机模式
            {
                WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->grid_freq > 41); }, EN_ALGO_WARNING_GRID_FREQ_LOW);
                WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->grid_freq < 59); }, EN_ALGO_WARNING_GRID_FREQ_OVER);
            }
            else
            {
                WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->grid_freq > ALGOWARNING_THRESHOLD_GRID_FREQ_LOW_RECOVER); }, EN_ALGO_WARNING_GRID_FREQ_LOW);
                WARNING_CHECK_CLEAR({ rst = (_gAlgoInfo->grid_freq < ALGOWARNING_THRESHOLD_GRID_FREQ_OVER_RECOVER); }, EN_ALGO_WARNING_GRID_FREQ_OVER);
            }

            break;
        }
        case 3:
        {
//            WARNINGDLY_CHECK(
//                {
//                    rst =
//                    (gAlgoWarningDef.err.pv_loss_control == 0) &&
//                    (
//                    (((_gAlgoInfo->pv1_pwr + _gAlgoInfo->pv2_pwr) > (_gAlgoInfo->pv_set_pwr + 200))
//                           && (gAlgoWarningDef.err.pv_recover_permit == 0)) || //PV告警恢复许可（PV的刚接入时失控功率也无法消耗）
//                     (_gAlgoInfo->pv1_cur_ave > ALGOERR_THRESHOLD_PV_CUR_OVER) ||
//                     (_gAlgoInfo->pv2_cur_ave > ALGOERR_THRESHOLD_PV_CUR_OVER) ||
//                     (_gAlgoInfo->sensor.bus_vol > 525)  //bus电压大于525持续300ms关PV，报警告
//                    );
//                    // ((_gAlgoInfo->pv1_cur_ave > 1) && (_gAlgoInfo->pv1_vol_ave < 10)) || //PV反接
//                    // ((_gAlgoInfo->pv2_cur_ave > 1) && (_gAlgoInfo->pv2_vol_ave < 10));
//                }, 6000, EN_ALGO_WARNING_PV_LOSS_CONTROL); //对应三个PV控制周期


            WARNINGDLY_CHECK(
                {
                    rst =
                    (gAlgoWarningDef.err.pv_loss_control == 0) &&
                    (
                    (((_gAlgoInfo->pv1_pwr + _gAlgoInfo->pv2_pwr) > (_gAlgoInfo->pv_set_pwr + 400))
                           && (gAlgoWarningDef.err.pv_recover_permit == 0)) || //PV告警恢复许可（PV的刚接入时失控功率也无法消耗）
                     (_gAlgoInfo->pv1_cur_ave > ALGOERR_THRESHOLD_PV_CUR_OVER + 10) ||
                     (_gAlgoInfo->pv2_cur_ave > ALGOERR_THRESHOLD_PV_CUR_OVER + 10) ||
                     (_gAlgoInfo->sensor.bus_vol > 550) &&  //bus电压大于525持续300ms关PV，报警告
                     ((_gAlgoInfo->pv1_cur_ave > 2) || (_gAlgoInfo->pv2_cur_ave > 2)) && //20260511,新增PV失控必须满足平均电流大于2A
                     ((gstPv1Data.duty_eff < 0.15) && (gstPv2Data.duty_eff < 0.15))  //20260511,新增PV失控必须满足占空比小于15%
                    );
                    // ((_gAlgoInfo->pv1_cur_ave > 1) && (_gAlgoInfo->pv1_vol_ave < 10)) || //PV反接
                    // ((_gAlgoInfo->pv2_cur_ave > 1) && (_gAlgoInfo->pv2_vol_ave < 10));
                }, 60000, EN_ALGO_WARNING_PV_LOSS_CONTROL); //对应三个PV控制周期


            WARNINGDLY_CHECK({ rst = ((_gAlgoInfo->sensor.bus_vol > 520) && (_gAlgoInfo->bus_vol_dt > 3)); }, 5, EN_ALGO_WARNING_BUS_VOL_FASTOVER);//关闭PV在电池电压大于58V时，取一定裕度为57.8V，对应母线电压为57.8*9=520.2
            WARNINGDLY_CLEAR(
                { rst = (gAlgoWarningDef.err.bus_vol_fastover == 1);
                }, 10000, EN_ALGO_WARNING_BUS_VOL_FASTOVER); //延迟半秒清除bus快速过压警告
            WARNINGDLY_CHECK(
                { rst = (_gAlgoInfo->pv1_vol_ave <= 20) && (_gAlgoInfo->pv1_cur_ave < ALGOWARNING_THRESHOLD_PV_CUR_INVERSED);
                }, 6000, EN_ALGO_WARNING_PV1_INVERSED); //对应三个PV控制周期
            WARNINGDLY_CLEAR(
                { rst = (_gAlgoInfo->pv1_vol_ave >= 30);
                }, 6000, EN_ALGO_WARNING_PV1_INVERSED); //对应三个PV控制周期
            WARNINGDLY_CHECK(
                { rst = (_gAlgoInfo->pv2_vol_ave <= 20) && (_gAlgoInfo->pv2_cur_ave < ALGOWARNING_THRESHOLD_PV_CUR_INVERSED);
                }, 6000, EN_ALGO_WARNING_PV2_INVERSED); //对应三个PV控制周期
            WARNINGDLY_CLEAR(
                { rst = (_gAlgoInfo->pv2_vol_ave >= 30);
                }, 6000, EN_ALGO_WARNING_PV2_INVERSED); //对应三个PV控制周期
            WARNINGDLY_CLEAR(
                {
                    rst = (gAlgoWarningDef.err.pv_loss_control == 1);
                }, 600000, EN_ALGO_WARNING_PV_LOSS_CONTROL);

            break;
        }
        case 4:
        {
            WARNINGDLY_CHECK({ rst = (_gAlgoInfo->grid_vol_rms < ALGOWARNING_THRESHOLD_GRID_VOL_LOW_1); }, 200000, EN_ALGO_WARNING_GRID_VOL_LOW);
            WARNINGDLY_CHECK({ rst = (_gAlgoInfo->grid_vol_rms < ALGOWARNING_THRESHOLD_GRID_VOL_LOW_2); }, 25000, EN_ALGO_WARNING_GRID_VOL_LOW);
            WARNINGDLY_CHECK({ rst = (_gAlgoInfo->grid_vol_rms > ALGOWARNING_THRESHOLD_GRID_VOL_OVER_1); }, 25000, EN_ALGO_WARNING_GRID_VOL_OVER);
            WARNINGDLY_CHECK({ rst = (_gAlgoInfo->grid_vol_rms > ALGOWARNING_THRESHOLD_GRID_VOL_OVER_2); }, 100, EN_ALGO_WARNING_GRID_VOL_OVER);
            if(GetGridToGenFinish() == 1)//已切换至发电机模式
            {
                WARNING_CHECK({ rst = (_gAlgoInfo->grid_freq < 40); }, EN_ALGO_WARNING_GRID_FREQ_LOW);
                WARNING_CHECK({ rst = (_gAlgoInfo->grid_freq > 60); }, EN_ALGO_WARNING_GRID_FREQ_OVER);
            }
            else
            {
                WARNINGDLY_CHECK({ rst = (_gAlgoInfo->grid_freq < ALGOWARNING_THRESHOLD_GRID_FREQ_LOW); }, 20000, EN_ALGO_WARNING_GRID_FREQ_LOW);
                WARNINGDLY_CHECK({ rst = (_gAlgoInfo->grid_freq > ALGOWARNING_THRESHOLD_GRID_FREQ_OVER); }, 200, EN_ALGO_WARNING_GRID_FREQ_OVER);
            }
            break;
        }
        default:
        {
            count = 0;
            break;
        }
    }
    gAlgoWarning = (gAlgoWarningDef.byte[0] == 0) && (gAlgoWarningDef.byte[1] == 0);
        //gAlgoErr = 0; //调试用屏蔽故障
}

void AlgoWarningSetPvRecoverPermit(Uint16 Vaule)
{
    gAlgoWarningDef.err.pv_recover_permit = Vaule;
}
void gAlgoErrDefSetInvVoltOver(Uint16 Vaule)
{
    gAlgoErrDef.err.inv_vol_over = Vaule;
}
uint16_t gAlgoErrDefGetInvVoltOver(void)
{
    return ((uint16_t)gAlgoErrDef.err.inv_vol_over);
}
void gAlgoErrDefSetCmpssCurOver(Uint16 Vaule)
{
    gAlgoErrDef.err.cmpss_cur_over = Vaule;
}
uint16_t gAlgoErrDefGetCmpssCurOver(void)
{
    return ((uint16_t)gAlgoErrDef.err.cmpss_cur_over);
}
void gAlgoClearErr(void)
{
    gAlgoErr = 0;
}
void AlgoErrCheckInit()
{
    _gAlgoInfo = AlgoStateGet();
    gAlgoErrDef.byte[0] = 0;
    gAlgoErrDef.byte[1] = 0;
    gAlgoWarningDef.byte[0] = 0;
    gAlgoWarningDef.byte[1] = 0;
}


//ERR_EXPORT_CHECK_ALIGN(begin);
//
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 1; });
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 2; });
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 3; });
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 4; });
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 5; });
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 1; });
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 2; });
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 3; });
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 4; });
//ERR_EXPORT_CHECK_AT_LINE({ rst = TestErr == 5; });
//
//ERR_EXPORT_CHECK_ALIGN(end);
//
//
//
//WARNING_EXPORT_CHECK_ALIGN(begin, 1);
//
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 6; }, 20000);
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 7; }, 20000);
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 8; }, 20000);
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 9; }, 20000);
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 10; }, 20000);
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 6; }, 20000);
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 7; }, 20000);
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 8; }, 20000);
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 9; }, 20000);
//WARNING_EXPORT_CHECK_AT_LINE({ rst = TestErr == 10; }, 20000);
//
//WARNING_EXPORT_CHECK_ALIGN(end, 1);
