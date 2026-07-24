/*
 * sys_info.c
 *
 *  Created on: 2025-3-20
 *      Author: Nova006352
 */

#include "sys_info.h"
#include "cla_cpu_share.h"
#include "ph3_calc_ifc_update.h"
#include "firmware_upgrate.h"
#include "adc.h"
#include "fault.h"
#include "algo_interface.h"
#include "fan.h"
#include "ntc.h"
#include "firmware_upgrate.h"
#include "electric_calculate.h"
#include "ct.h"
#include "debug_analysis.h"
#include "flash_default.h"
#include "control_state.h"
UploadInfo_t    gstUploadInfo;

#define PCS_HOLD_REG_FLASH_ADDR             (0x200000)    /*PCS hold register flash address*/
#define PCS_HOLD_REG_VALID_FLASH_ADDR       (0x200FF8)    /*PCS hold register valid flag address*/
#define BACK_UP_PAGE_NUM                    (20)          /*Number of local backup flash pages*/


/**
* @brief: DefaultInfoGet
* @param:
* @return:
* @note:
*
*/
void DefaultInfoGet(void)
{
}
#define INVERTER_MODEL 0X83

// Macro: low 16 bits of 32-bit value
#define GET_LOW_16_BITS(value) ((uint16_t)(((int32_t)(value)) & 0x0000FFFF))

// Macro: high 16 bits of 32-bit value
#define GET_HIGH_16_BITS(value) ((uint16_t)((((int32_t)(value)) >> 16) & 0x0000FFFF))

/**
* @brief: UploadInfoInit
* @param: void
* @return:Nothing
* @note:Upload info initialization
*/
void UploadInfoInit()
{
    memset((uint16_t*)&gstUploadInfo, 0x00, (int)sizeof(UploadInfo_t));
}
/**
* @brief: GetPcsModeId
* @param: void
* @return:Nothing
* @note:Get PCS mode ID
*/
void GetPcsModeId()
{
    gstUploadInfo.upload_basic_info.mode_id = gstVersionInfo.pcs_version_info.model_id;
}
/**
* @brief: GetPcsVersionInfo
* @param: void
* @return:Nothing
* @note:Get PCS hardware/software version
*/
void GetPcsVersionInfo()
{
    uint16_t temp[3] = {0x0000,0x0002,0x0000};
    gstUploadInfo.upload_basic_info.software_ver[0] = (temp[0] << 12) | (temp[1] << 6) | temp[2];
//    gstUploadInfo.software_ver = gstVersionInfo.pcs_version_info.soft_version;
    memcpy((uint16_t*)&gstUploadInfo.upload_basic_info.software_ver_Extend, (uint16_t*)&gstVersionInfo.pcs_version_info.extern_version, (int)sizeof(gstVersionInfo.pcs_version_info.extern_version));
}

/**
* @brief: GetInverterModel
* @param: void
* @return:Nothing
* @note:Get inverter model
*/
void GetInverterModel()
{
    gstUploadInfo.upload_basic_info.hw_version = INVERTER_MODEL;
}
/**
* @brief: GetSysInputType
* @param: void
* @return:Nothing
* @note:Get system input type
*/

void GetSysInputType()
{
    if(gstPcsInvRunState == INV_STATE_GRID_CONNECTED)//Grid connected
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.grid_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.grid_input = 0;
    }
    if(gstPcsInvRunState == INV_STATE_GEN_CONNECTED)//Generator
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.gen_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.gen_input = 0;
    }
    if(Read_Mppt_state(0) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 0;
    }
    if(Read_Mppt_state(1) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 0;
    }
    if(Read_Mppt_state(2) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 0;
    }
    if(Read_Mppt_state(3) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 0;
    }
}
/**
* @brief: GetSysState
* @param: void
* @return:Nothing
* @note:Get system status
*/
 void GetSysState()
 {
     //TODO: hybrid/bypass/shutdown not handled yet
     if(gstPcsInvRunState == INV_STATE_STANDBY)//Standby state
     {
         gstUploadInfo.upload_module_status.inv_status = 0;
     }
     else if(gstPcsInvRunState == INV_STATE_SELFTEST)//Self-test state
     {
         gstUploadInfo.upload_module_status.inv_status = 1;
     }
     else if(gstPcsInvRunState == INV_STATE_DISCONNECTED)
     {
         gstUploadInfo.upload_module_status.inv_status = 3;
     }
     else
     {
         gstUploadInfo.upload_module_status.inv_status = 0;
     }
 }
 /**
 * @brief: GetInvState
 * @param: void
 * @return:Nothing
 * @note:Get inverter status
 */
