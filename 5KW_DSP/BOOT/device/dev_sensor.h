/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_sensor.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __DEV_SENSOR_H__
#define __DEV_SENSOR_H__

#include "adc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_VOL_COEFFICIENT   /*(float32)(0.310417624081)*/(float32)(423.82352941176)		//(470*6+62)/6.8 //* ADC_12BIT_COEFFICIENT
#define ADC_VOL_COEFFICIENT1  /*(float32)(0.209171070772)*/(float32)(285.58823529411)		//(470*4+62)/6.8 //* ADC_12BIT_COEFFICIENT
#define ADC_VOL_COEFFICIENT2  /*(float32)(0.057301240809)*/(float32)(78.235294117647)		//(470+62)/6.8 //* ADC_12BIT_COEFFICIENT
#define ADC_12BIT_COEFFICIENT (float32)(0.000732421875)		//3.0/4096
#define ADC_REF				  (int32_t)(2048)

#define ADC_LOAD_CURRENT        (AdcaResultRegs.ADCRESULT0)
#define ADC_MICRO_GRID_CURRENT  (AdcaResultRegs.ADCRESULT1)
#define ADC_POWER_GRID_CURRENT  (AdcaResultRegs.ADCRESULT2)
#define ADC_LEAK_CURRENT        (AdcaResultRegs.ADCRESULT3)
#define ADC_INDUCTOR_CURRENT    (AdcaResultRegs.ADCRESULT4)
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

typedef struct _sensor_handle_stru_t
{
    volatile float load_current;
    volatile float micro_grid_current;
    volatile float power_grid_current;
    volatile float leak_current;
    volatile float inductor_current;
    volatile float transformer_current;
    volatile float pv2_current;
    volatile float pv1_voltage;
    volatile float pv2_voltage;
    volatile float invert_voltage;
    volatile float dc_component;
    volatile float ct_current;
    volatile float temp[8];
    volatile float power_grid_voltage;
    volatile float micro_grid_voltage;
    volatile float load_voltage;
    volatile float bat_voltage;
    volatile float bus_voltage;
    volatile float bat_current;
    volatile float pv1_current;
} Sensor_Handle_t;


extern Sensor_Handle_t stSensorData;
extern uint16_t CalibrationFlag;
extern float AdcRef;

inline void readCurrVolADCSignals(void);
void SensorInit(void);
void CD4051ChannelSet(uint16_t channel);



