/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        comm_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-4-23
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-4-23    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "fault_diagnosis.h"
#include "energy_management.h"
#include "modbus_analysis.h"
#include "algorithm_interface.h"
#include "inverter_loop_cntl.h"
#include "cpu_timers.h"
#include "mppt_process.h"
#include "sys_info.h"
#include "algo_info.h"
#include "algo_err_check.h"
#include "generator.h"
#include "continus_operation.h"
#include "authentication_func.h"
#include "math.h"

#define CSIP_SELF_TEST 0

float  gChgPower = 0;
float  gDsgPower = 0;
int32_t  gFeedPower = 0; // 20260519，补充备注：逆变器执行的CT端最大目标并网卖电功率，恒定正数，例：4000表示执行的从CT端流向电网的最大目标卖电功率为4000W
int32_t  gNetworkPower = 0;
GridCtrl_t gstCsipPowerCtrl;
GridCtrl_t gstModbusPowerCtrl;
uint16_t gNetworkPowerEnable = 0;
//float _Bat_Ref = 0;
float gPvPowerSet = 0;
float gGridPowerSet = 0;
float gGridReactivePowerSet = 0;
float gOilPowerSet = 0;
float gBatRealPower = 0;
float gBatTargetPower = 0; // 20260312，新增变量定义为电池目标充放电功率，用于功率变化率限制计算
float gBatSetPower = 0;    // 20260312，修改变量定义为电池执行充放电功率，用于功率变化率限制计算
uint32_t gOnGridCnt = 0;
uint16_t gRunstate = 0;
uint16_t PvSetRunstate = 0;
uint16_t gEnChgClose = 0;
uint16_t gEnDsgClose = 0;
uint16_t gSyncHostStandby = 0xFFFF;     //0会导致待机

static uint16_t pv_state_old = 0;
static uint16_t pv_start_flag = 0;
static uint16_t pv_startok_flag = 0; //TODO: 测试完成后更改为局部静态变量

static uint16_t _gOnGridOk = 0;
static float gReconnectTime ;
static InverterState_t* _gInvSta;

uint16_t gRelayCloseFlag = 0;
static float _gBatRealPower(float basic);
uint16_t gOnGridTime = 0;
uint8_t  gBatStatusFlag;//0:正常，1:满电，2:空电

uint16_t gBackupSwitchUserSet = 0;
uint16_t gBackupSwitchAutoSet = 0;
extern uint16_t gArcFaultStandbyFlag;
//#define  LOADPOWER  (_gInvSta->sta_load_cur_rms * _gInvSta->sta_load_vol_rms)
//#define  GRID_POWER  (grid_power)
#define  LOADPOWER  (-1 * _gState_algo.load_active_power) // 转化为正数方便计算
#define  NORLOADPOWER  _gState_algo.norload_active_power
#define  GRID_POWER  (_gState_algo.grid_active_power)
#define  GRID_SPOWER  (_gState_algo.grid_pwr)
#define  PV1POWER    (gPv1_Voltage_Ave * gPv1_Current_Ave + gPv2_Voltage_Ave * gPv2_Current_Ave)
#define  OIL_POWER    (GRID_POWER)//油机功率 等于市电功率
//#define  PV2POWER    (gstSensorData.pv2_voltage * gstSensorData.pv2_current)
#define  BATPOWER   (gBat_Voltage_Ave * gBat_Current_Ave)
#define  CHG_THERSHOLD  (56.5)
#define  DSG_THERSHOLD  (47)
#define  CHG_DISCHG_PWR (200)
#define  DISCHG_CHG_PWR (-300)
#define  GRID_VRMS (_gState_algo.grid_vol_rms)

#define BAT_DSG_PWR     (1000)

#define BAT_V gBat_Voltage_Ave

#define VAR_CAP -25.0f
#define VAR_COM_STEP -0.05f
#define Y_CAP 0.0070f  //输出滤波电容在50Hz下导纳
#define CSIP_PROCESS_COUNT 10    //1*100ms执行

static float32 _Var_complete(float32 vrms, float32 spwr, float32 inv_reactive_pwr);
static float32 _Var_slope(float32 grid_power_ref_in);
#define MAX_INV_OUT_POWER _gMaxInvPwrLimit
static float _Bat_power_limit(float charge_power_origin);
static float AntiReFlowSetGridPwr(float grid_pwr);
static void _BatOnlyChgDsgCtrl(void);
static void _Pv_softstart(float grid_power_set, float grid_power, float pv_power_set, float pv_power, float load_power, float chargpower_set);
// EN_PV_WORK_MODE Pv_mode = EN_PV_IDLE;
// EN_INV_BAT_WORK_MODE Bat_mode = EN_INV_BAT_SPONTANEOUS_USE;
// EN_INV_BAT_WORK_MODE Inv_mode = EN_INV_BAT_SPONTANEOUS_USE;
EN_PARALLEL_OFF_NETWORK_STA geInvsta = EN_OFF_GRID_MODE;
EN_POWER_WORK_MODE_STA geEnergysta = EN_OFFGRID_ONLY;

EN_INV_PWR_LEVEL geInvPwrLevel = EN_INV_PWR_LEVEL1;
// EN_CHARGE_INPUT_TYPE charge_mode = EN_FORCE_ELECTRIC;
//123
// EN_POLLING_SERVICE_TYPE polling_service = EN_POLLING_SERVICE_PV;
static uint16_t EnLlc = 0;
static uint16_t EnInv = 0;

uint16_t Engrid = 0;
uint16_t Enoil = 0;

uint16_t gMaxPvPwr = 6000;
uint16_t gMaxInvPwr = 2500;
static float32 _gMaxInvPwrLimit = 2500.0f;

//float PVRealPower = 0;
//float LoadTotalPower = 0;
//float flag_if = 0;
//float flag_switch = 0;
//extern SysInfoSet_t gSysSetInfo;

float PVRealTimePower = 0;
float LoadTotalPower = 0;
float flag_if = 0;
float flag_switch = 0;
extern SysInfoSet_t gSysSetInfo;
extern ModbusRegCSIP_t gstModbusRegCSIP;
float BatRealTimePower = 0;
float LoadBackup = 0;
float LoadRemote = 0;
static bool _gPVGoodFlag[2];
float flag_CT_exist = 0;

ParallelComm_t gParallelComm;

bool ParallelMasterPowerAlloc(uint8_t mode);
bool ParallelSlavePowerAlloc();
void NonParallelDataClear();
void ParallelMasterOffGrid();
void ParallelSlaveOffGrid();
int16_t CAN_Master_Send_Master_Workmode;
int16_t CAN_Master_Send_Master_Antiflow;
int16_t CAN_Master_Send_Master_Export_Limit;
int16_t CAN_Master_Send_Master_Import_Limit;
int16_t CAN_Master_Send_Slave_AC_Set;
int16_t CAN_Master_Receive_Slave_AC_Output_Limit;
int16_t CAN_Master_Receive_Slave_AC_Input_Limit;
int16_t CAN_Master_Receive_Slave_AC_RealTime;
int16_t CAN_Master_Receive_Slave_Backup;
int16_t CAN_Master_Receive_Slave_PV_RealTime;
int16_t CAN_Master_Receive_Slave_BAT_RealTime;
int16_t CAN_Slave_Send_Slave_AC_Output_Limit;
int16_t CAN_Slave_Send_Slave_AC_Input_Limit;
int16_t CAN_Slave_Send_Slave_AC_RealTime;
int16_t CAN_Slave_Send_Slave_Backup;
int16_t CAN_Slave_Send_Slave_PV_RealTime;
int16_t CAN_Slave_Send_Slave_BAT_RealTime;
int16_t CAN_Slave_Receive_Master_Workmode;
int16_t CAN_Slave_Receive_Master_Antiflow;
int16_t CAN_Slave_Receive_Master_Export_Limit;
int16_t CAN_Slave_Receive_Master_Import_Limit;
int16_t CAN_Slave_Receive_Slave_AC_Set;

int16_t flag_NetDisableTest_Enable  = 0;
int16_t flag_HardExportlimit_Enable = 0;
int16_t flag_SoftExportlimit_Enable = 0;
int16_t flag_PowerRatelimit_Enable  = 0;
int16_t flag_CertificationTest = 0;

float PowerChangeStep = 0;

int16_t count_OverSoftExportlimit = 0;
int16_t count_OverHardExportlimit = 0;

int16_t flag_NetDisableTest_Trigger  = 0;
int16_t flag_NetDisableTest_Recover  = 0;
int16_t flag_HardExportlimit_Trigger = 0;
int16_t flag_SoftExportlimit_Trigger = 0;
int16_t flag_PowerRatelimit_Trigger  = 0;

int16_t flag_EnableState_new  = 0;
int16_t flag_EnableState_last = 0;

int16_t WATCH_flag = 0;
float WATCH_INV_OUT_POWER = 0;
int16_t flag_OffGrid_Trigger = 0;
int16_t flag_OffGrid_Completed = 0;
float WATCH_Current = 0;

CSIP_t gstCSIP;


// 定义求最小值的宏
#define MIN_OF_TWO(a, b)  (((a) < (b)) ? (a) : (b)) // 20260519，新增对a,b比较最小值，然后输出最小值，例：MIN_OF_TWO(2000, 5500)输出为2000
#define MIN_OF_THREE_ABS(A, B, C)  MIN_OF_TWO(MIN_OF_TWO(fabsf(A), fabsf(B)), fabsf(C)) // 20260519，新增对A,B,C先分别取绝对值，再比较最小值，最后输出最小的绝对值，例：MIN_OF_THREE_ABS(2000, -1000, 5500)输出为1000
#define SAFE_INT16_ABS_LIMIT(val)  ((val) == -32768 ? -32767 : (val))
#define SAFE_ABS(val) abs(SAFE_INT16_ABS_LIMIT(val))

void PvInvAllowMaxPower(void)
{
    if(gSysSetInfo.pv_max_chg_power >= 7500)
    {
        if(_gPVGoodFlag[0] == true && _gPVGoodFlag[1] == true)
        {
            gMaxPvPwr = 7500;
        }
        else
        {
            gMaxPvPwr = 5000;
        }
    }
    else if(gSysSetInfo.pv_max_chg_power <= 100)
    {
        gMaxPvPwr = 100;
    }
    else
    {
        gMaxPvPwr = gSysSetInfo.pv_max_chg_power;
    }

    if(gSysSetInfo.max_inv_power == 2500)
    {
        geInvPwrLevel = EN_INV_PWR_LEVEL1;
        _gMaxInvPwrLimit = 2500.0f;
    }
    else if(gSysSetInfo.max_inv_power == 5000)
    {
        geInvPwrLevel = EN_INV_PWR_LEVEL2;
        _gMaxInvPwrLimit = 5000.0f;
    }
    else if(gSysSetInfo.max_inv_power == 6000)
    {
        geInvPwrLevel = EN_INV_PWR_LEVEL2;
        _gMaxInvPwrLimit = 5000.0f;
    }
}


float ct_current_count = 0;
uint16_t CheckCtExistsErr(void)
{
    if(geInvsta == EN_GRID_CONNECT_MODE) // 逆变器在并网状态
    {
        if(gSysInfoGet.parallel_mode_stat == 1)  // 并机主机模式
        {
            if(_gState_algo.ct_cur_rms < 0.5)
            {
                ct_current_count++;
                if(ct_current_count > 10)
                {
                    //ModbusReg.parallel_mode_enable = 0;  // 20260205新增，并网并机状态主机CT损坏，主机切换到单机并网无CT模式
                    //gSysSetInfo.parallel_mode_enable = 0;  // 20260205新增，并网并机状态主机CT损坏，主机切换到单机并网无CT模式
                    gParallelComm.parallel_master_forbidden_flag = 1;
                    gSysInfoGet.parallel_mode_stat = 0;  // 20260205新增，并网并机状态主机CT损坏，主机切换到单机并网无CT模式
                    gParallelComm.mode = 0;  // 20260205新增，并网并机状态主机CT损坏，主机切换到单机并网无CT模式

                    gRunstate = 80;
                    ct_current_count = 11;
                    gAlgoErrDef.err.r_phase_ct_break = 1;
                    flag_CT_exist = 0;
                    return 1;
                }
                return 0;
            }
            else
            {
                gRunstate = 81;
                gParallelComm.parallel_master_forbidden_flag = 0;
                ct_current_count = 0;
                gAlgoErrDef.err.r_phase_ct_break = 0;
                flag_CT_exist = 1;
                return 0;
            }
        }
        else if(gSysInfoGet.parallel_mode_stat == 2)  // 并机从机模式
        {
            gAlgoErrDef.err.r_phase_ct_break = 0;
            flag_CT_exist = 0;
        }
        else   // 非并机模式，单机独立运行模式
        {
            if(_gState_algo.ct_cur_rms < 0.5)
            {
                ct_current_count++;
                if(ct_current_count > 10)
                {
                    gRunstate = 80;
                    gParallelComm.parallel_master_forbidden_flag = 1;
                    ct_current_count = 11;
                    gAlgoErrDef.err.r_phase_ct_break = 1;
                    flag_CT_exist = 0;
                    return 1;
                }
                return 0;
            }
            else
            {
                gRunstate = 81;
                gParallelComm.parallel_master_forbidden_flag = 0;
                ct_current_count = 0;
                gAlgoErrDef.err.r_phase_ct_break = 0;
                flag_CT_exist = 1;
                return 0;
            }
        }
    }
    else // 逆变器在离网状态
    {
        gAlgoErrDef.err.r_phase_ct_break = 0;
        flag_CT_exist = 0;
    }

    return 0;
}

void CheckSlaveErr(void)
{
    //放在100ms轮询
    static uint16_t slave_err_count = 0;
    static uint16_t slave_com_err_count = 0;

    if(gAlgoErr == 0) // 逆变器在非故障
    {
        // 主机非故障下且使能并机做从机通信故障判断
        if(gSysSetInfo.parallel_mode_enable == 1)
        {
            if(gSysInfoGet.parallel_mode_stat != 1)
            {
                slave_com_err_count++;
                if(slave_com_err_count > 50)
                {
                    slave_com_err_count = 51;
                    gAlgoErrDef.err.parallel_com_err = 1;
                }
            }
            else
            {
                slave_com_err_count = 0;
                gAlgoErrDef.err.parallel_com_err = 0;
            }
        }
        else    // 非使能并机不做从机通信故障判断
        {
            slave_com_err_count = 0;
            gAlgoErrDef.err.parallel_com_err = 0;
        }

        // 并网状态下且主机识别成功后做从机故障判断
        if(gSysInfoGet.parallel_mode_stat == 1)  // 并机主机模式
        {
            if((gParallelComm.slave_info.slave_grid_port_allow_max_power == 0) && (gParallelComm.slave_info.slave_grid_port_allow_min_power == 0) && \
                    (gParallelComm.slave_info.slave_bat_allow_max_power == 0) && (gParallelComm.slave_info.slave_bat_allow_min_power == 0))
            {
                slave_err_count++;
                if(slave_err_count > 300) // 20260512,更改从机故障触发等待时间，从10秒改为30秒
                {
                    slave_err_count = 301;
                    gAlgoErrDef.err.parallel_slave_err = 1;
                }
            }
            else
            {
                slave_err_count = 0;
                gAlgoErrDef.err.parallel_slave_err = 0;
            }
        }
        else   // 非并机模式，单机独立运行模式
        {
            slave_err_count = 0;
            gAlgoErrDef.err.parallel_slave_err = 0;
        }

    }
    else   // 离网模式，单机独立运行模式
    {
        // 离网模式，不做从机故障判断
        slave_err_count = 0;
        gAlgoErrDef.err.parallel_slave_err = 0;
        // 离网模式不做从机通信故障判断
        slave_com_err_count = 0;
        gAlgoErrDef.err.parallel_com_err = 0;
    }
}

void KeyLedCtrl(void)
{
    if((gSysSetInfo.remote_standby_cmd == 1) || (gArcFaultStandbyFlag == 1) || (gSyncHostStandby == EN_INV_STANDBY)) // 20260514,新增当待机寄存器为1时，控制按键LED亮灭
    {
        GPIO_WritePin(KEY_GPIO, 0); //按键LED灭
    }
    else
    {
        GPIO_WritePin(KEY_GPIO, 1); //按键LED亮
    }
}

float32 ct_change_trend = 0;
float32 inv_change_trend = 0;
float32 grid_change_trend = 0;
float32 grid_dir = 0;
float32 ct_dir = 0;
void CtReverseDetect()
{
    static uint16_t ct_positive_dir_count = 0;
    static uint16_t ct_negative_dir_count = 0;
    static uint16_t inv_positive_dir_count = 0;
    static uint16_t inv_negative_dir_count = 0;
    static uint16_t grid_positive_dir_count = 0;
    static uint16_t grid_negative_dir_count = 0;
    static uint16_t ct_dir_detect_ok = 0;
    static uint16_t ct_filter_ok = 0;
    static uint16_t inv_filter_ok = 0;
    static uint16_t grid_filter_ok = 0;
    static int16_t last_ct_cur = 0;
    static int16_t last_inv_cur = 0;
    static int16_t last_grid_cur = 0;

    grid_dir = GridDirGet();
    ct_dir = CtDirGet();
    int16_t new_ct_cur = ct_dir * _gState_algo.ct_cur_rms;
    int16_t new_inv_cur = grid_dir * _gState_algo.inv_cur_rms;
    int16_t new_grid_cur = grid_dir * _gState_algo.grid_cur_rms;

//    int16_t ct_change_trend = 0;
//    int16_t inv_change_trend = 0;
//    int16_t grid_change_trend = 0;
    gRunstate = 95;
    if(!ct_dir_detect_ok)
    {
        gRunstate = 96;
        if((new_ct_cur > -20) && (new_ct_cur < 20))
        {
            gRunstate = 97;
            ct_change_trend = (new_ct_cur - last_ct_cur > 0) ? 1 : (new_ct_cur - last_ct_cur < 0) ? -1 : 0;
            inv_change_trend = (new_inv_cur - last_inv_cur > 0) ? 1 : (new_inv_cur - last_inv_cur < 0) ? -1 : 0;
            grid_change_trend = (new_grid_cur - last_grid_cur > 0) ? 1 : (new_grid_cur - last_grid_cur < 0) ? -1 : 0;
            if((ct_change_trend != 0) && (ct_filter_ok != 1))
            {
                gRunstate = 98;
                if(ct_change_trend > 0)
                {
                    ct_positive_dir_count ++;
                    ct_negative_dir_count = 0;
                    gRunstate = 99;
                    if(ct_positive_dir_count >= 1)
                    {
                        ct_change_trend = 1;
                        ct_filter_ok = 1;
                        gRunstate = 100;
                    }
                }
                if(ct_change_trend < 0)
                {
                    ct_positive_dir_count = 0;
                    ct_negative_dir_count ++;
                    gRunstate = 101;
                    if(ct_negative_dir_count >= 1)
                    {
                        ct_change_trend = -1;
                        ct_filter_ok = 1;
                        gRunstate = 102;
                    }
                }
            }
//            if((inv_change_trend != 0) && (inv_filter_ok != 1))
//            {
//                gRunstate = 103;
//                if(inv_change_trend > 0)
//                {
//                    inv_positive_dir_count ++;
//                    inv_negative_dir_count = 0;
//                    gRunstate = 104;
//                    if(inv_positive_dir_count >= 1)
//                    {
//                        inv_change_trend = 1;
//                        inv_filter_ok = 1;
//                        gRunstate = 105;
//                    }
//                }
//                if(inv_change_trend < 0)
//                {
//                    inv_positive_dir_count = 0;
//                    inv_negative_dir_count ++;
//                    gRunstate = 106;
//                    if(inv_negative_dir_count >= 1)
//                    {
//                        inv_change_trend = -1;
//                        inv_filter_ok = 1;
//                        gRunstate = 107;
//                    }
//                }
//
//            }

            if((grid_change_trend != 0) && (grid_filter_ok != 1))
            {
                gRunstate = 108;
                if(grid_change_trend > 0)
                {
                    grid_positive_dir_count ++;
                    grid_negative_dir_count = 0;
                    gRunstate = 109;
                    if(grid_positive_dir_count >= 1)
                    {
                        grid_change_trend = 1;
                        grid_filter_ok = 1;
                        gRunstate = 110;
                    }
                }
                if(grid_change_trend < 0)
                {
                    grid_positive_dir_count = 0;
                    grid_negative_dir_count ++;
                    gRunstate = 111;
                    if(grid_negative_dir_count >= 1)
                    {
                        grid_change_trend = -1;
                        grid_filter_ok = 1;
                        gRunstate = 112;
                    }
                }
            }
            last_ct_cur = new_ct_cur;
            last_inv_cur = new_inv_cur;
            last_grid_cur = new_grid_cur;
//            if((inv_filter_ok == 1) && (ct_filter_ok == 1))
//            {
//                if(inv_change_trend == ct_change_trend)
//                {
//                   gstFaultInfo.worning_code.b.r_ct_reverse = 0;
//                   gRunstate = 108;
//                }
//                else
//                {
//                   gstFaultInfo.worning_code.b.r_ct_reverse = 1;
//                   gRunstate = 109;
//                }
//                ct_dir_detect_ok = 1;
//            }
            if((grid_filter_ok == 1) && (ct_filter_ok == 1))
            {
                if(grid_change_trend == ct_change_trend)
                {
                    gstFaultInfo.worning_code.b.r_ct_reverse = 0;
                    gRunstate = 113;
                }
                else
                {
                    gstFaultInfo.worning_code.b.r_ct_reverse = 1;
                    gRunstate = 114;
                }
                ct_dir_detect_ok = 1;
            }
        }
    }
}




static float AntiReFlowSetGridPwr(float grid_pwr)
{
    float ct_power = 0;
    float grid_power = 0;
    float power_gap = 0;
    ct_power = _gState_algo.CT_active_power;
    grid_power = _gState_algo.grid_active_power;
    power_gap = grid_power - ct_power;

    return power_gap;
}


void UserBackupSwitchSet(void)//用户设置backup开关
{
    if(gSysSetInfo.backup_output_enable == 1)
    {
        gRunstate = 200;
        gBackupSwitchUserSet = 1;
        if(gRelayCloseFlag == 0)//放电深度满足要求
        {
            gRunstate = 201;
            AlgoEnDischarge(1);//打开负载继电器
            if(gSysSetInfo.charging_input_typesetting == 1)
            {
                RelaySetValue(EN_RELAY_DEF_LOAD, 1);
            }
        }
    }
    else
    {
        gRunstate = 202;
        AlgoEnDischarge(0);//关闭负载继电器
        if(gSysSetInfo.charging_input_typesetting == 1)
        {
            RelaySetValue(EN_RELAY_DEF_LOAD, 0);
        }
        gBackupSwitchUserSet = 0;
    }
}


