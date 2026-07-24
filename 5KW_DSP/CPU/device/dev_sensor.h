/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_sensor.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-7-24
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-7-24    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __DEV_SENSOR_H__
#define __DEV_SENSOR_H__

#include "adc.h"
#include "math.h"
#include "hard_cmpss.h"
#include "sys_info.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_GPIO        31

#define ADC_VOL_COEFFICIENT   /*(float32)(0.310417624081)*/(float32)(423.82352941176)       //(470*6+62)/6.8 //* ADC_12BIT_COEFFICIENT
#define ADC_VOL_COEFFICIENT1  /*(float32)(0.209171070772)*/(float32)(423.82352941176)       //(470*4+62)/6.8 //* ADC_12BIT_COEFFICIENT 旧板285.58823529411
#define ADC_VOL_COEFFICIENT2  /*(float32)(0.057301240809)*/(float32)(78.235294117647)       //(470+62)/6.8 //* ADC_12BIT_COEFFICIENT
#define ADC_12BIT_COEFFICIENT (float32)(0.000732421875)     //3.0/4096
#define ADC_REF               (int32_t)(2048)
#define PV_RECOVER_COUNT    (20000) // 50us * 60000 = 1s
#define PV_OC_POINT         (28)    //28A

#define ADC_LOAD_CURRENT        (AdcaResultRegs.ADCRESULT4)
#define ADC_LOAD_CURRENT2       (AdcaResultRegs.ADCRESULT12)//负载过零采样
#define ADC_MICRO_GRID_CURRENT  (AdcaResultRegs.ADCRESULT1)//油机
#define ADC_MICRO_GRID_CURRENT2 (AdcaResultRegs.ADCRESULT9)//油机零点采样
#define ADC_POWER_GRID_CURRENT  (AdcaResultRegs.ADCRESULT2)//电网
#define ADC_POWER_GRID_CURRENT2 (AdcaResultRegs.ADCRESULT10)//电网零点采样
#define ADC_LEAK_CURRENT        (AdcaResultRegs.ADCRESULT3)
#define ADC_INDUCTOR_CURRENT    (AdcaResultRegs.ADCRESULT0)
#define ADC_INDUCTOR_CURRENT2   (AdcaResultRegs.ADCRESULT8)
#define ADC_TRANSFORMER_CURRENT (AdcaResultRegs.ADCRESULT5)
#define ADC_PV2_CURRENT         (AdcbResultRegs.ADCRESULT0)
#define ADC_PV1_VOLTAGE         (AdcbResultRegs.ADCRESULT1)
#define ADC_PV2_VOLTAGE         (AdcbResultRegs.ADCRESULT2)
#define ADC_INVERT_VOLTAGE      (AdcbResultRegs.ADCRESULT3)
#define ADC_DC_COMPONENT        (AdccResultRegs.ADCRESULT0)
#define ADC_CT_CURRENT          (AdccResultRegs.ADCRESULT1)
#define ADC_TEMP                (AdccResultRegs.ADCRESULT2)
#define ADC_POWER_GRID_VOLTAGE  (AdcdResultRegs.ADCRESULT0)
#define ADC_MICRO_GRID_VOLTAGE  (AdcdResultRegs.ADCRESULT1)
#define ADC_LOAD_VOLTAGE        (AdcdResultRegs.ADCRESULT2)
#define ADC_BAT_VOLTAGE         (AdcdResultRegs.ADCRESULT3)
#define ADC_BUS_VOLTAGE         (AdcdResultRegs.ADCRESULT4)
#define ADC_BAT_CURRENT         (AdcaResultRegs.ADCRESULT6)
#define ADC_PV1_CURRENT         (AdcaResultRegs.ADCRESULT7)

#define RTEMP_B     4050
#define RTEMP25     10//100
#define T25         (25 + 273.15)
#define T25_1       (float)(1.0 / (25 + 273.15))
#define VOL_UP      3.3
#define R_UP        10
#define R_DOWN      75

#define CT_DIR 1  //TODO: 链接为变量，上位机写下来，往逆变器为正方向


