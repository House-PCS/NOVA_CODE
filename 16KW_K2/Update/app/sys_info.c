/*
 * sys_info.c
 *
 *  Created on: 2025年3月20日
 *      Author: Nova006352
 */

#include "sys_info.h"
#include "cla_cpu_share.h"
#include "firmware_upgrate.h"
#include "adc.h"
#include "fault.h"
 
//#include "dev_fan.h"
#include "dev_ntc.h"
#include "ElectricInfoGet.h"
#include "flash_default.h"
#include "AlgoFw/package/cntl/pv/pv.h"

UploadInfo_t    gstUploadInfo;

// typedef struct err_test_str
// {
//     uint16_t    GridError;                          //0x61C0，电网故障
//     uint16_t    PVError[2];                         //0x61C1-0x61C2，Pv故障
//     uint16_t    BusError;                           //0x61C3，母线故障
//     uint16_t    InvError;                           //0x61C4，逆变故障
//     uint16_t    LoadError;                          //0x61C5，负载故障
//     uint16_t    TempError;                          //0x61C6，温度故障
//     uint16_t    GenError;                           //0x61C7，油机故障
//     uint16_t    OtherError;  

// }err_test_t;

// err_test_t err_test;


#define INVERTER_MODEL 0X02
// 宏1：获取32位整数的低16位
#define GET_LOW_16_BITS(value) ((uint16_t)(((int32_t)(value)) & 0x0000FFFF))

// 宏2：获取32位整数的高16位
#define GET_HIGH_16_BITS(value) ((uint16_t)((((int32_t)(value)) >> 16) & 0x0000FFFF))

/**
* @brief: UploadInfoInit
* @param: void
* @return:Nothing
* @note:上传信息初始化
*/
void UploadInfoInit()
{
    memset((uint16_t*)&gstUploadInfo, 0x00, (int)sizeof(UploadInfo_t));
    Init_System_Parameter(); //系统参数初始化
}
/**
* @brief: GetPcsModeId
* @param: void
* @return:Nothing
* @note:获取PCSModeId
*/
void GetPcsModeId()
{
    gstUploadInfo.upload_basic_info.mode_id = gstVersionInfo.pcs_version_info.model_id;
}
/**
* @brief: GetPcsVersionInfo
* @param: void
* @return:Nothing
* @note:获取PCS软硬件版本
*/
void GetPcsVersionInfo()
{
    uint16_t temp[3] = {0x0000,0x0001,0x0005}; //软件版本号：x.y.z，分别存储在temp[0]、temp[1]、temp[2]中
    gstUploadInfo.upload_basic_info.software_ver[1] = (temp[1] << 8)| temp[2];
    gstUploadInfo.upload_basic_info.software_ver[0] = (temp[0] << 8) | (temp[1] >> 8);
//    gstUploadInfo.software_ver = gstVersionInfo.pcs_version_info.soft_version;
    memcpy((uint16_t*)&gstUploadInfo.upload_basic_info.software_ver_Extend, (uint16_t*)&gstVersionInfo.pcs_version_info.extern_version, (int)sizeof(gstVersionInfo.pcs_version_info.extern_version));
}

/**
* @brief: GetInverterModel
* @param: void
* @return:Nothing
* @note:获取逆变型号
*/
void GetInverterModel()
{
    gstUploadInfo.upload_basic_info.hw_version = INVERTER_MODEL;
}
/**
* @brief: GetSysInputType
* @param: void
* @return:Nothing
* @note:获取系统输入类型
*/

