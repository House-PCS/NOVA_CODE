/**
 * @file energy_manage.h
 * @brief 能量管理接口：对接 energy_ctrl 输入输出
 */

#ifndef ENERGY_MANAGE_H
#define ENERGY_MANAGE_H

#include "energy_management_def.h"
#include "energy_ctrl.h"

typedef struct energy_manage_pwr_out_stru_t
{
    float32 pv_pwr;              /**< 光伏有功设定 (W) */
    float32 grid_active_pwr;     /**< 电网有功设定 (W) */
    float32 gen_pwr;             /**< 油机有功设定 (W) */
} EnergyManagePwrOut_t;

extern EnergyManagePwrOut_t gstEnergyManagePwrOut;

/**
 * @brief 能量调度：CtReversePower(0) 后调用 energy_ctrl，输出三路有功设定
 * @param enable 非0表示逆变使能且 FSM 处于 NOMAL_RUN 或 TEST_MODE，否则直接返回
 */
void EnergyManage_Run(const EnergyCtrlInput_t* p_in);

#endif // ENERGY_MANAGE_H
