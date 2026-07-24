/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        algorithm_interface.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-5-24
  * @brief       ????
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-5-24    <td>V1.0.0    <td>dengjiahui    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "algorithm_interface.h"
#include "voltagesourceinvlcfltr.h"
#include <math.h>
#include "authentication_func.h"
#include "algo_err_check.h"
#include "algo_info.h"
#include "Solar_F.h"
#include "dev_sensor.h"
#include "inverter_loop_cntl.h"
#include "energy_management.h"
#include "llc_dcdc_cntl.h"
#include "sci.h"
#include "../cntl/TimeTrigger.h"
#include "pll.h"
#include "cla_algo.h"
#include "inverter_func_def.h"
#include "modbus_analysis.h"
#include "F2837xD_device.h"

#define INV_DELTA (0.008/20000)   //I_coff
//#define  BATPOWER   (gBat_Voltage_Ave*gBat_Current_Ave)

typedef struct sta_data_stru_t
{
    float pwr_grid;
    float pwr_inv;
    float pwr_load;
    float pwr_bat;
    float pwr_pv1;
    float pwr_pv2;

} StaData_t;

static InverterState_t* _gInvSta;
static StaData_t _gSta;
static float32 _garrSciBuf[12];   //SCI_buffer
static float _gInvCurRefStartUp = 0;

static char _gGridDir = 1;
static char _gInvDir = 1;
static char _gCtDir = 1;

float gActivePwrRef = 0;
float gReactivePwrRef = 0;

float grid_power = 0;
float gGridErr;
float gLoadPower = 0;
float gBatpower = 0;
int gTestErr = 0;
extern int gAlgoErr;
extern uint16_t gGridConnectedTime;
static const AlgoInfo_t* _gAlgoInfo;
extern uint16_t Engrid;
extern uint16_t Enoil;
extern float flag_switch;

void AlgoInit()
{
    _gInvSta = InverterGetState();

    gActivePwrRef = 0;
    gReactivePwrRef = -300;
    _gInvCurRefStartUp = InverterGetGridConnectedCur();
    _gAlgoInfo = AlgoStateGet();
}
extern float _gBatVolRefPv, _gGridPwrRefPv;
extern float _gBatVolRefGrid, _gGridPwrRefGrid;
extern float gBat_Voltage_Ave, gBat_Current_Ave;
extern float grid_power_set_diff;
extern float total_load_power;
extern float chg_use_power;
extern float gPvPowerSet;
extern float PVRealTimePower;

extern float count_PV1_150;
extern float count_PV1_200;
extern float count_PV2_150;
extern float count_PV2_200;

extern float flag_PV1_150;
extern float flag_PV1_200;
extern float flag_PV1_good;
extern float flag_PV2_150;
extern float flag_PV2_200;
extern float flag_PV2_good;

extern SysInfoSet_t gSysSetInfo;
extern AlgoInfo_t _gState_algo;
extern float gBatRealPower;
extern float gBatTargetPower; // 20260312，新增变量定义为电池目标充放电功率，用于功率变化率限制计算
extern float gBatSetPower;    // 20260312，修改变量定义为电池执行充放电功率，用于功率变化率限制计算
extern float flag_if;
extern float pv_test1,pv_test2,grid_test;
extern uint16_t gMaxPvPwr;
extern float LoadTotalPower;
extern int32_t gFeedPower;
extern float flag_age_test;

extern float Inv_voltage;
extern float Inv_state;
extern uint16_t gUnConnectRlyCount;


extern float PV1_Current;
extern float PV2_Current;
extern float PV_set_power;
extern SysInfoGet_t gSysInfoGet;

extern float work_mode;
extern float CanReceiveTest_01;
extern float CanReceiveTest_02;
extern float CanReceiveTest_03;
extern float CanReceiveTest_04;
extern float CanReceiveTest_05;
extern float CanReceiveTest_06;
extern float CanReceiveTest_07;

