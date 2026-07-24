/**
 * @file pwr_ctrl_interface.c
 * @brief 功率控制接口：输入采集与初始化
 */

#include "pwr_ctrl_interface.h"
#include "energy_ctrl.h"
#include "biz_fsm_def.h"
#include "ethernet_analysis.h"
#include "sample_update.h"
#include "cla_cpu_share.h"
#include "ct.h"
#include "cpu_timers.h"
#include "control_state.h"
#include "ph3_calc_ifc_update.h"
#include <string.h>
#include <math.h>
#include "algo_interface.h"
#include <sample_update.h>
#include "cpu_timers.h"
#include "biz_fsm_interface.h"
#include "energy_manage.h"
#include "temp_pwr_derate.h"
#include "inv_pwr_set.h"
#include "drm_pwr_sched.h"
#include "grid_connect_bus_pwr_limit.h"
#include "grid_connect_time_ctrl.h"
/* ========================================================================== */
/*  全局变量                                                                   */
/* ========================================================================== */
#pragma DATA_SECTION(gstPwrCtrlInterface,"CpuToCla1MsgRAM");

DeployInfo_t gstDeployInfo;
EnergyManageData_t gstEnergyManageData;
EnergyCtrlInput_t gstEnergyCtrlInputData;
PwrCtrlInteface_t gstPwrCtrlInterface;

/* ========================================================================== */
/*  模块私有函数                                                               */
/* ========================================================================== */

static uint16_t _PwrCtrl_MapInvPhase(EN_INV_STATE inv_state)
{
    if(inv_state == INV_STATE_DISCONNECTED)
    {
        return GRID_CONNECT_TIME_INV_OFFGRID;
    }
    if((inv_state == INV_STATE_GRID_CONNECTING) || (inv_state == INV_STATE_GEN_CONNECTING))
    {
        return GRID_CONNECT_TIME_INV_CONNECTING;
    }
    if((inv_state == INV_STATE_GRID_CONNECTED) || (inv_state == INV_STATE_GEN_CONNECTED))
    {
        return GRID_CONNECT_TIME_INV_ON_GRID;
    }
    if((inv_state == INV_STATE_GRID_DISCONNECTING) || (inv_state == INV_STATE_GEN_DISCONNECTING))
    {
        return GRID_CONNECT_TIME_INV_DISCONNECTING;
    }
    return GRID_CONNECT_TIME_INV_OTHER;
}
/* ========================================================================== */
/*  对外接口                                                                   */
/* ========================================================================== */
uint16_t GetDrmGridConnectEN(void)
{
    return gstDrmPwrSchedOut.drm_relay_control;
}

uint16_t GetConnectTimeEN(void)
{
    return gstGridConnectTimeCtrlOut.grid_enable;
}

