/**
******************************************************************************
* @copyright (C) COPYRIGHT Nova
* @file afe_control.h
* @author Xi'an NovaStar Tech Co.,LTD.
* @version V1.0.0
* @date 2022-8-30
* @brief
******************************************************************************
* @attention
* @relation
* @par :(修改日志)
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* </table>
******************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MID_PV_DATA_CONVERSION_H
#define __MID_PV_DATA_CONVERSION_H
#ifdef __cplusplus
extern "C" {
#endif
/* Inculdes ------------------------------------------------------------------*/
//#include "bsp.h"
#include "F28x_Project.h"
#ifdef __cplusplus
}
#endif

#define  ADC_MaxValue       4095.0f
#define  ADC_Ref            3.303f
#define  ADC_REFER          ADC_Ref/ADC_MaxValue                  /*ADC的参考电压与分辨率比值*/
#define  Samp_R             0.002f                                /*采样电阻值*/
#define  I_AF               40.0f                                 /*电流放大器放大倍数*/
#define  I_BIAS             0.3f                                  /*直流偏置电压*/
#define  I_BIAS_ADC         372.f                               /*电流采样直流偏置电压(0.3V)，与中断文件参数一致*/
#define  I_CM               99.297F                              /*根据公式计算Samp_R*I_AF*ADC_MaxValue/ADC_Ref*/ 
#define  V_AF               100.0f/3.0f                           /*电压采样放大倍数100/3*/
#define  V_DR               100/3                                 /*采样电压分压比1*/
#define  V_DR1              100/10                                /*采样电压分压比2*/
#define  V_DR2              12/2                                  /*采样电压分压比3*/
typedef enum 
{   
    
     SHUTDOWN = 0,        /*停机*/
     FAULT_SHUTDOWN,      /*故障停机*/
     SOFT_START,          /*软起动*/
     NORMAL_OPERATION    /*正常运行*/
    
}run_state;

typedef enum 
{   
    
     STOP_CHARGE = 0,        /*停止充电*/
     CHARGE,                 /*充电中*/
     END_CHARGE,             /*充满*/
    
}charge_state;

typedef enum 
{   
     DC_CHARGE = 0,             /*DC充电*/
     PV_CHARGE,                   /*PV充电*/
     None_InPut,                     /*无输入*/  
}charge_mode;

typedef struct Mppt_state_stru_t
{
    
    run_state                runstate;                     /* 系统运行状态  */
    charge_state             chargestate;                  /* 充电运行状态  */
    charge_mode              chargemode;                   /* 充电模式状态 */
    
}SysState_t;

typedef enum 
{   
    
     POWER_OFF = 0,              /*系统关机*/
     POWER_ON,                   /*系统开机*/
    
}power_control;

typedef enum 
{   
    
     DC_OFF = 0,                 /*DC停止运行*/
     DC_ON,                      /*DC开启*/
    
}switch_dc;

typedef struct Mppt_control_stru_t
{
    
    power_control               power;                   /* 开关机控制  */
    switch_dc                   dc1;                     /* DC1控制  */
    switch_dc                   dc2;                     /* DC2控制 */
    switch_dc                   dc3;                     /* DC3控制 */
    switch_dc                   dc4;                     /* DC4控制 */
    
}SysControl_t;

typedef struct Mppt_value_stru_t
{
    
    int16_t       involtage ;
		int16_t       start_involtage ;                     /*用于起始继电器开关使用*/
    int16_t       incurrent ;
    int16_t       inpower ;
    int16_t       outvoltage ;
    int16_t       outcurrent ;
    int16_t       outpower ;
    
}mppt_value;

typedef struct Mos_value_stru_t
{
    
    int16_t       q1temp;
    int16_t       q2temp;
    int16_t       q3temp;
    int16_t       q4temp;
    
}mos_temp;

typedef struct PV_meter_stru_t
{
    
    int16_t       in_singlmeter;
    int16_t       out_singmeter;      
    uint32_t      in_totalmeter;
    uint32_t      out_totalmeter;
    
}pv_meter;

typedef struct DC_value_stru_t
{
    int16_t       invaltage;
    int16_t       incurrent; 
    int16_t       inpower;    
    int16_t       outvaltage;
    int16_t       outcurrent; 
    int16_t       outpower; 
    int16_t       temp;
}dc_value;

typedef struct Else_Voltage_stru_t
{
    
    int16_t     pvin_voltage;
    int16_t     dc12v_ap;

}else_voltage;