void GetSysInputType()
{
    if(Get_cla_Inv_state() == INV_STATE_GRID_CONNECTED)//并网
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.grid_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.grid_input = 0;
    }
    if(Get_cla_Inv_state() == INV_STATE_GEN_CONNECTED)//油机
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.gen_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.gen_input = 0;
    }
    if(Pv_Get_Channel_State(0) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 0;
    }
    if(Pv_Get_Channel_State(1) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv2_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv2_input = 0;
    }
    if(Pv_Get_Channel_State(2) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv3_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv3_input = 0;
    }
    if(Pv_Get_Channel_State(3) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv4_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv4_input = 0;
    }
}
/**
* @brief: GetSysState
* @param: void
* @return:Nothing
* @note:获取系统状态
*/
 void GetSysState()
 {
     //todo：混合并网、旁路、关机状态暂未判断
     if(Get_cla_Inv_state() == INV_STATE_STANDBY)//待机状态
     {
         gstUploadInfo.upload_module_status.inv_status = 0;
     }
     else if(Get_cla_Inv_state() == INV_STATE_SELFTEST)//自检状态
     {
         gstUploadInfo.upload_module_status.inv_status = 1;
     }
     else if(Get_cla_Inv_state() == INV_STATE_DISCONNECTED)
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
 * @note:获取逆变状态
 */
void GetInvState()
{
    if(Get_cla_Inv_state() == INV_STATE_STANDBY)//待机
    {
        gstUploadInfo.upload_module_status.invOnGridState = 0;
    }
    else if(Get_cla_Inv_state() == INV_STATE_DISCONNECTED)//离网
    {
        gstUploadInfo.upload_module_status.invOnGridState = 1;
    }
    else if(Get_cla_Inv_state() == INV_STATE_GRID_CONNECTED || Get_cla_Inv_state() == INV_STATE_GEN_CONNECTED )//并网
    {
        gstUploadInfo.upload_module_status.invOnGridState = 2;
    }
    else if(Get_cla_Inv_state() == INV_STATE_GRID_CONNECTING || Get_cla_Inv_state() == INV_STATE_GEN_CONNECTING)//离->并
    {
        gstUploadInfo.upload_module_status.invOnGridState = 3;
    }
    else if(Get_cla_Inv_state() == INV_STATE_GRID_DISCONNECTING || Get_cla_Inv_state() == INV_STATE_GEN_DISCONNECTING)//并->离
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
* @note:获取Mppt1状态
*/
void GetMppt1State()
{
    gstUploadInfo.upload_module_status.mppt1_status = Pv_Get_Channel_State(0);
}

/**
* @brief: GetMppt2State
* @param: void
* @return:Nothing
* @note:获取Mppt2状态
*/
void GetMppt2State()
{
    gstUploadInfo.upload_module_status.mppt2_status = Pv_Get_Channel_State(1);
}



/**
* @brief: GetMppt3State
* @param: void
* @return:Nothing
* @note:获取Mppt3状态
*/
void GetMppt3State()
{
    gstUploadInfo.upload_module_status.mppt3_status = Pv_Get_Channel_State(2);
}


/**
* @brief: GetMppt4State
* @param: void
* @return:Nothing
* @note:获取Mppt4状态
*/
void GetMppt4State()
{
    gstUploadInfo.upload_module_status.mppt4_status = Pv_Get_Channel_State(3);
}

/**
* @brief: GetAlarmCode
* @param: void
* @return:Nothing
* @note:获取警告码
*/
void GetAlarmCode()
{
    gstUploadInfo.upload_fault_info.GridAlarm = gstAlgoInterface.grid_vol_rms > 20 ? GetGridWarningHighValue() : 0;
    gstUploadInfo.upload_fault_info.PVAlarm[0] = GetPvWarningLowValue();
    gstUploadInfo.upload_fault_info.PVAlarm[1] = GetPvWarningHighValue();
    gstUploadInfo.upload_fault_info.BusAlarm = GetBusWarningValue();
    gstUploadInfo.upload_fault_info.InvAlarm = GetInvWarningValue();
    gstUploadInfo.upload_fault_info.LoadAlarm = GetLoadWarningValue();
    gstUploadInfo.upload_fault_info.TempAlarm = GetTempWarningValue();
    gstUploadInfo.upload_fault_info.GenAlarm  = GetGenWarningValue();
    gstUploadInfo.upload_fault_info.OtherAlarm = GetOtherWarningValue();

}



/**
* @brief: GetErrorCode
* @param: void
* @return:Nothing
* @note:获取故障码
*/
void GetErrorCode()
{
    gstUploadInfo.upload_fault_info.GridError = GetGridErrorValue();
    gstUploadInfo.upload_fault_info.PVError[0] = GetPvErrorLowValue();
    gstUploadInfo.upload_fault_info.PVError[1] = GetPvErrorHighValue();
    gstUploadInfo.upload_fault_info.BusError = GetBusErrorValue();
    gstUploadInfo.upload_fault_info.InvError = GetInvErrorValue();
    gstUploadInfo.upload_fault_info.LoadError = GetLoadErrorValue();
    gstUploadInfo.upload_fault_info.TempError = GetTempErrorValue();
    gstUploadInfo.upload_fault_info.GenError = GetGenErrorValue();
    gstUploadInfo.upload_fault_info.OtherError = GetOtherErrorValue();
}

void GetNoticeCode()
{
    gstUploadInfo.upload_fault_info.GridNotice = 0x0000;
    gstUploadInfo.upload_fault_info.PVNotice[0] = GetPvNoticeHighValue();
    gstUploadInfo.upload_fault_info.PVNotice[1] = GetPvNoticeLowValue();
    gstUploadInfo.upload_fault_info.BusNotice = 0x0000;
    gstUploadInfo.upload_fault_info.InvNotice = 0x0000;
    gstUploadInfo.upload_fault_info.LoadNotice = 0x0000;
    gstUploadInfo.upload_fault_info.TempNotice = 0x0000;
    gstUploadInfo.upload_fault_info.GenNotice = 0x0000;
    gstUploadInfo.upload_fault_info.OtherNotice = GetOtherNoticeValue();
}

/**
* @brief: GetCtOrMeterInfo
* @param: void
* @return:Nothing
* @note:获取故障码
*/

void GetCtOrMeterInfo()
{
    if(MeterModeGet() == 1)//电表
    {
        gstUploadInfo.upload_Power_info.r_phase_meter_curr =  GetMeterCurInfo() * 10;
        gstUploadInfo.upload_Power_info.r_phase_meter_power[1] = GET_LOW_16_BITS(gstAlgoInterface.grid_vol_rms * GetMeterCurInfo());
        gstUploadInfo.upload_Power_info.r_phase_meter_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.grid_vol_rms * GetMeterCurInfo());
        gstUploadInfo.upload_Power_info.r_phase_meter_active_power[1] = GET_LOW_16_BITS(gstAlgoInterface.grid_active_power);
        gstUploadInfo.upload_Power_info.r_phase_meter_active_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.grid_active_power);
        gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gstAlgoInterface.grid_reactive_power);
        gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.grid_reactive_power);
    }
    else//CT信息
    {

        gstUploadInfo.upload_Power_info.r_phase_meter_power[1] = GET_LOW_16_BITS(gstAlgoInterface.ct_apperant_power);
        gstUploadInfo.upload_Power_info.r_phase_meter_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.ct_apperant_power);
        gstUploadInfo.upload_Power_info.r_phase_meter_active_power[1] = GET_LOW_16_BITS(gstAlgoInterface.ct_active_power);
        gstUploadInfo.upload_Power_info.r_phase_meter_active_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.ct_active_power);
        gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gstAlgoInterface.ct_reactive_power);
        gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.ct_reactive_power);
        if(gstAlgoInterface.ct_active_power < 0)
        {
            gstUploadInfo.upload_Power_info.r_phase_meter_curr =  -1 * (int16_t)(gstAlgoInterface.ct_cur_rms * 10);
        }
        else
        {
            gstUploadInfo.upload_Power_info.r_phase_meter_curr = (int16_t)(gstAlgoInterface.ct_cur_rms * 10);
        }
    }
}

