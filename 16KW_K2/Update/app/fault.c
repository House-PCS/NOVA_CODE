/*
 * fault.c
 *
 *  Created on: 2025年3月26日
 *      Author: Nova006352
 */

#include <fault.h>
#include "cla_cpu_share.h"
#include "inv_state.h"
 
#include "ethernet_analysis.h"
#include "fan.h"
#include "dev_ntc.h"
#include "energy_management.h"
#include "energy_ctrl.h"
#include "adc.h"
#include "data_process.h"
#include "SysSelfTest.h"
#include "rcmu.h"
#include "relay_ctrl.h"
#include <algo_interface.h>
#include "debug_analysis.h"
#include "modbus_analysis.h"
#include "AlgoFw/package/cntl/pv/pv.h"
#include "AlgoFw/package/cntl/pv/pv_def.h"
#include "dev_dual_core_receive.h"
#include "dual_core.h"
#include "firmware_upgrate.h"

#define SCALER 5
#define GPIO_ARC 54

FaultInfo_t gstFaultInfo;
EN_TIME_DIV count;
sliding_window_t CTcurr_detect;
bool CT_DisConnectCheck;
float32 CTcurr_data_buffer[CT_ADCSIZE];

uint16_t gCode = 0;
float32 Pv_Total_Pwr_Ave;

uint8_t Diagnosis_Warning_Fan_Error(void);

// #pragma CODE_SECTION(ErrWarningHandler,".TI.ramfunc");
// #pragma CODE_SECTION(CT_Var_DisConnectCheck,".TI.ramfunc");
#pragma CODE_SECTION(_ErroTrig,".TI.ramfunc");
#pragma CODE_SECTION(_WarningTrig,".TI.ramfunc");
#pragma CODE_SECTION(_ClearwarningTrig,".TI.ramfunc");


/*
* @brief: CountTriggerInit
* @param: CountTrigger_t* ，uint32_t
* @return:Nothing
* @note:计数触发器初始化
* */
void CountTriggerInit(CountTrigger_t* p_trigger, uint32_t threshold)
{
    if(p_trigger == 0)
    {
        return;
    }
    p_trigger->trig_threshold = threshold;
    p_trigger->trig_hook = 0;
    p_trigger->trigged = 0;
    p_trigger->count_interval = 1;
}

void CountTriggerReset(CountTrigger_t* p_trigger)
{
    if(p_trigger == 0)
    {
        return;
    }
    p_trigger->trigged = 0;
    p_trigger->count = 0;
}

#pragma CODE_SECTION(CountTriggerRun,".TI.ramfunc");
void CountTriggerRun(CountTrigger_t* p_trigger, uint8_t value)
{
    if(p_trigger == 0)
    {
        return;
    }
    if(value == 1)
        p_trigger->count += p_trigger->count_interval;
    else if(value == 0)
        p_trigger->count -= p_trigger->count_interval;

    if(p_trigger->count < 0)
        p_trigger->count = 0;
    if(p_trigger->count >= p_trigger->trig_threshold)
    {
        p_trigger->trigged = 1;
        if(p_trigger->trig_hook != 0)
        {
            p_trigger->trig_hook(p_trigger->hook_param);
        }
        p_trigger->count = 0;
    }
}

static void _SetErrCode(uint16_t code, uint16_t* field_array)
{
    if(field_array != 0)
    {
        field_array[(code & 0xFFF0) >> 4] |= (1 << (code & 0x000F));
    }
}

static void _ResetErrCode(uint16_t code, uint16_t* field_array)
{
    if(field_array != 0)
    {
        field_array[(code & 0xFFF0) >> 4] &= ~(1 << (code & 0x000F));
    }
}


static void _SetWarningCode(uint16_t code, uint16_t* field_array)
{
    if(field_array != 0)
    {
        field_array[(code & 0xFFF0) >> 4] |= (1 << (code & 0x000F));
    }
}

static void _ResetWarningCode(uint16_t code, uint16_t* field_array)
{
    if(field_array != 0)
    {
        field_array[(code & 0xFFF0) >> 4] &= ~(1 << (code & 0x000F));
    }
}

typedef struct {
    uint16_t setcode;
    uint16_t* field_array;
} Param_t;

static void _ErroTrig(void* param)
{
    Param_t* p_param = (Param_t*)param;
    if(p_param != 0)
    {
        gstFaultInfo.err_flag |= 1;
        if(p_param->field_array != 0)
            _SetErrCode(p_param->setcode, p_param->field_array);
    }
}


void _ClearErrTrig(void* param)
{
    Param_t* p_param = (Param_t*)param;
    if(p_param != 0 && p_param->field_array != 0)
    {
        _ResetErrCode(p_param->setcode, p_param->field_array);
    }
}

void _WarningTrig(void* param)
{
    Param_t* p_param = (Param_t*)param;
    if(p_param != 0)
    {
        gstFaultInfo.warning_flag |= 1;
        if(p_param->field_array != 0)
            _SetWarningCode(p_param->setcode, p_param->field_array);
    }
}

void _ClearwarningTrig(void* param)
{
    Param_t* p_param = (Param_t*)param;
    if(p_param != 0 && p_param->field_array != 0)
    {
        _ResetWarningCode(p_param->setcode, p_param->field_array);
    }
}