//extern float OnGridState;
extern float count_delay;
//extern float flag_parallel_master;
//extern float flag_parallel_slave;
//extern float flag_disconnect;
extern float InvState;
extern uint16_t gMaxPvPwr;
extern float BatRealTimePower;
extern float LoadBackup;
extern float LoadRemote;
extern float gGridPowerSet;
extern float flag_CT_exist;
extern int16_t gird_activepower;
extern float ct_current_count;
extern int16_t CAN_Master_Send_Master_Workmode;
extern int16_t CAN_Master_Send_Master_Antiflow;
extern int16_t CAN_Master_Send_Master_Export_Limit;
extern int16_t CAN_Master_Send_Master_Import_Limit;
extern int16_t CAN_Master_Send_Slave_AC_Set;
extern int16_t CAN_Master_Receive_Slave_AC_Output_Limit;
extern int16_t CAN_Master_Receive_Slave_AC_Input_Limit;
extern int16_t CAN_Master_Receive_Slave_AC_RealTime;
extern int16_t CAN_Master_Receive_Slave_Backup;
extern int16_t CAN_Master_Receive_Slave_PV_RealTime;
extern int16_t CAN_Master_Receive_Slave_BAT_RealTime;
extern int16_t CAN_Slave_Send_Slave_AC_Output_Limit;
extern int16_t CAN_Slave_Send_Slave_AC_Input_Limit;
extern int16_t CAN_Slave_Send_Slave_AC_RealTime;
extern int16_t CAN_Slave_Send_Slave_Backup;
extern int16_t CAN_Slave_Send_Slave_PV_RealTime;
extern int16_t CAN_Slave_Send_Slave_BAT_RealTime;
extern int16_t CAN_Slave_Receive_Master_Workmode;
extern int16_t CAN_Slave_Receive_Master_Antiflow;
extern int16_t CAN_Slave_Receive_Master_Export_Limit;
extern int16_t CAN_Slave_Receive_Master_Import_Limit;
extern int16_t CAN_Slave_Receive_Slave_AC_Set;

extern float flag_IndependentMode;
extern float flag_MasterMode;
extern float flag_SlaveMode;
extern float WATCH_grid_port_max_power;//最大输出功率
extern float WATCH_grid_port_min_power;//最大输入功率

extern ParallelComm_t gParallelComm;
//extern uint32_t CantxTest1;
//extern uint32_t CantxTest2;
//extern uint32_t cana_test,canb_test;
//extern uint32_t cantxindextest;
extern float master_count, master_count2,master_count3,master_count4,master_count5,master_count6,master_count7;
extern float master_miss_power;
extern uint16_t pv_unstable_test;

extern uint32_t count_BadGrid;
extern uint32_t flag_disconnect;
extern uint32_t gStorageCount1, gStorageCount2;

extern int16_t flag_NetDisableTest_Enable;
extern int16_t flag_HardExportlimit_Enable;
extern int16_t flag_SoftExportlimit_Enable;
extern int16_t flag_PowerRatelimit_Enable;
extern int16_t flag_CertificationTest;

extern float PowerChangeStep;

extern int16_t count_OverSoftExportlimit;
extern int16_t count_OverHardExportlimit;

extern int16_t flag_NetDisableTest_Trigger;
extern int16_t flag_NetDisableTest_Recover;
extern int16_t flag_HardExportlimit_Trigger;
extern int16_t flag_SoftExportlimit_Trigger;
extern int16_t flag_PowerRatelimit_Trigger;

extern float WATCH_LeakCurrent;
extern float WATCH_LeakCurrent_rms;
extern uint16_t count_SlidingWindow;
extern uint16_t flag_SlidingWindow;

extern uint16_t WATCH_flag_warning_LeakCurrent;
extern uint32_t WATCH_count_warning_LeakCurrent;
extern float WATCH_CT_Power;
extern float WATCH_INV_AC_Port_Power;
extern float WATCH_INV_Total_Power; // INV_AC_Port + INV_Backup_Port

extern int16_t WATCH_flag;
extern float WATCH_INV_OUT_POWER;
extern int16_t flag_OffGrid_Trigger;
extern int16_t flag_OffGrid_Completed;
extern float WATCH_Current;

extern int16_t flag_EnableState_new;
extern int16_t flag_EnableState_last;

extern uint8_t gEnInvPwm;
extern uint8_t gEnLlcPwm;
extern EN_PARALLEL_OFF_NETWORK_STA geInvsta;
extern float WATCH_num;

extern CSIP_t gstCSIP;
extern ModbusRegCSIP_t      gstModbusRegCSIP;
//extern float WATCH_geLlcFsm;
//extern float WATCH_gSignE_0;

//extern float Watch_limit_vol_low;
//extern float Watch_limit_vol_high;
//extern float Watch_GridVoltage;
//extern uint32_t count_BadGrid;
//extern uint32_t flag_disconnect;
//extern uint32_t limit_BadGrid;