void PwrCtrl_Set_DeployFromComm(void)
{
    int32_t high_value;
    int32_t low_value;
    int32_t combine_value;
    gstDeployInfo.work_mode = gstUdpPcsPara.udp_pcs_hold.work_mode;
//    if(gstDeployInfo.charging_input_type == 1U)
    if(WorkPermit_GenForceEn())
    {
        gstDeployInfo.work_mode = EN_OIL_MODE;
    }    

    gstDeployInfo.charging_input_type = gstUdpPcsPara.udp_pcs_hold.charging_input_type;

    high_value = (int32_t)gstUdpPcsPara.udp_pcs_hold.set_batt_chg_power[0];
    low_value = (int32_t)gstUdpPcsPara.udp_pcs_hold.set_batt_chg_power[1];
    combine_value = (high_value << 16) | low_value;
    gstDeployInfo.set_batt_chg_power = (float32)(-1 * combine_value);

    high_value = (int32_t)gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_power[0];
    low_value = (int32_t)gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_power[1];
    combine_value = (high_value << 16) | low_value;
    gstDeployInfo.set_batt_dsg_power = (float32)combine_value;

    gstDeployInfo.chg_enable = gstUdpPcsPara.udp_pcs_hold.pcs_chg_ctrl;
    gstDeployInfo.dsg_enable = gstUdpPcsPara.udp_pcs_hold.pcs_dsg_ctrl;
    gstDeployInfo.on_grid_reconnect_time =
        (uint32_t)gstUdpPcsPara.udp_pcs_hold.on_grid_reconnect_time * 1000U;

    high_value = (int32_t)gstUdpPcsPara.udp_pcs_hold.on_grid_power[0];
    low_value = (int32_t)gstUdpPcsPara.udp_pcs_hold.on_grid_power[1];
    combine_value = (high_value << 16) | low_value;
    gstDeployInfo.on_grid_pwr = (float32)(-1 * combine_value);

    high_value = (int32_t)gstUdpPcsPara.udp_pcs_hold.InvGenMaxPower[0];
    low_value = (int32_t)gstUdpPcsPara.udp_pcs_hold.InvGenMaxPower[1];
    combine_value = (high_value << 16) | low_value;
    gstDeployInfo.set_oil_power = (float32)combine_value;

    gstDeployInfo.antireverseflow_enable = gstUdpPcsPara.udp_pcs_hold.anti_reverse_flow_enable;
    gstDeployInfo.pv_feekback_pwr = PvPwrRead();
    gstDeployInfo.backup_feekback_activerpwr = -1.0f * gstPh3CalcInfo.load_power.total_active;
    gstDeployInfo.ct_direction = gstUdpPcsPara.udp_pcs_hold.ct_direction;
}

void PwrCtrl_Set_EnergyCtrlInput(EnergyCtrlInput_t* p_input)
{
    if(p_input == NULL)
    {
        return;
    }
    memset(p_input, 0, sizeof(EnergyCtrlInput_t));
    if(gstPcsInvRunState == INV_STATE_STANDBY)
    {
        p_input->inv_stat = EN_STAND_BY_MODE;
    }
    else if(gstPcsInvRunState == INV_STATE_DISCONNECTED)
    {
        p_input->inv_stat = EN_OFF_GRID_MODE;
    }
    else if((gstPcsInvRunState == INV_STATE_GRID_CONNECTING)
            || (gstPcsInvRunState == INV_STATE_GEN_CONNECTING))
    {
        p_input->inv_stat = EN_OFF_GRID_TO_GRID_CONNECT_MODE;
    }
    else if((gstPcsInvRunState == INV_STATE_GRID_CONNECTED)
            || (gstPcsInvRunState == INV_STATE_GEN_CONNECTED))
    {
        p_input->inv_stat = EN_GRID_CONNECT_MODE;
    }
    else if((gstPcsInvRunState == INV_STATE_GRID_DISCONNECTING)
            || (gstPcsInvRunState == INV_STATE_GEN_DISCONNECTING))
    {
        p_input->inv_stat = EN_GRID_CONNECT_TO_OFF_GRID_MODE;
    }

    p_input->work_mode = (EN_WORK_MODE)gstDeployInfo.work_mode;

    if(gstDeployInfo.chg_enable == 1U)
    {
        p_input->peak_shift_stat = 1U;
    }
    else if(gstDeployInfo.dsg_enable == 1U)
    {
        p_input->peak_shift_stat = 2U;
    }

    p_input->antireverseflow_enable = (uint8_t)gstDeployInfo.antireverseflow_enable;
    p_input->power_max_household_get = fabsf(gstDeployInfo.on_grid_pwr);
    p_input->power_max_ongrid_get = fabsf(gstDeployInfo.on_grid_pwr);
    p_input->power_max_gen_get = fabsf(gstDeployInfo.set_oil_power);
    p_input->power_max_bat_chg_get = fabsf(gstDeployInfo.set_batt_chg_power);
    p_input->power_max_bat_dsg_get = fabsf(gstDeployInfo.set_batt_dsg_power);
    p_input->power_pv2bus_get = PvPwrRead();
    p_input->power_bat2bus_get = gstPh3CalcInfo.bus.power_filter;

    p_input->power_act_ct2grid_get = gstPh3CalcInfo.ct_power.total_active;
    p_input->power_act_norload_get =
        gstPh3CalcInfo.ct_power.total_active - gstPh3CalcInfo.grid_power.total_active;
    p_input->power_act_grid2pcs_get = gstPh3CalcInfo.grid_power.total_active;
    p_input->power_act_inv2pcs_get = gstPh3CalcInfo.inv_power.total_active;
    p_input->power_act_gen2pcs_get = gstPh3CalcInfo.gen_power.total_active;
    p_input->power_act_eps_get = fabsf(gstPh3CalcInfo.load_power.total_active);

    p_input->power_react_ct2grid_get = gstPh3CalcInfo.ct_power.total_reactive;
    p_input->power_react_norload_get =
        gstPh3CalcInfo.ct_power.total_reactive - gstPh3CalcInfo.grid_power.total_reactive;
    p_input->power_react_grid2pcs_get = gstPh3CalcInfo.grid_power.total_reactive;
    p_input->power_react_inv2pcs_get = gstPh3CalcInfo.inv_power.total_reactive;
    p_input->power_react_gen2pcs_get = gstPh3CalcInfo.gen_power.total_reactive;
    p_input->power_react_eps_get = gstPh3CalcInfo.load_power.total_reactive;

    p_input->power_appar_ct2grid_get = gstPh3CalcInfo.ct_power.total_apperant;
    p_input->power_appar_norload_get =
        fabsf(gstPh3CalcInfo.ct_power.total_apperant - gstPh3CalcInfo.grid_power.total_apperant);
    p_input->power_appar_grid2pcs_get = gstPh3CalcInfo.grid_power.total_apperant;
    p_input->power_appar_inv2pcs_get = gstPh3CalcInfo.inv_power.total_apperant;
    p_input->power_appar_gen2pcs_get = gstPh3CalcInfo.gen_power.total_apperant;
    p_input->power_appar_eps_get = fabsf(gstPh3CalcInfo.load_power.total_apperant);

}

