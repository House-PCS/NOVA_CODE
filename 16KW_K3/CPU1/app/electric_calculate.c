/*
 * electric_calculate.c
 *
 *  Created on: 2025-11-19
 *      Author: Nova006348
 */

#include "electric_calculate.h"
#include "string.h"
#include "cpu_timers.h"
#include "cla_cpu_share.h"
#include "ph3_calc_ifc_update.h"
#include "algo_interface.h"
#include "ct.h"

static SysElectricMeter_t _gSysElectricMeter;
static void _gSysElectricMeterUpdate(void);

void ElectricParamClear(uint16_t *flag)
{
    if (flag != NULL && (*flag != 0U))
    {
        _gSysElectricMeter.bat_chg_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.bat_dsg_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.buy_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.eps_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.ev_chg_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.ev_dsg_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.gen_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.pv_energr_meter[0].total_01kwh = 0;
        _gSysElectricMeter.pv_energr_meter[1].total_01kwh = 0;
        _gSysElectricMeter.pv_energr_meter[2].total_01kwh = 0;
        _gSysElectricMeter.remote_load_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.sell_electric_meter.total_01kwh = 0;
        *flag = 0U;
    }
}

/**
* @brief: ElectricParamInit()
* @param: nothing
* @return:nothing
* @note: Energy meter data initialization
*/
static void _gElectricParamInit(void)
{
    memset(&_gSysElectricMeter, 0, sizeof(SysElectricMeter_t));
}
/**
* @brief: _gEnergyAccumulate()
* @param: nothing
* @return:nothing
* @note: Accumulate power
*/
bool _gEnergyAccumulate(ElectricCalculate_t *acc, float power)
{
    if (acc == NULL || power <= 0)
    {
        return false;
    }
    // Accumulate power (0.1s sample to watt-seconds)
    acc->power_accumulate_ws += power * 0.1f;
    acc->sample_count++;

    // Check 1-minute sample and auto-calc enable
    if (acc->sample_count >= SAMPLES_PER_MINUTE)
    {
        acc ->calc_active_flag = true;
        acc ->power_minute_ws = acc->power_accumulate_ws;
        acc->sample_count = 0;
        acc->power_accumulate_ws = 0;
    }
    return true;
}

/**
* @brief: _gEnergyCalculate()
* @param: nothing
* @return:nothing
* @note: Calculate energy (read variables within one minute)
*/
void _gEnergyCalculate(ElectricCalculate_t *acc)
{
    if (acc == NULL || (acc ->calc_active_flag == false))
    {
        return ;
    }
    // Convert accumulation to Wh (divide by 3600s)
    float minute_wh = acc ->power_minute_ws / 3600.0f;
    // Total Wh = 1-min sum + remainder
    float total_wh = minute_wh + acc->remainder_wh;
    // Compute 0.1 kWh integer and remainder
    uint32_t integer_part = (uint32_t)(total_wh / 100.0f);     /*Convert to 0.1 kWh*/
    float new_remainder = total_wh - (integer_part * 100.0f);  /*Remainder from last conversion*/

    // Accumulate total 0.1 kWh
    acc->total_01kwh += integer_part;
    acc->remainder_wh = new_remainder;

    // Reset accumulator
    acc->sample_count = 0;
    acc ->calc_active_flag = false;
}

