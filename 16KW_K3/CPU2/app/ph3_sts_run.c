/*
 * ph3_sts_run.c
 *
 * CPU2 three-phase status runtime.
 */
#include "ph3_sts_run.h"
#include "ph3_calc_run.h"
#include "dev_dual_core_receive.h"
#include <string.h>

static Sts_Input_t _gstStsInput;
#pragma DATA_SECTION(_gstStsOutput, "ramgs1")
StsOutputStatus_t _gstStsOutput;

StsInputStatus_t _gstStsInputStatus;

Sts_Adjust_Input_t _gstStsAdjustInput;

static uint8_t _gstStsFlagUpdateFlag = 0;
StsOutputStatus_t *StsOutputUpdate_Get(void)
{
    return &_gstStsOutput;
}

void Ph3_Sts_FlagUpdate(void)
{
    volatile StsInputStatus_t *sts_input = DevDcr_GetStsInput();

    if(sts_input == (volatile StsInputStatus_t *)0)
    {
        return;
    }
    _gstStsInputStatus.sts_run_en = sts_input->sts_run_en;
    _gstStsInputStatus.force_gen_mode = sts_input->force_gen_mode;
    _gstStsInputStatus.en_connect = sts_input->en_connect;
    _gstStsInputStatus.en_disconnect = sts_input->en_disconnect;
    _gstStsInputStatus.warn_clear = sts_input->warn_clear;
    _gstStsInputStatus.err_clear = sts_input->err_clear;
}

void Ph3_Sts_DataSlowUpdate(void)
{
    if(_gstStsFlagUpdateFlag == 0)
    {
        return;
    }
    _gstStsFlagUpdateFlag = 0;
    if(_gstStsInputStatus.force_gen_mode != _gstStsOutput.sts_is_gen_mode)
    {
        _gstStsOutput.sts_is_gen_mode = _gstStsInputStatus.force_gen_mode;
    }
    if(_gstStsOutput.sts_is_gen_mode)
    {
        _gstStsInput.connected_volt_rms_a = s_gen.phase_a.vol.rms;
        _gstStsInput.connected_volt_rms_b = s_gen.phase_b.vol.rms;
        _gstStsInput.connected_volt_rms_c = s_gen.phase_c.vol.rms;

        _gstStsInput.connected_curr_rms_a = s_gen.phase_a.cur.rms;
        _gstStsInput.connected_curr_rms_b = s_gen.phase_b.cur.rms;
        _gstStsInput.connected_curr_rms_c = s_gen.phase_c.cur.rms;
    }
    else
    {
        _gstStsInput.connected_volt_rms_a = s_grid.phase_a.vol.rms;
        _gstStsInput.connected_volt_rms_b = s_grid.phase_b.vol.rms;
        _gstStsInput.connected_volt_rms_c = s_grid.phase_c.vol.rms;

        _gstStsInput.connected_curr_rms_a = s_grid.phase_a.cur.rms;
        _gstStsInput.connected_curr_rms_b = s_grid.phase_b.cur.rms;
        _gstStsInput.connected_curr_rms_c = s_grid.phase_c.cur.rms;
    }

    _gstStsInput.inv_volt_rms_a = s_inv.phase_a.vol.rms;
    _gstStsInput.inv_volt_rms_b = s_inv.phase_b.vol.rms;
    _gstStsInput.inv_volt_rms_c = s_inv.phase_c.vol.rms;

    Sts_WarnCheck_Run();
    Sts_FaultCheck_Run();

    _gstStsOutput.sts_err = *Sts_Get_Err_Obj();
    _gstStsOutput.sts_warn = *Sts_Get_Warn_Obj();
    _gstStsOutput.sts_is_connecting = Sts_Get_IsConnecting();
    _gstStsOutput.sts_is_disconnecting = Sts_Get_IsDisconnecting();
    _gstStsOutput.sts_is_disconnectd = Sts_Get_IsDisconnected();
    _gstStsOutput.sts_is_connected = Sts_Get_IsConnected();
    _gstStsOutput.sts_state = Sts_Get_State();
}

