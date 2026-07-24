/*
 * cla_cpu_share.h
 *
 *  Created on: 2025年3月26日
 *      Author: Nova006352
 */

#ifndef APP_CLA_CPU_SHARE_H_
#define APP_CLA_CPU_SHARE_H_

#include "F28x_Project.h"

#include "cap.h"

// 枚举定义
typedef enum
{
    INV_STATE_STANDBY,         //逆变待机状态
    INV_STATE_SELFTEST,        //逆变自检状态
    INV_STATE_OPENING,         //逆变开环状态
    INV_STATE_SOFTSTARTING,    //逆变软起状态
    INV_STATE_DISCONNECTED,    //逆变闭环离网状态
    INV_STATE_GRID_CONNECTING,      //逆变并网中状态
    INV_STATE_GRID_CONNECTED,       //逆变已并网状态
    INV_STATE_GRID_DISCONNECTING,   //逆变离网中状态
    INV_STATE_GEN_CONNECTING,      //油机并网中状态
    INV_STATE_GEN_CONNECTED,       //油机已并网状态
    INV_STATE_GEN_DISCONNECTING,   //油机离网中状态
} EN_INV_STATE;

typedef struct cla_pll_stru_t
{
    float32 input01;            //输入迭代值
    float32 cycle_time;         //周期时间
    float32 temp_cnt;           //计数器
    float32 freq;               //频率
    float32 wn;                 //陷波频率
    float32 w0;                 //目标频率
    float32 k1;                 //陷波器系数
    float32 k2;                 //陷波器系数
    float32 a1;
    float32 a2;
    float32 a3;
    float32 b0;
    float32 b1;
    float32 b2;
    float32 b3;
    float32 upd0;               //陷波器输入迭代
    float32 upd1;               //陷波器输入迭代
    float32 upd2;               //陷波器输入迭代
    float32 y_notch0;           //陷波器迭代系数
    float32 y_notch1;           //陷波器迭代系数
    float32 y_notch2;           //陷波器迭代系数
    float32 kp;                 //PI控制参数
    float32 ki;                 //PI控制参数
    float32 lpf0;               //PI输出迭代
    float32 lpf1;               //PI输出迭代
    float32 wo;                 //VCO部分临时变量
    float32 sin0;               //VCO部分临时变量
    float32 sin1;               //VCO部分临时变量
    float32 cos1;               //VCO部分临时变量
    float32 cos0;               //VCO部分临时变量
    float32 theta;              //输出角度
    float32 theta1;
    float32 output;             //输出sin值
} ClaPll_t;
typedef struct cla_value_stru_t
{
    float32 instant;             //瞬时值
    float32 average;             //平均值（与 RmsPwr vol_avg 同步）
    float32 rms;                 //有效值
} ClaValue_t;
typedef struct cla_phase_stru_t
{
    ClaValue_t value;
    ClaPll_t   pll;
} ClaType_t;
typedef struct cla_power_type_stru_t
{
    float32 p;              //有功功率
    float32 q;              //无功功率
    float32 q_filter;              //无功功率滤波值
    float32 power_factor;   //功率因数
    float32 power_refactor; //无功功率因数
    float32 all;            //总功率
} ClaPowerType_t;

typedef struct cla_alternating_type_stru_t
{
    ClaType_t volt;              //电压
    ClaType_t curr;              //电流
    ClaPowerType_t power;        //功率
} ClaAlternatingType_t;

