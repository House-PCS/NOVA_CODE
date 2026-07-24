/**
 * @file pwr_ctrl_func.c
 * @brief 功率控制功能：调度逻辑与继电器控制
 */

#include "pwr_ctrl_func.h"
#include "pwr_ctrl_interface.h"
#include "energy_manage.h"
#include "temp_pwr_derate.h"
#include "inv_pwr_set.h"
#include "drm_pwr_sched.h"
#include "grid_connect_bus_pwr_limit.h"
#include "grid_connect_time_ctrl.h"
#include "energy_ctrl.h"
#include "cla_cpu_share.h"
#include <string.h>
#include "relay_ctrl.h"
#include <math.h>
#include <algo_interface.h>
extern EnergyCtrlInput_t gstEnergyCtrlInputData;

/* ========================================================================== */
/*  对外接口                                                                   */
/* ========================================================================== */

void PwrCtrlInterface_Init(void)
{
    memset(&gstDeployInfo, 0, sizeof(gstDeployInfo));
    memset(&gstEnergyManageData, 0, sizeof(gstEnergyManageData));
    memset(&gstEnergyManagePwrOut, 0, sizeof(gstEnergyManagePwrOut));

    gstEnergyManageData.reconnect_ms = 10000U;
    gstEnergyManageData.connect_timeout_ms = 10000U;
    gstEnergyManageData.max_pwr_slope = INV_PWR_SET_MAX_PWR_SLOPE_W;
    gstEnergyManageData.inv_run_state = GRID_CONNECT_TIME_INV_OTHER;

    GridConnectTimeCtrl_Init();
    InvPwrSet_Init();
    DrmPwrSched_Init();
    GridBusPwrLimit_Init();
    TempPwrDerate_Init();
}