//// 定义寄存器地址和自定义变量
////volatile uint32_t * const TARGET_REGISTER = (volatile uint32_t *)0x910B;
//volatile uint32_t * const TARGET_REGISTER = (volatile uint32_t *)0x6018;
//volatile uint32_t x = 0;
//
//// 读取寄存器状态的函数
//void read_target_register(void)
//{
//    x = *TARGET_REGISTER;
//}
////
extern float  gNetworkPower;
void* AlgoGetStateWave()
{
    // _garrSciBuf[0]  = gstCSIP.flag_csip_enable;
    // _garrSciBuf[1]  = gstCSIP.flag_import_power_status;
    // _garrSciBuf[2]  = gstCSIP.flag_export_power_status;
    // _garrSciBuf[3]  = gstCSIP.num_import_power_step;
    // _garrSciBuf[4]  = gstCSIP.num_export_power_step;
    // _garrSciBuf[5]  = gstCSIP.target_import_power;
    // _garrSciBuf[6]  = gstCSIP.target_export_power;
    // _garrSciBuf[7]  = gstCSIP.final_safe_import_power_limit;
    // _garrSciBuf[8]  = gstCSIP.final_safe_export_power_limit;
    // _garrSciBuf[9]  = _gState_algo.CT_active_power;
    // _garrSciBuf[10] = gstModbusRegCSIP.CSIP_enable;
    // _garrSciBuf[11] = gNetworkPower;
    if(gstCSIP.flag_import_power_status < 2)
    {
        _garrSciBuf[0]  = (gstCSIP.flag_csip_enable & 0x000F) | ((gstCSIP.flag_import_power_status << 4) & 0x00F0) | ((gstCSIP.flag_export_power_status << 8) & 0x0F00)| ((gstCSIP.disconnect_grid << 12) & 0xF000);
        _garrSciBuf[1]  = gstCSIP.flag_import_ct_source;
        _garrSciBuf[2]  = gstCSIP.num_import_power_ramp_up_value;
        _garrSciBuf[3]  = gstCSIP.flag_import_power_ramp_up_type;
        _garrSciBuf[4]  = gstCSIP.num_import_max_power;
        _garrSciBuf[5]  = gstCSIP.flag_import_ac_source;
        _garrSciBuf[6]  = gstCSIP.flag_ac_in_power_ramp_up_type;
        _garrSciBuf[7]  = gstCSIP.num_ac_in_power_ramp_up_value;
        _garrSciBuf[8]  = gstCSIP.num_ac_in_max_power;
        _garrSciBuf[9]  = gstCSIP.num_import_power_step;
        _garrSciBuf[10] = gstCSIP.target_import_power;
        _garrSciBuf[11] = gstCSIP.final_safe_import_power_limit;
    }
    else 
    {
        _garrSciBuf[0]  = (gstCSIP.flag_csip_enable & 0x000F) | ((gstCSIP.flag_import_power_status << 4) & 0x00F0) | ((gstCSIP.flag_export_power_status << 8) & 0x0F00)| ((gstCSIP.disconnect_grid << 12) & 0xF000);
        _garrSciBuf[1]  = gstCSIP.flag_export_ct_source;
        _garrSciBuf[2]  = gstCSIP.num_export_power_ramp_up_value;
        _garrSciBuf[3]  = gstCSIP.flag_export_power_ramp_up_type;
        _garrSciBuf[4]  = gstCSIP.num_export_max_power;
        _garrSciBuf[5]  = gstCSIP.flag_export_ac_source;
        _garrSciBuf[6]  = gstCSIP.flag_ac_out_power_ramp_up_type;
        _garrSciBuf[7]  = gstCSIP.num_ac_out_power_ramp_up_value;
        _garrSciBuf[8]  = gstCSIP.num_ac_out_max_power;
        _garrSciBuf[9]  = gstCSIP.num_export_power_step;
        _garrSciBuf[10] = gstCSIP.target_export_power;
        _garrSciBuf[11] = gstCSIP.final_safe_export_power_limit;
    }
    return _garrSciBuf;
}
int16_t  GridDirGet(void)
{
    return _gGridDir;
}

int16_t CtDirGet(void)
{
    return _gCtDir;
}



