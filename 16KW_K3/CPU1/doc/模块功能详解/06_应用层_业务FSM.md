# 06 应用层：业务 FSM

对应源码：`app/biz_fsm/*`，底层框架 `middleware/BizFw_Fsm.*`

---

## 1. 定位

业务 FSM 是 PCS **高层运行许可状态机**：决定何时允许离网、并网、PV，以及自检、恢复出厂、关机等流程。  
真正的电力电子控制在 Inv3Ph/CLA/能量管理中完成；本层输出 **许可与命令**。

---

## 2. 状态与命令定义（`biz_fsm_def`）

### 2.1 运行状态 `EN_RUN_STATE`

`IDLE → SELF_TEST → READY → GEN_START / PV_START / NOMAL_START → NOMAL_RUN / TEST_MODE`  

另有：`FAULT`、`SHUT_DOWN`、`BOOT`、`INIT_PARAM`、`POWER_OFF`。

### 2.2 其它枚举

- 启动模式：`NONE / NORMAL / PV / GEN`  
- 工作模式：`NORMAL / TEST`  
- 故障类别：`NONE / 可恢复 / 不可恢复`  

### 2.3 全局对象

- `BizFsm_Cmd_t`：开关机、升级、恢复出厂、故障及类别  
- `BizFsm_Status_t`：各状态完成/失败位、计数、当前状态  
- `BizFsm_ExtAllow_t`：`allow_offgrid` / `allow_grid_connect` / `allow_pv`  

---

## 3. 配置（`biz_fsm_cfg`）

Ready 母线条件示例：

- 电压 350~450 V  
- 连续 5 次成立  

---

## 4. 延时触发别名（`biz_fsm_flag_trigger`）

对 `flag_dly_trigger` 的宏封装：

- `BIZ_FSM_FLAG_DLY_DECLARE`  
- `BIZ_FSM_FLAG_DLY_RUN`  
- `BIZ_FSM_FLAG_DLY_RESET`  

`.c` 无独立运行逻辑。

---

## 5. 状态处理函数（`biz_fsm_func`）

| 状态函数 | 行为摘要 |
|----------|----------|
| `BizStateIdle_Function` | 重置逆变算法状态，`idle_done=1` |
| `BizStateSelfTest_Function` | 跑 `SysSelfTest`；成功 `self_test_pass`，超时 fail |
| `BizStateReady_Function` | LLC 使能且母线 350~450V 连续 5 次 → 启动模式 NORMAL |
| `BizStateGenStart / PvStart` | 当前实现相同：母线满足即启动完成 |
| `BizStateNomalStart_Function` | 直接完成 |
| `BizStateNomalRun / TestMode` | 主体调度代码多被注释 |
| 退出 Normal/Test | `Inv3Ph_State_Init()` |
| `Fault / ShutDown / Boot` | 空骨架或极简 |
| `BizStateInitParam_Function` | `Restore_Factory_Settings(..., false)` 仅使 header 无效 |
| `BizStatePowerOff_Function` | `ShutDown_SaveFlash`，失败最多 5 次 |

---

## 6. 状态迁移表（`biz_fsm_state`）

### 6.1 通用高优先级

多数状态迁移表前几项统一：

1. Fault  
2. Boot  
3. Factory reset  
4. Power off  

### 6.2 主路径

```
IDLE --idle_done--> SELF_TEST --self_test_pass--> READY
READY --power_on & NORMAL--> NOMAL_START
READY --power_on & PV------> PV_START
READY --power_on & GEN-----> GEN_START
START 成功 --> NOMAL_RUN 或 TEST_MODE
START 失败 --> FAULT
power_on==0 --> READY
NORMAL_RUN/TEST --is_err--> FAULT
NORMAL_RUN/TEST --power_on==0--> READY
FAULT --不可恢复--> SHUT_DOWN
FAULT --错误清除且类别 NONE--> IDLE
```

### 6.3 特殊注意

- **BOOT 状态没有任何离开迁移**（进入后无法靠表退出）  
- POWER_OFF / INIT_PARAM 离开依赖 `fault_cleared==0` 等条件，需外部 `SetFaultCleared`  

---

## 7. 对外接口（`biz_fsm_interface`）