#define ERR_CHECK(cmd, code, field)            \
{cmd}                                          \
if(rst == 1) {                                 \
    gstFaultInfo.err_flag |= 1;                \
    _SetErrCode(code, (uint16_t*)&gstFaultInfo.inside_err_##field.byte); \
}

#define ERR_CHECK_CLEAR(cmd, code, field)      \
{cmd}                                          \
if(rst == 1) {                                 \
    _ResetErrCode(code, (uint16_t*)&gstFaultInfo.inside_err_##field.byte); \
}

#define WARNING_CHECK(cmd, code, field)        \
{cmd}                                          \
if(rst == 1) {                                 \
    gstFaultInfo.warning_flag |= 1;            \
    _SetWarningCode(code, (uint16_t*)&gstFaultInfo.inside_warn_##field.byte); \
}

#define WARNING_CHECK_CLEAR(cmd, code, field)  \
{cmd}                                          \
if(rst == 1) {                                 \
    _ResetWarningCode(code, (uint16_t*)&gstFaultInfo.inside_warn_##field.byte); \
}

/***********/
/*故障计数检测宏*/
#define ERRDLY_CHECK_ALIGN(cmd, ID, threshold, code, field)    \
static Param_t Param_##ID = {                                  \
    .setcode = code,                                              \
    .field_array = (uint16_t*)&gstFaultInfo.inside_err_##field.byte        \
};                                                             \
static CountTrigger_t errDlyTrigger_##ID =                     \
{                                                              \
    .trigged = 0,                                              \
    .count = 0,                                                \
    .count_interval = 1,                                       \
    .hook_param = &Param_##ID,                                 \
    .trig_hook = _ErroTrig,                                    \
    .trig_threshold = threshold ,                              \
};                                                             \
{cmd};                                                          \
CountTriggerRun(&errDlyTrigger_##ID, rst);
#define ERRDLY_CHECK_ALIAS(cmd,ID,threshold,code,field) ERRDLY_CHECK_ALIGN(cmd,ID,threshold,code,field)
#define ERRDLY_CHECK(cmd,threshold,code,field) ERRDLY_CHECK_ALIAS(cmd,__LINE__,threshold,code,field)


/**************/
/*告警清楚计数检测宏*/
#define ERRDLY_CLEAR_ALIGN(cmd,ID,threshold,code,field)                                    \
static Param_t Param_##ID = {                                                              \
    .setcode = code,                                                                       \
    .field_array = (uint16_t*)&gstFaultInfo.inside_err_##field.byte                                   \
};                                                                                         \
static CountTrigger_t errDlyClearTrigger##ID=                                              \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &Param_##ID,                                                             \
    .trig_hook = _ClearErrTrig,                                                            \
    .trig_threshold = threshold ,                                                          \
};                                                                                         \
{cmd};                                                                                     \
CountTriggerRun(&errDlyClearTrigger##ID,rst);
#define ERRDLY_CLEAR_ALIAS(cmd,ID,threshold,code,field) ERRDLY_CLEAR_ALIGN(cmd,ID,threshold,code,field)
#define ERRDLY_CLEAR(cmd,threshold,code,field) ERRDLY_CLEAR_ALIAS(cmd,__LINE__,threshold,code,field)



/***********/
/*告警计数检测宏*/
#define WARNINGDLY_CHECK_ALIGN(cmd,ID,threshold,code,field)                                \
static Param_t Param_##ID = {                                                              \
    .setcode = code,                                                                          \
    .field_array = (uint16_t*)&gstFaultInfo.inside_warn_##field.byte                                   \
};                                                                                         \
static CountTrigger_t warningDlyTrigger##ID=                                               \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &Param_##ID,                                                             \
    .trig_hook = _WarningTrig,                                                             \
    .trig_threshold = threshold ,                                                  \
};                                                                                         \
{cmd};                                                                                      \
CountTriggerRun(&warningDlyTrigger##ID,rst);
#define WARNINGDLY_CHECK_ALIAS(cmd,ID,threshold,code,field) WARNINGDLY_CHECK_ALIGN(cmd,ID,threshold,code,field)
#define WARNINGDLY_CHECK(cmd,threshold,code,field) WARNINGDLY_CHECK_ALIAS(cmd,__LINE__,threshold,code,field)


/**************/
/*告警清楚计数检测宏*/
#define WARNINGDLY_CLEAR_ALIGN(cmd,ID,threshold,code,field)                                \
static Param_t Param_##ID = {                                                              \
    .setcode = code,                                                                          \
    .field_array = (uint16_t*)&gstFaultInfo.inside_warn_##field.byte                                   \
};                                                                                         \
static CountTrigger_t warningDlyClearTrigger##ID=                                          \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &Param_##ID,                                                             \
    .trig_hook = _ClearwarningTrig,                                                        \
    .trig_threshold = threshold ,                                                          \
};                                                                                         \
{cmd};                                                                                      \
CountTriggerRun(&warningDlyClearTrigger##ID,rst);
#define WARNINGDLY_CLEAR_ALIAS(cmd,ID,threshold,code,field) WARNINGDLY_CLEAR_ALIGN(cmd,ID,threshold,code,field)
#define WARNINGDLY_CLEAR(cmd,threshold,code,field) WARNINGDLY_CLEAR_ALIAS(cmd,__LINE__,threshold,code,field)



#define ERR_CHECK_GRID(cmd, code)    ERR_CHECK(cmd, code, gird)
#define ERR_CHECK_PV(cmd, code)      ERR_CHECK(cmd, code, pv)
#define ERR_CHECK_BUS(cmd, code)     ERR_CHECK(cmd, code, bus)
#define ERR_CHECK_INV(cmd, code)     ERR_CHECK(cmd, code, inv)
#define ERR_CHECK_LOAD(cmd, code)    ERR_CHECK(cmd, code, load)
#define ERR_CHECK_TEMP(cmd, code)    ERR_CHECK(cmd, code, temp)
#define ERR_CHECK_GEN(cmd, code)     ERR_CHECK(cmd, code, gen)
#define ERR_CHECK_OTHER(cmd, code)   ERR_CHECK(cmd, code, other)

#define ERR_CHECK_CLEAR_GRID(cmd, code)    ERR_CHECK_CLEAR(cmd, code, gird)
#define ERR_CHECK_CLEAR_PV(cmd, code)      ERR_CHECK_CLEAR(cmd, code, pv)
#define ERR_CHECK_CLEAR_BUS(cmd, code)     ERR_CHECK_CLEAR(cmd, code, bus)
#define ERR_CHECK_CLEAR_INV(cmd, code)     ERR_CHECK_CLEAR(cmd, code, inv)
#define ERR_CHECK_CLEAR_LOAD(cmd, code)    ERR_CHECK_CLEAR(cmd, code, load)
#define ERR_CHECK_CLEAR_TEMP(cmd, code)    ERR_CHECK_CLEAR(cmd, code, temp)
#define ERR_CHECK_CLEAR_GEN(cmd, code)     ERR_CHECK_CLEAR(cmd, code, gen)
#define ERR_CHECK_CLEAR_OTHER(cmd, code)   ERR_CHECK_CLEAR(cmd, code, other)

#define ERRDLY_CHECK_GRID(cmd, threshold, code)    ERRDLY_CHECK(cmd, threshold, code, gird)
#define ERRDLY_CHECK_PV(cmd, threshold, code)      ERRDLY_CHECK(cmd, threshold, code, pv)
#define ERRDLY_CHECK_BUS(cmd, threshold, code)     ERRDLY_CHECK(cmd, threshold, code, bus)
#define ERRDLY_CHECK_INV(cmd, threshold, code)     ERRDLY_CHECK(cmd, threshold, code, inv)
#define ERRDLY_CHECK_LOAD(cmd, threshold, code)    ERRDLY_CHECK(cmd, threshold, code, load)
#define ERRDLY_CHECK_TEMP(cmd, threshold, code)    ERRDLY_CHECK(cmd, threshold, code, temp)
#define ERRDLY_CHECK_GEN(cmd, threshold, code)     ERRDLY_CHECK(cmd, threshold, code, gen)
#define ERRDLY_CHECK_OTHER(cmd, threshold, code)   ERRDLY_CHECK(cmd, threshold, code, other)

#define WARNING_CHECK_GRID(cmd, code)        WARNING_CHECK(cmd, code, grid)
#define WARNING_CHECK_PV(cmd, code)          WARNING_CHECK(cmd, code, pv)
#define WARNING_CHECK_BUS(cmd, code)         WARNING_CHECK(cmd, code, bus)
#define WARNING_CHECK_INV(cmd, code)         WARNING_CHECK(cmd, code, inv)
#define WARNING_CHECK_LOAD(cmd, code)        WARNING_CHECK(cmd, code, load)
#define WARNING_CHECK_TEMP(cmd, code)        WARNING_CHECK(cmd, code, temp)
#define WARNING_CHECK_GEN(cmd, code)         WARNING_CHECK(cmd, code, gen)
#define WARNING_CHECK_OTHER(cmd, code)       WARNING_CHECK(cmd, code, other)

#define WARNING_CHECK_CLEAR_GRID(cmd, code)      WARNING_CHECK_CLEAR(cmd, code, grid)
#define WARNING_CHECK_CLEAR_PV(cmd, code)        WARNING_CHECK_CLEAR(cmd, code, pv)
#define WARNING_CHECK_CLEAR_BUS(cmd, code)       WARNING_CHECK_CLEAR(cmd, code, bus)
#define WARNING_CHECK_CLEAR_INV(cmd, code)       WARNING_CHECK_CLEAR(cmd, code, inv)
#define WARNING_CHECK_CLEAR_LOAD(cmd, code)      WARNING_CHECK_CLEAR(cmd, code, load)
#define WARNING_CHECK_CLEAR_TEMP(cmd, code)      WARNING_CHECK_CLEAR(cmd, code, temp)
#define WARNING_CHECK_CLEAR_GEN(cmd, code)       WARNING_CHECK_CLEAR(cmd, code, gen)
#define WARNING_CHECK_CLEAR_OTHER(cmd, code)     WARNING_CHECK_CLEAR(cmd, code, other)

#define WARNINGDLY_CHECK_GRID(cmd,threshold,code)       WARNINGDLY_CHECK(cmd,threshold,code,grid)
#define WARNINGDLY_CHECK_PV(cmd,threshold,code)         WARNINGDLY_CHECK(cmd,threshold,code,pv)
#define WARNINGDLY_CHECK_BUS(cmd,threshold,code)        WARNINGDLY_CHECK(cmd,threshold,code,bus)
#define WARNINGDLY_CHECK_INV(cmd,threshold,code)        WARNINGDLY_CHECK(cmd,threshold,code,inv)
#define WARNINGDLY_CHECK_LOAD(cmd,threshold,code)       WARNINGDLY_CHECK(cmd,threshold,code,load)
#define WARNINGDLY_CHECK_TEMP(cmd,threshold,code)       WARNINGDLY_CHECK(cmd,threshold,code,temp)
#define WARNINGDLY_CHECK_GEN(cmd,threshold,code)        WARNINGDLY_CHECK(cmd,threshold,code,gen)
#define WARNINGDLY_CHECK_OTHER(cmd,threshold,code)      WARNINGDLY_CHECK(cmd,threshold,code,other)

#define WARNINGDLY_CLEAR_GRID(cmd,threshold, code)       WARNINGDLY_CLEAR(cmd,threshold, code, grid)
#define WARNINGDLY_CLEAR_PV(cmd, threshold, code)        WARNINGDLY_CLEAR(cmd, threshold, code, pv)
#define WARNINGDLY_CLEAR_BUS(cmd, threshold, code)       WARNINGDLY_CLEAR(cmd, threshold, code, bus)
#define WARNINGDLY_CLEAR_INV(cmd, threshold, code)       WARNINGDLY_CLEAR(cmd, threshold, code, inv)
#define WARNINGDLY_CLEAR_LOAD(cmd, threshold, code)      WARNINGDLY_CLEAR(cmd, threshold, code, load)
#define WARNINGDLY_CLEAR_TEMP(cmd, threshold, code)      WARNINGDLY_CLEAR(cmd, threshold, code, temp)
#define WARNINGDLY_CLEAR_GEN(cmd, threshold, code)       WARNINGDLY_CLEAR(cmd, threshold, code, gen)
#define WARNINGDLY_CLEAR_OTHER(cmd, threshold, code)     WARNINGDLY_CLEAR(cmd, threshold, code, other)

#define ERRDLY_CLEAR_GRID(cmd,threshold, code)       ERRDLY_CLEAR(cmd, threshold, code, grid)
#define ERRDLY_CLEAR_PV(cmd, threshold, code)        ERRDLY_CLEAR(cmd, threshold, code, pv)
#define ERRDLY_CLEAR_BUS(cmd, threshold, code)       ERRDLY_CLEAR(cmd, threshold, code, bus)
#define ERRDLY_CLEAR_INV(cmd, threshold, code)       ERRDLY_CLEAR(cmd, threshold, code, inv)
#define ERRDLY_CLEAR_LOAD(cmd, threshold, code)      ERRDLY_CLEAR(cmd, threshold, code, load)
#define ERRDLY_CLEAR_TEMP(cmd, threshold, code)      ERRDLY_CLEAR(cmd, threshold, code, temp)
#define ERRDLY_CLEAR_GEN(cmd, threshold, code)       ERRDLY_CLEAR(cmd, threshold, code, gen)
#define ERRDLY_CLEAR_OTHER(cmd, threshold, code)     ERRDLY_CLEAR(cmd, threshold, code, other)


/**
* @brief: ErrWarningCheckInit()
* @param: void
* @return:Nothing
* @note:故障告警检测初始化
*/
void ErrWarningCheckInit()
{
    gstFaultInfo.inside_err_grid.byte = 0;
    gstFaultInfo.inside_err_pv.byte[0] = 0;
    gstFaultInfo.inside_err_pv.byte[1] = 0;
    gstFaultInfo.inside_err_bus.byte = 0;
    gstFaultInfo.inside_err_inv.byte = 0;
    gstFaultInfo.inside_err_load.byte = 0;
    gstFaultInfo.inside_err_temp.byte = 0;
    gstFaultInfo.inside_err_gen.byte = 0;
    gstFaultInfo.inside_err_other.byte = 0;

    gstFaultInfo.inside_warn_grid.byte = 0;
    gstFaultInfo.inside_warn_pv.byte[0] = 0;
    gstFaultInfo.inside_warn_pv.byte[1] = 0;
    gstFaultInfo.inside_warn_bus.byte = 0;
    gstFaultInfo.inside_warn_inv.byte = 0;
    gstFaultInfo.inside_warn_load.byte = 0;
    gstFaultInfo.inside_warn_temp.byte = 0;
    gstFaultInfo.inside_warn_gen.byte = 0;
    gstFaultInfo.inside_warn_other.byte = 0;

    gstFaultInfo.warning_flag = 0;
    gstFaultInfo.err_flag = 0;
}


/**
* @brief: CtDisConnectCheck()
* @param: bool
* @return:1:断线 0：正常
* @note: CT断线检测
*/
bool CtDisConnectCheck()
{
    static uint16_t ct_detect_cnt = 0;
    if(gstAlgoInterface.ct_cur_rms < 0.8)//等阈值确认
    {
        ct_detect_cnt ++;
    }
    else
    {
        ct_detect_cnt = 0;
    }
    if(ct_detect_cnt > 500)
    {
        ct_detect_cnt = 500;
        return true;
    }
    else
    {
        return false;
    }

}

/**
* @brief: CT_Var_DisConnectCheck
* @param: void
* @return:Nothing
* @note:由CT方差噪声检测CT传感器断线
*/

bool CT_Var_DisConnectCheck(sliding_window_t* CTcurrADC , float32 CTcurr_result)
{
    static uint16_t is_init;
    static uint16_t Ct_Vardetect_cnt = 0;
    if (!is_init)
    {
        sliding_window_init(CTcurrADC , CTcurr_data_buffer, CT_ADCSIZE);
        is_init = 1;
    }
    //滑动窗口计算ct电流的方差
    sliding_window_add_data(CTcurrADC , CTcurr_result);

    //对
    if (CTcurrADC->variance < CT_currvar_limit)
    {
        Ct_Vardetect_cnt ++;  
        if (Ct_Vardetect_cnt > 200)
        {
            Ct_Vardetect_cnt = 200;
        }       
    }
    else
    {
        if (Ct_Vardetect_cnt >= 1)
        {
            Ct_Vardetect_cnt --;
        }  
        else
        {
            Ct_Vardetect_cnt = 0;
        }      
    }   
    if (Ct_Vardetect_cnt >= 100) 
    {
        return true;
    }
    else
    {
        return false;
    }
}



/**
* @brief: MeterDisConnectCheck()
* @param: bool
* @return:1:断线 0：正常
* @note: 电表断线检测
*/
bool MeterDisConnectCheck()
{
    return false;//todo:等电表断线逻辑确认
}

/**
* @brief: ThreadBreakDetect()
* @param: void
* @return:Nothing
* @note:断线检测
*/
void ThreadBreakDetect()
{
    uint16_t threadbreak_flag = 0;
    if(GetIgnorCtOrMeter() == 0)//不清除警告
    {
        if(MeterModeGet() == 0)//使能CT
        {
            threadbreak_flag = CtDisConnectCheck();
        }
        if(MeterModeGet() == 1)//使能电表
        {
            threadbreak_flag = MeterDisConnectCheck();
        }
        if(threadbreak_flag == 1)
        {
            gstFaultInfo.inside_warn_other.bit.r_ct_disconnect = 1;
        }
    }
    else
    {
        gstFaultInfo.inside_warn_other.bit.r_ct_disconnect = 0;
    }
}


void Fault20msHandle()
{
    ThreadBreakDetect();
}

/**
* @brief: LeakCurrCheck()
* @param: void
* @return:Nothing
* @note:漏电流检测
*/

void LeakCurrCheck (void)
{
    /*漏电流检测认证临时使用，暂不确认故障定义码发送err还是warn，待确认*/
    /*该故障在DSP内部应为err，但前期设计为warn，发送警告信息*/
//    static uint16_t Leak_trigger_count = 0;
//    static uint16_t Leak_trigger_count2 = 0;
//    if (fabs(gstAdcResult.leak_curr - 2050) > WARNING_THRESHOLD_LEAK_CURR_OVER )
//    {
//        Leak_trigger_count++;
//        if (Leak_trigger_count > 160)
//        {
//            gstFaultInfo.err_flag |=1;
//            _SetErrCode(EN_ERR_LEAKCURR_HIGH);
//            Leak_trigger_count2 = 0;
//        }
//    }
}

/**
 * @brief
 * @param： ArcInsFaultDetect
 * @param： 拉弧绝缘检测函数
 * @return
 */

void ArcInsFaultDetect(void)
{
    static uint16_t trigger_count = 0;
    if(GPIO_ReadPin(GPIO_ARC) == 0)//拉弧绝缘触发故障将该引脚拉低
    {
        trigger_count ++;
        if(trigger_count > 10)
        {
            gstFaultInfo.err_flag |=1;
            _SetErrCode(EN_OTHER_ERR_ARC_FAULT , &gstFaultInfo.inside_err_other.byte);
        }
    }
    else
    {
        trigger_count = 0;
    }

}

void ErrWarningHandler_100ms()
{
    char rst = 0;
    ERR_CHECK_OTHER({ rst = GetSysTimeOutFlag();},EN_OTHER_ERR_START_TIMEOUT);         //软起超时
    ERR_CHECK_OTHER({ rst = (GetSysTimeOutFlag() && (GetRelaySelfTestResult() != 1));},EN_OTHER_ERR_DALAY_FAULT);//继电器故障
    // ERR_CHECK_OTHER({ rst = (GetSysTimeOutFlag() && (GetRelaySelfTestResult() != 1)); }, EN_OTHER_ERR_TEMP_SENSOR_FAULT);//逆变过温计数检测,时延5s

    ERRDLY_CHECK_TEMP({rst = ((gstNtcTemperature.rediator_temperature > ERROR_THRESHOLD_RADIATOR_TEMPERATURE_OVER));}, 600 , EN_TEMP_ERR_RADIATOR_OVER_TEMP); 
    ERRDLY_CHECK_TEMP({rst = ((gstAlgoInterface.inv_start_ok == 1)&&
                                (gstNtcTemperature.chassis_temperature > ERROR_THRESHOLD_CLASSIS_TEMPERATURE_OVER)&&
                                (TimGetSystime() < 300000));}, 100 , EN_TEMP_ERR_CHASSIS_OVER_TEMP);//开机五分钟内有10s以上过温则判定散热器过温故障
    ERRDLY_CHECK_TEMP({rst = ((gstNtcTemperature.chassis_temperature < ERROR_THRESHOLD_CLASSIS_TEMPERATURE_LOW)&&
                                (TimGetSystime() < 300000));}, 100 , EN_TEMP_ERR_CHASSIS_LOW_TEMP);
    ERRDLY_CHECK_TEMP({rst = (gstNtcTemperature.pv_temperature > ERROR_THRESHOLD_PV_TEMPERATURE_OVER)||
                                (gstNtcTemperature.Inv_temperature > ERROR_THRESHOLD_INV_TEMPERATURE_OVER);
                            }, 600 , EN_TEMP_ERR_PV_OVER_TEMP);//超过1min判定管子过热

    if (gstFaultInfo.inside_err_temp.byte != 0 )
    {
        ERRDLY_CLEAR_TEMP({rst = (gstNtcTemperature.rediator_temperature < ERROR_THRESHOLD_RADIATOR_TEMPERATURE_OVER_RECOVER);}, 600 , EN_TEMP_ERR_RADIATOR_OVER_TEMP);
        ERRDLY_CLEAR_TEMP({rst = (gstNtcTemperature.chassis_temperature < ERROR_THRESHOLD_CLASSIS_TEMPERATURE_OVER_RECOVER);}, 100 , EN_TEMP_ERR_CHASSIS_OVER_TEMP);
        ERRDLY_CLEAR_TEMP({rst = (gstNtcTemperature.chassis_temperature > ERROR_THRESHOLD_CLASSIS_TEMPERATURE_LOW_RECOVER);}, 100 , EN_TEMP_ERR_CHASSIS_LOW_TEMP);
        ERRDLY_CLEAR_TEMP({rst = (gstNtcTemperature.pv_temperature < ERROR_THRESHOLD_PV_TEMPERATURE_OVER_RECOVER) &&
                            (gstNtcTemperature.Inv_temperature < ERROR_THRESHOLD_INV_TEMPERATURE_OVER_RECOVER);}, 600 , EN_TEMP_ERR_PV_OVER_TEMP);
    }

        WARNINGDLY_CHECK_TEMP({ rst = (gstAlgoInterface.inv_start_ok == 1)&&(gstNtcTemperature.chassis_temperature > 80);}, 200, EN_TEMP_WARNING_RADIATOR_OVER_TEMP); //散热器过温告警判断
    if (gstFaultInfo.inside_warn_temp.byte )
    {
        WARNINGDLY_CLEAR_TEMP({ rst = (gstNtcTemperature.chassis_temperature < 78);}, 200, EN_TEMP_WARNING_RADIATOR_OVER_TEMP); //散热器过温告警清除判断
    } 
    
    
    if (gstModBusPara.arc_para.arc_only_read_para.rescheckoverflag == 1) //绝缘阻抗检测
    {
        ERR_CHECK_PV({ rst = ((gstModBusPara.arc_para.arc_only_read_para.pv1_channel_Rn < ERROR_THRESHOLD_INS_RES)||
                                (gstModBusPara.arc_para.arc_only_read_para.pv1_channel_Rp < ERROR_THRESHOLD_INS_RES) );}, EN_PV_ERR_PV1_INS_FAULT);
        ERR_CHECK_PV({ rst = ((gstModBusPara.arc_para.arc_only_read_para.pv2_channel_Rn < ERROR_THRESHOLD_INS_RES)||
                                (gstModBusPara.arc_para.arc_only_read_para.pv2_channel_Rp < ERROR_THRESHOLD_INS_RES) );}, EN_PV_ERR_PV2_INS_FAULT);
        ERR_CHECK_PV({ rst = ((gstModBusPara.arc_para.arc_only_read_para.pv3_channel_Rn < ERROR_THRESHOLD_INS_RES)||
                                (gstModBusPara.arc_para.arc_only_read_para.pv3_channel_Rp < ERROR_THRESHOLD_INS_RES) );}, EN_PV_ERR_PV3_INS_FAULT);
        ERR_CHECK_PV({ rst = ((gstModBusPara.arc_para.arc_only_read_para.pv4_channel_Rn < ERROR_THRESHOLD_INS_RES)||
                                (gstModBusPara.arc_para.arc_only_read_para.pv4_channel_Rp < ERROR_THRESHOLD_INS_RES) );}, EN_PV_ERR_PV4_INS_FAULT);


    }
    else if ((gstFaultInfo.inside_err_pv.bit.pv1_ins_fault == 1) ||
                (gstFaultInfo.inside_err_pv.bit.pv2_ins_fault == 1) ||
                (gstFaultInfo.inside_err_pv.bit.pv3_ins_fault == 1) ||
                (gstFaultInfo.inside_err_pv.bit.pv4_ins_fault == 1))
    {
        /* 仅在 rescheckoverflag=0 且已有绝缘故障时执行恢复 */
        ERR_CHECK_CLEAR_PV({rst = (gstModBusPara.arc_para.arc_only_read_para.pv1_channel_Rn > ERROR_THRESHOLD_INS_RES)&&
                                    (gstModBusPara.arc_para.arc_only_read_para.pv1_channel_Rp > ERROR_THRESHOLD_INS_RES);}, EN_PV_ERR_PV1_INS_FAULT);
        ERR_CHECK_CLEAR_PV({rst = (gstModBusPara.arc_para.arc_only_read_para.pv2_channel_Rn > ERROR_THRESHOLD_INS_RES)&&
                                    (gstModBusPara.arc_para.arc_only_read_para.pv2_channel_Rp > ERROR_THRESHOLD_INS_RES);}, EN_PV_ERR_PV2_INS_FAULT);
        ERR_CHECK_CLEAR_PV({rst = (gstModBusPara.arc_para.arc_only_read_para.pv3_channel_Rn > ERROR_THRESHOLD_INS_RES)&&
                                    (gstModBusPara.arc_para.arc_only_read_para.pv3_channel_Rp > ERROR_THRESHOLD_INS_RES);}, EN_PV_ERR_PV3_INS_FAULT);
        ERR_CHECK_CLEAR_PV({rst = (gstModBusPara.arc_para.arc_only_read_para.pv4_channel_Rn > ERROR_THRESHOLD_INS_RES)&&
                                    (gstModBusPara.arc_para.arc_only_read_para.pv4_channel_Rp > ERROR_THRESHOLD_INS_RES);}, EN_PV_ERR_PV4_INS_FAULT);
    }

    ERR_CHECK_PV({rst = ((gstModBusPara.arc_para.arc_only_read_para.arc_occur & 0x01) != 0);},EN_PV_ERR_PV1_ARC_FAULT);
    ERR_CHECK_PV({rst = ((gstModBusPara.arc_para.arc_only_read_para.arc_occur & 0x02) != 0);},EN_PV_ERR_PV2_ARC_FAULT);
    ERR_CHECK_PV({rst = ((gstModBusPara.arc_para.arc_only_read_para.arc_occur & 0x04) != 0);},EN_PV_ERR_PV3_ARC_FAULT);
    ERR_CHECK_PV({rst = ((gstModBusPara.arc_para.arc_only_read_para.arc_occur & 0x08) != 0);},EN_PV_ERR_PV4_ARC_FAULT);

    gstFaultInfo.inside_err_pv.bit.pv1_short = gPvData.channel_cntl[0].channel_protect.err.bit.pv_short_cut_err;
    gstFaultInfo.inside_err_pv.bit.pv2_short = gPvData.channel_cntl[1].channel_protect.err.bit.pv_short_cut_err;
    gstFaultInfo.inside_err_pv.bit.pv3_short = gPvData.channel_cntl[2].channel_protect.err.bit.pv_short_cut_err;
    gstFaultInfo.inside_err_pv.bit.pv4_short = gPvData.channel_cntl[3].channel_protect.err.bit.pv_short_cut_err;
    gstFaultInfo.inside_err_pv.bit.pv1_reverse = gPvData.channel_cntl[0].channel_protect.err.bit.pv_reverse_err;
    gstFaultInfo.inside_err_pv.bit.pv2_reverse = gPvData.channel_cntl[1].channel_protect.err.bit.pv_reverse_err;
    gstFaultInfo.inside_err_pv.bit.pv3_reverse = gPvData.channel_cntl[2].channel_protect.err.bit.pv_reverse_err;
    gstFaultInfo.inside_err_pv.bit.pv4_reverse = gPvData.channel_cntl[3].channel_protect.err.bit.pv_reverse_err;
    gstFaultInfo.inside_err_pv.bit.pv1_over_curr = gPvData.channel_cntl[0].channel_protect.err.bit.pv_over_current;
    gstFaultInfo.inside_err_pv.bit.pv2_over_curr = gPvData.channel_cntl[1].channel_protect.err.bit.pv_over_current;
    gstFaultInfo.inside_err_pv.bit.pv3_over_curr = gPvData.channel_cntl[2].channel_protect.err.bit.pv_over_current;
    gstFaultInfo.inside_err_pv.bit.pv4_over_curr = gPvData.channel_cntl[3].channel_protect.err.bit.pv_over_current;


}

//todo:调试用，后续删除
//float bus_volt;
//float inv_volt;
//float inv_volt_rms;
//float inv_curr;
//float inv_pwr;
//float load_pwr;
//float grid_volt_rms;
//float grid_pwr;
//float grid_pwr_rms;
//float grid_freq;
//float debug_flag;
void ErrWarningHandler()
{
    char rst = 0;
    static uint16_t delay_cnt = 0;
    delay_cnt++;
//    todo:调试用
//    gstAlgoInterface.bus_vol = bus_volt;
//    gstAlgoInterface.inv_vol = inv_volt;
//    gstAlgoInterface.inv_vol_rms = inv_volt_rms;
//    gstAlgoInterface.inv_cur = inv_curr;
//    gstAlgoInterface.inv_apperant_power = inv_pwr;
//
//    gstAlgoInterface.load_apperant_power = load_pwr;
//    gstAlgoInterface.grid_vol_rms = grid_volt_rms;
//    gstAlgoInterface.grid_apperant_power = grid_pwr;
//    gstAlgoInterface.grid_vol_rms = grid_volt_rms;
//    gstAlgoInterface.grid_freq = grid_freq;

//    LeakCurrCheck();
    Pv_Total_Pwr_Ave = Pv_Get_Total_Pwr_Ave();
    CT_DisConnectCheck = CT_Var_DisConnectCheck(&CTcurr_detect , gstAdcResult.ct_curr);
    LeakCurrProcess(gstAdcResult.leak_curr,20000,50);    /*漏电流故障检测*/
    ERR_CHECK_OTHER({ rst = (EPwm1Regs.TZFLG.bit.OST == 1);}, EN_OTHER_ERR_TZ_FAULT);//cmpss过流


    //ArcInsFaultDetect(); //todo：拉弧绝缘检测功能还需要完善
    //ERR_CHECK_INV({ rst = (gstAlgoInterface.inv_vol > ERROR_THRESHOLD_INV_VOLT_OVER);},EN_INV_ERR_VOL_OVER);//逆变过压故障判断 //todo:根据实际情况判断后续是否加入并离网状态判断
    ERR_CHECK_INV({ rst = (fabsf(gstAlgoInterface.inv_cur) > ERROR_THRESHOLD_INV_CURR_OVER); }, EN_INV_ERR_OVER_CURR);//逆变过流故障判断

    // ERRDLY_CHECK_LOAD({rst =  (gstAlgoInterface.inv_state != INV_STATE_STANDBY) &&
    //     ((gstAlgoInterface.grid_apperant_power > ERROR_THRESHOLD_GRID_PWR_OVER) ||
    //     (gstAlgoInterface.inv_apperant_power > ERROR_THRESHOLD_INV_PWR_OVER)||
    //     (gstAlgoInterface.load_apperant_power > ERROR_THRESHOLD_LOAD_OVER_L2));
    //     },40000,EN_LOAD_ERR_OVER_LOAD2);//过载2，时延2s

    
//    gstFaultInfo.inside_err_pv.byte[0] |= (uint16_t)(gstPVfault.PV_ErrorCode.byte & 0xffff);
//    gstFaultInfo.inside_err_pv.byte[1] |= (uint16_t)((gstPVfault.PV_ErrorCode.byte >> 16) & 0xffff);
//    gstFaultInfo.inside_warn_pv.byte[0] |= (uint16_t)(gstPVfault.PV_WarningCode.byte & 0xffff);
//    gstFaultInfo.inside_warn_pv.byte[1] |= (uint16_t)((gstPVfault.PV_WarningCode.byte >> 16) & 0xffff);
    //todo:逆变输出短路

        
        if(++count >= SCALER)
        {
            count = EN_TIME_DIV_1;
        }
        switch(count)
        {
            case EN_TIME_DIV_1:
            {
                
                ERRDLY_CHECK_INV({ rst = (fabsf(gstAlgoInterface.inv_vol_rms) > ERRORDLY_THRESHOLD_INV_VOLT_OVER);}, 2000, EN_INV_ERR_VOL_OVER);//逆变过压计数检测，是否加入并离网状态判断,时延5ms
                ERRDLY_CHECK_INV({ rst = (gstAlgoInterface.inv_start_ok == 1) && (fabsf(gstAlgoInterface.inv_vol_rms) < ERRORDLY_THRESHOLD_INV_VOLT_LOW); }, 20000, EN_INV_ERR_VOL_LOW);//逆变欠压有效值计数检测+逆变使能判断
                ERRDLY_CHECK_INV({ rst = ((//(gstAlgoInterface.inv_vol_rms < ERRORDLY_THRESHOLD_INV_VOLT_LOW)||
                                            (gstAlgoInterface.inv_vol_rms > ERRORDLY_THRESHOLD_INV_VOLT_OVER))
                                          &&(GSTATE_RELAY_TOTAL == 1)); },2000, EN_INV_ERR_OUT_FAULT);//逆变过流计数检测
                ERRDLY_CHECK_LOAD({ rst = ((gstAlgoInterface.load_apperant_power > 100)&&
                    (gstAlgoInterface.load_apperant_power > (gstDeployInfo.set_batt_dsg_power_input*1.1 + Pv_Total_Pwr_Ave))
                    &&(gstAlgoInterface.inv_state == INV_STATE_DISCONNECTED))||
                    (gstAlgoInterface.load_apperant_power > ERROR_THRESHOLD_LOAD_OVER_L1)||
                    (gstAlgoInterface.grid_apperant_power > ERRORDLY_THRESHOLD_GRID_PWR_OVER) ||
                    (gstAlgoInterface.inv_apperant_power > ERRORDLY_THRESHOLD_INV_PWR_OVER);
                    },96000,EN_LOAD_ERR_OVER_LOAD);//过载，时延24s
                
                ERR_CHECK_OTHER({ rst = GetLeakCurrFaultFlag() == 1; }, EN_OTHER_ERR_LEAKCURR_HIGH);    //漏电流故障

                            
                break;
            }
            case EN_TIME_DIV_2:
            {
                // WARNINGDLY_CHECK_INV({ rst = (((gstAlgoInterface.inv_vol_rms < ERRORDLY_THRESHOLD_INV_VOLT_LOW)||
                //                               (gstAlgoInterface.inv_vol_rms > ERRORDLY_THRESHOLD_INV_VOLT_OVER))
                //                               &&(GSTATE_RELAY_TOTAL == 1)
                //                              );},20,EN_INV_WARNING_OUT_FAULT);  //逆变输出异常检测
                // WARNINGDLY_CHECK_INV({ rst = (((gstAlgoInterface.inv_vol_rms > ERRORDLY_THRESHOLD_INV_VOLT_OVER)||
                //                             (gstAlgoInterface.inv_vol > ERROR_THRESHOLD_INV_VOLT_OVER
                //                             )));},20,EN_INV_WARNING_VOLT_OVER); // 逆变过压
                // WARNINGDLY_CHECK_INV({ rst = ((gstAlgoInterface.inv_vol_rms < ERRORDLY_THRESHOLD_INV_VOLT_LOW));},20,EN_INV_WARNING_VOLT_LOW); // 逆变欠压

                WARNINGDLY_CHECK_LOAD({ rst = ((gstAlgoInterface.load_apperant_power > 100)&&
                                              (gstAlgoInterface.load_apperant_power > (gstDeployInfo.set_batt_dsg_power_input+100 + Pv_Total_Pwr_Ave))&&
                                              (gstAlgoInterface.inv_state == INV_STATE_DISCONNECTED))||
                                              (gstAlgoInterface.load_apperant_power > WARNING_THRESHOLD_LOAD_OVER)
                                              ;},20,EN_LOAD_WARNING_OVER_LOAD); //过载告警检测

                if(gstDeployInfo.work_mode == EN_OIL_MODE)
                {
                    WARNINGDLY_CHECK_GEN({ rst = ((gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTED)||
                                               (gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTING))&&
                                              (gstAlgoInterface.generator_vol_rms < WARNING_THRESHOLD_OIL_VOLT_LOW)
                                               ; }, 20, EN_GEN_WARNING_VOL_LOW);     //油机电压低告警判断
                    WARNINGDLY_CHECK_GEN({ rst =((gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTED)||
                                              (gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTING))&&  
                                             ((gstAlgoInterface.generator_vol_rms > WARNING_THRESHOLD_OIL_VOLT_OVER)||
                                               (fabs(gstAlgoInterface.generator_vol_ave) > 10.0f))
                                               ; }, 20, EN_GEN_WARNING_VOL_HIGH);   //油机电压高告警判断
                }
                else
                {
                    WARNING_CHECK_GRID({ rst =  ((gstAlgoInterface.inv_state != INV_STATE_STANDBY)
                                                 && (gstAlgoInterface.grid_vol_rms < WARNING_THRESHOLD_GRID_VOLT_LOW)); 
                                                }, EN_GRID_WARNING_VOL_LOW);     //电网电压低告警判断
                    WARNING_CHECK_GRID({ rst =  ((gstAlgoInterface.inv_state != INV_STATE_STANDBY) 
                                                 && ( gstAlgoInterface.grid_vol_rms > WARNING_THRESHOLD_GRID_VOLT_OVER)||
                                                 (fabs(gstAlgoInterface.grid_vol_ave) > 10.0f))
                                                ; }, EN_GRID_WARNING_VOL_HIGH);   //电网电压高告警判断
                }
                // WARNING_CHECK_CLEAR_INV({ rst = (gstAlgoInterface.inv_vol_rms >= ERRORDLY_THRESHOLD_INV_VOLT_LOW_RECOVER);}, EN_INV_WARNING_VOLT_LOW); //逆变欠压告警清除判断
                // WARNING_CHECK_CLEAR_INV({ rst = (gstAlgoInterface.inv_vol_rms <= ERRORDLY_THRESHOLD_INV_VOLT_OVER_RECOVER);}, EN_INV_WARNING_VOLT_OVER); //逆变过压告警清除判断
                // WARNING_CHECK_CLEAR_INV({ rst = ((gstAlgoInterface.inv_vol_rms >= ERRORDLY_THRESHOLD_INV_VOLT_LOW_RECOVER)&&
                //                                 (gstAlgoInterface.inv_vol_rms <= ERRORDLY_THRESHOLD_INV_VOLT_OVER_RECOVER))
                //                                ;}, EN_INV_WARNING_OUT_FAULT);  //逆变输出异常告警清除判断
                
                WARNINGDLY_CLEAR_LOAD({ rst = (gstAlgoInterface.load_apperant_power < gstDeployInfo.set_batt_dsg_power_input + Pv_Total_Pwr_Ave)||
                                                (((gstAlgoInterface.inv_state == INV_STATE_GRID_CONNECTED)||
                                                (gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTED))&&
                                                (gstAlgoInterface.load_apperant_power < WARNING_THRESHOLD_LOAD_OVER_RECOVER))
                                                ;},200,EN_LOAD_WARNING_OVER_LOAD); //过载告警检测                               
                //todo:认证电网停止电压故障判断未加
                break;
            }
            case EN_TIME_DIV_3:
            {
                if(gstDeployInfo.work_mode  == EN_OIL_MODE)
                {
                    WARNINGDLY_CLEAR_GEN({ rst = (gstAlgoInterface.generator_vol_rms >= WARNING_THRESHOLD_OIL_VOLT_LOW_RECOVER); }, 2000, EN_GEN_WARNING_VOL_LOW);     //电网电压低告警判断
                    WARNINGDLY_CLEAR_GEN({ rst = (gstAlgoInterface.generator_vol_rms < WARNING_THRESHOLD_OIL_VOLT_OVER_RECOVER); }, 2000, EN_GEN_WARNING_VOL_HIGH);   //电网电压高告警判断
                }
                else
                {
                    WARNING_CHECK_CLEAR_GRID({ rst = (gstAlgoInterface.grid_vol_rms >= WARNING_THRESHOLD_GRID_VOLT_LOW_RECOVER); }, EN_GRID_WARNING_VOL_LOW); //电网电压低告警清除判断
                    WARNING_CHECK_CLEAR_GRID({ rst = (gstAlgoInterface.grid_vol_rms < WARNING_THRESHOLD_GRID_VOLT_OVER_RECOVER); }, EN_GRID_WARNING_VOL_HIGH);//电网电压高告警清除判断
                }

                ERRDLY_CHECK_INV({ rst = (fabsf(gstAlgoInterface.inv_cur_rms) > ERRORDLY_THRESHOLD_INV_CURR_OVER);}, 20, EN_INV_ERR_OVER_CURR);//逆变过流计数检测
                ERR_CHECK_INV({ rst = ((fabsf(gstAlgoInterface.inv_cur_rms) > ERROR_THRESHOLD_INV_CURR_SHORT_CIRCUIT)&&
                           (fabsf(gstAlgoInterface.inv_vol_rms) < ERROR_THRESHOLD_INV_VOLT_SHORT_CIRCUIT)                                    
                          ); }, EN_INV_ERR_OUT_SHORT);//逆变短路故障保护

                ERRDLY_CHECK_PV({rst = (gPvCfg.pwr_limit<Pv_Get_Total_Pwr_Ave()-100
                && gPvData.pv_state == EN_PV_STATE_DEF_NORMAL );},4000,EN_PV_ERR_PV1_LOSE_CONTROL);
                break;
            }
            case EN_TIME_DIV_4:
            {
                if(gstDeployInfo.work_mode  == EN_OIL_MODE)
                {
                    WARNINGDLY_CHECK_GEN({ rst = ((gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTED) ||
                                               (gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTING))&& 
                                               (gstAlgoInterface.generator_freq < WARNING_THERSHOLD_OIL_FREQ_LOW); }, 20, EN_GEN_WARNING_FRE_LOW);     //油机频率低
                    WARNINGDLY_CHECK_GEN({ rst = ((gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTED) ||
                                               (gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTING))&&
                                               (gstAlgoInterface.generator_freq > WARNING_THERSHOLD_OIL_FREQ_OVER); }, 20, EN_GEN_WARNING_FRE_HIGH);   //油机频率高
                }
                else
                {
                    WARNING_CHECK_GRID({ rst = ((gstAlgoInterface.inv_state != INV_STATE_STANDBY) 
                                                 && (gstAlgoInterface.grid_freq < WARNING_THRESHOLD_GRID_FREQ_LOW)); }, EN_GRID_WARNING_FRE_LOW);     //电网频率低
                    WARNING_CHECK_GRID({ rst = ((gstAlgoInterface.inv_state != INV_STATE_STANDBY) 
                                                 && (gstAlgoInterface.grid_freq > WARNING_THRESHOLD_GRID_FREQ_OVER)); }, EN_GRID_WARNING_FRE_HIGH);   //电网频率高
                }

                ERRDLY_CHECK_BUS({ rst = (gstAlgoInterface.inv_state != INV_STATE_STANDBY) && (gstAlgoInterface.bus_vol < ERROR_THRESHOLD_BUS_VOLT_LOW);} , 4, EN_BUS_ERR_VOL_LOW);//bus电压低故障判断
                ERRDLY_CHECK_BUS({ rst = (gstAlgoInterface.bus_vol > ERROR_THRESHOLD_BUS_VOLT_OVER);}, 4, EN_BUS_ERR_VOL_OVER);//BUS过压计数检测 50*20 = 100us
                ERRDLY_CHECK_BUS({rst = ((gstAlgoInterface.bus_vol < ERROR_THRESHOLD_BUS_VOLT_SHORT_CIRCUIT)&&
                             (gstAlgoInterface.bus_cur_rms > ERROR_THRESHOLD_BUS_CURR_CHG_SHORT_CIRCUIT))||
                            ((gstAlgoInterface.bus_vol < ERROR_THRESHOLD_BUS_VOLT_SHORT_CIRCUIT)&&
                             (gstAlgoInterface.bus_cur_rms < ERROR_THRESHOLD_BUS_CURR_DSG_SHORT_CIRCUIT))
                            ;},4,EN_BUS_ERR_SHORT_CIRCUIT);    //母线短路判断

                
                break;
            }
            case EN_TIME_DIV_5:
            {
                if(gstDeployInfo.work_mode  == EN_OIL_MODE)
                {
                    WARNINGDLY_CLEAR_GEN({ rst = (gstAlgoInterface.generator_freq >= WARNING_THERSHOLD_OIL_FREQ_LOW_RECOVER); }, 2000, EN_GEN_WARNING_FRE_LOW);
                    WARNINGDLY_CLEAR_GEN({ rst = (gstAlgoInterface.generator_freq <= WARNING_THERSHOLD_OIL_FREQ_OVER_RECOVER); },2000, EN_GEN_WARNING_FRE_HIGH);
                }
                else
                {
                    WARNING_CHECK_CLEAR_GRID({ rst = (gstAlgoInterface.grid_freq >= WARNING_THRESHOLD_GRID_FREQ_LOW_RECOVER); }, EN_GRID_WARNING_FRE_LOW);
                    WARNING_CHECK_CLEAR_GRID({ rst = (gstAlgoInterface.grid_freq <= WARNING_THRESHOLD_GRID_FREQ_OVER_RECOVER); }, EN_GRID_WARNING_FRE_HIGH);
                }
                ERRDLY_CHECK_BUS({rst = (gstAlgoInterface.bus_vol_rms < ERROR_THRESHOLD_BUS_VOLT_SHORT_CIRCUIT);},4000,EN_BUS_ERR_OUT_FAULT);//电池连接异常
                ERRDLY_CHECK_BUS({rst = (gstAlgoInterface.bus_cur_rms < ERROR_THRESHOLD_BUS_CURR_DSG_OVER);},4,EN_BUS_ERR_DSG_OVERCURR); //电池放电过流
                ERRDLY_CHECK_BUS({rst = (gstAlgoInterface.bus_cur_rms > ERROR_THRESHOLD_BUS_CURR_CHG_OVER);},4,EN_BUS_ERR_CHG_OVERCURR); //电池充电过流

                break;
            }
            default:
            {
                count = EN_TIME_DIV_1;
                break;
            }
        }

    if (delay_cnt % 20 == 1)   // 1ms检测函数
    {
        /* code */
    }

    if (delay_cnt % 200 == 2)   // 10ms检测函数
    {

        
        WARNINGDLY_CHECK_OTHER({rst = ((gstAlgoInterface.grid_vol_rms > 150.0f) && (CT_DisConnectCheck == true) && (gstAlgoInterface.inv_state == INV_STATE_GRID_CONNECTED))
                                       ;} , 300, EN_OTHER_WARNING_R_CT_DISCONNECT); //Ct断线检测

        WARNINGDLY_CLEAR_OTHER({rst = !CT_DisConnectCheck;} , 200, EN_OTHER_WARNING_R_CT_DISCONNECT); //Ct断线恢复检测

        FanFaultProcessAll();                                  /*风扇故障判断函数*/
        gstFaultInfo.inside_warn_other.bit.fan_fault = Diagnosis_Warning_Fan_Error();

        ERRDLY_CHECK_OTHER({( rst=(fabs(gstAlgoInterface.load_vol_rms - gstAlgoInterface.grid_vol_rms)>5)&&
                    (GSTATE_RELAY_GRID ==1)&&
                    (GSTATE_RELAY_BACKUP == 1));
                    },1000,EN_OTHER_ERR_BYPASS_FAULT);    //旁路故障

    }

    if (delay_cnt % 2000 == 3)   // 100ms检测函数
    {
        static uint32_t s_cpu2LastHb = 0U;
        static uint8_t s_cpu2StallCnt = 0U;

        ERRDLY_CHECK_OTHER({
            rst = 0;
            if (FirmwareUpgrateIsBusy())
            {
                s_cpu2StallCnt = 0U;
            }
            else if (DualCore_IsCpu2Online())
            {
                uint32_t hb = DevDcr_GetCpu2Heartbeat();
                if (hb == s_cpu2LastHb)
                {
                    if (s_cpu2StallCnt < 255U)
                    {
                        s_cpu2StallCnt++;
                    }
                }
                else
                {
                    s_cpu2LastHb = hb;
                    s_cpu2StallCnt = 0U;
                }
                rst = (s_cpu2StallCnt >= 10U) ? 1 : 0;
            }
        }, 10, EN_OTHER_ERR_CPU2_FAULT);

        if (gstFaultInfo.inside_err_other.bit.cpu2_fault != 0U)
        {
            ERRDLY_CLEAR_OTHER({
                if (FirmwareUpgrateIsBusy())
                {
                    rst = 0;
                }
                else
                {
                    rst = (DualCore_IsCpu2Online() && (s_cpu2StallCnt < 10U)) ? 1 : 0;
                }
            }, 10, EN_OTHER_ERR_CPU2_FAULT);
        }
    } 
    
    delay_cnt  = delay_cnt > 2000 ? 0 : delay_cnt;

}


// /**
// * @brief: Diagnosis_Warning_Leak_Curr_High
// * @param: void
// * @note: 漏电流高告警
// */
// void Diagnosis_Warning_Leak_Curr_High(void)
// {
//     gstFaultInfo.inside_warn_other.bit.leak_curr_high = 0;
// }
/**
* @brief: Diagnosis_Warning_Fan_Error
* @param: void
* @note: 风扇故障告警
*/
uint8_t Diagnosis_Warning_Fan_Error(void)
{
    static uint8_t fault_flag = 0;
    /*故障恢复函数,获取所有风扇中是否存在风扇出现故障*/
    fault_flag = FanGetAllFaultState();
    return fault_flag;
}
// /**
// * @brief: Diagnosis_Warning_R_Ct_Reverse
// * @param: void
// * @note: R_CT反接
// */
// void Diagnosis_Warning_R_Ct_Reverse(void)
// {
//     gstFaultInfo.inside_warn.bit.r_ct_reverse = 0;
// }
// /**
// * @brief: Diagnosis_Warning_S_Ct_Reverse
// * @param: void
// * @note: S_CT反接
// */
// void Diagnosis_Warning_S_Ct_Reverse(void)
// {
//     gstFaultInfo.inside_warn.bit.s_ct_reverse = 0;
// }
// /**
// * @brief: Diagnosis_Warning_T_Ct_Reverse
// * @param: void
// * @note: T_CT反接
// */
// void Diagnosis_Warning_T_Ct_Reverse(void)
// {
//     gstFaultInfo.inside_warn.bit.t_ct_reverse = 0;
// }

// // /**
// // * @brief: Diagnosis_Error_Start_Timeout
// // * @param: void
// // * @note: 软启超时故障
// // */
// // void Diagnosis_Error_Start_Timeout(void)
// // {
// //     gstFaultInfo.inside_err.bit.start_timeout = 0;
// // }
// // /**
// // * @brief: Diagnosis_Error_Inv_Out_Short
// // * @param: void
// // * @note: 逆变输出短路
// // */
// // void Diagnosis_Error_Inv_Out_Short(void)
// // {
// //     gstFaultInfo.inside_err.bit.inv_out_short = 0;
// // }
// // /**
// // * @brief: Diagnosis_Error_Dc_Component_High
// // * @param: void
// // * @note: 直流分量高故障
// // */
// // void Diagnosis_Error_Dc_Component_High(void)
// // {
// //     gstFaultInfo.inside_err.bit.dc_component_high = 0;
// // }
// // /**
// // * @brief: Diagnosis_Error_Box_Over_Temp
// // * @param: void
// // * @note: 箱体内环境过温
// // */
// // void Diagnosis_Error_Box_Over_Temp(void)
// // {
// //     gstFaultInfo.inside_err.bit.box_over_temp = 0;
// // }
// // /**
// // * @brief: Diagnosis_Error_Inv_Tz_Fault
// // * @param: void
// // * @note: 逆变TZ故障
// // */
// // //void Diagnosis_Error_Inv_Tz_Fault(void)
// // //{
// // //   gstFaultInfo.inside_err.bit.inv_tz_fault = EPwm1Regs.TZFLG.bit.OST;
// // //}
// // /**
// // * @brief: Diagnosis_Error_Inv_Out_Fault
// // * @param: void
// // * @note: 逆变输出故障
// // */
// // void Diagnosis_Error_Inv_Out_Fault(void)
// // {
// //     gstFaultInfo.inside_err.bit.inv_out_fault = 0;
// // }
// // /**
// // * @brief: Diagnosis_Error_R_Ct_Disconnect
// // * @param: void
// // * @note: R相CT断线
// // */
// // void Diagnosis_Error_R_Ct_Disconnect(void)
// // {
// //     gstFaultInfo.inside_err.bit.r_ct_disconnect = 0;
// // }
// // /**
// // * @brief: Diagnosis_Error_S_Ct_Disconnect
// // * @param: void
// // * @note: S相CT断线
// // */
// // void Diagnosis_Error_S_Ct_Disconnect(void)
// // {
// //     gstFaultInfo.inside_err.bit.s_ct_disconnect = 0;
// // }
// // /**
// // * @brief: Diagnosis_Error_T_Ct_Disconnect
// // * @param: void
// // * @note: T相CT断线
// // */

// // void Diagnosis_Error_T_Ct_Disconnect(void)
// // {
// //     gstFaultInfo.inside_err.bit.t_ct_disconnect = 0;
// // }

// // /**
// // * @brief: Diagnosis_Error_Always_Comm_Error
// // * @param: void
// // * @note: 主从机通信错误
// // */

// // void Diagnosis_Error_Always_Comm_Error(void)
// // {
// //     gstFaultInfo.inside_err.bit.always_comm_error = 0;
// // }


/**
* @brief: GetError_Grid_VoltLow
* @param: void
* @note: 获取电网电压低故障标志位
*/
uint16_t GetError_Grid_VoltLow(void)
{
    return (gstFaultInfo.inside_err_grid.bit.grid_volt_low);
}

/**
* @brief: GetError_Grid_VoltHigh
* @param: void
* @note: 获取电网电压高故障标志位
*/
uint16_t GetError_Grid_VoltHigh(void)
{
    return (gstFaultInfo.inside_err_grid.bit.grid_volt_high);
}

/**
* @brief: GetError_Grid_FreqLow
* @param: void
* @note: 获取电网频率低故障标志位
*/
uint16_t GetError_Grid_FreqLow(void)
{
    return (gstFaultInfo.inside_err_grid.bit.grid_freq_low);
}

/**
* @brief: GetError_Grid_FreqHigh
* @param: void
* @note: 获取电网频率高故障标志位
*/
uint16_t GetError_Grid_FreqHigh(void)
{
    return (gstFaultInfo.inside_err_grid.bit.grid_freq_high);
}

/**
* @brief: GetError_Pv1_Short
* @param: void
* @note: 获取PV1短路故障标志位
*/
uint16_t GetError_Pv1_Short(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv1_short);
}