/**
* @brief: GetTemperatureValue
* @param: void
* @return:Nothing
* @note:获取故障码
*/

void GetTemperatureValue(void)
{
    if(Get_Temperature_Ready() == 1)
    {
        gstUploadInfo.upload_module_status.chassis_internal_temp = (int16_t)(ChassisTemperatureGet() * 10);
//        gstUploadInfo.upload_module_status.inverter_radiator_temp = (int16_t)(RadiatorTemperatureGet() * 10);
        gstUploadInfo.upload_module_status.inverter_radiator_temp = (int16_t)(GetNTCMaxTemp()*10);
        gstUploadInfo.upload_module_status.pv_temp = (int16_t)(PvTemperatureGet() * 10);
        InvTemperatureGet();
    }
//  gstUploadInfo.  协议缺少逆变器温度，硬件有
}


/**
* @brief: GetErrorCode
* @param: void
* @return:Nothing
* @note:获取各电量信息
*/
void ElectricInfoGet_UPload(void)
{
    //电量统计信息以0.1Kwh为单位传送
    gstUploadInfo.upload_Capacity_info.PV_ElectricGenTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.PV_ElectricGenTotal/100); //PV总发电量
    gstUploadInfo.upload_Capacity_info.PV_ElectricGenTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.PV_ElectricGenTotal/100);

    gstUploadInfo.upload_Capacity_info.elec_purchase_consum_total[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.elec_purchase_consum_total/100); //从电网买电量
    gstUploadInfo.upload_Capacity_info.elec_purchase_consum_total[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.elec_purchase_consum_total/100);

    gstUploadInfo.upload_Capacity_info.elec_sale_consum_total[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.elec_sale_consum_total/100);   //向电网卖电量
    gstUploadInfo.upload_Capacity_info.elec_sale_consum_total[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.elec_sale_consum_total/100);

    gstUploadInfo.upload_Capacity_info.Bat_ElectricChgTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.Bat_ElectricChgTotal/100);  //电池总充电量
    gstUploadInfo.upload_Capacity_info.Bat_ElectricChgTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.Bat_ElectricChgTotal/100);

    gstUploadInfo.upload_Capacity_info.Bat_ElectricDsgTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.Bat_ElectricDsgTotal/100);   //电池总放电量
    gstUploadInfo.upload_Capacity_info.Bat_ElectricDsgTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.Bat_ElectricDsgTotal/100);

    gstUploadInfo.upload_Capacity_info.backup_total_sonsum_power[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.backup_total_consum_power/100);    //负载总用电量
    gstUploadInfo.upload_Capacity_info.backup_total_sonsum_power[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.backup_total_consum_power/100);

    gstUploadInfo.upload_Capacity_info.load_total_sonsum_power[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.load_total_consum_power/100);     //远端负载用电量
    gstUploadInfo.upload_Capacity_info.load_total_sonsum_power[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.load_total_consum_power/100);

    gstUploadInfo.upload_Capacity_info.Gen_ElectricTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.Gen_ElectricTotal/100);     //油机总用电量
    gstUploadInfo.upload_Capacity_info.Gen_ElectricTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.Gen_ElectricTotal/100);

    gstUploadInfo.upload_Capacity_info.EV_ElectricChgTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.EV_ElectricChgTotal/100);    //充电桩给设备总充电量
    gstUploadInfo.upload_Capacity_info.EV_ElectricChgTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.EV_ElectricChgTotal/100);

    gstUploadInfo.upload_Capacity_info.EV_ElectricDsgTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.EV_ElectricDsgTotal/100);     //充电桩给设备总放电量
    gstUploadInfo.upload_Capacity_info.EV_ElectricDsgTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.EV_ElectricDsgTotal/100);

    gstUploadInfo.upload_Capacity_info.PV1_ElectricGenTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.PV1_ElectricGenTotal/100);   //pv1总充电量
    gstUploadInfo.upload_Capacity_info.PV1_ElectricGenTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.PV1_ElectricGenTotal/100);

    gstUploadInfo.upload_Capacity_info.PV2_ElectricGenTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.PV2_ElectricGenTotal/100);   //pv2总充电量
    gstUploadInfo.upload_Capacity_info.PV2_ElectricGenTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.PV2_ElectricGenTotal/100);

    gstUploadInfo.upload_Capacity_info.PV3_ElectricGenTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.PV3_ElectricGenTotal/100);   //pv3总充电量
    gstUploadInfo.upload_Capacity_info.PV3_ElectricGenTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.PV3_ElectricGenTotal/100);

    gstUploadInfo.upload_Capacity_info.PV4_ElectricGenTotal[0] = GET_HIGH_16_BITS(gstInvEnergyStats_Send.PV4_ElectricGenTotal/100);   //pv4总充电量
    gstUploadInfo.upload_Capacity_info.PV4_ElectricGenTotal[1] = GET_LOW_16_BITS(gstInvEnergyStats_Send.PV4_ElectricGenTotal/100);

    //todo:加入PV3 \PV4的电量统计
