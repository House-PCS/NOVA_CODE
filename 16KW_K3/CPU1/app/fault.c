/*
 * fault.c
 *
 *  Created on: 2025-3-22
 *      Author: Nova007456
 */

#include "fault.h"
#include "fan.h"
#include "gpio.h"
#include <sample_update.h>
#include "ntc.h"
#include "rcmu.h"
#include "post.h"
#include "relay_ctrl.h"
#include "ct.h"
#include "cla_cpu_share.h"
#include "ph3_calc_ifc_update.h"
#include <algo_interface.h>
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_interface.h"
#include "control_state.h"
//ModuleFaultAndDataInfo_t _gModuleFaultData;

FaultInfo_t gstFaultInfo;
uint16_t gCode = 0;
#define SCALER 3

static uint16_t _gLoadRelayOpenEn = 1;

static void _SetErrCode(uint16_t code)
{
    gstFaultInfo.inside_err.byte[(code & 0xFFF0) >> 4] |= (1 << (code & 0x000F));
}
static void _SetErrCode_Inside(uint16_t code)
{
    gstFaultInfo.inside_add_fault.byte[(code & 0xFFF0) >> 4] |= (1 << (code & 0x000F));
}

static void _SetWarningCode(uint16_t code)
{
    gstFaultInfo.inside_warn.byte[(code & 0xFFF0) >> 4] |= (1 << (code & 0x000F));
}

static void _ResetWarningCode(uint16_t code)
{
    gstFaultInfo.inside_warn.byte[(code & 0xFFF0) >> 4] &= ~(1 << (code & 0x000F));
}

static void _ErroTrig(void* param)
{
    uint16_t* p_code = param;
    gstFaultInfo.err_flag |= 1;
    if(p_code != 0)
        _SetErrCode(*p_code);
}

void _WarningTrig(void* param)
{
    uint16_t* p_code = param;
    gstFaultInfo.warning_flag |= 1;

    if(p_code != 0)
        _SetWarningCode(*p_code);
}

void _ClearwarningTrig(void* param)
{
    uint16_t* p_code = param;
    if(p_code != 0)
        _ResetWarningCode(*p_code);
}

#define ERR_CHECK(cmd,code)            \
{cmd}                                  \
if(rst==1){gstFaultInfo.err_flag |=1;_SetErrCode(code);}

#define ERR_CHECK_INSIDE(cmd,code)            \
{cmd}                                  \
if(rst==1){gstFaultInfo.err_flag |=1;_SetErrCode_Inside(code);}

#define WARNING_CHECK(cmd,code)        \
{cmd}                                  \
if(rst==1){gstFaultInfo.warning_flag |=1;_SetWarningCode(code);}

#define WARNING_CHECK_CLEAR(cmd,code)  \
{cmd}                                  \
if(rst==1){_ResetWarningCode(code);}