/**
* @brief: GetError_Pv2_Short
* @param: void
* @note: 获取PV2短路故障标志位
*/
uint16_t GetError_Pv2_Short(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv2_short);
}

/**
* @brief: GetError_Pv3_Short
* @param: void
* @note: 获取PV3短路故障标志位
*/
uint16_t GetError_Pv3_Short(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv3_short);
}

/**
* @brief: GetError_Pv4_Short
* @param: void
* @note: 获取PV4短路故障标志位
*/
uint16_t GetError_Pv4_Short(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv4_short);
}

/**
* @brief: GetError_Pv1_Reverse
* @param: void
* @note: 获取PV1反接故障标志位
*/
uint16_t GetError_Pv1_Reverse(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv1_reverse);
}

/**
* @brief: GetError_Pv2_Reverse
* @param: void
* @note: 获取PV2反接故障标志位
*/
uint16_t GetError_Pv2_Reverse(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv2_reverse);
}

/**
* @brief: GetError_Pv3_Reverse
* @param: void
* @note: 获取PV3反接故障标志位
*/
uint16_t GetError_Pv3_Reverse(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv3_reverse);
}

/**
* @brief: GetError_Pv4_Reverse
* @param: void
* @note: 获取PV4反接故障标志位
*/
uint16_t GetError_Pv4_Reverse(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv4_reverse);
}

