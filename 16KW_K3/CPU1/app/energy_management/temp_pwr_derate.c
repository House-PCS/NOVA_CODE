/**
 * @file temp_pwr_derate.c
 * @brief 温度降额：光伏 / 逆变 / 控制板功率限制
 */

#include "temp_pwr_derate.h"
#include <math.h>

TempPwrDerate_Output_t gstTempPwrDerateOut;

/* ========================================================================== */
/*  模块私有变量                                                               */
/* ========================================================================== */

static float32 _gPvTempFilt;
static float32 _gInvTempFilt;
static float32 _gCtrTempFilt;

/* ========================================================================== */
/*  模块私有函数                                                               */
/* ========================================================================== */

static float32 _TempPwrDerate_LinearDerateHighTemp(float32 temp,
                                                   float32 t_start,
                                                   float32 t_stop,
                                                   float32 max_pwr)
{
    float32 slope;
    float32 diff;

    if(temp > t_stop)
    {
        return 0.0f;
    }
    if(temp <= t_start)
    {
        return max_pwr;
    }

    slope = max_pwr / (t_stop - t_start);
    diff = temp - t_start;
    return max_pwr - diff * slope;
}

static float32 _TempPwrDerate_PvTempPwrDerate(float32 pv_temp)
{
    float32 pv_pwr;

    pv_pwr = _TempPwrDerate_LinearDerateHighTemp(pv_temp,
                                                 TEMP_PWR_DERATE_PV_TEMP_START,
                                                 TEMP_PWR_DERATE_PV_TEMP_STOP,
                                                 PV_MAX_PWR_W);
    if(pv_pwr < 0.0f)
    {
        pv_pwr = 0.0f;
    }

    return pv_pwr;
}

static float32 _TempPwrDerate_InvCtrTempPwrDerate(float32 inv_temp,
                                                  float32 ctr_temp,
                                                  float32 max_pwr)
{
    float32 inv_pwr = max_pwr;
    float32 ctr_pwr = max_pwr;
    uint16_t inv_limited = 0U;
    uint16_t ctr_limited = 0U;

    if(inv_temp > TEMP_PWR_DERATE_INV_TEMP_START && inv_temp <= TEMP_PWR_DERATE_INV_TEMP_STOP)
    {
        inv_pwr = _TempPwrDerate_LinearDerateHighTemp(inv_temp,
                                                      TEMP_PWR_DERATE_INV_TEMP_START,
                                                      TEMP_PWR_DERATE_INV_TEMP_STOP,
                                                      max_pwr);
        inv_limited = 1U;
    }

    if(ctr_temp > TEMP_PWR_DERATE_CTR_TEMP_START && ctr_temp <= TEMP_PWR_DERATE_CTR_TEMP_STOP)
    {
        ctr_pwr = _TempPwrDerate_LinearDerateHighTemp(ctr_temp,
                                                      TEMP_PWR_DERATE_CTR_TEMP_START,
                                                      TEMP_PWR_DERATE_CTR_TEMP_STOP,
                                                      max_pwr);
        ctr_limited = 1U;
    }

    if(inv_limited != 0U && ctr_limited != 0U)
    {
        return (inv_pwr < ctr_pwr) ? inv_pwr : ctr_pwr;
    }
    if(inv_limited != 0U)
    {
        return inv_pwr;
    }
    if(ctr_limited != 0U)
    {
        return ctr_pwr;
    }

    return max_pwr;
}

static float32 _TempPwrDerate_OffGridVoltFromTempPwrDerate(float32 pwr_derate,
                                                           float32 volt_rms_ref,
                                                           float32 load_total_active_pwr)
{
    float32 vol_rms = (volt_rms_ref > 0.0f) ? volt_rms_ref : INV_RATED_VOLT_RMS_V;
    float32 single_pwr_ref = pwr_derate / TEMP_PWR_DERATE_PHASE_NUM;
    float32 single_pwr_act = load_total_active_pwr / TEMP_PWR_DERATE_PHASE_NUM;
    float32 actual_resister;
    float32 vol_rms_out;

    if(single_pwr_act < 0.0f)
    {
        single_pwr_act = -single_pwr_act;
    }
    if(single_pwr_act < 1.0f)
    {
        single_pwr_act = 1.0f;
    }

    actual_resister = (vol_rms * vol_rms) / single_pwr_act;
    vol_rms_out = sqrtf(single_pwr_ref * actual_resister);
    if(vol_rms_out < INV_MIN_VOLT_RMS_V)
    {
        vol_rms_out = INV_MIN_VOLT_RMS_V;
    }
    if(vol_rms_out > INV_RATED_VOLT_RMS_V)
    {
        vol_rms_out = INV_RATED_VOLT_RMS_V;
    }
    return vol_rms_out;
}

static float32 _TempPwrDerate_ClampF32(float32 val, float32 min_val, float32 max_val)
{
    if(val > max_val)
    {
        return max_val;
    }
    if(val < min_val)
    {
        return min_val;
    }
    return val;
}

