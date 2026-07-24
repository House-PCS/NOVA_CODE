/**
 * @file biz_fsm_def.h
 * @brief PCS business FSM type definitions and global data declarations
 */

#ifndef BIZ_FSM_DEF_H
#define BIZ_FSM_DEF_H

#include "F28x_Project.h"
#include "BizFw_Fsm.h"

/** @brief PCS system business states (PDF 2.1) */
typedef enum
{
    EN_RUN_STATE_DEF_IDLE = 0,
    EN_RUN_STATE_DEF_SELF_TEST = 1,
    EN_RUN_STATE_DEF_READY = 2,
    EN_RUN_STATE_DEF_GEN_START = 3,
    EN_RUN_STATE_DEF_PV_START = 4,
    EN_RUN_STATE_DEF_NOMAL_START = 5,
    EN_RUN_STATE_DEF_NOMAL_RUN = 6,
    EN_RUN_STATE_DEF_TEST_MODE = 7,
    EN_RUN_STATE_DEF_FAULT = 8,
    EN_RUN_STATE_DEF_SHUT_DOWN = 9,
    EN_RUN_STATE_DEF_BOOT = 10,
    EN_RUN_STATE_DEF_INIT_PARAM = 11,
    /** @note Power-off task state; power_off_done set when shutdown completes; exit to IDLE when power_off_cmd cleared, power_off_done set, and fault_cleared cleared externally */
    EN_RUN_STATE_POWER_OFF = 12,
    EN_RUN_STATE_TAIL = 0xFFFFFFFF
} EN_RUN_STATE;

/** @brief Start mode selection in READY state */
typedef enum
{
    EN_RUN_START_MODE_NONE = 0,    /**< No action (default; restored when leaving READY) */
    EN_RUN_START_MODE_NORMAL = 1,
    EN_RUN_START_MODE_PV = 2,
    EN_RUN_START_MODE_GEN = 3,
    EN_RUN_START_MODE_TAIL = 0xFFFFFFFF
} EN_RUN_START_MODE;

/** @brief Normal run / test mode (PDF 2.1.7 / 2.1.8, mutually exclusive) */
typedef enum
{
    EN_RUN_WORK_MODE_NORMAL = 0,
    EN_RUN_WORK_MODE_TEST = 1,
    EN_RUN_WORK_MODE_TAIL = 0xFFFFFFFF
} EN_RUN_WORK_MODE;

/** @brief Fault recoverability class */
typedef enum
{
    EN_RUN_FAULT_NONE = 0,
    EN_RUN_FAULT_RECOVERABLE = 1,
    EN_RUN_FAULT_UNRECOVERABLE = 2,
    EN_RUN_FAULT_TAIL = 0xFFFFFFFF
} EN_RUN_FAULT_CLASS;

/**
 * @brief External operation permissions (published via biz_fsm_interface)
 */
typedef struct biz_fsm_ext_allow_stru_t
{
    uint8_t allow_offgrid;       /**< 1 = off-grid inverter operation permitted */
    uint8_t allow_grid_connect;  /**< 1 = grid-tied operation permitted */
    uint8_t allow_pv;            /**< 1 = PV operation permitted */
} BizFsm_ExtAllow_t;

/**
 * @brief Monitor card / host commands (written and cleared by comm layer only)
 */
typedef struct biz_fsm_cmd_stru_t
{
    uint8_t power_on_cmd;       /**< Power-on command (power_on_cmd == 1) */
    uint8_t power_off_cmd;      /**< Power-off command */
    uint8_t upgrade_cmd;        /**< Upgrade command -> BOOT */
    uint8_t factory_reset_cmd;  /**< Factory reset -> INIT_PARAM */
    uint8_t power_on_reset;     /**< Power-on reset (exit SHUT_DOWN) */
    uint8_t is_err;             /**< Fault trigger (written and cleared by comm layer only) */
    EN_RUN_FAULT_CLASS fault_class; /**< Fault recoverability (written and cleared by comm layer only) */
} BizFsm_Cmd_t;

/**
 * @brief Per-state completion flags (set by state handlers / HW layer)
 */
typedef struct biz_fsm_status_stru_t
{
    uint8_t idle_done;
    uint8_t idle_fail;
    uint8_t self_test_pass;
    uint8_t self_test_fail;
    uint8_t gen_start_done;
    uint8_t gen_start_fail;
    uint8_t pv_start_done;
    uint8_t pv_start_fail;
    uint8_t nomal_start_done;
    uint8_t nomal_start_fail;
    uint8_t init_param_done;
    uint8_t init_param_fail;
    uint8_t boot_done;
    uint8_t boot_fail;
    uint8_t power_off_done;
    uint8_t power_off_fail;
    uint8_t fault_cleared; /**< Set to 1 on POWER_OFF entry; cleared externally via BizFsm_Interface_SetFaultCleared only */
    uint16_t factory_reset_cnt;
    uint16_t power_off_cnt;
    uint16_t auto_save_err_cnt;
    EN_RUN_STATE state;
} BizFsm_Status_t;

/**
 * @brief Runtime configuration
 */
typedef struct biz_fsm_cfg_stru_t
{
    EN_RUN_START_MODE start_mode;
    EN_RUN_WORK_MODE work_mode;
} BizFsm_Cfg_t;

/** @note Globals are for biz_fsm internal use only; external modules shall use biz_fsm_interface.h */
extern BizFsm_Cfg_t gBizFsm_Cfg;
extern BizFsm_Status_t gBizFsm_Status;
extern BizFsm_Cmd_t gBizFsm_Cmd;
extern BizFsm_ExtAllow_t gBizFsm_ExtAllow;
extern BizFw_Fsm_t gBizFsm_Fsm;

#endif /* BIZ_FSM_DEF_H */
