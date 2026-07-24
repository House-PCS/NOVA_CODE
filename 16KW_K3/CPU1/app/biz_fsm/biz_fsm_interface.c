/**
 * @file biz_fsm_interface.c
 * @brief Sole implementation of biz_fsm interaction with external modules
 */

#include "biz_fsm_interface.h"
#include "biz_fsm_state.h"
#include "post.h"
#include "ethernet_analysis.h"
#include "sample_update.h"
#include "BizFw_Fsm.h"
#include "flash_default.h"
#include "debug_analysis.h"
/* -------------------------------------------------------------------------- */
/* Internal: sync published status from FSM runtime                         */
/* -------------------------------------------------------------------------- */

static void BizFsm_Interface_UpdateAllowFlags(void)
{
    EN_RUN_STATE state;

    state = (EN_RUN_STATE)gBizFsm_Fsm.current_state_id;
    gBizFsm_Status.state = state;

    SetCpuDebugData(14,  gBizFsm_Status.state);//30
    SetCpuDebugData(15,  gBizFsm_ExtAllow.allow_pv);//31
    if(state == EN_RUN_STATE_DEF_NOMAL_RUN || state == EN_RUN_STATE_DEF_TEST_MODE)
    {
        /* Normal run: all operations allowed */
        gBizFsm_ExtAllow.allow_offgrid = 1U;
        gBizFsm_ExtAllow.allow_grid_connect = 1U;
        gBizFsm_ExtAllow.allow_pv = 1U;
    }
    else if(state == EN_RUN_STATE_DEF_PV_START)
    {

    }
    else if(state == EN_RUN_STATE_DEF_GEN_START)
    {

    }
    else
    {
        gBizFsm_ExtAllow.allow_offgrid = 0U;
        gBizFsm_ExtAllow.allow_grid_connect = 0U;
        gBizFsm_ExtAllow.allow_pv = 0U;
    }
}

static void BizFsm_Interface_SetPowerOnCmd(uint8_t cmd)
{
    gBizFsm_Cmd.power_on_cmd = cmd;
}

static void BizFsm_Interface_SetPowerOffCmd(uint8_t cmd)
{
    gBizFsm_Cmd.power_off_cmd = cmd;
}

static void BizFsm_Interface_SetFactoryResetCmd(uint8_t cmd)
{
    gBizFsm_Cmd.factory_reset_cmd = cmd;
}

/* -------------------------------------------------------------------------- */
/* Module lifecycle                                                           */
/* -------------------------------------------------------------------------- */

void BizFsm_Interface_Init(void)
{
    BizFsm_State_Init();
    BizFsm_Interface_UpdateAllowFlags();
}

int BizFsm_Interface_Update(void* context)
{
    int ret;
    BizFsm_Interface_SetPowerOnCmd((uint8_t)(InvSwitchStateGet() ? 1U : 0U));
    BizFsm_Interface_SetPowerOffCmd((uint8_t)(MeterPowerOffGet() ? 1U : 0U));
    BizFsm_Interface_SetFactoryResetCmd((uint8_t)(MeterResetFactorydGet() ? 1U : 0U));
    if(BizFsm_Interface_GetRunState() !=EN_RUN_STATE_DEF_BOOT && \
            BizFsm_Interface_GetRunState() !=EN_RUN_STATE_DEF_INIT_PARAM && \
            BizFsm_Interface_GetRunState() !=EN_RUN_STATE_POWER_OFF && \
            gBizFsm_Status.auto_save_err_cnt < 0xFFFFu)
    {
        UdpDspHoldPara_AutoSaveIfNeeded(&gBizFsm_Status.auto_save_err_cnt);
    }
    ret = BizFw_Fsm_Update(&gBizFsm_Fsm, context);
    BizFsm_Interface_UpdateAllowFlags();
    return ret;
}

/* -------------------------------------------------------------------------- */
/* External -> biz_fsm: command writes                                          */
/* -------------------------------------------------------------------------- */

void BizFsm_Interface_SetUpgradeCmd(uint8_t cmd)
{
    gBizFsm_Cmd.upgrade_cmd = cmd;
}

void BizFsm_Interface_SetPowerOnReset(uint8_t cmd)
{
    gBizFsm_Cmd.power_on_reset = cmd;
}

void BizFsm_Interface_SetIsErr(uint8_t err)
{
    gBizFsm_Cmd.is_err = err;
}

void BizFsm_Interface_SetFaultClass(EN_RUN_FAULT_CLASS fault_class)
{
    gBizFsm_Cmd.fault_class = fault_class;
}

void BizFsm_Interface_SetFaultCleared(uint8_t cleared)
{
    gBizFsm_Status.fault_cleared = cleared;
}

/* -------------------------------------------------------------------------- */
/* biz_fsm -> external: status reads                                          */
/* -------------------------------------------------------------------------- */

EN_RUN_STATE BizFsm_Interface_GetRunState(void)
{
    return gBizFsm_Status.state;
}

EN_RUN_FAULT_CLASS BizFsm_Interface_GetFaultClass(void)
{
    return gBizFsm_Cmd.fault_class;
}

uint8_t BizFsm_Interface_GetPowerOnCmd(void)
{
    return gBizFsm_Cmd.power_on_cmd;
}

uint8_t BizFsm_Interface_GetAllowOffgrid(void)
{
    return gBizFsm_ExtAllow.allow_offgrid;
}

uint8_t BizFsm_Interface_GetAllowGridConnect(void)
{
    return gBizFsm_ExtAllow.allow_grid_connect;
}

uint8_t BizFsm_Interface_GetAllowPv(void)
{
    return gBizFsm_ExtAllow.allow_pv;
}

/* -------------------------------------------------------------------------- */
/* Internal use: external data reads / function calls                         */
/* -------------------------------------------------------------------------- */

void BizFsm_Interface_ExtSelfTestRun(void)
{
    SysSelfTest();
}

uint8_t BizFsm_Interface_ExtGetSelfTestPass(void)
{
    return (uint8_t)(GetSysSelfTestFlag() ? 1U : 0U);
}

uint8_t BizFsm_Interface_ExtGetSelfTestTimeout(void)
{
    return (uint8_t)(GetSysTimeOutFlag() ? 1U : 0U);
}

uint8_t BizFsm_Interface_ExtGetPcsLLCEnable(void)
{
//    return (uint8_t)(GetPcsLLCEnable() != 0U ? 1U : 0U);
    return 1;
}

float BizFsm_Interface_ExtGetBusVoltNeg(void)
{
    return gstAdcResult.bus_volt_neg;
}

float BizFsm_Interface_ExtGetBusVoltPos(void)
{
    return gstAdcResult.bus_volt_pos;
}
