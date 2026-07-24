/*
 * post.c
 *
 *  Created on: 2025-11-10
 *      Author: Nova006348
 */
#include "post.h"
#include <fan.h>
#include <ntc.h>
#include "rcmu.h"
#include "relay_ctrl.h"
#include "cpu_timers.h"
#include "fault.h"
#include "cla_cpu_share.h"
#include "ph3_calc_ifc_update.h"
#include "algo_interface.h"
#include "debug_analysis.h"
#include <sample_update.h>
SysSelfTest_t _gSysSelfTest;

/**
* @brief: SysSelfTestInit
* @param: Nothing
* @return:Nothing
* @note:System self-test initialization
*/
void SysSelfTestInit(void)
{
    _gSysSelfTest.complete_flag = false;
    _gSysSelfTest.retest_request_flag = false;
    _gSysSelfTest.max_test_time = SYS_SELF_TEST_TIMEOUT;
    _gSysSelfTest.module_state.byte = 0;
    _gSysSelfTest.timeout_flag = false;
}
/**
* @brief: _gFanSelfTest
* @param: Nothing
* @return:Nothing
* @note:Fan self-test
*/
static void _gFanSelfTest(void)
{
    if(!GetFanFaultState())
    {
        _gSysSelfTest.module_state.bit.fan_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.fan_test_complete = 0;
    }
}
/**
* @brief: _gNtcSelfTest
* @param: Nothing
* @return:Nothing
* @note:NTC self-test
*/
static void _gNtcSelfTest(void)
{
    if(!GetNTCFault())
    {
        _gSysSelfTest.module_state.bit.ntc_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.ntc_test_complete = 0;
    }
}
/**
* @brief: _gAdcSelfTest
* @param: Nothing
* @return:Nothing
* @note:ADC self-test
*/
static void _gAdcSelfTest(void)
{
    if(!GetError_Adc_Calibration_Fault())
    {
        _gSysSelfTest.module_state.bit.adc_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.adc_test_complete = 0;
    }
}
/**
* @brief: _gLeakCurrentSelfTest
* @param: Nothing
* @return:Nothing
* @note:Leakage current self-test
*/
static void _gLeakCurrentSelfTest(void)
{
    if(GetLeakCurrSelfTestState() == 1)
    {
        _gSysSelfTest.module_state.bit.Leak_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.Leak_test_complete = 0;
    }
}
/**
* @brief: _gRelaySelfTestProcess
* @param: Nothing
* @return:Nothing
* @note:Relay self-test
*/
static void _gRelaySelfTestProcess(void)
{
    RelaySelfTestInValue_t* relay_in_value = GetRelaySelfInputValue();
    relay_in_value -> grid_a_vol_instant = gstAdcResult.grid_volt_phase_a;
    relay_in_value -> grid_b_vol_instant = gstAdcResult.grid_volt_phase_b;
    relay_in_value -> grid_c_vol_instant = gstAdcResult.grid_volt_phase_c;

    relay_in_value -> mid_a_vol_instant = gstAdcResult.relay_middle_volt_phase_a;
    relay_in_value -> mid_b_vol_instant = gstAdcResult.relay_middle_volt_phase_b;
    relay_in_value -> mid_c_vol_instant = gstAdcResult.relay_middle_volt_phase_c;

    relay_in_value -> grid_a_vol_rms = gstPh3CalcInfo.grid_vol.a_phase_rms;
    relay_in_value -> grid_b_vol_rms = gstPh3CalcInfo.grid_vol.b_phase_rms;
    relay_in_value -> grid_c_vol_rms = gstPh3CalcInfo.grid_vol.c_phase_rms;

    relay_in_value -> mid_a_vol_rms = gstPh3CalcInfo.mid_vol.a_phase_rms;
    relay_in_value -> mid_b_vol_rms = gstPh3CalcInfo.mid_vol.b_phase_rms;
    relay_in_value -> mid_c_vol_rms = gstPh3CalcInfo.mid_vol.c_phase_rms;

    relay_in_value ->Inv_a_vol_rms = gstPh3CalcInfo.inv_vol.a_phase_rms;
    relay_in_value ->Inv_b_vol_rms = gstPh3CalcInfo.inv_vol.b_phase_rms;
    relay_in_value ->Inv_c_vol_rms = gstPh3CalcInfo.inv_vol.c_phase_rms;

    relay_in_value ->Gen_a_vol_rms = gstPh3CalcInfo.gen_vol.a_phase_rms;
    relay_in_value ->Gen_b_vol_rms = gstPh3CalcInfo.gen_vol.b_phase_rms;
    relay_in_value ->Gen_c_vol_rms = gstPh3CalcInfo.gen_vol.c_phase_rms;

    relay_in_value ->Load_a_vol_rms = gstPh3CalcInfo.load_vol.a_phase_rms;
    relay_in_value ->Load_b_vol_rms = gstPh3CalcInfo.load_vol.b_phase_rms;
    relay_in_value ->Load_c_vol_rms = gstPh3CalcInfo.load_vol.c_phase_rms;
    RelaySelfTest();

    if(GetRelaySelfTestResult() == 1)
    {
        _gSysSelfTest.module_state.bit.relay_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.relay_test_complete = 0;
    }
}
/**
* @brief: _gISOSelfTest
* @param: Nothing
* @return:Nothing
* @note:ISO self-test
*/
static void _gISOSelfTest(void)
{
    if(!GetError_IsoAndArcFault())
    {
        _gSysSelfTest.module_state.bit.iso_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.iso_test_complete = 0;
    }
}

/**
* @brief: SysSelfTest
* @param: void
* @return:Nothing
* @note:System self-test; PV/inverter run only after flag is set
*/
/************************************/
// * TODO: gen relay/start-mode detection on hardware start
//If no gen start, enable PV start
//Verify hardware BOOT detection
 /************************************/
void SysSelfTest(void)
{

    static uint32_t timeout;
    static bool first_run_flag = true;
    if(first_run_flag)
    {
        first_run_flag = false;
        timeout = TimGetSystime();
    }
//        SetCpuDebugData(10,_gSysSelfTest.module_state.bit.adc_test_complete);//26
//        SetCpuDebugData(11,_gSysSelfTest.module_state.bit.fan_test_complete);
//        SetCpuDebugData(12,_gSysSelfTest.module_state.bit.ntc_test_complete);
//        SetCpuDebugData(13,_gSysSelfTest.module_state.bit.Leak_test_complete);
//        SetCpuDebugData(14,_gSysSelfTest.module_state.bit.relay_test_complete);
//        SetCpuDebugData(15,_gSysSelfTest.module_state.bit.iso_test_complete);//31
    if(_gSysSelfTest.retest_request_flag)       /*Re-run self-test*/
    {
        _gSysSelfTest.retest_request_flag = false;
        _gSysSelfTest.timeout_flag = false;
        timeout = TimGetSystime();
        first_run_flag = true;
    }
    if(_gSysSelfTest.complete_flag || _gSysSelfTest.timeout_flag)     /*Exit on self-test success*/
    {
        return;
    }
    /*Self-test code*/
    _gAdcSelfTest();
    _gFanSelfTest();
    if(_gSysSelfTest.module_state.bit.adc_test_complete)
    {
        _gNtcSelfTest();
        _gLeakCurrentSelfTest();
        _gRelaySelfTestProcess();
        _gISOSelfTest();
    }
    if(_gSysSelfTest.module_state.byte == 63 && TimDifftimeFromNow(timeout)  > SYS_TEST_TIME)
    {
        _gSysSelfTest.complete_flag = true;
        RelayControl(EN_RELAT_CTRL_TYPE_INV,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_GRID,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_GEN,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_LOAD,RELAY_OFF);
    }

    if(TimDifftimeFromNow(timeout)  > SYS_SELF_TEST_TIMEOUT)    /*Timeout handling*/
    {
        _gSysSelfTest.timeout_flag = true;
        _gSysSelfTest.complete_flag =false;
        RelayControl(EN_RELAT_CTRL_TYPE_INV,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_GRID,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_GEN,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_LOAD,RELAY_OFF);
    }

}

/**
* @brief: SysRetestRequestSet
* @param: value
* @return:Nothing
* @note:Set system self-test request
*/
void SysRetestRequestSet(bool value)
{
    _gSysSelfTest.retest_request_flag = value;
}
/**
* @brief: GetSysTimeOutFlag
* @param: Nothing
* @return:_gSelfTestSysPermit
* @note:Get system timeout flag
*/
bool GetSysTimeOutFlag(void)
{
    return _gSysSelfTest.timeout_flag;
}
/**
* @brief: GetSysSelfTestFlag
* @param: Nothing
* @return:_gSelfTestSysPermit
* @note:Get system self-test result
*/
bool GetSysSelfTestFlag(void)
{
    return _gSysSelfTest.complete_flag;
}
