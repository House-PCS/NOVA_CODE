/**
 * @file energy_manage.c
 * @brief 能量管理：转发输入至 energy_ctrl 并读取三路功率输出
 */

#include "energy_manage.h"
#include "energy_ctrl.h"
#include <string.h>

EnergyManagePwrOut_t gstEnergyManagePwrOut;

void EnergyManage_Run(const EnergyCtrlInput_t* p_in)
{
    EnergyCtrlInput_t* p_work;
    EnergyCtrlOutput_t* p_ctrl_out;
    if((p_in == NULL) || (gstEnergyManageData.inv_enable == 0U))
    {
        gstEnergyManagePwrOut.pv_pwr = 0.0f;
        gstEnergyManagePwrOut.grid_active_pwr = 0.0f;
        gstEnergyManagePwrOut.gen_pwr = 0.0f;
        return;
    }
    p_work = GetEnergyInput();
    memcpy(p_work, p_in, sizeof(EnergyCtrlInput_t));
    EnergyCtrlProcess();
    p_ctrl_out = GetEnergyOutput();
    gstEnergyManagePwrOut.pv_pwr = POWER_VALUE_LIMIT(p_ctrl_out->power_pv2bus_set,
                                                     0,
                                                     PV_MAX_PWR_W);
    gstEnergyManagePwrOut.grid_active_pwr = POWER_VALUE_LIMIT(p_ctrl_out->power_act_grid2pcs_set,
                                                              -GRID_OUT_MAX_ACTIVE_PWR_W,
                                                              GRID_IN_MAX_ACTIVE_PWR_W);
    gstEnergyManagePwrOut.gen_pwr = POWER_VALUE_LIMIT(p_ctrl_out->power_act_gen2pcs_set,
                                                      0,
                                                      OIL_MAX_ACTIVE_PWR_W);
}