void GetInvState()
{
    if(gstPcsInvRunState == INV_STATE_STANDBY)//Standby
    {
        gstUploadInfo.upload_module_status.invOnGridState = 0;
    }
    else if(gstPcsInvRunState == INV_STATE_DISCONNECTED)//Off-grid
    {
        gstUploadInfo.upload_module_status.invOnGridState = 1;
    }
    else if(gstPcsInvRunState == INV_STATE_GRID_CONNECTED || gstPcsInvRunState == INV_STATE_GEN_CONNECTED )//Grid connected
    {
        gstUploadInfo.upload_module_status.invOnGridState = 2;
    }
    else if(gstPcsInvRunState == INV_STATE_GRID_CONNECTING || gstPcsInvRunState == INV_STATE_GEN_CONNECTING)//Off-grid to connect
    {
        gstUploadInfo.upload_module_status.invOnGridState = 3;
    }
    else if(gstPcsInvRunState == INV_STATE_GRID_DISCONNECTING || gstPcsInvRunState == INV_STATE_GEN_DISCONNECTING)//Connect to off-grid
    {
        gstUploadInfo.upload_module_status.invOnGridState = 4;
    }
    else
    {
        gstUploadInfo.upload_module_status.invOnGridState = 0;
    }
}

/**
* @brief: GetMppt1State
* @param: void
* @return:Nothing
* @note:Get MPPT1 status
*/
void GetMppt1State()
{
    gstUploadInfo.upload_module_status.mppt1_status = Read_Mppt_state(0);
}

/**
* @brief: GetMppt2State
* @param: void
* @return:Nothing
* @note:Get MPPT2 status
*/
void GetMppt2State()
{
    gstUploadInfo.upload_module_status.mppt2_status = Read_Mppt_state(1);
}



/**
* @brief: GetMppt3State
* @param: void
* @return:Nothing
* @note:Get MPPT3 status
*/
void GetMppt3State()
{
    gstUploadInfo.upload_module_status.mppt3_status = Read_Mppt_state(2);
}


/**
* @brief: GetMppt4State
* @param: void
* @return:Nothing
* @note:Get MPPT4 status
*/
void GetMppt4State()
{
    gstUploadInfo.upload_module_status.mppt4_status = Read_Mppt_state(3);
}

/**
* @brief: GetAlarmCode
* @param: void
* @return:Nothing
* @note:Get warning code
*/
void GetAlarmCode()
{
//    gstUploadInfo.upload_fault_info.GridAlarm = GetGridWarningHighValue();
//    gstUploadInfo.upload_fault_info.PVAlarm[0] = GetPvWarningLowValue();
//    gstUploadInfo.upload_fault_info.PVAlarm[1] = GetPvWarningHighValue();
//    gstUploadInfo.upload_fault_info.BusAlarm = GetBusWarningValue();
//    gstUploadInfo.upload_fault_info.InvAlarm = GetInvWarningValue();
//    gstUploadInfo.upload_fault_info.LoadAlarm = GetLoadWarningValue();
//    gstUploadInfo.upload_fault_info.TempAlarm = GetTempWarningValue();
//    gstUploadInfo.upload_fault_info.GenAlarm  = GetGenWarningValue();
//    gstUploadInfo.upload_fault_info.OtherAlarm = GetOtherWarningValue();

}

/**
* @brief: GetErrorCode
* @param: void
* @return:Nothing
* @note:Get fault code
*/
void GetErrorCode()
{
//    gstUploadInfo.upload_fault_info.GridError = GetGridErrorValue();
//    gstUploadInfo.upload_fault_info.PVError[0] = GetPvErrorLowValue();
//    gstUploadInfo.upload_fault_info.PVError[1] = GetPvErrorHighValue();
//    gstUploadInfo.upload_fault_info.BusError = GetBusErrorValue();
//    gstUploadInfo.upload_fault_info.InvError = GetInvErrorValue();
//    gstUploadInfo.upload_fault_info.LoadError = GetLoadErrorValue();
//    gstUploadInfo.upload_fault_info.TempError = GetTempErrorValue();
//    gstUploadInfo.upload_fault_info.GenError = GetGenErrorValue();
//    gstUploadInfo.upload_fault_info.OtherError = GetOtherErrorValue();
}

