/*
 * adc.h
 *
 *  Created on: 2025年1月22日
 *      Author: Nova007456
 */

#ifndef DRIVER_ADC_H_
#define DRIVER_ADC_H_

#include "F28x_Project.h"
#include "system.h"

#define ACQPS_SYS_CLKS      (49) 
#define ADC_TRIG_SOURCE     (18)            //触发源为EPWM7的SOCB事件 
#define ADC_ZERO_MAX        (2100)          //ADC采样零点校准最大值
#define ADC_ZERO_MIN        (2000)          //ADC采样零点校准最小值


#define ADC_ZERO_LIMIT(x,common)    ((x>ADC_ZERO_MAX)?common:((x<ADC_ZERO_MIN)?common:x))
#define VALUE_LIMIT(x,max,min)      ((x>max)?max:((x<min)?min:x))

typedef struct adc_calibration_stru_t
{
    uint16_t calibration_finish;                 //ADC零点校准已完成
    float32 common_calibration_zero;             //通用零点偏执值
    float32 inv_volt_zero;               //A相逆变电压零点偏执值  //逆变电压
    // float32 inv_volt_phase_b_zero;               //B相逆变电压零点偏执值  //NC
    // float32 inv_volt_phase_c_zero;               //C相逆变电压零点偏执值     //NC
    float32 inv_L_curr_zero;               //A相逆变电感电流零点偏执值  //ADC_L_CURR
    // float32 inv_curr_phase_b_zero;               //B相逆变电感电流零点偏执值 //NC
    float32 inv_curr_tol_zero;               //C相逆变电感电流零点偏执值  //ADC_CURR_TOL
    float32 ct_curr_zero;                      //CT的A相电流零点偏执值      //CT电流
    // float32 ct_b_curr_zero;                      //CT的B相电流零点偏执值  //NC
    // float32 ct_c_curr_zero;                      //CT的C相电流零点偏执值  //NC
    float32 leak_curr_zero;                      //漏电流零点偏执值
    // float32 Grid_volt_phase_a_zero;              //A相市电电压零点偏执值 //NC
    // float32 Grid_volt_phase_b_zero;              //B相市电电压零点偏执值 //NC
    // float32 Grid_volt_phase_c_zero;              //C相市电电压零点偏执值 //NC
    // float32 Grid_curr_phase_a_zero;              //A相市电电流零点偏执值 //NC
    // float32 Grid_curr_phase_b_zero;              //B相市电电流零点偏执值 //NC
    float32 Grid_curr_zero;              //C相市电电流零点偏执值    //电网电流
    float32 pv1_volt_zero;                       //PV1电压零点偏执值
    float32 pv1_curr_zero;                       //PV1电流零点偏执值
    float32 bus_volt_all_zero;                   //正bus电压零点偏执值     //正负母线电压
    // float32 gen_volt_phase_a_zero;               //A相发电机电压零点偏执值 //NC
    // float32 gen_volt_phase_b_zero;               //B相发电机电压零点偏执值 //NC
    float32 gen_volt_zero;               //C相发电机电压零点偏执值    //油机电压
    // float32 gen_curr_phase_a_zero;               //A相发电机电流零点偏执值 //NC
    // float32 gen_curr_phase_b_zero;               //B相发电机电流零点偏执值 //NC
    float32 gen_curr_zero;               //C相发电机电流零点偏执值    //油机电流
    float32 pv2_volt_zero;                       //PV2电压零点偏执值
    float32 pv2_curr_zero;                       //PV2电流零点偏执值
    float32 bus_volt_pn_zero;                   //负bus电压零点偏执值       //BUS+N电压
    // float32 load_volt_phase_a_zero;              //A相负载电压零点偏执值  //NC
    // float32 load_volt_phase_b_zero;              //B相负载电压零点偏执值  //NC
    float32 load_volt_zero;              //C相负载电压零点偏执值
    // float32 load_current_phase_a_zero;           //A相负载电流零点偏执值  //NC
    // float32 load_current_phase_b_zero;           //B相负载电流零点偏执值  //NC
    float32 load_current_zero;           //C相负载电流零点偏执值
    float32 pv3_volt_zero;                       //PV3电压零点偏执值
    float32 pv3_curr_zero;                       //PV3电流零点偏执值
    float32 bus_curr_zero;                       //母线电流零点偏执值
    // float32 relay_middle_volt_phase_a_zero;      //主继电器与单继电器间电压A相零点偏执值  //NC
    // float32 relay_middle_volt_phase_b_zero;      //主继电器与单继电器间电压A相零点偏执值  //NC
    // float32 relay_middle_volt_phase_c_zero;      //主继电器与单继电器间电压A相零点偏执值  //NC
    float32 pv4_volt_zero;                       //PV4电压零点偏执值
    float32 pv4_curr_zero;                       //PV4电流零点偏执值
    float32 grid_vol_zero;                       //电网电压零点偏置值
} CpuAdcCalibration_t;