typedef struct Sys_paramvalue_stru_t
{ 
    mppt_value       mpptvalue;           /* MPPT电路信息  */
    mos_temp         mostemp;             /* MOS温度  */
    pv_meter         pvmeter;             /* PV电量计量  */
    dc_value         dc1value;            /* dc1电路信息  */
    dc_value         dc2value;            /* dc2电路信息  */
    dc_value         dc3value;            /* dc3电路信息  */
    dc_value         dc4value;            /* dc4电路信息  */  
    else_voltage     elsevoltage;
}SysValue_t;

typedef struct DC_Value_stru_t
{ 
    
    int16_t       dc1_voltage;
    int16_t       dc1_current; 
    int16_t       dc2_voltage;
    int16_t       dc2_current; 
    int16_t       dc3_voltage;
    int16_t       dc3_current; 
    int16_t       dc4_voltage;
    int16_t       dc4_current; 
    
}dc_set;

typedef struct Cell_Value_stru_t
{ 
    
    int16_t       ov_value;
    int16_t       uv_value; 
    int16_t       charge_current;
    int16_t       charge_power;    
    
}Battery_set;

typedef struct Loop_Set_stru_t
{ 
    dc_set            dc_set;
    Battery_set       battery_set; 
}Loop_Set;


typedef enum 
{   
  EN_SAMPLE_GET_IN_VOL = 0 ,
	EN_SAMPLE_GET_IN_CURR ,
	EN_SAMPLE_GET_IN_POWER ,
	EN_SAMPLE_GET_OUT_VOL ,
	EN_SAMPLE_GET_OUT_CURR ,
	EN_SAMPLE_GET_OUT_POWER ,
	EN_SAMPLE_GET_IN_TEMP ,
	EN_SAMPLE_GET_OUT_TEMP 
}EN_SAMPLE_GET;


typedef struct{
	uint16_t minute_cont;                 /*记录每分钟的次数*/
	uint16_t half_hour_cont;              /*记录到半小时的次数*/
	int32_t  minute_in_electric;          /*记录每分钟的输入电量W*/
	int32_t  minute_out_electric;         /*记录每分钟的输出电量W*/
	int16_t  single_in_electric;          /*记录单次的输入电量W*/
	int16_t  single_out_electric;         /*记录单次的输出电量W*/
	int64_t  half_hour_in_electric;       /*记录半小时的输入电量W*/
	int64_t  half_hour_out_electric;      /*记录半小时的输出电量W*/
	int32_t  hour_in_electric;            /*记录1小时的输入电量KW/H (0.001/bit)*/
	int32_t  hour_out_electric;           /*记录1小时的输出电量KW/H (0.001/bit)*/
	int64_t  total_in_electric;         	/*记录输入总电量KW/H*/
	int64_t  total_out_electric;          /*记录输出总电量KW/H*/	
}Electric_Calculate_t;

typedef struct _sample_raw_stru_t
{
		 int16_t pv_in_Uadc_process;
		 int16_t mppt_in_Uadc_process;
		 int16_t mppt_in_Iadc_process;
		 int16_t mppt_out_Uadc_process;
		 int16_t mppt_out_Iadc_process;
}SampleRawValue;

typedef enum
{
	TEMP_LEVEL_STOP = 0,
	TEMP_LEVEL_LOW,
	TEMP_LEVEL_MID,
	TEMP_LEVEL_HIGH,
	TEMP_LEVEL_OVER_HIGH,
	TEMP_LEVEL_GET_ERROR,                       /*堵转*/
	TEMP_LEVEL_NUM
}EN_TEMP_LEVEL;

extern SysState_t       gsysstate;
extern SysControl_t     gsyscontrol;
extern SysValue_t       gsysvalue;
extern Loop_Set         gloopset;
extern Electric_Calculate_t electric_calculate;


void SysValue_NormalizeUpdata(void); 
void Normalize_ADC(charge_mode in_souce) ; 
int16_t SampleDataGet(EN_SAMPLE_GET sta);
bool  MinuteElectric(bool single_calculate_flag);
void ElectricUpdata(void) ;
void SysStateSet(run_state sta);
void ChargeModeSet(charge_mode sta);
charge_mode GetChargeMode(void);
void ChargeStateSet(charge_state sta);

void PowerDataWRInfo(bool value);

int16_t VolConverseADC(int16_t vol);
int16_t CurrConverseADC(int16_t vol);
#endif /* __AFE_CONTROL_H */
/************************ (C) COPYRIGHT Nova *************END OF FILE**********/ 
                