/***********/
/*Fault counter macro*/
#define ERRDLY_CHECK_ALIGN(cmd,ID,threshold,code)                                         \
static CountTrigger_t errDlyTrigger_##ID=                                                 \
{                                                                                         \
    .trigged = 0,                                                                         \
    .count = 0,                                                                           \
    .count_interval = 1,                                                                  \
    .hook_param = &gCode,                                                                 \
    .trig_hook = _ErroTrig,                                                               \
    .trig_threshold = threshold / SCALER,                                                 \
};                                                                                        \
{cmd}{gCode=code;}                                                                        \
CondCum_CountTrigger_Run(&errDlyTrigger_##ID, rst);
#define ERRDLY_CHECK_ALIAS(cmd,ID,threshold,code) ERRDLY_CHECK_ALIGN(cmd,ID,threshold,code)
#define ERRDLY_CHECK(cmd,threshold,code) ERRDLY_CHECK_ALIAS(cmd,__LINE__,threshold,code)

/***********/
/*Warning counter macro*/
#define WARNINGDLY_CHECK_ALIGN(cmd,ID,threshold,code)                                      \
static CountTrigger_t warningDlyTrigger##ID=                                               \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &gCode,                                                                  \
    .trig_hook = _WarningTrig,                                                             \
    .trig_threshold = threshold / SCALER,                                                  \
};                                                                                         \
{cmd}{gCode=code;}                                                                         \
CondCum_CountTrigger_Run(&warningDlyTrigger##ID, rst);
#define WARNINGDLY_CHECK_ALIAS(cmd,ID,threshold,code) WARNINGDLY_CHECK_ALIGN(cmd,ID,threshold,code)
#define WARNINGDLY_CHECK(cmd,threshold,code) WARNINGDLY_CHECK_ALIAS(cmd,__LINE__,threshold,code)

/**************/
/*Warning clear counter macro*/
#define WARNINGDLY_CLEAR_ALIGN(cmd,ID,threshold,code)                                      \
static CountTrigger_t warningDlyClearTrigger##ID=                                          \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &gCode,                                                                  \
    .trig_hook = _ClearwarningTrig,                                                        \
    .trig_threshold = threshold / SCALER,                                                  \
};                                                                                         \
{cmd}{gCode=code;}                                                                         \
CondCum_CountTrigger_Run(&warningDlyClearTrigger##ID, rst);
#define WARNINGDLY_CLEAR_ALIAS(cmd,ID,threshold,code) WARNINGDLY_CLEAR_ALIGN(cmd,ID,threshold,code)
#define WARNINGDLY_CLEAR(cmd,threshold,code) WARNINGDLY_CLEAR_ALIAS(cmd,__LINE__,threshold,code)

/**
* @brief: Diagnosis_Error_Adc_Calibration_Fault
* @param: void
* @note: ADC sampling calibration error
*/
void Diagnosis_Error_Adc_Calibration_Fault(void)
{
    static float32 adc_calibration_finish;

    if((adc_calibration_finish == 0) && (Get_Adc_Time0_Calibration_State() == 1) && \
       (Get_Adc_Time1_Calibration_State() == 1) && (Get_Adc_Time2_Calibration_State() == 1) && \
       (Get_Adc_Time3_Calibration_State() == 1) && (Get_Adc_Time4_Calibration_State() == 1) )
    {
        if((gstAdcZero.common_calibration_zero >= ADC_ZERO_ERR_MAX)         || (gstAdcZero.common_calibration_zero <= ADC_ZERO_ERR_MIN)         ||
           (gstAdcZero.inv_volt_phase_a_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.inv_volt_phase_a_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.inv_volt_phase_b_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.inv_volt_phase_b_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.inv_volt_phase_c_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.inv_volt_phase_c_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.inv_curr_phase_a_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.inv_curr_phase_a_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.inv_curr_phase_b_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.inv_curr_phase_b_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.inv_curr_phase_c_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.inv_curr_phase_c_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.leak_curr_zero >= ADC_ZERO_ERR_MAX)                  || (gstAdcZero.leak_curr_zero <= ADC_ZERO_ERR_MIN)                  ||
           (gstAdcZero.relay_middle_volt_phase_a_zero >= ADC_ZERO_ERR_MAX)  || (gstAdcZero.relay_middle_volt_phase_a_zero <= ADC_ZERO_ERR_MIN)  ||
           (gstAdcZero.relay_middle_volt_phase_b_zero >= ADC_ZERO_ERR_MAX)  || (gstAdcZero.relay_middle_volt_phase_b_zero <= ADC_ZERO_ERR_MIN)  ||
           (gstAdcZero.relay_middle_volt_phase_c_zero >= ADC_ZERO_ERR_MAX)  || (gstAdcZero.relay_middle_volt_phase_c_zero <= ADC_ZERO_ERR_MIN)  ||
           (gstAdcZero.grid_volt_phase_a_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.grid_volt_phase_a_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.grid_volt_phase_b_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.grid_volt_phase_b_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.grid_volt_phase_c_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.grid_volt_phase_c_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.grid_curr_phase_a_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.grid_curr_phase_a_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.grid_curr_phase_b_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.grid_curr_phase_b_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.grid_curr_phase_c_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.grid_curr_phase_c_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.ct_a_curr_zero >= ADC_ZERO_ERR_MAX)                  || (gstAdcZero.ct_a_curr_zero <= ADC_ZERO_ERR_MIN)                  ||
           (gstAdcZero.ct_b_curr_zero >= ADC_ZERO_ERR_MAX)                  || (gstAdcZero.ct_b_curr_zero <= ADC_ZERO_ERR_MIN)                  ||
           (gstAdcZero.ct_c_curr_zero >= ADC_ZERO_ERR_MAX)                  || (gstAdcZero.ct_c_curr_zero <= ADC_ZERO_ERR_MIN)                  ||
           (gstAdcZero.load_volt_phase_a_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.load_volt_phase_a_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.load_volt_phase_b_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.load_volt_phase_b_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.load_volt_phase_c_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.load_volt_phase_c_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.load_curr_phase_a_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.load_curr_phase_a_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.load_curr_phase_b_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.load_curr_phase_b_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.load_curr_phase_c_zero >= ADC_ZERO_ERR_MAX)          || (gstAdcZero.load_curr_phase_c_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.pv1_volt_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.pv1_volt_zero <= ADC_ZERO_ERR_MIN)                   ||
           (gstAdcZero.pv1_curr_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.pv1_curr_zero <= ADC_ZERO_ERR_MIN)                   ||
           (gstAdcZero.pv2_volt_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.pv2_volt_zero <= ADC_ZERO_ERR_MIN)                   ||
           (gstAdcZero.pv2_curr_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.pv2_curr_zero <= ADC_ZERO_ERR_MIN)                   ||
           (gstAdcZero.pv3_volt_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.pv3_volt_zero <= ADC_ZERO_ERR_MIN)                   ||
           (gstAdcZero.pv3_curr_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.pv3_curr_zero <= ADC_ZERO_ERR_MIN)                   ||
           (gstAdcZero.pv4_volt_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.pv4_volt_zero <= ADC_ZERO_ERR_MIN)                   ||
           (gstAdcZero.pv4_curr_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.pv4_curr_zero <= ADC_ZERO_ERR_MIN)                   ||
           (gstAdcZero.bus_volt_pos_zero >= ADC_ZERO_ERR_MAX)               || (gstAdcZero.bus_volt_pos_zero <= ADC_ZERO_ERR_MIN)               ||
           (gstAdcZero.bus_volt_neg_zero >= ADC_ZERO_ERR_MAX)               || (gstAdcZero.bus_volt_neg_zero <= ADC_ZERO_ERR_MIN)               ||
           (gstAdcZero.bus_curr_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.bus_curr_zero <= ADC_ZERO_ERR_MIN)                   ||
           (gstAdcZero.gen_volt_phase_a_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.gen_volt_phase_a_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.gen_volt_phase_b_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.gen_volt_phase_b_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.gen_volt_phase_c_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.gen_volt_phase_c_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.gen_curr_phase_a_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.gen_curr_phase_a_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.gen_curr_phase_b_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.gen_curr_phase_b_zero <= ADC_ZERO_ERR_MIN)           ||
           (gstAdcZero.gen_curr_phase_c_zero >= ADC_ZERO_ERR_MAX)           || (gstAdcZero.gen_curr_phase_c_zero <= ADC_ZERO_ERR_MIN)          ||
           (gstAdcZero.bus_volt_pos_zero >= ADC_ZERO_ERR_MAX)               || (gstAdcZero.bus_volt_pos_zero <= ADC_ZERO_ERR_MIN)               ||
           (gstAdcZero.bus_volt_neg_zero >= ADC_ZERO_ERR_MAX)               || (gstAdcZero.bus_volt_neg_zero <= ADC_ZERO_ERR_MIN)               ||
           (gstAdcZero.bus_curr_zero >= ADC_ZERO_ERR_MAX)                   || (gstAdcZero.bus_curr_zero <= ADC_ZERO_ERR_MIN))
        {
            gstFaultInfo.inside_err.bit.samping_sensor = 1;
            gstFaultInfo.err_flag = 1;
            adc_calibration_finish = 1;
        }
        else
        {
            gstFaultInfo.inside_err.bit.samping_sensor = 0;
            adc_calibration_finish = 1;
        }
    }
}

/**
* @brief: Diagnosis_Warning_Fan_Error
* @param: void
* @note: Fan fault warning
*/
uint8_t Diagnosis_Warning_Fan_Error(void)
{
    static uint8_t fault_flag = 0;
    /*Fault recovery: check if any fan has fault*/
    fault_flag = FanGetAllFaultState();
    return fault_flag;
}

/**
* @brief: ErrWarningCheckInit()
* @param: void
* @return:Nothing
* @note:Fault/warning detection initialization
*/
void ErrWarningCheckInit()
{
    gstFaultInfo.inside_warn.byte[0] = 0;
    gstFaultInfo.inside_warn.byte[1] = 0;
    gstFaultInfo.inside_err.byte[0]  = 0;
    gstFaultInfo.inside_err.byte[1]  = 0;
    gstFaultInfo.inside_add_fault.byte[0] = 0;
    gstFaultInfo.inside_add_fault.byte[1] = 0;
    gstFaultInfo.warning_flag = 0;
    gstFaultInfo.err_flag = 0;
}
/**
* @brief: FastErrWarningHandler()
* @param: void
* @return:nothing
* @note:Fast fault check (runs in ISR)
*/
void FastErrWarningHandler(void)
{
    char rst = 0;
    FanFaultProcessAll();                                  /*Fan fault check*/
    LeakCurrProcess(gstAdcResult.leak_curr,20000,50);    /*Leakage current fault detection*/
    bool *break_result = CtbreakJudge();
    /*修改条件为电网无故障状态下进行判断*/
//    ERRDLY_CHECK({ rst = (gstAlgoInterface.grid_start_ok  && break_result[0]);}, 400, EN_ERR_CT_R_DISCONNECT);
//    ERRDLY_CHECK({ rst = (gstAlgoInterface.grid_start_ok  && break_result[1]);}, 400, EN_ERR_CT_S_DISCONNECT);
//    ERRDLY_CHECK({ rst = (gstAlgoInterface.grid_start_ok  && break_result[2]);}, 400, EN_ERR_CT_T_DISCONNECT);

//    ERR_CHECK({ rst = (GetLeakCurrFaultFlag()) == 1; }, EN_ERR_LEAKCURR_HIGH);
    ERR_CHECK({ rst = ((EPwm1Regs.TZFLG.bit.OST == 1) || (EPwm2Regs.TZFLG.bit.OST == 1) || (EPwm3Regs.TZFLG.bit.OST == 1) || \
                       (EPwm4Regs.TZFLG.bit.OST == 1) || (EPwm5Regs.TZFLG.bit.OST == 1) || (EPwm6Regs.TZFLG.bit.OST == 1)); }, EN_ERR_INV_SHORT);
    ERR_CHECK({ rst = ((gstAdcResult.inv_volt_phase_a > ERROR_THRESHOLD_INV_VOLT_OVER) || \
                       (gstAdcResult.inv_volt_phase_b > ERROR_THRESHOLD_INV_VOLT_OVER) || \
                       (gstAdcResult.inv_volt_phase_c > ERROR_THRESHOLD_INV_VOLT_OVER)); }, EN_ERR_INV_VOLT_OVER);

    ERRDLY_CHECK({ rst = (gstPh3CalcInfo.bus.curr_filter > ERROR_THRESHOLD_BUS_CURR_OVER);}, 10, EN_ERR_BUS_DSG_CURR_OVER);
    ERRDLY_CHECK({ rst = (gstPh3CalcInfo.bus.curr_filter < -1 * ERROR_THRESHOLD_BUS_CURR_OVER);}, 10, EN_ERR_BUS_CHG_CURR_OVER);
    ERR_CHECK({ rst = (Get3PhIsRunFlag()) && ((gstPh3CalcInfo.bus.pos_vol_filter < BUS_VOLT_LOW_FAULT) ||
                      (gstPh3CalcInfo.bus.neg_vol_filter < BUS_VOLT_LOW_FAULT));}, EN_ERR_BUS_VOL_LOW);//Bus undervoltage fault check
    ERRDLY_CHECK({ rst = (gstPh3CalcInfo.bus.pos_vol_filter > BUS_VOLT_HIGH) ||
                         (gstPh3CalcInfo.bus.pos_vol_filter > BUS_VOLT_HIGH);}, 20, EN_ERR_BUS_VOL_OVER);//Bus overvoltage detect, 50*20=100 us
    ERR_CHECK({ rst = (gstPh3CalcInfo.bus.pos_vol_filter > BUS_PEAK_VOLT_HIGH) ||
                         (gstPh3CalcInfo.bus.pos_vol_filter > BUS_PEAK_VOLT_HIGH);}, EN_ERR_BUS_VOL_OVER);//Bus overvoltage detect, 50*20=100 us

    ERRDLY_CHECK({ rst = ((gstPh3CalcInfo.bus.vol_filter < 20) && ((fabs(gstPh3CalcInfo.bus.curr_filter) > 5)));}, 20, EN_ERR_DCDC_SHORT_ERR);

    ERR_CHECK({ rst = ((gstAdcResult.inv_curr_phase_a > ERROR_THRESHOLD_INV_CURR_OVER) || \
                       (gstAdcResult.inv_curr_phase_b > ERROR_THRESHOLD_INV_CURR_OVER) || \
                       (gstAdcResult.inv_curr_phase_c > ERROR_THRESHOLD_INV_CURR_OVER)); }, EN_ERR_INV_CUR_OVER);

    ERR_CHECK({ rst = (EPwm1Regs.TZFLG.bit.OST == 1);}, EN_ERR_HARDWARE_CMPSS_OVER_CUR);//CMPSS overcurrent
    //    ERR_CHECK({ rst = _gModuleFaultData.err_flag.inv_volt_low_flag; }, EN_ERR_INV_VOL_LOW);
//    ERR_CHECK({ rst = _gModuleFaultData.err_flag.dc_component_high_flag; }, EN_ERR_INV_VOL_DC_OFFSET_OVER);
//    ERR_CHECK({ rst = (_gModuleFaultData.err_flag.inv_out_fault_flag == 1);}, EN_ERR_INV_OUTPUT_ERR);
}

static void _PvErrWarningHandler(void)
{
    if(PvVolLowWarningCodeRead(0))
    {
        gstFaultInfo.inside_warn.bit.pv1_no_connected = 1;
    }
    else
    {
        gstFaultInfo.inside_warn.bit.pv1_no_connected = 0;
    }

    if(PvVolLowWarningCodeRead(1))
    {
        gstFaultInfo.inside_warn.bit.pv2_no_connected = 1;
    }
    else
    {
        gstFaultInfo.inside_warn.bit.pv2_no_connected = 0;
    }

    if(PvVolLowWarningCodeRead(2))
    {
        gstFaultInfo.inside_warn.bit.pv3_no_connected = 1;
    }
    else
    {
        gstFaultInfo.inside_warn.bit.pv3_no_connected = 0;
    }

    if(PvLossCtrlWarningCodeRead(0) || PvLossCtrlWarningCodeRead(1) || PvLossCtrlWarningCodeRead(2))
    {
        gstFaultInfo.inside_warn.bit.pv_loss_control = 1;
    }
    else
    {
        gstFaultInfo.inside_warn.bit.pv_loss_control = 0;
    }

    if(PvReverseErrCodeRead(0) || PvReverseErrCodeRead(1) || PvReverseErrCodeRead(2))
    {
        gstFaultInfo.inside_err.bit.pv_reverse = 1;
    }
    else
    {
        gstFaultInfo.inside_err.bit.pv_reverse = 0;
    }
    if(gstFaultInfo.inside_err.bit.bus_volt_high || gstFaultInfo.inside_err.bit.box_over_temp|| gstFaultInfo.inside_err.bit.radiator_over_temp)
    {
        PVRunSet(false);
    }
//    gstPvData[0].
}
/**
* @brief: SlowErrWarningHandler()
* @param: void
* @return:nothing
* @note:SlowErrWarningHandler (runs in main loop)
*/
void SlowErrWarningHandler(void)
{
    static EN_TIME_DIV count = EN_TIME_DIV_1;
    char rst = 0;
    _PvErrWarningHandler();
    Inv3Ph_FaultSlowCheck_Run();             //���ģ�����ٹ��ϴ���
    ERR_CHECK({rst = (gstPh3CalcInfo.ct_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L2) ||
                     (gstPh3CalcInfo.ct_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.ct_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.ct_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.gen_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L2) ||
                     (gstPh3CalcInfo.gen_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.gen_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.gen_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.inv_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L2) ||
                     (gstPh3CalcInfo.inv_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.inv_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.inv_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.grid_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L2) ||
                     (gstPh3CalcInfo.grid_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.grid_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.grid_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.load_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L2) ||
                     (gstPh3CalcInfo.load_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.load_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3)) ||
                     (gstPh3CalcInfo.load_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L2 / 3));},EN_ERR_OVERLOAD_2);//Overload level 2


    ERR_CHECK({ rst = ((gstPh3CalcInfo.inv_vol.a_phase_rms > INV_VOLT_RMS_HIGH) || \
                       (gstPh3CalcInfo.inv_vol.b_phase_rms > INV_VOLT_RMS_HIGH) || \
                       (gstPh3CalcInfo.inv_vol.c_phase_rms > INV_VOLT_RMS_HIGH)); }, EN_ERR_INV_VOLT_OVER);

    ERR_CHECK({ rst = (Get3PhIsRunFlag() && ((gstPh3CalcInfo.inv_vol.a_phase_rms < INV_VOLT_RMS_LOW) || \
                       (gstPh3CalcInfo.inv_vol.b_phase_rms < INV_VOLT_RMS_LOW) || \
                       (gstPh3CalcInfo.inv_vol.c_phase_rms < INV_VOLT_RMS_LOW))); }, EN_ERR_INV_VOL_LOW);
    ERR_CHECK({ rst = ((gstPh3CalcInfo.inv_curr.a_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS) || \
                       (gstPh3CalcInfo.inv_curr.b_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS) || \
                       (gstPh3CalcInfo.inv_curr.c_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS)); }, EN_ERR_INV_CUR_OVER);

    ERR_CHECK_INSIDE({ rst = (GetRelayState(EN_RELAT_CTRL_TYPE_GRID) && (gstPcsInvRunState == INV_STATE_GRID_CONNECTED) && ((gstPh3CalcInfo.grid_vol.a_phase_rms > GRID_VOLT_RMS_HIGH) || \
                       (gstPh3CalcInfo.grid_vol.b_phase_rms > GRID_VOLT_RMS_HIGH) || \
                       (gstPh3CalcInfo.grid_vol.c_phase_rms > GRID_VOLT_RMS_HIGH))); }, EN_ERR_GRID_VOL_HIGH_FAULT);

    ERR_CHECK_INSIDE({ rst = (GetRelayState(EN_RELAT_CTRL_TYPE_GRID) && (gstPcsInvRunState == INV_STATE_GRID_CONNECTED) && ((gstPh3CalcInfo.grid_vol.a_phase_rms < GRID_VOLT_RMS_LOW) || \
                       (gstPh3CalcInfo.grid_vol.b_phase_rms < GRID_VOLT_RMS_LOW) || \
                       (gstPh3CalcInfo.grid_vol.c_phase_rms < GRID_VOLT_RMS_LOW))); }, EN_ERR_GRID_VOL_LOW_FAULT);

    ERR_CHECK_INSIDE({ rst = (GetRelayState(EN_RELAT_CTRL_TYPE_GRID) && (gstPcsInvRunState == INV_STATE_GRID_CONNECTED) && ((gstPh3CalcInfo.grid_curr.a_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS) || \
                       (gstPh3CalcInfo.grid_curr.b_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS) || \
                       (gstPh3CalcInfo.grid_curr.c_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS))); }, EN_ERR_GRID_CURR_OVER_FAULT);

    ERR_CHECK_INSIDE({ rst = (GetRelayState(EN_RELAT_CTRL_TYPE_GEN) && (gstPcsInvRunState == INV_STATE_GEN_CONNECTED) && ((gstPh3CalcInfo.gen_vol.a_phase_rms > GEN_VOLT_RMS_HIGH) || \
                       (gstPh3CalcInfo.gen_vol.b_phase_rms > GEN_VOLT_RMS_HIGH) || \
                       (gstPh3CalcInfo.gen_vol.c_phase_rms > GEN_VOLT_RMS_HIGH))); }, EN_ERR_GEN_VOL_HIGH_FAULT);

    ERR_CHECK_INSIDE({ rst = (GetRelayState(EN_RELAT_CTRL_TYPE_GEN) && (gstPcsInvRunState == INV_STATE_GEN_CONNECTED) && ((gstPh3CalcInfo.gen_vol.a_phase_rms < GEN_VOLT_RMS_LOW) || \
                       (gstPh3CalcInfo.gen_vol.b_phase_rms < GEN_VOLT_RMS_LOW) || \
                       (gstPh3CalcInfo.gen_vol.c_phase_rms < GEN_VOLT_RMS_LOW))); }, EN_ERR_GEN_VOL_LOW_FAULT);

    ERR_CHECK_INSIDE({ rst = (GetRelayState(EN_RELAT_CTRL_TYPE_GEN) && (gstPcsInvRunState == INV_STATE_GEN_CONNECTED) && ((gstPh3CalcInfo.gen_curr.a_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS) || \
                       (gstPh3CalcInfo.gen_curr.b_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS) || \
                       (gstPh3CalcInfo.gen_curr.c_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS))); }, EN_ERR_GEN_CURR_OVER_FAULT);

    ERR_CHECK_INSIDE({ rst = (GetRelayState(EN_RELAT_CTRL_TYPE_LOAD) &&  Get3PhIsRunFlag() && ((gstPh3CalcInfo.load_vol.a_phase_rms > INV_VOLT_RMS_HIGH) || \
                       (gstPh3CalcInfo.load_vol.b_phase_rms > INV_VOLT_RMS_HIGH) || \
                       (gstPh3CalcInfo.load_vol.c_phase_rms > INV_VOLT_RMS_HIGH))); }, EN_ERR_EPS_VOL_HIGH_FAULT);

