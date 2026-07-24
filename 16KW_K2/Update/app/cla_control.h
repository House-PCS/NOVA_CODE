
/*
 * cla_init.h
 *
 *  Created on: 2024年8月27日
 *      Author: Nova007456
 */
#ifndef MIDDLEWARE_CLA_ALGO_H_
#define MIDDLEWARE_CLA_ALGO_H_
#endif

#include "F28x_Project.h"
#include "adc.h"
#include "cla_cpu_share.h"

#define PLL_FREQ_MAX                (70)        //锁相环频率最大值
#define PLL_FREQ_MIN                (40)        //锁相环频率最小值
//#define L                           (0.00095)   //逆变电感参数
//#define C                           (0.0000082) //逆变电容参数
#define VALUE_LIMIT(x,max,min)      ((x>max)?max:((x<min)?min:x))
#define VALUE_MAX3(x,y,z)           ((((x > y) ? x : y) > z) ? ((x > y) ? x : y) : z)
#define VALUE_MIN3(x,y,z)           ((((x < y) ? x : y) < z) ? ((x < y) ? x : y) : z)

// 枚举定义
// 结构体定义

typedef struct cla_pi_loop_stru_t
{
    float32 ref_amplitude;     //给定的幅值
    float32 ref_amplitude_eff;     //生效给定的幅值
    float32 ref_real;          //实际给定值
    float32 err;               //误差值
    float32 err1;

    float32 kp;
    float32 ki;
    float32 out;
    float32 out1;

    float32 a1;
    float32 b0;
    float32 b1;
} ClaPiLoop_t;

typedef struct cla_pr_loop_stru_t
{
    float32 ref_amplitude;     //给定的幅值
    float32 ref_amplitude_eff;     //给定的幅值
    float32 ref_real;          //实际给定值
    float32 err;               //误差值
    float32 err1;
    float32 err2;

    float32 kp;
    float32 kr;
    float32 wc;
    float32 w0;
    float32 out;
    float32 out1;
    float32 out2;

    float32 a0;
    float32 a1;
    float32 a2;
    float32 b0;
    float32 b1;
    float32 b2;
    float32 Umax;
    float32 Umin;
} ClaPrLoop_t;

typedef struct cla_connect_power_loop_stru_t
{
    ClaPiLoop_t p_power;          //有功功率
    ClaPiLoop_t q_power;          //无功功率
    float32 ref;                  //并网功率给定
} ConnectPowerLoop_t;

typedef struct cla_inv_loop_stru_t
{
    ClaPiLoop_t disconnect_curr;          //电流
    ClaPrLoop_t disconnect_volt;          //电压
    ClaPiLoop_t connect_volt;             //并网电压
    ClaPiLoop_t connect_curr;             //并网电流
    ConnectPowerLoop_t connect_power;    //并网功率环
    float32    duty;                     //占空比
    Uint32     en_inv;                   //逆变器使能
    Uint32     inv_start_ok;             //逆变软起标志位
} InvLoop_t;


typedef struct  cla_sin_cos_signal_stru_t
{
    float32 sin_ext[2];                   //内部正弦信号
    float32 cos_ext[2];                   //内部正弦信号
    float32 theta_ext;          //离网正弦参考信号
    float32 sin_disconnect;            //离网正弦参考信号
    float32 cos_disconnect;            //离网正弦参考信号
    float32 theta_disconnect;          //生效正弦参考信号
    float32 wn_disconnect;
    float32 sin_eff[2];            //生效正弦参考信号
    float32 cos_eff[2];            //生效正弦参考信号
    float32 theta_eff;          //生效正弦参考信号
    float32 wn_eff;          //生效正弦角频率
    float32 delta_wn;           //目标正弦参考信号和生效正弦参考信号间追踪角频率

    float32 sin_grid;           //电网正弦参考信号
    float32 cos_grid;           //电网正弦参考信号
    float32 theta_grid;         //电网正弦参考信号
    float32 wn_grid;         //电网角频率
    float32 sin_generator;      //油机正弦参考信号
    float32 cos_generator;      //油机正弦参考信号
    float32 theta_generator;    //油机正弦参考信号
    float32 wn_generator;         //油机角频率

    float32 sin_aim;        //目标正弦参考信号
    float32 cos_aim;        //目标正弦参考信号
    float32 theta_aim;          //目标正弦参考信号
    float32 wn_aim;          //目标正弦角频率
} ClaSinCosType_t;





__attribute__((interrupt)) void cla1Isr1();
__attribute__((interrupt)) void cla1Isr2();
__attribute__((interrupt)) void cla1Isr3();
__attribute__((interrupt)) void cla1Isr4();
__attribute__((interrupt)) void cla1Isr5();
__attribute__((interrupt)) void cla1Isr6();
__attribute__((interrupt)) void cla1Isr7();
__attribute__((interrupt)) void cla1Isr8();

__attribute__((interrupt)) void Cla1Task1();
__attribute__((interrupt)) void Cla1Task2();
__attribute__((interrupt)) void Cla1Task3();
__attribute__((interrupt)) void Cla1Task4();
__attribute__((interrupt)) void Cla1Task5();
__attribute__((interrupt)) void Cla1Task6();
__attribute__((interrupt)) void Cla1Task7();
__attribute__((interrupt)) void Cla1Task8();

void SetClaDebugData(uint16_t buff_num,float value);

float32 Cla_Get_InvA_Volt_Rms(void);
float32 Cla_Get_InvB_Volt_Rms(void);
float32 Cla_Get_InvC_Volt_Rms(void);