//    if(gPvPowerHoursComputedFlag == 1)//1min上报更新一次数据
//    {
//        static uint32_t pv1_power_hours_total = 0;
//        static uint32_t pv2_power_hours_total = 0;
//        static uint32_t purchase_power_hours_total = 0;
//        static uint32_t sale_power_hours_total = 0;
//        static uint32_t load_power_hours_total = 0;
//
//        static uint32_t pv1_power_hours_mod = 0;
//        static uint32_t pv2_power_hours_mod = 0;
//        static uint32_t purchase_power_hours_mod = 0;
//        static uint32_t sale_power_hours_mod = 0;
//        static uint32_t load_power_hours_mod = 0;
//
//         /*1h电量+上一小时不满1h电量*/
//        pv1_power_hours_total = gPv1PowerTotalHours / 36000 + pv1_power_hours_mod;
//        pv2_power_hours_total = gPv2PowerTotalHours / 36000 + pv2_power_hours_mod;
//        purchase_power_hours_total = gPurchasePowerTotalHours / 36000 + purchase_power_hours_mod;
//        sale_power_hours_total = gSalePowerTotalHours /36000 + sale_power_hours_mod;
//        load_power_hours_total = gLoadPowerTotalHours / 36000 + load_power_hours_mod;
//
//        /*取整*/
//        pv1_power_genration_total +=  pv1_power_hours_total / 100;//0.1KWH
//        pv2_power_genration_total += pv2_power_hours_total / 100;
//        purchase_power_genration_total += purchase_power_hours_total / 100;
//        sale_power_genration_total += sale_power_hours_total / 100;
//        load_power_genration_total += load_power_hours_total / 100;
//        /*取模*/
//        pv1_power_hours_mod = pv1_power_hours_total % 100;
//        pv2_power_hours_mod = pv2_power_hours_total % 100;
//        purchase_power_hours_mod = purchase_power_hours_total % 100;
//        sale_power_hours_mod = sale_power_hours_total % 100;
//        load_power_hours_mod = load_power_hours_total % 100;
//
//
//        gPvPowerHoursComputedFlag = 0; //外部读取后清0，等待下一次置位累加
//    }
//    if(gSysSetInfo.clean_current_cap == 1)
//    {
//        pv1_power_genration_total = 0;
//        pv2_power_genration_total = 0;
//        purchase_power_genration_total = 0;
//        sale_power_genration_total = 0;
//        load_power_genration_total = 0;
//        bat_chg_power_genration_total = 0;
//        bat_dsg_power_genration_total = 0;
//        gSysSetInfo.clean_current_cap = 0;
//        ModbusReg.clean_current_cap = 0;
//    }
//    gstUploadInfo.pv1_gennerate_total[0] = pv1_power_genration_total
//    gstUploadInfo.pv1_gennerate_total[1] = pv1_power_genration_total >> 16;
//    gstUploadInfo.pv2_gennerate_total[0] = pv2_power_genration_total
//    gstUploadInfo.pv2_gennerate_total[1] = pv2_power_genration_total >> 16;
//    gstUploadInfo.elec_purchase_consum_total[0] = purchase_power_genration_total;
//    gstUploadInfo.elec_purchase_consum_total[1] = purchase_power_genration_total >> 16;
//    gstUploadInfo.elec_sale_consum_total[0] = sale_power_genration_total;
//    gstUploadInfo.elec_sale_consum_total[1] = sale_power_genration_total >> 16;
//    gstUploadInfo.backup_total_sonsum_power[0] = load_power_genration_total;
//    gstUploadInfo.backup_total_sonsum_power[1] = load_power_genration_total >> 16;
}


