/**
 * @file biz_fsm_state.c
 * @brief PCS business FSM state definitions and transitions
 */

#include "biz_fsm_state.h"
#include "biz_fsm_func.h"
#include "biz_fsm_def.h"

static BizFw_FsmState_t _gBizFwStateIdle;
static BizFw_FsmState_t _gBizFwStateSelfTest;
static BizFw_FsmState_t _gBizFwStateReady;
static BizFw_FsmState_t _gBizFwStateGenStart;
static BizFw_FsmState_t _gBizFwStatePvStart;
static BizFw_FsmState_t _gBizFwStateNomalStart;
static BizFw_FsmState_t _gBizFwStateNomalRun;
static BizFw_FsmState_t _gBizFwStateTestMode;
static BizFw_FsmState_t _gBizFwStateFault;
static BizFw_FsmState_t _gBizFwStateShutDown;
static BizFw_FsmState_t _gBizFwStateBoot;
static BizFw_FsmState_t _gBizFwStateInitParam;
static BizFw_FsmState_t _gBizFwStatePowerOff;

/* -------------------------------------------------------------------------- */
/* Flag / fault helpers (clear sticky flags on transition to avoid false jumps) */
/* -------------------------------------------------------------------------- */

static void _ClearAllProgressFlags(void)
{
    gBizFsm_Status.idle_done = 0U;
    gBizFsm_Status.idle_fail = 0U;
    gBizFsm_Status.self_test_pass = 0U;
    gBizFsm_Status.self_test_fail = 0U;
    gBizFsm_Status.gen_start_done = 0U;
    gBizFsm_Status.gen_start_fail = 0U;
    gBizFsm_Status.pv_start_done = 0U;
    gBizFsm_Status.pv_start_fail = 0U;
    gBizFsm_Status.nomal_start_done = 0U;
    gBizFsm_Status.nomal_start_fail = 0U;
    gBizFsm_Status.power_off_done = 0U;
    gBizFsm_Status.power_off_fail = 0U;
    gBizFsm_Status.init_param_done = 0U;
    gBizFsm_Status.init_param_fail = 0U;
    gBizFsm_Status.boot_done = 0U;
    gBizFsm_Status.boot_fail = 0U;
    gBizFsm_Status.factory_reset_cnt = 0U;
    gBizFsm_Status.power_off_cnt = 0U;
}

static void _ClearIdleFlags(void)
{
    gBizFsm_Status.idle_done = 0U;
    gBizFsm_Status.idle_fail = 0U;
}

static void _ClearSelfTestFlags(void)
{
    gBizFsm_Status.self_test_pass = 0U;
    gBizFsm_Status.self_test_fail = 0U;
}

static void _ClearGenStartFlags(void)
{
    gBizFsm_Status.gen_start_done = 0U;
    gBizFsm_Status.gen_start_fail = 0U;
}

static void _ClearPvStartFlags(void)
{
    gBizFsm_Status.pv_start_done = 0U;
    gBizFsm_Status.pv_start_fail = 0U;
}

static void _ClearNomalStartFlags(void)
{
    gBizFsm_Status.nomal_start_done = 0U;
    gBizFsm_Status.nomal_start_fail = 0U;
}

static void _ClearBootFlags(void)
{
    gBizFsm_Status.boot_done = 0U;
    gBizFsm_Status.boot_fail = 0U;
}

static void _ClearInitParamFlags(void)
{
    gBizFsm_Status.init_param_done = 0U;
    gBizFsm_Status.init_param_fail = 0U;
    gBizFsm_Status.factory_reset_cnt = 0U;
}

static void _ClearPowerOffFlags(void)
{
    gBizFsm_Status.power_off_done = 0U;
    gBizFsm_Status.power_off_fail = 0U;
    gBizFsm_Status.power_off_cnt = 0U;
}

/* -------------------------------------------------------------------------- */
/* Global transitions: fault / upgrade / factory reset                          */
/* -------------------------------------------------------------------------- */

static uint16_t _FaultCheck(void* context)
{
    (void)context;
    return gBizFsm_Cmd.is_err != 0U;
}

static uint16_t _BootCheck(void* context)
{
    (void)context;
    return gBizFsm_Cmd.upgrade_cmd != 0U;
}