typedef struct _sensor_handle_stru_t
{
    volatile float load_current;
    volatile float micro_grid_current;
    volatile float power_grid_current;
    volatile float leak_current;
    volatile float inductor_current;
    volatile float inductor_current_test;
    volatile float transformer_current;
    volatile float pv2_current;
    volatile float pv1_voltage;
    volatile float pv2_voltage;
    volatile float invert_voltage;
    volatile float dc_component;
    volatile float CT_current;
    volatile float temp[8];         //1:INV 2:PV 3:DCDC
    volatile float power_grid_voltage;
    volatile float micro_grid_voltage;
    volatile float load_voltage;
    volatile float bat_voltage;
    volatile float bus_voltage;
    volatile float bat_current;
    volatile float pv1_current;
} Sensor_Handle_t;

typedef struct _sensor_drift_stru_t
{
    volatile float inductor_cur_drift;
    volatile float grid_cur_drift;
    volatile float load_cur_drift;
    volatile float pv1_cur_drift;
    volatile float pv2_cur_drift;
} Sensor_Drift_t;


extern Sensor_Handle_t gstSensorData;
extern uint16_t CalibrationFlag;
extern float AdcRef;
extern uint16_t gIntCount;
extern uint32_t gPvOvrCurRecoverCount;

extern uint16_t gCD4051Chinnel;
extern uint16_t garrTemperInitialData[8];
extern Sensor_Drift_t gstSenorDrift;

inline void ReadCurrVolADCSignals(void);
void SensorInit(void);
void CD4051ChannelSet(uint16_t channel);
void InvTemperatureGet(void);
void PvTemperatureGet(void);
void DcdcTemperatureGet(void);
void ChassisInternalTemperatureGet(void);
void InvTemperMaxGet(void);
extern uint16_t Updata_cmpss_dac(float adcref_reg); // 前置声明：解决循环包含导致hard_cmpss.h声明不可见的问题
#define SENSOR_TEMP_MAX_IDX 8 // 20260429，优化代码


