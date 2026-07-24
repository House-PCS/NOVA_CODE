/**
 * @file grid_connect_bus_pwr_limit.c
 * @brief 并网母线电压有功功率限制（四区 + PI，mid_sliding_filter）
 */

#include "grid_connect_bus_pwr_limit.h"
#include "mid_sliding_filter.h"

/* ========================================================================== */
/*  模块私有类型与变量                                                         */
/* ========================================================================== */

typedef struct grid_bus_pwr_ctrl_stru_t
{
    float32 busvol_protect_upper;
    float32 busvol_protect_lower;
    float32 busvol_theory_upper;
    float32 busvol_theory_lower;
    float32 kp;
    float32 ki;
    float32 integral;
    float32 bus_vol_error;
    float32 pwr_out;
    EN_GRID_BUS_PWR_CTRL ctrl_mode;
} GridBusPwrCtrl_t;

static float32 _gBusVoltFilterBuf[GRID_BUS_VOLT_FILTER_WIN_SIZE];
static Mid_SlidingFilter_t _gBusVoltFilter;
static GridBusPwrCtrl_t _gPwrCtrl;
GridBusPwrLimit_Output_t gstGridBusPwrLimitOut;
/* ========================================================================== */
/*  模块私有函数                                                               */
/* ========================================================================== */

static float32 _GridBusPwrLimit_InvPwrSet(float32 active_pwr_set, float32 load_active_pwr)
{
    return -1.0f * (active_pwr_set + load_active_pwr);
}

static float32 _GridBusPwrLimit_ClampInvPwr(float32 inv_pwr_set, float32 max_pwr, float32 min_pwr)
{
    if(inv_pwr_set > max_pwr)
    {
        return max_pwr;
    }
    if(inv_pwr_set < min_pwr)
    {
        return min_pwr;
    }
    return inv_pwr_set;
}

static void _GridBusPwrCtrl_Init(void)
{
    _gPwrCtrl.busvol_protect_upper = GRID_BUS_PWR_LIMIT_PROTECT_UPPER_V;
    _gPwrCtrl.busvol_protect_lower = GRID_BUS_PWR_LIMIT_PROTECT_LOWER_V;
    _gPwrCtrl.busvol_theory_upper = GRID_BUS_PWR_LIMIT_THEORY_UPPER_V;
    _gPwrCtrl.busvol_theory_lower = GRID_BUS_PWR_LIMIT_THEORY_LOWER_V;
    _gPwrCtrl.kp = GRID_BUS_PWR_LIMIT_KP;
    _gPwrCtrl.ki = GRID_BUS_PWR_LIMIT_KI;
    _gPwrCtrl.integral = 0.0f;
    _gPwrCtrl.bus_vol_error = 0.0f;
    _gPwrCtrl.pwr_out = 0.0f;
    _gPwrCtrl.ctrl_mode = EN_GRID_BUS_PWR_CTRL_NORMAL;
}

static void _GridBusPwrLimit_ResetRuntime(void)
{
    Mid_SlidingFilter_Init(&_gBusVoltFilter,
                           _gBusVoltFilterBuf,
                           (uint16_t)GRID_BUS_VOLT_FILTER_WIN_SIZE);
    _GridBusPwrCtrl_Init();
}

static void _GridBusPwrCtrl_ModeSelect(GridBusPwrCtrl_t* p_ctrl, float32 filtered_voltage)
{
    if((filtered_voltage > p_ctrl->busvol_theory_lower)
       && (filtered_voltage < p_ctrl->busvol_theory_upper))
    {
        p_ctrl->ctrl_mode = EN_GRID_BUS_PWR_CTRL_NORMAL;
    }
    else if(((filtered_voltage > p_ctrl->busvol_theory_upper)
              && (filtered_voltage < p_ctrl->busvol_protect_upper))
             || ((filtered_voltage < p_ctrl->busvol_theory_lower)
                 && (filtered_voltage > p_ctrl->busvol_protect_lower)))
    {
        p_ctrl->ctrl_mode = EN_GRID_BUS_PWR_CTRL_HOLD;
    }
    else if(filtered_voltage >= p_ctrl->busvol_protect_upper)
    {
        p_ctrl->ctrl_mode = EN_GRID_BUS_PWR_CTRL_PI_CHG;
    }
    else if(filtered_voltage <= p_ctrl->busvol_protect_lower)
    {
        p_ctrl->ctrl_mode = EN_GRID_BUS_PWR_CTRL_PI_DSG;
    }
    else
    {
        p_ctrl->ctrl_mode = EN_GRID_BUS_PWR_CTRL_HOLD;
    }
}

static void _GridBusPwrCtrl_HoldRun(GridBusPwrCtrl_t* p_ctrl, float32 inv_pwr_set)
{
    float32 out = p_ctrl->pwr_out;

    if((out > 0.0f) && (inv_pwr_set > 0.0f))
    {
        out = (out < inv_pwr_set) ? out : inv_pwr_set;
    }
    else if((out < 0.0f) && (inv_pwr_set < 0.0f))
    {
        out = (out > inv_pwr_set) ? out : inv_pwr_set;
    }
    else
    {
        out = inv_pwr_set;
    }

    p_ctrl->pwr_out = out;
    p_ctrl->integral = 0.0f;
}

