/**
 * @file energy_management_def.h
 * @brief 能量管理模块数据结构定义
 */

#ifndef ENERGY_MANAGEMENT_DEF_H
#define ENERGY_MANAGEMENT_DEF_H

#include "F28x_Project.h"
#include "energy_management_cfg.h"

typedef struct deploy_info_stru_t
{
    uint16_t dsg_enable;                /**< 放电使能 */
    uint16_t chg_enable;                /**< 充电使能 */
    uint16_t backup_output_enable;      /**< 备电输出使能 */
    uint16_t antireverseflow_enable;    /**< 防逆流使能 */
    uint16_t charging_input_type;       /**< 充电输入类型 */
    uint16_t work_mode;                 /**< 工作模式 */
    uint16_t grid_standards;            /**< 电网标准 */
    uint16_t cto_meter;                 /**< CT 电表 */
    uint16_t ct_direction;              /**< CT 方向 */
    uint32_t on_grid_reconnect_time;    /**< 并网重连时间 (ms) */
    float32 set_oil_power;              /**< 油机功率设定 (W) */
    float32 set_batt_chg_power;         /**< 电池充电功率设定 (W) */
    float32 set_batt_dsg_power;         /**< 电池放电功率设定 (W) */
    float32 on_grid_pwr;                /**< 并网功率设定 (W) */
    float32 backup_feekback_activerpwr; /**< 备电回读有功 (W) */
    float32 pv_feekback_pwr;            /**< 光伏回读功率 (W) */
} DeployInfo_t;

/** @brief 能量管理数据 */
typedef struct energy_manage_data_stru_t
{
    uint16_t grid_enable;     /**< 电网使能：1 使能，0 禁止 */
    uint16_t oil_enable;      /**< 油机使能：1 使能，0 禁止 */
    uint16_t drm_state_bits;  /**< DRM 状态位 (udp_pcs_hold.drm_state) */
    uint16_t inv_enable;      /**< 逆变使能：1 使能，0 禁止 */
    float32 ctr_temp;         /**< 控制板温度 (℃) */
    float32 inv_temp;         /**< 逆变温度 (℃) */
    float32 pv_temp;          /**< 光伏温度 (℃) */
    float32 inv_volt_rms;     /**< 逆变电压有效值 (V) */
    uint16_t inv_run_state;   /**< GRID_CONNECT_TIME_INV_* */
    uint32_t now_ms;          /**< 系统时间 (ms) */
    uint32_t reconnect_ms;    /**< 离网后重连等待时间 (ms) */
    uint32_t connect_timeout_ms; /**< 并网中状态超时 (ms) */
    float32 bus_voltage;      /**< 直流母线电压 (V) */
    float32 load_total_active_pwr; /**< 负载总有功功率 (W) */
    float32 max_pwr_slope;    /**< 有功斜坡步长 (W/周期) */
} EnergyManageData_t;

extern DeployInfo_t gstDeployInfo;
extern EnergyManageData_t gstEnergyManageData;
#endif // ENERGY_MANAGEMENT_DEF_H
