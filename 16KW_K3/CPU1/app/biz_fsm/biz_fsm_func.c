/**
 * @file biz_fsm_func.c
 * @brief Periodic state handler functions (skeleton; fill in when integrating HW/comm layer)
 */

#include "biz_fsm_func.h"
#include "biz_fsm_def.h"
#include "biz_fsm_cfg.h"
#include "biz_fsm_flag_trigger.h"
#include "biz_fsm_interface.h"
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_interface.h"
#include "flash_default.h"

static uint8_t _greadyBusNegOk = 0U;
static uint8_t _greadyBusPosOk = 0U;

BIZ_FSM_FLAG_DLY_DECLARE(ready_bus_neg, &_greadyBusNegOk, BIZ_FSM_BUS_VOLT_DLY_COUNT);
BIZ_FSM_FLAG_DLY_DECLARE(ready_bus_pos, &_greadyBusPosOk, BIZ_FSM_BUS_VOLT_DLY_COUNT);


void BizFsm_Ready_BusVoltDlyEntryReset(void)
{
    BIZ_FSM_FLAG_DLY_RESET(ready_bus_neg, &_greadyBusNegOk);
    BIZ_FSM_FLAG_DLY_RESET(ready_bus_pos, &_greadyBusPosOk);
}

void BizStateIdle_Function(void* context)
{
    (void)context;
    /* TODO: HW identification, parameter load, limit-parameter update */
//    DeployInfoInit();
    Inv3Ph_State_Init();
    gBizFsm_Status.idle_done = 1;
    /* TODO: on fail fault, add retry counter and eventually set idle_fail */
    gBizFsm_Status.idle_fail = 0;
}

void BizStateSelfTest_Function(void* context)
{
    (void)context;

    BizFsm_Interface_ExtSelfTestRun();

    if(BizFsm_Interface_ExtGetSelfTestPass() != 0U) {
        gBizFsm_Status.self_test_pass = 1U;
        gBizFsm_Status.self_test_fail = 0U;
    } else if(BizFsm_Interface_ExtGetSelfTestTimeout() != 0U) {
        gBizFsm_Status.self_test_pass = 0U;
        gBizFsm_Status.self_test_fail = 1U;
    }
    /* TODO: on fail fault, add retry counter and eventually set self_test_fail */
}

void BizStateReady_Function(void* context)
{
    char rst = 0;

    (void)context;
    if(BizFsm_Interface_ExtGetPcsLLCEnable() != 0U )
    {
        BIZ_FSM_FLAG_DLY_RUN(ready_bus_neg, { rst = (BizFsm_Interface_ExtGetBusVoltNeg() > BIZ_FSM_BUS_VOLT_MIN_THRESHOLD && BizFsm_Interface_ExtGetBusVoltNeg() < BIZ_FSM_BUS_VOLT_MAX_THRESHOLD); });
        BIZ_FSM_FLAG_DLY_RUN(ready_bus_pos, { rst = (BizFsm_Interface_ExtGetBusVoltPos() > BIZ_FSM_BUS_VOLT_MIN_THRESHOLD && BizFsm_Interface_ExtGetBusVoltPos() < BIZ_FSM_BUS_VOLT_MAX_THRESHOLD); });
        if(_greadyBusNegOk != 0U && _greadyBusPosOk != 0U)
        {
            gBizFsm_Cfg.start_mode = EN_RUN_START_MODE_NORMAL;
        }
    }
}

void BizStateGenStart_Function(void* context)
{
    char rst = 0;

    (void)context;
    /* TODO: generator start, bus voltage detection */
    /* Complete start actions and battery pack bus connection */
    if(BizFsm_Interface_ExtGetPcsLLCEnable() != 0U)
    {
        BIZ_FSM_FLAG_DLY_RUN(ready_bus_neg, { rst = (BizFsm_Interface_ExtGetBusVoltNeg() > BIZ_FSM_BUS_VOLT_MIN_THRESHOLD && BizFsm_Interface_ExtGetBusVoltNeg() < BIZ_FSM_BUS_VOLT_MAX_THRESHOLD); });
        BIZ_FSM_FLAG_DLY_RUN(ready_bus_pos, { rst = (BizFsm_Interface_ExtGetBusVoltPos() > BIZ_FSM_BUS_VOLT_MIN_THRESHOLD && BizFsm_Interface_ExtGetBusVoltPos() < BIZ_FSM_BUS_VOLT_MAX_THRESHOLD); });
        if(_greadyBusNegOk && _greadyBusPosOk)
        {
            gBizFsm_Status.gen_start_done = 1;
            /* Default to normal run mode */
            gBizFsm_Cfg.work_mode = EN_RUN_WORK_MODE_NORMAL;
        }
    }
}