void EnergyScheduling(void)
{
    static uint16_t sched_cht_div = 0;
    float32 bus_pwr_derate_in = 0;
    float32 inv_abs_pwr = 0;
    float32 pv_pwr_set = 0;
    float32 inv_active_pwr_sign = 1;
    float32 inv_active_pwr_set = 0 ;
    float32 connect_active_set = 0;

    sched_cht_div ++;
    if(sched_cht_div % 10 == 0)
    {
        EnergyManage_Run(&gstEnergyCtrlInputData);
//        gstEnergyManagePwrOut.gen_pwr
//        gstEnergyManagePwrOut.grid_active_pwr
//        gstEnergyManagePwrOut.pv_pwr
    }
    if(sched_cht_div % 15 == 0)
    {
        GridConnectTimeCtrl_Update();
    }
    if(sched_cht_div % 20 == 0)
    {
        TempPwrDerate_Update();
//        gstTempPwrDerateOut.pv_pwr
//        gstTempPwrDerateOut.inv_active_pwr
//        gstTempPwrDerateOut.disconnect_volt_ref
    }
    if(sched_cht_div % 100 == 0)
    {
        DrmPwrSched_Update();
//        gstDrmPwrSchedOut.input_rated_apparent_pwr
//        gstDrmPwrSchedOut.output_rated_apparent_pwr
    }

    if(gstEnergyManageData.inv_enable && gstEnergyManageData.grid_enable)
    {
        bus_pwr_derate_in = gstEnergyManagePwrOut.grid_active_pwr;
    }
    else if(gstEnergyManageData.inv_enable && gstEnergyManageData.oil_enable)
    {
        bus_pwr_derate_in = gstEnergyManagePwrOut.gen_pwr;
    }
    GridBusPwrLimit_Update(bus_pwr_derate_in);                  //母线降额运行
    /*******************************PV功率设置***********************************/
    if(gstEnergyManagePwrOut.pv_pwr >= gstTempPwrDerateOut.pv_pwr)
    {
        pv_pwr_set = gstTempPwrDerateOut.pv_pwr;
    }
    else
    {
        pv_pwr_set = gstEnergyManagePwrOut.pv_pwr;
    }
    pv_pwr_set = POWER_VALUE_LIMIT(pv_pwr_set,0,PV_MAX_PWR_W);

    if(sched_cht_div % 20 == 0)
    {
        MpptSetPvPwr(pv_pwr_set);
    }
    gstPwrCtrlInterface.pv_pwr = pv_pwr_set;
    /*******************************逆变功率设置***********************************/
    if(fabsf(gstGridBusPwrLimitOut.inv_pwr_limited) >= fabsf(gstTempPwrDerateOut.inv_active_pwr))
    {
        inv_abs_pwr = fabsf(gstTempPwrDerateOut.inv_active_pwr);
    }
    else
    {
        inv_abs_pwr = fabsf(gstGridBusPwrLimitOut.inv_pwr_limited);
    }
    inv_active_pwr_sign = ((gstGridBusPwrLimitOut.inv_pwr_limited >= 0) ? 1.0f : -1.0f);
    inv_active_pwr_set = inv_abs_pwr * inv_active_pwr_sign;
    inv_active_pwr_set = POWER_VALUE_LIMIT(inv_active_pwr_set,-INV_ON_GRID_MAX_ACTIVE_PWR_W,INV_ON_GRID_MAX_ACTIVE_PWR_W);

    gstPwrCtrlInterface.inv_active_pwr = inv_active_pwr_set;
    gstPwrCtrlInterface.inv_reactive_pwr = 0;
    /*******************************电网及油机功率设置***********************************/
    if(gstEnergyManageData.inv_enable && gstEnergyManageData.grid_enable)
    {
        connect_active_set = GridBusPwrLimit_GridPwr(gstPwrCtrlInterface.inv_active_pwr, gstEnergyManageData.load_total_active_pwr);
        connect_active_set = POWER_VALUE_LIMIT(connect_active_set,-GRID_OUT_MAX_ACTIVE_PWR_W,GRID_IN_MAX_ACTIVE_PWR_W);
    }
    else if(gstEnergyManageData.inv_enable && gstEnergyManageData.oil_enable)
    {
        connect_active_set = GridBusPwrLimit_GridPwr(gstPwrCtrlInterface.inv_active_pwr, gstEnergyManageData.load_total_active_pwr);
        connect_active_set = POWER_VALUE_LIMIT(connect_active_set,0,OIL_MAX_ACTIVE_PWR_W);
    }
    gstPwrCtrlInterface.connect_active_pwr = connect_active_set;
    gstPwrCtrlInterface.connect_reactive_pwr = 0;
    /*******************************电压幅值给定***********************************/
    if(sched_cht_div % 20 == 0)
    {
        gstPwrCtrlInterface.off_grid_inv_ref = gstTempPwrDerateOut.disconnect_volt_ref;
    }
    /**********************************逆变及电网功率斜坡给定***********************************/
    InvPwrSet_Update(gstPwrCtrlInterface.inv_active_pwr,gstPwrCtrlInterface.inv_reactive_pwr ,\
                     gstPwrCtrlInterface.connect_active_pwr,gstPwrCtrlInterface.connect_reactive_pwr);

    if(gstEnergyManageData.inv_enable && gstEnergyManageData.grid_enable)
    {
        gstPwrCtrlInterface.ramp_connect_active_pwr = \
                POWER_VALUE_LIMIT(gstGridPwrSetOut.p_power,-GRID_OUT_MAX_ACTIVE_PWR_W,GRID_IN_MAX_ACTIVE_PWR_W);
        gstPwrCtrlInterface.ramp_connect_reactive_pwr = \
                POWER_VALUE_LIMIT(gstGridPwrSetOut.q_power,-GRID_IN_MAX_REACTIVE_PWR_VAR,GRID_IN_MAX_REACTIVE_PWR_VAR);
    }
    else if(gstEnergyManageData.inv_enable && gstEnergyManageData.oil_enable)
    {
        gstPwrCtrlInterface.ramp_connect_active_pwr = \
                POWER_VALUE_LIMIT(gstGridPwrSetOut.p_power,-OIL_MAX_ACTIVE_PWR_W,OIL_MAX_ACTIVE_PWR_W);
        gstPwrCtrlInterface.ramp_connect_reactive_pwr = \
                POWER_VALUE_LIMIT(gstGridPwrSetOut.q_power,-OIL_MAX_REACTIVE_PWR_VAR,OIL_MAX_REACTIVE_PWR_VAR);
    }
    gstPwrCtrlInterface.ramp_inv_active_pwr = \
            POWER_VALUE_LIMIT(gstInvPwrSetOut.p_power,-INV_ON_GRID_MAX_ACTIVE_PWR_W,INV_ON_GRID_MAX_ACTIVE_PWR_W);
    gstPwrCtrlInterface.ramp_inv_reactive_pwr = \
            POWER_VALUE_LIMIT(gstInvPwrSetOut.q_power,-INV_ON_GRID_MAX_REACTIVE_PWR_VAR,INV_ON_GRID_MAX_REACTIVE_PWR_VAR);
    sched_cht_div = ((sched_cht_div >= 100) ? 0 : sched_cht_div);
}

