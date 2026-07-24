/**
 * @file inv_pwr_set.c
 * @brief 逆变功率设定：斜坡、限幅与并网 P/Q 输出
 */

#include "inv_pwr_set.h"
#include <math.h>

InvPwrSet_Output_t gstInvPwrSetOut;
InvPwrSet_Output_t gstGridPwrSetOut;
/* ========================================================================== */
/*  模块私有类型与变量                                                         */
/* ========================================================================== */

typedef struct inv_pwr_set_ctrl_stru_t
{
    float32 ramp_p;
    float32 ramp_q;
} InvPwrSet_Ctrl_t;

static InvPwrSet_Ctrl_t _gInvPwrSet;
static InvPwrSet_Ctrl_t _gGridPwrSet;
/* ========================================================================== */
/*  模块私有函数                                                               */
/* ========================================================================== */

static float32 _InvPwrSet_PowerRampUpdate(float32 current, float32 max_slope, float32 target)
{
    float32 delta = fabsf(target) - fabsf(current);

    if(((current >= 0.0f) && (target >= 0.0f)) || ((current <= 0.0f) && (target <= 0.0f)))
    {
        if(fabsf(delta) <= INV_PWR_SET_RAMP_DEADBAND_W)
        {
            return target;
        }
        if(fabsf(delta) <= max_slope)
        {
            return target;
            }
        if(delta > 0.0f)
        {
            return current + ((target > 0.0f) ? max_slope : -max_slope);
        }
        return target;
    }

    return 0.0f;
}

static void _InvPwrSet_ClampPwr(float32 active_pwr,
                                float32 reactive_pwr,
                                float32 p_min_limit,
                                float32 p_max_limit,
                                float32 q_min_limit,
                                float32 q_max_limit,
                                InvPwrSet_Output_t* p_output)
{
    if(active_pwr >= p_max_limit)
    {
        active_pwr = p_max_limit;
    }
    else if(active_pwr <= p_min_limit)
    {
        active_pwr = p_min_limit;
    }

    if(reactive_pwr >= q_max_limit)
    {
        reactive_pwr = q_max_limit;
    }
    else if(reactive_pwr <= q_min_limit)
    {
        reactive_pwr = q_min_limit;
    }

    p_output->p_power = active_pwr;
    p_output->q_power = reactive_pwr;
}

static void _PwrSet_RampUpdate(InvPwrSet_Ctrl_t *pwr_set,float32 active_pwr, float32 reactive_pwr)
{
    pwr_set->ramp_p = _InvPwrSet_PowerRampUpdate(pwr_set->ramp_p,
                                                     gstEnergyManageData.max_pwr_slope,
                                                     active_pwr);
    pwr_set->ramp_q = _InvPwrSet_PowerRampUpdate(pwr_set->ramp_q,
                                                     gstEnergyManageData.max_pwr_slope,
                                                     reactive_pwr);
}

/* ========================================================================== */
/*  对外接口                                                                   */
/* ========================================================================== */

void InvPwrSet_Init(void)
{
    _gInvPwrSet.ramp_p = 0.0f;
    _gInvPwrSet.ramp_q = 0.0f;
    gstInvPwrSetOut.p_power = 0.0f;
    gstInvPwrSetOut.q_power = 0.0f;

    _gGridPwrSet.ramp_p = 0.0f;
    _gGridPwrSet.ramp_q = 0.0f;
    gstGridPwrSetOut.p_power = 0.0f;
    gstGridPwrSetOut.q_power = 0.0f;
}

void InvPwrSet_Update(float32 inv_active_pwr, float32 inv_reactive_pwr,\
                      float32 connect_active_pwr, float32 connect_reactive_pwr)
{
    if(gstEnergyManageData.grid_enable == 0U && gstEnergyManageData.oil_enable == 0U)
    {
        InvPwrSet_Init();
        return;
    }
    _PwrSet_RampUpdate(&_gInvPwrSet, inv_active_pwr, inv_reactive_pwr);
    _InvPwrSet_ClampPwr(_gInvPwrSet.ramp_p,
                         _gInvPwrSet.ramp_q,
                         -INV_ON_GRID_MAX_ACTIVE_PWR_W,
                         INV_ON_GRID_MAX_ACTIVE_PWR_W,
                         -INV_ON_GRID_MAX_REACTIVE_PWR_VAR,
                         INV_ON_GRID_MAX_REACTIVE_PWR_VAR,
                         &gstInvPwrSetOut);

    _PwrSet_RampUpdate(&_gGridPwrSet, connect_active_pwr, connect_reactive_pwr);
    _InvPwrSet_ClampPwr(_gGridPwrSet.ramp_p,
                         _gGridPwrSet.ramp_q,
                         -GRID_OUT_MAX_ACTIVE_PWR_W,
                         GRID_IN_MAX_ACTIVE_PWR_W,
                         -GRID_IN_MAX_REACTIVE_PWR_VAR,
                         GRID_IN_MAX_REACTIVE_PWR_VAR,
                         &gstGridPwrSetOut);

}
