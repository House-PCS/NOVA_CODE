/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_ntc.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       NTC温度传感器相关驱动实现，包含ADC采集、电阻-温度转换、GPIO控制等功能
  ******************************************************************************
  * @attention   1. 依赖ADC驱动模块和GPIO驱动模块
  *              2. 温度计算基于NTC热敏电阻B值公式
  *              3. 供电电压为3.3V，ADC采集电压需限制在3.3V以内
  * @relation
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_ntc.h"
#include "math.h"

#define LOG_TAG     "ntc"       // 日志标签，用于调试输出
#define RTEMP_B     3435        // NTC热敏电阻B值（3435K）
#define RTEMP25     10          // NTC在25℃时的标称电阻值（单位：KΩ）
#define T25         (25 + 273.15)  // 25℃对应的开尔文温度（单位：K）
#define R_UP        10          // 默认上拉电阻值（单位：KΩ）
#define LN         _appro_ln    // 自定义自然对数计算函数宏定义

/**
  * @brief    自然对数近似计算函数（泰勒级数展开）
  * @param    a: 输入正数（需大于0）
  * @return   double: 输入值的自然对数结果（ln(a)）
  * @note     1. 采用泰勒级数展开前16项（N=15）进行近似计算，保证精度
  *           2. 适用范围：a > 0，推荐a在0.1~10之间以获得最佳精度
  */
static double _appro_ln(double a)
{
    uint8_t N = 15;        // 泰勒级数展开项数控制（取前15+1项）
    uint8_t k, nk;         // 循环变量（k：反向循环计数，nk：级数项分母）
    double x, x2, y;       // 中间变量（x：变换后的输入值，x2：x的平方，y：级数累加结果）

    x = (a - 1) / (a + 1); // 输入值变换，优化泰勒级数收敛速度
    x2 = x * x;            // 预计算x的平方，减少重复运算
    nk = 2 * N + 1;        // 初始级数项分母（第16项分母）
    y = 1.0 / nk;          // 初始累加值（第16项的值）

    // 反向循环计算泰勒级数前15项，累加得到最终结果
    for(k = N; k > 0; k--)
    {
        nk -= 2;           // 更新级数项分母（依次递减2）
        y = 1.0 / nk + x2 * y; // 累加当前项，结合前一项结果
    }

    return 2.0 * x * y;    // 还原变换后的结果，得到ln(a)
}

/**
  * @brief    根据ADC采集的电压值计算NTC电阻值
  * @param    u_rtemp: ADC采集到的NTC分压电压（单位：V）
  * @return   float: NTC热敏电阻的当前电阻值（单位：KΩ）
  * @note     1. 电路为串联分压：VCC(3.3V) -> 上拉电阻(R_UP) -> NTC -> GND
  *           2. 电压限制：当输入电压≥3.3V时，强制设为3.299999V，避免除法溢出
  *           3. 计算公式：Rntc = R_UP * U_ntc / (VCC - U_ntc)
  */
static float RtempCal(float u_rtemp)
{
    float rtemp;           // 存储计算得到的NTC电阻值

    // 电压上限保护：避免因ADC误差导致分母为0
    if(u_rtemp >= 3.3f)
    {
        u_rtemp = 3.299999f;
    }

    // 分压公式计算NTC电阻值
    rtemp = R_UP * u_rtemp / (3.3f - u_rtemp);
    return rtemp;
}

/**
  * @brief    根据NTC电阻值计算温度（B值法）
  * @param    Rntc: NTC当前电阻值（单位：KΩ）
  * @return   float: 计算得到的环境温度（单位：℃）
  * @note     1. 基于NTC热敏电阻B值公式：1/T = 1/T25 + (ln(Rntc/R25))/B
  *           2. 温度转换：开尔文温度(T) = 摄氏度 + 273.15
  */
float GetTemperature(float Rntc)
{
    float tmp1;            // 中间变量（存储1/T的计算结果）
    float res;             // 最终温度结果（单位：℃）

    // B值公式计算1/T（T为开尔文温度）
    tmp1 = LN(Rntc / RTEMP25) / RTEMP_B + 1 / T25;
    // 转换为摄氏度并返回
    res = 1 / tmp1 - 273.15f;
    return res;
}

/**
  * @brief    特定B值（4200K）NTC的温度计算函数
  * @param    Rntc: NTC当前电阻值（单位：KΩ）
  * @return   float: 计算得到的环境温度（单位：℃）
  * @note     1. 适用于B值为4200K、25℃标称电阻100KΩ的NTC
  *           2. 公式与GetTemperature一致，仅B值和标称电阻不同
  */
float GetTemperature4200(float Rntc)
{
    float tmp1;            // 中间变量（存储1/T的计算结果）
    float res;             // 最终温度结果（单位：℃）

    // B值4200K，标称电阻100KΩ，计算1/T
    tmp1 = LN(Rntc / 100) / 4200 + 1 / T25;
    // 转换为摄氏度并返回
    res = 1 / tmp1 - 273.15f;
    return res;
}

/**
  * @brief    指定ADC通道采集NTC电压并计算温度
  * @param    channel: ADC采集通道号（需与硬件配置一致）
  * @return   float: 采集通道对应的环境温度（单位：℃）；采集失败返回false
  * @note     1. 通过ADC_Handle结构体配置采集通道和电压存储地址
  *           2. 调用AdcVoltageGet获取ADC电压，失败则返回错误
  *           3. 依赖ADC驱动的AdcVoltageGet函数实现
  */
