/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        algo_interface.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-3-19
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-3-19    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __MPPT_H__
#define __MPPT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <F28x_Project.h>
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_interface.h"


#define PV_LOW_VOL_TEST
// 系统参数定义
#define PV_CHANNELS          3

#ifdef PV_LOW_VOL_TEST
//#define PV_CHANNEL_MIN_VOLTAGE       200.0f
//#define PV_CHANNEL_MAX_VOLTAGE       950.0f
//#define PV_CHANNEL_MAX_CURRENT       16.0f
//#define PV_CHANNEL_MAX_DUTY          0.85f
//#define PV_BUS_MAX_VOLTAGE           950.0f
#define PV_BUS_START_UP_VOLTAGE      650.0f
#define PV_BUS_NOMAL_LIMIT_VOLTAGE   880.0f
//#define PV_CHANNEL_MAX_POWER         8000.0f
//#define PV_TOTAL_MAX_POWER           24000.0f
#else
#define PV_CHANNEL_MIN_VOLTAGE       200.0f
#define PV_CHANNEL_MAX_VOLTAGE       1000.0f
#define PV_CHANNEL_MAX_CURRENT       16.0f
#define PV_CHANNEL_MAX_DUTY          0.85f
#define PV_BUS_MAX_VOLTAGE           950.0f
#define PV_BUS_START_UP_VOLTAGE      700.0f
#define PV_BUS_NOMAL_LIMIT_VOLTAGE   950.0f
#define PV_CHANNEL_MAX_POWER         8000.0f
#define PV_TOTAL_MAX_POWER           24000.0f
#endif

#define PV_MPPT_NOMAL_STEP           1.0f
#define PV_MPPT_MAX_STEP             10.0f
#define PV_MPPT_MIN_STEP             0.5f

#define PV_ADD_COUNT      ((uint32_t)((uint32_t)100 * (uint32_t)20))    // 1ms = 20，5对应5ms


typedef struct {
    // 控制器参数
    float Kp;           // 比例增益
    float Ki;           // 积分增益
    float Ts;           // 采样时间（秒）
    // 控制器状态
    float prev_error;   // 上一次误差（用于增量式）
    float propotion_test;

    float integral;     // 积分项累计值
    // 限制参数
    float output_limit_upper;   // 输出上限
    float output_limit_lower;   // 输出下限

    float out;                  // 输出值
} PIParam_t;


typedef struct
{
    //占空比变量
    float actual_duty;

    //MPPT算法变量
    float mppt_vref;           // 参考电压
    float step_size;       // 当前步长
    float step_min;        // 最小步长

    float power_prev;           // 前一次功率
    float v_prev;               // 前一次电压

    bool  mppt_fast_flag;     //快速追踪标志

    float  mppt_vmp;
    float  mppt_maxpwr;
    float  power_prev_delta;
    float  mppt_osci;

    int16_t    s16OsiCnt;
    uint16_t   mppt_state;
    uint16_t   mppt_fast_cnt;
    uint16_t   mppt_slow_cnt;
    float      pwrloop_vref;      //变步长因子

   // uint8_t direction_prev;     // 前一次扰动方向 (1: 增加, -1: 减少)

    //PV,输入滤波变量
    float pv_v_instant;
    float pv_i_instant;
    float pv_v_filter;
    float pv_i_filter;
    float pv_p_filter;
    float pv_i_filter_ctrl;
    float pv_v_filter_ctrl;
    //环路计算参数
    float pv_curr_loop_inref; //PV电流参考值
    float pv_vol_loop_inref;

    float single_limit_curr_ref;
} Channel_Value_t;

typedef struct
{
    float pv_v_max;
    float pv_v_min;
    float pv_i_max;
    float pv_p_max;
    float pv_p_set;
}channelLimit_t;

// MPPT通道结构体
typedef struct
{

    Channel_Value_t            value;
    channelLimit_t             limit_value;
    PIParam_t                  PvVolLoop;
    PIParam_t                  PvCurrLoop;

    uint16_t                   cont;
    uint16_t                   run_judge_cont;
    bool                       first_run_cl_flag;
    bool                       pv_run_flag;


} Channel_t;

typedef struct
{
    float bus_v_instant;
    float bus_v_filter_ctrl;
    float total_p;

    float total_p_set;
    //环路计算值
    float bus_loop_inref;
    float power_loop_limit_step;
    float pv_volt_ref;
    float total_p_ctrl;
    float dutytest;
} PvValue_t;

typedef struct
{
    float    total_power_max;
    float    bus_v_max;
    float    pv_active_channel;

    float    bus_startup_vol_ref_amplitude;   //母线启机参考幅值
    float    bus_nomalrun_vol_ref_amplitude;  //BUS正常运行参考幅值
} pvValueLimit_t;

typedef struct {
    Channel_t       channel[PV_CHANNELS];
    PIParam_t       BusVolLoop;
    PIParam_t       PvPowerLoop;
    // 系统参数
    PvValue_t       pv_value;
    pvValueLimit_t  pv_value_limit;

    //EN_PV_STATE     en_pv_state;
    bool            en_pv;
    uint16_t            en_power_limit;
} PVController_t;
/*外部交互接口信息*/
typedef struct
{
    float pv_vol[PV_CHANNELS];
    float pv_curr[PV_CHANNELS];
    float bus_vol;
} PvInInterface_t;

typedef struct
{
    float pv_duty[PV_CHANNELS];
} PvOutInterface_t;

typedef struct
{
    PvInInterface_t  pv_in_value;
    PvOutInterface_t pv_out_value;
} PVInterfaceParam_t;

typedef struct
{
    float total_power_max;
    float bus_max_vol;

    float pv_max_curr;
    float channel_max_power;
    float pv_min_vol;
    float pv_max_vol;
    float pv_active_channel;
} PvLimitInterface_t;



extern void PvFliterDataUpdate(void);
extern PVController_t  _gPvData;
/**********************************逆变部分接口****************************************/

void Inv3PhWorkRun(void);
void Inv3PhSetEnRun(uint8_t flag);
void Inv3PhSetEnConnect(uint8_t flag);
void Inv3PhSetEnConnectRun(uint8_t flag);
void Inv3PhDataUpdate(void);
void Inv3PhCpuTransforClaData(void);
uint8_t GetInvRelayFlag(void);

uint8_t Get3PhIsRunFlag(void);
uint8_t Get3PhIsConnectFlag(void);
uint8_t Get3PhIsErrFlag(void);
EN_INV3PH_STATE Get3PhRunState(void);
Inv3Ph_Err_Un_t Get3PhErrType(void);
Inv3Ph_Warn_Un_t Get3PhWarnType(void);
/***************************************************************************/
float PvCurAveRead(uint16_t channal_number);
float PvVolAveRead(uint16_t channal_number);
float PvPwrAveRead(uint16_t channal_number);
uint16_t Read_Mppt_state(uint16_t num) ;
uint16_t PvVolLowWarningCodeRead(uint16_t channal_number);
uint16_t PvLossCtrlWarningCodeRead(uint16_t channal_number);
uint16_t PvWarningCodeRead(uint16_t channal_number);
uint16_t PvErroCodeRead(uint16_t channal_number);
uint16_t PvReverseErrCodeRead(uint16_t channal_number);


void AlgoInterfaceInit(void);
void PVRunSet(bool run_set);
//void PvRunStateSet(EN_PV_STATE state);
float PvPwrRead(void);
//外界输入信息限制(方便后续不同功率段PV移植)
void MpptSetPvPwr(float pwr);
void PvMaxPowerSet(float pwr);

#endif

//===========================================================================
// End of file.
//===========================================================================