/**
* @brief: GetError_Pv1_LoseControl
* @param: void
* @note: 获取PV1失控故障标志位
*/
uint16_t GetError_Pv1_LoseControl(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv1_lose_control);
}

/**
* @brief: GetError_Pv2_LoseControl
* @param: void
* @note: 获取PV2失控故障标志位
*/
uint16_t GetError_Pv2_LoseControl(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv2_lose_control);
}

/**
* @brief: GetError_Pv3_LoseControl
* @param: void
* @note: 获取PV3失控故障标志位
*/
uint16_t GetError_Pv3_LoseControl(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv3_lose_control);
}

/**
* @brief: GetError_Pv4_LoseControl
* @param: void
* @note: 获取PV4失控故障标志位
*/
uint16_t GetError_Pv4_LoseControl(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv4_lose_control);
}

/**
* @brief: GetError_Pv1_OverCurr
* @param: void
* @note: 获取PV1过流故障标志位
*/
uint16_t GetError_Pv1_OverCurr(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv1_over_curr);
}

/**
* @brief: GetError_Pv2_OverCurr
* @param: void
* @note: 获取PV2过流故障标志位
*/
uint16_t GetError_Pv2_OverCurr(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv2_over_curr);
}

/**
* @brief: GetError_Pv3_OverCurr
* @param: void
* @note: 获取PV3过流故障标志位
*/
uint16_t GetError_Pv3_OverCurr(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv3_over_curr);
}