static void _BatOnlyChgDsgCtrl(void)
{
    gBatSetPower = gChgPower*(-1);
    if(gSysSetInfo.pcs_dsg_ctrl == 0)//禁止放电，断开负载
    {
        AlgoEnDischarge(0);
        gRelayCloseFlag = 1;
    }
    else
    {
        if(gBackupSwitchUserSet == 1)//自动打开前，确保用户是设置负载继电器开启状态
        {
            AlgoEnDischarge(1);
            gRelayCloseFlag = 0;
        }
    }
    if(gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0)
    {
        gBatSetPower = CHG_DISCHG_PWR;
    }
    gBatRealPower = _gBatRealPower(gBatSetPower);
    gPvPowerSet = LOADPOWER - gBatRealPower;
//     if(gPvPowerSet < 0)
//     {
//         gPvPowerSet = 0;
//     }
//     if(gPvPowerSet > 5000)
//     {
//         gPvPowerSet = 5000;
//     }
    gPvPowerSet = VALUE_LIMIT(gPvPowerSet, DUAL_PV_MAX_POWER, 0);
}

void OffGridHandle(void)
{
    static uint32_t delay_time = 0;
    static uint16_t i = 0;
    AlgoSetGridPwr(0);

    WATCH_Current = fabsf(_gInvSta->sta_grid_cur_rms);

    if(TimDifftimeFromNow(delay_time) > 100)
    {
        if(fabsf(_gInvSta->sta_grid_cur_rms) < 10)
//        if(fabsf(_gState_algo.grid_active_power) < 200)
        {
            i++;
        }
        else
        {
            i = 0;
        }
        if(i > 5)
        {
            InverterSetGridConnect(0);
            flag_OffGrid_Completed = 1; // 20260319，新增逆变器主动脱网完成标志位，脱网完成后置1
        }
        delay_time = TimGetSystime();
    }
}

uint16_t ThreadBreakDetect(void)
{
    if(gSysSetInfo.charging_input_typesetting == 1)
    {
        return 0;
    }
    if(gSysSetInfo.ignore_ct_err == 0) //ignore_ct_err
    {
        if(gSysSetInfo.ct_or_meter == 1)//电表
        {
            if(((gSysSetInfo.meter_brand & 0x8000) != 0) || (ReadContinusOperationState() == 1))
            {
                OffGridHandle();//脱网处理
                return 1 ;
            }
        }
        if(gSysSetInfo.ct_or_meter == 0)//默认CT
        {
            if(geInvsta == EN_GRID_CONNECT_MODE)
            {
//                if((CheckCtExistsErr() == 1) || (ReadContinusOperationState() == 1))
                if(ReadContinusOperationState() == 1)
                {
//                    flag_CT_exist = 0;
                    OffGridHandle();//脱网处理
                    gRunstate = 82;
                    return 1;
                }
                else
                {
//                    if(abs(_gState_algo.CT_active_power) >= 800)
//                    {
//                        CtReverseDetect();
//                        gRunstate = 83;
//                    }
//                    flag_CT_exist = 1;
                    return 0;
                }
            }
        }
        return 0;
    }
    else
    {
        return 0;
    }
}

uint16_t DeviceSwitchCtrl()
{
    static uint32_t sn_time = 0;
    static uint32_t delay_time3 = 0;
    static uint16_t i = 0;
    if((gArcFaultStandbyFlag != 1) && (gSysSetInfo.remote_standby_cmd != 1) && ((gSysSetInfo.pcs_dsg_ctrl != 0)||(gSysSetInfo.pcs_chg_ctrl != 0)) && (gDeviceUpFlag == 0) &&(gSyncHostStandby != EN_INV_STANDBY))
    {
        if(!EnLlc)
        {
            gEnLlcPwm = 1;
            EnLlc = 1;
            sn_time = TimGetSystime();
        }
        if(!EnInv)
        {
            if((TimGetSystime() - sn_time) > 5000)
            {
                gEnInvPwm = 1;
                EnInv = 1;
                return 1 ;
            }
            return 0;
        }
        else
        {
            return 1 ;
        }
    }
    else
    {
        if(EnLlc||EnInv)
        {
            MpptSetPv1Pwr(0);
            AlgoSetGridPwr(0);
            if(TimDifftimeFromNow(delay_time3) > 100)
            {
                if(fabsf(_gInvSta->sta_grid_cur_rms) < 10)
                {
                    i++;
                }
                else
                {
                    i = 0;
                }
                if(i > 5)
                {
                    InverterSetGridConnect(0);
                    if(geInvsta == EN_OFF_GRID_MODE)
                    {
                        gEnInvPwm = 0;
                        gEnLlcPwm = 0;
                        EnLlc = 0;
                        EnInv = 0;
                    }
                }
                delay_time3 = TimGetSystime();
            }
        }
        return 0;
    }
}




