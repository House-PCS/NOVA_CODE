/**
 * @file pv_cfg.h
 * @brief 光伏(PV)控制系统配置头文件
 * @details 定义了光伏控制系统的所有配置参数，包括：
 *          - 通道数量配置
 *          - 电压/电流/功率限制参数
 *          - MPPT控制参数
 *          - 控制环路参数
 *          - 平均计算参数
 * @note 支持通过 PV_LOW_VOL_TEST 宏切换低压测试模式和正常模式
 */
#ifndef PV_CFG_H
#define PV_CFG_H

/**
 * @brief Boost 电感值（H），用于PV电压混合模式卡尔曼观测器
 */
#define PV_HW_INDUCTANCE_H         (600e-6f)

/** 开关频率（Hz），物理硬件常数 */
#define PV_HW_SWITCH_FREQ_HZ           (30000.0f)

/* ========================================================================
 * 系统配置
 * ======================================================================== */

/**
 * @brief 光伏通道数量
 */
#define PV_CHANNELS          4

/* ========================================================================
 * 电压/电流/功率限制配置
 * ======================================================================== */

#ifdef PV_LOW_VOL_TEST
/* 低压测试模式参数 */

/** @brief 通道最低工作电压(V) */
#define PV_CHANNEL_MIN_VOLTAGE       50.0f

/** @brief 通道最高工作电压(V) */
#define PV_CHANNEL_MAX_VOLTAGE       120.0f

/** @brief 通道最大工作电流(A) */
#define PV_CHANNEL_MAX_CURRENT       16.0f

/** @brief 通道最大占空比(0~1) */
#define PV_CHANNEL_MAX_DUTY          0.95f

/** @brief 通道最小占空比(0~1) */
#define PV_CHANNEL_MIN_DUTY          0.05f

/** @brief 母线最大电压(V) */
#define PV_BUS_MAX_VOLTAGE           120.0f

/** @brief 母线启动电压阈值(V) */
#define PV_BUS_START_UP_VOLTAGE      650.0f

/** @brief 母线正常运行限制电压(V) */
#define PV_BUS_NOMAL_LIMIT_VOLTAGE   880.0f

/** @brief 单通道最大功率(W) */
#define PV_CHANNEL_MAX_POWER         1000.0f

/** @brief 系统总功率限制(W) */
#define PV_TOTAL_MAX_POWER           4000.0f



#define PV_ERR_CURR_OVER_THD          (16.0f) //过流告警阈值，单位A
#define PV_ERR_BUS_VOLT_OVER_THD      (125.0f) //母线过压告警阈值，单位V

#define PV_WARN_VOLT_LOW_THD          (50.0f) //电压过低告警阈值，单位V
#define PV_WARN_VOLT_OVER_THD         (120.0f) //电压过高告警阈值，单位V

#else
/* 正常工作模式参数 */

/**
 * @brief 通道最低工作电压(V)
 * @details 低于此值停止工作
 */
#define PV_CHANNEL_MIN_VOLTAGE       70.0f

/**
 * @brief 通道最高工作电压(V)
 * @details 超过此值限压保护
 */
#define PV_CHANNEL_MAX_VOLTAGE       450.0f

/**
 * @brief 通道最大工作电流(A)
 * @details 超过此值限流保护
 */
#define PV_CHANNEL_MAX_CURRENT       18.0f

/**
 * @brief 通道最大占空比(0~1)
 * @details PWM占空比上限
 */
#define PV_CHANNEL_MAX_DUTY          0.95f

/**
 * @brief 通道最小占空比(0~1)
 * @details PWM占空比下限，防止升压变换器完全关断
 */
#define PV_CHANNEL_MIN_DUTY          0.05f

/**
 * @brief 母线最大电压(V)
 * @details 母线电压上限
 */
#define PV_BUS_MAX_VOLTAGE           520.0f

/**
 * @brief 母线启动电压阈值(V)
 * @details 系统启动所需最低母线电压
 */
#define PV_BUS_START_UP_VOLTAGE      480.0f

/**
 * @brief 母线正常运行限制电压(V)
 * @details 正常运行时母线电压上限
 */