/**
* @brief: GetError_Pv4_OverCurr
* @param: void
* @note: 获取PV4过流故障标志位
*/
uint16_t GetError_Pv4_OverCurr(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv4_over_curr);
}

/**
* @brief: GetError_Pv1_InsFault
* @param: void
* @note: 获取PV1绝缘阻抗小故障标志位
*/
uint16_t GetError_Pv1_InsFault(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv1_ins_fault);
}

/**
* @brief: GetError_Pv2_InsFault
* @param: void
* @note: 获取PV2绝缘阻抗小故障标志位
*/
uint16_t GetError_Pv2_InsFault(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv2_ins_fault);
}

/**
* @brief: GetError_Pv3_InsFault
* @param: void
* @note: 获取PV3绝缘阻抗小故障标志位
*/
uint16_t GetError_Pv3_InsFault(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv3_ins_fault);
}

/**
* @brief: GetError_Pv4_InsFault
* @param: void
* @note: 获取PV4绝缘阻抗小故障标志位
*/
uint16_t GetError_Pv4_InsFault(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv4_ins_fault);
}

/**
* @brief: GetError_Pv1_ArcFault
* @param: void
* @note: 获取PV1拉弧故障标志位
*/
uint16_t GetError_Pv1_ArcFault(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv1_arc_fault);
}