inline void readCurrVolADCSignals(void)
{
//  stSensorData.pv1_voltage            = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_PV1_VOLTAGE - 1.5)) * ADC_VOL_COEFFICIENT;
//  stSensorData.pv2_voltage            = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_PV2_VOLTAGE - 1.5)) * ADC_VOL_COEFFICIENT;
//  stSensorData.invert_voltage         = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_INVERT_VOLTAGE - 1.5)) * ADC_VOL_COEFFICIENT1;
//  stSensorData.power_grid_voltage     = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_POWER_GRID_VOLTAGE - 1.5)) * ADC_VOL_COEFFICIENT1;
//  stSensorData.micro_grid_voltage     = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_MICRO_GRID_VOLTAGE - 1.5)) * ADC_VOL_COEFFICIENT1;
//  stSensorData.load_voltage           = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_LOAD_VOLTAGE - 1.5)) * ADC_VOL_COEFFICIENT1;
//  stSensorData.bat_voltage            = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_BAT_VOLTAGE - 1.5)) * ADC_VOL_COEFFICIENT2;
//  stSensorData.bus_voltage            = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_BUS_VOLTAGE - 1.5)) * ADC_VOL_COEFFICIENT;
//  stSensorData.dc_component           = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_DC_COMPONENT - 1.5)) * 941;
//  stSensorData.transformer_current    = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_TRANSFORMER_CURRENT - 1.5)) * 20;
//  stSensorData.inductor_current       = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_INDUCTOR_CURRENT - 1.5)) * 80;
//  stSensorData.power_grid_current     = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_POWER_GRID_CURRENT - 1.5)) * 20;
//  stSensorData.micro_grid_current     = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_MICRO_GRID_CURRENT - 1.5)) * 20;
//  stSensorData.load_current           = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_LOAD_CURRENT - 1.5)) * 20;
//  stSensorData.pv1_current            = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_PV1_CURRENT - 1.5)) * 40;
//  stSensorData.pv2_current            = ((float32)(/* ADC_12BIT_COEFFICIENT * */ADC_PV2_CURRENT - 1.5)) * 40;
//  stSensorData.leak_current           = 0;
//  stSensorData.bat_current            = 0;
	if(!CalibrationFlag)
	{
		stSensorData.temp[6] = (float32)ADC_TEMP;
		if(stSensorData.temp[6] > 2000 && stSensorData.temp[6] < 2100)
		{
			AdcRef = 1.5 / stSensorData.temp[6];
			CalibrationFlag = 1;
		}
	}
	else
	{
		stSensorData.pv1_voltage  			= ((float32)(ADC_PV1_VOLTAGE - stSensorData.temp[6])) * ADC_VOL_COEFFICIENT * AdcRef;// * ADC_12BIT_COEFFICIENT;
		stSensorData.pv2_voltage  			= ((float32)(ADC_PV2_VOLTAGE - stSensorData.temp[6])) * ADC_VOL_COEFFICIENT * AdcRef;// * ADC_12BIT_COEFFICIENT;
		stSensorData.invert_voltage 		= ((float32)(ADC_INVERT_VOLTAGE - ADC_REF)) * ADC_VOL_COEFFICIENT1 * AdcRef;// * ADC_12BIT_COEFFICIENT;
		stSensorData.power_grid_voltage 	= ((float32)(ADC_POWER_GRID_VOLTAGE - ADC_REF)) * ADC_VOL_COEFFICIENT1 * AdcRef;// * ADC_12BIT_COEFFICIENT;
		stSensorData.micro_grid_voltage 	= ((float32)(ADC_MICRO_GRID_VOLTAGE - ADC_REF)) * ADC_VOL_COEFFICIENT1 * AdcRef;// * ADC_12BIT_COEFFICIENT;
		stSensorData.load_voltage 			= ((float32)(ADC_LOAD_VOLTAGE - ADC_REF)) * ADC_VOL_COEFFICIENT1 * AdcRef;// * ADC_12BIT_COEFFICIENT;
		stSensorData.bat_voltage 			= ((float32)(ADC_BAT_VOLTAGE - ADC_REF)) * ADC_VOL_COEFFICIENT2 * AdcRef;// * ADC_12BIT_COEFFICIENT;
		stSensorData.bus_voltage			= ((float32)(ADC_BUS_VOLTAGE - ADC_REF)) * ADC_VOL_COEFFICIENT * AdcRef;// * ADC_12BIT_COEFFICIENT;
		stSensorData.dc_component		 	= ((float32)(ADC_DC_COMPONENT - ADC_REF)) * 941 * AdcRef;//941 * ADC_12BIT_COEFFICIENT;
		stSensorData.transformer_current 	= ((float32)(ADC_TRANSFORMER_CURRENT - ADC_REF)) * 20 * AdcRef;//20 * ADC_12BIT_COEFFICIENT;
		stSensorData.inductor_current		= ((float32)(ADC_INDUCTOR_CURRENT - ADC_REF)) * 80 * AdcRef;//80 * ADC_12BIT_COEFFICIENT;
		stSensorData.power_grid_current		= ((float32)(ADC_POWER_GRID_CURRENT - ADC_REF)) * 20 * AdcRef;//20 * ADC_12BIT_COEFFICIENT;
		stSensorData.micro_grid_current		= ((float32)(ADC_MICRO_GRID_CURRENT - ADC_REF)) * 20 * AdcRef;//20 * ADC_12BIT_COEFFICIENT;
		stSensorData.load_current			= ((float32)(ADC_LOAD_CURRENT - ADC_REF)) * 20 * AdcRef;//20 * ADC_12BIT_COEFFICIENT;
		stSensorData.pv1_current			= ((float32)(ADC_PV1_CURRENT - stSensorData.temp[6])) * 40 * AdcRef;//40 * ADC_12BIT_COEFFICIENT;
		stSensorData.pv2_current			= ((float32)(ADC_PV2_CURRENT - stSensorData.temp[6])) * 40 * AdcRef;//40 * ADC_12BIT_COEFFICIENT;
		stSensorData.leak_current			= 0;
		stSensorData.bat_current			= 0;
	}

}
#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif
