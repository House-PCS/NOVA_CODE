/**
 * @file grid_connect_time_ctrl.c
 * @brief 并网时间控制：离网冷却与并网超时许可
 */

#include "grid_connect_time_ctrl.h"
#include "cpu_timers.h"
GridConnectTimeCtrl_Output_t gstGridConnectTimeCtrlOut;

/* ========================================================================== */
/*  模块私有变量                                                               */
/* ========================================================================== */

static uint32_t _gCooldownMs;
static uint32_t _gConnectMs;
static uint16_t _gLastInvRunState;

/* ========================================================================== */
/*  对外接口                                                                   */
/* ========================================================================== */

void GridConnectTimeCtrl_Init(void)
{
    _gCooldownMs = 0;
    _gConnectMs = 0;
    _gLastInvRunState = GRID_CONNECT_TIME_INV_OTHER;
    gstGridConnectTimeCtrlOut.grid_enable = 0U;
}

void GridConnectTimeCtrl_Update(void)
{
    uint16_t grid_enable = 0U;
    uint32_t now_ms;

    now_ms = gstEnergyManageData.now_ms;

    if((gstEnergyManageData.inv_run_state == GRID_CONNECT_TIME_INV_CONNECTING)
       && (_gLastInvRunState != GRID_CONNECT_TIME_INV_CONNECTING))
    {
        _gConnectMs = now_ms;
    }
    if((gstEnergyManageData.inv_run_state == GRID_CONNECT_TIME_INV_DISCONNECTING)
       && (_gLastInvRunState != GRID_CONNECT_TIME_INV_DISCONNECTING))
    {
        _gCooldownMs = now_ms;
    }

    if(gstEnergyManageData.inv_run_state == GRID_CONNECT_TIME_INV_ON_GRID)
    {
        grid_enable = 1U;
        _gCooldownMs = now_ms;
    }
    else if(gstEnergyManageData.inv_run_state == GRID_CONNECT_TIME_INV_DISCONNECTING)
    {
        grid_enable = 0U;
    }
    else if(gstEnergyManageData.inv_run_state == GRID_CONNECT_TIME_INV_CONNECTING)
    {
        grid_enable = (TimDifftime(_gConnectMs, now_ms) <= gstEnergyManageData.connect_timeout_ms)
                          ? 1U
                          : 0U;
        if(grid_enable == 0U)
        {
            _gCooldownMs = now_ms;
        }
    }
    else if(gstEnergyManageData.inv_run_state == GRID_CONNECT_TIME_INV_OFFGRID)
    {
        if(TimDifftime(_gCooldownMs, now_ms) > gstEnergyManageData.reconnect_ms)
        {
            grid_enable = 1U;
            _gConnectMs = now_ms;
        }
    }

    _gLastInvRunState = gstEnergyManageData.inv_run_state;
    gstGridConnectTimeCtrlOut.grid_enable = grid_enable;
}
