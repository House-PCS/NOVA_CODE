#ifndef INVERTER_LOOP_CNTL_H
#define INVERTER_LOOP_CNTL_H
#include "F2837xD_device.h"
#include "inverter_state_def.h"
#include <algo_err_check.h>

#define INV_PWR_LIMIT_CUR 25


#ifdef LOW_VOL_TEST
#define BUS_VOL_SET 60
#define INV_VOL_SET 30
#else
#ifdef MID_VOL_TEST
#define BUS_VOL_SET 200
#define INV_VOL_SET 100
#else
#define BUS_VOL_SET 525
#define INV_VOL_SET 325
#endif
#endif

#define OPEN_LOOP_RADIO ((float)INV_VOL_SET/(float)BUS_VOL_SET)
#define BUS_VOLTAGE_NORMAL 450.0 //母线电压典型值

typedef enum
{
    EN_GRID_CONNECT_STA_UNCONNECT = 0x00,
    EN_GRID_CONNECT_STA_CONNECTING = 0x01,
    EN_GRID_CONNECT_STA_CONNECTED = 0x02,
} EN_GRID_CONNECT_STA;

/**
 * @brief 逆变器状态结构体
 */
typedef struct inverter_state_stru_t
{
    float32 sign_sin;           //逆变器内部Sin信号
    float32 sign_cos;           //逆变器内部Cos信号
    float32 sign_stamp;         //逆变器内部斜坡信号
    float32 sta_gv_ref;         //电压环给定值
    float32 sta_gv_out;         //电压环输出
    float32 sta_gi_ref;         //电流环给定值
    float32 sta_gi_ref_rms;     //电流环给定的有效值
    float32 sta_gi_out;         //电流环输出
    float32 sta_grid_phase_err; //电网相位偏差
    float32 sta_grid_connect;   //并网状态
//    EN_GRID_CONNECT_STA sta_grid_connect;   //并网状态
    float32 sta_grid_vol_rms;   //电网电压有效值
    float32 sta_inv_vol_rms;    //逆变电压有效值
    float32 sta_load_vol_rms;    //逆变电压有效值
    float32 sta_grid_cur_rms;
    float32 sta_inv_cur_rms;
    float32 sta_load_cur_rms;
    float32 sta_grid_vol_phase;
    float32 sta_grid_cur_phase;
    float32 sta_grid_ct_phase;
    float32 sta_inv_vol_phase;
    float32 sta_inv_cur_phase;
    float32 sta_grid_vol_phase_cos;
    float32 sta_grid_cur_phase_cos;
    float32 sta_inv_vol_phase_cos;
    float32 sta_inv_cur_phase_cos;
    float32 sta_inv_active_pwr;
    float32 sta_inv_reactive_pwr;
    float32 sta_inv_freq;
    float32 sta_grid_connected_cur_ref; 
    float32 sta_pwm;
} InverterState_t;

void* InverterGetStateWave();
void InverterLoopCntl(void);
void InverterLoopInit();
void InverterSendStateWave();
void InverterSetGridConnect(uint8_t state);
InverterState_t* InverterGetState();
EN_INV_STATE_DEF GetInvRunState(void);
void InverterSetGridConnectedCur(float cur);
float InverterGetGridConnectedCur();
EN_INV_STATE_DEF InvGetState();
void InverterSetDeg(float d);
float InverterGetDeg();
extern uint8_t gInvErr;
extern uint8_t gInvStartOk, gInvReleaseOk;
#endif