### 7.1 `BizFsm_Interface_Init`

初始化状态机与许可。

### 7.2 `BizFsm_Interface_Update`（主循环 10 ms）

1. 取通信开机/关机/恢复出厂命令  
2. 非 Boot/Init/PowerOff 时自动保存持有参数  
3. 调底层 `BizFw_Fsm_Update`  
4. 更新状态与外部许可  

### 7.3 许可规则

- 处于 `NORMAL_RUN` / `TEST_MODE`：离网、并网、PV **全允许**  
- 其它状态：全禁  
- `PV_START` / `GEN_START` 分支若为空，可能保留上一轮许可（粘滞风险）  

### 7.4 外部适配

- 自检 → POST  
- LLC 许可当前硬编码返回 1  
- 母线电压直接读 `gstAdcResult`  

---

## 8. 与系统的接线缺口（重要）

1. **`BizFsm_Interface_SetIsErr()` 未与 `GetErrorFlag()` 自动连接** → 真实故障不一定进 FAULT  
2. **升级命令未看到与升级寄存器完整接线**  
3. BOOT 不可退出  
4. `fault_cleared` 清零路径应用层缺失时，Init/PowerOff 可能卡住  
5. Normal/Test 内大量调度注释 → 框架已建、内容未闭环  

因此：当前业务 FSM 是 **可运行骨架**，不能视为完整安全主状态机。

---

## 9. 底层框架行为（BizFw_Fsm）

每拍：

1. 执行当前状态 `run`，计数 +1  
2. 顺序测试转移条件  
3. 命中：`on_exit` → 转移动作 → 切状态 → 新状态计数清零  

详见 [10_中间件](./10_中间件_middleware.md)。

---

## 附录：本章源码

> 以下为本章涉及的全部自研源码原文（不含 TI `include/`、`source/` 库）。

### `app/biz_fsm/biz_fsm_cfg.h`

```c
/**
 * @file biz_fsm_cfg.h
 * @brief PCS business FSM: fault/alarm/derate and related app-layer limits (excludes algo params)
 * @details Business thresholds and FSM criteria live here; fault/alarm codes in fault.h; algo in algo/pv_cfg.h
 */

#ifndef BIZ_FSM_CFG_H
#define BIZ_FSM_CFG_H

#include "F28x_Project.h"

/* PCS FSM: bus voltage criteria / debounce */
#define BIZ_FSM_BUS_VOLT_MIN_THRESHOLD   (350.0f)
#define BIZ_FSM_BUS_VOLT_MAX_THRESHOLD   (450.0f)
#define BIZ_FSM_BUS_VOLT_DLY_COUNT       (5U)

#endif /* BIZ_FSM_CFG_H */
```

### `app/biz_fsm/biz_fsm_def.c`

```c
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
```

### `app/biz_fsm/biz_fsm_def.h`

```c
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
```

### `app/biz_fsm/biz_fsm_flag_trigger.c`

```c
/**
 * @file biz_fsm_flag_trigger.c
 * @brief Debounced flag trigger hook for biz_fsm (see flag_dly_trigger)
 */

#include "biz_fsm_flag_trigger.h"
```

### `app/biz_fsm/biz_fsm_flag_trigger.h`

```c
/**
 * @file biz_fsm_flag_trigger.h
 * @brief Debounced condition-to-flag trigger macros for biz_fsm
 */

#ifndef BIZ_FSM_FLAG_TRIGGER_H
#define BIZ_FSM_FLAG_TRIGGER_H

#include "flag_dly_trigger.h"

#define BIZ_FSM_FLAG_DLY_DECLARE(id, p_flag, threshold)  FLAG_DLY_SET_DECLARE(id, p_flag, threshold)
#define BIZ_FSM_FLAG_DLY_RUN(id, cmd)                     FLAG_DLY_SET_RUN(id, cmd)
#define BIZ_FSM_FLAG_DLY_RESET(id, p_flag)               FLAG_DLY_SET_RESET(id, p_flag)

#endif /* BIZ_FSM_FLAG_TRIGGER_H */
```

### `app/biz_fsm/biz_fsm_func.c`

```c
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
```

### `app/biz_fsm/biz_fsm_func.h`