/**
* @brief: GetElectricalInfo
* @param: void
* @return:Nothing
* @note:获取电气信息
*/
void GetElectricalInfo()
{
    //GetCtOrMeterInfo();
    static uint8_t CT_None = 0;
    static uint8_t oil_mode = 0;
    oil_mode = (gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTED || gstAlgoInterface.inv_state == INV_STATE_GEN_CONNECTING) ? 1 : 0;
    CT_None = GetWarning_Other_R_Ct_Disconnect();

    gstUploadInfo.upload_Power_info.r_phase_grid_volt = (uint16_t)(gstAlgoInterface.grid_vol_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_grid_freq = (gstAlgoInterface.grid_vol_rms>10) ? (uint16_t)(gstAlgoInterface.grid_freq * 100):0;
    gstUploadInfo.upload_Power_info.r_phase_meter_curr = oil_mode ? 0 : (fabs(gstAlgoInterface.ct_active_power) > 50 ? ((gstAlgoInterface.ct_active_power > 0) ? ((gstAlgoInterface.grid_cur_rms * 10)):(-1 * gstAlgoInterface.grid_cur_rms * 10)) : (gstAlgoInterface.grid_cur_rms * 10)); //todo:修改    //GetCtDirection()? gstUploadInfo.upload_Power_info.r_phase_meter_curr = -gstUploadInfo.upload_Power_info.r_phase_meter_curr : gstUploadInfo.upload_Power_info.r_phase_meter_curr;
    gstUploadInfo.upload_Power_info.r_phase_meter_power[1] = oil_mode ? 0 : (CT_None ? GET_LOW_16_BITS(gstAlgoInterface.grid_apperant_power) : GET_LOW_16_BITS(gstAlgoInterface.ct_apperant_power));
    gstUploadInfo.upload_Power_info.r_phase_meter_power[0] = oil_mode ? 0 : (CT_None ? GET_HIGH_16_BITS(gstAlgoInterface.grid_apperant_power) : GET_HIGH_16_BITS(gstAlgoInterface.ct_apperant_power));
    gstUploadInfo.upload_Power_info.r_phase_meter_active_power[1] = oil_mode ? 0 : (CT_None ? GET_LOW_16_BITS(gstAlgoInterface.grid_active_power) : GET_LOW_16_BITS(gstAlgoInterface.ct_active_power));
    gstUploadInfo.upload_Power_info.r_phase_meter_active_power[0] = oil_mode ? 0 : (CT_None ? GET_HIGH_16_BITS(gstAlgoInterface.grid_active_power) : GET_HIGH_16_BITS(gstAlgoInterface.ct_active_power));
    gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[1] = oil_mode ? 0 : (CT_None ? GET_LOW_16_BITS(gstAlgoInterface.grid_reactive_power) : GET_LOW_16_BITS(gstAlgoInterface.ct_reactive_power));
    gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[0] = oil_mode ? 0 : (CT_None ? GET_HIGH_16_BITS(gstAlgoInterface.grid_reactive_power) : GET_HIGH_16_BITS(gstAlgoInterface.ct_reactive_power));


    gstUploadInfo.upload_Power_info.r_phase_ongrid_volt = (uint16_t)(gstAlgoInterface.grid_vol_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_ongrid_freq = (gstAlgoInterface.grid_vol_rms>10) ? (uint16_t)(gstAlgoInterface.grid_freq * 100):0;
    gstUploadInfo.upload_Power_info.r_phase_ongrid_curr = oil_mode ? 0 : (uint16_t)(gstAlgoInterface.grid_cur_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_ongrid_power[1] = oil_mode ? 0 : GET_LOW_16_BITS(gstAlgoInterface.grid_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_ongrid_power[0] = oil_mode ? 0 : GET_HIGH_16_BITS(gstAlgoInterface.grid_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_ongrid_active_power[1] = oil_mode ? 0 : GET_LOW_16_BITS(gstAlgoInterface.grid_active_power);
    gstUploadInfo.upload_Power_info.r_phase_ongrid_active_power[0] = oil_mode ? 0 : GET_HIGH_16_BITS(gstAlgoInterface.grid_active_power);
    gstUploadInfo.upload_Power_info.r_phase_ongrid_reactive_power[1] = oil_mode ? 0 : GET_LOW_16_BITS(gstAlgoInterface.grid_reactive_power);
    gstUploadInfo.upload_Power_info.r_phase_ongrid_reactive_power[0] = oil_mode ? 0 : GET_HIGH_16_BITS(gstAlgoInterface.grid_reactive_power);

//    gstUploadInfo.r_phase_load_power = 1;

    gstUploadInfo.upload_Power_info.r_phase_oil_volt = (uint16_t)(gstAlgoInterface.generator_vol_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_oil_freq = (gstAlgoInterface.generator_vol_rms>10) ? (uint16_t)(gstAlgoInterface.generator_freq * 100):0;
    gstUploadInfo.upload_Power_info.r_phase_oil_curr = (uint16_t)(gstAlgoInterface.generator_cur_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_oil_power[1] = GET_LOW_16_BITS(gstAlgoInterface.generator_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_oil_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.generator_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_oil_active_power[1] = GET_LOW_16_BITS(gstAlgoInterface.generator_active_power);
    gstUploadInfo.upload_Power_info.r_phase_oil_active_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.generator_active_power);
    gstUploadInfo.upload_Power_info.r_phase_oil_reactive_power[1] = GET_LOW_16_BITS(gstAlgoInterface.generator_reactive_power);
    gstUploadInfo.upload_Power_info.r_phase_oil_reactive_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.generator_reactive_power);


    gstUploadInfo.upload_Power_info.r_phase_backup_volt = (uint16_t)(gstAlgoInterface.load_vol_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_backup_curr = (uint16_t)(gstAlgoInterface.load_cur_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_backup_power[1] = GET_LOW_16_BITS(gstAlgoInterface.load_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_backup_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.load_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_backup_active_power[1]  = GET_LOW_16_BITS(fabs(gstAlgoInterface.load_active_power));
    gstUploadInfo.upload_Power_info.r_phase_backup_active_power[0]  = GET_HIGH_16_BITS(fabs(gstAlgoInterface.load_active_power));
    gstUploadInfo.upload_Power_info.r_phase_backup_reactive_power[1] = GET_LOW_16_BITS(fabs(gstAlgoInterface.load_reactive_power));//todo:确定协议类型，上传int
    gstUploadInfo.upload_Power_info.r_phase_backup_reactive_power[0] = GET_HIGH_16_BITS(fabs(gstAlgoInterface.load_reactive_power));

    gstUploadInfo.upload_Power_info.r_phase_load_volt = (uint16_t)(gstAlgoInterface.normalload_vol_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_load_cur = (uint16_t)(gstAlgoInterface.normalload_cur_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_load_power[1] = CT_None ? 0: GET_LOW_16_BITS(gstAlgoInterface.normalload_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_load_power[0] = CT_None ? 0 : GET_HIGH_16_BITS(gstAlgoInterface.normalload_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_load_active_power[1] = CT_None ? 0 : GET_LOW_16_BITS(gstAlgoInterface.normalload_active_power - GetMeterPowerInfo()*100);
    gstUploadInfo.upload_Power_info.r_phase_load_active_power[0] = CT_None ? 0 : GET_HIGH_16_BITS(gstAlgoInterface.normalload_active_power - GetMeterPowerInfo()*100);
    gstUploadInfo.upload_Power_info.r_phase_load_reactive_power[1] = CT_None ? 0 : GET_LOW_16_BITS(gstAlgoInterface.normalload_reactive_power);
    gstUploadInfo.upload_Power_info.r_phase_load_reactive_power[0] = CT_None ? 0 : GET_HIGH_16_BITS(gstAlgoInterface.normalload_reactive_power);


    gstUploadInfo.upload_Power_info.r_phase_ThirdParty_Volt =  gstUploadInfo.upload_Power_info.r_phase_grid_volt;
    gstUploadInfo.upload_Power_info.r_phase_ThirdParty_cur = 0 ; //gstUploadInfo.upload_Power_info.r_phase_meter_curr;//暂未适配读取电表电流功能
    gstUploadInfo.upload_Power_info.r_phase_ThirdParty_freq = gstUploadInfo.upload_Power_info.r_phase_grid_freq;
    gstUploadInfo.upload_Power_info.r_phase_ThirdParty_active_power[1] = CT_None ? 0 : GET_LOW_16_BITS(GetMeterPowerInfo()*100);
    gstUploadInfo.upload_Power_info.r_phase_ThirdParty_active_power[0] = CT_None ? 0 : GET_HIGH_16_BITS(GetMeterPowerInfo()*100);
    gstUploadInfo.upload_Power_info.r_phase_ThirdParty_reactive_power[1] = 0; //暂未适配读取电表无功功能
    gstUploadInfo.upload_Power_info.r_phase_ThirdParty_reactive_power[0] = 0; //暂未适配读取电表无功功能
    gstUploadInfo.upload_Power_info.r_phase_ThirdParty_power[1] = CT_None ? 0 : GET_LOW_16_BITS(GetMeterPowerInfo()*100);
    gstUploadInfo.upload_Power_info.r_phase_ThirdParty_power[0] = CT_None ? 0 : GET_HIGH_16_BITS(GetMeterPowerInfo()*100);


    gstUploadInfo.upload_Power_info.r_phase_inverter_volt = (uint16_t)(gstAlgoInterface.inv_vol_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_inverter_curr = (uint16_t)(gstAlgoInterface.inv_cur_rms * 10);
    gstUploadInfo.upload_Power_info.r_phase_inverter_freq = (gstAlgoInterface.inv_vol_rms>10) ? (uint16_t)(gstAlgoInterface.inv_freq * 100):0;
    gstUploadInfo.upload_Power_info.r_phase_inverter_power[1] = GET_LOW_16_BITS(gstAlgoInterface.inv_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_inverter_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.inv_apperant_power);
    gstUploadInfo.upload_Power_info.r_phase_inverter_active_power[1] = GET_LOW_16_BITS(gstAlgoInterface.inv_active_power);
    gstUploadInfo.upload_Power_info.r_phase_inverter_active_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.inv_active_power);
    gstUploadInfo.upload_Power_info.r_phase_inverter_reactive_power[1] = GET_LOW_16_BITS(gstAlgoInterface.inv_reactive_power);
    gstUploadInfo.upload_Power_info.r_phase_inverter_reactive_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.inv_reactive_power); 

    gstUploadInfo.upload_Power_info.bus_voltage = (uint16_t)(gstAlgoInterface.bus_vol * 10);    

    gstUploadInfo.upload_Power_info.batt_power[1] = GET_LOW_16_BITS(gstAlgoInterface.bus2batpower);
    gstUploadInfo.upload_Power_info.batt_power[0] = GET_HIGH_16_BITS(gstAlgoInterface.bus2batpower);


    if(Pv_Get_Channel_Volt(0) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch1_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch1_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[0] = 0;
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[1] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch1_input_volt = (uint16_t)(Pv_Get_Channel_Volt(0) * 10);
        gstUploadInfo.upload_Power_info.pv_ch1_input_curr = (int16_t)(Pv_Get_Channel_Curr(0) * 10);
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[0] = GET_HIGH_16_BITS(Pv_Get_Channel_Pwr(0));
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[1] = GET_LOW_16_BITS(Pv_Get_Channel_Pwr(0));
    }
    if(Pv_Get_Channel_Volt(1) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch2_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch2_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[0] = 0;
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[1] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch2_input_volt = (uint16_t)(Pv_Get_Channel_Volt(1) * 10);
        gstUploadInfo.upload_Power_info.pv_ch2_input_curr = (int16_t)(Pv_Get_Channel_Curr(1) * 10);
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[0] = GET_HIGH_16_BITS(Pv_Get_Channel_Pwr(1));
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[1] = GET_LOW_16_BITS(Pv_Get_Channel_Pwr(1));
    }
    if(Pv_Get_Channel_Volt(2) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch3_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch3_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[0] = 0;
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[1] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch3_input_volt = (uint16_t)(Pv_Get_Channel_Volt(2) * 10);
        gstUploadInfo.upload_Power_info.pv_ch3_input_curr = (int16_t)(Pv_Get_Channel_Curr(2) * 10);
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[0] = GET_HIGH_16_BITS(Pv_Get_Channel_Pwr(2));
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[1] = GET_LOW_16_BITS(Pv_Get_Channel_Pwr(2));
    }
    if(Pv_Get_Channel_Volt(3) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch4_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch4_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[0] = 0;
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[1] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch4_input_volt = (uint16_t)(Pv_Get_Channel_Volt(3) * 10);
        gstUploadInfo.upload_Power_info.pv_ch4_input_curr = (int16_t)(Pv_Get_Channel_Curr(3) * 10);
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[0] = GET_HIGH_16_BITS(Pv_Get_Channel_Pwr(3));
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[1] = GET_LOW_16_BITS(Pv_Get_Channel_Pwr(3));
    }

//
//    gstUploadInfo.dsp_wake_up_status = 2;

}

/**
* @brief: init_upload_info
* @param: void
* @return:Nothing
* @note:联调通信协议自测试代码，仅联调使用
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
//     gstUploadInfo.upload_module_status.sys_input_stat.byte = 0x3333;   //0x6060，PowerInterfaceState，功率接口状态     
//     gstUploadInfo.upload_Capacity_info.PV_ElectricGenTotal[0] = 0x4444;//0x6080 Pv总发电量
//     gstUploadInfo.upload_Power_info.r_phase_grid_volt = 0x5555; //0x60B0 R 相电网电压
//     gstUploadInfo.upload_fault_info.GridError = 0x6666; //0x61C0，电网故障
//     gstUploadInfo.upload_fault_info.fault_info_reserve[31] = 0x7777;//0x621F
// }


/**
* @brief: SysInfoUpload
* @param: void
* @return:Nothing
* @note:获取可读寄存器信息
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
    GetMppt4State();//mppt是否合在一起
    GetAlarmCode();
    GetErrorCode();
    GetNoticeCode();
    GetTemperatureValue();
    GetElectricalInfo();
    ElectricInfoGet_UPload();
    //init_upload_info(&gstUploadInfo);  //通信协议自测试代码
}