typedef struct cla_direct_type_stru_t
{
    ClaValue_t volt;              //电压
    ClaValue_t curr;              //电流
    float32   power;             //功率
} ClaDirectType_t;
typedef struct algo_state_stru_t
{
    //瞬时值
    volatile float32 inv_vol;      //逆变电压
    volatile float32 inv_cur;      //逆变电流
    volatile float32 inv_vol_ave;    //逆变平均电压
    volatile float32 grid_vol;      //市电电压
    volatile float32 grid_cur;      //市电电流
    volatile float32 grid_vol_ave;    //市电平均电压
    volatile float32 generator_vol;      //油机电压
    volatile float32 generator_cur;      //油机电流
    volatile float32 generator_vol_ave;    //油机平均电压
    
    volatile float32 bus_vol;      //bus电压
    volatile float32 bus_cur;      //bus电流
    volatile float32 bus_vol_rms;      //bus有效值电压,仅滤除纹波
    volatile float32 bus_cur_rms;      //bus有效值电流,仅滤除纹波

    volatile float32 inv_freq;
    volatile float32 inv_vol_rms; //
    volatile float32 inv_cur_rms; //
    volatile float32 inv_active_power; //
    volatile float32 inv_reactive_power; //
    volatile float32 inv_apperant_power; //

    volatile float32 grid_freq;
    volatile float32 grid_vol_rms; //
    volatile float32 grid_cur_rms; //
    volatile float32 grid_active_power; //
    volatile float32 grid_reactive_power; //
    volatile float32 grid_apperant_power; //

    volatile float32 generator_freq;
    volatile float32 generator_vol_rms; //
    volatile float32 generator_cur_rms; //
    volatile float32 generator_active_power; //
    volatile float32 generator_reactive_power; //
    volatile float32 generator_apperant_power; //

    volatile float32 load_vol_rms; //
    volatile float32 load_cur_rms; //
    volatile float32 load_active_power; //
    volatile float32 load_reactive_power; //
    volatile float32 load_apperant_power; //

    volatile float32 normalload_vol_rms; //
    volatile float32 normalload_cur_rms; //
    volatile float32 normalload_active_power; //
    volatile float32 normalload_reactive_power; //
    volatile float32 normalload_apperant_power; //

    volatile float32 ct_vol_rms; //
    volatile float32 ct_cur_rms; //
    volatile float32 ct_active_power; //
    volatile float32 ct_reactive_power; //
    volatile float32 ct_apperant_power; //

    volatile float32 inv_vol_theta;
    volatile float32 grid_vol_theta;
    volatile float32 grid_vol_sin;
    volatile float32 generator_vol_theta;

    volatile float32 relay_middle_volt_phase_a_rms;
    volatile float32 bus2batpower;  //给电池充电为正，放电为负

//     uint32_t grid_connect;   //并网状态
    uint32_t inv_start_ok;
    EN_INV_STATE inv_state;
} AlgoInfo_t;
float32 Get_Cla_Grid_A_Volt_Rms(void);
//float32 Get_Cla_Grid_B_Volt_Rms(void);
//float32 Get_Cla_Grid_C_Volt_Rms(void);
float32 Get_Cla_Grid_A_Volt_Instant(void);
//float32 Get_Cla_Grid_B_Volt_Instant(void);
//float32 Get_Cla_Grid_C_Volt_Instant(void);
float32 Get_Cla_Grid_PLL_Theta(void);

float32 Get_Cla_Inv_A_Volt_Rms(void);
//float32 Get_Cla_Inv_B_Volt_Rms(void);
//float32 Get_Cla_Inv_C_Volt_Rms(void);
float32 Get_Cla_Inv_A_Volt_Instant(void);
//float32 Get_Cla_Inv_B_Volt_Instant(void);
//float32 Get_Cla_Inv_C_Volt_Instant(void);
float32 Get_Cla_Inv_PLL_Theta(void);
float32 Get_Cla_Inv_Power_P(void);
float32 Get_Cla_Inv_Power_Q(void);
EN_INV_STATE Get_cla_Inv_state(void);

//交流量结构体
// extern ClaAlternatingType_t gstGrid;   //市电
// extern ClaAlternatingType_t gstInv;    //逆变
// extern ClaAlternatingType_t gstGenerator;    //逆变
// extern ClaAlternatingType_t gstBackUp;    //负载
// extern ClaAlternatingType_t gstNormalLoad;    //远端负载
//直流量结构体
extern ClaDirectType_t gstBusPos;      //BUS正
extern ClaDirectType_t gstBusNeg;      //BUS负
extern volatile AlgoInfo_t gstAlgoInterface;
extern volatile float garrDebugDataCla[16];
extern EN_INV_STATE geInvControlState;    //逆变运行状态 CPU控制CLA的状态接口
extern EN_INV_STATE geInvWorkState;
extern volatile float32 gInvTemp;

/* gstMidRelay / gstBusData 有效值在 CPU 中断 EN_CPU_INTERRUPT_TIME_3 中计算 */
void PwrCpuRelayBusInit(void);
void PwrCpuRelayBusStep(void);
#endif /* APP_CLA_CPU_SHARE_H_ */