void GetNoticeCode()
{
    gstUploadInfo.upload_fault_info.GridNotice = 0x0000;
    gstUploadInfo.upload_fault_info.PVNotice[0] = 0x0000;
    gstUploadInfo.upload_fault_info.PVNotice[1] = 0x0000;
    gstUploadInfo.upload_fault_info.BusNotice = 0x0000;
    gstUploadInfo.upload_fault_info.InvNotice = 0x0000;
    gstUploadInfo.upload_fault_info.LoadNotice = 0x0000;
    gstUploadInfo.upload_fault_info.TempNotice = 0x0000;
    gstUploadInfo.upload_fault_info.GenNotice = 0x0000;
    gstUploadInfo.upload_fault_info.OtherNotice = 0x0000;
}

/**
* @brief: GetCtOrMeterInfo
* @param: void
* @return:Nothing
* @note:Get fault code
*/

void GetCtOrMeterInfo()
{
    static uint16_t cont = 0;
    if(MeterModeGet() == 1)//Meter
       {
           if(cont == 0)
           {
               gstUploadInfo.upload_Power_info.r_phase_meter_curr =  GetMeterRCurInfo() * 10;
               gstUploadInfo.upload_Power_info.r_phase_meter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_vol.a_phase_rms * GetMeterRCurInfo());
               gstUploadInfo.upload_Power_info.r_phase_meter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_vol.a_phase_rms * GetMeterRCurInfo());
               gstUploadInfo.upload_Power_info.r_phase_meter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.a_phase_active);
               gstUploadInfo.upload_Power_info.r_phase_meter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.a_phase_active);
               gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.a_phase_reactive);
               gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.a_phase_reactive);
           }
           else if(cont == 1)
           {
               gstUploadInfo.upload_Power_info.s_phase_meter_curr =  GetMeterSCurInfo() * 10;
               gstUploadInfo.upload_Power_info.s_phase_meter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_vol.b_phase_rms * GetMeterSCurInfo());
               gstUploadInfo.upload_Power_info.s_phase_meter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_vol.b_phase_rms * GetMeterSCurInfo());
               gstUploadInfo.upload_Power_info.s_phase_meter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.b_phase_active);
               gstUploadInfo.upload_Power_info.s_phase_meter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.b_phase_active);
               gstUploadInfo.upload_Power_info.s_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.b_phase_reactive);
               gstUploadInfo.upload_Power_info.s_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.b_phase_reactive);
           }
           else if(cont == 2)
           {
               gstUploadInfo.upload_Power_info.t_phase_meter_curr =  GetMeterTCurInfo() * 10;
               gstUploadInfo.upload_Power_info.t_phase_meter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_vol.c_phase_rms * GetMeterTCurInfo());
               gstUploadInfo.upload_Power_info.t_phase_meter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_vol.c_phase_rms * GetMeterTCurInfo());
               gstUploadInfo.upload_Power_info.t_phase_meter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.c_phase_active);
               gstUploadInfo.upload_Power_info.t_phase_meter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.c_phase_active);
               gstUploadInfo.upload_Power_info.t_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.c_phase_reactive);
               gstUploadInfo.upload_Power_info.t_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.c_phase_reactive);

           }
       }
       else//CT info
       {
           if(cont == 0)
           {
               gstUploadInfo.upload_Power_info.r_phase_meter_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.a_phase_apperant);
               gstUploadInfo.upload_Power_info.r_phase_meter_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.a_phase_apperant);
               gstUploadInfo.upload_Power_info.r_phase_meter_active_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.a_phase_active);
               gstUploadInfo.upload_Power_info.r_phase_meter_active_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.a_phase_active);
               gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.a_phase_reactive);
               gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.a_phase_reactive);
               if(gCtDataInfo.ct_power.a_phase_active < 0)
               {
                   gstUploadInfo.upload_Power_info.r_phase_meter_curr =  -1 * (int16_t)(gstPh3CalcInfo.ct_curr.a_phase_rms * 10);
               }
               else
               {
                   gstUploadInfo.upload_Power_info.r_phase_meter_curr = (int16_t)(gstPh3CalcInfo.ct_curr.a_phase_rms * 10);
               }
           }
           else if(cont == 1)
           {
               gstUploadInfo.upload_Power_info.s_phase_meter_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.b_phase_apperant);
               gstUploadInfo.upload_Power_info.s_phase_meter_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.b_phase_apperant);
               gstUploadInfo.upload_Power_info.s_phase_meter_active_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.b_phase_active);
               gstUploadInfo.upload_Power_info.s_phase_meter_active_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.b_phase_active);
               gstUploadInfo.upload_Power_info.s_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.b_phase_reactive);
               gstUploadInfo.upload_Power_info.s_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.b_phase_reactive);
               if(gCtDataInfo.ct_power.b_phase_active < 0)
               {
                   gstUploadInfo.upload_Power_info.s_phase_meter_curr =  -1 * (int16_t)(gstPh3CalcInfo.ct_curr.b_phase_rms * 10);
               }
               else
               {
                   gstUploadInfo.upload_Power_info.s_phase_meter_curr = (int16_t)(gstPh3CalcInfo.ct_curr.b_phase_rms * 10);
               }
           }
           else if(cont == 2)
           {
               gstUploadInfo.upload_Power_info.t_phase_meter_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.c_phase_apperant);
               gstUploadInfo.upload_Power_info.t_phase_meter_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.c_phase_apperant);
               gstUploadInfo.upload_Power_info.t_phase_meter_active_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.c_phase_active);
               gstUploadInfo.upload_Power_info.t_phase_meter_active_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.c_phase_active);
               gstUploadInfo.upload_Power_info.t_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.c_phase_reactive);
               gstUploadInfo.upload_Power_info.t_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.c_phase_reactive);
               if(gCtDataInfo.ct_power.c_phase_active < 0)
               {
                   gstUploadInfo.upload_Power_info.t_phase_meter_curr =  -1 * (int16_t)(gstPh3CalcInfo.ct_curr.c_phase_rms * 10);
               }
               else
               {
                   gstUploadInfo.upload_Power_info.t_phase_meter_curr = (int16_t)(gstPh3CalcInfo.ct_curr.c_phase_rms * 10);
               }
           }
       }
    cont++;
    if(cont >= 3)
    {
        cont = 0;
    }
}

/**
* @brief: GetTemperatureValue
* @param: void
* @return:Nothing
* @note:Get fault code
*/

void GetTemperatureValue(void)
{
    gstUploadInfo.upload_module_status.chassis_internal_temp = (int16_t)(NTCGetFilterTemp(NTC_CHASSIS_TEMP) * 10);
    gstUploadInfo.upload_module_status.inverter_radiator_temp = (int16_t)(NTCGetFilterTemp(NTC_INV_TEMP) * 10);
    gstUploadInfo.upload_module_status.pv_temp = (int16_t)(NTCGetFilterTemp(NTC_PV_TEMP) * 10);
}


/**
* @brief: GetErrorCode
* @param: void
* @return:Nothing
* @note:Get all energy meter info
*/
void ElectricInfoGet_UPload(void)
{
    //Energy stats sent in 0.1 kWh units
    gstUploadInfo.upload_Capacity_info.PV_ElectricGenTotal[0] = GET_HIGH_16_BITS(GetPvTotalElectric); //Total PV generation
    gstUploadInfo.upload_Capacity_info.PV_ElectricGenTotal[1] = GET_LOW_16_BITS(GetPvTotalElectric);

    gstUploadInfo.upload_Capacity_info.elec_purchase_consum_total[0] = GET_HIGH_16_BITS(GetBuyElectric()); //Energy bought from grid
    gstUploadInfo.upload_Capacity_info.elec_purchase_consum_total[1] = GET_LOW_16_BITS(GetBuyElectric());

    gstUploadInfo.upload_Capacity_info.elec_sale_consum_total[0] = GET_HIGH_16_BITS(GetSellElectric());   //Energy sold to grid
    gstUploadInfo.upload_Capacity_info.elec_sale_consum_total[1] = GET_LOW_16_BITS(GetSellElectric());

    gstUploadInfo.upload_Capacity_info.Bat_ElectricChgTotal[0] = GET_HIGH_16_BITS(GetBatChgElectric());  //Total battery charge energy
    gstUploadInfo.upload_Capacity_info.Bat_ElectricChgTotal[1] = GET_LOW_16_BITS(GetBatChgElectric());

    gstUploadInfo.upload_Capacity_info.Bat_ElectricDsgTotal[0] = GET_HIGH_16_BITS(GetBatDsgElectric());   //Total battery discharge energy
    gstUploadInfo.upload_Capacity_info.Bat_ElectricDsgTotal[1] = GET_LOW_16_BITS(GetBatDsgElectric());

    gstUploadInfo.upload_Capacity_info.backup_total_sonsum_power[0] = GET_HIGH_16_BITS(GetEpsElectric());    //Total load energy
    gstUploadInfo.upload_Capacity_info.backup_total_sonsum_power[1] = GET_LOW_16_BITS(GetEpsElectric());

    gstUploadInfo.upload_Capacity_info.load_total_sonsum_power[0] = GET_HIGH_16_BITS(GetRemoteLoadElectric());     //Remote load energy
    gstUploadInfo.upload_Capacity_info.load_total_sonsum_power[1] = GET_LOW_16_BITS(GetRemoteLoadElectric());

    gstUploadInfo.upload_Capacity_info.Gen_ElectricTotal[0] = GET_HIGH_16_BITS(GetGenElectric());     //Total generator energy
    gstUploadInfo.upload_Capacity_info.Gen_ElectricTotal[1] = GET_LOW_16_BITS(GetGenElectric());

    gstUploadInfo.upload_Capacity_info.EV_ElectricChgTotal[0] = GET_HIGH_16_BITS(GetEvChgElectric());    //Total EV charge energy
    gstUploadInfo.upload_Capacity_info.EV_ElectricChgTotal[1] = GET_LOW_16_BITS(GetEvChgElectric());

    gstUploadInfo.upload_Capacity_info.EV_ElectricDsgTotal[0] = GET_HIGH_16_BITS(GetEvDsgElectric());     //Total EV discharge energy
    gstUploadInfo.upload_Capacity_info.EV_ElectricDsgTotal[1] = GET_LOW_16_BITS(GetEvDsgElectric());

    gstUploadInfo.upload_Capacity_info.PV1_ElectricGenTotal[0] = GET_HIGH_16_BITS(GetPv1Electric());   //PV1 total generation
    gstUploadInfo.upload_Capacity_info.PV1_ElectricGenTotal[1] = GET_LOW_16_BITS(GetPv1Electric());

    gstUploadInfo.upload_Capacity_info.PV2_ElectricGenTotal[0] = GET_HIGH_16_BITS(GetPv2Electric());   //PV2 total generation
    gstUploadInfo.upload_Capacity_info.PV2_ElectricGenTotal[1] = GET_LOW_16_BITS(GetPv2Electric());

    gstUploadInfo.upload_Capacity_info.PV3_ElectricGenTotal[0] = GET_HIGH_16_BITS(GetPv3Electric());   //PV3 total generation
    gstUploadInfo.upload_Capacity_info.PV3_ElectricGenTotal[1] = GET_LOW_16_BITS(GetPv3Electric());

    gstUploadInfo.upload_Capacity_info.PV4_ElectricGenTotal[0] = GET_HIGH_16_BITS(0);   //PV4 total generation
    gstUploadInfo.upload_Capacity_info.PV4_ElectricGenTotal[1] = GET_LOW_16_BITS(0);
}


