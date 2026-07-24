/*
 * ph3_sts_ifc_update.c
 *
 * CPU1 STS data info (mapped from CPU2 shared memory).
 * NOTE: Intentionally not implemented yet.
 */
#include "ph3_sts_ifc_update.h"
#include <string.h>

#pragma DATA_SECTION(_gstStsEnStatus, "ramgs0")
static StsInputStatus_t _gstStsEnStatus;

static StsOutputStatus_t _gstStsOutputStatus;



volatile StsInputStatus_t *StsEnUpdate_Get(void)
{
    return &_gstStsEnStatus;
}






void Ph3StsInfo_Init(void)
{
    (void)memset(&_gstStsEnStatus, 0, sizeof(_gstStsEnStatus));
    (void)memset(&_gstStsOutputStatus, 0, sizeof(_gstStsOutputStatus));
}

void SetStsEnRunEn(uint8_t sts_run_en)
{
    _gstStsEnStatus.sts_run_en = sts_run_en;
}

uint8_t GetStsEnRunEn(void)
{
    return _gstStsEnStatus.sts_run_en;
}

uint8_t GetStsEnForceGenMode(void)
{
    return _gstStsEnStatus.force_gen_mode;
}
void SetStsEnForceGenMode(uint8_t force_gen_mode)
{
    _gstStsEnStatus.force_gen_mode = force_gen_mode;
}

void SetStsEnEnConnect(uint8_t en_connect)
{
    _gstStsEnStatus.en_connect = en_connect;
}

void SetStsEnEnDisconnect(uint8_t en_disconnect)
{
    _gstStsEnStatus.en_disconnect = en_disconnect;
}

void SetStsEnWarnClear(uint8_t warn_clear)
{
    _gstStsEnStatus.warn_clear = warn_clear;
}

void SetStsEnErrClear(uint8_t err_clear)
{
    _gstStsEnStatus.err_clear = err_clear;
}

void StsOutputUpdate(void)
{
    volatile StsOutputStatus_t *stsOutput = DevDcr_GetStsOutput();
    if (stsOutput != (volatile StsOutputStatus_t *)0)
    {
        _gstStsOutputStatus.sts_output = stsOutput->sts_output;
        _gstStsOutputStatus.sts_err = stsOutput->sts_err;
        _gstStsOutputStatus.sts_warn = stsOutput->sts_warn;
        _gstStsOutputStatus.sts_state = stsOutput->sts_state;
        _gstStsOutputStatus.sts_is_disconnectd = stsOutput->sts_is_disconnectd;
        _gstStsOutputStatus.sts_is_disconnecting = stsOutput->sts_is_disconnecting;
        _gstStsOutputStatus.sts_is_connecting = stsOutput->sts_is_connecting;
        _gstStsOutputStatus.sts_is_connected = stsOutput->sts_is_connected;
        _gstStsOutputStatus.sts_is_gen_mode = stsOutput->sts_is_gen_mode;
    }
}

uint8_t Get_StsOutput_RelagState(void)
{
    return _gstStsOutputStatus.sts_output.en_relay;
}

float32 Get_StsOutput_VolRmsAdjust(void)
{
    return _gstStsOutputStatus.sts_output.adjust_volt;
}

float32 Get_StsOutput_wAdjust(void)
{
    return _gstStsOutputStatus.sts_output.adjust_w;
}

uint8_t Get_StsOutput_IsDisconnectd(void)
{
    return _gstStsOutputStatus.sts_is_disconnectd;
}

uint8_t Get_StsOutput_IsDisconnecting(void)
{
    return _gstStsOutputStatus.sts_is_disconnecting;
}

uint8_t Get_StsOutput_IsConnecting(void)
{
    return _gstStsOutputStatus.sts_is_connecting;
}

uint8_t Get_StsOutput_IsConnected(void)
{
    return _gstStsOutputStatus.sts_is_connected;
}

EN_STS_STATE Get_StsOutput_State(void)
{
    return _gstStsOutputStatus.sts_state;
}

uint8_t Get_StsOutput_IsWarn(void)
{
    return (_gstStsOutputStatus.sts_warn.bytes[0] != 0U)
        || (_gstStsOutputStatus.sts_warn.bytes[1] != 0U);
}

uint8_t Get_StsOutput_IsErr(void)
{
    return (_gstStsOutputStatus.sts_err.bytes[0] != 0U)
        || (_gstStsOutputStatus.sts_err.bytes[1] != 0U);
}

const Sts_Err_Un_t *Get_StsOutput_Err(void)
{
    return &_gstStsOutputStatus.sts_err;
}
const Sts_Warn_Un_t *Get_StsOutput_Warn(void)
{
    return &_gstStsOutputStatus.sts_warn;
}

uint8_t  GetStsGenForceState(void)
{
    return _gstStsOutputStatus.sts_is_gen_mode;
}