void Ph3_Sts_InstDateUpdate(void)
{
    volatile CpuAdc_t *adc = DevDcr_GetCpuAdcResult();

    if(adc == (volatile CpuAdc_t *)0)
    {
        return;
    }
    if(_gstStsOutput.sts_is_gen_mode)
    {
        _gstStsInput.connected_volt_inst_a = adc->gen_volt_phase_a;
        _gstStsInput.connected_volt_inst_b = adc->gen_volt_phase_b;
        _gstStsInput.connected_volt_inst_c = adc->gen_volt_phase_c;

        _gstStsInput.connected_theta = s_gen.volt_pll.theta_sin;

        _gstStsInput.connected_w0 = s_gen.freq_a * 2.0f * PH3_CALC_PI;

    }
    else
    {
        _gstStsInput.connected_volt_inst_a = adc->grid_volt_phase_a;
        _gstStsInput.connected_volt_inst_b = adc->grid_volt_phase_b;
        _gstStsInput.connected_volt_inst_c = adc->grid_volt_phase_c;

        _gstStsInput.connected_theta = s_grid.volt_pll.theta_sin;

        _gstStsInput.connected_w0 = s_grid.freq_a * 2.0f * PH3_CALC_PI;

    }

    _gstStsInput.inv_volt_inst_a = adc->inv_volt_phase_a;
    _gstStsInput.inv_volt_inst_b = adc->inv_volt_phase_b;
    _gstStsInput.inv_volt_inst_c = adc->inv_volt_phase_c;

    _gstStsInput.inv_theta = s_inv.volt_pll.theta_sin;

    _gstStsInput.inv_w0 = s_inv.freq_a * 2.0f * PH3_CALC_PI;

    Sts_Set_InputData(&_gstStsInput);
    _gstStsFlagUpdateFlag = 1;
}

void ph3Sts_Adjust_DataUpdate(void)
{

    //STS相位调整器运行
    _gstStsAdjustInput.inv_w0 = _gstStsInput.inv_w0;
    _gstStsAdjustInput.inv_theta = _gstStsInput.inv_theta;
    _gstStsAdjustInput.inv_volt_rms = (_gstStsInput.inv_volt_rms_a + _gstStsInput.inv_volt_rms_b +_gstStsInput.inv_volt_rms_c)/3;
    _gstStsAdjustInput.grid_w0 = _gstStsInput.connected_w0;
    _gstStsAdjustInput.grid_theta = _gstStsInput.connected_theta;
    _gstStsAdjustInput.grid_volt_rms = (_gstStsInput.connected_volt_rms_a + _gstStsInput.connected_volt_rms_b +_gstStsInput.connected_volt_rms_c)/3;
    gStsAdjust.state = Sts_Get_State();
    Sts_Adjust_SetInput(&_gstStsAdjustInput);
}

void Ph3_Sts_Init(void)
{
    Sts_Init();
    memset(&_gstStsInput, 0, sizeof(Sts_Input_t));
    memset(&_gstStsOutput, 0, sizeof(StsOutputStatus_t));
    Sts_Adjust_Init();
}

void Ph3_Sts_Run(void)
{
    if(_gstStsInputStatus.sts_run_en)
    {
        Sts_Set_EnConnect(_gstStsInputStatus.en_connect);
        Sts_Set_Warn_clear(_gstStsInputStatus.warn_clear);
        Sts_Set_Err_clear(_gstStsInputStatus.err_clear);
        Ph3_Sts_InstDateUpdate();
        Sts_Run();
        ph3Sts_Adjust_DataUpdate();
        Sts_Adjust_Run();
        gStsData.adjust_w = gStsAdjust.o_adj_w;
        gStsData.adjust_volt = gStsAdjust.o_adj_volt;
        Sts_Get_OutputData(&_gstStsOutput.sts_output);
    }
    else
    {
        Ph3_Sts_Init();
    }
}