//    ERR_CHECK_INSIDE({ rst = (GetRelayState(EN_RELAT_CTRL_TYPE_LOAD) && Get3PhIsRunFlag() && ((gstPh3CalcInfo.load_vol.a_phase_rms < INV_VOLT_RMS_LOW) || \
//                       (gstPh3CalcInfo.load_vol.b_phase_rms < INV_VOLT_RMS_LOW) || \
//                       (gstPh3CalcInfo.load_vol.c_phase_rms < INV_VOLT_RMS_LOW))); }, EN_ERR_EPS_VOL_LOW_FAULT);

    ERR_CHECK_INSIDE({ rst = (GetRelayState(EN_RELAT_CTRL_TYPE_LOAD) && Get3PhIsRunFlag() && ((gstPh3CalcInfo.load_curr.a_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS) || \
                       (gstPh3CalcInfo.load_curr.b_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS) || \
                       (gstPh3CalcInfo.load_curr.c_phase_rms > ERROR_THRESHOLD_INV_CURR_OVER_RMS))); }, EN_ERR_EPS_CURR_OVER_FAULT);
        if(++count >= SCALER)
        {
            count = EN_TIME_DIV_1;
        }
        switch(count)
        {
            case EN_TIME_DIV_1:
            {
                Diagnosis_Error_Adc_Calibration_Fault();
                ERR_CHECK({rst = GetSysTimeOutFlag();},EN_ERR_SOFT_START_OVERTIME);
//                ERRDLY_CHECK({ rst = (ArcIOStateRead() == 1); }, 200, EN_ERR_ARCINS_FAULT);        /*ISO detection*/

                ERRDLY_CHECK({ rst = ((gstAdcResult.inv_temp > ERRORDLY_INV_TEMPERATURE_OVER) || (gstAdcResult.pv_temp > ERRORDLY_PV_TEMPERATURE_OVER)); }, 250, EN_ERR_INV_RADIATOR_OVERHEAT);
//                ERRDLY_CHECK({ rst = ; }, 250, EN_ERR_INV_RADIATOR_OVERHEAT);

                ERRDLY_CHECK({ rst = (gstAdcResult.ctr_temp > ERRORDLY_THRESHOLD_BOX_TEMPERATURE_OVER); }, 250, EN_ERR_INV_CASE_OVERHEAT);//Inverter overtemperature detect, 5 s delay
                ERR_CHECK({ rst = (GetSysTimeOutFlag() && (GetRelaySelfTestResult() != 1)); }, EN_ERR_RELAY_FAULT);//Inverter overtemperature detect, 5 s delay

                ERRDLY_CHECK({rst = (gstPh3CalcInfo.ct_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L1) ||
                    (gstPh3CalcInfo.ct_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.ct_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.ct_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.gen_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L1) ||
                    (gstPh3CalcInfo.gen_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.gen_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.gen_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.inv_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L1) ||
                    (gstPh3CalcInfo.inv_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.inv_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.inv_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.grid_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L1) ||
                    (gstPh3CalcInfo.grid_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.grid_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.grid_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.load_power.total_apperant > ERROR_THRESHOLD_LOAD_OVER_L1) ||
                    (gstPh3CalcInfo.load_power.a_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.load_power.b_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3)) ||
                    (gstPh3CalcInfo.load_power.c_phase_apperant > (ERROR_THRESHOLD_LOAD_OVER_L1 / 3));},6000,EN_ERR_OVERLOAD_1);//Overload level 1

                ERR_CHECK({ rst = (GetNTCFault()); }, EN_ERR_TEM_SENSOR);//Inverter overtemperature detect, 5 s delay
//                ERR_CHECK({ rst = (gstPvData[0].error_code.err.pv_reverse_erro ||\
//                                  gstPvData[1].error_code.err.pv_reverse_erro ||\
//                                  gstPvData[2].error_code.err.pv_reverse_erro); }, EN_ERR_PV_REVERSE_CONNECT);//Inverter overtemperature detect, 5 s delay

                break;
            }
            case EN_TIME_DIV_2:
            {
//                if(gstDeployInfo.work_mode == EN_OIL_MODE)
//                {
                    WARNING_CHECK({ rst = (gstPh3CalcInfo.gen_vol.a_phase_rms < GEN_VOLT_RMS_LOW) ||\
                                          (gstPh3CalcInfo.gen_vol.b_phase_rms < GEN_VOLT_RMS_LOW) ||\
                                          (gstPh3CalcInfo.gen_vol.c_phase_rms < GEN_VOLT_RMS_LOW) ;}, EN_WARNING_GEN_VOL_LOW);     //Generator undervoltage warning

                    WARNING_CHECK({ rst = (gstPh3CalcInfo.gen_vol.a_phase_rms > GEN_VOLT_RMS_HIGH) ||\
                                          (gstPh3CalcInfo.gen_vol.b_phase_rms > GEN_VOLT_RMS_HIGH) ||\
                                          (gstPh3CalcInfo.gen_vol.c_phase_rms > GEN_VOLT_RMS_HIGH); }, EN_WARNING_GEN_VOL_OVER);   //Generator overvoltage warning

                    WARNING_CHECK({ rst = (gstPh3CalcInfo.gen_vol.freq < GRID_FREQ_LOW); }, EN_WARNING_GEN_FREQ_LOW);     //Generator frequency low
                    WARNING_CHECK({ rst = (gstPh3CalcInfo.gen_vol.freq > GRID_FREQ_HIGH); }, EN_WARNING_GEN_FREQ_OVER);   //Generator frequency high
//                }
//                else
//                {
                    WARNING_CHECK({ rst = (gstPh3CalcInfo.grid_vol.a_phase_rms < GRID_VOLT_RMS_LOW) ||\
                                          (gstPh3CalcInfo.grid_vol.b_phase_rms < GRID_VOLT_RMS_LOW) ||\
                                          (gstPh3CalcInfo.grid_vol.c_phase_rms < GRID_VOLT_RMS_LOW) ;}, EN_WARNING_GRID_VOL_LOW);     //Generator undervoltage warning

                    WARNING_CHECK({ rst = (gstPh3CalcInfo.grid_vol.a_phase_rms > GRID_VOLT_RMS_HIGH) ||\
                                          (gstPh3CalcInfo.grid_vol.b_phase_rms > GRID_VOLT_RMS_HIGH) ||\
                                          (gstPh3CalcInfo.grid_vol.c_phase_rms > GRID_VOLT_RMS_HIGH); }, EN_WARNING_GRID_VOL_OVER);   //Generator overvoltage warning

                    WARNING_CHECK({ rst = (gstPh3CalcInfo.grid_vol.freq < GEN_FREQ_LOW); }, EN_WARNING_GRID_FREQ_LOW);     //Generator frequency low
                    WARNING_CHECK({ rst = (gstPh3CalcInfo.grid_vol.freq > GEN_FREQ_HIGH); }, EN_WARNING_GRID_FREQ_OVER);   //Generator frequency high
//                }

                WARNING_CHECK({rst =  Diagnosis_Warning_Fan_Error() == 1 ;},EN_WARNING_FAN_FAULT);
//                WARNINGDLY_CHECK({rst =  gstPvData[0].warning_code.err.pv_vol_low ;},25,EN_WARNING_PV1_VOL_LOW);    /*500ms*/
//                WARNINGDLY_CHECK({rst =  gstPvData[1].warning_code.err.pv_vol_low ;},25,EN_WARNING_PV2_VOL_LOW);    /*500ms*/
//                WARNINGDLY_CHECK({rst =  gstPvData[2].warning_code.err.pv_vol_low ;},25,EN_WARNING_PV3_VOL_LOW);    /*500ms*/
////                WARNINGDLY_CHECK({rst =  _gModuleFaultData.warning_flag.pv4_no_connected_flag ;},25,EN_WARNING_PV4_VOL_LOW);    /*500ms*/
//                WARNINGDLY_CHECK({rst =  gstPvData[2].warning_code.err.pv_loss_control ;},25,EN_WARNING_PV_LOSS_CONTROL);    /*500ms*/
                WARNINGDLY_CHECK({rst = (gstPh3CalcInfo.ct_power.total_apperant > WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.ct_power.a_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.ct_power.b_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.ct_power.c_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.gen_power.total_apperant > WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.gen_power.a_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.gen_power.b_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.gen_power.c_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.inv_power.total_apperant > WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.inv_power.a_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.inv_power.b_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.inv_power.c_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.grid_power.total_apperant > WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.grid_power.a_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.grid_power.b_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.grid_power.c_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.load_power.total_apperant > WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.load_power.a_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.load_power.b_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.load_power.c_phase_apperant > (WARNDLY_THRESHOLD_PWR_OVER / 3));},12000,EN_WARNING_OVER_LOAD);//Overload level 1

                break;
            }
            case EN_TIME_DIV_3:
            {
//                if(gstDeployInfo.work_mode == EN_OIL_MODE)
//                {
                    WARNING_CHECK_CLEAR({ rst = (gstPh3CalcInfo.gen_vol.a_phase_rms > GEN_VOLT_RMS_LOW_RECOVER) ||\
                                          (gstPh3CalcInfo.gen_vol.b_phase_rms > GEN_VOLT_RMS_LOW_RECOVER) ||\
                                          (gstPh3CalcInfo.gen_vol.c_phase_rms > GEN_VOLT_RMS_LOW_RECOVER) ;}, EN_WARNING_GEN_VOL_LOW);     //Generator undervoltage warning

                    WARNING_CHECK_CLEAR({ rst = (gstPh3CalcInfo.gen_vol.a_phase_rms < GEN_VOLT_RMS_HIGH_RECOVER) ||\
                                          (gstPh3CalcInfo.gen_vol.b_phase_rms < GEN_VOLT_RMS_HIGH_RECOVER) ||\
                                          (gstPh3CalcInfo.gen_vol.c_phase_rms < GEN_VOLT_RMS_HIGH_RECOVER); }, EN_WARNING_GEN_VOL_OVER);   //Generator overvoltage warning

                    WARNING_CHECK_CLEAR({ rst = (gstPh3CalcInfo.gen_vol.freq > GRID_FREQ_LOW_RECOVER); }, EN_WARNING_GEN_FREQ_LOW);     //Generator frequency low
                    WARNING_CHECK_CLEAR({ rst = (gstPh3CalcInfo.gen_vol.freq < GRID_FREQ_HIGH_RECOVER); }, EN_WARNING_GEN_FREQ_OVER);   //Generator frequency high
//                }
//                else
//                {
                    WARNING_CHECK_CLEAR({ rst = (gstPh3CalcInfo.grid_vol.a_phase_rms > GRID_VOLT_RMS_LOW_RECOVER) ||\
                                          (gstPh3CalcInfo.grid_vol.b_phase_rms > GRID_VOLT_RMS_LOW_RECOVER) ||\
                                          (gstPh3CalcInfo.grid_vol.c_phase_rms > GRID_VOLT_RMS_LOW_RECOVER) ;}, EN_WARNING_GRID_VOL_LOW);     //Generator undervoltage warning

                    WARNING_CHECK_CLEAR({ rst = (gstPh3CalcInfo.grid_vol.a_phase_rms < GRID_VOLT_RMS_HIGH_RECOVER) ||\
                                          (gstPh3CalcInfo.grid_vol.b_phase_rms < GRID_VOLT_RMS_HIGH_RECOVER) ||\
                                          (gstPh3CalcInfo.grid_vol.c_phase_rms < GRID_VOLT_RMS_HIGH_RECOVER); }, EN_WARNING_GRID_VOL_OVER);   //Generator overvoltage warning

                    WARNING_CHECK_CLEAR({ rst = (gstPh3CalcInfo.grid_vol.freq > GEN_FREQ_LOW_RECOVER); }, EN_WARNING_GRID_FREQ_LOW);     //Generator frequency low
                    WARNING_CHECK_CLEAR({ rst = (gstPh3CalcInfo.grid_vol.freq < GEN_FREQ_HIGH_RECOVER); }, EN_WARNING_GRID_FREQ_OVER);   //Generator frequency high
//                }

                WARNING_CHECK_CLEAR({rst =  (Diagnosis_Warning_Fan_Error() != 1) ;},EN_WARNING_FAN_FAULT);
//                WARNINGDLY_CLEAR({rst =  (_gModuleFaultData.warning_flag.pv1_no_connected_flag == false) ;},25,EN_WARNING_PV1_VOL_LOW);    /*500ms*/
//                WARNINGDLY_CLEAR({rst =  (_gModuleFaultData.warning_flag.pv2_no_connected_flag == false) ;},25,EN_WARNING_PV2_VOL_LOW);    /*500ms*/
//                WARNINGDLY_CLEAR({rst =  (_gModuleFaultData.warning_flag.pv3_no_connected_flag == false) ;},25,EN_WARNING_PV3_VOL_LOW);    /*500ms*/
//                WARNINGDLY_CLEAR({rst =  (_gModuleFaultData.warning_flag.pv4_no_connected_flag == false) ;},25,EN_WARNING_PV4_VOL_LOW);    /*500ms*/
//                WARNINGDLY_CLEAR({rst =  (_gModuleFaultData.warning_flag.pv_loss_control_flag == false) ;},25,EN_WARNING_PV_LOSS_CONTROL);    /*500ms*/
                WARNINGDLY_CLEAR({rst = (gstPh3CalcInfo.ct_power.total_apperant <= WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.ct_power.a_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.ct_power.b_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.ct_power.c_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.gen_power.total_apperant <= WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.gen_power.a_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.gen_power.b_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.gen_power.c_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.inv_power.total_apperant <= WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.inv_power.a_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.inv_power.b_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.inv_power.c_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.grid_power.total_apperant <= WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.grid_power.a_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.grid_power.b_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.grid_power.c_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.load_power.total_apperant <= WARNDLY_THRESHOLD_PWR_OVER) ||
                    (gstPh3CalcInfo.load_power.a_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.load_power.b_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3)) ||
                    (gstPh3CalcInfo.load_power.c_phase_apperant <= (WARNDLY_THRESHOLD_PWR_OVER / 3));},12000,EN_WARNING_OVER_LOAD);//Overload level 1
                break;
            }
            default:
            {
                count = EN_TIME_DIV_1;
                break;
            }
        }
}

