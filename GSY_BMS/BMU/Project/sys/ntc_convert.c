/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        ntc_convert.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-8-20    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "ntc_convert.h"



#define LN         _appro_ln
//#define LN            log


/** 
 * @brief    NTC当前阻值计算
 * @param    a:要计算ln的指数
 * @return   ln值
 * @note:    
 */
static double _appro_ln(double a)
{
    uint8_t N = 15;//我们取前15+1项来估算
    uint8_t k, nk;
    double x, x2, y;

    x = (a - 1) / (a + 1);
    x2 = x * x;
    nk = 2 * N + 1;
    y = 1.0 / nk;

    for(k = N; k > 0; k--)
    {
        nk -= 2;
        y = 1.0 / nk + x2 * y;
    }
    return 2.0 * x * y;
}

/** 
 * @brief    NTC当前阻值计算
 * @param    adc_volt：ADC采样的电压值，单位mV
 * @param    volt_max：上拉电阻所接的电压
 * @param    r_up：上拉电阻阻值，单位Ω
 * @return   ntc当前阻值，单位Ω
 * @note:    
 */
static float _RNTCCalc(uint16_t adc_volt, uint16_t volt_max, uint32_t r_up)
{
    float r_ntc;
    if(adc_volt >= volt_max)
    {
        adc_volt = volt_max - 1;
    }
    r_ntc = ((float)(adc_volt * r_up)) / (volt_max - adc_volt);
    return r_ntc;
}



/** 
 * @brief    获取当前采样的温度值
 * @param    adc_volt：ADC采样的电压值，单位mV
 * @param    volt_max：上拉电阻所接的电压
 * @param    r_up：上拉电阻阻值，单位Ω
 * @param    ntc_b:ntc对应的B值
 * @param    ntc_t25:ntc在25度时的阻值，单位Ω
 * @return   ntc当前阻值，单位Ω
 * @note:    
 */
float TemperatureGet(uint16_t adc_volt, uint16_t volt_max, uint32_t r_up, EN_NTC_B ntc_b, EN_NTC_T25 ntc_t25)
{
    float r_ntc;
    float temp;
    
    r_ntc = _RNTCCalc(adc_volt, volt_max, r_up);
    temp = LN(r_ntc / ntc_t25) / ntc_b + 1 / (25 + 273.15);
    
    return (1 / temp - 273.15f);
}
    




