void AlgoCntl()
{
    grid_power = _gSta.pwr_grid * _gGridDir;
    _gSta.pwr_grid = _gInvSta->sta_grid_vol_rms * _gInvSta->sta_grid_cur_rms;
    _gSta.pwr_inv = _gInvSta->sta_inv_vol_rms * _gInvSta->sta_inv_cur_rms;
    _gSta.pwr_load = _gInvSta->sta_load_vol_rms * _gInvSta->sta_load_cur_rms;
    _gSta.pwr_bat = gstSensorData.bat_voltage * gstSensorData.bat_current;
    _gSta.pwr_pv1 = gstSensorData.pv1_voltage * gstSensorData.pv1_current;
    _gSta.pwr_pv2 = gstSensorData.pv2_voltage * gstSensorData.pv2_current;
    gLoadPower = _gSta.pwr_load;
    gBatpower = _gSta.pwr_bat;
    static float theta = 0, theta_last = 0;
    static float cur_ref_last = 0;
    static uint16_t recover_count = 0;
    theta = _gAlgoInfo->inv_vol_theta;
    if(theta > 3.1415926 / 2 && theta_last < 3.1415926 / 2)
    {
        _gGridDir = ((_gAlgoInfo->sensor.grid_vol * _gAlgoInfo->sensor.grid_cur) > 0) ? 1 : -1;
        _gInvDir = ((_gAlgoInfo->sensor.inv_vol * _gAlgoInfo->sensor.inv_cur) > 0) ? 1 : -1;
        _gCtDir = ((_gAlgoInfo->sensor.grid_vol * _gAlgoInfo->sensor.CT_cur) > 0) ? 1 : -1;
    }
    theta_last = theta;

//    OnGridState = _gAlgoInfo->inv_state;///////////////////逆变器并网状态标志位

    if(_gAlgoInfo->inv_state == EN_INV_STATE_DEF_CONNECTED || _gAlgoInfo->inv_state == EN_INV_STATE_DEF_DISCONNECTING)
    {
        float pwr_ref = 0;
        float reactive_ref = 0;
        if(_gAlgoInfo->inv_state == EN_INV_STATE_DEF_CONNECTED)
        {
            pwr_ref = gActivePwrRef;
        }
        else if(_gAlgoInfo->inv_state == EN_INV_STATE_DEF_DISCONNECTING)
        {
            pwr_ref = 0;
        }
        if(gAlgoWarningDef.err.grid_stop == 1)
        {
            InverterSetGridConnectedCur(0);
            recover_count = 0;
            reactive_ref = 0;
        }
        else if(gGridConnectedTime >= 20000)
        {
            if(recover_count < 400)
            {
                recover_count++;
                InverterSetGridConnectedCur(0);
            }
            else
            {
                float inv_cur_ref = cur_ref_last;

            float pwr_gird = _gAlgoInfo->grid_active_power;
            float delta_ref = INV_DELTA * (pwr_ref - pwr_gird);
            float delta_max = 0.2;
            if(((_gInvSta->sta_grid_vol_rms > VRMS_VV2) && (_gInvSta->sta_grid_vol_rms < VRMS_VV3)) && \
               ((_gState_algo.grid_freq < FREQ_ULCO) && ( _gState_algo.grid_freq > FREQ_LLCO))) 
            //    (_gAlgoInfo->inv_state == EN_INV_STATE_DEF_CONNECTED))
            {
                delta_max = 0.2;

//                if(flag_PowerRatelimit_Enable == 1)
//                {
//                    delta_max = 0.000005; // 20260304，功率变化率限制，使用此系数直接作用在电流环，会限制逆变器功率上升和下降的变化率为1/6 Pn/min，Pn = 5kW
//                }
//                else
//                {
//                    delta_max = 0.2;
//                }

            }
            
            if(delta_ref > delta_max)
            {
                delta_ref = delta_max;
            }
            if(delta_ref < -delta_max)
            {
                delta_ref = -delta_max;
            }

                InverterSetGridConnectedCur(inv_cur_ref - delta_ref);
                reactive_ref = gReactivePwrRef;
                cur_ref_last = InverterGetGridConnectedCur();
            }

            float d = InverterGetDeg();
            InverterSetDeg(d + (_gAlgoInfo->inv_reactive_power - reactive_ref) * 0.00005 * 0.00005);
        }
    }
    else
    {
        InverterSetGridConnectedCur(0);
        cur_ref_last = 0;
    }

    //uint16_t pwr_grid=gstSensorData.power_grid_voltage*gstSensorData
    //AlgoGetStateWave();
}

void AlgoSendStateWave()
{
    AlgoGetStateWave();
}

void AlgoTest(uint8_t data)
{
    /*if(gTestErr == 0)
        gTestErr = data;
    else
    {
        gTestErr = 0;
    }
    ScidSendMsg("get data");*/
}

void AlgoSetGridPwr(float pwr)
{

    if((pwr * pwr + gReactivePwrRef * gReactivePwrRef) <= 29700000.0)
    {
        float pwr_gird = _gGridDir * _gSta.pwr_grid;
        if((((pwr - pwr_gird) > 0) && (gstSensorData.bat_voltage < 57)) || (((pwr - pwr_gird) < 0) && (gstSensorData.bat_voltage > 45)))
        {
            gActivePwrRef = pwr;
        }
    }
}

void AlgoSetReactivePwr(float pwr)
{
    if(fabsf(pwr) <= 3800 && ((pwr * pwr + gActivePwrRef * gActivePwrRef) <= 29700000.0))
    {
        gReactivePwrRef = pwr;
    }
}