/**
* @brief: EnergyInputModeSwitch
* @param: void
* @return:
* @note:
*能量输入模式切换
*油机模式/市电模式*/
void EnergyInputModeSwitch()
{
    static uint16_t charging_input_type = 0;
    static uint16_t delay_cnt = 0;

    InputModeChange();//输入模式切换函数

    switch(charging_input_type)
    {
        case 0://输入类型为市电类型
        {
            if(gSysSetInfo.charging_input_typesetting == 1)//强制油机模式
            {
                Engrid = 0;
                gPvPowerSet = 0;
                gGridPowerSet = 0;
                //断继电器流程
                AlgoSetGridPwr(0);
                if(fabsf(_gInvSta->sta_grid_cur_rms) < 10)
                {
                    delay_cnt ++;
                }
                else
                {
                    delay_cnt = 0;
                }
                if(delay_cnt > 5)
                {
                    InverterSetGridConnect(0);
                }

                if(GetGridToGenFinish() == 1)//算法层需返回切换完成标志位，作为判断依据。满足才可进入油机调度业务 2.油机与市电切换完成标志位可用一个
                {
                    Enoil = 1;
                    Engrid = 0;
                    delay_cnt = 0;
                    charging_input_type = 1;
                }
            }
            else
            {
                Enoil = 0;
                Engrid = 1;
                delay_cnt = 0;
            }
            break;
        }
        case 1://输入类型为强制油机
        {
            if(gSysSetInfo.charging_input_typesetting == 0)//市电模式
            {
                Enoil = 0;
                gPvPowerSet = 0;
                gGridPowerSet = 0;
                //断继电器流程
                AlgoSetGridPwr(0);
                if(fabsf(_gInvSta->sta_grid_cur_rms) < 10)
                {
                    delay_cnt ++;
                }
                else
                {
                    delay_cnt = 0;
                }
                if(delay_cnt > 5)
                {
                    InverterSetGridConnect(0);
                }

                if(GetGenToGridFinish() == 1)//1.算法层需返回切换完成标志位，作为判断依据，满足才可进入市电调度业务；2.油机与市电切换完成标志位可用一个
                {
                    Engrid = 1;
                    Enoil = 0;
                    delay_cnt = 0;
                    charging_input_type = 0;
                    //打开负载继电器
                    AlgoEnDischarge(1);
                    gSysSetInfo.backup_output_enable = 1;
                    ModbusReg.backup_output_enable = 1;
                    gRelayCloseFlag = 0;
                    RelaySetValue(EN_RELAY_DEF_MAIN, 1);
                }
            }
            else
            {
                Enoil = 1;
                Engrid = 0;
                delay_cnt = 0;
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
/**
* @brief: _BackupRelaySwitch
* @param: void
* @return:
* @note:
*油机模式下输出不足工况
*执行动作为关闭负载继电器*/
// static void _BackupRelaySwitch()
// {
//     static uint32_t relay_close_timer = 0;
//     static uint16_t relay_close_flag = 0;
//
//     if(gSysSetInfo.pcs_dsg_ctrl == 0)//禁止放电
//     {
//         if(relay_close_flag == 0)
//         {
//             //PV功率和油机功率先降为0后切负载
//             gPvPowerSet = 0;
//             gGridPowerSet = 0;//油机或市电功率置0
//             if((PV1POWER < 500) && (OIL_POWER < 100))//同时检测到实时功率低于阈值，关负载继电器
//             {
//                 AlgoEnDischarge(0);//关闭负载继电器
//                 gSysSetInfo.backup_output_enable = 0;
//                 ModbusReg.backup_output_enable = 0;
//                 relay_close_flag = 1;
//                 relay_close_timer = TimGetSystime();
//             }
//         }
//     }
//     else
//     {
//         if(relay_close_flag == 1)
//         {
//             if(gSysSetInfo.backup_output_enable == 1)
//             {
//                 if(TimGetSystime() - relay_close_timer > 20000)
//                 {
//                     AlgoEnDischarge(1);
//                     relay_close_flag = 0;
//                 }
//             }
//         }
//     }
// }

/**
* @brief: 判断当前光伏是否真正有效
* @param: none
* @return: none
* @note:
*/

static bool _PVValidJudge()
{
    static uint16_t count[2];//两路光伏
    static float pv_voltage[2][5];//保存各路光伏的连续五次采样值
//  static bool pv_good_flag[2];

    float   pv_valid_threshold_level_1 = 70;//工作截止电压
    float   pv_valid_threshold_level_2 = 155;//启动电压
    float   pv_valid_threshold_level_3 = 300;//稳定电压
    float   count_max = 1000;
    bool    ret = true;


    uint16_t hist_volt_num = sizeof(pv_voltage[0])/sizeof(float);
    uint16_t pv_num = sizeof(count)/sizeof(uint16_t);
    uint16_t i, j;

    pv_voltage[0][0] = gPv1_Voltage_Ave;
    pv_voltage[1][0] = gPv2_Voltage_Ave;
    for(i = 0; i < pv_num; i++)
    {
        for(j = 0; j < hist_volt_num; j++)
        {
            if(pv_voltage[i][j] < pv_valid_threshold_level_2)
            {
                break;
            }
        }
        if(j == hist_volt_num)//连续多次电压均不低于pv_valid_threshold_level_2
        {
            if(count[i] < count_max)
            {
                count[i]++;
            }
        }
        else
        {
            if(count[i] > 0)
            {
                count[i]--;
            }
        }
        if(count[i] == count_max)
        {
            _gPVGoodFlag[i] = true;
        }
        else if(count[i] == 0)
        {
            //pv_good_flag[i] = false;
        }

        for(j = 0; j < hist_volt_num; j++)
        {
            if(pv_voltage[i][j] < pv_valid_threshold_level_3)
            {
                break;
            }
        }
        if(j == hist_volt_num)//电压达到稳定值，强制使能(历史数据均大于等于阈值触发)
        {
            _gPVGoodFlag[i] = true;
        }
        for(j = 0; j < hist_volt_num; j++)
        {
            if(pv_voltage[i][j] < pv_valid_threshold_level_1)
            {
                break;
            }
        }
        if(j < hist_volt_num)//电压达到截止值，强制禁能(历史数据有一次低于阈值触发)
        {
            _gPVGoodFlag[i] = false;
        }
    }
    for(i = 0; i < pv_num; i++)
    {
        if(_gPVGoodFlag[i] == true)
        {
            break;
        }
    }
    if(i == pv_num)
    {
        ret = false;
    }
    for(i = 0; i < pv_num; i++)
    {
        for(j = hist_volt_num - 1; j >= 1; j--)
        {
            pv_voltage[i][j] = pv_voltage[i][j - 1];
        }
    }

    // pv1Ave = pv_voltage[0][0];
    return ret;
}


/**
* @brief: OilEngineMode
* @param: void
* @return:
* @note:
*强制油机调度模式*/
void OilEngineMode()
{
    static float pvpowersettemp = 0;
    bool pv_volt_valid_flag = _PVValidJudge();
    float pv_power;
    if(pv_volt_valid_flag == true)
    {
        pv_power = PV1POWER;
        if(pv_power < 0)
        {
            pv_power = 0;
        }
    }
    else
    {
        pv_power = 0;
    }

    /*************/
    /*电池功率给定*/
    if((gSysSetInfo.pcs_dsg_ctrl == 0) || (GetGenLoadFault() == 1))//禁止放电，断开负载
    {
        gRunstate = 900;
        AlgoEnDischarge(0);
        RelaySetValue(EN_RELAY_DEF_LOAD, 0);
        gRelayCloseFlag = 1;
    }
    else
    {
        if(gBackupSwitchUserSet == 1)//自动打开前，确保用户是设置负载继电器开启状态
        {
            gRunstate = 901;
            AlgoEnDischarge(1);
            RelaySetValue(EN_RELAY_DEF_LOAD, 1);
            gRelayCloseFlag = 0;
        }
    }

    gGridReactivePowerSet = 0;//油机下逆变无功输出给定0
    gBatSetPower = gChgPower*(-1);
    if((gSysSetInfo.pcs_chg_ctrl == 0)||(gBatSetPower == 0))//禁止充电或充电功率为0
    {
        gBatSetPower = CHG_DISCHG_PWR;
    }

    gBatRealPower = _gBatRealPower(gBatSetPower);//PCS端防止电池充过压/放欠压进而限制给定充放电功率


    /***************/
    /*PV和油机功率给定*/
    gOilPowerSet = - pv_power + LOADPOWER - gBatRealPower;
    if((gOilPowerSet < 0)//油机不存在负功率给定
            ||(geInvsta == EN_OFF_GRID_MODE))
    {
        gOilPowerSet = 0;
//          _BatOnlyChgDsgCtrl();
    }
    if(gOilPowerSet >= 5000)//油机功率限值
    {
        gOilPowerSet = 5000;
    }
//      gPvPowerSet = LOADPOWER - gBatRealPower - gOilPowerSet + 100;//1.PV给定 2.+100使PV一直往上扰
//      if(gPvPowerSet < 0)//PV不存在负功率给定
//      {
//          gPvPowerSet = 0;
//      }
    pvpowersettemp = LOADPOWER - gBatRealPower - gOilPowerSet + 100;//1.PV给定 2.+100使PV一直往上扰
    if(pvpowersettemp > gPvPowerSet)
    {
        gPvPowerSet += 20;
    }
    else
    {
        gPvPowerSet = pvpowersettemp;
    }
    if((GetGenVoltNormal() == 1) && (GetInvRunState() != EN_INV_STATE_DEF_CONNECTED))//发电机电压正常，逆变非并网状态，PV功率设为0
    {
        gPvPowerSet = 0;
    }
    if(gPvPowerSet < 0)//PV不存在负功率给定
    {
        gPvPowerSet = 0;
    }
    if(gOilPowerSet > gGridPowerSet)//避免直接给大功率
    {
        gGridPowerSet += 5;  //预计100s达到最大
    }
    else
    {
        gGridPowerSet = gOilPowerSet;
    }

    if(gGridPowerSet >= 5000)
    {
        gGridPowerSet = 5000;
    }
}
static float _gBatRealPower(float basic)
{
    static float _BatChgPowerOffSet = 0;
    static float _BatDsgPowerOffSet = 0;
    float BatRealPower = 0;
    if(basic < 0)
    {
        if(BAT_V > CHG_THERSHOLD)
        {
            _BatChgPowerOffSet += (BAT_V - CHG_THERSHOLD) *5;
            gBatStatusFlag = 1;
        }
        else
        {
            _BatChgPowerOffSet *= 0.999;
            gBatStatusFlag = 0;
        }
        BatRealPower = basic + _BatChgPowerOffSet;
        _BatDsgPowerOffSet = 0;
    }
    if(basic > 0)
    {
        if(BAT_V < DSG_THERSHOLD)
        {
            _BatDsgPowerOffSet += (BAT_V - DSG_THERSHOLD) *5;
            gBatStatusFlag = 2;
        }
        else
        {
            _BatDsgPowerOffSet *= 0.999;
            gBatStatusFlag = 0;
        }
        BatRealPower = basic + _BatDsgPowerOffSet;
        _BatChgPowerOffSet = 0;
    }
    return BatRealPower;
}

float grid_power_set_diff = 0; //GirdPowerSet实际差值
//float total_load_power;
float chg_use_power;

void SpontaneousUse()
{
    static uint32_t delay_time = 0;
    static uint32_t delay_time1 = 0;
    static uint32_t full_chg_flag = 0;
    static uint32_t full_chg_count = 0;
    float inv_output_power = 0;
    float inv_input_actpwr = 0;
    float grid_power_set_compensate = 0; //补偿后的GirdPowerSet
    float chg_max_power = gChgPower;
    uint32_t chg_disable_flag = 0;
//    float grid_power_set_diff = 0; //GirdPowerSet实际差值
    float total_load_power;
    grid_power_set_diff = 0;
    bool pv_volt_valid_flag = _PVValidJudge();
    float on_grid_max_power = 0;
    switch(geInvsta)
    {
        case EN_OFF_GRID_MODE:
        {
            gRunstate = 1;
            gGridPowerSet = 0;
            gGridReactivePowerSet = 0;
            // AlgoSetGridPwr(0);
            _BatOnlyChgDsgCtrl();
            if(TimDifftimeFromNow(delay_time) > gReconnectTime)
            {
                if((ThreadBreakDetect() != 1) && (ReadContinusOperationState() == 0) && \
                        ((gstModbusRegCSIP.CSIP_enable == 0) || ((gstModbusRegCSIP.CSIP_enable == 1) && ((gstModbusRegCSIP.CSIP_run_ctrl == 0) || (gstModbusRegCSIP.CSIP_run_ctrl == 2)))))  //CSIP允许并网
                {
                    gRunstate = 2;
                    InverterSetGridConnect(1);
                }
                else
                {
                    InverterSetGridConnect(0);
                }
                delay_time = TimGetSystime();
            }
            else
            {
                InverterSetGridConnect(0);
            }
            delay_time1 = TimGetSystime();
            _gOnGridOk = 0;

            ParallelMasterOffGrid(); // 20260513,新增并机主机离网状态把远端负载交给从机出力
            ParallelSlaveOffGrid();  // 20260513,新增并机从机离网状态并网口的最大输入输出功率清零
            break;
        }
        case EN_OFF_GRID_TO_GRID_CONNECT_MODE:
        {
            gRunstate = 3;
            if((TimGetSystime() - delay_time1) > 10000)
            {
                gRunstate = 4;
                _gOnGridOk = 0;
                InverterSetGridConnect(0);
                delay_time = TimGetSystime();
            }
            break;
        }
        case EN_GRID_CONNECT_MODE:
        {
            bool pv_inc_flag = false;
            if(gRelayCloseFlag == 1)
            {
                gRelayCloseFlag = 0;
            }
            if(_gOnGridOk == 0)
            {
                gOnGridCnt ++;
                _gOnGridOk = 1;
                gOnGridTime = TimGetSystime() - delay_time1;
            }
            if(gParallelComm.mode > 2)
            {
                gParallelComm.mode = 0;
            }
            if(gParallelComm.mode == 0)
            {
                if(Read_Mppt_state())
                {
                    float pv_power;
                    gRunstate = 5;
                    if(pv_volt_valid_flag == true)
                    {
                        pv_power = PV1POWER;
                        if(pv_power < 0)
                        {
                            pv_power = 0;
                        }
                    }
                    else
                    {
                        pv_power = 0;
                    }
                    total_load_power = (-gAntiReFlowValue) + LOADPOWER;
                    total_load_power = VALUE_LIMIT(total_load_power, 5000, -5000);
                    LoadTotalPower = total_load_power;

                    gBatTargetPower = total_load_power - pv_power; // 20260312，新增功率变化率限制
//                  gBatSetPower = total_load_power - pv_power;
                    if(flag_PowerRatelimit_Trigger == 1)
                    {
                        PowerChangeStep = 1.38; // 20260317，开启功率变化率限制，逆变器功率上升和下降的变化率为为每100ms变化1.38W，即13.8W/s
                        if((gBatSetPower + 5) <= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower + PowerChangeStep;
                        }
                        else if((gBatSetPower - 5) >= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower - PowerChangeStep;
                        }
                        else
                        {
                            gBatSetPower = gBatTargetPower;
                        }
                    }
                    else
                    {
                        gBatSetPower = gBatTargetPower;
                    }

                    chg_use_power = chg_max_power;

                    if(gBatSetPower <= (chg_max_power*(-1) + CHG_DISCHG_PWR))
                    {
                        gRunstate = 6;
                        if(chg_max_power == 0 || gSysSetInfo.pcs_chg_ctrl == 0)
                        {
                            chg_disable_flag = 1;
                            gBatSetPower = CHG_DISCHG_PWR;
                        }
                        else if(gBatSetPower <= (chg_max_power*(-1)))
                        {
                            gBatSetPower = chg_max_power*(-1);
                        }
                    }
                    if(gBatSetPower >= (gDsgPower + DISCHG_CHG_PWR))
                    {
                        gRunstate = 7;
                        if (gDsgPower == 0)
                        {
                            gBatSetPower = DISCHG_CHG_PWR;
                        }
                        else if(gBatSetPower > gDsgPower)
                        {
                            gBatSetPower = gDsgPower;
                        }
                    }
                    if(gSysSetInfo.pcs_dsg_ctrl == 0)
                    {
                        gRunstate = 8;
                        if(gBatSetPower > 0)
                        {
                            gBatSetPower = DISCHG_CHG_PWR;
                        }
                    }
                    gBatRealPower = _gBatRealPower(gBatSetPower);

                    gGridPowerSet = -pv_power + LOADPOWER - gBatRealPower;
                    grid_power_set_compensate = gGridPowerSet;
					
					on_grid_max_power = gFeedPower;
					if(gSysSetInfo.antireverseflow_enable == 1)
					{
						if(on_grid_max_power > 0)
						{
							on_grid_max_power = 0;
						}
					}

					if(gGridPowerSet < (-1)*on_grid_max_power + gAntiReFlowValue)//并网功率过大
					{
						gRunstate = 46;
                        if(gBatStatusFlag == 1)//电池满电无法吸收，当前管不了电网，下一拍降光伏，如无光伏，则直接并给电网
                        {
                            //grid_power_set_diff += ((-1)*on_grid_max_power + gAntiReFlowValue) - gGridPowerSet;
                        }
                        else if(chg_disable_flag == 1)
                        {
                            if(gBatRealPower - CHG_DISCHG_PWR >= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet)
                            {
                                gBatRealPower -= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet;
                            }
                            else
                            {
                                gBatRealPower = CHG_DISCHG_PWR;
                            }
                            gGridPowerSet = (-1)*on_grid_max_power + gAntiReFlowValue;
                        }
                        else//电池不满电，可以由设备吸收
                        {
                            if(gBatRealPower - (-1 * chg_max_power) >= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet)//电池有足够能量吸收外部残余
                            {
                                gBatRealPower -= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet;
                            }
                            else//电池不足以吸收残余，仅吸收能力范围内的
                            {
                                gBatRealPower = (-1 * chg_max_power);
                            }
                            gGridPowerSet = (-1)*on_grid_max_power + gAntiReFlowValue;
                        }
					}					
                    if(gGridPowerSet > gNetworkPower + gAntiReFlowValue)
                    {
                        gRunstate = 11;
                        gGridPowerSet = gNetworkPower + gAntiReFlowValue;
                    }
                    gGridPowerSet = VALUE_LIMIT(gGridPowerSet,5000,-5000);

                    if(ThreadBreakDetect() == 1)
                    {
                        gRunstate = 12;
                        gstFaultInfo.worning_code.b.r_ct_reverse = 0;
                        gGridPowerSet = 0;
                    }
                    if(flag_CT_exist == 0)
                    {
                        gGridPowerSet = 0;
                    }
                    gPvPowerSet = LOADPOWER - gBatRealPower - gGridPowerSet;
                }
                else
                {
                    total_load_power = (-gAntiReFlowValue) + LOADPOWER;
                    total_load_power = VALUE_LIMIT(total_load_power, 5000, -5000);
                    gPvPowerSet = 0;

                    gBatTargetPower = total_load_power; // 20260312，新增功率变化率限制
//                  gBatSetPower = total_load_power;
                    if(flag_PowerRatelimit_Trigger == 1)
                    {
                        PowerChangeStep = 1.38; // 20260317，开启功率变化率限制，逆变器功率上升和下降的变化率为为每100ms变化1.38W，即13.8W/s
                        if((gBatSetPower + 5) <= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower + PowerChangeStep;
                        }
                        else if((gBatSetPower - 5) >= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower - PowerChangeStep;
                        }
                        else
                        {
                            gBatSetPower = gBatTargetPower;
                        }
                    }
                    else
                    {
                        gBatSetPower = gBatTargetPower;
                    }

                    if(gBatSetPower <= (chg_max_power*(-1) + CHG_DISCHG_PWR))
                    {
                        gRunstate = 6;
                        if(chg_max_power == 0 || gSysSetInfo.pcs_chg_ctrl == 0)
                        {
                            chg_disable_flag = 1;
                            gBatSetPower = CHG_DISCHG_PWR;
                        }
                        else if(gBatSetPower <= (chg_max_power*(-1)))
                        {
                            gBatSetPower = chg_max_power*(-1);
                        }
                    }
                    if(gBatSetPower >= (gDsgPower + DISCHG_CHG_PWR))
                    {
                        gRunstate = 7;
                        if (gDsgPower == 0)
                        {
                            gBatSetPower = DISCHG_CHG_PWR;
                        }
                        else if(gBatSetPower > gDsgPower)
                        {
                            gBatSetPower = gDsgPower;
                        }
                    }
                    if(gSysSetInfo.pcs_dsg_ctrl == 0)
                    {
                        gRunstate = 8;
                        if(gBatSetPower > 0)
                        {
                            gBatSetPower = DISCHG_CHG_PWR;
                        }
                    }
                    gBatRealPower = _gBatRealPower(gBatSetPower);
                    gGridPowerSet =  LOADPOWER - gBatRealPower;
					on_grid_max_power = gFeedPower;
					if(gSysSetInfo.antireverseflow_enable == 1)
					{
						if(on_grid_max_power > 0)
						{
							on_grid_max_power = 0;
						}
					}
					if(gGridPowerSet < (-1)*on_grid_max_power + gAntiReFlowValue)//并网功率过大
					{
						gGridPowerSet = (-1)*on_grid_max_power + gAntiReFlowValue;
					}
					
                    gGridPowerSet = VALUE_LIMIT(gGridPowerSet,5000,-5000);
                    if(ThreadBreakDetect() == 1)
                    {
                        gRunstate = 17;
                        gstFaultInfo.worning_code.b.r_ct_reverse = 0;
                        gGridPowerSet = 0;
                    }
                    if(flag_CT_exist == 0)
                    {
                        gGridPowerSet = 0;
                    }
                    gPvPowerSet = 0;
                }


                //gPvPowerSet -= grid_power_set_diff;     //减少光伏
                if(gPvPowerSet < 0)
                {
                    float diff = 0 - gPvPowerSet;
                    gRunstate = 39;
                    gPvPowerSet = 0;
                    gGridPowerSet -= diff;
                }
                if(gPvPowerSet > gMaxPvPwr)
                {
                    float diff = gPvPowerSet - gMaxPvPwr;
                    gPvPowerSet = gMaxPvPwr;
                    gGridPowerSet += diff;
                }
                else
                {
                    if(gPvPowerSet + (-1 * gChgPower) < 5000 - 100)//PV直接流向逆变器的能量不超过逆变最大功率
                    {
                        gPvPowerSet += 100;
                    }
                    else
                    {
                        gPvPowerSet = 5000 + gChgPower;
                    }
                }
            }
            else if(gParallelComm.mode == 1)//并机主机模式
            {
                pv_inc_flag = ParallelMasterPowerAlloc(0);
                if(pv_inc_flag == true)
                {
                    gPvPowerSet += 100;
                }
            }
            else if(gParallelComm.mode == 2)//并机从机模式
            {
                pv_inc_flag = ParallelSlavePowerAlloc();
                if(pv_inc_flag == true)
                {
                    gPvPowerSet += 100;
                }
            }

            inv_input_actpwr = gGridPowerSet - LOADPOWER;//逆变输入有功给定 = 电网有功给定 - 负载有功功率（注：负载有功功率为负）
            PowerCtrlModeHandle(e_national_standards,inv_input_actpwr,&_gInvPwr);
            _gInvPwr.inv_output_act_pwr += LOADPOWER;//电网有功输出给定 = 逆变输出有功给定 + 负载有功功率（注：负载有功功率为负）
            gGridPowerSet = _gInvPwr.inv_output_act_pwr;
            if(fabs(gGridPowerSet) > 5000)
            {
                gGridPowerSet = gGridPowerSet > 0 ? 5000 : -5000;
            }
//            gGridReactivePowerSet = _gInvPwr.inv_output_reactive_pwr + _Var_complete(GRID_VRMS,GRID_SPOWER,_gInvPwr.inv_output_reactive_pwr);
            gGridReactivePowerSet = _gInvPwr.inv_output_reactive_pwr;
            /*****************************/



            delay_time = TimGetSystime();
            break;
        }
    }
}


void PeakValley()
{
    static uint32_t  delay_time = 0;
    static uint32_t  delay_time1 = 0;
    float inv_output_power = 0;
    float inv_input_actpwr = 0;
    float grid_power_set_compensate = 0; //补偿后的GirdPowerSet
    float grid_power_set_diff = 0; //GirdPowerSet实际差值
    uint32_t chg_disable_flag = 0;
    bool pv_volt_valid_flag = _PVValidJudge();
    float on_grid_max_power = 0;

    switch(geInvsta)
    {
        case EN_OFF_GRID_MODE:
        {
            gRunstate = 18;
            gGridPowerSet = 0;
            gGridReactivePowerSet = 0;
            _BatOnlyChgDsgCtrl();
            if(TimDifftimeFromNow(delay_time) > gReconnectTime)
            {
                if((ThreadBreakDetect() != 1) && (ReadContinusOperationState() == 0) && \
                        ((gstModbusRegCSIP.CSIP_enable == 0) || ((gstModbusRegCSIP.CSIP_enable == 1) && ((gstModbusRegCSIP.CSIP_run_ctrl == 0) || (gstModbusRegCSIP.CSIP_run_ctrl == 2)))))  //CSIP允许并网
                {
                    gRunstate = 2;
                    InverterSetGridConnect(1);
                }
                else
                {
                    InverterSetGridConnect(0);
                }
                delay_time = TimGetSystime();
            }
            else
            {
                InverterSetGridConnect(0);
            }
            delay_time1 = TimGetSystime();
            _gOnGridOk = 0;

            ParallelMasterOffGrid(); // 20260513,新增并机主机离网状态把远端负载交给从机出力
            ParallelSlaveOffGrid();// 20260513,新增并机从机离网状态并网口的最大输入输出功率清零
            break;
        }
        case EN_OFF_GRID_TO_GRID_CONNECT_MODE:
        {
            gRunstate = 21;
            if((TimGetSystime() - delay_time1) > 10000)
            {
                InverterSetGridConnect(0);
                _gOnGridOk = 0;
                delay_time = TimGetSystime();
            }
            break;
        }
        case EN_GRID_CONNECT_MODE:
        {
            bool pv_inc_flag = false;
            float pv_power;
            if(pv_volt_valid_flag == true)
            {
                pv_power = PV1POWER;
                if(pv_power < 0)
                {
                    pv_power = 0;
                }
            }
            else
            {
                pv_power = 0;
            }

            if(gRelayCloseFlag == 1)
            {
                gRelayCloseFlag = 0;
            }
            if(_gOnGridOk == 0)
            {
                gOnGridCnt ++;
                _gOnGridOk = 1;
                gOnGridTime = TimGetSystime() - delay_time1;
            }
            if(gParallelComm.mode > 2)
            {
                gParallelComm.mode = 0;
            }
            if(gParallelComm.mode == 0)
            {
                if(gSysSetInfo.pcs_chg_ctrl == 1 && gSysSetInfo.pcs_dsg_ctrl != 1) // 充电模式
                {
                    gRunstate = 23;

                    gBatTargetPower = gChgPower*(-1); // 20260312，新增功率变化率限制
//                  gBatSetPower =  gChgPower*(-1);
                    if(flag_PowerRatelimit_Trigger == 1)
                    {
                        PowerChangeStep = 1.38; // 20260317，开启功率变化率限制，逆变器功率上升和下降的变化率为为每100ms变化1.38W，即13.8W/s
                        if((gBatSetPower + 5) <= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower + PowerChangeStep;
                        }
                        else if((gBatSetPower - 5) >= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower - PowerChangeStep;
                        }
                        else
                        {
                            gBatSetPower = gBatTargetPower;
                        }
                    }
                    else
                    {
                        gBatSetPower = gBatTargetPower;
                    }

                    if (gChgPower == 0)
                    {
                        gBatSetPower = CHG_DISCHG_PWR;
                        chg_disable_flag = 1;
                    }
                    if(gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0)
                    {
                        if(gBatSetPower > DISCHG_CHG_PWR)
                        {
                            gBatSetPower = DISCHG_CHG_PWR;
                        }
                    }
                    gBatRealPower = _gBatRealPower(gBatSetPower);

                    gGridPowerSet = -pv_power + LOADPOWER - gBatRealPower;
                    grid_power_set_compensate = gGridPowerSet;
					
					on_grid_max_power = gFeedPower;
					if(gSysSetInfo.antireverseflow_enable == 1)
					{
						if(on_grid_max_power > 0)
						{
							on_grid_max_power = 0;
						}
					}
					if(gGridPowerSet < (-1)*0 + gAntiReFlowValue)//并网功率过大
					{
						if(gBatStatusFlag == 1)//电池满电无法吸收，当前管不了电网，下一拍降光伏，如无光伏，则直接并给电网
                        {
                            //grid_power_set_diff += ((-1)*0 + gAntiReFlowValue) - gGridPowerSet;
                        }
                        else if(chg_disable_flag == 1)
                        {
                            if(gBatRealPower - CHG_DISCHG_PWR >= (-1)*0 + gAntiReFlowValue - gGridPowerSet)
                            {
                                gBatRealPower -= (-1)*0 + gAntiReFlowValue - gGridPowerSet;
                            }
                            else
                            {
                                gBatRealPower = CHG_DISCHG_PWR;
                            }
                            gGridPowerSet = (-1)*0 + gAntiReFlowValue;
                        }
                        else//电池不满电，可以由设备吸收
                        {
                            //if(gAntiReFlowValue <= 0)
                            if(gBatRealPower - (-1 * gChgPower) >= (-1)*0 + gAntiReFlowValue - gGridPowerSet)//电池有足够能量吸收外部残余
                            {
                                gBatRealPower -= (-1)*0 + gAntiReFlowValue - gGridPowerSet;
                            }
                            else
                            {
                                gBatRealPower = (-1 * gChgPower);
                            }
                            gGridPowerSet = (-1)*0 + gAntiReFlowValue;
                        }
					}
                    if(gGridPowerSet >= gNetworkPower + gAntiReFlowValue)
                    {
                        gGridPowerSet = gNetworkPower + gAntiReFlowValue;
                    }

                    gGridPowerSet = VALUE_LIMIT(gGridPowerSet,5000,-5000);

                    if(ThreadBreakDetect() == 1)
                    {
                        gRunstate = 26;
                        gstFaultInfo.worning_code.b.r_ct_reverse = 0;
                        gGridPowerSet = 0;
                    }
                    if(flag_CT_exist == 0)
                    {
                        gGridPowerSet = 0;
                    }
                }

                else if(gSysSetInfo.pcs_chg_ctrl != 1 && gSysSetInfo.pcs_dsg_ctrl == 1) // 放电模式
                {
                    gRunstate = 28;
					on_grid_max_power = gFeedPower;
					if(gSysSetInfo.antireverseflow_enable == 1)
					{
						if(on_grid_max_power > 0)
						{
							on_grid_max_power = 0;
						}
					}
					if(flag_CT_exist == 0)
                    {
                        on_grid_max_power =  0;
                    }
                    inv_output_power = LOADPOWER + on_grid_max_power - gAntiReFlowValue;
                    if(inv_output_power > 5000)
                    {
                        inv_output_power = 5000;
                    }
                    WATCH_INV_OUT_POWER = inv_output_power;

                    gBatTargetPower = inv_output_power - pv_power; // 20260312，新增功率变化率限制
//                  gBatSetPower = inv_output_power - pv_power;
//                  if(flag_PowerRatelimit_Trigger == 1 && flag_NetDisableTest_Trigger == 1) // 20260317，新增模拟断网取消功率功率变化率限制，允许功率快速降低
                    if(flag_PowerRatelimit_Trigger == 1) // 20260319，新增功率功率变化率限制
                    {
                        PowerChangeStep = 1.38; // 20260317，开启功率变化率限制，逆变器功率上升和下降的变化率为为每100ms变化1.38W，即13.8W/s
                        if((gBatSetPower + 5) <= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower + PowerChangeStep;
                        }
                        else if((gBatSetPower - 5) >= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower - PowerChangeStep;
                        }
                        else
                        {
                            gBatSetPower = gBatTargetPower;
                        }
                    }
                    else
                    {
                        gBatSetPower = gBatTargetPower;
                    }


                    if(gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0)
                    {
                        chg_disable_flag = 1;
                    }
                    if(gBatSetPower <= -gChgPower + BAT_DSG_PWR)
                    {
                        gRunstate = 29;
                        if (gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0)
                        {

                            gBatSetPower = CHG_DISCHG_PWR;
                        }
                        else if(gBatSetPower <= (gChgPower*(-1)))
                        {
                            gBatSetPower = gChgPower*(-1);
                        }
                    }
                    if(gBatSetPower >= gDsgPower + DISCHG_CHG_PWR)
                    {
                        gRunstate = 30;
                        if (gDsgPower == 0)
                        {
                            gBatSetPower = DISCHG_CHG_PWR;
                        }
                        else if(gBatSetPower > gDsgPower)
                        {
                            gBatSetPower = gDsgPower;
                        }
                    }
                    gBatRealPower = _gBatRealPower(gBatSetPower);

                    gGridPowerSet = -pv_power + LOADPOWER - gBatRealPower;
                    grid_power_set_compensate = gGridPowerSet;
					
					
					if(gSysSetInfo.antireverseflow_enable == 0)
					{
						on_grid_max_power += 100;//保留原来的增加100W的余量
					}
					
					if(gGridPowerSet < ((-1)*on_grid_max_power + gAntiReFlowValue))//并网功率过大
					{
						WATCH_flag = 1;
						gRunstate = 46;
                        if(gBatStatusFlag == 1)//电池满电无法吸收，当前管不了电网，下一拍降光伏，如无光伏，则直接并给电网
                        {
                            //grid_power_set_diff += ((-1)*on_grid_max_power + gAntiReFlowValue) - gGridPowerSet;
                        }
                        else if(chg_disable_flag == 1)
                        {
                            if(gBatRealPower - CHG_DISCHG_PWR >= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet)
                            {
                                gBatRealPower -= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet;
                            }
                            else
                            {
                                gBatRealPower = CHG_DISCHG_PWR;
                            }
                            gGridPowerSet = (-1)*on_grid_max_power + gAntiReFlowValue;
                        }
                        else//电池不满电，可以由设备吸收
                        {
                            if(gBatRealPower - (-1 * gChgPower) >= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet)//电池有足够能量吸收外部残余
                            {
                                gBatRealPower -= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet;
                            }
                            else//电池不足以吸收残余，仅吸收能力范围内的
                            {
                                gBatRealPower = (-1 * gChgPower);
                            }
                            gGridPowerSet = (-1)*on_grid_max_power + gAntiReFlowValue;
                        }
					}
                    if(gGridPowerSet > gNetworkPower + gAntiReFlowValue)
                    {
                        gRunstate = 11;
                        gGridPowerSet = gNetworkPower + gAntiReFlowValue;
                    }

                    gGridPowerSet = VALUE_LIMIT(gGridPowerSet,5000,-5000);

                    if(ThreadBreakDetect() == 1)
                    {
                        gRunstate = 31;
                        gstFaultInfo.worning_code.b.r_ct_reverse = 0;
                        gGridPowerSet = 0;
                    }
                    if(flag_CT_exist == 0)
                    {
                        gGridPowerSet = 0;
                    }
                }
                else if (gSysSetInfo.pcs_chg_ctrl != 1 && gSysSetInfo.pcs_dsg_ctrl != 1) // 静止模式
                {
                    gRunstate = 33;
                    inv_output_power = LOADPOWER - gAntiReFlowValue;//静态模式下，逆变只管负载，本地负载+远端负载
                    if(inv_output_power > 5000)
                    {
                        inv_output_power = 5000;
                    }

                    gBatTargetPower = inv_output_power - pv_power; // 20260312，新增功率变化率限制
//                  gBatSetPower = inv_output_power - pv_power;//本地负载+远端负载-光伏
                    if(flag_PowerRatelimit_Trigger == 1)
                    {
                        PowerChangeStep = 1.38; // 20260317，开启功率变化率限制，逆变器功率上升和下降的变化率为为每100ms变化1.38W，即13.8W/s
                        if((gBatSetPower + 5) <= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower + PowerChangeStep;
                        }
                        else if((gBatSetPower - 5) >= gBatTargetPower)
                        {
                            gBatSetPower = gBatSetPower - PowerChangeStep;
                        }
                        else
                        {
                            gBatSetPower = gBatTargetPower;
                        }
                    }
                    else
                    {
                        gBatSetPower = gBatTargetPower;
                    }

                    if(gChgPower == 0)
                    {
                        chg_disable_flag = 1;
                    }
                    if(gBatSetPower <= (-1)*gChgPower + CHG_DISCHG_PWR)
                    {
                        gRunstate = 34;
                        if (gChgPower == 0)
                        {
                            gBatSetPower = CHG_DISCHG_PWR;
                        }
                        else if(gBatSetPower <= (gChgPower*(-1)))
                        {
                            gBatSetPower = gChgPower*(-1);
                        }
                    }
                    if(gBatSetPower >= DISCHG_CHG_PWR)
                    {
                        gRunstate = 35;
                        if (gDsgPower == 0 || gSysSetInfo.pcs_dsg_ctrl == 0)
                        {
                            gBatSetPower = DISCHG_CHG_PWR;
                        }
                        // else if(gBatSetPower > 0)
                        // {
                        // gBatSetPower = 0;
                        // }
                    }
                    if(gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0)
                    {
                        gBatSetPower = DISCHG_CHG_PWR;
                    }
                    if(gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0)
                    {
                        chg_disable_flag = 1;
                        if(gBatSetPower < CHG_DISCHG_PWR)
                        {
                            gBatSetPower = CHG_DISCHG_PWR;
                        }
                    }
                    if(gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0)
                    {
                        gBatSetPower = DISCHG_CHG_PWR;
                    }
                    gBatRealPower = _gBatRealPower(gBatSetPower);
                    //                gBatRealPower = 0;

                    gGridPowerSet = -pv_power + LOADPOWER - gBatRealPower;
                    grid_power_set_compensate = gGridPowerSet;
					
					on_grid_max_power = gFeedPower;
					if(on_grid_max_power > 0)//削峰填谷静置模式不并网
					{
						on_grid_max_power = 0;
					}
                    
					if(gGridPowerSet < (-1)*on_grid_max_power + gAntiReFlowValue)//并网功率过大
					{
						if(gBatStatusFlag == 1)//电池满电无法吸收，当前管不了电网，下一拍降光伏，如无光伏，则直接并给电网
                        {
                            //grid_power_set_diff += ((-1)*on_grid_max_power + gAntiReFlowValue) - gGridPowerSet;
                        }
                        else if(chg_disable_flag == 1)
                        {
                            if(gBatRealPower - CHG_DISCHG_PWR >= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet)
                            {
                                gBatRealPower -= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet;
                            }
                            else
                            {
                                gBatRealPower = CHG_DISCHG_PWR;
                            }
                            gGridPowerSet = (-1)*on_grid_max_power + gAntiReFlowValue;
                        }
                        else//电池不满电，可以由设备吸收
                        {
                            if(gBatRealPower - (-1 * gChgPower) >= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet)//电池有足够能量吸收外部残余
                            {
                                gBatRealPower -= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet;
                            }
                            else//电池不足以吸收残余，仅吸收能力范围内的
                            {
                                gBatRealPower = (-1 * gChgPower);
                            }
                            gGridPowerSet = (-1)*on_grid_max_power + gAntiReFlowValue;
                        }
					}
					if(gGridPowerSet > gNetworkPower + gAntiReFlowValue)
                    {
                        gRunstate = 11;
                        gGridPowerSet = gNetworkPower + gAntiReFlowValue;
                    }
                    gGridPowerSet = VALUE_LIMIT(gGridPowerSet,5000,-5000);

                    if(ThreadBreakDetect() == 1)
                    {
                        gRunstate = 38;
                        gstFaultInfo.worning_code.b.r_ct_reverse = 0;
                        gGridPowerSet = 0;
                    }
                    if(flag_CT_exist == 0)
                    {
                        gGridPowerSet = 0;
                    }
                }

                gPvPowerSet = LOADPOWER - gBatRealPower - gGridPowerSet;
                //gPvPowerSet -= grid_power_set_diff;     //减少光伏

                if(gPvPowerSet < 0)
                {
                    float diff = 0 - gPvPowerSet;
                    gRunstate = 39;
                    gPvPowerSet = 0;
                    gGridPowerSet -= diff;
                }
                if(gPvPowerSet > gMaxPvPwr)
                {
                    float diff = gPvPowerSet - gMaxPvPwr;
                    gPvPowerSet = gMaxPvPwr;
                    //                gGridPowerSet += diff;
                }
                else
                {
                    if(gPvPowerSet + (-1 * gChgPower) < 5000 - 100)//PV直接流向逆变器的能量不超过逆变最大功率
                    {
                        gPvPowerSet += 100;
                    }
                    else
                    {
                        gPvPowerSet = 5000 + gChgPower;
                    }
                }
            }
            else if(gParallelComm.mode == 1)//并机主机模式
            {
                if(gSysSetInfo.pcs_chg_ctrl == 1) // 充电模式
                {
                    pv_inc_flag = ParallelMasterPowerAlloc(3);
                }
                else if(gSysSetInfo.pcs_dsg_ctrl == 1) // 放电模式
                {
                    pv_inc_flag = ParallelMasterPowerAlloc(4);
                }
                else
                {
                    pv_inc_flag = ParallelMasterPowerAlloc(2);
                }
                if(pv_inc_flag == true)
                {
                    gPvPowerSet += 100;
                }
            }
            else if(gParallelComm.mode == 2)//并机从机模式
            {
                pv_inc_flag = ParallelSlavePowerAlloc();
                if(pv_inc_flag == true)
                {
                    gPvPowerSet += 100;
                }
            }

            inv_input_actpwr = gGridPowerSet - LOADPOWER;//逆变输入有功给定 = 电网有功给定 - 负载有功功率（注：负载有功功率为负）
            PowerCtrlModeHandle(e_national_standards,inv_input_actpwr,&_gInvPwr);
            _gInvPwr.inv_output_act_pwr += LOADPOWER;//电网有功输出给定 = 逆变输出有功给定 + 负载有功功率（注：负载有功功率为负）
            gGridPowerSet = _gInvPwr.inv_output_act_pwr;
            if(fabs(gGridPowerSet) > 5000)
            {
                gGridPowerSet = gGridPowerSet > 0 ? 5000 : -5000;
            }
//            gGridReactivePowerSet = _gInvPwr.inv_output_reactive_pwr + _Var_complete(GRID_VRMS,GRID_SPOWER,_gInvPwr.inv_output_reactive_pwr);
            gGridReactivePowerSet = _gInvPwr.inv_output_reactive_pwr;
            /*****************************/


            //变更未跟赵越确定TODO:待确定
            /****************************/

            delay_time = TimGetSystime();
            break;
        }
        default:
            gRunstate = 100;
            break;
    }
}

void BatPriority()
{
    static uint32_t delay_time = 0;
    static uint32_t delay_time1 = 0;
    float inv_output_power = 0;
    float inv_input_actpwr = 0;
    float grid_power_set_compensate = 0; //补偿后的GirdPowerSet
    float grid_power_set_diff = 0; //GirdPowerSet实际差值
    uint32_t chg_disable_flag = 0;
    bool pv_volt_valid_flag = _PVValidJudge();
    float on_grid_max_power = 0;
	
    switch(geInvsta)
    {
        case EN_OFF_GRID_MODE:
        {
            gRunstate = 40;
            gGridPowerSet = 0;
            gGridReactivePowerSet = 0;
            // AlgoSetGridPwr(0);
            _BatOnlyChgDsgCtrl();
            if(TimDifftimeFromNow(delay_time) > gReconnectTime)
            {
                if((ThreadBreakDetect() != 1) && (ReadContinusOperationState() == 0) && \
                        ((gstModbusRegCSIP.CSIP_enable == 0) || ((gstModbusRegCSIP.CSIP_enable == 1) && ((gstModbusRegCSIP.CSIP_run_ctrl == 0) || (gstModbusRegCSIP.CSIP_run_ctrl == 2)))))  //CSIP允许并网
                {
                    gRunstate = 2;
                    InverterSetGridConnect(1);
                }
                else
                {
                    InverterSetGridConnect(0);
                }
                delay_time = TimGetSystime();
            }
            else
            {
                InverterSetGridConnect(0);
            }
            delay_time1 = TimGetSystime();
            _gOnGridOk = 0;

            ParallelMasterOffGrid(); // 20260513,新增并机主机离网状态把远端负载交给从机出力
            ParallelSlaveOffGrid();// 20260513,新增并机从机离网状态并网口的最大输入输出功率清零
            break;
        }
        case EN_OFF_GRID_TO_GRID_CONNECT_MODE:
        {
            gRunstate = 42;
            if((TimGetSystime() - delay_time1) > 10000)
            {
                InverterSetGridConnect(0);
                _gOnGridOk = 0;
                delay_time = TimGetSystime();
            }
            break;
        }
        case EN_GRID_CONNECT_MODE:
        {
            bool pv_inc_flag = false;
            float pv_power;
            gRunstate = 43;
            if(pv_volt_valid_flag == true)
            {
                pv_power = PV1POWER;
                if(pv_power < 0)
                {
                    pv_power = 0;
                }
            }
            else
            {
                pv_power = 0;
            }
            if(gRelayCloseFlag == 1)
            {
                gRelayCloseFlag = 0;
            }
            if(_gOnGridOk == 0)
            {
                gOnGridCnt ++;
                _gOnGridOk = 1;
                gOnGridTime = TimGetSystime() - delay_time1;
            }

            if(gParallelComm.mode > 2)
            {
                gParallelComm.mode = 0;
            }
            if(gParallelComm.mode == 0)
            {
                gBatTargetPower = gChgPower*(-1); // 20260312，新增功率变化率限制
//                  gBatSetPower =  gChgPower*(-1);
                if(flag_PowerRatelimit_Trigger == 1)
                {
                    PowerChangeStep = 1.38; // 20260317，开启功率变化率限制，逆变器功率上升和下降的变化率为为每100ms变化1.38W，即13.8W/s
                    if((gBatSetPower + 5) <= gBatTargetPower)
                    {
                        gBatSetPower = gBatSetPower + PowerChangeStep;
                    }
                    else if((gBatSetPower - 5) >= gBatTargetPower)
                    {
                        gBatSetPower = gBatSetPower - PowerChangeStep;
                    }
                    else
                    {
                        gBatSetPower = gBatTargetPower;
                    }
                }
                else
                {
                    gBatSetPower = gBatTargetPower;
                }

                if(gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0)
                {
                    chg_disable_flag = 1;
                    if(gBatSetPower < CHG_DISCHG_PWR)
                    {
                        gBatSetPower = CHG_DISCHG_PWR;
                    }
                }
                if(gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0)
                {
                    if(gBatSetPower > DISCHG_CHG_PWR)
                    {
                        gBatSetPower = DISCHG_CHG_PWR;
                    }
                }
                gBatRealPower = _gBatRealPower(gBatSetPower);

                gGridPowerSet = -pv_power + LOADPOWER - gBatRealPower;
                grid_power_set_compensate = gGridPowerSet;
				
				on_grid_max_power = gFeedPower;
				if(gSysSetInfo.antireverseflow_enable == 1)
				{
					if(on_grid_max_power > 0)
					{
						on_grid_max_power = 0;
					}
				}
				
				if(gGridPowerSet < (-1)*on_grid_max_power + gAntiReFlowValue)//并网功率过大
				{
					flag_if = 1;
					gRunstate = 46;
					if(gBatStatusFlag == 1)//电池满电无法吸收，当前管不了电网，下一拍降光伏，如无光伏，则直接并给电网
                    {
                        //grid_power_set_diff += ((-1)*on_grid_max_power + gAntiReFlowValue) - gGridPowerSet;
                    }
                    else if(chg_disable_flag == 1)
                    {
                        if(gBatRealPower - CHG_DISCHG_PWR >= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet)
                        {
                            gBatRealPower -= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet;
                        }
                        else
                        {
                            gBatRealPower = CHG_DISCHG_PWR;
                        }
                        gGridPowerSet = (-1)*on_grid_max_power + gAntiReFlowValue;
                    }
                    else//电池不满电，可以由设备吸收
                    {
                        //if(gAntiReFlowValue <= 0)
                        if(gBatRealPower - (-1 * gChgPower) >= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet)//电池有足够能量吸收外部残余
                        {
                            gBatRealPower -= (-1)*on_grid_max_power + gAntiReFlowValue - gGridPowerSet;
                        }
                        else
                        {
                            gBatRealPower = (-1 * gChgPower);
                        }
                        gGridPowerSet = (-1)*on_grid_max_power + gAntiReFlowValue;
                    }
				}

                if(gGridPowerSet > gNetworkPower + gAntiReFlowValue)
                {
                    gRunstate = 11;
                    gGridPowerSet = gNetworkPower + gAntiReFlowValue;
                }

                gGridPowerSet = VALUE_LIMIT(gGridPowerSet,5000,-5000);

                if(ThreadBreakDetect() == 1)
                {
                    gRunstate = 47;
                    gstFaultInfo.worning_code.b.r_ct_reverse = 0;
                    gGridPowerSet = 0;
                }
                if(flag_CT_exist == 0)
                {
                    gGridPowerSet = 0;
                }

                gPvPowerSet = LOADPOWER - gBatRealPower - gGridPowerSet;
                //gPvPowerSet -= grid_power_set_diff;     //减少光伏

                if(gPvPowerSet < 0)
                {
                    float diff = 0 - gPvPowerSet;
                    gRunstate = 39;
                    gPvPowerSet = 0;
                    gGridPowerSet -= diff;
                }
                if(gPvPowerSet > gMaxPvPwr)
                {
                    float diff = gPvPowerSet - gMaxPvPwr;
                    gPvPowerSet = gMaxPvPwr;
                    //gGridPowerSet += diff;
                }
                else
                {
                    if(gPvPowerSet + (-1 * gChgPower) < 5000 - 100)//PV直接流向逆变器的能量不超过逆变最大功率
                    {
                        gPvPowerSet += 100;
                    }
                    else
                    {
                        gPvPowerSet = 5000 + gChgPower;
                    }
                }
            }
            else if(gParallelComm.mode == 1)//并机主机模式
            {
                pv_inc_flag = ParallelMasterPowerAlloc(1);
                if(pv_inc_flag == true)
                {
                    gPvPowerSet += 100;
                }
            }
            else if(gParallelComm.mode == 2)//并机从机模式
            {
                pv_inc_flag = ParallelSlavePowerAlloc();
                if(pv_inc_flag == true)
                {
                    gPvPowerSet += 100;
                }
            }

            inv_input_actpwr = gGridPowerSet - LOADPOWER;//逆变输入有功给定 = 电网有功给定 - 负载有功功率（注：负载有功功率为负）
            PowerCtrlModeHandle(e_national_standards,inv_input_actpwr,&_gInvPwr);
            _gInvPwr.inv_output_act_pwr += LOADPOWER;//电网有功输出给定 = 逆变输出有功给定 + 负载有功功率（注：负载有功功率为负）
            gGridPowerSet = _gInvPwr.inv_output_act_pwr;
            if(fabs(gGridPowerSet) > 5000)
            {
                gGridPowerSet = gGridPowerSet > 0 ? 5000 : -5000;
            }
//            gGridReactivePowerSet = _gInvPwr.inv_output_reactive_pwr + _Var_complete(GRID_VRMS,GRID_SPOWER,_gInvPwr.inv_output_reactive_pwr);
            gGridReactivePowerSet = _gInvPwr.inv_output_reactive_pwr;
            /*****************************/


            delay_time = TimGetSystime();
            break;
        }
    }

}

float flag_age_test = 0;

float work_mode = 0;
float InvState = 0;

//extern float flag_parallel_master;
//extern float flag_parallel_slave;
//extern float flag_disconnect;

#define LOW_SYNC_OUT    1
#define HIGH_SYNC_OUT   0

void MainPollingProcess()
{
//    static int16_t flag_EnableState_new  = 0;
//    static int16_t flag_EnableState_last = 0;
    static uint32_t csip_process_count = 0;  //100ms计数一次
    gChgPower = (int)gSysSetInfo.battery_charging_power;// 20260311，新增变量定义为从监控卡获取的充电功率，用于功率变化率限制计算
    gDsgPower = (int)gSysSetInfo.battery_discharging_power;// 20260311，新增变量定义为从监控卡获取的放电功率，用于功率变化率限制计算
    gstModbusPowerCtrl.export_power = (int)gSysSetInfo.ongrid_power;// 20260304，增加功率变化率限制，屏蔽此处卖电功率直接赋值
    gReconnectTime = (gSysSetInfo.ongrid_reconnect_time*1000);
    _gInvSta = InverterGetState();
    geInvsta = (EN_PARALLEL_OFF_NETWORK_STA)(_gInvSta->sta_grid_connect);

//    if(flag_parallel_master == 0 && flag_parallel_slave == 1 && flag_disconnect == 1)
//    {
//        geInvsta = 0;
//    }
    InvState = geInvsta;

    uint16_t switch_flag = DeviceSwitchCtrl();
    KeyLedCtrl();   //2026年6月11日幸逍按键灯控制
    EnergyInputModeSwitch();//市电油机切换过程
    uint16_t en_continus_check = 0;
    uint16_t en_continus_recover_check = 0;
    gNetworkPowerEnable = gSysSetInfo.network_power_limit_enable;
    gstModbusPowerCtrl.import_power = gSysSetInfo.network_power_limit;

    flag_NetDisableTest_Enable  = (gSysSetInfo.certification_test_switch >> 0) & 0x0001; //20260305，取bit0位的值给flag_NetDisableTest_Enable
    flag_HardExportlimit_Enable = (gSysSetInfo.certification_test_switch >> 1) & 0x0001; //20260305，取bit1位的值给flag_HardExportlimit_Enable
    flag_SoftExportlimit_Enable = (gSysSetInfo.certification_test_switch >> 2) & 0x0001; //20260305，取bit2位的值给flag_SoftExportlimit_Enable
    flag_PowerRatelimit_Enable  = (gSysSetInfo.certification_test_switch >> 3) & 0x0001; //20260305，取bit3位的值给flag_PowerRatelimit_Enable

//    if(flag_NetDisableTest_Enable == 1 || flag_HardExportlimit_Enable == 1 || flag_SoftExportlimit_Enable == 1 || flag_PowerRatelimit_Enable == 1)
//    {
//        flag_CertificationTest = 1;
//    }
//    else
//    {
//        flag_CertificationTest = 0;
//    }

    flag_EnableState_new = flag_NetDisableTest_Enable;
    if(flag_EnableState_new == 1 && flag_EnableState_last == 0) // 20260319，新增模拟断网触发标志位，上升沿触发
    {
        flag_NetDisableTest_Trigger = 1;
    }
    if(flag_EnableState_new == 0 && flag_EnableState_last == 1) // 20260320，新增模拟断网恢复标志位，下降沿触发
    {
        flag_NetDisableTest_Recover = 1;
    }
    flag_EnableState_last = flag_EnableState_new;

    // 2026年5月28日 CSIP使能状态下不运行软硬限制
    if(gstModbusRegCSIP.CSIP_enable == 0)
    {
        if(flag_HardExportlimit_Enable == 1 && flag_SoftExportlimit_Enable == 1) // 20260320，新增同时开启输出功率硬限制开关和输出功率软限制开关，不进行功率变化率限制
        {
            flag_PowerRatelimit_Trigger = 0; // 20260320，新增在同时开启输出功率硬限制和输出功率软限制时，关闭功率变化率限制
        }
        else if(flag_HardExportlimit_Enable == 1 && flag_SoftExportlimit_Enable == 0) // 20260319，新增单开输出功率硬限制开关，flag_HardExportlimit_Enable = 1，进入输出功率硬限制判断
        {
            //        if((-1 * gSysInfoGet.r_phase_meter_activepower) >= 50 && (LoadBackup + LoadRemote + gSysSetInfo.ongrid_power) <= 5000) // 20260320，当逆变器往电网卖电功率大于50W，且逆变器AC口输出总功率小于5000W时，启动输出功率硬限制计数器
            if((LoadBackup + LoadRemote + gSysSetInfo.ongrid_power + 100) <= 5000) // 20260320，当逆变器设定卖电功率与总负载功率之和小于额定最大输出功率5000W时，启动输出功率硬限制计数器，加100W用于防止误判
                //        if((_gState_algo.grid_pwr + 100) <= gSysSetInfo.ongrid_power) // 20260320，当逆变器AC侧实际输出功率小于设定卖电功率时，启动输出功率硬限制计数器，加100W用于防止误判
            {
                count_OverHardExportlimit++;
                if(count_OverHardExportlimit >= 30)
                {
                    count_OverHardExportlimit = 30;
                    flag_HardExportlimit_Trigger = 1; // 20260319，新增在输出功率硬限制计数器累计达到30次时，触发输出功率硬限制保护，逆变器脱网
                }
            }
            else
            {
                count_OverHardExportlimit--;
                if(count_OverHardExportlimit <= 1)
                {
                    count_OverHardExportlimit = 1;
                    flag_HardExportlimit_Trigger = 0;
                }
            }
            if(flag_HardExportlimit_Trigger == 1 || flag_NetDisableTest_Trigger == 1)// 20260319，新增在输出功率硬限制触发，或模拟断网触发时，逆变器脱网
            {
                flag_OffGrid_Trigger = 1;
            }
            if(flag_OffGrid_Completed == 1)
            {
                flag_PowerRatelimit_Trigger = 1; // 20260319，新增在逆变器主动脱网完成后，启动功率变化率限制
                if((gBatRealPower + 5) >= gBatTargetPower && (gBatRealPower - 5) <= gBatTargetPower) // 20260319，新增在逆变器重新并网后，且逆变器电池的实时功率与内部计算设定的目标功率差值范围在±5W以内，启动输出功率软限制计数器
                {
                    count_OverSoftExportlimit++;
                    if(count_OverSoftExportlimit >= 30)
                    {
                        count_OverSoftExportlimit = 30;
                        flag_OffGrid_Trigger = 0; // 20260319，新增在输出功率软限制计数器累计达到30次时，清除脱网保护触发标志位
                        flag_OffGrid_Completed = 0; // 20260319，新增在输出功率软限制计数器累计达到30次时，清除脱网保护完成标志位
                        flag_NetDisableTest_Trigger = 0; // 20260319，新增在输出功率软限制计数器累计达到30次时，清除模拟断网触发标志位
                        flag_NetDisableTest_Recover = 0; // 20260320，新增在输出功率软限制计数器累计达到30次时，清除模拟断网恢复标志位
                        flag_PowerRatelimit_Trigger = 0; // 20260319，新增在硬限制脱网触发后逆变器重新并网，且电池功率重新达到稳定后，关闭功率变化率限制
                    }
                }
                else
                {
                    count_OverSoftExportlimit--;
                    if(count_OverSoftExportlimit <= 1)
                    {
                        count_OverSoftExportlimit = 1;
                    }
                }
            }
            else
            {
                count_OverSoftExportlimit--;
                if(count_OverSoftExportlimit <= 1)
                {
                    count_OverSoftExportlimit = 1;
                }
            }
        }
        else if(flag_HardExportlimit_Enable == 0 && flag_SoftExportlimit_Enable == 1) // 20260317，新增单开输出功率软限制开关，flag_SoftExportlimit_Enable = 1，进入输出功率软限制判断
        {
            if(flag_NetDisableTest_Recover == 1)
            {
                flag_PowerRatelimit_Trigger = 1; // 20260319，新增在输出功率软限制开启，且模拟断网恢复后，启动功率变化率限制
                if((gBatRealPower + 5) >= gBatTargetPower && (gBatRealPower - 5) <= gBatTargetPower) // 20260319，新增在模拟断网触发时，且逆变器电池的实时功率与内部计算设定的目标功率差值范围在±5W以内，启动输出功率软限制计数器
                {
                    count_OverSoftExportlimit++;
                    if(count_OverSoftExportlimit >= 30)
                    {
                        count_OverSoftExportlimit = 30;
                        flag_NetDisableTest_Trigger = 0; // 20260319，新增在输出功率软限制计数器累计达到30次时，清除模拟断网触发标志位
                        flag_NetDisableTest_Recover = 0; // 20260320，新增在输出功率软限制计数器累计达到30次时，清除模拟断网恢复标志位
                    }
                }
                else
                {
                    count_OverSoftExportlimit--;
                    if(count_OverSoftExportlimit <= 1)
                    {
                        count_OverSoftExportlimit = 1;
                    }
                }
            }
            else
            {
                flag_PowerRatelimit_Trigger = 0; // 20260319，新增在输出功率软限制开启，且模拟断网没有触发或触发后清除时，关闭功率变化率限制
            }
        }
        else if(flag_PowerRatelimit_Enable == 1) // 20260316，新增功率变化率限制开关，flag_PowerRatelimit_Enable = 1时启动功率变化率限制
        {
            flag_PowerRatelimit_Trigger = 1;
            PowerChangeStep = 1.38; // 20260317，开启功率变化率限制，逆变器功率上升和下降的变化率为为每100ms变化1.38W，即13.8W/s
        }
        else
        {
            flag_PowerRatelimit_Trigger = 0;
            PowerChangeStep = 0; // 20260312，非功率变化率限制状态下，逆变器不执行功率变化步长，在每一个工作模式独立判断
        }
    }

    if(gSysSetInfo.ignore_ct_err == 0)
    {
        flag_age_test = 0;
    }
    else
    {
        flag_age_test = 1;
    }

    if(gNetworkPowerEnable == 0)
    {
        gstModbusPowerCtrl.import_power = 1000000;
    }

    CheckCtExistsErr(); // 20251226增加，检测CT有没有安装，返回标志位，没有安装CT时flag_CT_exist = 0，安装CT时flag_CT_exist = 1
    CheckSlaveErr();// 20260116增加，主从机并机通信故障检测和从机故障检测

    switch(geInvsta)
    {
        case EN_GRID_CONNECT_MODE:
        {
            en_continus_check = 1;
            en_continus_recover_check = 0;
            break;
        }
        case EN_OFF_GRID_MODE:
        {
            en_continus_check = 0;
            en_continus_recover_check = 1;
            break;
        }
        default:
        {
            en_continus_check = 0;
            en_continus_recover_check = 0;
            break;
        }
    }
    ContinusOperationRecover(_gState_algo.grid_vol_rms, 245, en_continus_recover_check);
    ContinusOperationCheck(_gState_algo.grid_vol_rms, 258, en_continus_check);

    // CSIP状态机
    csip_process_count++;
    if(csip_process_count >= CSIP_PROCESS_COUNT)
    {
        csip_process_count = 0;
        CSIPOnGridCertification();
    }

    if(gstModbusRegCSIP.CSIP_enable == 1)
    {
        gFeedPower    = gstCsipPowerCtrl.export_power;
        gNetworkPower = gstCsipPowerCtrl.import_power;
    }
    else
    {
        gFeedPower    = gstModbusPowerCtrl.export_power;
        gNetworkPower = gstModbusPowerCtrl.import_power;
    }

    flag_switch = switch_flag;
    if(switch_flag)
    {
        if(Engrid)//市电模式
        {
            flag_if = 10;
            work_mode = gSysSetInfo.work_mode;
            if(gSysInfoGet.parallel_mode_stat == 0)
            {
                NonParallelDataClear();
            }
            switch(gSysSetInfo.work_mode)
            {
                case EN_SPONTANEOUS_USE_MODE:
                {
                    flag_if = 11;
                    SpontaneousUse();
                    break;
                }
                case EN_BAT_PRIORITY_MODE:
                {
                    flag_if = 12;
                    BatPriority();
                    break;
                }
                case EN_PEAK_VALLEY_MODE:
                {
                    flag_if = 13;
                    PeakValley();
                    break;
                }
                default:
                    break;
            }
            //gPvPowerSet = 0;
            PVRealTimePower  = PV1POWER;
            BatRealTimePower = BATPOWER;
            LoadBackup       = LOADPOWER;
            LoadRemote       = -1 * gAntiReFlowValue;

            if(geInvsta != EN_GRID_CONNECT_MODE)
            {
                gBatTargetPower = 0; // 20260319，新增在非并网模式时，电池目标充放电功率，注意此变量只用于在重新并网时保证电池功率从0开始变化，不参与离网模式功率调度
                gBatSetPower = gBatTargetPower;
            }

            if(flag_OffGrid_Trigger == 1 && flag_OffGrid_Completed == 0)// 20260319，新增在逆变器脱网保护触发标志位置1时，逆变器主动脱网，持续执行直到脱网保护完成标志位置1时，退出脱网逻辑
            {
                gGridPowerSet = 0; // 20260319，新增在逆变器脱网保护触发标志位flag_OffGrid_Trigger = 1时，并网功率强制设置为0
                OffGridHandle(); //逆变器主动脱网，注意此函数需要至少0.6s的时间才能执行
            }

            //2026年5月28日 ctrl == 0 正常并网    ctrl == 1 离网   ctrl == 2 并网ac口功率为0
            if(gstModbusRegCSIP.CSIP_enable == 1)
            {
                if(gstModbusRegCSIP.CSIP_run_ctrl == 0)
                {
                    gstCSIP.cease_power = 1;
                    flag_OffGrid_Trigger = 0;
                    flag_OffGrid_Completed = 0;
                }
                else if(gstModbusRegCSIP.CSIP_run_ctrl == 1)
                {
                    gstCSIP.cease_power = 2;
                    flag_OffGrid_Trigger = 1;
                }
                else if(gstModbusRegCSIP.CSIP_run_ctrl == 2)
                {
                    gstCSIP.cease_power = 3;
                    gGridPowerSet = 0;
                    flag_OffGrid_Trigger = 0;
                    flag_OffGrid_Completed = 0;
                }
            }

            _Pv_softstart(gGridPowerSet, GRID_POWER, gPvPowerSet, PV1POWER, LOADPOWER, gChgPower);
        }
        else if(Enoil)
        {
            flag_if = 20;

            OilEngineMode();
            PVRealTimePower  = PV1POWER;
            BatRealTimePower = BATPOWER;
            LoadBackup       = LOADPOWER;
            LoadRemote       = -1 * gAntiReFlowValue;
            _Pv_softstart(gGridPowerSet, GRID_POWER, gPvPowerSet, PV1POWER, LOADPOWER, gChgPower);
        }
    }
    else
    {
        ParallelMasterOffGrid();
        ParallelSlaveOffGrid();
    }
}

static float _Bat_power_limit(float charge_power_origin)
{
    float charge_power_local;
    if((charge_power_origin <= 1000) && (BAT_V > 56.5))
    {
        charge_power_local = 0;
    }
    else if(BAT_V > 57)
    {
        charge_power_local = 200;
    }
    else if(BAT_V > 56.8)
    {
        charge_power_local = 500;
    }
    else if(BAT_V > 56.5)
    {
        charge_power_local = 1000;
    }
    else
    {
        charge_power_local = charge_power_origin;
    }

    return charge_power_local;


}
//PV_soft软起
float pv_test1,pv_test2,grid_test;
uint16_t pv_unstable_test;
float master_count, master_count2,master_count3,master_count4,master_count5,master_count6,master_count7;
static void _Pv_softstart(float grid_power_set, float grid_power, float pv_power_set, float pv_power, float load_power, float chargpower_set)
{

    float limit_activepwr = 0;
    /**************************************************/
    // PV软起状态判断
    if(pv_state_old == 0)  //是否进入软起动状态
    {
        pv_startok_flag = 0; //PV启动完成标志位清零
        if (Read_Mppt_state() != 0) //PV开始启动
        {
            pv_start_flag = 1;  // PV开始启动标志位置位
        }
        else  //PV不启动
        {
            pv_start_flag = 0;  //PV不存在，清零开始启动标志位
        }
    }
    pv_test1 = gPvPowerSet;
    /**************************************************/
    // 状态判断
    switch (geInvsta)
    {
        case EN_GRID_CONNECT_MODE:
        {
            if(gSysSetInfo.antireverseflow_enable == 1 || gFeedPower == 0) //禁止发电状态
            {
                if((pv_start_flag == 0) && (pv_startok_flag == 0))
                    geEnergysta = EN_STOPGENRATION_WITH_GRID_ONLY;
                else if((pv_start_flag == 1) && (pv_startok_flag == 0))
                    geEnergysta = EN_STOPGENRATION_WITH_GRID_AND_PVSOFT;
                else
                    geEnergysta = EN_STOPGENRATION_WITH_GRID_AND_PV;
            }
            else
            {
                if((pv_start_flag == 0) && (pv_startok_flag == 0))
                    geEnergysta = EN_LOAD_WITH_GRID_ONLY;
                else if((pv_start_flag == 1) && (pv_startok_flag == 0))
                    geEnergysta = EN_LOAD_WITH_GRID_AND_PVSOFT;
                else
                    geEnergysta = EN_LOAD_WITH_GRID_AND_PV;
            }
            break;
        }
        case EN_OFF_GRID_MODE:
        {
            if(Read_Mppt_state() == 0)
            {
                geEnergysta = EN_OFFGRID_ONLY;
            }
            else
            {
                geEnergysta = EN_OFFGRID_AND_PV;
            }
            break;
        }
        default:
        {
            geEnergysta = EN_GRID_NORMAL;
            break;
        }
    }
    if(geEnergysta == EN_LOAD_WITH_GRID_AND_PVSOFT || geEnergysta == EN_STOPGENRATION_WITH_GRID_AND_PVSOFT)
    {
        pv_unstable_test = 1;
    }
    else
    {
        pv_unstable_test = 0;
    }

    switch (geEnergysta)
    {
        case EN_LOAD_WITH_GRID_ONLY:
        {
            master_count4 = 0;
            master_count5 = 0;
            gPvPowerSet = 0;
            PvSetRunstate = 1;
            break;
        }
        case EN_LOAD_WITH_GRID_AND_PVSOFT:
        {
            master_count4 = 1;
            master_count6++;
            gGridPowerSet = load_power - 1500;
            float inv_ref_diff = gGridPowerSet - LOADPOWER + MAX_INV_OUT_POWER;
            if(inv_ref_diff < 0)
            {
                gGridPowerSet = gGridPowerSet - inv_ref_diff;
                pv_power_set = pv_power_set + inv_ref_diff;
            }
//            gGridPowerSet =  VALUE_LIMIT(gGridPowerSet, 5000, gFeedPower*(-1));

            if(gSysSetInfo.work_mode == EN_SPONTANEOUS_USE_MODE)
            {
                gPvPowerSet = load_power + gChgPower - GRID_POWER + 100;
            }
            else
            {
                gPvPowerSet = load_power - gBatRealPower - GRID_POWER + 100;
            }

            gPvPowerSet = (gPvPowerSet > 1500)?gPvPowerSet:0;
            PvSetRunstate = 2;
            break;
        }
        case EN_LOAD_WITH_GRID_AND_PV:
        {
            master_count4 = 0;
            master_count5 = 0;
            gGridPowerSet = grid_power_set;
            float inv_ref_diff = gGridPowerSet - LOADPOWER + MAX_INV_OUT_POWER;
            if(inv_ref_diff < 0)
            {
                gGridPowerSet = gGridPowerSet - inv_ref_diff;
                pv_power_set = pv_power_set + inv_ref_diff;
            }
//            gGridPowerSet =  VALUE_LIMIT(gGridPowerSet, 5000, gFeedPower*(-1));
            gPvPowerSet = pv_power_set;
            PvSetRunstate = 3;
            break;
        }
        case EN_STOPGENRATION_WITH_GRID_ONLY:
        {
            master_count4 = 0;
            master_count5 = 0;
            gGridPowerSet = grid_power_set;
            float inv_ref_diff = gGridPowerSet - LOADPOWER + MAX_INV_OUT_POWER;
            if(inv_ref_diff < 0)
            {
                gGridPowerSet = gGridPowerSet - inv_ref_diff;
                pv_power_set = pv_power_set + inv_ref_diff;
            }
//            gGridPowerSet =  VALUE_LIMIT(gGridPowerSet, 5000, 0);
            gPvPowerSet = 0;
            PvSetRunstate = 4;
            break;
        }
        case EN_STOPGENRATION_WITH_GRID_AND_PVSOFT:
        {
            master_count5 = 1;
            master_count7++;
            gGridPowerSet = load_power - 1500;
            float inv_ref_diff = gGridPowerSet - LOADPOWER + MAX_INV_OUT_POWER;
            if(inv_ref_diff < 0)
            {
                gGridPowerSet = gGridPowerSet - inv_ref_diff;
                pv_power_set = pv_power_set + inv_ref_diff;
            }
//            gGridPowerSet =  VALUE_LIMIT(gGridPowerSet, 5000, 0);
            gPvPowerSet = load_power - gBatRealPower - GRID_POWER + 100;
            gPvPowerSet = (gPvPowerSet > 1500)?gPvPowerSet:0;
            PvSetRunstate = 5;
            break;
        }
        case EN_STOPGENRATION_WITH_GRID_AND_PV:
        {
            master_count4 = 0;
            master_count5 = 0;
            gGridPowerSet = grid_power_set;
            float inv_ref_diff = gGridPowerSet - LOADPOWER + MAX_INV_OUT_POWER;
            if(inv_ref_diff < 0)
            {
                gGridPowerSet = gGridPowerSet - inv_ref_diff;
                pv_power_set = pv_power_set + inv_ref_diff;
            }
//            gGridPowerSet =  VALUE_LIMIT(gGridPowerSet, 5000, 0);
            gPvPowerSet = pv_power_set;
            PvSetRunstate = 6;
            break;
        }
        default:
        {
            master_count4 = 0;
            master_count5 = 0;
            gGridPowerSet = grid_power_set;
            float inv_ref_diff = gGridPowerSet - LOADPOWER + MAX_INV_OUT_POWER;
            if(inv_ref_diff < 0)
            {
                gGridPowerSet = gGridPowerSet - inv_ref_diff;
                pv_power_set = pv_power_set + inv_ref_diff;
            }
//            gGridPowerSet =  VALUE_LIMIT(gGridPowerSet, 5000, gFeedPower*(-1));
            gPvPowerSet = pv_power_set;
            PvSetRunstate = 7;
            break;
        }
    }
    gPvPowerSet = VALUE_LIMIT(gPvPowerSet, DUAL_PV_MAX_POWER, 0);

    if((gPvPowerSet > 1500.0f) || \
            (geEnergysta == EN_LOAD_WITH_GRID_AND_PVSOFT) || \
            (geEnergysta == EN_STOPGENRATION_WITH_GRID_AND_PVSOFT))
    {
        AlgoWarningSetPvRecoverPermit(1);
    }
    else
    {
        AlgoWarningSetPvRecoverPermit(0);
    }
    /***********************测试模式******************/
    if(gSysSetInfo.dc_source == 1)//测试模式下强制给定功率给4500
    {
        gPvPowerSet = load_power - gGridPowerSet - 100;
    }
    /*************************************************/
    limit_activepwr = sqrtf(25000000 - _gState_algo.inv_reactive_power * _gState_algo.inv_reactive_power);
    if(fabs(gGridPowerSet) > limit_activepwr)
    {
        gGridPowerSet = gGridPowerSet > 0 ? limit_activepwr : (-1 * limit_activepwr);
    }
    gGridPowerSet = _Var_slope(gGridPowerSet);
    AlgoSetGridPwr(gGridPowerSet);         //设置电网测有功功率
    AlgoSetReactivePwr(gGridReactivePowerSet + _Var_complete(GRID_VRMS,GRID_SPOWER,_gInvPwr.inv_output_reactive_pwr));//设置逆变侧无功功率
    pv_test2 = gPvPowerSet;
    grid_test = gGridPowerSet;
    //gPvPowerSet = 0;//gc
    MpptSetPv1Pwr(gPvPowerSet);     //设置PV功率
    /**************************************************/
    if ((pv_start_flag == 1) && (pv_startok_flag == 0)) //软起动是否完成判断
    {
        if (fabs(gGridPowerSet -  grid_power) <= 300)
        {
            pv_startok_flag = 1; //软起完成后置位标志位
            pv_start_flag = 0;   //清除软起开始标志位
        }
        else
        {
            pv_startok_flag = 0;
        }
    }
    pv_state_old = Read_Mppt_state();
}

/**
* @brief: LLC_Set_EN
* @param:
* @return:
* @note: 用于PV激活状态机下，打开LLC使能用
*
*/
void LLC_Set_EN()
{

    gEnLlcPwm = 1;
    EnLlc = 1;
}

float32 _Var_complete(float32 vrms, float32 spwr, float32 inv_reactive_pwr)
{
    float32 var_complete = 0.0f;
    static float32 inv_reactive_crr = 200.0f;
    static uint16_t count = 0;
    static float inv_reactive_comlete = 0.0f;
    if((inv_reactive_pwr > -100) && (inv_reactive_pwr <100))
    {
        count ++;
        if (count >= 12)
        {
            inv_reactive_crr = inv_reactive_crr * -1.0f;
            inv_reactive_comlete = inv_reactive_crr;
            count = 0;
        }

    }
    else
    {
        inv_reactive_comlete = 0.0f;
    }


    if(inv_reactive_pwr < -500)
    {
        var_complete = VAR_CAP + VAR_COM_STEP * spwr - vrms * vrms * Y_CAP + (vrms - 220.0f) + 80 + 250.0f;
    }
    else
    {
        var_complete = VAR_CAP + VAR_COM_STEP * spwr - vrms * vrms * Y_CAP + (vrms - 220.0f) + 250.0f;
    }
    return var_complete;
}

float32 _Var_slope(float32 grid_power_ref_in)
{
    static float32 grid_power_ref_out = 0.0;
    static float32 slope = 1.3915f; //0.167pu/min
    float32 diff_pwr = grid_power_ref_in - grid_power_ref_out;
    if(((GRID_VRMS > VRMS_VV2) && (GRID_VRMS < VRMS_VV3)) && \
            ((_gState_algo.grid_freq < FREQ_ULCO) && ( _gState_algo.grid_freq > FREQ_LLCO)) && \
            (ThreadBreakDetect() == 0))
    {
        slope = 1.3915f;
    }
    else
    {
        slope = 10000.0f;
    }
    if(diff_pwr > slope)
    {
        diff_pwr = slope;
    }
    else if(diff_pwr < -slope)
    {
        diff_pwr = -slope;
    }
    grid_power_ref_out = grid_power_ref_out + diff_pwr;
    if(grid_power_ref_out > 5000.0f)
    {
        grid_power_ref_out = 5000.0f;
    }
    else if(grid_power_ref_out < -5000.0f)
    {
        grid_power_ref_out = -5000.0f;
    }
    grid_power_ref_out = grid_power_ref_in;
    return grid_power_ref_out;
}

float master_miss_power  = 0;//针对总负载，还欠缺的能量
////////////////并机相关/////////////////
/**
* @brief: 并网并机模式下主机能量分配
* @param: mode:0-自发自用，1-电池优先，2-削峰填谷(静态)，3-削峰填谷(买电)，4-削峰填谷(卖电)
* @return: 光伏是否步进
* @note:
*/
bool ParallelMasterPowerAlloc(uint8_t mode)
{
    float total_load_power;
    float slave_grid_port_max_power = gParallelComm.slave_info.slave_grid_port_allow_max_power;
    float slave_grid_port_min_power = gParallelComm.slave_info.slave_grid_port_allow_min_power;
    float slave_grid_port_power = gParallelComm.slave_info.slave_grid_port_power;//从机并网口功率
    float slave_pv_power = gParallelComm.slave_info.slave_pv_power;
    float slave_bat_power = gParallelComm.slave_info.slave_bat_power;
    float slave_bat_allow_max_power = gParallelComm.slave_info.slave_bat_allow_max_power;
    float slave_bat_allow_min_power = gParallelComm.slave_info.slave_bat_allow_min_power;

    float slave_grid_port_power_set = gParallelComm.slave_info.slave_grid_port_power;//要设置给从机并网口的功率
    float grid_port_min_power;//最大输出功率
    float grid_port_max_power;//最大输入功率
    float bat_max_power = gDsgPower;
    float bat_min_power = 0 - gChgPower;
    float ideal_total_grid_power;//期望并网口总功率
    float ideal_master_grid_power;//期望主机并网口总功率
    float mast_grid_port_power = GRID_POWER;

    bool pv_volt_valid_flag = _PVValidJudge();
    float pv_power;

    bool ret = true;//光伏下次扰动的值

    float nor_load = (-gAntiReFlowValue) - slave_grid_port_power;
    float nor_load_with_slave = (-gAntiReFlowValue);
    float backup_load = LOADPOWER;

    float feed_limit = gFeedPower;
    uint16_t master_import_limit_flag = 0;//主机触发最大入户功率限制
    float ct_power = _gState_algo.CT_active_power;
    float master_max_output_power = 0;
    master_miss_power = 0;
    if(mode == 0 || mode == 1 || mode == 4)
    {
        feed_limit = 1 * gFeedPower; // 20260303，修改并机状态下最大卖电功率直接等于监控卡面板的设置功率，不进行乘2
    }
    else if(mode == 2 || mode == 3)
    {
        feed_limit = 0;
    }
    else
    {
        ret = false;
        return ret;
    }
    if(gSysSetInfo.antireverseflow_enable == 1)
    {
        feed_limit = 0;
    }

    if(pv_volt_valid_flag == true)
    {
        pv_power = PV1POWER;
        if(pv_power < 0)
        {
            pv_power = 0;
        }
    }
    else
    {
        pv_power = 0;
    }

    if((gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0) &&
            (gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0))
    {
        gBatSetPower = 0;
        bat_max_power = 0;
        bat_min_power = 0;
    }
    else if(gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0)
    {
        bat_max_power = DISCHG_CHG_PWR;
        if(bat_min_power > bat_max_power)
        {
            bat_min_power = bat_max_power;
        }
    }
    else if(gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0)
    {
        bat_min_power = CHG_DISCHG_PWR;
        if(bat_max_power < bat_min_power)
        {
            bat_max_power = bat_min_power;
        }
    }
    // else
    // {

    // }
    //master_count4 = bat_max_power;

    //自发自用的情况下，并网口总期望功率为远端负载实际功率
    // ideal_total_grid_power = nor_load;
    // ideal_master_grid_power = ideal_total_grid_power - slave_grid_port_power;//实际就是(-gAntiReFlowValue)
    master_max_output_power = pv_power + bat_max_power;
    if(master_max_output_power > 5000)
    {
        master_max_output_power = 5000;
    }
    if(mode == 0 || mode == 2)//自发自用或削峰填谷(静态)
    {
        total_load_power = backup_load + nor_load_with_slave;//只包含本地负载和远端负载
        master_miss_power = total_load_power - master_max_output_power;
        //master_miss_power = total_load_power - 5000;
        total_load_power = VALUE_LIMIT(total_load_power, 5000, -5000);
        gBatSetPower = total_load_power - pv_power;
    }
    else if(mode == 1 || mode == 3)//电池优先或削峰填谷(买电)
    {
        gBatSetPower = gChgPower*(-1);
        master_miss_power = 0;
    }
    else if(mode == 4)//削峰填谷(卖电)
    {
        total_load_power = backup_load + nor_load_with_slave + feed_limit;//包含本地负载和远端负载+最大并网功率
        master_miss_power = total_load_power - master_max_output_power;
        //master_miss_power = total_load_power - 5000;
        total_load_power = VALUE_LIMIT(total_load_power, 5000, -5000);
        gBatSetPower = total_load_power - pv_power;
    }


    gBatSetPower = VALUE_LIMIT(gBatSetPower, bat_max_power, bat_min_power);
    grid_port_min_power = (-1)*feed_limit - nor_load_with_slave;
    if(grid_port_min_power < -5000 + backup_load)
    {
        grid_port_min_power = -5000 + backup_load;
    }
    if(grid_port_min_power < 0 - (bat_max_power + pv_power))
    {
        grid_port_min_power = 0 - (bat_max_power + pv_power);
    }

    grid_port_max_power = backup_load - (pv_power + bat_min_power);
    // grid_port_max_power = gNetworkPower - nor_load_with_slave;
    if(grid_port_max_power > 5000)
    {
        grid_port_max_power = 5000;
    }
    if(grid_port_max_power > gNetworkPower - nor_load_with_slave)
    {
        grid_port_max_power = gNetworkPower - nor_load_with_slave;
        master_import_limit_flag = 1;
    }
    gBatRealPower = _gBatRealPower(gBatSetPower);
    gGridPowerSet = -pv_power + backup_load - gBatRealPower;
    gPvPowerSet = pv_power;
    master_count2 = bat_min_power;
    master_count3 = grid_port_min_power;
    if(gGridPowerSet < grid_port_min_power - 100)//并网能量超限制(留100W余量),只有当远端负载突然减小且光伏能量大的情况下才会触发
    {
        float diff = grid_port_min_power - gGridPowerSet;
        if(gBatRealPower > gBatSetPower)//电池充电功率受限，不能由电池吸收多出的能量
        {
            gPvPowerSet -= diff;
            if(gPvPowerSet < 0)
            {
                gPvPowerSet = 0;
            }
        }
        else
        {
            if(gBatRealPower - bat_min_power >= diff)//多出的能量可以由电池全部吸收
            {
                gBatRealPower -= diff;
                gGridPowerSet +=  diff;
            }
            else//电池只能吸收部分
            {
                gBatRealPower = bat_min_power;
                gPvPowerSet -= (diff - (gBatRealPower - bat_min_power));
                if(gPvPowerSet < 0)
                {
                    gPvPowerSet = 0;//从机输出太多
                }
                gGridPowerSet = -gPvPowerSet + backup_load - gBatRealPower;
            }
        }
        master_count = diff;
    }

    if(gGridPowerSet > grid_port_max_power + 100)//吸收电网能量超限制(留100W余量)
    {
        gGridPowerSet = grid_port_max_power;
        gBatRealPower = (0 - gPvPowerSet) + backup_load - gGridPowerSet;
    }
    gGridPowerSet = VALUE_LIMIT(gGridPowerSet,5000,-5000);
    if(ThreadBreakDetect() == 1)
    {
        gstFaultInfo.worning_code.b.r_ct_reverse = 0;
        gGridPowerSet = 0;
        ret = false;
        return ret;
    }

    if(mode != 1 && mode != 3)//电池优先和削峰填谷(买电)不执行输出均衡策略
    {
        //判断两台设备谁的电池出力多
        if(gBatRealPower - slave_bat_power >= 200)//主机出力多
        {
            if(mode == 0 && ct_power <= -100 && (slave_bat_power - slave_bat_allow_min_power >= 100))//特殊情况，防止在自发自用模式下，从机为了并网而不给电池充电
            {
                if(slave_grid_port_max_power - slave_grid_port_power >= 100)
                {
                    slave_grid_port_power_set = slave_grid_port_power + 100;
                }
            }
            else
            {
                if(slave_grid_port_min_power - slave_grid_port_power <= -100)//从机有输出余量
                {
                    slave_grid_port_power_set = slave_grid_port_power - 100;
                }
            }
        }
        else if(gBatRealPower - slave_bat_power <= -200)//从机出力多
        {
            if(master_miss_power > 100)//整体输出能量欠缺
            {
                if(slave_grid_port_min_power - slave_grid_port_power <= -100)//从机有输出余量
                {
                    slave_grid_port_power_set = slave_grid_port_power - 100;
                }
            }
            else if(master_miss_power <= -100)//输出能量有富余
            {
                if(bat_max_power - gBatRealPower >= 100)//电池有释放余量
                {
                    if(slave_grid_port_max_power - slave_grid_port_power >= 100)//从机有输入余量
                    {
                        slave_grid_port_power_set = slave_grid_port_power + 100;
                    }
                }
            }
        }
        else
        {
            if(master_miss_power > 100)
            {
                if(slave_grid_port_min_power - slave_grid_port_power <= -100)//从机有输出余量
                {
                    slave_grid_port_power_set = slave_grid_port_power - 100;
                }
            }
            else if(master_miss_power <= -100)
            {
                if(mode == 0 && ct_power <= -100 && (slave_bat_power - slave_bat_allow_min_power >= 100))//特殊情况，防止在自发自用模式下，从机为了并网而不给电池充电
                {
                    if(slave_grid_port_max_power - slave_grid_port_power >= 100)
                    {
                        slave_grid_port_power_set = slave_grid_port_power + 100;
                    }
                }
            }
        }
    }
    else//电池优先和削峰填谷(买电)的情况
    {
        //按照从机最大输入功率赋值
        if(master_import_limit_flag == 0)
        {
            if(slave_grid_port_power_set <= slave_grid_port_max_power - 100)
            {
                slave_grid_port_power_set += 100;
            }
        }
        else//触发最大入户限制，控制从机，主机下一拍自适应
        {
            if(gBatRealPower - slave_bat_power > 100)//主机充电功率小于从机，则从机-100
            {
                if(slave_grid_port_power_set >= slave_grid_port_min_power + 100)
                {
                    slave_grid_port_power_set -= 100;
                }
            }
            else if(gBatRealPower - slave_bat_power < -100)
            {
                if(slave_grid_port_power_set <= slave_grid_port_max_power - 100)
                {
                    slave_grid_port_power_set += 100;
                }
            }
        }
    }

    if(-(gGridPowerSet + slave_grid_port_power_set) - nor_load >= feed_limit + 200)//如果并网太多，尝试先减从机
    {
        if(slave_grid_port_max_power - slave_grid_port_power_set >= 150)//从机有输入余量
        {
            slave_grid_port_power_set += 150;
        }
    }
    else
    {
        //flag3 = 0;
    }
    if((gGridPowerSet + slave_grid_port_power_set) + nor_load > gNetworkPower + 100)//如果主从机输入+远端负责超过最大入户功率，降低从机输入，主机下一拍自动适配
    {
        if(slave_grid_port_min_power - slave_grid_port_power_set <= -100)//从机有输入余量
        {
            slave_grid_port_power_set -= 100;
        }
    }

    gBatRealPower = (0 - gPvPowerSet) + backup_load - gGridPowerSet;

    //判断光伏是否该扰动
    if((gPvPowerSet + bat_min_power < 5000 - 100) && gPvPowerSet <= gMaxPvPwr - 100)
    {
        if(gBatRealPower + gGridPowerSet - 100 < bat_min_power + grid_port_min_power)
        {
            ret = false;
        }
        else
        {
            ret = true;
        }
    }
    else
    {
        gPvPowerSet = POWER_VALUE_LIMIT(5000 - bat_min_power, gMaxPvPwr);
        ret = false;
    }

    //设置从机数据
    gParallelComm.slave_set.slave_grid_port_power_set = slave_grid_port_power_set;
    return ret;
}

/**
 * @brief   并机从机功率分配器 — 根据本机电池充放电能力、光伏状态和负载需求，
 *          计算电网口允许功率范围，执行功率分配，并判断光伏是否可继续扰动。
 *
 * @details 处理流程：
 *          1. 读取电池充放电功率能力（bat_max/bat_min）和本地负载；
 *          2. 判断 PV 电压有效性，确定光伏功率；
 *          3. 根据充放电使能位仲裁电池功率范围，含死区保护
 *            （DISCHG_CHG_PWR / CHG_DISCHG_PWR）；
 *          4. 由能量守恒公式计算电网口允许的最小/最大功率，并做 ±5000W 硬件限幅；
 *          5. 将主机下发的电网口设定值钳位到允许范围内；
 *          6. 按 gBat = -gPv + Load - gGrid 反算电池实际功率，保持功率平衡；
 *          7. 综合判断光伏是否仍有 MPPT 扰动空间；
 *          8. 将本机功率能力与实际值写入 slave_info 供主机读取。
 *
 * @return  bool 光伏扰动允许标志。
 *          - true:  功率平衡允许光伏继续升功率扰动；
 *          - false: 光伏已达上限或功率平衡不允许，应停止扰动。
 *
 * @note    功率符号约定：正数表示放出/注入能量，负数表示吸收/消耗能量。
 * @note    电网口功率公式：grid_port = backup_load - pv_power - bat_power。
 * @note    当充放电均禁止时，电池功率清零；仅允许单向时，用死区常数防止功率越界到反向。
 */
bool ParallelSlavePowerAlloc()
{
    float bat_max_power = gDsgPower;
    float bat_min_power = 0 - gChgPower;
    float grid_port_min_power;//最大输出功率
    float grid_port_max_power;//最大输入功率

    bool pv_volt_valid_flag = _PVValidJudge();
    float pv_power;

    bool ret = true;//光伏下次扰动的值
    float backup_load = LOADPOWER;

    if(pv_volt_valid_flag == true)
    {
        pv_power = PV1POWER;
        if(pv_power < 0)
        {
            pv_power = 0;
        }
    }
    else
    {
        pv_power = 0;
    }

    if((gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0) &&
            (gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0))
    {
        gBatSetPower = 0;
        bat_max_power = 0;
        bat_min_power = 0;
    }
    else if(gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0)
    {
        bat_max_power = DISCHG_CHG_PWR;
        if(bat_min_power > bat_max_power)
        {
            bat_min_power = bat_max_power;
        }
    }
    else if(gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0)
    {
        bat_min_power = CHG_DISCHG_PWR;
        if(bat_max_power < bat_min_power)
        {
            bat_max_power = bat_min_power;
        }
    }

    //先计算当前电网口允许的最大最小功率
    grid_port_min_power = backup_load - (pv_power + bat_max_power);
    if(grid_port_min_power < -5000 + backup_load)
    {
        grid_port_min_power = -5000 + backup_load;
    }

    grid_port_max_power = backup_load - (pv_power + bat_min_power);
    if(grid_port_max_power > 5000)
    {
        grid_port_max_power = 5000;
    }

    gGridPowerSet = gParallelComm.slave_set.slave_grid_port_power_set;
    gGridPowerSet = VALUE_LIMIT(gGridPowerSet, grid_port_max_power, grid_port_min_power);

    gPvPowerSet = pv_power;

    gBatSetPower = (0 - gPvPowerSet) + backup_load - gGridPowerSet;
    gBatRealPower = gBatSetPower;//_gBatRealPower(gBatSetPower);

    //判断光伏是否该扰动
    if((gPvPowerSet + bat_min_power < 5000 - 100) && gPvPowerSet <= gMaxPvPwr - 100)
    {
        if(gBatRealPower + gGridPowerSet - 100 < bat_min_power + grid_port_min_power)
        {
            ret = false;
        }
        else
        {
            ret = true;
        }
    }
    else
    {
        gPvPowerSet = POWER_VALUE_LIMIT(5000 - bat_min_power, gMaxPvPwr);
        ret = false;
    }

    gParallelComm.slave_info.slave_grid_port_allow_max_power = grid_port_max_power;
    gParallelComm.slave_info.slave_grid_port_allow_min_power = grid_port_min_power;
    gParallelComm.slave_info.slave_grid_port_power = GRID_POWER;//gGridPowerSet;
    gParallelComm.slave_info.slave_backup_power = backup_load;
    gParallelComm.slave_info.slave_pv_power = gPvPowerSet;
    gParallelComm.slave_info.slave_bat_power = gBatRealPower;
    gParallelComm.slave_info.slave_bat_allow_max_power = bat_max_power;
    gParallelComm.slave_info.slave_bat_allow_min_power = bat_min_power;

    return ret;
}

/**
* @brief: 非并机模式下数据清除
* @param: none
* @return: none
* @note:
*/
void NonParallelDataClear()
{
    gParallelComm.slave_info.slave_grid_port_allow_max_power = 0;
    gParallelComm.slave_info.slave_grid_port_allow_min_power = 0;
    gParallelComm.slave_info.slave_grid_port_power = 0;
    gParallelComm.slave_info.slave_backup_power = 0;
    gParallelComm.slave_info.slave_pv_power = 0;
    gParallelComm.slave_info.slave_bat_power = 0;
    gParallelComm.slave_info.slave_bat_allow_max_power = 0;
    gParallelComm.slave_info.slave_bat_allow_min_power = 0;
}


/**
* @brief: 主机离网
* @param:  none
* @return: none
* @note:
*/
void ParallelMasterOffGrid()
{
    int32_t slave_grid_port_max_power = gParallelComm.slave_info.slave_grid_port_allow_max_power;
    int32_t slave_grid_port_min_power = gParallelComm.slave_info.slave_grid_port_allow_min_power;
    int32_t slave_grid_port_power = gParallelComm.slave_info.slave_grid_port_power;
    float ct_power = _gState_algo.CT_active_power;
    if(gParallelComm.mode == 1)
    {
        if(geInvsta == EN_OFF_GRID_MODE && (slave_grid_port_max_power != 0 || slave_grid_port_min_power != 0)) //主机离网，且并机开启，且从机并网
        {
//            float feed_limit = gFeedPower;

//            if(_gState_algo.ct_cur_rms_ave <= 2.0)
//            {
//                ct_power = (_gState_algo.ct_cur_rms_ave - 1.5) * 230;
//            }
//            else
//            {
//                ct_power = (_gState_algo.ct_cur_rms_ave - 0.3) * 230;
//            }
//            if(ct_power > 100)
//            {
//                gParallelComm.slave_set.slave_grid_port_power_set = slave_grid_port_power - 100;
//            }
            gParallelComm.slave_set.slave_grid_port_power_set = 0;//临时方案20260513
        }
        else
        {
            gParallelComm.slave_set.slave_grid_port_power_set = 0;
        }
    }
}

/**
* @brief: 从机离网
* @param:  none
* @return: none
* @note:
*/
void ParallelSlaveOffGrid()
{
    float bat_max_power = gDsgPower;
    float bat_min_power = 0 - gChgPower;

    if((gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0) &&
            (gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0))
    {
        gBatSetPower = 0;
        bat_max_power = 0;
        bat_min_power = 0;
    }
    else if(gSysSetInfo.pcs_dsg_ctrl == 0 || gDsgPower == 0)
    {
        bat_max_power = DISCHG_CHG_PWR;
        if(bat_min_power > bat_max_power)
        {
            bat_min_power = bat_max_power;
        }
    }
    else if(gSysSetInfo.pcs_chg_ctrl == 0 || gChgPower == 0)
    {
        bat_min_power = CHG_DISCHG_PWR;
        if(bat_max_power < bat_min_power)
        {
            bat_max_power = bat_min_power;
        }
    }

    if(gParallelComm.mode == 2)
    {
        gParallelComm.slave_info.slave_grid_port_allow_max_power = 0;
        gParallelComm.slave_info.slave_grid_port_allow_min_power = 0;
        gParallelComm.slave_set.slave_grid_port_power_set = 0;
        gParallelComm.slave_info.slave_grid_port_power = 0;
        gParallelComm.slave_info.slave_pv_power  = PV1POWER;
        gParallelComm.slave_info.slave_bat_power = (0 - PV1POWER) + LOADPOWER;
        gParallelComm.slave_info.slave_backup_power = LOADPOWER;
        gParallelComm.slave_info.slave_bat_allow_max_power = bat_max_power;
        gParallelComm.slave_info.slave_bat_allow_min_power = bat_min_power;
    }
}

#if CSIP_SELF_TEST
// CSIP认证查表重构：将27个hl_crossover_enable分支替换为常量表
typedef struct {
    uint16_t import_source;
    uint16_t import_ramp_type;
    uint16_t import_ramp_value;
    uint16_t import_max_power;
    uint16_t ac_in_source;
    uint16_t ac_in_ramp_type;
    uint16_t ac_in_ramp_value;
    uint16_t ac_in_max_power;
    uint16_t export_source;
    uint16_t export_ramp_type;
    uint16_t export_ramp_value;
    uint16_t export_max_power;
    uint16_t ac_out_source;
    uint16_t ac_out_ramp_type;
    uint16_t ac_out_ramp_value;
    uint16_t ac_out_max_power;
} CSIPCertTable_t;

// 字段含义 (source=功率来源 / type=爬坡类型 / value=爬坡值 / max=最大功率):
//   source: 0=队列(queue)  1=默认值(default)
//   type:   0=时间型(秒)   1=功率型(W)  65535=不限制(直通)
//   value:  type=0时为爬坡时间(s), type=1时为爬坡功率(W), type=65535时无效
//   max:    该侧最大功率限制(W)
// 字段顺序: {import_src, import_type, import_value, import_max,
//            ac_in_src, ac_in_type, ac_in_value, ac_in_max,
//            export_src, export_type, export_value, export_max,
//            ac_out_src, ac_out_type, ac_out_value, ac_out_max}
static const CSIPCertTable_t csipCertConfig[] = {
    // index 0: unused (保留)
    //  src type value  max   |  src type value  max  |  src  type value  max   |  src type value  max
    {0},
    // index 1: import=队列+不限制+10s+5kw | ac_in=队列+时间型+20s+4kw | export=队列+不限制+10s+5kw | ac_out=队列+时间型+20s+4kw
    {0, 65535, 10, 5000,  0, 0, 20, 4000,  0, 65535, 10, 5000,  0, 0, 20, 4000},
    // index 2: import=队列+不限制+10s+5kw | ac_in=队列+时间型+20s+4kw | export=队列+不限制+10s+5kw | ac_out=队列+时间型+5s+4kw
    {0, 65535, 10, 5000,  0, 0, 20, 4000,  0, 65535, 10, 5000,  0, 0, 5,  4000},
    // index 3: import=队列+时间型+10s+5kw | ac_in=队列+不限制+20s+4kw | export=队列+时间型+10s+5kw | ac_out=队列+不限制+20s+4kw
    {0, 0,     10, 5000,  0, 65535, 20, 4000,  0, 0,     10, 5000,  0, 65535, 20, 4000},
    // index 4: import=队列+时间型+2s+5kw  | ac_in=队列+不限制+5s+4kw  | export=队列+时间型+2s+5kw  | ac_out=队列+不限制+5s+4kw
    {0, 0,     2,  5000,  0, 65535, 5,  4000,  0, 0,     2,  5000,  0, 65535, 5,  4000},
    // index 5: import=队列+时间型+10s+5kw | ac_in=队列+时间型+20s+4kw | export=队列+时间型+10s+5kw | ac_out=队列+时间型+20s+4kw
    {0, 0,     10, 5000,  0, 0,     20, 4000,  0, 0,     10, 5000,  0, 0,     20, 4000},
    // index 6: import=队列+时间型+10s+5kw | ac_in=队列+时间型+5s+4kw  | export=队列+时间型+10s+5kw | ac_out=队列+时间型+5s+4kw
    {0, 0,     10, 5000,  0, 0,     5,  4000,  0, 0,     10, 5000,  0, 0,     5,  4000},
    // index 7: import=队列+功率型+1kw+5kw | ac_in=队列+功率型+2kw+4kw  | export=队列+功率型+1kw+5kw | ac_out=队列+功率型+2kw+4kw
    {0, 1,     1000, 5000,  0, 1,   2000, 4000,  0, 1,   1000, 5000,  0, 1,   2000, 4000},
    // index 8: import=队列+功率型+1kw+5kw | ac_in=队列+功率型+0.5kw+4kw | export=队列+功率型+1kw+5kw | ac_out=队列+功率型+0.5kw+4kw
    {0, 1,     1000, 5000,  0, 1,   500,  4000,  0, 1,   1000, 5000,  0, 1,   500,  4000},
    // index 9: import=队列+不限制+10s+5kw | ac_in=队列+不限制+20s+4kw | export=队列+不限制+10s+5kw | ac_out=队列+不限制+20s+4kw
    {0, 65535, 10,   5000,  0, 65535, 20, 4000,  0, 65535, 10, 5000,  0, 65535, 20, 4000},
    // index 10: import=队列+不限制+10s+5kw | ac_in=队列+不限制+5s+4kw  | export=队列+不限制+10s+5kw | ac_out=队列+不限制+5s+4kw
    {0, 65535, 10,   5000,  0, 65535, 5,  4000,  0, 65535, 10, 5000,  0, 65535, 5,  4000},
    // index 11: import=队列+时间型+10s+5kw | ac_in=默认+时间型+5s+4kw    | export=队列+时间型+10s+5kw | ac_out=默认+时间型+55s+4kw
    {0, 0,     10,   5000,  1, 0,     5,  4000,  0, 0,     10, 5000,  1, 0,     55, 4000},
    // index 12: import=队列+功率型+1kw+5kw | ac_in=默认+功率型+2kw+4kw  | export=队列+功率型+1kw+5kw | ac_out=默认+功率型+2kw+4kw
    {0, 1,     1000, 5000,  1, 1,     2000, 4000,  0, 1,   1000, 5000,  1, 1,   2000, 4000},
    // index 13: import=队列+不限制+10s+5kw | ac_in=默认+时间型+20s+4kw  | export=队列+不限制+10s+5kw | ac_out=默认+时间型+20s+4kw
    {0, 65535, 10,   5000,  1, 0,     20,  4000,  0, 65535, 10, 5000,  1, 0,   20,  4000},
    // index 14: import=默认+时间型+10s+5kw | ac_in=队列+时间型+20s+4kw   | export=默认+时间型+10s+5kw | ac_out=队列+时间型+20s+5kw
    {1, 0,     10,   5000,  0, 0,     20,  4000,  1, 0,     10, 5000,  0, 0,   20,  5000},
    // index 15: import=默认+功率型+4kw+5kw | ac_in=队列+功率型+2kw+4kw   | export=默认+功率型+4kw+5kw | ac_out=队列+功率型+2kw+4kw
    {1, 1,     4000, 5000,  0, 1,     2000, 4000,  1, 1,   4000, 5000,  0, 1,   2000, 4000},
    // index 16: import=默认+时间型+10s+5kw | ac_in=队列+不限制+20s+4kw   | export=默认+时间型+10s+5kw | ac_out=队列+不限制+20s+4kw
    {1, 0,     10,   5000,  0, 65535, 20,  4000,  1, 0,     10, 5000,  0, 65535, 20, 4000},
    // index 17: import=默认+不限制+10s+5kw | ac_in=默认+时间型+20s+4kw   | export=默认+不限制+10s+5kw | ac_out=默认+时间型+20s+4kw
    {1, 65535, 10,   5000,  1, 0,     20,  4000,  1, 65535, 10, 5000,  1, 0,   20,  4000},
    // index 18: import=默认+不限制+10s+5kw | ac_in=默认+时间型+5s+4kw    | export=默认+不限制+10s+5kw | ac_out=默认+时间型+5s+4kw
    {1, 65535, 10,   5000,  1, 0,     5,   4000,  1, 65535, 10, 5000,  1, 0,   5,   4000},
    // index 19: import=默认+时间型+10s+5kw | ac_in=默认+不限制+20s+4kw   | export=默认+时间型+10s+5kw | ac_out=默认+不限制+20s+4kw
    {1, 0,     10,   5000,  1, 65535, 20,  4000,  1, 0,     10, 5000,  1, 65535, 20, 4000},
    // index 20: import=默认+时间型+10s+5kw | ac_in=默认+不限制+5s+4kw    | export=默认+时间型+10s+5kw | ac_out=默认+不限制+5s+4kw
    {1, 0,     10,   5000,  1, 65535, 5,   4000,  1, 0,     10, 5000,  1, 65535, 5,  4000},
    // index 21: import=默认+时间型+10s+5kw | ac_in=默认+时间型+20s+4kw   | export=默认+时间型+10s+5kw | ac_out=默认+时间型+20s+4kw
    {1, 0,     10,   5000,  1, 0,     20,  4000,  1, 0,     10, 5000,  1, 0,   20,  4000},
    // index 22: import=默认+时间型+10s+5kw | ac_in=默认+时间型+5s+4kw    | export=默认+时间型+10s+5kw | ac_out=默认+时间型+5s+4kw
    {1, 0,     10,   5000,  1, 0,     5,   4000,  1, 0,     10, 5000,  1, 0,   5,   4000},
    // index 23: import=默认+功率型+1kw+5kw | ac_in=默认+功率型+2kw+4kw   | export=默认+功率型+1kw+5kw | ac_out=默认+功率型+2kw+4kw
    {1, 1,     1000, 5000,  1, 1,     2000, 4000,  1, 1,   1000, 5000,  1, 1,   2000, 4000},
    // index 24: import=默认+功率型+1kw+5kw | ac_in=默认+功率型+0.5kw+4kw | export=默认+功率型+1kw+5kw | ac_out=默认+功率型+0.5kw+4kw
    {1, 1,     1000, 5000,  1, 1,     500,  4000,  1, 1,   1000, 5000,  1, 1,   500,  4000},
    // index 25: import=默认+不限制+10s+5kw | ac_in=默认+不限制+20s+4kw   | export=默认+不限制+10s+5kw | ac_out=默认+不限制+20s+4kw
    {1, 65535, 10,   5000,  1, 65535, 20,  4000,  1, 65535, 10, 5000,  1, 65535, 20, 4000},
    // index 26: import=默认+不限制+10s+5kw | ac_in=默认+不限制+5s+4kw    | export=默认+不限制+10s+5kw | ac_out=默认+不限制+5s+4kw
    {1, 65535, 10,   5000,  1, 65535, 5,   4000,  1, 65535, 10, 5000,  1, 65535, 5,  4000},
    // index 27: import=队列+时间型+10s+1kw | ac_in=默认+不限制+5s+4kw    | export=队列+不限制+10s+15kw | ac_out=默认+不限制+5s+4kw
    {0, 0,     10,   1000,  1, 65535, 5,   4000,  0, 65535, 10, 15000, 1, 65535, 5,   4000},
    // index 28: import=默认+时间型+10s+1kw | ac_in=队列+时间型+5s+1kw    | export=队列+不限制+10s+15kw | ac_out=默认+不限制+5s+4kw
    {1, 0,     10,   1000,  0, 0,     5,   1000,  0, 65535, 10, 15000, 1, 65535, 5,   4000},
    // index 29: import=队列+不限制+10s+15kw | ac_in=默认+不限制+5s+4kw   | export=队列+时间型+10s+1kw  | ac_out=默认+不限制+5s+4kw
    {0, 65535, 10,   15000, 1, 65535, 5,   4000,  0, 0,     10, 1000,  1, 65535, 5,   4000},
    // index 30: unused (预留)
    {0},
    // index 31: import=队列+不限制+10s+15kw | ac_in=默认+不限制+5s+4kw   | export=默认+时间型+10s+1kw  | ac_out=队列+时间型+5s+1kw
    {0, 65535, 10,   15000, 1, 65535, 5,   4000,  1, 0,     10, 1000,  0, 0,     5,   1000},
    // index 32: import=队列+时间型+10s+1kw | ac_in=默认+不限制+5s+4kw    | export=队列+时间型+10s+1kw  | ac_out=默认+不限制+5s+4kw
    {0, 0,     10,   1000,  1, 65535, 5,   4000,  0, 0,     10, 1000,  1, 65535, 5,   4000},
    // index 33: import=默认+时间型+10s+1kw | ac_in=队列+时间型+5s+1kw    | export=默认+时间型+10s+1kw  | ac_out=队列+时间型+5s+1kw
    {1, 0,     10,   1000,  0, 0,     5,   1000,  1, 0,     10, 1000,  0, 0,     5,   1000},
};
#endif

/**
* @brief: CSIP并网认证
* @param:  none
* @return: none
* @note:
*/
// 定义CSIP并网认证处理函数，此函数放在主程序的100ms定时轮询任务中，每隔100ms周期性执行一次
void CSIPOnGridCertification(void)  // 此函数放在100ms轮询里面，每100ms执行一次
{
    uint32_t inverter_max_power = 0;
    uint32_t import_stept_count_max = 0;
    uint32_t export_stept_count_max = 0;
    static uint32_t import_stept_count = 0;
    static uint32_t export_stept_count = 0;
#if CSIP_SELF_TEST
    // 特殊值254: 关闭CSIP
    if(ModbusReg.hl_crossover_enable == 254)
    {
        gstModbusRegCSIP.CSIP_enable = 0;
    }
    // 特殊值255: 辅助认证模式（输入清零，AC侧设500W）
    else if(ModbusReg.hl_crossover_enable == 255)
    {
        gstModbusRegCSIP.CSIP_enable = 1;

        gstModbusRegCSIP.CSIP_import_power_source = 0;
        gstModbusRegCSIP.CSIP_import_power_ramp_up_type = 0;
        gstModbusRegCSIP.CSIP_import_power_ramp_up_value = 10;
        gstModbusRegCSIP.CSIP_import_max_power[0] = 0;

        gstModbusRegCSIP.CSIP_ac_in_power_source = 1;
        gstModbusRegCSIP.CSIP_ac_in_power_ramp_up_type = 0;
        gstModbusRegCSIP.CSIP_ac_in_power_ramp_up_value = 5;
        gstModbusRegCSIP.CSIP_ac_in_max_power[0] = 500;

        gstModbusRegCSIP.CSIP_export_power_source = 0;
        gstModbusRegCSIP.CSIP_export_power_ramp_up_type = 0;
        gstModbusRegCSIP.CSIP_export_power_ramp_up_value = 10;
        gstModbusRegCSIP.CSIP_export_max_power[0] = 0;

        gstModbusRegCSIP.CSIP_ac_out_power_source = 1;
        gstModbusRegCSIP.CSIP_ac_out_power_ramp_up_type = 0;
        gstModbusRegCSIP.CSIP_ac_out_power_ramp_up_value = 5;
        gstModbusRegCSIP.CSIP_ac_out_max_power[0] = 500;
    }
    // 查表值1-33（含27-29、31-33），30预留
    else if(ModbusReg.hl_crossover_enable >= 1 && ModbusReg.hl_crossover_enable <= 33)
    {
        const CSIPCertTable_t *cfg = &csipCertConfig[ModbusReg.hl_crossover_enable];

        gstModbusRegCSIP.CSIP_enable = 1;
        gstModbusRegCSIP.CSIP_import_power_source         = cfg->import_source;
        gstModbusRegCSIP.CSIP_import_power_ramp_up_type    = cfg->import_ramp_type;
        gstModbusRegCSIP.CSIP_import_power_ramp_up_value   = cfg->import_ramp_value;
        gstModbusRegCSIP.CSIP_import_max_power[0]          = cfg->import_max_power;
        gstModbusRegCSIP.CSIP_ac_in_power_source           = cfg->ac_in_source;
        gstModbusRegCSIP.CSIP_ac_in_power_ramp_up_type     = cfg->ac_in_ramp_type;
        gstModbusRegCSIP.CSIP_ac_in_power_ramp_up_value    = cfg->ac_in_ramp_value;
        gstModbusRegCSIP.CSIP_ac_in_max_power[0]           = cfg->ac_in_max_power;
        gstModbusRegCSIP.CSIP_export_power_source          = cfg->export_source;
        gstModbusRegCSIP.CSIP_export_power_ramp_up_type    = cfg->export_ramp_type;
        gstModbusRegCSIP.CSIP_export_power_ramp_up_value   = cfg->export_ramp_value;
        gstModbusRegCSIP.CSIP_export_max_power[0]          = cfg->export_max_power;
        gstModbusRegCSIP.CSIP_ac_out_power_source          = cfg->ac_out_source;
        gstModbusRegCSIP.CSIP_ac_out_power_ramp_up_type    = cfg->ac_out_ramp_type;
        gstModbusRegCSIP.CSIP_ac_out_power_ramp_up_value   = cfg->ac_out_ramp_value;
        gstModbusRegCSIP.CSIP_ac_out_max_power[0]          = cfg->ac_out_max_power;
}
#endif
    // 从Modbus寄存器中读取CSIP功能的全局使能标志位
    gstCSIP.flag_csip_enable                = gstModbusRegCSIP.CSIP_enable;
    // 新增：读取CT和AC输入/输出的power source标志位 (0: 队列, 1: 默认值)
    gstCSIP.flag_import_ct_source           = gstModbusRegCSIP.CSIP_import_power_source;
    gstCSIP.flag_import_ac_source           = gstModbusRegCSIP.CSIP_ac_in_power_source;
    gstCSIP.flag_export_ct_source           = gstModbusRegCSIP.CSIP_export_power_source;
    gstCSIP.flag_export_ac_source           = gstModbusRegCSIP.CSIP_ac_out_power_source;
    // 获取从电网导入功率（Import Power）的爬坡控制类型（如：按时间爬坡或按斜率/速率爬坡）
    gstCSIP.flag_import_power_ramp_up_type  = gstModbusRegCSIP.CSIP_import_power_ramp_up_type;
    // 获取从电网导入功率的具体爬坡设定值（时间或斜率值）
    gstCSIP.num_import_power_ramp_up_value  = gstModbusRegCSIP.CSIP_import_power_ramp_up_value;
    // 拼接32位数据：获取CT并网点允许的最大导入（买电）功率（高16位左移16位后与低16位做或运算）
    gstCSIP.num_import_max_power            = ((uint32_t)gstModbusRegCSIP.CSIP_import_max_power[1] << 16) |
            gstModbusRegCSIP.CSIP_import_max_power[0];
    // 获取向电网导出功率（Export Power）的爬坡控制类型
    gstCSIP.flag_export_power_ramp_up_type  = gstModbusRegCSIP.CSIP_export_power_ramp_up_type;
    // 获取向电网导出功率的具体爬坡设定值
    gstCSIP.num_export_power_ramp_up_value  = gstModbusRegCSIP.CSIP_export_power_ramp_up_value;
    // 拼接32位数据：获取CT并网点允许的最大导出（卖电）功率
    gstCSIP.num_export_max_power            = ((uint32_t)gstModbusRegCSIP.CSIP_export_max_power[1] << 16) |
            gstModbusRegCSIP.CSIP_export_max_power[0];
    // 获取逆变器交流输入侧（AC In）功率的爬坡控制类型
    gstCSIP.flag_ac_in_power_ramp_up_type   = gstModbusRegCSIP.CSIP_ac_in_power_ramp_up_type;
    // 获取交流输入侧功率的具体爬坡设定值
    gstCSIP.num_ac_in_power_ramp_up_value   = gstModbusRegCSIP.CSIP_ac_in_power_ramp_up_value;
    // 拼接32位数据：获取逆变器交流输入侧允许的最大功率
    gstCSIP.num_ac_in_max_power             = ((uint32_t)gstModbusRegCSIP.CSIP_ac_in_max_power[1] << 16) |
            gstModbusRegCSIP.CSIP_ac_in_max_power[0];
    // 获取逆变器交流输出侧（AC Out）功率的爬坡控制类型
    gstCSIP.flag_ac_out_power_ramp_up_type  = gstModbusRegCSIP.CSIP_ac_out_power_ramp_up_type;
    // 获取交流输出侧功率的具体爬坡设定值
    gstCSIP.num_ac_out_power_ramp_up_value  = gstModbusRegCSIP.CSIP_ac_out_power_ramp_up_value;
    // 拼接32位数据：获取逆变器交流输出侧允许的最大功率
    gstCSIP.num_ac_out_max_power            = ((uint32_t)gstModbusRegCSIP.CSIP_ac_out_max_power[1] << 16) |
            gstModbusRegCSIP.CSIP_ac_out_max_power[0];
    gstCSIP.count_csip = 1;
    // 判断CSIP并网认证功能是否被使能
#if 0
    if(gstCSIP.flag_csip_enable == 1)
#else
    if((gstCSIP.flag_csip_enable == 1) && (Engrid == 1) && (gSysSetInfo.ongrid_power != 0) && \
            ((gSysSetInfo.network_power_limit_enable == 0) || ((gSysSetInfo.network_power_limit_enable == 1) && (gSysSetInfo.network_power_limit != 0))))
#endif
    {
        if(gSysInfoGet.parallel_mode_stat == 0)     // 0:非并机状态 1：并机为主机    2：并机为从机
        {
            inverter_max_power = 5000;
        }
        else if((gSysInfoGet.parallel_mode_stat == 1) || (gSysInfoGet.parallel_mode_stat == 2))
        {
            inverter_max_power = 10000;
        }
        else
        {
            inverter_max_power = 5000;
        }
		
		// 如果用户本地未开启入户最大功率限制 (network power limit enable == 0)
        // 使用局部变量避免污染全局配置
        uint32_t effective_network_limit = gSysSetInfo.network_power_limit;
        if(gSysSetInfo.network_power_limit_enable == 0)
        {
            effective_network_limit = 15000;
            // 20260520，如果在进行CSIP认证，且入户功率限制没有使能，使用默认值15kW
        }
		
		// gstCSIP.num_ac_in_max_power = (gstCSIP.num_ac_in_max_power > 15000) ? 15000 : gstCSIP.num_ac_in_max_power;
		// gstCSIP.num_ac_out_max_power = (gstCSIP.num_ac_out_max_power > 15000) ? 15000 : gstCSIP.num_ac_out_max_power;

		gstCSIP.num_ac_in_max_power = MIN_OF_TWO(gstCSIP.num_ac_in_max_power, inverter_max_power);
		gstCSIP.num_ac_out_max_power = MIN_OF_TWO(gstCSIP.num_ac_out_max_power, inverter_max_power);
		
		gstCSIP.num_import_max_power = MIN_OF_TWO(gstCSIP.num_import_max_power, effective_network_limit);
		gstCSIP.num_export_max_power = MIN_OF_TWO(gstCSIP.num_export_max_power, gSysSetInfo.ongrid_power);

        // 检查这一周期的所有限值和爬坡参数是否相较于上一周期发生了变化（即调度中心是否下发了新指令）
        if((gstCSIP.num_import_max_power != gstCSIP.num_import_max_power_last) || \
                (gstCSIP.num_export_max_power != gstCSIP.num_export_max_power_last) || \
                (gstCSIP.num_ac_in_max_power  != gstCSIP.num_ac_in_max_power_last)  || \
                (gstCSIP.num_ac_out_max_power != gstCSIP.num_ac_out_max_power_last) || \
                (gstCSIP.flag_import_power_ramp_up_type != gstCSIP.flag_import_power_ramp_up_type_last) || \
                (gstCSIP.flag_export_power_ramp_up_type != gstCSIP.flag_export_power_ramp_up_type_last) || \
                (gstCSIP.flag_ac_in_power_ramp_up_type  != gstCSIP.flag_ac_in_power_ramp_up_type_last)  || \
                (gstCSIP.flag_ac_out_power_ramp_up_type != gstCSIP.flag_ac_out_power_ramp_up_type_last) || \
                (gstCSIP.num_import_power_ramp_up_value != gstCSIP.num_import_power_ramp_up_value_last) || \
                (gstCSIP.num_export_power_ramp_up_value != gstCSIP.num_export_power_ramp_up_value_last) || \
                (gstCSIP.num_ac_in_power_ramp_up_value  != gstCSIP.num_ac_in_power_ramp_up_value_last)  || \
                (gstCSIP.num_ac_out_power_ramp_up_value != gstCSIP.num_ac_out_power_ramp_up_value_last))
        {
            // 如果任意指令发生变化，记录下当前CT测量的实时有功功率，作为新一轮平滑爬坡的“起始参考点”
            gstCSIP.num_ct_power_hold = _gState_algo.CT_active_power;
            gstCSIP.target_import_power = gstCSIP.num_ct_power_hold;
            gstCSIP.target_export_power = -gstCSIP.num_ct_power_hold;
            gstCSIP.disconnect_grid++;
        }
 

        // 计算映射到并网点的期望交流输入限值：用AC侧限制值减去防逆流保护的本地负载功耗
        gstCSIP.power_ac_in_ct_import = (int32_t)gstCSIP.num_ac_in_max_power - gAntiReFlowValue;     //gAntiReFlowValue远端负载 负数为消耗功率

        // =======================================================================================================================================================
        // 新增修改：提前分别计算CT和AC的输入功率爬坡步长
        gstCSIP.num_import_power_ramp_up_step = 0;
        gstCSIP.num_ac_in_power_ramp_up_step = 0;

        if(gstCSIP.flag_import_power_ramp_up_type == 0 && gstCSIP.num_import_power_ramp_up_value != 0xFFFF)
        {
            gstCSIP.num_import_power_ramp_up_time = gstCSIP.num_import_power_ramp_up_value;
            if(gstCSIP.num_import_power_ramp_up_time == 0)
            {
                gstCSIP.num_import_power_ramp_up_time = 1;  // 避免除零，最小时间为1
            }
            float32 ramp_step_f = fabsf(gstCSIP.num_import_max_power - gstCSIP.num_ct_power_hold) * CSIP_PROCESS_COUNT / (gstCSIP.num_import_power_ramp_up_time * 10) + 0.999f;
            gstCSIP.num_import_power_ramp_up_step = (ramp_step_f > 65535.0f) ? 65535 : (uint16_t)ramp_step_f;
        }
        else if(gstCSIP.flag_import_power_ramp_up_type == 1 && gstCSIP.num_import_power_ramp_up_value != 0xFFFF)
        {
            gstCSIP.num_import_power_ramp_up_rate = gstCSIP.num_import_power_ramp_up_value;
            float32 ramp_step_f = (float32)gstCSIP.num_import_power_ramp_up_rate * CSIP_PROCESS_COUNT * inverter_max_power / 10000.0f / 10.0f + 0.999f;
            gstCSIP.num_import_power_ramp_up_step = (ramp_step_f > 65535.0f) ? 65535 : (uint16_t)ramp_step_f;
        }
        else
        {
            gstCSIP.num_import_power_ramp_up_step = gstCSIP.num_import_max_power;
        }

        if(gstCSIP.flag_ac_in_power_ramp_up_type == 0 && gstCSIP.num_ac_in_power_ramp_up_value != 0xFFFF)
        {
            gstCSIP.num_ac_in_power_ramp_up_time = gstCSIP.num_ac_in_power_ramp_up_value;
            if(gstCSIP.num_ac_in_power_ramp_up_time == 0)
            {
                gstCSIP.num_ac_in_power_ramp_up_time = 1;  // 避免除零，最小时间为1
            }

            float32 ramp_step_f = fabsf(gstCSIP.power_ac_in_ct_import - gstCSIP.num_ct_power_hold) * CSIP_PROCESS_COUNT / (gstCSIP.num_ac_in_power_ramp_up_time * 10) + 0.999f;
            gstCSIP.num_ac_in_power_ramp_up_step = (ramp_step_f > 65535.0f) ? 65535 : (uint16_t)ramp_step_f;
        }
        else if(gstCSIP.flag_ac_in_power_ramp_up_type == 1 && gstCSIP.num_ac_in_power_ramp_up_value != 0xFFFF)
        {
            gstCSIP.num_ac_in_power_ramp_up_rate = gstCSIP.num_ac_in_power_ramp_up_value;
            float32 ramp_step_f = (float32)gstCSIP.num_ac_in_power_ramp_up_rate * CSIP_PROCESS_COUNT * inverter_max_power / 10000.0f / 10.0f + 0.999f;
            gstCSIP.num_ac_in_power_ramp_up_step = (ramp_step_f > 65535.0f) ? 65535 : (uint16_t)ramp_step_f;
        }
        else
        {
            gstCSIP.num_ac_in_power_ramp_up_step = abs(gstCSIP.power_ac_in_ct_import);
        }

        // 导入功率（Import）限制与爬坡计算逻辑
        if((gstCSIP.flag_import_ct_source == 0) && (gstCSIP.flag_import_ac_source == 0))    //都为队列来源
        {
            if((gstCSIP.flag_ac_in_power_ramp_up_type != 0xFFFF) && (gstCSIP.flag_import_power_ramp_up_type == 0xFFFF))     //ac爬坡类型有效
            {
                // 标记位：使用AC口(交流输入端)的爬坡设定
                gstCSIP.flag_import_power_status = 1;
                // 使用AC口ac_in计算的功率变化率
                gstCSIP.num_import_power_step = gstCSIP.num_ac_in_power_ramp_up_step;
                gstCSIP.num_import_power_step_reduce = gstCSIP.num_ac_in_power_ramp_up_step;
            }
            else if((gstCSIP.flag_ac_in_power_ramp_up_type == 0xFFFF) && (gstCSIP.flag_import_power_ramp_up_type != 0xFFFF))    //ct爬坡类型有效
            {
                // 标记位：使用并网口(CT口)的爬坡设定
                gstCSIP.flag_import_power_status = 0;
                // 使用CT口import计算的功率变化率
                gstCSIP.num_import_power_step = gstCSIP.num_import_power_ramp_up_step;
                gstCSIP.num_import_power_step_reduce = gstCSIP.num_import_power_ramp_up_step;
            }
            else if((gstCSIP.flag_ac_in_power_ramp_up_type != 0xFFFF) && (gstCSIP.flag_import_power_ramp_up_type != 0xFFFF))    //爬坡类型都有效
            {
                if(gstCSIP.num_import_power_ramp_up_step >= gstCSIP.num_ac_in_power_ramp_up_step)   //ct斜率大
                {
                    // 标记位：使用并网口(CT口)的爬坡设定
                    gstCSIP.flag_import_power_status = 0;
                    // 使用CT口import计算的功率变化率
                    gstCSIP.num_import_power_step = gstCSIP.num_import_power_ramp_up_step;
                    gstCSIP.num_import_power_step_reduce = gstCSIP.num_import_power_ramp_up_step;

                }
                else    //ac斜率大
                {
                    // 标记位：使用AC口(交流输入端)的爬坡设定
                    gstCSIP.flag_import_power_status = 1;
                    // 使用AC口ac_in计算的功率变化率
                    gstCSIP.num_import_power_step = gstCSIP.num_ac_in_power_ramp_up_step;
                    gstCSIP.num_import_power_step_reduce = gstCSIP.num_ac_in_power_ramp_up_step;

                }
            }
            else    //爬坡类型都无效
            {
                gstCSIP.flag_import_power_status = 2;   // 标志位：2表示不需要爬坡限制，瞬间到达目标值
            }

            //功率限制取小值（ct or ac）
            gstCSIP.final_safe_import_power_limit = MIN_OF_TWO(gstCSIP.num_import_max_power, gstCSIP.power_ac_in_ct_import);
        }
        else if((gstCSIP.flag_import_ct_source == 0) && (gstCSIP.flag_import_ac_source == 1))    //ct队列来源
        {
            // 将绝对安全目标设定为CT口的调度限值
            gstCSIP.final_safe_import_power_limit = gstCSIP.num_import_max_power;

            if(gstCSIP.flag_import_power_ramp_up_type != 0xFFFF)    //ct爬坡类型有效
            {
                // 标记位：使用并网口(CT口)的爬坡设定
                gstCSIP.flag_import_power_status = 0;
                // 使用CT口import计算的功率变化率
                gstCSIP.num_import_power_step = gstCSIP.num_import_power_ramp_up_step;
                gstCSIP.num_import_power_step_reduce = gstCSIP.num_import_power_ramp_up_step;

            }
            else    // ct爬坡类型无效
            {
                // 标志位：2表示不需要爬坡限制，瞬间到达目标值
                gstCSIP.flag_import_power_status = 2;
            }
        }
        else if((gstCSIP.flag_import_ct_source == 1) && (gstCSIP.flag_import_ac_source == 0))    //ac队列来源
        {
            // 将安全目标设置为受AC端口等效限制的值
            gstCSIP.final_safe_import_power_limit = gstCSIP.power_ac_in_ct_import;

            if(gstCSIP.flag_ac_in_power_ramp_up_type != 0xFFFF)    //ac爬坡类型有效
            {
                // 标记位：使用AC口(交流输入端)的爬坡设定
                gstCSIP.flag_import_power_status = 1;
                // 使用AC口ac_in计算的功率变化率
                gstCSIP.num_import_power_step = gstCSIP.num_ac_in_power_ramp_up_step;
                gstCSIP.num_import_power_step_reduce = gstCSIP.num_ac_in_power_ramp_up_step;

            }
            else    // ac爬坡类型无效
            {
                // 标志位：2表示不需要爬坡限制，瞬间到达目标值
                gstCSIP.flag_import_power_status = 2;
            }
        }
        else    //都为默认值
        {
            if((gstCSIP.flag_ac_in_power_ramp_up_type != 0xFFFF) && (gstCSIP.flag_import_power_ramp_up_type == 0xFFFF))     //ac有效
            {
                // 标记位：使用AC口(交流输入端)的爬坡设定
                gstCSIP.flag_import_power_status = 1;
                // 使用AC口ac_in计算的功率变化率
                gstCSIP.num_import_power_step = gstCSIP.num_ac_in_power_ramp_up_step;
                gstCSIP.num_import_power_step_reduce = gstCSIP.num_ac_in_power_ramp_up_step;
            }
            else if((gstCSIP.flag_ac_in_power_ramp_up_type == 0xFFFF) && (gstCSIP.flag_import_power_ramp_up_type != 0xFFFF))    //ct有效
            {
                // 标记位：使用并网口(CT口)的爬坡设定
                gstCSIP.flag_import_power_status = 0;
                // 使用CT口import计算的功率变化率
                gstCSIP.num_import_power_step = gstCSIP.num_import_power_ramp_up_step;
                gstCSIP.num_import_power_step_reduce = gstCSIP.num_import_power_ramp_up_step;
            }
            else if((gstCSIP.flag_ac_in_power_ramp_up_type != 0xFFFF) && (gstCSIP.flag_import_power_ramp_up_type != 0xFFFF))    //都有效
            {
                if(gstCSIP.num_import_power_ramp_up_step >= gstCSIP.num_ac_in_power_ramp_up_step)   //ct斜率大
                {
                    // 标记位：使用并网口(CT口)的爬坡设定
                    gstCSIP.flag_import_power_status = 0;
                    // 使用CT口import计算的功率变化率
                    gstCSIP.num_import_power_step = gstCSIP.num_import_power_ramp_up_step;
                    gstCSIP.num_import_power_step_reduce = gstCSIP.num_import_power_ramp_up_step;

                }
                else    //ac斜率大
                {
                    // 标记位：使用AC口(交流输入端)的爬坡设定
                    gstCSIP.flag_import_power_status = 1;
                    // 使用AC口ac_in计算的功率变化率
                    gstCSIP.num_import_power_step = gstCSIP.num_ac_in_power_ramp_up_step;
                    gstCSIP.num_import_power_step_reduce = gstCSIP.num_ac_in_power_ramp_up_step;

                }
            }
            else    //都无效
            {
                // 标志位：2表示不需要爬坡限制，瞬间到达目标值
                gstCSIP.flag_import_power_status = 2;
            }

            //功率限制取小值（ct or ac）
            gstCSIP.final_safe_import_power_limit = MIN_OF_TWO(gstCSIP.num_import_max_power, gstCSIP.power_ac_in_ct_import);
        }

        //功率限制取小值（final or network）
        gstCSIP.final_safe_import_power_limit = MIN_OF_TWO(gstCSIP.final_safe_import_power_limit, effective_network_limit);

        // 仲裁后的最小步长限制
        if(gstCSIP.num_import_power_step < 0.1f)
        {
            gstCSIP.num_import_power_step *= 100;
            gstCSIP.num_import_power_step_reduce *= 100;
            import_stept_count_max = 100;
        }
        else if(gstCSIP.num_import_power_step < 1)
        {
            gstCSIP.num_import_power_step *= 10;
            gstCSIP.num_import_power_step_reduce *= 10;
            import_stept_count_max = 10;
        }
        else
        {
            import_stept_count_max = 0;
        }

        if(gstCSIP.flag_import_power_status == 2)
        {
            import_stept_count_max = 0;
        }

        if(gstCSIP.final_safe_import_power_limit <= -15000)
        {
            gstCSIP.final_safe_import_power_limit = -15000;
        }

        // 导入功率（Import）具体的递增/递减执行代码：
        // 如果当前的中间目标比安全目标限值小，则本周期在当前基础上加上步长（往上爬坡）
        if(import_stept_count >= import_stept_count_max)
        {
            if(gstCSIP.flag_import_power_status == 2)
            {
                gstCSIP.target_import_power = gstCSIP.final_safe_import_power_limit;
            }
            else if(gstCSIP.target_import_power < gstCSIP.final_safe_import_power_limit)
            {
                gstCSIP.target_import_power = gstCSIP.target_import_power + gstCSIP.num_import_power_step;
                if(gstCSIP.target_import_power >= gstCSIP.final_safe_import_power_limit)
                {
                    gstCSIP.target_import_power = gstCSIP.final_safe_import_power_limit;
                }
            }
            // 如果当前的中间目标比安全目标大，则本周期减去步长（往下收回功率）
            else if(gstCSIP.target_import_power > gstCSIP.final_safe_import_power_limit)
            {
                gstCSIP.target_import_power = gstCSIP.target_import_power - gstCSIP.num_import_power_step_reduce;

                if(gstCSIP.target_import_power <= gstCSIP.final_safe_import_power_limit)
                {
                    gstCSIP.target_import_power = gstCSIP.final_safe_import_power_limit;
                }
            }
            // 如果恰好等于限值，保持限值不变，停止爬坡
            else
            {
                gstCSIP.target_import_power = gstCSIP.final_safe_import_power_limit;
            }
            import_stept_count = 0;
        }
        else
        {
            import_stept_count++;
        }

        // =======================================================================================================================================================
        // 导出功率（Export，卖电）限制与爬坡计算逻辑（逻辑同上，只是计算对象换成了卖电方向）
        // 映射出基于AC侧的最大卖电能力：AC输出限制加上本地负载可以消耗的电量（因为本地负载自用可以允许AC端输出更大总能量）
        gstCSIP.power_ac_out_ct_export = (int32_t)gstCSIP.num_ac_out_max_power + gAntiReFlowValue;

        // 新增修改：提前分别计算CT和AC的输出功率爬坡步长
        gstCSIP.num_export_power_ramp_up_step = 0;
        gstCSIP.num_ac_out_power_ramp_up_step = 0;

        if(gstCSIP.flag_export_power_ramp_up_type == 0 && gstCSIP.num_export_power_ramp_up_value != 0xFFFF)
        {
            gstCSIP.num_export_power_ramp_up_time = gstCSIP.num_export_power_ramp_up_value;
            if(gstCSIP.num_export_power_ramp_up_time == 0)
            {
                gstCSIP.num_export_power_ramp_up_time = 1;  // 避免除零，最小时间为1
            }
            float32 ramp_step_f = fabsf(gstCSIP.num_export_max_power + gstCSIP.num_ct_power_hold) * CSIP_PROCESS_COUNT / (gstCSIP.num_export_power_ramp_up_time * 10) + 0.999f;
            gstCSIP.num_export_power_ramp_up_step = (ramp_step_f > 65535.0f) ? 65535 : (uint16_t)ramp_step_f;
        }
        else if(gstCSIP.flag_export_power_ramp_up_type == 1 && gstCSIP.num_export_power_ramp_up_value != 0xFFFF)
        {
            gstCSIP.num_export_power_ramp_up_rate = gstCSIP.num_export_power_ramp_up_value;
            float32 ramp_step_f = (float32)gstCSIP.num_export_power_ramp_up_rate * CSIP_PROCESS_COUNT * inverter_max_power / 10000.0f / 10.0f + 0.999f;
            gstCSIP.num_export_power_ramp_up_step = (ramp_step_f > 65535.0f) ? 65535 : (uint16_t)ramp_step_f;
        }
        else
        {
            gstCSIP.num_export_power_ramp_up_step = gstCSIP.num_export_max_power;
        }

        if(gstCSIP.flag_ac_out_power_ramp_up_type == 0 && gstCSIP.num_ac_out_power_ramp_up_value != 0xFFFF)
        {
            gstCSIP.num_ac_out_power_ramp_up_time = gstCSIP.num_ac_out_power_ramp_up_value;
            if(gstCSIP.num_ac_out_power_ramp_up_time == 0)
            {
                gstCSIP.num_ac_out_power_ramp_up_time = 1;  // 避免除零，最小时间为1
            }
            float32 ramp_step_f = fabsf(gstCSIP.power_ac_out_ct_export + gstCSIP.num_ct_power_hold) * CSIP_PROCESS_COUNT / (gstCSIP.num_ac_out_power_ramp_up_time * 10) + 0.999f;
            gstCSIP.num_ac_out_power_ramp_up_step = (ramp_step_f > 65535.0f) ? 65535 : (uint16_t)ramp_step_f;
        }
        else if(gstCSIP.flag_ac_out_power_ramp_up_type == 1 && gstCSIP.num_ac_out_power_ramp_up_value != 0xFFFF)
        {
            gstCSIP.num_ac_out_power_ramp_up_rate = gstCSIP.num_ac_out_power_ramp_up_value;
            float32 ramp_step_f = (float32)gstCSIP.num_ac_out_power_ramp_up_rate * CSIP_PROCESS_COUNT * inverter_max_power / 10000.0f / 10.0f + 0.999f;
            gstCSIP.num_ac_out_power_ramp_up_step = (ramp_step_f > 65535.0f) ? 65535 : (uint16_t)ramp_step_f;
        }
        else
        {
            gstCSIP.num_ac_out_power_ramp_up_step = abs(gstCSIP.power_ac_out_ct_export);
        }

        // 导出功率（Export）限制与爬坡计算逻辑
        if((gstCSIP.flag_export_ct_source == 0) && (gstCSIP.flag_export_ac_source == 0))    //都为队列来源
        {
            if((gstCSIP.flag_ac_out_power_ramp_up_type != 0xFFFF) && (gstCSIP.flag_export_power_ramp_up_type == 0xFFFF))     //ac有效
            {
                // 标记位：使用AC口(交流输出端)的爬坡设定
                gstCSIP.flag_export_power_status = 1;
                // 使用AC口ac_out计算的功率变化率
                gstCSIP.num_export_power_step = gstCSIP.num_ac_out_power_ramp_up_step;
                gstCSIP.num_export_power_step_reduce = gstCSIP.num_ac_out_power_ramp_up_step;
            }
            else if((gstCSIP.flag_ac_out_power_ramp_up_type == 0xFFFF) && (gstCSIP.flag_export_power_ramp_up_type != 0xFFFF))    //ct有效
            {
                // 标记位：使用并网口(CT口)的爬坡设定
                gstCSIP.flag_export_power_status = 0;
                // 使用CT口export计算的功率变化率
                gstCSIP.num_export_power_step = gstCSIP.num_export_power_ramp_up_step;
                gstCSIP.num_export_power_step_reduce = gstCSIP.num_export_power_ramp_up_step;
            }
            else if((gstCSIP.flag_ac_out_power_ramp_up_type != 0xFFFF) && (gstCSIP.flag_export_power_ramp_up_type != 0xFFFF))    //都有效
            {
                if(gstCSIP.num_export_power_ramp_up_step >= gstCSIP.num_ac_out_power_ramp_up_step)   //ct斜率大
                {
                    // 标记位：使用并网口(CT口)的爬坡设定
                    gstCSIP.flag_export_power_status = 0;
                    // 使用CT口export计算的功率变化率
                    gstCSIP.num_export_power_step = gstCSIP.num_export_power_ramp_up_step;
                    gstCSIP.num_export_power_step_reduce = gstCSIP.num_export_power_ramp_up_step;

                }
                else    //ac斜率大
                {
                    // 标记位：使用AC口(交流输出端)的爬坡设定
                    gstCSIP.flag_export_power_status = 1;
                    // 使用AC口ac_out计算的功率变化率
                    gstCSIP.num_export_power_step = gstCSIP.num_ac_out_power_ramp_up_step;
                    gstCSIP.num_export_power_step_reduce = gstCSIP.num_ac_out_power_ramp_up_step;

                }
            }
            else    //都无效
            {
                // 标志位：2表示不需要爬坡限制，瞬间到达目标值
                gstCSIP.flag_export_power_status = 2;
            }

            //功率限制取小值（ct or ac）
            gstCSIP.final_safe_export_power_limit = MIN_OF_TWO(gstCSIP.num_export_max_power, gstCSIP.power_ac_out_ct_export);
        }
        else if((gstCSIP.flag_export_ct_source == 0) && (gstCSIP.flag_export_ac_source == 1))    //ct队列来源
        {
            // 将绝对安全目标设定为CT口的调度限值
            gstCSIP.final_safe_export_power_limit = gstCSIP.num_export_max_power;

            if(gstCSIP.flag_export_power_ramp_up_type != 0xFFFF)    //ct有效
            {
                // 标记位：使用并网口(CT口)的爬坡设定
                gstCSIP.flag_export_power_status = 0;
                // 使用CT口export计算的功率变化率
                gstCSIP.num_export_power_step = gstCSIP.num_export_power_ramp_up_step;
                gstCSIP.num_export_power_step_reduce = gstCSIP.num_export_power_ramp_up_step;

            }
            else    // ct无效
            {
                // 标志位：2表示不需要爬坡限制，瞬间到达目标值
                gstCSIP.flag_export_power_status = 2;
            }
        }
        else if((gstCSIP.flag_export_ct_source == 1) && (gstCSIP.flag_export_ac_source == 0))    //ac队列来源
        {
            // 将安全目标设置为受AC端口等效限制的值
            gstCSIP.final_safe_export_power_limit = gstCSIP.power_ac_out_ct_export;

            if(gstCSIP.flag_ac_out_power_ramp_up_type != 0xFFFF)    //ac有效
            {
                // 标记位：使用AC口(交流输出端)的爬坡设定
                gstCSIP.flag_export_power_status = 1;
                // 使用AC口ac_out计算的功率变化率
                gstCSIP.num_export_power_step = gstCSIP.num_ac_out_power_ramp_up_step;
                gstCSIP.num_export_power_step_reduce = gstCSIP.num_ac_out_power_ramp_up_step;

            }
            else    // ac无效
            {
                // 标志位：2表示不需要爬坡限制，瞬间到达目标值
                gstCSIP.flag_export_power_status = 2;
            }
        }
        else    //都为默认值
        {
            if((gstCSIP.flag_ac_out_power_ramp_up_type != 0xFFFF) && (gstCSIP.flag_export_power_ramp_up_type == 0xFFFF))     //ac有效
            {
                // 标记位：使用AC口(交流输出端)的爬坡设定
                gstCSIP.flag_export_power_status = 1;
                // 使用AC口ac_out计算的功率变化率
                gstCSIP.num_export_power_step = gstCSIP.num_ac_out_power_ramp_up_step;
                gstCSIP.num_export_power_step_reduce = gstCSIP.num_ac_out_power_ramp_up_step;
            }
            else if((gstCSIP.flag_ac_out_power_ramp_up_type == 0xFFFF) && (gstCSIP.flag_export_power_ramp_up_type != 0xFFFF))    //ct有效
            {
                // 标记位：使用并网口(CT口)的爬坡设定
                gstCSIP.flag_export_power_status = 0;
                // 使用CT口export计算的功率变化率
                gstCSIP.num_export_power_step = gstCSIP.num_export_power_ramp_up_step;
                gstCSIP.num_export_power_step_reduce = gstCSIP.num_export_power_ramp_up_step;
            }
            else if((gstCSIP.flag_ac_out_power_ramp_up_type != 0xFFFF) && (gstCSIP.flag_export_power_ramp_up_type != 0xFFFF))    //都有效
            {
                if(gstCSIP.num_export_power_ramp_up_step >= gstCSIP.num_ac_out_power_ramp_up_step)   //ct斜率大
                {
                    // 标记位：使用并网口(CT口)的爬坡设定
                    gstCSIP.flag_export_power_status = 0;
                    // 使用CT口export计算的功率变化率
                    gstCSIP.num_export_power_step = gstCSIP.num_export_power_ramp_up_step;
                    gstCSIP.num_export_power_step_reduce = gstCSIP.num_export_power_ramp_up_step;

                }
                else    //ac斜率大
                {
                    // 标记位：使用AC口(交流输出端)的爬坡设定
                    gstCSIP.flag_export_power_status = 1;
                    // 使用AC口ac_out计算的功率变化率
                    gstCSIP.num_export_power_step = gstCSIP.num_ac_out_power_ramp_up_step;
                    gstCSIP.num_export_power_step_reduce = gstCSIP.num_ac_out_power_ramp_up_step;

                }
            }
            else    //都无效
            {
                // 标志位：2表示不需要爬坡限制，瞬间到达目标值
                gstCSIP.flag_export_power_status = 2;
            }

            //功率限制取小值（ct or ac）
            gstCSIP.final_safe_export_power_limit = MIN_OF_TWO(gstCSIP.num_export_max_power, gstCSIP.power_ac_out_ct_export);
        }

        //功率限制取小值（final or ongrid
        gstCSIP.final_safe_export_power_limit = MIN_OF_TWO(gstCSIP.final_safe_export_power_limit, gSysSetInfo.ongrid_power);

        // 仲裁后的最小步长限制
        if(gstCSIP.num_export_power_step < 0.1f)
        {
            gstCSIP.num_export_power_step *= 100;
            gstCSIP.num_export_power_step_reduce *= 100;
            export_stept_count_max = 100;
        }
        else if(gstCSIP.num_export_power_step < 1)
        {
            gstCSIP.num_export_power_step *= 10;
            gstCSIP.num_export_power_step_reduce *= 10;
            export_stept_count_max = 10;
        }
        else
        {
            export_stept_count_max = 0;
        }

        if(gstCSIP.flag_export_power_status == 2)
        {
            export_stept_count_max = 0;
        }

        if(gstCSIP.final_safe_export_power_limit <= -15000)
        {
            gstCSIP.final_safe_export_power_limit = -15000;     //最大进15kw
        }

        // 导出功率（Export）具体的递增/递减执行代码：
        // 如果当前的中间目标比安全目标限值小，则本周期在当前基础上加上步长（往上爬坡）
        if(export_stept_count >= export_stept_count_max)
        {
            if(gstCSIP.flag_export_power_status == 2)
            {
                gstCSIP.target_export_power = gstCSIP.final_safe_export_power_limit;
            }
            else if(gstCSIP.target_export_power < gstCSIP.final_safe_export_power_limit)
            {
                gstCSIP.target_export_power = gstCSIP.target_export_power + gstCSIP.num_export_power_step;
                if(gstCSIP.target_export_power >= gstCSIP.final_safe_export_power_limit)
                {
                    gstCSIP.target_export_power = gstCSIP.final_safe_export_power_limit;
                }
            }
            // 如果当前的中间目标比安全目标大，则本周期减去步长（往下收回功率）
            else if(gstCSIP.target_export_power > gstCSIP.final_safe_export_power_limit)
            {
                gstCSIP.target_export_power = gstCSIP.target_export_power - gstCSIP.num_export_power_step_reduce;

                if(gstCSIP.target_export_power <= gstCSIP.final_safe_export_power_limit)
                {
                    gstCSIP.target_export_power = gstCSIP.final_safe_export_power_limit;
                }
            }
            // 如果恰好等于限值，保持限值不变，停止爬坡
            else
            {
                gstCSIP.target_export_power = gstCSIP.final_safe_export_power_limit;
            }
            export_stept_count = 0;
        }
        else
        {
            export_stept_count++;
        }

        // =======================
        // 缓存更新模块
        // 将本周期处理过的当前各项限值缓存到 `_last` 结尾的历史变量中。这样下一个100ms进来时即可比对
        // 判定调度主站是否又下发了新的Modbus命令，以便随时重新计算步长并捕获起始CT功率 (num_ct_power_hold)
        gstCSIP.num_import_max_power_last           = gstCSIP.num_import_max_power;
        gstCSIP.num_export_max_power_last           = gstCSIP.num_export_max_power;
        gstCSIP.num_ac_in_max_power_last            = gstCSIP.num_ac_in_max_power;
        gstCSIP.num_ac_out_max_power_last           = gstCSIP.num_ac_out_max_power;
        gstCSIP.num_import_power_ramp_up_value_last = gstCSIP.num_import_power_ramp_up_value;
        gstCSIP.num_export_power_ramp_up_value_last = gstCSIP.num_export_power_ramp_up_value;
        gstCSIP.num_ac_in_power_ramp_up_value_last  = gstCSIP.num_ac_in_power_ramp_up_value;
        gstCSIP.num_ac_out_power_ramp_up_value_last = gstCSIP.num_ac_out_power_ramp_up_value;
        gstCSIP.flag_import_power_ramp_up_type_last = gstCSIP.flag_import_power_ramp_up_type;
        gstCSIP.flag_export_power_ramp_up_type_last = gstCSIP.flag_export_power_ramp_up_type;
        gstCSIP.flag_ac_in_power_ramp_up_type_last  = gstCSIP.flag_ac_in_power_ramp_up_type;
        gstCSIP.flag_ac_out_power_ramp_up_type_last = gstCSIP.flag_ac_out_power_ramp_up_type;

        // 最终输出对接：
        // 将经过了严密判断、限幅和平滑处理（爬坡）后的内部目标，供逆变器底层直接调用
        // gFeedPower 控制对电网的最终允许注入（卖出）设定
        gstCsipPowerCtrl.export_power = gstCSIP.target_export_power;
        // gNetworkPower 控制从电网最终允许抽取（买入）设定
        gstCsipPowerCtrl.import_power = gstCSIP.target_import_power;
        gstCSIP.count_csip = 2;
    }
    // 如果CSIP功能在Modbus端未被激活 (CSIP_enable == 0)，绕过复杂的爬坡机制
    else
    {
        gstCSIP.num_import_max_power_last           = 0;
        gstCSIP.num_export_max_power_last           = 0;
        gstCSIP.num_ac_in_max_power_last            = 0;
        gstCSIP.num_ac_out_max_power_last           = 0;
        gstCSIP.num_import_power_ramp_up_value_last = 0;
        gstCSIP.num_export_power_ramp_up_value_last = 0;
        gstCSIP.num_ac_in_power_ramp_up_value_last  = 0;
        gstCSIP.num_ac_out_power_ramp_up_value_last = 0;
        gstCSIP.flag_import_power_ramp_up_type_last = 0;
        gstCSIP.flag_export_power_ramp_up_type_last = 0;
        gstCSIP.flag_ac_in_power_ramp_up_type_last  = 0;
        gstCSIP.flag_ac_out_power_ramp_up_type_last = 0;

//        memset(&gstCSIP, 0, sizeof(CSIP_t));
        // 暴力平替：直接将用户配置里面的静态限额下发给底层全局变量使用
        gstCsipPowerCtrl.export_power = gSysSetInfo.ongrid_power;
        gstCsipPowerCtrl.import_power = gSysSetInfo.network_power_limit;
    }

}

void CsipInit(void)
{
    memset(&gstCSIP, 0, sizeof(CSIP_t));
    memset(&gstModbusPowerCtrl, 0, sizeof(GridCtrl_t));
    memset(&gstCsipPowerCtrl, 0, sizeof(GridCtrl_t));
}

bool GetInvStandbyState(void)
{
    if((gEnLlcPwm == 0) && (gEnLlcPwm == 0))
    {
        return true;
    }
    
//    if((gSysSetInfo.remote_standby_cmd == 1) || (gSyncHostStandby == EN_INV_STANDBY) || (gArcFaultStandbyFlag == 1))
//    {
//        return true;
//    }

    return false;
}