float TemperatureGet(uint8_t channel)
{
    float res;             // 最终温度结果（单位：℃）
    float volt;            // 存储ADC采集到的电压值（单位：V）
    float rntc;            // 计算得到的NTC电阻值（单位：KΩ）

    // 配置ADC采集通道和电压存储指针
    Adc_Handle.current_chl = channel;
    Adc_Handle.pRValtage = &volt;

    // 采集ADC电压，失败则返回false
    if(AdcVoltageGet(&Adc_Handle) == false)
    {
        return false;
    }

    // 计算NTC电阻值
    rntc = RtempCal(volt);
    // 计算温度并返回
    res  = GetTemperature(rntc);
    return res;
}

/**
  * @brief    自定义上拉电阻和ADC电压的温度转换函数
  * @param    adc_vol: ADC采集到的NTC分压电压（单位：V）
  * @param    r_up: 自定义上拉电阻值（单位：KΩ）
  * @return   float: 计算得到的环境温度（单位：℃）
  * @note     1. 支持用户自定义上拉电阻值，灵活适配不同硬件电路
  *           2. 电压上限保护：输入电压≥3.3V时强制设为3.299999V
  *           3. 基于B值3435K的NTC进行温度计算
  */
float TemperatureDataChange(float adc_vol, uint8_t r_up)
{
    float res;             // 最终温度结果（单位：℃）
    float volt;            // 存储处理后的ADC电压（单位：V）
    float rntc;            // 计算得到的NTC电阻值（单位：KΩ）

    volt = adc_vol;        // 接收输入的ADC电压值

    // 电压上限保护，避免分母为0
    if(volt >= 3.3f)
    {
        volt = 3.299999;
    }

    // 分压公式计算NTC电阻值（自定义上拉电阻）
    rntc = r_up * volt / (3.3f - volt);   // 上拉电阻100K（注释为示例，实际为输入参数r_up）
    // 计算温度并返回（B值3435K）
    res  = GetTemperature(rntc);
    return res;
}

/**
  * @brief    自定义上拉电阻和ADC电压的特定B值（4200K）温度转换函数
  * @param    adc_vol: ADC采集到的NTC分压电压（单位：V）
  * @param    r_up: 自定义上拉电阻值（单位：KΩ）
  * @return   float: 计算得到的环境温度（单位：℃）
  * @note     1. 支持用户自定义上拉电阻值，适配不同硬件电路
  *           2. 电压上限保护：输入电压≥3.3V时强制设为3.299999V
  *           3. 基于B值4200K的NTC进行温度计算
  */
float TemperatureDataChange4200(float adc_vol, uint8_t r_up)
{
    float res;             // 最终温度结果（单位：℃）
    float volt;            // 存储处理后的ADC电压（单位：V）
    float rntc;            // 计算得到的NTC电阻值（单位：KΩ）

    volt = adc_vol;        // 接收输入的ADC电压值

    // 电压上限保护，避免分母为0
    if(volt >= 3.3f)
    {
        volt = 3.299999;
    }

    // 分压公式计算NTC电阻值（自定义上拉电阻）
    rntc = r_up * volt / (3.3f - volt);   // 上拉电阻100K（注释为示例，实际为输入参数r_up）
    // 计算温度并返回（B值4200K）
    res  = GetTemperature4200(rntc);
    return res;
}

/**
  * @brief    设置BMS温度传感器S0通道GPIO为高电平
  * @param    无
  * @return   无
  * @note     1. 控制BMS温度采集通道S0的选通GPIO
  *           2. TempS0Handle为GPIO句柄，需在硬件配置中预先定义
  */
void SetBMSTempS0(void)
{
    GpioBitSet((&TempS0Handle));
}

/**
  * @brief    设置BMS温度传感器S0通道GPIO为低电平
  * @param    无
  * @return   无
  * @note     1. 关闭BMS温度采集通道S0的选通
  *           2. 与SetBMSTempS0配合使用，实现多通道切换
  */
void ResetBMSTempS0(void)
{
    GpioBitReset((&TempS0Handle));
}

/**
  * @brief    设置BMS温度传感器S1通道GPIO为高电平
  * @param    无
  * @return   无
  * @note     1. 控制BMS温度采集通道S1的选通GPIO
  *           2. TempS1Handle为GPIO句柄，需在硬件配置中预先定义
  */
void SetBMSTempS1(void)
{
    GpioBitSet((&TempS1Handle));
}

/**
  * @brief    设置BMS温度传感器S1通道GPIO为低电平
  * @param    无
  * @return   无
  * @note     1. 关闭BMS温度采集通道S1的选通
  *           2. 与SetBMSTempS1配合使用，实现多通道切换
  */
void ResetBMSTempS1(void)
{
    GpioBitReset((&TempS1Handle));
}

/**
  * @brief    设置BMS温度传感器S2通道GPIO为高电平
  * @param    无
  * @return   无
  * @note     1. 控制BMS温度采集通道S2的选通GPIO
  *           2. TempS2Handle为GPIO句柄，需在硬件配置中预先定义
  */
void SetBMSTempS2(void)
{
    GpioBitSet((&TempS2Handle));
}

/**
  * @brief    设置BMS温度传感器S2通道GPIO为低电平
  * @param    无
  * @return   无
  * @note     1. 关闭BMS温度采集通道S2的选通
  *           2. 与SetBMSTempS2配合使用，实现多通道切换
  */
void ResetBMSTempS2(void)
{
    GpioBitReset((&TempS2Handle));
}