/**
* @brief: GetError_Pv2_ArcFault
* @param: void
* @note: 获取PV2拉弧故障标志位
*/
uint16_t GetError_Pv2_ArcFault(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv2_arc_fault);
}

/**
* @brief: GetError_Pv3_ArcFault
* @param: void
* @note: 获取PV3拉弧故障标志位
*/
uint16_t GetError_Pv3_ArcFault(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv3_arc_fault);
}

/**
* @brief: GetError_Pv4_ArcFault
* @param: void
* @note: 获取PV4拉弧故障标志位
*/
uint16_t GetError_Pv4_ArcFault(void)
{
    return (gstFaultInfo.inside_err_pv.bit.pv4_arc_fault);
}

/**
* @brief: GetError_Bus_VoltLow
* @param: void
* @note: 获取母线电压低故障标志位
*/
uint16_t GetError_Bus_VoltLow(void)
{
    return (gstFaultInfo.inside_err_bus.bit.bus_volt_low);
}

/**
* @brief: GetError_Bus_VoltOver
* @param: void
* @note: 获取母线电压高故障标志位
*/
uint16_t GetError_Bus_VoltOver(void)
{
    return (gstFaultInfo.inside_err_bus.bit.bus_volt_over);
}

/**
* @brief: GetError_Bus_ShortCircut
* @param: void
* @note: 获取母线短路故障标志位
*/
uint16_t GetError_Bus_ShortCircut(void)
{
    return (gstFaultInfo.inside_err_bus.bit.bus_short_circut);
}

