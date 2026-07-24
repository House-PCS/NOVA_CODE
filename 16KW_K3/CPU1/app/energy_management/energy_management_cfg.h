/**
 * @file energy_management_cfg.h
 * @brief 能量管理模块统一配置参数
 */

#ifndef ENERGY_MANAGEMENT_CFG_H
#define ENERGY_MANAGEMENT_CFG_H

/* -------------------------------------------------------------------------- */
/* 电网 / 油机 / 逆变额定功率限制                                               */
/* -------------------------------------------------------------------------- */

/* 电网输入（从电网取电） */
#define GRID_IN_MAX_ACTIVE_PWR_W        (24000.0f)  /**< 电网输入最大有功功率 (W) */
#define GRID_IN_MAX_REACTIVE_PWR_VAR    (16000.0f)  /**< 电网输入最大无功功率 (VAR) */
#define GRID_IN_MAX_APPARENT_PWR_VA     (24000.0f)  /**< 电网输入最大视在功率 (VA) */

/* 电网输出（向电网馈电） */
#define GRID_OUT_MAX_ACTIVE_PWR_W       (16000.0f)  /**< 电网输出最大有功功率 (W) */
#define GRID_OUT_MAX_REACTIVE_PWR_VAR   (16000.0f)  /**< 电网输出最大无功功率 (VAR) */
#define GRID_OUT_MAX_APPARENT_PWR_VA    (16000.0f)  /**< 电网输出最大视在功率 (VA) */

#define OIL_MAX_ACTIVE_PWR_W            (16000.0f)  /**< 油机最大有功功率 (W) */
#define OIL_MAX_REACTIVE_PWR_VAR        (16000.0f)  /**< 油机最大无功功率 (VAR) */
#define OIL_MAX_APPARENT_PWR_VA         (16000.0f)  /**< 油机最大视在功率 (VA) */

/* 逆变并网 */
#define INV_ON_GRID_MAX_ACTIVE_PWR_W    (16000.0f)  /**< 逆变并网最大有功功率 (W) */
#define INV_ON_GRID_MAX_REACTIVE_PWR_VAR (16000.0f) /**< 逆变并网最大无功功率 (VAR) */
#define INV_ON_GRID_MAX_APPARENT_PWR_VA (16000.0f)  /**< 逆变并网最大视在功率 (VA) */

/* 逆变离网 */
#define INV_OFFGRID_MAX_ACTIVE_PWR_W    (17500.0f)  /**< 逆变离网最大有功功率 (W) */
#define INV_OFFGRID_MAX_REACTIVE_PWR_VAR (16000.0f) /**< 逆变离网最大无功功率 (VAR) */
#define INV_OFFGRID_MAX_APPARENT_PWR_VA (17500.0f)  /**< 逆变离网最大视在功率 (VA) */

/* 光伏 */
#define PV_MAX_PWR_W                    (24000.0f)  /**< 光伏最大功率 (W) */

/* 逆变电压 */
#define INV_RATED_VOLT_RMS_V            (230.0f)  /**< 逆变额定电压有效值 (V) */
#define INV_MIN_VOLT_RMS_V              (180.0f)  /**< 逆变电压有效值下限 (V) */
#define INV_MAX_VOLT_RMS_V              (240.0f)  /**< 逆变电压有效值上限 (V) */

/* -------------------------------------------------------------------------- */
/* 温度降额 (temp_pwr_derate)                                                  */
/* -------------------------------------------------------------------------- */

#define TEMP_PWR_DERATE_PV_TEMP_START           (98.0f)   /**< 光伏降额起始温度 (℃) */
#define TEMP_PWR_DERATE_PV_TEMP_STOP            (108.0f)  /**< 光伏降额截止温度 (℃) */
#define TEMP_PWR_DERATE_INV_TEMP_START          (103.0f)  /**< 逆变降额起始温度 (℃) */
#define TEMP_PWR_DERATE_INV_TEMP_STOP           (110.0f)  /**< 逆变降额截止温度 (℃) */
#define TEMP_PWR_DERATE_CTR_TEMP_START          (88.0f)   /**< 控制板降额起始温度 (℃) */
#define TEMP_PWR_DERATE_CTR_TEMP_STOP           (105.0f)  /**< 控制板降额截止温度 (℃) */