#define PV_BUS_NOMAL_LIMIT_VOLTAGE   950.0f

/**
 * @brief 单通道最大功率(W)
 * @details 单个PV通道最大输出功率
 */
#define PV_CHANNEL_MAX_POWER         6500.0f

/**
 * @brief 系统总功率限制(W)
 * @details 所有通道总功率上限
 */
#define PV_TOTAL_MAX_POWER           24000.0f


#define PV_ERR_CURR_OVER_THD          (22.0f) //过流告警阈值，单位A
#define PV_ERR_BUS_VOLT_OVER_THD      (550.0f) //母线过压告警阈值，单位V

#define PV_WARN_VOLT_LOW_THD          (70.0f) //电压过低告警阈值，单位V
#define PV_WARN_VOLT_OVER_THD         (520.0f) //电压过高告警阈值，单位V
#endif

/* ========================================================================
 * MPPT(最大功率点跟踪)控制参数
 * ======================================================================== */

/**
 * @brief MPPT电压参考值变化最大值(V/s)
 * @details 电压参考值每秒最大变化量
 */
#define PV_MPPT_D_VREF_MAX             (80.0f)

/**
 * @brief MPPT电压参考值变化比例系数
 * @details 调节电压参考值变化速度的增益
 */
#define PV_MPPT_D_VREF_K               (40.0f)

/**
 * @brief MPPT控制频率(Hz)
 * @details MPPT算法执行频率
 */
#define PV_MPPT_CNTL_FS              (12.5f) 

/**
 * @brief MPPT控制周期(s)
 * @details 由控制频率计算得出
 */
#define PV_MPPT_CNTL_TS              (1.0f/PV_MPPT_CNTL_FS)

/**
 * @brief 电流限制范围(A)
 * @details 进入电流限制状态的容差范围,0-1
 */
#define PV_MPPT_CUR_LIMIT_ZONE       (0.1f)

/**
 * @brief 功率限制范围(W)
 * @details 进入功率限制状态的容差范围,0-1
 */
#define PV_MPPT_PWR_LIMIT_ZONE       (0.5f)

/**
 * @brief 功率标准差限制(W)
 * @details 用于判断功率稳定性
 * @note 暂未启用
 */
#define PV_MPPT_PWR_STD_LIMIT        (500.0f)

/**
 * @brief 是否启用分时控制 (0=禁用, 1=启用)
 * @details 多通道分时计算以降低负载，同时控制 MPPT 与内环（电流环/功率环）是否采用分时调度
 */
#define PV_CNTL_TIME_SLICE           (1)

/* ========================================================================
 * 控制环路参数
 * ======================================================================== */

/**
 * @brief 控制环执行频率(Hz)
 * @details 内环(电流环/功率环)的执行频率
 */
#define PV_LOOP_FS                  (5000U)

/**
 * @brief 控制环执行周期(s)
 * @details 由执行频率计算得出
 */
#define PV_LOOP_TS                  (1.0f/PV_LOOP_FS)

/**
 * @brief 功率调整限制(W)
 * @note 已弃用，保留用于参考
 */
#define PV_LOOP_PWR_ADJUST_LIMIT    (500.0f)

/**
 * @brief 功率调整比例系数
 * @details 功率调节速度调节参数
 */
#define PV_LOOP_PWR_ADJUST_K        (10.0f)

/**
 * @brief 多通道功率均衡比例系数
 * @details 用于平衡各通道功率输出，越大平衡力度越强
 */
#define PV_LOOP_PWR_BALANCE_K       (0.2f)

/**
 * @brief 功率调整阻尼系数
 * @details 抑制功率调节震荡
 * @note 暂未启用
 */
#define PV_LOOP_PWR_ADJUST_KD       (1.0f)

/* ========================================================================
 * 采样与平均计算参数
 * ======================================================================== */

/**
 * @brief 平均值计算采样点数
 * @details 在MPPT周期内采样次数，用于计算平均功率/电压/电流
 */
#define PV_AVG_SAMPLES              (PV_LOOP_FS / PV_MPPT_CNTL_FS)

#endif // PV_CFG_H