/**
* @brief: ElectricPowerAccumulate()
* @param: nothing
* @return:nothing
* @note: Energy accumulation should run at accurate timing
*/
void ElectricPowerAccumulate(void)
{
    static uint16_t i = 0;
    static uint32_t time_out = 0;
    static bool first_run_flag = true;
    if(first_run_flag)
    {
        first_run_flag = false;
        _gElectricParamInit();
        time_out = TimGetSystime();
    }
    if(TimDifftimeFromNow(time_out)  > 100)          /*Accumulate energy every 100 ms*/
    {
        _gEnergyAccumulate(&_gSysElectricMeter.bat_chg_electric_meter,(NON_NEGATIVE(-1 * gstPh3CalcInfo.bus.power_filter)));
        _gEnergyAccumulate(&_gSysElectricMeter.bat_dsg_electric_meter,(NON_NEGATIVE(gstPh3CalcInfo.bus.power_filter)));

        _gEnergyAccumulate(&_gSysElectricMeter.buy_electric_meter,(NON_NEGATIVE(-1 * gstPh3CalcInfo.grid_power.total_active)));
        _gEnergyAccumulate(&_gSysElectricMeter.sell_electric_meter,(NON_NEGATIVE(gstPh3CalcInfo.grid_power.total_active)));

        _gEnergyAccumulate(&_gSysElectricMeter.eps_electric_meter,(NON_NEGATIVE(-1 * gstPh3CalcInfo.load_power.total_active)));

        _gEnergyAccumulate(&_gSysElectricMeter.ev_chg_electric_meter,(NON_NEGATIVE(-1 * 0 )));
        _gEnergyAccumulate(&_gSysElectricMeter.ev_dsg_electric_meter,(NON_NEGATIVE( 0 )));

        _gEnergyAccumulate(&_gSysElectricMeter.gen_electric_meter,(NON_NEGATIVE( gstPh3CalcInfo.gen_power.total_active)));
        _gEnergyAccumulate(&_gSysElectricMeter.remote_load_electric_meter,(NON_NEGATIVE(gCtDataInfo.ct_power.total_active - gstPh3CalcInfo.grid_power.total_active)));

        _gEnergyAccumulate(&_gSysElectricMeter.pv_energr_meter[0],PvPwrAveRead(0));
        _gEnergyAccumulate(&_gSysElectricMeter.pv_energr_meter[1],PvPwrAveRead(1));
        _gEnergyAccumulate(&_gSysElectricMeter.pv_energr_meter[2],PvPwrAveRead(2));
        time_out = TimGetSystime();
    }
    i++;
    if(i >= 10 * 30)
    {
        _gSysElectricMeterUpdate();
        i = 0;
    }
}
/**
* @brief: SysElectricMeter()
* @param: nothing
* @return:nothing
* @note: Read energy within one minute
*/
static void _gSysElectricMeterUpdate(void)
{
    _gEnergyCalculate(&_gSysElectricMeter.bat_chg_electric_meter);
    _gEnergyCalculate(&_gSysElectricMeter.bat_dsg_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.buy_electric_meter);
    _gEnergyCalculate(&_gSysElectricMeter.sell_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.eps_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.ev_chg_electric_meter);
    _gEnergyCalculate(&_gSysElectricMeter.ev_dsg_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.gen_electric_meter);
    _gEnergyCalculate(&_gSysElectricMeter.remote_load_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.pv_energr_meter[0]);
    _gEnergyCalculate(&_gSysElectricMeter.pv_energr_meter[1]);
    _gEnergyCalculate(&_gSysElectricMeter.pv_energr_meter[2]);
}
/**
* @brief: GetPvTotalElectric()
* @param: nothing
* @return:nothing
* @note: Get total PV energy
*/
uint16_t GetPvTotalElectric(void)
{
    return (_gSysElectricMeter.pv_energr_meter[0].total_01kwh + _gSysElectricMeter.pv_energr_meter[1].total_01kwh +
                                                                _gSysElectricMeter.pv_energr_meter[2].total_01kwh);
}
/**
* @brief: GetPv1Electric()
* @param: nothing
* @return:nothing
* @note: Get PV1 energy
*/
uint16_t GetPv1Electric(void)
{
    return (_gSysElectricMeter.pv_energr_meter[0].total_01kwh);
}
/**
* @brief: GetPv2Electric()
* @param: nothing
* @return:nothing
* @note: Get PV2 energy
*/
uint16_t GetPv2Electric(void)
{
    return (_gSysElectricMeter.pv_energr_meter[1].total_01kwh);
}
/**
* @brief: GetPv3Electric()
* @param: nothing
* @return:nothing
* @note: Get PV3 energy
*/
uint16_t GetPv3Electric(void)
{
    return (_gSysElectricMeter.pv_energr_meter[2].total_01kwh);
}
/**
* @brief: GetBuyElectric()
* @param: nothing
* @return:nothing
* @note: Get import energy
*/
uint16_t GetBuyElectric(void)
{
    return (_gSysElectricMeter.buy_electric_meter.total_01kwh);
}
/**
* @brief: GetSellElectric()
* @param: nothing
* @return:nothing
* @note: Get export energy
*/
uint16_t GetSellElectric(void)
{
    return (_gSysElectricMeter.sell_electric_meter.total_01kwh);
}
/**
* @brief: GetBatChgElectric()
* @param: nothing
* @return:nothing
* @note: Get battery charge energy
*/
uint16_t GetBatChgElectric(void)
{
    return (_gSysElectricMeter.bat_chg_electric_meter.total_01kwh);
}
/**
* @brief: GetBatDsgElectric()
* @param: nothing
* @return:nothing
* @note: Get battery discharge energy
*/
uint16_t GetBatDsgElectric(void)
{
    return (_gSysElectricMeter.bat_dsg_electric_meter.total_01kwh);
}
/**
* @brief: GetEpsElectric()
* @param: nothing
* @return:nothing
* @note: Get local load (EPS) energy
*/
uint16_t GetEpsElectric(void)
{
    return (_gSysElectricMeter.eps_electric_meter.total_01kwh);
}
/**
* @brief: GetGenElectric()
* @param: nothing
* @return:nothing
* @note: Get generator energy
*/
uint16_t GetGenElectric(void)
{
    return (_gSysElectricMeter.gen_electric_meter.total_01kwh);
}
/**
* @brief: GetRemoteLoadElectric()
* @param: nothing
* @return:nothing
* @note: Get remote load energy
*/
uint16_t GetRemoteLoadElectric(void)
{
    return (_gSysElectricMeter.remote_load_electric_meter.total_01kwh);
}
/**
* @brief: GetEvChgElectric()
* @param: nothing
* @return:nothing
* @note: Get EV charge energy
*/
uint16_t GetEvChgElectric(void)
{
    return (_gSysElectricMeter.ev_chg_electric_meter.total_01kwh);
}
/**
* @brief: GetEvDsgElectric()
* @param: nothing
* @return:nothing
* @note: Get EV discharge energy
*/
uint16_t GetEvDsgElectric(void)
{
    return (_gSysElectricMeter.ev_dsg_electric_meter.total_01kwh);
}
