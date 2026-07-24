/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sensor_control.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       板级支持包(BSP)核心函数，包含电池包电压采集、BMS电流采集等ADC相关功能实现
  ******************************************************************************
  * @attention   1. 依赖sensor_control.h中的ADC句柄(Adc_Handle)和ADC电压采集函数(AdcVoltageGet)
  *              2. ADC采集需确保硬件通道配置与代码中指定通道一致
  *              3. 计算系数需根据实际硬件电路参数调整，请勿随意修改
  * @relation
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "sensor_control.h"

/**
  * @brief    获取电池包总电压
  * @details  通过ADC通道9采集电池包分压后的电压信号，经过硬件系数校准后计算实际总电压
  * @param    无
  * @return   uint32_t: 电池包实际总电压（单位：0.1V，例：返回360表示36.0V）
  *           返回false表示ADC采集失败（注：实际返回为uint32_t，false本质是0，需结合业务判断采集状态）
  * @note     1. 硬件电路：电池包电压经分压电阻网络（270KΩ + 62KΩ 上拉，6.8KΩ 下拉）分压后输入ADC
  *           2. 校准逻辑：(采集电压 - 基准偏移1.5V) × (分压总电阻/下拉电阻)，基准偏移由ADC硬件特性决定
  */
uint32_t PackVotageGet(void)
{
    uint8_t c_current_chl = ADC_CHANNEL_9;  // ADC采集通道：通道9（对应电池包电压采集通道）
    float Voltage = 0;                     // 存储ADC采集到的原始电压值（单位：V）
    float VoltageSum = 0;                   // 20次采样电压累加和（用于计算平均值）
    float VoltageAve = 0;              // 20次采样电压平均值（单位：V）
    // 配置ADC句柄：指定当前采集通道和电压存储地址
    Adc_Handle.current_chl = c_current_chl;
    Adc_Handle.pRValtage = &Voltage;

    // 20次循环采样，累加采样值（平均滤波）
    for(uint8_t i = 0; i < 20; i++)
    {
        // 单次采集失败则直接返回false（0.0f）
        if(AdcVoltageGet(&Adc_Handle) == false)
        {
            return false;
        }
        VoltageSum += Voltage;  // 累加单次采样电压
    }
    VoltageAve = VoltageSum / 20;  // 计算20次采样的平均电压
    // 电压计算：原始电压减去基准偏移，再乘以分压系数（总电阻/(下拉电阻)）
    // 分压电阻：上拉总电阻=270KΩ+62KΩ，下拉电阻=6.8KΩ，系数=(270+62)/6.8≈48.8235
    VoltageAve = (VoltageAve - 1.5f) * (270 + 62) / 6.8f;

    // 转换为0.01V单位返回（避免浮点数传输误差，提高数据稳定性）
    return (VoltageAve * 100);
}

/**
  * @brief    获取BMS系统电流值
  * @details  通过指定ADC通道采集电流采样电阻的电压信号，经20次采样平均滤波后计算实际电流
  * @param    channel: ADC采集通道（对应不同的电流采样通道，需根据硬件定义传入）
  * @return   float: BMS实际电流值（单位：A，具体范围由硬件采样电阻和放大电路决定）
  *           返回false表示ADC采集失败（注：实际返回为float，false本质是0.0f，需结合业务判断采集状态）
  * @note     1. 采用20次采样平均滤波，降低电流波动带来的测量误差
  *           2. 校准系数139.48f由硬件参数决定（基于采样电阻阻值、运放放大倍数计算得出）
  *           3. 基准偏移1.5V：ADC参考电压中点，电流为0时采集电压理论值为1.5V
  */
uint32_t AdcBMSCurrentGet(uint8_t channel)
{
    float VCurrent = 0;        // 单次ADC采集的原始电压值（单位：V）
    float CurrentSum = 0;      // 20次采样电压累加和（用于计算平均值）
    float VCurrentAve = 0;     // 20次采样电压平均值（单位：V）

    // 配置ADC句柄：指定当前电流采集通道和电压存储地址
    Adc_Handle.current_chl = channel;
    Adc_Handle.pRValtage = &VCurrent;

    // 20次循环采样，累加采样值（平均滤波）
    for(uint8_t i = 0; i < 20; i++)
    {
        // 单次采集失败则直接返回false（0.0f）
        if(AdcVoltageGet(&Adc_Handle) == false)
        {
            return false;
        }
        CurrentSum += VCurrent;  // 累加单次采样电压
    }

    VCurrentAve = CurrentSum / 20;  // 计算20次采样的平均电压

    // 电流计算：(平均电压 - 基准偏移1.5V) × 硬件校准系数
    // 校准系数139.48f：由采样电阻R和运放增益A推导，公式：系数 = 1/(R×A)，需根据实际硬件调整
    VCurrentAve = (VCurrentAve - 1.5f) * 139.48f;

    // 返回实际电流值（单位：A）
    return ((VCurrentAve));
}