/**
* @brief: GetElectricalInfo
* @param: void
* @return:Nothing
* @note:Get electrical info
*/
void GetElectricalInfo()
{
    static uint16_t cont = 0;
    GetCtOrMeterInfo();
    gstUploadInfo.upload_Power_info.bus_voltage = (uint16_t)((gstPh3CalcInfo.bus.pos_vol_filter +  gstPh3CalcInfo.bus.neg_vol_filter) * 10);

    gstUploadInfo.upload_Power_info.batt_volt = (uint16_t)gstUploadInfo.upload_Power_info.bus_voltage;
    gstUploadInfo.upload_Power_info.batt_current = gstPh3CalcInfo.bus.curr_filter * 10;
    gstUploadInfo.upload_Power_info.batt_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.bus.power_filter);
    gstUploadInfo.upload_Power_info.batt_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.bus.power_filter);
    if(cont == 0)
    {
        gstUploadInfo.upload_Power_info.r_phase_grid_volt = (uint16_t)(gstPh3CalcInfo.grid_vol.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_grid_freq = (uint16_t)(gstPh3CalcInfo.grid_vol.freq * 100);
        gstUploadInfo.upload_Power_info.r_phase_load_volt = (uint16_t)(0 * 10);
        gstUploadInfo.upload_Power_info.r_phase_load_cur = (uint16_t)(0 * 10);

        gstUploadInfo.upload_Power_info.r_phase_oil_volt = (uint16_t)(gstPh3CalcInfo.gen_vol.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_oil_freq = (uint16_t)(gstPh3CalcInfo.gen_vol.freq * 100);
        gstUploadInfo.upload_Power_info.r_phase_oil_curr = (uint16_t)(gstPh3CalcInfo.gen_curr.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_oil_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_oil_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_oil_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.a_phase_active);
        gstUploadInfo.upload_Power_info.r_phase_oil_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.a_phase_active);
        gstUploadInfo.upload_Power_info.r_phase_oil_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.a_phase_reactive);
        gstUploadInfo.upload_Power_info.r_phase_oil_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.a_phase_reactive);

        gstUploadInfo.upload_Power_info.r_phase_inverter_volt = (uint16_t)(gstPh3CalcInfo.inv_vol.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_inverter_curr = (uint16_t)(gstPh3CalcInfo.inv_curr.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_inverter_freq = (uint16_t)(gstPh3CalcInfo.inv_vol.freq * 100);
        gstUploadInfo.upload_Power_info.r_phase_inverter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_inverter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_inverter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.a_phase_active);
        gstUploadInfo.upload_Power_info.r_phase_inverter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.a_phase_active);
        gstUploadInfo.upload_Power_info.r_phase_inverter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.a_phase_reactive);
        gstUploadInfo.upload_Power_info.r_phase_inverter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.a_phase_reactive);

        gstUploadInfo.upload_Power_info.r_phase_backup_volt = (uint16_t)(gstPh3CalcInfo.load_vol.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_backup_curr = (uint16_t)(gstPh3CalcInfo.load_curr.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_backup_fre = (uint16_t)(gstPh3CalcInfo.load_vol.freq * 100);
        gstUploadInfo.upload_Power_info.r_phase_backup_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.load_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_backup_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.load_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_backup_active_power[1]  = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.a_phase_active));
        gstUploadInfo.upload_Power_info.r_phase_backup_active_power[0]  = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.a_phase_active));
        gstUploadInfo.upload_Power_info.r_phase_backup_reactive_power[1] = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.a_phase_reactive));//TODO: confirm protocol type for upload int
        gstUploadInfo.upload_Power_info.r_phase_backup_reactive_power[0] = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.a_phase_reactive));


    }
    else if(cont == 1)
    {
        gstUploadInfo.upload_Power_info.s_phase_grid_volt = (uint16_t)(gstPh3CalcInfo.grid_vol.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_grid_freq = (uint16_t)(gstPh3CalcInfo.grid_vol.freq * 100);
        gstUploadInfo.upload_Power_info.s_phase_load_volt = (uint16_t)(0 * 10);
        gstUploadInfo.upload_Power_info.s_phase_load_cur = (uint16_t)(0 * 10);

        gstUploadInfo.upload_Power_info.s_phase_oil_volt = (uint16_t)(gstPh3CalcInfo.gen_vol.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_oil_freq = (uint16_t)(gstPh3CalcInfo.gen_vol.freq * 100);
        gstUploadInfo.upload_Power_info.s_phase_oil_curr = (uint16_t)(gstPh3CalcInfo.gen_curr.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_oil_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_oil_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_oil_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.b_phase_active);
        gstUploadInfo.upload_Power_info.s_phase_oil_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.b_phase_active);
        gstUploadInfo.upload_Power_info.s_phase_oil_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.b_phase_reactive);
        gstUploadInfo.upload_Power_info.s_phase_oil_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.b_phase_reactive);

        gstUploadInfo.upload_Power_info.s_phase_inverter_volt = (uint16_t)(gstPh3CalcInfo.inv_vol.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_inverter_curr = (uint16_t)(gstPh3CalcInfo.inv_curr.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_inverter_freq = (uint16_t)(gstPh3CalcInfo.inv_vol.freq * 100);
        gstUploadInfo.upload_Power_info.s_phase_inverter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_inverter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_inverter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.b_phase_active);
        gstUploadInfo.upload_Power_info.s_phase_inverter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.b_phase_active);
        gstUploadInfo.upload_Power_info.s_phase_inverter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.b_phase_reactive);
        gstUploadInfo.upload_Power_info.s_phase_inverter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.b_phase_reactive);

        gstUploadInfo.upload_Power_info.s_phase_backup_volt = (uint16_t)(gstPh3CalcInfo.load_vol.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_backup_curr = (uint16_t)(gstPh3CalcInfo.load_curr.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_backup_fre = (uint16_t)(gstPh3CalcInfo.load_vol.freq * 100);
        gstUploadInfo.upload_Power_info.s_phase_backup_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.load_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_backup_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.load_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_backup_active_power[1]  = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.b_phase_active));
        gstUploadInfo.upload_Power_info.s_phase_backup_active_power[0]  = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.b_phase_active));
        gstUploadInfo.upload_Power_info.s_phase_backup_reactive_power[1] = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.b_phase_reactive));//TODO: confirm protocol type for upload int
        gstUploadInfo.upload_Power_info.s_phase_backup_reactive_power[0] = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.b_phase_reactive));


    }
    else if(cont == 2)
    {
        gstUploadInfo.upload_Power_info.t_phase_grid_volt = (uint16_t)(gstPh3CalcInfo.grid_vol.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_grid_freq = (uint16_t)(gstPh3CalcInfo.grid_vol.freq * 100);
        gstUploadInfo.upload_Power_info.t_phase_load_volt = (uint16_t)(0 * 10);
        gstUploadInfo.upload_Power_info.t_phase_load_cur = (uint16_t)(0 * 10);

        gstUploadInfo.upload_Power_info.t_phase_oil_volt = (uint16_t)(gstPh3CalcInfo.gen_vol.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_oil_freq = (uint16_t)(gstPh3CalcInfo.gen_vol.freq * 100);
        gstUploadInfo.upload_Power_info.t_phase_oil_curr = (uint16_t)(gstPh3CalcInfo.gen_curr.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_oil_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_oil_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_oil_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.c_phase_active);
        gstUploadInfo.upload_Power_info.t_phase_oil_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.c_phase_active);
        gstUploadInfo.upload_Power_info.t_phase_oil_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.c_phase_reactive);
        gstUploadInfo.upload_Power_info.t_phase_oil_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.c_phase_reactive);

        gstUploadInfo.upload_Power_info.t_phase_inverter_volt = (uint16_t)(gstPh3CalcInfo.inv_vol.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_inverter_curr = (uint16_t)(gstPh3CalcInfo.inv_curr.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_inverter_freq = (uint16_t)(gstPh3CalcInfo.inv_vol.freq * 100);
        gstUploadInfo.upload_Power_info.t_phase_inverter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_inverter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_inverter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.c_phase_active);
        gstUploadInfo.upload_Power_info.t_phase_inverter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.c_phase_active);
        gstUploadInfo.upload_Power_info.t_phase_inverter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.c_phase_reactive);
        gstUploadInfo.upload_Power_info.t_phase_inverter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.c_phase_reactive);

        gstUploadInfo.upload_Power_info.t_phase_backup_volt = (uint16_t)(gstPh3CalcInfo.load_vol.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_backup_curr = (uint16_t)(gstPh3CalcInfo.load_curr.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_backup_fre = (uint16_t)(gstPh3CalcInfo.load_vol.freq * 100);
        gstUploadInfo.upload_Power_info.t_phase_backup_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.load_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_backup_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.load_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_backup_active_power[1]  = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.c_phase_active));
        gstUploadInfo.upload_Power_info.t_phase_backup_active_power[0]  = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.c_phase_active));
        gstUploadInfo.upload_Power_info.t_phase_backup_reactive_power[1] = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.c_phase_reactive));//TODO: confirm protocol type for upload int
        gstUploadInfo.upload_Power_info.t_phase_backup_reactive_power[0] = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.c_phase_reactive));
    }
    if(PvVolAveRead(0) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch1_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch1_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[0] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch1_input_volt = (uint16_t)(PvVolAveRead(0) * 10);
        gstUploadInfo.upload_Power_info.pv_ch1_input_curr = (int16_t)(PvCurAveRead(0) * 10);
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[0] = GET_HIGH_16_BITS(PvPwrAveRead(0));
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[1] = GET_LOW_16_BITS(PvPwrAveRead(0));
    }
    if(PvVolAveRead(1) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch2_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch2_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[0] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch2_input_volt = (uint16_t)(PvVolAveRead(1) * 10);
        gstUploadInfo.upload_Power_info.pv_ch2_input_curr = (int16_t)(PvCurAveRead(1) * 10);
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[0] = GET_HIGH_16_BITS(PvPwrAveRead(1));
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[1] = GET_LOW_16_BITS(PvPwrAveRead(1));
    }
    if(PvVolAveRead(2) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch3_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch3_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[0] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch3_input_volt = (uint16_t)(PvVolAveRead(2) * 10);
        gstUploadInfo.upload_Power_info.pv_ch3_input_curr = (int16_t)(PvCurAveRead(2) * 10);
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[0] = GET_HIGH_16_BITS(PvPwrAveRead(2));
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[1] = GET_LOW_16_BITS(PvPwrAveRead(2));
    }
    if(PvVolAveRead(3) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch4_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch4_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[0] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch4_input_volt = (uint16_t)(PvVolAveRead(3) * 10);
        gstUploadInfo.upload_Power_info.pv_ch4_input_curr = (int16_t)(PvCurAveRead(3) * 10);
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[0] = GET_HIGH_16_BITS(PvPwrAveRead(3));
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[1] = GET_HIGH_16_BITS(PvPwrAveRead(3));
    }
    cont++;
    if(cont >= 3)
    {
        cont = 0;
    }
}