static uint16_t _InitParamCheck(void* context)
{
    (void)context;
    return gBizFsm_Cmd.factory_reset_cmd != 0U;
}

static uint16_t _PowerOffCheck(void* context)
{
    (void)context;
    return gBizFsm_Cmd.power_off_cmd != 0U;
}

static void _PowerOffAction(void* context)
{
    (void)context;
    _ClearPowerOffFlags();
}

static uint16_t _ToReadyCheck(void* context)
{
    (void)context;
    return gBizFsm_Cmd.power_on_cmd == 0U;
}

#define TRANSITION_FAULT                                                                                               \
    {                                                                                                                  \
        _FaultCheck, NULL, &_gBizFwStateFault                                                                               \
    }
#define TRANSITION_BOOT                                                                                                \
    {                                                                                                                  \
        _BootCheck, NULL, &_gBizFwStateBoot                                                                                 \
    }
#define TRANSITION_INIT_PARAM                                                                                          \
    {                                                                                                                  \
        _InitParamCheck, NULL, &_gBizFwStateInitParam                                                                       \
    }
#define TRANSITION_POWER_OFF                                                                                           \
    {                                                                                                                  \
        _PowerOffCheck, _PowerOffAction, &_gBizFwStatePowerOff                                                              \
    }
#define TRANSITION_READY                                                                                               \
    {                                                                                                                  \
        _ToReadyCheck, NULL, &_gBizFwStateReady                                                                            \
    }

/* -------------------------------------------------------------------------- */
/* IDLE                                                                       */
/* -------------------------------------------------------------------------- */

static uint16_t _IdleToSelfTest_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.idle_done != 0U;
}

static void _IdleOnExit(void* context)
{
    (void)context;
    _ClearIdleFlags();
}

static void _IdleToSelfTest_Action(void* context)
{
    (void)context;
    _ClearSelfTestFlags();
}

static BizFw_FsmState_t _gBizFwStateIdle = {
    .state_id = EN_RUN_STATE_DEF_IDLE,
    .transition_count = 5,
    .function = BizStateIdle_Function,
    .on_exit = _IdleOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_FAULT,
        TRANSITION_BOOT,
        TRANSITION_INIT_PARAM,
        TRANSITION_POWER_OFF,
        {
            .condition = _IdleToSelfTest_Cond,
            .action = _IdleToSelfTest_Action,
            .next_state = &_gBizFwStateSelfTest,
        },
    },
};

/* -------------------------------------------------------------------------- */
/* SELF_TEST                                                                  */
/* -------------------------------------------------------------------------- */

static void _SelfTestOnExit(void* context)
{
    (void)context;
    _ClearSelfTestFlags();
    BizFsm_Ready_BusVoltDlyEntryReset();
}

static uint16_t _SelfTestToReady_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.self_test_pass != 0U;
}

static BizFw_FsmState_t _gBizFwStateSelfTest = {
    .state_id = EN_RUN_STATE_DEF_SELF_TEST,
    .transition_count = 5,
    .function = BizStateSelfTest_Function,
    .on_exit = _SelfTestOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_FAULT,
        TRANSITION_BOOT,
        TRANSITION_INIT_PARAM,
        TRANSITION_POWER_OFF,
        {
            .condition = _SelfTestToReady_Cond,
            .next_state = &_gBizFwStateReady,
        },
    },
};

/* -------------------------------------------------------------------------- */
/* READY                                                                      */
/* -------------------------------------------------------------------------- */

static uint16_t _ReadyPowerOnCommon_Cond(void* context)
{
    (void)context;
    return gBizFsm_Cmd.power_on_cmd != 0U;
}

static uint16_t _ReadyToGenStart_Cond(void* context)
{
    (void)context;
    return _ReadyPowerOnCommon_Cond(context) && gBizFsm_Cfg.start_mode == EN_RUN_START_MODE_GEN;
}

static void _ReadyToGenStart_Action(void* context)
{
    (void)context;
    _ClearGenStartFlags();
}

static uint16_t _ReadyToPvStart_Cond(void* context)
{
    (void)context;
    return _ReadyPowerOnCommon_Cond(context) && gBizFsm_Cfg.start_mode == EN_RUN_START_MODE_PV;
}