static void _TempPwrDerate_ClampOutput(TempPwrDerate_Output_t* p_output)
{
    p_output->pv_pwr = _TempPwrDerate_ClampF32(p_output->pv_pwr,
                                               0,
                                               TEMP_PWR_DERATE_PV_PWR_MAX_W);

    p_output->inv_active_pwr = _TempPwrDerate_ClampF32(p_output->inv_active_pwr,
                                                       0,
                                                       TEMP_PWR_DERATE_INV_PWR_GRID_MAX_W);
    if(gstEnergyManageData.inv_enable == 1U && gstEnergyManageData.grid_enable == 0U &&\
        gstEnergyManageData.oil_enable == 0U)
    {
        p_output->disconnect_volt_ref =
            _TempPwrDerate_ClampF32(p_output->disconnect_volt_ref,
                                    TEMP_PWR_DERATE_DISCONNECT_VOLT_MIN_V,
                                    TEMP_PWR_DERATE_DISCONNECT_VOLT_MAX_V);
    }
}

static void _TempPwrDerate_InvOffGrid(float32 inv_temp,
                                      float32 ctr_temp,
                                      TempPwrDerate_Output_t* p_output)
{
    float32 inv_pwr;
    float32 volt_ref;

    if(inv_temp > TEMP_PWR_DERATE_INV_TEMP_STOP || ctr_temp > TEMP_PWR_DERATE_CTR_TEMP_STOP)
    {
        inv_pwr = TEMP_PWR_DERATE_INV_DISCONNECT_MIN_PWR;
    }
    else
    {
        inv_pwr = _TempPwrDerate_InvCtrTempPwrDerate(inv_temp,
                                                     ctr_temp,
                                                     INV_OFFGRID_MAX_ACTIVE_PWR_W);
        if(inv_pwr < TEMP_PWR_DERATE_INV_DISCONNECT_MIN_PWR)
        {
            inv_pwr = TEMP_PWR_DERATE_INV_DISCONNECT_MIN_PWR;
        }
    }

    p_output->inv_active_pwr = inv_pwr;

    if((inv_temp <= TEMP_PWR_DERATE_INV_TEMP_START)
       && (ctr_temp <= TEMP_PWR_DERATE_CTR_TEMP_START))
    {
        volt_ref = TEMP_PWR_DERATE_DISCONNECT_VOLT_RMS_V;
    }
    else
    {
        volt_ref = _TempPwrDerate_OffGridVoltFromTempPwrDerate(inv_pwr,
                                                               gstEnergyManageData.inv_volt_rms,
                                                               gstEnergyManageData.load_total_active_pwr);
    }

    p_output->disconnect_volt_ref = volt_ref;
}

static void _TempPwrDerate_InvGridConnected(float32 inv_temp,
                                            float32 ctr_temp,
                                            TempPwrDerate_Output_t* p_output)
{
    float32 inv_pwr;

    if(inv_temp > TEMP_PWR_DERATE_INV_TEMP_STOP || ctr_temp > TEMP_PWR_DERATE_CTR_TEMP_STOP)
    {
        p_output->inv_active_pwr = 0.0f;
        return;
    }

    inv_pwr = _TempPwrDerate_InvCtrTempPwrDerate(inv_temp,
                                                 ctr_temp,
                                                 INV_ON_GRID_MAX_ACTIVE_PWR_W);
    if(inv_pwr < 0.0f)
    {
        inv_pwr = 0.0f;
    }

    p_output->inv_active_pwr = inv_pwr;
}

/* ========================================================================== */
/*  对外接口                                                                   */
/* ========================================================================== */

void TempPwrDerate_Init(void)
{
    _gPvTempFilt = 0.0f;
    _gInvTempFilt = 0.0f;
    _gCtrTempFilt = 0.0f;

    gstTempPwrDerateOut.pv_pwr = (float32)PV_MAX_PWR_W;
    gstTempPwrDerateOut.inv_active_pwr = INV_ON_GRID_MAX_ACTIVE_PWR_W;
    gstTempPwrDerateOut.disconnect_volt_ref = TEMP_PWR_DERATE_DISCONNECT_VOLT_RMS_V;
}

void TempPwrDerate_Update(void)
{
    if(gstEnergyManageData.inv_enable == 0U )
    {   
        gstTempPwrDerateOut.pv_pwr = 0.0f;
        gstTempPwrDerateOut.inv_active_pwr = 0.0f;
        gstTempPwrDerateOut.disconnect_volt_ref = TEMP_PWR_DERATE_DISCONNECT_VOLT_RMS_V;
        return;
    }

    _gPvTempFilt = gstEnergyManageData.pv_temp;
    _gInvTempFilt = gstEnergyManageData.inv_temp;
    _gCtrTempFilt = gstEnergyManageData.ctr_temp;
    gstTempPwrDerateOut.pv_pwr = _TempPwrDerate_PvTempPwrDerate(_gPvTempFilt);
    gstTempPwrDerateOut.inv_active_pwr = 0.0f;
    gstTempPwrDerateOut.disconnect_volt_ref = TEMP_PWR_DERATE_DISCONNECT_VOLT_RMS_V;

    if(gstEnergyManageData.grid_enable  ||  gstEnergyManageData.oil_enable )
    {
        _TempPwrDerate_InvGridConnected(_gInvTempFilt, _gCtrTempFilt, &gstTempPwrDerateOut);
    }
    else 
    {
        _TempPwrDerate_InvOffGrid(_gInvTempFilt, _gCtrTempFilt, &gstTempPwrDerateOut);
    }

    _TempPwrDerate_ClampOutput(&gstTempPwrDerateOut);
}