/**
* @brief: GetError_Bus_OutFault
* @param: void
* @note: 获取母线连接异常故障标志位
*/
uint16_t GetError_Bus_OutFault(void)
{
    return (gstFaultInfo.inside_err_bus.bit.bus_out_fault);
}

/**
* @brief: GetError_Bus_ChgOvercurr
* @param: void
* @note: 获取电池充电过流故障标志位
*/
uint16_t GetError_Bus_ChgOvercurr(void)
{
    return (gstFaultInfo.inside_err_bus.bit.bus_chg_overcurr);
}

/**
* @brief: GetError_Bus_DsgOvercurr
* @param: void
* @note: 获取电池放电过流故障标志位
*/
uint16_t GetError_Bus_DsgOvercurr(void)
{
    return (gstFaultInfo.inside_err_bus.bit.bus_dsg_overcurr);
}

/**
* @brief: GetError_Inv_OutShort
* @param: void
* @note: 获取逆变输出短路故障标志位
*/
uint16_t GetError_Inv_OutShort(void)
{
    return (gstFaultInfo.inside_err_inv.bit.inv_out_short);
}

/**
* @brief: GetError_Inv_VolLow
* @param: void
* @note: 获取逆变电压低故障标志位
*/
uint16_t GetError_Inv_VolLow(void)
{
    return (gstFaultInfo.inside_err_inv.bit.inv_vol_low);
}

/**
* @brief: GetError_Inv_VolOver
* @param: void
* @note: 获取逆变电压高故障标志位
*/
uint16_t GetError_Inv_VolOver(void)
{
    return (gstFaultInfo.inside_err_inv.bit.inv_vol_over);
}

/**
* @brief: GetError_Inv_OverCurr
* @param: void
* @note: 获取逆变过流故障标志位
*/
uint16_t GetError_Inv_OverCurr(void)
{
    return (gstFaultInfo.inside_err_inv.bit.inv_over_curr);
}

/**
* @brief: GetError_Load_OverLoad1
* @param: void
* @note: 获取逆变输出短路故障标志位
*/
uint16_t GetError_Load_OverLoad(void)
{
    return (gstFaultInfo.inside_err_load.bit.over_load);
}


/**
* @brief: GetError_Temp_ChassisOverTemp
* @param: void
* @note: 获取腔体过温故障标志位，TEMP_A3
*/
uint16_t GetError_Temp_ChassisOverTemp(void)
{
    return (gstFaultInfo.inside_err_temp.bit.chassis_over_temp);
}

/**
* @brief: GetError_Temp_RadiatorOverTemp
* @param: void
* @note: 获取散热器过温故障标志位，TEMP_A2
*/
uint16_t GetError_Temp_RadiatorOverTemp(void)
{
    return (gstFaultInfo.inside_err_temp.bit.radiator_over_temp);
}

/**
* @brief: GetError_Temp_InvOverTemp
* @param: void
* @note: 获取逆变过温故障标志位，TEMP_A1
*/
// uint16_t GetError_Temp_InvOverTemp(void)
// {
//     return (gstFaultInfo.inside_err_temp.bit.inv_over_temp);
// }

/**
* @brief: GetError_Temp_PvOverTemp
* @param: void
* @note: 获取逆变过流故障标志位，TEMP_A0
*/
uint16_t GetError_Temp_PvOverTemp(void)
{
    return (gstFaultInfo.inside_err_temp.bit.pv_over_temp);
}

/**
* @brief: GetError_Other_StartTimeout
* @param: void
* @note: 获取软起超时故障标志位
*/
uint16_t GetError_Other_StartTimeout(void)
{
    return (gstFaultInfo.inside_err_other.bit.start_timeout);
}

/**
* @brief: GetError_Other_InvTzFault
* @param: void
* @note: 获取硬件过流故障标志位
*/
uint16_t GetError_Other_InvTzFault(void)
{
    return (gstFaultInfo.inside_err_other.bit.inv_tz_fault);
}

/**
* @brief: GetError_Other_LeakcurrHigh
* @param: void
* @note: 获取漏电流高故障标志位
*/
uint16_t GetError_Other_LeakcurrHigh(void)
{
    return (gstFaultInfo.inside_err_other.bit.leakcurr_high);
}

/**
* @brief: GetError_Other_ArcFault
* @param: void
* @note: 获取拉弧故障标志位
*/
uint16_t GetError_Other_ArcFault(void)
{
    return (gstFaultInfo.inside_err_other.bit.arc_fault);
}

/**
* @brief: GetError_Other_InsFault
* @param: void
* @note: 获取绝缘阻抗故障标志位
*/
uint16_t GetError_Other_InsFault(void)
{
    return (gstFaultInfo.inside_err_other.bit.ins_fault);
}

/**
* @brief: GetError_Other_DalayFault
* @param: void
* @note: 获取继电器短路故障标志位
*/
uint16_t GetError_Other_DalayFault(void)
{
    return (gstFaultInfo.inside_err_other.bit.dalay_fault);
}

/**
* @brief: GetError_Other_SamplingFault
* @param: void
* @note: 获取采样传感器故障标志位
*/
uint16_t GetError_Other_SamplingFault(void)
{
    return (gstFaultInfo.inside_err_other.bit.sampling_fault);
}

/**
* @brief: GetError_Other_TempSensorFault
* @param: void
* @note: 获取温度传感器故障标志位
*/
uint16_t GetError_Other_TempSensorFault(void)
{
    return (gstFaultInfo.inside_err_other.bit.temp_sensor_fault);
}

/**
* @brief: GetError_Other_CommFault
* @param: void
* @note: 获取主从通讯故障标志位
*/
uint16_t GetError_Other_CommFault(void)
{
    return (gstFaultInfo.inside_err_other.bit.comm_fault);
}

/**
* @brief: GetError_Other_BypassFault
* @param: void
* @note: 获取旁路故障标志位
*/
uint16_t GetError_Other_BypassFault(void)
{
    return (gstFaultInfo.inside_err_other.bit.bypass_fault);
}

/**
* @brief: GetWarning_Grid_VoltLow
* @param: void
* @note: 获取电网电压低警告标志位，小于176V，205V恢复
*/
uint16_t GetWarning_Grid_VoltLow(void)
{
    return (gstFaultInfo.inside_warn_grid.bit.grid_volt_low);
}

/**
* @brief: GetWarning_Grid_VoltHigh
* @param: void
* @note: 获取电网电压高警告标志位，大于265V，253V恢复
*/
uint16_t GetWarning_Grid_VoltHigh(void)
{
    return (gstFaultInfo.inside_warn_grid.bit.grid_volt_high);
}

/**
* @brief: GetWarning_Grid_FreqLow
* @param: void
* @note: 获取电网频率低警告标志位，小于47Hz，47.7恢复
*/
uint16_t GetWarning_Grid_FreqLow(void)
{
    return (gstFaultInfo.inside_warn_grid.bit.grid_freq_low);
}

/**
* @brief: GetWarning_Grid_FreqHigh
* @param: void
* @note: 获取电网频率高警告标志位，大于52Hz，50.15Hz恢复
*/
uint16_t GetWarning_Grid_FreqHigh(void)
{
    return (gstFaultInfo.inside_warn_grid.bit.grid_freq_high);
}

/**
* @brief: GetWarning_Pv1_VolLow
* @param: void
* @note: 获取PV1欠压警告标志位
*/
uint16_t GetWarning_Pv1_VolLow(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv1_vol_low);
}

/**
* @brief: GetWarning_Pv2_VolLow
* @param: void
* @note: 获取PV2欠压警告标志位
*/
uint16_t GetWarning_Pv2_VolLow(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv2_vol_low);
}

/**
* @brief: GetWarning_Pv3_VolLow
* @param: void
* @note: 获取PV3欠压警告标志位
*/
uint16_t GetWarning_Pv3_VolLow(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv3_vol_low);
}

/**
* @brief: GetWarning_Pv4_VolLow
* @param: void
* @note: 获取PV4欠压警告标志位
*/
uint16_t GetWarning_Pv4_VolLow(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv4_vol_low);
}

/**
* @brief: GetWarning_Pv1_VolHigh
* @param: void
* @note: 获取PV1过压警告标志位
*/
uint16_t GetWarning_Pv1_VolHigh(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv1_vol_high);
}

/**
* @brief: GetWarning_Pv2_VolHigh
* @param: void
* @note: 获取PV2过压警告标志位
*/
uint16_t GetWarning_Pv2_VolHigh(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv2_vol_high);
}

/**
* @brief: GetWarning_Pv3_VolHigh
* @param: void
* @note: 获取PV3过压警告标志位
*/
uint16_t GetWarning_Pv3_VolHigh(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv3_vol_high);
}

/**
* @brief: GetWarning_Pv4_VolHigh
* @param: void
* @note: 获取PV4过压警告标志位
*/
uint16_t GetWarning_Pv4_VolHigh(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv4_vol_high);
}

/**
* @brief: GetWarning_Pv1_CurHigh
* @param: void
* @note: 获取PV1过流警告标志位
*/
uint16_t GetWarning_Pv1_CurHigh(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv1_cur_high);
}

/**
* @brief: GetWarning_Pv2_CurHigh
* @param: void
* @note: 获取PV2过流警告标志位
*/
uint16_t GetWarning_Pv2_CurHigh(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv2_cur_high);
}

/**
* @brief: GetWarning_Pv3_CurHigh
* @param: void
* @note: 获取PV3过流警告标志位
*/
uint16_t GetWarning_Pv3_CurHigh(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv3_cur_high);
}

/**
* @brief: GetWarning_Pv4_CurHigh
* @param: void
* @note: 获取PV4过流警告标志位
*/
uint16_t GetWarning_Pv4_CurHigh(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv4_cur_high);
}

/**
* @brief: GetWarning_Pv1_OverPwr
* @param: void
* @note: 获取PV1过功率警告标志位
*/
uint16_t GetWarning_Pv1_OverPwr(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv1_over_pwr);
}

/**
* @brief: GetWarning_Pv2_OverPwr
* @param: void
* @note: 获取PV2过功率警告标志位
*/
uint16_t GetWarning_Pv2_OverPwr(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv2_over_pwr);
}

/**
* @brief: GetWarning_Pv3_OverPwr
* @param: void
* @note: 获取PV3过功率警告标志位
*/
uint16_t GetWarning_Pv3_OverPwr(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv3_over_pwr);
}

/**
* @brief: GetWarning_Pv4_OverPwr
* @param: void
* @note: 获取PV4过功率警告标志位
*/
uint16_t GetWarning_Pv4_OverPwr(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.pv4_over_pwr);
}

/**
* @brief: GetWarning_Pv_TotalOverPwr
* @param: void
* @note: 获取总功率过功率警告标志位
*/
uint16_t GetWarning_Pv_TotalOverPwr(void)
{
    return (gstFaultInfo.inside_warn_pv.bit.total_over_pwr);
}

/**
* @brief: GetWarning_Bus_VoltLow
* @param: void
* @note: 获取母线电压低警告标志位，低于理论值460V
*/
// uint16_t GetWarning_Bus_VoltLow(void)
// {
//     return (gstFaultInfo.inside_warn_bus.bit.bus_volt_low);
// }