static void _ReadyToPvStart_Action(void* context)
{
    (void)context;
    _ClearPvStartFlags();
}

static uint16_t _ReadyToNomalStart_Cond(void* context)
{
    (void)context;
    return _ReadyPowerOnCommon_Cond(context) && gBizFsm_Cfg.start_mode == EN_RUN_START_MODE_NORMAL;
}

static void _ReadyToNomalStart_Action(void* context)
{
    (void)context;
    _ClearNomalStartFlags();
}

static void _ReadyOnExit(void* context)
{
    (void)context;
    BizFsm_Ready_BusVoltDlyEntryReset();
    gBizFsm_Cfg.start_mode = EN_RUN_START_MODE_NONE;
}

static BizFw_FsmState_t _gBizFwStateReady = {
    .state_id = EN_RUN_STATE_DEF_READY,
    .transition_count = 7,
    .function = BizStateReady_Function,
    .on_exit = _ReadyOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_FAULT,
        TRANSITION_BOOT,
        TRANSITION_INIT_PARAM,
        TRANSITION_POWER_OFF,
        {
            .condition = _ReadyToNomalStart_Cond,
            .action = _ReadyToNomalStart_Action,
            .next_state = &_gBizFwStateNomalStart,
        },
        {
            .condition = _ReadyToPvStart_Cond,
            .action = _ReadyToPvStart_Action,
            .next_state = &_gBizFwStatePvStart,
        },
        {
            .condition = _ReadyToGenStart_Cond,
            .action = _ReadyToGenStart_Action,
            .next_state = &_gBizFwStateGenStart,
        },
    },
};

/* -------------------------------------------------------------------------- */
/* GEN_START                                                                  */
/* -------------------------------------------------------------------------- */

static uint16_t _GenStartOkToNomalRun_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.gen_start_done != 0U && gBizFsm_Cfg.work_mode == EN_RUN_WORK_MODE_NORMAL;
}

static uint16_t _GenStartOkToTest_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.gen_start_done != 0U && gBizFsm_Cfg.work_mode == EN_RUN_WORK_MODE_TEST;
}

static uint16_t _GenStartToFault_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.gen_start_fail != 0U;
}

static void _GenStartOnExit(void* context)
{
    (void)context;
    _ClearGenStartFlags();
    BizFsm_Ready_BusVoltDlyEntryReset();
}

static BizFw_FsmState_t _gBizFwStateGenStart = {
    .state_id = EN_RUN_STATE_DEF_GEN_START,
    .transition_count = 8,
    .function = BizStateGenStart_Function,
    .on_exit = _GenStartOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_FAULT,
        TRANSITION_BOOT,
        TRANSITION_INIT_PARAM,
        TRANSITION_POWER_OFF,
        TRANSITION_READY,
        {
            .condition = _GenStartToFault_Cond,
            .next_state = &_gBizFwStateFault,
        },
        {
            .condition = _GenStartOkToTest_Cond,
            .next_state = &_gBizFwStateTestMode,
        },
        {
            .condition = _GenStartOkToNomalRun_Cond,
            .next_state = &_gBizFwStateNomalRun,
        },
    },
};

/* -------------------------------------------------------------------------- */
/* PV_START                                                                   */
/* -------------------------------------------------------------------------- */

static uint16_t _PvStartOkToNomalRun_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.pv_start_done != 0U && gBizFsm_Cfg.work_mode == EN_RUN_WORK_MODE_NORMAL;
}

static uint16_t _PvStartOkToTest_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.pv_start_done != 0U && gBizFsm_Cfg.work_mode == EN_RUN_WORK_MODE_TEST;
}

static uint16_t _PvStartToFault_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.pv_start_fail != 0U;
}

static void _PvStartOnExit(void* context)
{
    (void)context;
    _ClearPvStartFlags();
    BizFsm_Ready_BusVoltDlyEntryReset();
}