void PwrCtrl_Set_DataInput(void)
{
    gstEnergyManageData.grid_enable = (gstPcsInvRunState == INV_STATE_GRID_CONNECTED);
    gstEnergyManageData.oil_enable =  (gstPcsInvRunState == INV_STATE_GEN_CONNECTED);
    gstEnergyManageData.drm_state_bits = gstUdpPcsPara.udp_pcs_hold.drm_state;
    gstEnergyManageData.inv_enable = ((BizFsm_Interface_GetRunState() == EN_RUN_STATE_DEF_NOMAL_RUN) ||\
                                     (BizFsm_Interface_GetRunState() == EN_RUN_STATE_DEF_TEST_MODE)) ;
    gstEnergyManageData.ctr_temp = gstAdcResult.ctr_temp;
    gstEnergyManageData.inv_temp = gstAdcResult.inv_temp;
    gstEnergyManageData.pv_temp = gstAdcResult.pv_temp;
    gstEnergyManageData.inv_volt_rms = gstPh3CalcInfo.inv_vol.a_phase_rms;
    gstEnergyManageData.inv_run_state = _PwrCtrl_MapInvPhase(gstPcsInvRunState);
    gstEnergyManageData.now_ms = TimGetSystime();
    gstEnergyManageData.reconnect_ms = 10000;
    gstEnergyManageData.connect_timeout_ms = 10000;
    gstEnergyManageData.bus_voltage = gstAdcResult.bus_volt_neg + gstAdcResult.bus_volt_pos;
    gstEnergyManageData.load_total_active_pwr = gstPh3CalcInfo.load_power.total_active;
    gstEnergyManageData.max_pwr_slope = INV_PWR_SET_MAX_PWR_SLOPE_W;
}

void EnergyManagementDataUpdate(void)
{
    PwrCtrl_Set_DeployFromComm();
    PwrCtrl_Set_EnergyCtrlInput(&gstEnergyCtrlInputData);
    PwrCtrl_Set_DataInput();
}