/**
* @brief: init_upload_info
* @param: void
* @return:Nothing
* @note:Comm protocol self-test code for integration debug only
*/
// void init_upload_info(UploadInfo_t* info)
// {
//     uint16_t *ptr = (uint16_t*)info;
//     size_t total_size = sizeof(UploadInfo_t);
//     size_t element_count = total_size / sizeof(uint16_t);
    
//     for (size_t i = 0; i < element_count; i++) 
//     {
//         ptr[i] = i;
//     }

//     gstUploadInfo.upload_basic_info.mode_id = 0x1111;   //0x6000
//     gstUploadInfo.upload_basic_info.arc_mode_id  = 0x2222;      //0x6020
//     gstUploadInfo.upload_module_status.sys_input_stat.byte = 0x3333;   //0x6060, PowerInterfaceState
//     gstUploadInfo.upload_Capacity_info.PV_ElectricGenTotal[0] = 0x4444;//0x6080 total PV energy
//     gstUploadInfo.upload_Power_info.r_phase_grid_volt = 0x5555; //0x60B0 R-phase grid voltage
//     gstUploadInfo.upload_fault_info.GridError = 0x6666; //0x61C0 grid fault
//     gstUploadInfo.upload_fault_info.fault_info_reserve[31] = 0x7777;//0x621F
// }


/**
* @brief: SysInfoUpload
* @param: void
* @return:Nothing
* @note:Get readable register info
*/
void SysInfoUpload()
{
    GetPcsModeId();
    GetPcsVersionInfo();
    GetInverterModel();
    GetSysInputType();
    GetSysState();
    GetInvState();
    GetMppt1State();
    GetMppt2State();
    GetMppt3State();
    GetMppt4State();//Whether MPPT values are combined
    GetAlarmCode();
    GetErrorCode();
    GetNoticeCode();
    GetTemperatureValue();
    GetElectricalInfo();
    ElectricInfoGet_UPload();
    //init_upload_info(&gstUploadInfo);  //Comm protocol self-test code
}