static BizFw_FsmState_t _gBizFwStatePvStart = {
    .state_id = EN_RUN_STATE_DEF_PV_START,
    .transition_count = 8,
    .function = BizStatePvStart_Function,
    .on_exit = _PvStartOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_FAULT,
        TRANSITION_BOOT,
        TRANSITION_INIT_PARAM,
        TRANSITION_POWER_OFF,
        TRANSITION_READY,
        {
            .condition = _PvStartToFault_Cond,
            .next_state = &_gBizFwStateFault,
        },
        {
            .condition = _PvStartOkToTest_Cond,
            .next_state = &_gBizFwStateTestMode,
        },
        {
            .condition = _PvStartOkToNomalRun_Cond,
            .next_state = &_gBizFwStateNomalRun,
        },
    },
};

/* -------------------------------------------------------------------------- */
/* NOMAL_START                                                                */
/* -------------------------------------------------------------------------- */

static uint16_t _NomalStartOkToNomalRun_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.nomal_start_done != 0U && gBizFsm_Cfg.work_mode == EN_RUN_WORK_MODE_NORMAL;
}

static uint16_t _NomalStartOkToTest_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.nomal_start_done != 0U && gBizFsm_Cfg.work_mode == EN_RUN_WORK_MODE_TEST;
}

static uint16_t _NomalStartToFault_Cond(void* context)
{
    (void)context;
    return gBizFsm_Status.nomal_start_fail != 0U;
}

static void _NomalStartOnExit(void* context)
{
    (void)context;
    _ClearNomalStartFlags();
    BizFsm_Ready_BusVoltDlyEntryReset();
}

static BizFw_FsmState_t _gBizFwStateNomalStart = {
    .state_id = EN_RUN_STATE_DEF_NOMAL_START,
    .transition_count = 8,
    .function = BizStateNomalStart_Function,
    .on_exit = _NomalStartOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_FAULT,
        TRANSITION_BOOT,
        TRANSITION_INIT_PARAM,
        TRANSITION_POWER_OFF,
        TRANSITION_READY,
        {
            .condition = _NomalStartToFault_Cond,
            .next_state = &_gBizFwStateFault,
        },
        {
            .condition = _NomalStartOkToTest_Cond,
            .next_state = &_gBizFwStateTestMode,
        },
        {
            .condition = _NomalStartOkToNomalRun_Cond,
            .next_state = &_gBizFwStateNomalRun,
        },
    },
};

/* -------------------------------------------------------------------------- */
/* NOMAL_RUN                                                                  */
/* -------------------------------------------------------------------------- */

static void _NomalRunOnExit(void* context)
{
    BizFsm_NomalRun_OnExit(context);
}

static BizFw_FsmState_t _gBizFwStateNomalRun = {
    .state_id = EN_RUN_STATE_DEF_NOMAL_RUN,
    .transition_count = 2,
    .function = BizStateNomalRun_Function,
    .on_exit = _NomalRunOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_FAULT,
        TRANSITION_READY,
    },
};

/* -------------------------------------------------------------------------- */
/* TEST_MODE                                                                  */
/* -------------------------------------------------------------------------- */

static void _TestModeOnExit(void* context)
{
    BizFsm_TestMode_OnExit(context);
}

static BizFw_FsmState_t _gBizFwStateTestMode = {
    .state_id = EN_RUN_STATE_DEF_TEST_MODE,
    .transition_count = 2,
    .function = BizStateTestMode_Function,
    .on_exit = _TestModeOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_FAULT,
        TRANSITION_READY,
    },
};

/* -------------------------------------------------------------------------- */
/* POWER_OFF (shutdown task state) -> IDLE                                      */
/* -------------------------------------------------------------------------- */

static uint16_t _PowerOffToIdleCheck(void* context)
{
    (void)context;
    return (gBizFsm_Cmd.power_off_cmd == 0U) && (gBizFsm_Status.power_off_done != 0U)
           && (gBizFsm_Status.fault_cleared == 0U);
}

static void _PowerOffToIdleAction(void* context)
{
    (void)context;
    _ClearPowerOffFlags();
}

static void _PowerOffOnExit(void* context)
{
    (void)context;
    _ClearPowerOffFlags();
}

#define TRANSITION_POWER_OFF_TO_IDLE                                                                                   \
    {                                                                                                                  \
        _PowerOffToIdleCheck, _PowerOffToIdleAction, &_gBizFwStateIdle                                                      \
    }