```c
/**
 * @file biz_fsm_func.h
 * @brief Periodic business state handler functions
 */

#ifndef BIZ_FSM_FUNC_H
#define BIZ_FSM_FUNC_H

void BizStateIdle_Function(void* context);
void BizStateSelfTest_Function(void* context);
void BizStateReady_Function(void* context);
void BizStateGenStart_Function(void* context);
void BizStatePvStart_Function(void* context);
void BizStateNomalStart_Function(void* context);
void BizStateNomalRun_Function(void* context);
void BizStateTestMode_Function(void* context);
void BizStateFault_Function(void* context);
void BizStateShutDown_Function(void* context);
void BizStateBoot_Function(void* context);
void BizStateInitParam_Function(void* context);
void BizStatePowerOff_Function(void* context);

void BizFsm_Ready_BusVoltDlyEntryReset(void);
void BizFsm_NomalRun_OnExit(void* context);
void BizFsm_TestMode_OnExit(void* context);

#endif /* BIZ_FSM_FUNC_H */
```

### `app/biz_fsm/biz_fsm_interface.c`

```c
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
```

### `app/biz_fsm/biz_fsm_interface.h`

```c
/**
 * @file biz_fsm_interface.h
 * @brief Sole interaction interface between biz_fsm and external modules
 * @details External modules shall access biz_fsm only through this header.
 *          Internal biz_fsm modules shall access external data/functions only through this header.
 */

#ifndef BIZ_FSM_INTERFACE_H
#define BIZ_FSM_INTERFACE_H

#include "biz_fsm_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Module lifecycle (called from external main loop)                          */
/* -------------------------------------------------------------------------- */

void BizFsm_Interface_Init(void);
int BizFsm_Interface_Update(void* context);

/* -------------------------------------------------------------------------- */
/* External -> biz_fsm: command writes (comm layer / monitor card, etc.)      */
/* -------------------------------------------------------------------------- */

void BizFsm_Interface_SetUpgradeCmd(uint8_t cmd);
void BizFsm_Interface_SetPowerOnReset(uint8_t cmd);
void BizFsm_Interface_SetIsErr(uint8_t err);
void BizFsm_Interface_SetFaultClass(EN_RUN_FAULT_CLASS fault_class);
void BizFsm_Interface_SetFaultCleared(uint8_t cleared);

/* -------------------------------------------------------------------------- */
/* biz_fsm -> external: status reads (for external module queries)            */
/* -------------------------------------------------------------------------- */

EN_RUN_STATE BizFsm_Interface_GetRunState(void);
EN_RUN_FAULT_CLASS BizFsm_Interface_GetFaultClass(void);
uint8_t BizFsm_Interface_GetPowerOnCmd(void);

/** @brief Operation permission flags (1 = allowed, 0 = not allowed) */
uint8_t BizFsm_Interface_GetAllowOffgrid(void);      /**< Off-grid inverter */
uint8_t BizFsm_Interface_GetAllowGridConnect(void);  /**< Grid-tied inverter */
uint8_t BizFsm_Interface_GetAllowPv(void);           /**< PV boost / MPPT */

/* -------------------------------------------------------------------------- */
/* Internal use: read external module data / call external functions          */
/* -------------------------------------------------------------------------- */

void BizFsm_Interface_ExtSelfTestRun(void);
uint8_t BizFsm_Interface_ExtGetSelfTestPass(void);
uint8_t BizFsm_Interface_ExtGetSelfTestTimeout(void);
uint8_t BizFsm_Interface_ExtGetPcsLLCEnable(void);
float BizFsm_Interface_ExtGetBusVoltNeg(void);
float BizFsm_Interface_ExtGetBusVoltPos(void);

#ifdef __cplusplus
}
#endif

#endif /* BIZ_FSM_INTERFACE_H */
```

### `app/biz_fsm/biz_fsm_state.c`

```c
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
```

### `app/biz_fsm/biz_fsm_state.h`

```c
/**
 * @file biz_fsm_state.h
 * @brief PCS business FSM state machine initialization
 */

#ifndef BIZ_FSM_STATE_H
#define BIZ_FSM_STATE_H

#include "BizFw_Fsm.h"

void BizFsm_State_Init(void);

#endif /* BIZ_FSM_STATE_H */
```

