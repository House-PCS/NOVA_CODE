/**
 * @file biz_fsm_def.c
 * @brief PCS business FSM global data definitions
 */

#include "biz_fsm_def.h"

BizFsm_Cfg_t gBizFsm_Cfg = {
    .start_mode = EN_RUN_START_MODE_NONE,
    .work_mode = EN_RUN_WORK_MODE_NORMAL,
};

BizFsm_Status_t gBizFsm_Status = {0};
BizFsm_Cmd_t gBizFsm_Cmd = {0};
BizFsm_ExtAllow_t gBizFsm_ExtAllow = {0};

BizFw_Fsm_t gBizFsm_Fsm = {
    .current_state_id = EN_RUN_STATE_DEF_IDLE,
    .current_state = NULL,
};