static BizFw_FsmState_t _gBizFwStatePowerOff = {
    .state_id = EN_RUN_STATE_POWER_OFF,
    .transition_count = 1,
    .function = BizStatePowerOff_Function,
    .on_exit = _PowerOffOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_POWER_OFF_TO_IDLE,
    },
};

/* -------------------------------------------------------------------------- */
/* FAULT                                                                      */
/* -------------------------------------------------------------------------- */

static uint16_t _FaultToShutDown_Cond(void* context)
{
    (void)context;
    return (gBizFsm_Cmd.is_err != 0U) && (gBizFsm_Cmd.fault_class == EN_RUN_FAULT_UNRECOVERABLE);
}

static uint16_t _FaultToIdle_Cond(void* context)
{
    (void)context;
    return (gBizFsm_Cmd.is_err == 0U) && (gBizFsm_Cmd.fault_class == EN_RUN_FAULT_NONE);
}

static void _FaultToIdle_Action(void* context)
{
    (void)context;
    _ClearAllProgressFlags();
}

static BizFw_FsmState_t _gBizFwStateFault = {
    .state_id = EN_RUN_STATE_DEF_FAULT,
    .transition_count = 4,
    .function = BizStateFault_Function,
    .transitions = (BizFw_FsmTransition_t[]){
        {
            .condition = _FaultToShutDown_Cond,
            .action = NULL,
            .next_state = &_gBizFwStateShutDown,
        },
        TRANSITION_BOOT,
        TRANSITION_INIT_PARAM,
        {
            .condition = _FaultToIdle_Cond,
            .action = _FaultToIdle_Action,
            .next_state = &_gBizFwStateIdle,
        },
    },
};

/* -------------------------------------------------------------------------- */
/* SHUT_DOWN                                                                  */
/* -------------------------------------------------------------------------- */

static void _ShutDownOnExit(void* context)
{
    (void)context;

}

static BizFw_FsmState_t _gBizFwStateShutDown = {
    .state_id = EN_RUN_STATE_DEF_SHUT_DOWN,
    .transition_count = 3,
    .function = BizStateShutDown_Function,
    .on_exit = _ShutDownOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        TRANSITION_BOOT,
        TRANSITION_INIT_PARAM,
        TRANSITION_POWER_OFF,
    },
};

/* -------------------------------------------------------------------------- */
/* INIT_PARAM                                                                 */
/* -------------------------------------------------------------------------- */

static uint16_t _InitParamToIdle_Cond(void* context)
{
    (void)context;
    return (gBizFsm_Status.init_param_done != 0U) && (gBizFsm_Cmd.factory_reset_cmd == 0U)
           && (gBizFsm_Status.fault_cleared == 0U);
}

static void _InitParamToIdle_Action(void* context)
{
    (void)context;
    _ClearInitParamFlags();
}

static void _InitParamOnExit(void* context)
{
    (void)context;
    _ClearInitParamFlags();
}

static BizFw_FsmState_t _gBizFwStateInitParam = {
    .state_id = EN_RUN_STATE_DEF_INIT_PARAM,
    .transition_count = 1,
    .function = BizStateInitParam_Function,
    .on_exit = _InitParamOnExit,
    .transitions = (BizFw_FsmTransition_t[]){
        {
            .condition = _InitParamToIdle_Cond,
            .action = _InitParamToIdle_Action,
            .next_state = &_gBizFwStateIdle,
        },
    },
};

/* -------------------------------------------------------------------------- */
/* BOOT (no FSM exit; boot_done / boot_fail set by upgrade handler)           */
/* -------------------------------------------------------------------------- */

static BizFw_FsmState_t _gBizFwStateBoot = {
    .state_id = EN_RUN_STATE_DEF_BOOT,
    .transition_count = 0,
    .function = BizStateBoot_Function,
    .transitions = NULL,
};

void BizFsm_State_Init(void)
{
    gBizFsm_Status.auto_save_err_cnt = 0U;
    BizFw_Fsm_Init(&gBizFsm_Fsm, &_gBizFwStateIdle);
    gBizFsm_Status.state = EN_RUN_STATE_DEF_IDLE;
    _ClearAllProgressFlags();
    gBizFsm_Status.fault_cleared = 0U;
}