static void _GridBusPwrCtrl_PiChgRun(GridBusPwrCtrl_t* p_ctrl, float32 filtered_voltage, float32 dt_s)
{
    float32 out;

    p_ctrl->bus_vol_error = p_ctrl->busvol_protect_upper - filtered_voltage;
    p_ctrl->integral += p_ctrl->bus_vol_error * dt_s;
    out = p_ctrl->pwr_out + p_ctrl->kp * p_ctrl->bus_vol_error + p_ctrl->ki * p_ctrl->integral;

    if(out > GRID_BUS_PWR_LIMIT_GRID_PWR_MAX_W)
    {
        out = GRID_BUS_PWR_LIMIT_GRID_PWR_MAX_W;
        if(p_ctrl->bus_vol_error < 0.0f)
        {
            p_ctrl->integral = 0.0f;
        }
    }
    else if(out < 0.0f)
    {
        out = 0.0f;
        if(p_ctrl->bus_vol_error < 0.0f)
        {
            p_ctrl->integral = 0.0f;
        }
    }

    p_ctrl->pwr_out = out;
}

static void _GridBusPwrCtrl_PiDsgRun(GridBusPwrCtrl_t* p_ctrl, float32 filtered_voltage, float32 dt_s)
{
    float32 out;

    p_ctrl->bus_vol_error = p_ctrl->busvol_protect_lower - filtered_voltage;
    p_ctrl->integral += p_ctrl->bus_vol_error * dt_s;
    out = p_ctrl->pwr_out + p_ctrl->kp * p_ctrl->bus_vol_error + p_ctrl->ki * p_ctrl->integral;

    if(out > 0.0f)
    {
        out = 0.0f;
        if(p_ctrl->bus_vol_error > 0.0f)
        {
            p_ctrl->integral = 0.0f;
        }
    }
    else if(out < GRID_BUS_PWR_LIMIT_INV_PWR_MIN_W)
    {
        out = GRID_BUS_PWR_LIMIT_INV_PWR_MIN_W;
        if(p_ctrl->bus_vol_error > 0.0f)
        {
            p_ctrl->integral = 0.0f;
        }
    }

    p_ctrl->pwr_out = out;
}

static void _GridBusPwrLimit_Calc(GridBusPwrCtrl_t* p_ctrl,
                                  float32 filtered_voltage,
                                  float32 inv_pwr_set,
                                  float32 dt_s)
{
    _GridBusPwrCtrl_ModeSelect(p_ctrl, filtered_voltage);

    switch(p_ctrl->ctrl_mode)
    {
    case EN_GRID_BUS_PWR_CTRL_NORMAL:
        p_ctrl->pwr_out = inv_pwr_set;
        p_ctrl->integral = 0.0f;
        break;

    case EN_GRID_BUS_PWR_CTRL_HOLD:
        _GridBusPwrCtrl_HoldRun(p_ctrl, inv_pwr_set);
        break;

    case EN_GRID_BUS_PWR_CTRL_PI_CHG:
        _GridBusPwrCtrl_PiChgRun(p_ctrl, filtered_voltage, dt_s);
        break;

    case EN_GRID_BUS_PWR_CTRL_PI_DSG:
        _GridBusPwrCtrl_PiDsgRun(p_ctrl, filtered_voltage, dt_s);
        break;

    default:
        p_ctrl->pwr_out = inv_pwr_set;
        break;
    }
}

/* ========================================================================== */
/*  对外接口                                                                   */
/* ========================================================================== */

float32 GridBusPwrLimit_GridPwr(float32 inv_pwr, float32 load_active_pwr)
{
    return (-inv_pwr) - load_active_pwr;
}

void GridBusPwrLimit_Init(void)
{
    _GridBusPwrLimit_ResetRuntime();
    gstGridBusPwrLimitOut.inv_pwr_limited = 0.0f;
}   

void GridBusPwrLimit_Update(float32 bus_pwr_derate_in)
{
    float32 filtered_voltage;
    float32 pwr_set;

    if((gstEnergyManageData.grid_enable == 0U && gstEnergyManageData.oil_enable == 0U) || gstEnergyManageData.inv_enable == 0U)
    {
        _GridBusPwrLimit_ResetRuntime();
        gstGridBusPwrLimitOut.inv_pwr_limited = 0.0f;
        return;
    }
    pwr_set = _GridBusPwrLimit_InvPwrSet(bus_pwr_derate_in, gstEnergyManageData.load_total_active_pwr);
    pwr_set = _GridBusPwrLimit_ClampInvPwr(pwr_set, GRID_BUS_PWR_LIMIT_GRID_PWR_MAX_W, GRID_BUS_PWR_LIMIT_INV_PWR_MIN_W);

    filtered_voltage = Mid_SlidingFilter_Update(&_gBusVoltFilter, gstEnergyManageData.bus_voltage);
    _GridBusPwrLimit_Calc(&_gPwrCtrl, filtered_voltage, pwr_set, GRID_BUS_PWR_LIMIT_CTRL_DT_S);

//    gstGridBusPwrLimitOut.inv_pwr_limited = GridBusPwrLimit_GridPwr(_gPwrCtrl.pwr_out, gstEnergyManageData.load_total_active_pwr);

    gstGridBusPwrLimitOut.inv_pwr_limited = _GridBusPwrLimit_ClampInvPwr(_gPwrCtrl.pwr_out, GRID_BUS_PWR_LIMIT_GRID_PWR_MAX_W, GRID_BUS_PWR_LIMIT_INV_PWR_MIN_W);
}