void BizStatePvStart_Function(void* context)
{
    char rst = 0;

    (void)context;
    /* TODO: PV BOOST soft start */
    /* Complete start actions and battery pack bus connection */
    if(BizFsm_Interface_ExtGetPcsLLCEnable() != 0U)
    {
        BIZ_FSM_FLAG_DLY_RUN(ready_bus_neg, { rst = (BizFsm_Interface_ExtGetBusVoltNeg() > BIZ_FSM_BUS_VOLT_MIN_THRESHOLD && BizFsm_Interface_ExtGetBusVoltNeg() < BIZ_FSM_BUS_VOLT_MAX_THRESHOLD); });
        BIZ_FSM_FLAG_DLY_RUN(ready_bus_pos, { rst = (BizFsm_Interface_ExtGetBusVoltPos() > BIZ_FSM_BUS_VOLT_MIN_THRESHOLD && BizFsm_Interface_ExtGetBusVoltPos() < BIZ_FSM_BUS_VOLT_MAX_THRESHOLD); });
        if(_greadyBusNegOk && _greadyBusPosOk)
        {
            gBizFsm_Status.pv_start_done = 1;
            /* Default to normal run mode */
            gBizFsm_Cfg.work_mode = EN_RUN_WORK_MODE_NORMAL;
        }
    }
}

void BizStateNomalStart_Function(void* context)
{
    (void)context;
    gBizFsm_Status.nomal_start_done = 1;
    /* Default to normal run mode */
    gBizFsm_Cfg.work_mode = EN_RUN_WORK_MODE_NORMAL;
}

void BizStateNomalRun_Function(void* context)
{
    (void)context;

//    EnergyScheduling();
}

void BizFsm_NomalRun_OnExit(void* context)
{
    (void)context;
    /* TODO: stop PCS output, disable PV/INV, exit energy dispatch, open load relay (PDF 2.1.7) */
//    DeployInfoInit();
    Inv3Ph_State_Init();
}

void BizFsm_TestMode_OnExit(void* context)
{
    (void)context;
    /* TODO: stop PCS output, disable PV/INV, exit energy dispatch, open load relay (PDF 2.1.8) */
//    DeployInfoInit();
    Inv3Ph_State_Init();
}

void BizStateTestMode_Function(void* context)
{
    (void)context;

//    EnergyScheduling();
}

void BizStateFault_Function(void* context)
{
    (void)context;
    /* TODO: fault detection, fault code logging, stop PCS output (PDF 2.1.9) */
}

void BizStateShutDown_Function(void* context)
{
    (void)context;
    /* TODO: unrecoverable fault shutdown, wait for manual intervention (PDF 2.1.10) */
}

void BizStateBoot_Function(void* context)
{
    (void)context;
    /* TODO: upgrade file verification, CPU reset (PDF 2.1.12) */
}

void BizStateInitParam_Function(void* context)
{
    (void)context;
    if(gBizFsm_Fsm.current_state->running_time_count == 0U) {
        gBizFsm_Status.fault_cleared = 1U;
    }

    if(gBizFsm_Status.init_param_done || gBizFsm_Status.init_param_fail)
    {
        return;
    }

    /* TODO: restore factory defaults, clear run/fault records (PDF 2.1.11) */
    if(Restore_Factory_Settings(&gBizFsm_Status.factory_reset_cnt, false))
    {
        gBizFsm_Status.init_param_done = 1U;
    }
    if(gBizFsm_Status.factory_reset_cnt >= 5U)
    {
        gBizFsm_Status.init_param_fail = 1U;
    }
}

void BizStatePowerOff_Function(void* context)
{
    (void)context;
    if(gBizFsm_Fsm.current_state->running_time_count == 0U) {
        gBizFsm_Status.fault_cleared = 1U;
    }
    if(gBizFsm_Status.power_off_done || gBizFsm_Status.power_off_fail)
    {
        return;
    }
    /* TODO: shutdown tasks, storage, true/false shutdown detection (PDF 2.1 EN_RUN_STATE_POWER_OFF) */
    if(ShutDown_SaveFlash(&gBizFsm_Status.power_off_cnt))
    {
        gBizFsm_Status.power_off_done = 1U;
    }
    if(gBizFsm_Status.power_off_cnt >= 5U)
    {
        gBizFsm_Status.power_off_fail = 1U;
    }
}