/**
* @brief: Diagnosis_Warning_R_Ct_Reverse
* @param: void
* @note: R-phase CT reverse connection
*/
void Diagnosis_Warning_R_Ct_Reverse(void)
{
    gstFaultInfo.inside_warn.bit.r_ct_reverse = 0;
}
/**
* @brief: Diagnosis_Warning_S_Ct_Reverse
* @param: void
* @note: S-phase CT reverse connection
*/
void Diagnosis_Warning_S_Ct_Reverse(void)
{
    gstFaultInfo.inside_warn.bit.s_ct_reverse = 0;
}
/**
* @brief: Diagnosis_Warning_T_Ct_Reverse
* @param: void
* @note: T-phase CT reverse connection
*/
void Diagnosis_Warning_T_Ct_Reverse(void)
{
    gstFaultInfo.inside_warn.bit.t_ct_reverse = 0;
}

/**
* @brief: Diagnosis_Error_Hardware_Fault
* @param: void
* @note: Hardware fault
*/
void Diagnosis_Error_Hardware_Fault(void)
{
    gstFaultInfo.inside_err.bit.hardware_fault = 0;
}

/**
* @brief: Diagnosis_Error_Always_Comm_Error
* @param: void
* @note: Master/slave communication error
*/
void Diagnosis_Error_Always_Comm_Error(void)
{
    gstFaultInfo.inside_err.bit.always_comm_error = 0;
}