/**
* @brief: GetWarning_Bus_VoltHigh
* @param: void
* @note: 获取母线电压高警告标志位，高于理论值510V
*/
// uint16_t GetWarning_Bus_VoltHigh(void)
// {
//     return (gstFaultInfo.inside_warn_bus.bit.bus_volt_high);
// }

/**
* @brief: GetWarning_Inv_OutFault
* @param: void
* @note: 获取逆变输出故障警告标志位
*/
// uint16_t GetWarning_Inv_OutFault(void)
// {
//     return (gstFaultInfo.inside_warn_inv.bit.inv_out_fault);
// }

/**
* @brief: GetWarning_Inv_OutVolOver
* @param: void
* @note: 获取逆变输出电压高警告标志位
*/
// uint16_t GetWarning_Inv_OutVolOver(void)
// {
//     return (gstFaultInfo.inside_warn_inv.bit.inv_out_vol_over);
// }

/**
* @brief: GetWarning_Inv_OutVolLow
* @param: void
* @note: 获取逆变输出电压低警告标志位
*/
// uint16_t GetWarning_Inv_OutVolLow(void)
// {
//     return (gstFaultInfo.inside_warn_inv.bit.inv_out_vol_low);
// }

/**
* @brief: GetWarning_Load_OverLoad
* @param: void
* @note: 获取过载警告标志位
*/
uint16_t GetWarning_Load_OverLoad(void)
{
    return (gstFaultInfo.inside_warn_load.bit.over_load);
}

/**
* @brief: GetWarning_Temp_ChassisOverTemp
* @param: void
* @note: 获取腔体过温警告标志位，TEMP_A3
*/
// uint16_t GetWarning_Temp_ChassisOverTemp(void)
// {
//     return (gstFaultInfo.inside_warn_temp.bit.chassis_over_temp);
// }

/**
* @brief: GetWarning_Temp_RadiatorOverTemp
* @param: void
* @note: 获取散热器过温警告标志位，TEMP_A2
*/
// uint16_t GetWarning_Temp_RadiatorOverTemp(void)
// {
//     return (gstFaultInfo.inside_warn_temp.bit.radiator_over_temp);
// }

/**
* @brief: GetWarning_Temp_InvOverTemp
* @param: void
* @note: 获取逆变过温警告标志位，TEMP_A1
*/
// uint16_t GetWarning_Temp_InvOverTemp(void)
// {
//     return (gstFaultInfo.inside_warn_temp.bit.inv_over_temp);
// }

/**
* @brief: GetWarning_Temp_PvOverTemp
* @param: void
* @note: 获取逆变过流警告标志位，TEMP_A0
*/
// uint16_t GetWarning_Temp_PvOverTemp(void)
// {
//     return (gstFaultInfo.inside_warn_temp.bit.pv_over_temp);
// }

/**
* @brief: GetWarning_Gen_VolLow
* @param: void
* @note: 获取油机电压低警告标志位
*/
uint16_t GetWarning_Gen_VolLow(void)
{
    return (gstFaultInfo.inside_warn_gen.bit.gen_vol_low);
}

/**
* @brief: GetWarning_Gen_VolHigh
* @param: void
* @note: 获取油机电压高警告标志位
*/
uint16_t GetWarning_Gen_VolHigh(void)
{
    return (gstFaultInfo.inside_warn_gen.bit.gen_vol_high);
}

/**
* @brief: GetWarning_Gen_FreqLow
* @param: void
* @note: 获取油机频率低警告标志位
*/
uint16_t GetWarning_Gen_FreqLow(void)
{
    return (gstFaultInfo.inside_warn_gen.bit.gen_freq_low);
}

/**
* @brief: GetWarning_Gen_FreqHigh
* @param: void
* @note: 获取油机频率高警告标志位
*/
uint16_t GetWarning_Gen_FreqHigh(void)
{
    return (gstFaultInfo.inside_warn_gen.bit.gen_freq_high);
}

/**
* @brief: GetWarning_Other_FanFault
* @param: void
* @note: 获取风扇故障警告标志位
*/
uint16_t GetWarning_Other_FanFault(void)
{
    return (gstFaultInfo.inside_warn_other.bit.fan_fault);
}

/**
* @brief: GetWarning_Other_R_Ct_Disconnect
* @param: void
* @note: 获取R_CT未接警告标志位
*/
uint16_t GetWarning_Other_R_Ct_Disconnect(void)
{
    return (gstFaultInfo.inside_warn_other.bit.r_ct_disconnect);
}

/**
* @brief: GetWarning_Other_S_Ct_Disconnect
* @param: void
* @note: 获取S_CT未接警告标志位
*/
// uint16_t GetWarning_Other_S_Ct_Disconnect(void)
// {
//     return (gstFaultInfo.inside_warn_other.bit.s_ct_disconnect);
// }

// /**
// * @brief: GetWarning_Other_T_Ct_Disconnect
// * @param: void
// * @note: 获取T_CT未接警告标志位
// */
// uint16_t GetWarning_Other_T_Ct_Disconnect(void)
// {
//     return (gstFaultInfo.inside_warn_other.bit.t_ct_disconnect);
// }

// /**
// * @brief: GetWarning_Other_R_Ct_Reverse
// * @param: void
// * @note: 获取R_CT反接警告标志位
// */
// uint16_t GetWarning_Other_R_Ct_Reverse(void)
// {
//     return (gstFaultInfo.inside_warn_other.bit.r_ct_reverse);
// }

// /**
// * @brief: GetWarning_Other_S_Ct_Reverse
// * @param: void
// * @note: 获取S_CT反接警告标志位
// */
// uint16_t GetWarning_Other_S_Ct_Reverse(void)
// {
//     return (gstFaultInfo.inside_warn_other.bit.s_ct_reverse);
// }

// /**
// * @brief: GetWarning_Other_T_Ct_Reverse
// * @param: void
// * @note: 获取T_CT反接警告标志位
// */
// uint16_t GetWarning_Other_T_Ct_Reverse(void)
// {
//     return (gstFaultInfo.inside_warn_other.bit.t_ct_reverse);
// }

// /**
// * @brief: GetWarning_Other_FirmwarePackFault
// * @param: void
// * @note: 获取固件包故障警告标志位
// */
// uint16_t GetWarning_Other_FirmwarePackFault(void)
// {
//     return (gstFaultInfo.inside_warn_other.bit.firmware_pack_fault);
// }

// /**
// * @brief: GetWarning_Other_ModleIdFault
// * @param: void
// * @note: 获取模块ID故障警告标志位
// */
// uint16_t GetWarning_Other_ModleIdFault(void)
// {
//     return (gstFaultInfo.inside_warn_other.bit.modle_id_fault);
// }




uint16_t GetGridErrorValue(void)
{
    return (gstFaultInfo.inside_err_grid.byte);
}
uint16_t GetPvErrorHighValue(void)
{
    return (gstFaultInfo.inside_err_pv.byte[1]);
}
uint16_t GetPvErrorLowValue(void)
{
    return (gstFaultInfo.inside_err_pv.byte[0]);
}
uint16_t GetBusErrorValue(void)
{
    return (gstFaultInfo.inside_err_bus.byte);
}
uint16_t GetInvErrorValue(void)
{
    return (gstFaultInfo.inside_err_inv.byte);
}
uint16_t GetLoadErrorValue(void)
{
    return (gstFaultInfo.inside_err_load.byte);
}
uint16_t GetTempErrorValue(void)
{
    return (gstFaultInfo.inside_err_temp.byte);
}
uint16_t GetGenErrorValue(void)
{
    return (gstFaultInfo.inside_err_gen.byte);
}
uint16_t GetOtherErrorValue(void)
{
    return (gstFaultInfo.inside_err_other.byte);
}


/**
  * @brief  GetWarningValue
  * @param  None
  * @return 告警寄存器值
  * @note   None
*/
uint16_t GetGridWarningHighValue(void)
{
    return (gstFaultInfo.inside_warn_grid.byte);
}

uint16_t GetPvWarningHighValue(void)
{
    return (gstFaultInfo.inside_warn_pv.byte[1]);
}
uint16_t GetPvWarningLowValue(void)
{
    return (gstFaultInfo.inside_warn_pv.byte[0]);
}
uint16_t GetBusWarningValue(void)
{
    return (gstFaultInfo.inside_warn_bus.byte);
}
uint16_t GetInvWarningValue(void)
{
    return (gstFaultInfo.inside_warn_inv.byte);
}
uint16_t GetLoadWarningValue(void)
{
    return (gstFaultInfo.inside_warn_load.byte);
}
uint16_t GetTempWarningValue(void)
{
    return (gstFaultInfo.inside_warn_temp.byte);
}
uint16_t GetGenWarningValue(void)
{
    return (gstFaultInfo.inside_warn_gen.byte);
}
uint16_t GetOtherWarningValue(void)
{
    return (gstFaultInfo.inside_warn_other.byte);
}


/**
  * @brief  GetWarningValue
  * @param  None
  * @return 告警寄存器值
  * @note   None
*/
uint16_t GetPvNoticeHighValue(void)
{
    return (gstFaultInfo.inside_notice_pv.byte[1]);
}

uint16_t GetPvNoticeLowValue(void)
{
    return (gstFaultInfo.inside_notice_pv.byte[0]);
}

uint16_t GetOtherNoticeValue(void)
{
    return (gstFaultInfo.inside_notice_other.byte);
}


/**
  * @brief  GetErrorFlag
  * @param  None
  * @return 故障标志
  * @note   None
*/
uint16_t GetErrorFlag(void)
{
    return (gstFaultInfo.err_flag);
}

/**
  * @brief  GetWarningFlag
  * @param  None
  * @return 告警标志
  * @note   None
*/
uint16_t GetWarningFlag(void)
{
    return (gstFaultInfo.warning_flag);
}

// /**
// * @brief: GetError_Comm_Error
// * @param: void
// * @note: 获取主从机通讯异常故障标志位
// */
// uint16_t GetError_Comm_Error(void)
// {
//     return (gstFaultInfo.inside_err.bit.always_comm_error);
// }
// /**
// * @brief: Fault_Int1
// * @param: static inline void
// * @return:Nothing
// * @note:快速故障判断
// */
// void Fault_Int1(void)
// {
// //    Diagnosis_Error_Inv_Tz_Fault();
// }
/**
* @brief: Fault_20ms
* @param: static inline void
* @return:Nothing
* @note:所有与慢速故障相关的函数
*/
void Fault_20ms(void)
{
    /*      warn告警故障 bit0-bit31     */
    // Diagnosis_Warning_Leak_Curr_High();
    // Diagnosis_Warning_Fan_Error();
    // Diagnosis_Warning_R_Ct_Reverse();
    // Diagnosis_Warning_S_Ct_Reverse();
    // Diagnosis_Warning_T_Ct_Reverse();
    // /*      Erro严重警告故障 bit0-bit31     */
    // Diagnosis_Error_Start_Timeout();
    // Diagnosis_Error_Inv_Out_Short();
    // Diagnosis_Error_Dc_Component_High();
    // Diagnosis_Error_Box_Over_Temp();
    // Diagnosis_Error_Inv_Out_Fault();
    // Diagnosis_Error_R_Ct_Disconnect();
    // Diagnosis_Error_S_Ct_Disconnect();
    // Diagnosis_Error_T_Ct_Disconnect();
    // Diagnosis_Error_Always_Comm_Error();
}


