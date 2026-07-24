/**
 * @file ntc.c
 * @brief NTC temperature acquisition, filtering, and fault detection
 */

#include <ntc.h>
#include <sample_update.h>

#pragma CODE_SECTION(NTCProcess, ".TI.ramfunc");
#pragma CODE_SECTION(_DetectFault, ".TI.ramfunc");
#pragma CODE_SECTION(_ProcessChannelTempe, ".TI.ramfunc");

NtcTemp_t _gNtcTemperate;
static float _gNtcInVol[NTC_CHANNEL_NUM] = {0};
static float _gNtcFilterBuffer[NTC_CHANNEL_NUM][NTC_AVERAGE_COUNT];

static bool _SlidingFilterReady(const Mid_SlidingFilter_t* filter)
{
    return (filter->is_full != 0U) || (filter->index > 0U);
}

static void _FaultInit(NtcFaultChannel_t* fault)
{
    fault->current_fault = NTC_FAULT_NONE;
    fault->previous_fault = NTC_FAULT_NONE;
    fault->fault_active = false;
    fault->fault_count = 0U;
    fault->fault_run_cont = 0U;
}

void NTCInit(void)
{
    uint8_t i;

    for(i = 0U; i < NTC_CHANNEL_NUM; i++)
    {
        _gNtcTemperate.raw_temperature[i] = 0.0f;
        _gNtcTemperate.filtered_temperature[i] = 0.0f;
        _gNtcTemperate.valid[i] = false;
        Mid_SlidingFilter_Init(&_gNtcTemperate.filter[i], &_gNtcFilterBuffer[i][0], NTC_AVERAGE_COUNT);
        _FaultInit(&_gNtcTemperate.fault[i]);
    }

    _gNtcTemperate.current_channel = 0U;
}

static NTC_FAULT_TYPE _DetectFault(uint8_t channel, float temperature)
{
    NTC_FAULT_TYPE fault_type = NTC_FAULT_NONE;
    NtcFaultChannel_t* fault = &_gNtcTemperate.fault[channel];

    /* Open-circuit / break detection */
    if(temperature <= -45.0f || temperature >= 160.0f)
    {
        fault_type = NTC_FAULT_BREAK;
        _gNtcTemperate.filtered_temperature[channel] = temperature;
    }
    else if(fault->fault_active)
    {
        /* Use recovery threshold while fault is active */
        if(fault->current_fault == NTC_FAULT_BREAK && temperature > -40.0f && temperature < 155.0f)
        {
            fault_type = NTC_FAULT_NONE;
        }
        else
        {
            fault_type = fault->current_fault;
        }
    }

    if(fault_type != fault->current_fault)
    {
        fault->previous_fault = fault->current_fault;
        fault->current_fault = fault_type;

        if(fault_type != NTC_FAULT_NONE)
        {
            fault->fault_active = true;
            fault->fault_count++;
        }
        else
        {
            fault->fault_active = false;
        }
    }

    return fault_type;
}

static void _ProcessChannelTempe(uint8_t channel)
{
    float voltage = 0.0f;
    float raw_temperature = 0.0f;
    float filtered_temperature = 0.0f;
    Mid_SlidingFilter_t* filter = NULL;

    if(channel >= NTC_CHANNEL_NUM)
    {
        return;
    }

    filter = &_gNtcTemperate.filter[channel];
    voltage = _gNtcInVol[channel];
    raw_temperature = CalculateTemp(voltage);

    _gNtcTemperate.fault[channel].fault_run_cont++;
    _gNtcTemperate.raw_temperature[channel] =
        _gNtcTemperate.raw_temperature[channel] * 0.99f + raw_temperature * 0.01f;

    filtered_temperature = Mid_SlidingFilter_Update(filter, _gNtcTemperate.raw_temperature[channel]);

    if(_SlidingFilterReady(filter))
    {
        _gNtcTemperate.filtered_temperature[channel] = filtered_temperature;
        _gNtcTemperate.valid[channel] = true;

        if(_gNtcTemperate.fault[channel].fault_run_cont >= NTC_FAULT_JUDGE_COUNT)
        {
            _gNtcTemperate.fault[channel].fault_run_cont = 0U;
            _DetectFault(channel, filtered_temperature);
        }
    }
    else
    {
        _gNtcTemperate.valid[channel] = false;
    }
}

void NTCProcess(void)
{
    _ProcessChannelTempe(_gNtcTemperate.current_channel);

    _gNtcTemperate.current_channel++;
    if(_gNtcTemperate.current_channel >= NTC_CHANNEL_NUM)
    {
        _gNtcTemperate.current_channel = 0U;
    }
}

float NTCGetRawTemp(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return 0.0f;
    }
    return _gNtcTemperate.raw_temperature[channel];
}

float NTCGetFilterTemp(NTC_VALUE_TYPE channel)
{
    if(channel >= NTC_CHANNEL_NUM || !_gNtcTemperate.valid[channel])
    {
        return 0.0f;
    }
    return _gNtcTemperate.filtered_temperature[channel];
}

void NTCGetInVol(uint8_t channel, float vol)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return;
    }
    _gNtcInVol[channel] = vol;
}

bool NTCChkData(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return false;
    }
    return _gNtcTemperate.valid[channel];
}

bool NTCFaultActiveState(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return false;
    }
    return _gNtcTemperate.fault[channel].fault_active;
}

bool GetNTCFault(void)
{
    uint8_t i;

    for(i = 0U; i < NTC_CHANNEL_NUM; i++)
    {
        if(_gNtcTemperate.fault[i].fault_active)
        {
            return true;
        }
    }
    return false;
}

uint8_t NTCGetFaultCount(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return 0U;
    }
    return _gNtcTemperate.fault[channel].fault_count;
}

void NTCClearFault(uint8_t channel)
{
    if(channel < NTC_CHANNEL_NUM)
    {
        _FaultInit(&_gNtcTemperate.fault[channel]);
    }
}

void NTCClearAllFault(void)
{
    uint8_t i;

    for(i = 0U; i < NTC_CHANNEL_NUM; i++)
    {
        _FaultInit(&_gNtcTemperate.fault[i]);
    }
}

float GetNTCMaxTemp(void)
{
    uint8_t i;
    float max = -50.0f;

    for(i = 0U; i < NTC_CHANNEL_NUM; i++)
    {
        if(_gNtcTemperate.filtered_temperature[i] > max)
        {
            max = _gNtcTemperate.filtered_temperature[i];
        }
    }
    return max;
}

bool NTCFaultChangeState(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return false;
    }
    return (_gNtcTemperate.fault[channel].current_fault != _gNtcTemperate.fault[channel].previous_fault);
}
