/**
 * @file drm_pwr_sched.c
 * @brief DRM 功率调度
 */

#include "drm_pwr_sched.h"

DrmPwrSched_Output_t gstDrmPwrSchedOut;
/* ========================================================================== */
/*  模块私有函数                                                                  */
/* ========================================================================== */

static void _DrmPwrSched_SetRatedApparentPwr(float32 input_pwr, float32 output_pwr)
{
    gstDrmPwrSchedOut.input_rated_apparent_pwr = input_pwr;
    gstDrmPwrSchedOut.output_rated_apparent_pwr = output_pwr;
}

static DrmPwrSched_State_t _DrmPwrSched_ResolveState(uint16_t drm_state_bits)
{
    uint16_t drm_mask = drm_state_bits & 0x01FFU;
    uint8_t active_count = 0U;
    uint8_t i;
    DrmPwrSched_State_t drm_level = EN_DRM_DRM8;

    if(drm_mask == 0U)
    {
        return EN_DRM_DRM8;
    }

    for(i = 0U; i < 9U; i++)
    {
        if((drm_mask & (0x0001U << i)) != 0U)
        {
            active_count++;
            drm_level = (DrmPwrSched_State_t)i;
        }
    }

    if(active_count >= 2U)
    {
        return EN_DRM_DRM0;
    }

    return drm_level;
}

static void _DrmPwrSched_ApplyLevel(DrmPwrSched_State_t drm_level)
{
    float32 s_in_max = GRID_IN_MAX_APPARENT_PWR_VA;
    float32 s_out_max = GRID_OUT_MAX_APPARENT_PWR_VA;

    switch(drm_level)
    {
    case EN_DRM_DRM0:
        _DrmPwrSched_SetRatedApparentPwr(0.0f, 0.0f);
        break;
    case EN_DRM_DRM1:
        _DrmPwrSched_SetRatedApparentPwr(0.0f, s_out_max);
        break;
    case EN_DRM_DRM2:
        _DrmPwrSched_SetRatedApparentPwr(0.50f * s_in_max, s_out_max);
        break;
    case EN_DRM_DRM3:
        _DrmPwrSched_SetRatedApparentPwr(0.75f * s_in_max, s_out_max);
        break;
    case EN_DRM_DRM4:
        _DrmPwrSched_SetRatedApparentPwr(s_in_max, s_out_max);
        break;
    case EN_DRM_DRM5:
        _DrmPwrSched_SetRatedApparentPwr(s_in_max, 0.0f);
        break;
    case EN_DRM_DRM6:
        _DrmPwrSched_SetRatedApparentPwr(s_in_max, 0.50f * s_out_max);
        break;
    case EN_DRM_DRM7:
        _DrmPwrSched_SetRatedApparentPwr(s_in_max, 0.75f * s_out_max);
        break;
    case EN_DRM_DRM8:
        _DrmPwrSched_SetRatedApparentPwr(s_in_max, s_out_max);
        break;
    default:
        _DrmPwrSched_SetRatedApparentPwr(0.0f, 0.0f);
        break;
    }
    if(drm_level == EN_DRM_DRM0)
    {
        gstDrmPwrSchedOut.drm_relay_control = 0U;
    }
    else
    {
        gstDrmPwrSchedOut.drm_relay_control = 1U;
    }
}

/* ========================================================================== */
/*  对外接口                                                                   */
/* ========================================================================== */

void DrmPwrSched_Init(void)
{
    _DrmPwrSched_SetRatedApparentPwr(GRID_IN_MAX_APPARENT_PWR_VA, GRID_OUT_MAX_APPARENT_PWR_VA);
    gstDrmPwrSchedOut.drm_relay_control = 0U;
}

void DrmPwrSched_Update(void)
{
    DrmPwrSched_State_t drm_level;

    if(gstEnergyManageData.inv_enable == 0U)
    {
        DrmPwrSched_Init();
        return;
    }
    drm_level = _DrmPwrSched_ResolveState(gstEnergyManageData.drm_state_bits);
    _DrmPwrSched_ApplyLevel(drm_level);
}