inline void ReadCurrVolADCSignals(void)
{
    if(!CalibrationFlag)
    {
        gstSensorData.temp[0] = (float32)ADC_TEMP;
        if(gstSensorData.temp[0] > 2000 && gstSensorData.temp[0] < 2100)
        {
            AdcRef = 1.5 / gstSensorData.temp[0];

            CalibrationFlag = 1;

            /* 切换到第一个采集通道 */
            gCD4051Chinnel = 1;
            CD4051ChannelSet(gCD4051Chinnel);
            Updata_cmpss_dac(gstSensorData.temp[0]); //更新硬件cmpss过流保护阈值
        }
    }
    else
    {
        gstSensorData.pv1_voltage            = ((float32)(ADC_PV1_VOLTAGE - gstSensorData.temp[0])) * ADC_VOL_COEFFICIENT * AdcRef;// * ADC_12BIT_COEFFICIENT;
        gstSensorData.pv2_voltage            = ((float32)(ADC_PV2_VOLTAGE - gstSensorData.temp[0])) * ADC_VOL_COEFFICIENT * AdcRef;// * ADC_12BIT_COEFFICIENT;
        gstSensorData.invert_voltage         = ((float32)(ADC_INVERT_VOLTAGE - gstSensorData.temp[0])) * ADC_VOL_COEFFICIENT1 * AdcRef;// * ADC_12BIT_COEFFICIENT;
        gstSensorData.power_grid_voltage     = ((float32)(ADC_POWER_GRID_VOLTAGE - gstSensorData.temp[0])) * ADC_VOL_COEFFICIENT1 * AdcRef;// * ADC_12BIT_COEFFICIENT;
        gstSensorData.micro_grid_voltage     = ((float32)(ADC_MICRO_GRID_VOLTAGE - gstSensorData.temp[0])) * ADC_VOL_COEFFICIENT1 * AdcRef;// * ADC_12BIT_COEFFICIENT;
        gstSensorData.load_voltage           = ((float32)(ADC_LOAD_VOLTAGE - gstSensorData.temp[0])) * ADC_VOL_COEFFICIENT1 * AdcRef;// * ADC_12BIT_COEFFICIENT;
        gstSensorData.bat_voltage            = ((float32)(ADC_BAT_VOLTAGE - gstSensorData.temp[0])) * ADC_VOL_COEFFICIENT2 * AdcRef;// * ADC_12BIT_COEFFICIENT;
        gstSensorData.bus_voltage            = ((float32)(ADC_BUS_VOLTAGE - gstSensorData.temp[0])) * ADC_VOL_COEFFICIENT * AdcRef;// * ADC_12BIT_COEFFICIENT;
        gstSensorData.dc_component           = ((float32)(ADC_DC_COMPONENT - gstSensorData.temp[0])) * 941 * AdcRef;//941 * ADC_12BIT_COEFFICIENT;
        gstSensorData.transformer_current    = ((float32)(ADC_TRANSFORMER_CURRENT - gstSensorData.temp[0])) * 20 * AdcRef;//20 * ADC_12BIT_COEFFICIENT;
        gstSensorData.inductor_current       = ((float32)(((ADC_INDUCTOR_CURRENT + ADC_INDUCTOR_CURRENT2) >> 1) - gstSensorData.temp[0])) * 80 * AdcRef - gstSenorDrift.inductor_cur_drift;//80 * ADC_12BIT_COEFFICIENT;
        gstSensorData.inductor_current_test  = ((float32)(ADC_INDUCTOR_CURRENT2 - gstSensorData.temp[0])) * 80 * AdcRef;//80 * ADC_12BIT_COEFFICIENT;
        gstSensorData.power_grid_current     = ((float32)(((ADC_POWER_GRID_CURRENT + ADC_POWER_GRID_CURRENT2) >> 1) - gstSensorData.temp[0])) * 80 * AdcRef - gstSenorDrift.grid_cur_drift;//20 * ADC_12BIT_COEFFICIENT;
        gstSensorData.micro_grid_current     = ((float32)(((ADC_MICRO_GRID_CURRENT + ADC_MICRO_GRID_CURRENT2) >> 1) - gstSensorData.temp[0])) * 20 * AdcRef;//20 * ADC_12BIT_COEFFICIENT;
        gstSensorData.load_current           = ((float32)(((ADC_LOAD_CURRENT + ADC_LOAD_CURRENT2) >> 1) - gstSensorData.temp[0])) * 80 * AdcRef - gstSenorDrift.load_cur_drift;//20 * ADC_12BIT_COEFFICIENT;
        gstSensorData.pv1_current            = ((float32)(ADC_PV1_CURRENT - gstSensorData.temp[0])) * 20 * AdcRef - gstSenorDrift.pv1_cur_drift;//40 * ADC_12BIT_COEFFICIENT;
        gstSensorData.pv2_current            = ((float32)(ADC_PV2_CURRENT - gstSensorData.temp[0])) * 20 * AdcRef - gstSenorDrift.pv2_cur_drift;//40 * ADC_12BIT_COEFFICIENT;
        gstSensorData.leak_current           = (float32)ADC_LEAK_CURRENT;
        gstSensorData.bat_current            = ((float32)(ADC_BAT_CURRENT - gstSensorData.temp[0])) * -150.0 * AdcRef;//20 * ADC_12BIT_COEFFICIENT;//注意电流方向
        gstSensorData.CT_current             = ((float32)(ADC_CT_CURRENT - gstSensorData.temp[0])) * 121.95 * AdcRef * CT_DIR;//STK-BS1为20mV/A->50A/V,CT_DIR为安装方向,CT对应90A/90mA->0.738V/90A->121.95A/V

        if(gCD4051Chinnel < SENSOR_TEMP_MAX_IDX) // 20260429，优化代码
        {
            garrTemperInitialData[gCD4051Chinnel] = ADC_TEMP;
        }

        if((gstSensorData.pv1_current >= PV_OC_POINT) || (gstSensorData.pv2_current >= PV_OC_POINT))
        {
            gPvOvrCurRecoverCount = 0;
            EALLOW;
            EPwm11Regs.TZFRC.bit.OST = 1;
            EDIS;
        }
        else
        {
            gPvOvrCurRecoverCount++;

            if(gPvOvrCurRecoverCount >= PV_RECOVER_COUNT)
            {
                gPvOvrCurRecoverCount = PV_RECOVER_COUNT;  //计数3S
            }
        }
        
        gIntCount++;
        if(gIntCount > 1000)
        {
            gIntCount = 0;
            gCD4051Chinnel++;
        }
        if(gCD4051Chinnel > 7)
        {
            gCD4051Chinnel = 1;
        }
        CD4051ChannelSet(gCD4051Chinnel);
    }


}
#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif
