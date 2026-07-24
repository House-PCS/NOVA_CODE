/*
 * post.c
 *
 *  Created on: 2025年11月10日
 *      Author: Nova006348
 */
#include "SysSelfTest.h"
#include <fan.h>
//#include <ntc.h>
#include "rcmu.h"
#include "relay_ctrl.h"
#include "cpu_timers.h"
#include "fault.h"
#include "cla_cpu_share.h"
#include "adc.h"
#include <algo_interface.h>
 
#include "inv_state.h"
#include "cpu_control.h"
SysSelfTest_t _gSysSelfTest;

/**
* @brief: SysSelfTestInit
* @param: Nothing
* @return:Nothing
* @note:系统自检自测试初始化
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
* @note:风扇自检函数
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
* @note:NTC自检函数
*/
// static void _gNtcSelfTest(void)
// {
//     if(!GetNTCFault())
//     {
//         _gSysSelfTest.module_state.bit.ntc_test_complete = 1;
//     }
//     else
//     {
//         _gSysSelfTest.module_state.bit.ntc_test_complete = 0;
//     }
// }
/**
* @brief: _gAdcSelfTest
* @param: Nothing
* @return:Nothing
* @note:ADC自检函数
*/
// static void _gAdcSelfTest(void)
// {
//     if(!GetError_Adc_Calibration_Fault())
//     {
//         _gSysSelfTest.module_state.bit.adc_test_complete = 1;
//     }
//     else
//     {
//         _gSysSelfTest.module_state.bit.adc_test_complete = 0;
//     }
// }
/**
* @brief: _gLeakCurrentSelfTest
* @param: Nothing
* @return:Nothing
* @note:漏电流自检函数
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
* @note:继电器自检函数
*/
static void _gRelaySelfTestProcess(void)
{
    RelaySelfTestInValue_t* relay_in_value = GetRelaySelfInputValue();
    relay_in_value -> gird_a_vol_instant = gstAdcResult.grid_vol;
    relay_in_value -> mid_a_vol_instant = gstAdcResult.relay_middle_volt_phase_a;
    relay_in_value -> gird_a_vol_rms = gstAlgoInterface.grid_vol_rms;
    relay_in_value -> mid_a_vol_rms = gstAlgoInterface.relay_middle_volt_phase_a_rms;
    //relay_in_value -> mid_a_vol_rms = 60;
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
* @note:ISO自检函数
*/
// static void _gISOSelfTest(void)
// {
//     if(GetError_IsoAndArcFault())
//     {
//         _gSysSelfTest.module_state.bit.iso_test_complete = 1;
//     }
//     else
//     {
//         _gSysSelfTest.module_state.bit.iso_test_complete = 0;
//     }
// }

/**
* @brief: _gGenStartRelaySelfTest
* @param: Nothing
* @return:Nothing
* @note:油机取电继电器自检函数
*/
static void _gGenStartRelaySelfTest(void)
{
    static uint32_t timecount;
    if (gstAlgoInterface.bus_vol > 400.0f) //母线电压建立后，断开油机取电继电器
    {
        timecount ++;
        if (timecount > 1000)
        {
            RELAY_GEN_START_OPEN;
        }                
    }

    if (GSTATE_RELAY_GEN_START == 1)
    {
        _gSysSelfTest.module_state.bit.gen_startrelay_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.gen_startrelay_test_complete = 0;     
    }
    
}


/**
* @brief: SysSelfTest
* @param: void
* @return:Nothing
* @note:系统自检，该标志位置位后才可运行PV与逆变
*/
/************************************/
// * 在里面还需添加油机继电器识别与启机方式识别（检测到油机硬件启机时需要打开油机继电器后再关闭油机启机继电器)
//若无油机启机，则此时需设置PV启机
//还需检测硬件BOOT是否识别无异常
 /************************************/
void SysSelfTest(void)
{
    static uint32_t timeout;
    if(_gSysSelfTest.retest_request_flag)       /*重新自检*/
    {
        _gSysSelfTest.retest_request_flag = false;
        _gSysSelfTest.timeout_flag = false;
    }
    if(_gSysSelfTest.complete_flag || _gSysSelfTest.timeout_flag)     /*自检成功退出*/
    {
        return;
    }
    /*自检代码*/
    _gFanSelfTest();
//    _gNtcSelfTest();
//    _gAdcSelfTest();
    _gLeakCurrentSelfTest();
    _gRelaySelfTestProcess();
    _gGenStartRelaySelfTest();
//    _gISOSelfTest();
    if(_gSysSelfTest.module_state.bit.Leak_test_complete &&
       _gSysSelfTest.module_state.bit.fan_test_complete &&
       _gSysSelfTest.module_state.bit.relay_test_complete &&
       _gSysSelfTest.module_state.bit.gen_startrelay_test_complete
    )
    {
        _gSysSelfTest.complete_flag = true;
        RELAY_TOTAL_CLOSE;
        RELAY_GRID_CLOSE;
        RELAY_GEN_CLOSE;
        gSelfTestEnPvRun = 1;
        gstInvAction.self_test_disconnect_permit = 1;
    }
    if(TimDifftimeFromNow(timeout)  > SYS_SELF_TEST_TIMEOUT)    /*超时处理*/
    {
        _gSysSelfTest.timeout_flag = true;
        RELAY_TOTAL_CLOSE;
        RELAY_GRID_CLOSE;
        RELAY_GEN_CLOSE;
        gSelfTestEnPvRun = 0;
        gstInvAction.self_test_disconnect_permit = 0;
    }

}

/**
* @brief: SysRetestRequestSet
* @param: value
* @return:Nothing
* @note:设置系统自检请求
*/
void SysRetestRequestSet(bool value)
{
    _gSysSelfTest.retest_request_flag = value;
}
/**
* @brief: GetSysTimeOutFlag
* @param: Nothing
* @return:_gSelfTestSysPermit
* @note:获取系统是否超时
*/
bool GetSysTimeOutFlag(void)
{
    return _gSysSelfTest.timeout_flag;
}
/**
* @brief: GetSysSelfTestFlag
* @param: Nothing
* @return:_gSelfTestSysPermit
* @note:获取系统自检结果
*/
bool GetSysSelfTestFlag(void)
{
    return _gSysSelfTest.complete_flag;
}