#define TEMP_PWR_DERATE_INV_DISCONNECT_MIN_PWR  (1000.0f)
#define TEMP_PWR_DERATE_PHASE_NUM               (3.0f)

#define TEMP_PWR_DERATE_PV_PWR_MAX_W            (PV_MAX_PWR_W)
#define TEMP_PWR_DERATE_PV_PWR_MIN_W            (0.0f)

#define TEMP_PWR_DERATE_INV_PWR_GRID_MAX_W      (INV_ON_GRID_MAX_ACTIVE_PWR_W)
#define TEMP_PWR_DERATE_INV_PWR_GRID_MIN_W      (0.0f)

#define TEMP_PWR_DERATE_DISCONNECT_VOLT_MAX_V   (240)

#define TEMP_PWR_DERATE_DISCONNECT_VOLT_MIN_V   (185)
#define TEMP_PWR_DERATE_DISCONNECT_VOLT_RMS_V   (230)

/* -------------------------------------------------------------------------- */
/* 功率控制调度 (pwr_ctrl)                                                     */
/* -------------------------------------------------------------------------- */

#define PWR_CTRL_SCHED_CNT_MAX          (100U)  /**< 调度周期计数模值 */
#define PWR_CTRL_SCHED_SLOW_DIV         (100U)  /**< 慢任务分频（温度降额 / DRM） */
#define PWR_CTRL_SCHED_MID_DIV          (10U)   /**< 中任务分频（能量管理 / 并网时间） */

/* -------------------------------------------------------------------------- */
/* 逆变功率设定 (inv_pwr_set)                                                  */
/* -------------------------------------------------------------------------- */

#define INV_PWR_SET_RAMP_DEADBAND_W       (50.0f)  /**< 功率斜坡死区 (W) */
#define INV_PWR_SET_MAX_PWR_SLOPE_W       (2.0f)  /**< 逆变功率最大斜坡步长 (W/周期) */
/* -------------------------------------------------------------------------- */
/* 并网时间控制 (grid_connect_time_ctrl)                                       */
/* -------------------------------------------------------------------------- */

#define GRID_CONNECT_TIME_INV_OFFGRID       (0U)
#define GRID_CONNECT_TIME_INV_CONNECTING    (1U)
#define GRID_CONNECT_TIME_INV_ON_GRID       (2U)
#define GRID_CONNECT_TIME_INV_DISCONNECTING (3U)
#define GRID_CONNECT_TIME_INV_OTHER         (4U)

/* -------------------------------------------------------------------------- */
/* 并网母线电压限功率 (grid_connect_bus_pwr_limit)                             */
/* -------------------------------------------------------------------------- */

#define GRID_BUS_VOLT_FILTER_WIN_SIZE           (10U)

#define GRID_BUS_PWR_LIMIT_PROTECT_UPPER_V      (920.0f)  /**< 母线电压保护上限 (V) */
#define GRID_BUS_PWR_LIMIT_PROTECT_LOWER_V      (720.0f)  /**< 母线电压保护下限 (V) */
#define GRID_BUS_PWR_LIMIT_THEORY_UPPER_V       (890.0f)  /**< 母线电压理论上限 (V) */
#define GRID_BUS_PWR_LIMIT_THEORY_LOWER_V       (750.0f)  /**< 母线电压理论下限 (V) */

#define GRID_BUS_PWR_LIMIT_KP                   (50.0f)   /**< 母线 PI 比例系数 */
#define GRID_BUS_PWR_LIMIT_KI                   (0.1f)    /**< 母线 PI 积分系数 */
#define GRID_BUS_PWR_LIMIT_CTRL_DT_S            (0.001f)  /**< 控制周期 (s) */

#define GRID_BUS_PWR_LIMIT_GRID_PWR_MAX_W       (INV_ON_GRID_MAX_ACTIVE_PWR_W)
#define GRID_BUS_PWR_LIMIT_INV_PWR_MIN_W        (-INV_ON_GRID_MAX_ACTIVE_PWR_W)

#endif // ENERGY_MANAGEMENT_CFG_H