/**
* @brief: GetWarn_GridVoltLow
* @param: void
* @note: Get grid undervoltage fault flag
*/
uint16_t GetWarn_GridVoltLow(void)
{
    return (gstFaultInfo.inside_warn.bit.grid_volt_low);
}
/**
* @brief: GetWarn_GridVoltHigh
* @param: void
* @note: Get grid overvoltage fault flag
*/
uint16_t GetWarn_GridVoltHigh(void)
{
    return (gstFaultInfo.inside_warn.bit.grid_volt_high);
}
/**
* @brief: GetWarn_GridFreqLow
* @param: void
* @note: Get grid underfrequency fault flag
*/
uint16_t GetWarn_GridFreqLow(void)
{
    return (gstFaultInfo.inside_warn.bit.grid_freq_low);
}
/**
* @brief: GetWarn_GridFreqHigh
* @param: void
* @note: Get grid overfrequency fault flag
*/
uint16_t GetWarn_GridFreqHigh(void)
{
    return (gstFaultInfo.inside_warn.bit.grid_freq_high);
}
/**
* @brief: GetWarn_GenVoltLow
* @param: void
* @note: Get generator undervoltage flag
*/
uint16_t GetWarn_GenVoltLow(void)
{
    return (gstFaultInfo.inside_warn.bit.gen_volt_low);
}
/**
* @brief: GetWarn_GenVoltHigh
* @param: void
* @note: Get generator overvoltage flag
*/
uint16_t GetWarn_GenVoltHigh(void)
{
    return (gstFaultInfo.inside_warn.bit.gen_volt_high);
}
/**
* @brief: GetWarn_GenFreqLow
* @param: void
* @note: Get generator underfrequency flag
*/
uint16_t GetWarn_GenFreqLow(void)
{
    return (gstFaultInfo.inside_warn.bit.gen_freq_low);
}
/**
* @brief: GetWarn_GenFreqHigh
* @param: void
* @note: Get generator overfrequency flag
*/
uint16_t GetWarn_GenFreqHigh(void)
{
    return (gstFaultInfo.inside_warn.bit.gen_freq_high);
}
/**
* @brief: GetWarn_Pv1NoConnected
* @param: void
* @note: Get PV1 not connected fault flag
*/
uint16_t GetWarn_Pv1NoConnected(void)
{
    return (gstFaultInfo.inside_warn.bit.pv1_no_connected);
}
/**
* @brief: GetWarn_Pv2NoConnected
* @param: void
* @note: Get PV2 not connected fault flag
*/
uint16_t GetWarn_Pv2NoConnected(void)
{
    return (gstFaultInfo.inside_warn.bit.pv2_no_connected);
}
/**
* @brief: GetWarn_Pv3NoConnected
* @param: void
* @note: Get PV3 not connected fault flag
*/
uint16_t GetWarn_Pv3NoConnected(void)
{
    return (gstFaultInfo.inside_warn.bit.pv3_no_connected);
}
/**
* @brief: GetWarn_Pv4NoConnected
* @param: void
* @note: Get PV4 not connected fault flag
*/
uint16_t GetWarn_Pv4NoConnected(void)
{
    return (gstFaultInfo.inside_warn.bit.pv4_no_connected);
}
/**
* @brief: GetWarn_PvLoseControl
* @param: void
* @note: Get PV loss-of-control fault flag
*/
uint16_t GetWarn_PvLoseControl(void)
{
    return (gstFaultInfo.inside_warn.bit.pv_loss_control);
}
/**
* @brief: GetWarn_OverLoad
* @param: void
* @note: Get minor overload fault flag
*/
uint16_t GetWarn_OverLoad(void)
{
    return (gstFaultInfo.inside_warn.bit.overload);
}
/**
* @brief: GetWarn_LeakCurrHigh
* @param: void
* @note: Get leakage overcurrent fault flag
*/
uint16_t GetWarn_LeakCurrHigh(void)
{
    return (gstFaultInfo.inside_warn.bit.leak_curr_high);
}
/**
* @brief: GetWarn_FanError
* @param: void
* @note: Get fan fault flag
*/
uint16_t GetWarn_FanError(void)
{
    return (gstFaultInfo.inside_warn.bit.fan_error);
}
/**
* @brief: GetWarn_R_Ct_Reverse
* @param: void
* @note: Get R-phase CT reverse-connection fault flag
*/
uint16_t GetWarn_R_Ct_Reverse(void)
{
    return (gstFaultInfo.inside_warn.bit.r_ct_reverse);
}
/**
* @brief: GetWarn_S_Ct_Reverse
* @param: void
* @note: Get S-phase CT reverse-connection fault flag
*/
uint16_t GetWarn_S_Ct_Reverse(void)
{
    return (gstFaultInfo.inside_warn.bit.s_ct_reverse);
}
/**
* @brief: GetWarn_T_Ct_Reverse
* @param: void
* @note: Get T-phase CT reverse-connection fault flag
*/
uint16_t GetWarn_T_Ct_Reverse(void)
{
    return (gstFaultInfo.inside_warn.bit.t_ct_reverse);
}
/**
* @brief: GetError_StartTimeout
* @param: void
* @note: Get soft-start timeout fault flag
*/
uint16_t GetError_StartTimeout(void)
{
    return (gstFaultInfo.inside_err.bit.start_timeout);
}
/**
* @brief: GetError_InvOutShort
* @param: void
* @note: Get inverter output short fault flag
*/
uint16_t GetError_InvOutShort(void)
{
    return (gstFaultInfo.inside_err.bit.inv_out_short);
}
/**
* @brief: GetError_InvVoltLow
* @param: void
* @note: Get inverter undervoltage fault flag
*/
uint16_t GetError_InvVoltLow(void)
{
    return (gstFaultInfo.inside_err.bit.inv_volt_low);
}
/**
* @brief: GetError_InvVoltHigh
* @param: void
* @note: Get inverter overvoltage fault flag
*/
uint16_t GetError_InvVoltHigh(void)
{
    return (gstFaultInfo.inside_err.bit.inv_volt_high);
}
/**
* @brief: GetError_BusDsgOverCurr
* @param: void
* @note: Get bus discharge overcurrent fault flag
*/
uint16_t GetError_BusDsgOverCurr(void)
{
    return (gstFaultInfo.inside_err.bit.bus_dsg_over_curr);
}
/**
* @brief: GetError_BusChgOverCurr
* @param: void
* @note: Get bus charge overcurrent fault flag
*/
uint16_t GetError_BusChgOverCurr(void)
{
    return (gstFaultInfo.inside_err.bit.bus_chg_over_curr);
}
/**
* @brief: GetError_BusVoltLow
* @param: void
* @note: Get bus undervoltage fault flag
*/
uint16_t GetError_BusVoltLow(void)
{
    return (gstFaultInfo.inside_err.bit.bus_volt_low);
}
/**
* @brief: GetError_BusVoltHigh
* @param: void
* @note: Get bus overvoltage fault flag
*/
uint16_t GetError_BusVoltHigh(void)
{
    return (gstFaultInfo.inside_err.bit.bus_volt_high);
}
/**
* @brief: GetError_BusShort
* @param: void
* @note: Get bus short-circuit fault flag
*/
uint16_t GetError_BusShort(void)
{
    return (gstFaultInfo.inside_err.bit.bus_short);
}
/**
* @brief: GetError_HardwareFault
* @param: void
* @note: Get hardware fault flag
*/
uint16_t GetError_HardwareFault(void)
{
    return (gstFaultInfo.inside_err.bit.hardware_fault);
}
/**
* @brief: GetError_InvOverCurr
* @param: void
* @note: Get inverter overcurrent fault flag
*/
uint16_t GetError_InvOverCurr(void)
{
    return (gstFaultInfo.inside_err.bit.inv_over_curr);
}
/**
* @brief: GetError_DcComponentHigh
* @param: void
* @note: Get DC component high fault flag
*/
uint16_t GetError_DcComponentHigh(void)
{
    return (gstFaultInfo.inside_err.bit.dc_component_high);
}
/**
* @brief: GetError_BoxOverTemp
* @param: void
* @note: Get enclosure overtemperature fault flag
*/
uint16_t GetError_BoxOverTemp(void)
{
    return (gstFaultInfo.inside_err.bit.box_over_temp);
}
/**
* @brief: GetError_RadiatorOverTemp
* @param: void
* @note: Get heatsink overtemperature fault flag
*/
uint16_t GetError_RadiatorOverTemp(void)
{
    return (gstFaultInfo.inside_err.bit.radiator_over_temp);
}
/**
* @brief: GetError_InvTzFault
* @param: void
* @note: Get inverter TZ protection fault flag
*/
uint16_t GetError_InvTzFault(void)
{
    return (gstFaultInfo.inside_err.bit.inv_tz_fault);
}
/**
* @brief: GetError_InvOutFault
* @param: void
* @note: Get inverter output abnormal fault flag
*/
uint16_t GetError_InvOutFault(void)
{
    return (gstFaultInfo.inside_err.bit.inv_out_fault);
}
/**
* @brief: GetError_InvOverLoad1
* @param: void
* @note: Get inverter overload L1 fault flag
*/
uint16_t GetError_InvOverLoad1(void)
{
    return (gstFaultInfo.inside_err.bit.always_over_load1);
}
/**
* @brief: GetError_InvOverLoad2
* @param: void
* @note: Get inverter overload L2 fault flag
*/
uint16_t GetError_InvOverLoad2(void)
{
    return (gstFaultInfo.inside_err.bit.always_over_load2);
}
/**
* @brief: GetError_R_Ct_Disconnect
* @param: void
* @note: Get R-phase CT disconnect fault flag
*/
uint16_t GetError_R_Ct_Disconnect(void)
{
    return (gstFaultInfo.inside_err.bit.r_ct_disconnect);
}
/**
* @brief: GetError_S_Ct_Disconnect
* @param: void
* @note: Get S-phase CT disconnect fault flag
*/
uint16_t GetError_S_Ct_Disconnect(void)
{
    return (gstFaultInfo.inside_err.bit.s_ct_disconnect);
}
/**
* @brief: GetError_T_Ct_Disconnect
* @param: void
* @note: Get T-phase CT disconnect fault flag
*/
uint16_t GetError_T_Ct_Disconnect(void)
{
    return (gstFaultInfo.inside_err.bit.t_ct_disconnect);
}
/**
* @brief: GetError_Adc_Calibration_Fault
* @param: void
* @note: Get sampling calibration fault flag
*/
uint16_t GetError_Adc_Calibration_Fault(void)
{
    return (gstFaultInfo.inside_err.bit.samping_sensor);
}
/**
* @brief: GetError_Comm_Error
* @param: void
* @note: Get master/slave comm fault flag
*/
uint16_t GetError_Comm_Error(void)
{
    return (gstFaultInfo.inside_err.bit.always_comm_error);
}
/**
* @brief: GetError_IsoAndArcFault
* @param: void
* @note: Get arc/insulation detection fault flag
*/
uint16_t GetError_IsoAndArcFault(void)
{
    return (gstFaultInfo.inside_err.bit.arcins_fault);
}
/**
  * @brief  GetErrorFlag
  * @param  None
  * @return Fault flags
  * @note   None
*/
uint16_t GetErrorFlag(void)
{
    return (gstFaultInfo.err_flag);
}
uint16_t LoadRelayNoneFaultEn(void)
{
    return _gLoadRelayOpenEn;
}