typedef struct cpu_adc_stru_t
{
    float32 inv_volt;               //A相逆变电压  //逆变电压
    // float32 inv_volt_phase_b;               //B相逆变电压  NC
    // float32 inv_volt_phase_c;               //C相逆变电压  NC
    float32 inv_L_curr;               //逆变电感电流 // ADC_L_CURR
    float32 inv_L_curr1;               //逆变电感电流 // ADC_L_CURR
    float32 inv_L_curr2;               //逆变电感电流 // ADC_L_CURR
    float32 inv_L_curr3;               //逆变电感电流 // ADC_L_CURR
    float32 inv_L_curr4;               //逆变电感电流 // ADC_L_CURR
    // float32 inv_curr_phase_b;               //B相逆变电感电流
    float32 inv_curr_tol;               //逆变器总端口电流ADC_CURR_TOL
    float32 inv_curr_tol1;               //逆变器总端口电流ADC_CURR_TOL
    float32 inv_curr_tol2;               //逆变器总端口电流ADC_CURR_TOL
    float32 inv_curr_tol3;               //逆变器总端口电流ADC_CURR_TOL
    float32 inv_curr_tol4;               //逆变器总端口电流ADC_CURR_TOL
    float32 ct_curr;                      //CT电流
    // float32 ct_b_curr;                      //CT的B相电流
    // float32 ct_c_curr;                      //CT的C相电流
    float32 leak_curr;                      //漏电流
    // float32 Grid_volt_phase_a;              //A相市电电压 NC
    // float32 Grid_volt_phase_b;              //B相市电电压 NC
    // float32 Grid_volt_phase_c;              //C相市电电压 NC
    // float32 Grid_curr_phase_a;              //A相市电电流 NC
    // float32 Grid_curr_phase_b;              //B相市电电流 NC
    float32 Grid_curr;              //市电电流
    float32 pv1_volt;                       //PV1电压
    float32 pv1_curr;                       //PV1电流
    float32 bus_volt_all;                   //bus正对N电压
    // float32 gen_volt_phase_a;               //A相发电机电压 NC
    // float32 gen_volt_phase_b;               //B相发电机电压 NC
    float32 gen_volt;               //发电机电压
    // float32 gen_curr_phase_a;               //A相发电机电流 NC
    // float32 gen_curr_phase_b;               //B相发电机电流 NC
    float32 gen_curr;               //C相发电机电流
    float32 pv2_volt;                       //PV2电压
    float32 pv2_curr;                       //PV2电流
    float32 bus_volt_pn;                   //正对Nbus电压
    // float32 load_volt_phase_a;              //A相负载电压 NC
    // float32 load_volt_phase_b;              //B相负载电压 NC
    float32 load_volt;              //负载电压
    // float32 load_current_phase_a;           //A相负载电流 NC
    // float32 load_current_phase_b;           //B相负载电流 NC
    float32 load_current;           //负载电流
    float32 pv3_volt;                       //PV3电压
    float32 pv3_curr;                       //PV3电流
    float32 bus_curr;                       //母线电流
    // float32 relay_middle_volt_phase_a;      //主继电器与单继电器间电压A相 NC
    // float32 relay_middle_volt_phase_b;      //主继电器与单继电器间电压A相 NC
    // float32 relay_middle_volt_phase_c;      //主继电器与单继电器间电压A相
    float32 pv4_volt;                       //PV4电压
    float32 pv4_curr;                       //PV4电流
    float32 grid_vol;                       //电网电压

    float32 Temp_A0;                         //预留温度采样
    float32 Temp_A1;                         //预留温度采样
    float32 Temp_A2;                         //预留温度采样
    float32 Temp_A3;                         //预留温度采样
    float32 Temp_A4;                         //预留温度采样
    float32 Temp_A5;                         //预留温度采样
    float32 Temp_A6;                         //预留温度采样
    float32 Temp_A7;                         //预留温度采样
} CpuAdc_t;

extern CpuAdc_t  gstAdcResult;              //ADC的采样结果
extern CpuAdc_t  gstAdcCoeff;               //ADC的采样系数
extern CpuAdcCalibration_t  gstAdcZero;     //ADC的零点值

void Adc_Init(void);
void GetAdcResult(EN_CPU_INTERRUPT_TIME value);
uint16_t Get_Adc_Calibration_State(void);

#endif /* DRIVER_ADC_H_ */
