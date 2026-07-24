#ifndef ALGO_INFO_H
#define ALGO_INFO_H
#include "F2837xD_device.h"
#include "inverter_state_def.h"

typedef enum
{
    EN_RELAY_DEF_MAIN,
    EN_RELAY_DEF_LOAD,
    EN_RELAY_DEF_GRID,
    EN_RELAY_DEF_ENGINE,
    EN_RELAY_DEF_GRID_RECTIFY,
    EN_RELAY_DEF_ENGINE_RECTIFY,
} EN_RELAY_DEF;

typedef struct algo_info_sensor_stru_t
{
    float32   inv_vol;
    float32   inv_cur;
    float32   grid_vol;
    float32   grid_cur;
    float32   load_vol;
    float32   load_cur;
    float32   bus_vol;
    float32   bat_vol;
    float32   bat_cur;
    float32   pv1_vol;
    float32   pv1_cur;
    float32   pv2_vol;
    float32   pv2_cur;
    float32   CT_cur;
} AlgoInfoSensor_t;


typedef struct algo_info_relay_stru_t
{
    uint16_t rly_main;
    uint16_t rly_load;
    uint16_t rly_grid;
    uint16_t rly_engine;
    uint16_t rly_grid_rectify;
    uint16_t rly_engine_rectify;
} AlgoInfoRelay_t;

typedef struct algo_state_stru_t
{
    AlgoInfoSensor_t   sensor;
    AlgoInfoRelay_t relay;
    float32 sta_inv_pwm;         //电流环输出
    float32 grid_phase_err; //电网相位偏差
    float32 grid_deg_err; //电网相位偏差
    uint16_t grid_connect;   //并网状态
    float32 bat_cur_ave;
    float32 grid_cur_rms;
    float32 grid_cur_ave;
    float32 grid_cur_rms_ave;
    float32 grid_vol_rms;   //电网电压有效值
    float32 grid_vol_rms_filter;   //电网电压有效值
    float32 inv_cur_rms;
    float32 inv_vol_rms;    //逆变电压有效值
    float32 load_cur_rms;
    float32 load_vol_rms;    //逆变电压有效值
    float32 load_cur_phase;    //逆变电压有效值
    float32 ct_cur_rms;  //远端负载电流有效值
    float32 ct_cur_rms_ave;
    float32 grid_vol_phase;
    float32 grid_cur_phase;
    float32 CT_cur_phase;  //远端负载电流相位
    float32 inv_vol_phase;
    float32 inv_cur_phase;
    float32 grid_vol_phase_cos;
    float32 grid_cur_phase_cos;
    float32 CT_cur_phase_cos;  //远端负载电流相位
    float32 inv_vol_phase_cos;
    float32 inv_vol_theta;
    float32 inv_cur_phase_cos;
    uint16_t inv_start_ok;
    uint16_t inv_release_ok;
    uint16_t llc_start_ok;
    uint16_t inv_enable;
    uint16_t llc_enable;
    uint16_t sys_enable;
    uint16_t discharge_enable;
    uint16_t charge_enable;
    float32 grid_pwr;
    float32 load_pwr;
    float32 inv_pwr;
    float32 bat_pwr;
    float32 CT_pwr;
    float32 norload_pwr;
    float32 pv1_pwr;
    float32 pv2_pwr;
    float32 grid_freq;
    float32 grid_omiga;
    float32 inv_freq;
    float32 pv1_vol_ave;
    float32 pv2_vol_ave;
    float32 pv1_cur_ave;
    float32 pv2_cur_ave;
    float32 pv_set_pwr;
    uint16_t pv_state;
    float32 power_deg_err;
    float32 grid_power_factor;
    float32 grid_power_refactor;
    float32 grid_active_power;
    float32 grid_reactive_power;
    float32 CT_power_factor;
    float32 CT_power_refactor;
    float32 CT_active_power;
    float32 CT_reactive_power;
    float32 norload_active_power;
    float32 norload_reactive_power;
    float32 inv_power_factor;
    float32 inv_power_refactor;
    float32 inv_active_power;
    float32 inv_reactive_power;
    float32 load_power_factor;
    float32 load_power_refactor;
    float32 load_active_power;
    float32 load_reactive_power;
    EN_INV_STATE_DEF inv_state;
    float32 bus_vol_last;
    float32 bus_vol_dt;
    float32 bus_vol_filter;
    float32 current_extra;
} AlgoInfo_t;

typedef struct algo_msg_stru_t
{
    float32 grid_vol;
    float32 inv_vol;
    float32 load_vol;
    float32 grid_cur;
    float32 inv_cur;
    float32 load_cur;
    float32 ct_cur;
} AlgoMsg_t;

extern AlgoMsg_t gAlgoMsg;
extern AlgoInfo_t _gState_algo;
const AlgoInfo_t* AlgoStateGet();
void AlgoStateInit();
void AlgoStateUpdate();
void RelaySetValue(EN_RELAY_DEF relay, uint16_t value);
void AlgoPwrInfoCalc();
void AlgoEnDischarge(uint16_t state);
void AlgoEnCharge(uint16_t state);
uint16_t GetGridRlyState(void);
uint16_t GetEngineRlyState(void);
uint16_t GetMainRlyState(void);
uint16_t GetLoadRlyState(void);

#endif
