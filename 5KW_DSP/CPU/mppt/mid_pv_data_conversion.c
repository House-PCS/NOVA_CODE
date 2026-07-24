/**
******************************************************************************
* @copyright (C) COPYRIGHT Nova
* @file afe_control.c
* @author Xi'an NovaStar Tech Co.,LTD.
* @version V1.0.0
* @date 2023-8-29
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
/* Includes ------------------------------------------------------------------*/
#include "mid_pv_data_conversion.h"
//#include "dev_sample.h"
//#include "sys_info.h"
#include "mid_pv_interrupt.h"
//#include "dev_ntc.h"
#include "stdio.h"
#include "mid_pv_fault.h"
//#include "dev_powersave.h"

SysState_t       gsysstate;
SysControl_t     gsyscontrol;
SysValue_t       gsysvalue;
Loop_Set         gloopset;
Electric_Calculate_t electric_calculate;
 static void _SysValue_Normalize(SampleRawValue *samp_value);
float IN_Tempture_Get(void) ;
float OUT_Tempture_Get(void); 
uint32_t AP12_Voltage_Get(void);
uint32_t x_aaaaaa = 0;
uint32_t y_aaaaaa = 0;
/*该函数下所有数值信号均放大十倍*/
void _SysValue_Normalize(SampleRawValue *samp_value)        /*转化的值供给报警和上位机使用*/
{


//    gsysvalue.pvmeter.in_singlmeter  = (int16_t)(0);
//    gsysvalue.pvmeter.out_singmeter  = (int16_t)(0);
    gsysvalue.pvmeter.in_totalmeter  = (int32_t)electric_calculate.total_in_electric;;
    gsysvalue.pvmeter.out_totalmeter = (int32_t)electric_calculate.total_out_electric;    
    gsysvalue.mostemp.q1temp = (int16_t)(IN_Tempture_Get() * 10);
    gsysvalue.mostemp.q2temp = (int16_t)(OUT_Tempture_Get() * 10); 
//    gsysvalue.mostemp.q3temp = (int16_t)(OUT_Tempture_Get() * 10);
//    gsysvalue.mostemp.q4temp = (int16_t)(OUT_Tempture_Get() * 10);   
    /*使用的为中断平均后的数据*/       
    gsysvalue.mpptvalue.incurrent  = (int16_t)(((float)samp_value->mppt_in_Iadc_process * ADC_Ref / 0.08f / ADC_MaxValue) * 10);
		if(gsysvalue.mpptvalue.incurrent < 0 && gsysvalue.mpptvalue.incurrent > - 2)
		{
			gsysvalue.mpptvalue.incurrent = 0;
		}
    gsysvalue.mpptvalue.outcurrent = (int16_t)(((float)samp_value->mppt_out_Iadc_process * ADC_Ref / 0.08f / ADC_MaxValue)  * 10);
		/*该电压即为PV电压*/
		if(samp_value->mppt_in_Iadc_process < 99)
		{
		 gsysvalue.mpptvalue.involtage  = (int16_t)((float)samp_value->mppt_in_Uadc_process * ADC_Ref * V_AF / ADC_MaxValue  * 10);  	
		}
		else
		{
			if(samp_value->mppt_in_Uadc_process < 10)
			{
				 gsysvalue.mpptvalue.involtage  = 0;
			}
			else
			{
//				gsysvalue.mpptvalue.involtage  = (int16_t)(((float)samp_value->mppt_in_Uadc_process * ADC_Ref * V_AF / ADC_MaxValue + ((float)gsysvalue.mpptvalue.incurrent / 10) * 0.004f * 103.0f / 3.0f  ) * 10);  
				 gsysvalue.mpptvalue.involtage  = (int16_t)((((float)samp_value->mppt_in_Uadc_process * 0.0271f ) + ((float)samp_value->mppt_in_Iadc_process * 0.0021f ) - 0.4975f) * 10); 
				 if(gsysvalue.mpptvalue.involtage < 0)
				 {
						gsysvalue.mpptvalue.involtage = 0;
				 }
			}
		}
		/*end*/
    gsysvalue.mpptvalue.inpower    = (int16_t)((float)gsysvalue.mpptvalue.involtage * (float)gsysvalue.mpptvalue.incurrent / 10);           
		gsysvalue.mpptvalue.outvoltage = (int16_t)((float)samp_value->mppt_out_Uadc_process * ADC_Ref * V_AF / ADC_MaxValue * 10);                
    gsysvalue.mpptvalue.outpower   = (int16_t)((float)gsysvalue.mpptvalue.outvoltage * (float)gsysvalue.mpptvalue.outcurrent / 10);
		/*该值为瞬时值*/
    //TODO
//    gsysvalue.mpptvalue.start_involtage  =  		(int16_t)(((float)SampleValueGet(EN_MPPT_IN_VOLTAGE_GET) * ADC_Ref * V_AF / ADC_MaxValue + ((float)gsysvalue.mpptvalue.incurrent / 10) * 0.004f * 103.0f / 3.0f  ) * 10);  /*用于起始继电器计算使用*/
//    gsysvalue.elsevoltage.pvin_voltage   = 				(int16_t)(((float)SampleValueGet(EN_PV_VOLTAGE_GET) * ADC_Ref * V_AF / ADC_MaxValue + ((float)gsysvalue.mpptvalue.incurrent / 10) * 0.004f * 103.0f / 3.0f  ) * 10);
    gsysvalue.elsevoltage.dc12v_ap = (int16_t)((float)AP12_Voltage_Get() * (ADC_REFER) * (V_DR2) * 10);    
		
		/*警报值计算*/
    //TODO
//		gloop_setadc.battery_adc.ov_adcvalue = VolConverseADC(MPPT_OUT_OV);
//		gloop_setadc.battery_adc.uv_adcvalue = VolConverseADC(MPPT_OUT_UV);
  #if DC1_ENABLE 
    gsysvalue.dc1value.invaltage  = (int16_t)((float)samp_value->pack_out_voltage_adc_value * (ADC_REFER) * (V_AF) * 10);         
    gsysvalue.dc1value.incurrent  = (int16_t)(((float)samp_value->mppt_out_current_adc_value - I_BIAS_ADC) / I_CM * 10);           
    gsysvalue.dc1value.inpower    = (int16_t)((float)gsysvalue.mpptvalue.outvoltage * (float)gsysvalue.mpptvalue.outcurrent / 10);
    gsysvalue.dc1value.outvaltage = (int16_t)((float)samp_value->dc24v_voltage_adc_value * (ADC_REFER) * (V_DR1) * 10);       
    gsysvalue.dc1value.outcurrent = (int16_t)(((float)samp_value->dc24v_current_adc_value - I_BIAS_ADC) / I_CM * 10);           
    gsysvalue.dc1value.outpower   = (int16_t)((float)gsysvalue.dc1value.outvaltage * (float)gsysvalue.dc1value.outcurrent / 10);
    gsysvalue.dc1value.temp       = (int16_t)(samp_value->dc24v_temp_adc_value);
  #endif	
  
  #if DC2_ENABLE 
    gsysvalue.dc2value.invaltage  = (int16_t)((float)samp_value->pack_out_voltage_adc_value * (ADC_REFER) * (V_AF) * 10);         //30
    gsysvalue.dc2value.incurrent  = (int16_t)(((float)samp_value->mppt_out_current_adc_value - I_BIAS_ADC) / I_CM * 10);           //20
    gsysvalue.dc2value.inpower    = (int16_t)((float)gsysvalue.mpptvalue.outvoltage * (float)gsysvalue.mpptvalue.outcurrent / 10);
    gsysvalue.dc2value.outvaltage = (int16_t)((float)samp_value->dc12v_voltage_adc_value * (ADC_REFER) * (V_DR2) * 10);
    gsysvalue.dc2value.outcurrent = (int16_t)((float)(samp_value->dc12v_current_adc_value-I_BIAS_ADC)/ I_CM * 10);    
    gsysvalue.dc2value.outpower   = (int16_t)((float)gsysvalue.dc2value.outvaltage * (float)gsysvalue.dc2value.outcurrent / 10);
    gsysvalue.dc2value.temp       = (int16_t)(0);
  #endif
  #if DC3_ENABLE 
    gsysvalue.dc3value.invaltage  = (int16_t)(0);
    gsysvalue.dc3value.incurrent  = (int16_t)(0);
    gsysvalue.dc3value.inpower    = (int16_t)(0); 
    gsysvalue.dc3value.outvaltage = (int16_t)(0);
    gsysvalue.dc3value.outcurrent = (int16_t)(0);    
    gsysvalue.dc3value.outpower   = (int16_t)(0);
    gsysvalue.dc3value.temp       = (int16_t)(0);
  #endif
  #if DC4_ENABLE
    gsysvalue.dc4value.invaltage  = (int16_t)(0);
    gsysvalue.dc4value.incurrent  = (int16_t)(0);
    gsysvalue.dc4value.inpower    = (int16_t)(0);  
    gsysvalue.dc4value.outvaltage = (int16_t)(0);
    gsysvalue.dc4value.outcurrent = (int16_t)(0);    
    gsysvalue.dc4value.outpower   = (int16_t)(0);
    gsysvalue.dc4value.temp       = (int16_t)(0);
  #endif
  
}  

int16_t VolConverseADC(int16_t vol)
{
    int16_t value = 0 ;
    value = (int16_t)(((float)vol/10)*3.0f/100.0f*ADC_MaxValue/ADC_Ref);
    return value;
}

int16_t CurrConverseADC(int16_t curr)
{
    int16_t value = 0 ;
    value = (int16_t)(((float)curr/10) * Samp_R * I_AF * ADC_MaxValue / ADC_Ref);
    return value;
}
int16_t SampleDataGet(EN_SAMPLE_GET sta)
{
	int16_t value;
	switch (sta)
	{
		case EN_SAMPLE_GET_IN_VOL:
		{
			value = gsysvalue.mpptvalue.involtage;
			break;
		}
		case EN_SAMPLE_GET_IN_CURR:
		{
			value = gsysvalue.mpptvalue.incurrent; 
			break;
		}
		case EN_SAMPLE_GET_IN_POWER:
		{
			value = gsysvalue.mpptvalue.inpower;
			break;
		}
		case EN_SAMPLE_GET_OUT_VOL:
		{
			value = gsysvalue.mpptvalue.outvoltage ;  
			break;
		}
		case EN_SAMPLE_GET_OUT_CURR:
		{
			value = gsysvalue.mpptvalue.outcurrent ;
			break;
		}
		case EN_SAMPLE_GET_OUT_POWER:
		{
			value = gsysvalue.mpptvalue.outpower;
			break;
		}
		case EN_SAMPLE_GET_IN_TEMP:
		{
			value = gsysvalue.mostemp.q1temp;
			break;
		}
		case EN_SAMPLE_GET_OUT_TEMP:
		{
			value = gsysvalue.mostemp.q2temp;
			break;
		}
				default:break;
	}
	return value;
}



void ChargeStateSet(charge_state sta)
{
	if(sta == STOP_CHARGE)
	{
			gsysstate.chargestate = STOP_CHARGE;                 /*停止充电*/
	}
	else if(sta == CHARGE)
	{
			gsysstate.chargestate = CHARGE;                       /*充电中*/
	}
	else if(sta == END_CHARGE)
	{
			gsysstate.chargestate = END_CHARGE;                 /*充电结束*/
	}

}
void SysStateSet(run_state sta)
{
	if(sta == SHUTDOWN)
	{
			gsysstate.runstate = SHUTDOWN;                 /*停机状态*/
	}
	else if(sta == FAULT_SHUTDOWN)
	{
			gsysstate.runstate = FAULT_SHUTDOWN;           /*故障停机*/
	}
	else if(sta == SOFT_START)
	{
			gsysstate.runstate = SOFT_START;               /*软起动*/
	}
	else if(sta == NORMAL_OPERATION)
	{
			gsysstate.runstate = NORMAL_OPERATION;         /*正常运行*/
	}
}
void ChargeModeSet(charge_mode sta)
{
	if(sta == DC_CHARGE)
	{
			gsysstate.chargemode = DC_CHARGE;                 /*DC充电*/
	}
	else if(sta == PV_CHARGE)
	{
			gsysstate.chargemode = PV_CHARGE;                 /*PV充电*/
	}
	else if(sta == None_InPut)
	{
			gsysstate.chargemode = None_InPut;               /*无充电*/
	}
}


charge_mode GetChargeMode(void)
{
	return gsysstate.chargemode;
}
void SysValue_NormalizeUpdata(void)
{
    _SysValue_Normalize(&gSampleRawValue);
}    

void Normalize_ADC(charge_mode in_souce)        /*转化的值供给环路控制和MPPT控制使用*/
{
		charge_mode sta = in_souce;
		float  charge_current_value = 0;
		float  charge_power_value = 0;
		float  compute_out_current = 0;
		float  compute_in_current = 0;
		float  out_voltage = 0;
		float  in_voltage = 0;
		float  set_current = 0;
		switch (sta)
		{
			case PV_CHARGE:
			{
				charge_current_value = ((float)(gloopset.battery_set.charge_current))/10;           /*取出充电电流*/
#if		 EN_MAX_POWER_CHARGE
			charge_current_value = 20;           /*设定充电电流为1A*/
#endif	
				if(charge_current_value < 0)                              /*限制充电电流为0-20A*/
				{
					charge_current_value = 0;
				}
				else if(charge_current_value > (MPPT_OUT_NOMAL_MAX_CURR / 10))
				{
					charge_current_value = (MPPT_OUT_NOMAL_MAX_CURR / 10);
				}
				charge_power_value   = ((float)(gloopset.battery_set.charge_power))/10;           /*取出充电功率*/
				if(charge_power_value < 0)                              /*限制充电功率为0-800w*/
				{
					charge_power_value = 0;
				}
				else if(charge_power_value > (MPPT_OUT_NOMAL_MAX_POWER / 10))
				{
					charge_power_value = (MPPT_OUT_NOMAL_MAX_POWER / 10);
				}
				if((GetFanState() == TEMP_LEVEL_HIGH )|| (GetFanState() == EN_TEMP_OVER_HIGH) )                       /*若出现温度高限制功率*/
				{
					charge_power_value = (MPPT_HIGE_TEMP_MAX_POWER / 10);
				}                
				out_voltage = ((float)SampleDataGet(EN_SAMPLE_GET_OUT_VOL) / 10);
				compute_out_current = charge_power_value/out_voltage;  					/*取出该功率下充电电流*/
				if(compute_out_current <= charge_current_value)                      /*取出该功率下的电流和设定电流的最小值*/
				{
					set_current = compute_out_current;
				}
				else
				{
					set_current = charge_current_value;
				}
                
				//TODO
//				gloop_setadc.battery_adc.charge_current = CurrConverseADC(set_current * 10);
				TragetValue.input_max_current_ADC = CurrConverseADC(MPPT_PVIN_NOMAL_MAX_CURR ); 
				TragetValue.input_max_power = (MPPT_PVIN_NOMAL_MAX_POWER );
				//TODO
//				TragetValue.output_max_current_ADC = gloop_setadc.battery_adc.charge_current;
				TragetValue.output_max_power = 	(MPPT_OUT_NOMAL_MAX_POWER );
				break;
			}
			case DC_CHARGE:
			{
				charge_current_value = ((float)(gloopset.battery_set.charge_current))/10;           /*取出充电电流*/
#if		 EN_MAX_POWER_CHARGE
			 charge_current_value = 8;           /*设定充电电流为1A*/
#endif	
				if(charge_current_value < 0)                              /*限制充电电流为0-20A*/
				{
					charge_current_value = 0;
				}
				else if(charge_current_value > (MPPT_OUT_NOMAL_MAX_CURR / 10))
				{
					charge_current_value = (MPPT_OUT_NOMAL_MAX_CURR / 10);
				}
				charge_power_value   = ((float)(gloopset.battery_set.charge_power))/10;           /*取出充电功率*/
				if(charge_power_value < 0)                              /*限制充电功率为0-800w*/
				{
					charge_power_value = 0;
				}
				else if(charge_power_value > (MPPT_OUT_NOMAL_MAX_POWER / 10))
				{
					charge_power_value = (MPPT_OUT_NOMAL_MAX_POWER / 10);
				}
				if((GetFanState() == TEMP_LEVEL_HIGH )|| (GetFanState() == EN_TEMP_OVER_HIGH) )                       /*若出现温度高限制功率*/
				{
					charge_power_value = (MPPT_HIGE_TEMP_MAX_POWER / 10);
				}                
				out_voltage = ((float)SampleDataGet(EN_SAMPLE_GET_OUT_VOL) / 10);
				compute_out_current = charge_power_value/out_voltage;  					/*取出该功率下充电电流*/
				if(compute_out_current <= charge_current_value)                      /*取出该功率下的电流和设定电流的最小值*/
				{
					set_current = compute_out_current;
				}
				else
				{
					set_current = charge_current_value;
				}
				compute_in_current = ((float)MPPT_DCIN_NOMAL_MAX_POWER / 10) / out_voltage;           /*计算设定最小输入功率下对应输入电流*/
				if(compute_in_current > set_current)
				{
					set_current = set_current;
				}
				else 
				{
					set_current = compute_in_current;
				}
				//TODO
//				gloop_setadc.battery_adc.charge_current = CurrConverseADC(set_current * 10);
                TragetValue.input_max_current_ADC = CurrConverseADC(MPPT_DCIN_NOMAL_MAX_CURR); 
                TragetValue.input_max_power = (MPPT_DCIN_NOMAL_MAX_POWER );
                //TODO
//                TragetValue.output_max_current_ADC = gloop_setadc.battery_adc.charge_current;
                TragetValue.output_max_power = 	(MPPT_OUT_NOMAL_MAX_POWER);
				break;
			}
			case None_InPut:
			{
                TragetValue.input_max_current_ADC = CurrConverseADC(MPPT_NONEIN_NOMAL_MAX_CURR ); 
                TragetValue.input_max_power = (MPPT_NONEIN_NOMAL_MAX_POWER );
                TragetValue.output_max_current_ADC = CurrConverseADC(MPPT_OUT_NOMAL_MAX_CURR); 
                TragetValue.output_max_power = 	(MPPT_OUT_NOMAL_MAX_POWER);
				break;
			}
			default:
			{
				break;			
			}

		}
}

int16_t SingleInElectric(int16_t in_vol,int16_t in_curr)    /*单次600ms的电量(W)*/
{
	
	int16_t in_current = in_curr;
	int16_t in_voltage = in_vol;
	uint16_t power      = 0;
	if(in_current < 0)             /*负电流即由于参数计算导致*/
	{
		in_current = 0;
	}
	power = in_current * in_voltage;
	
	return (float)power / 100;
}
int16_t SingleOutElectric(int16_t out_vol,int16_t out_curr)
{
	int16_t out_current = out_curr;
	int16_t out_voltage = out_vol;
	int32_t power      = 0;
	if(out_current < 0)             /*负电流即由于参数计算导致*/
	{
		out_current = 0;
	}
	power = out_current * out_voltage;
	return (float)power / 100;
}

bool  MinuteElectric(bool single_calculate_flag)
{
	bool value = false;
	if(single_calculate_flag)
	{
		electric_calculate.minute_cont ++ ;		
		electric_calculate.single_in_electric = SingleInElectric(SampleDataGet(EN_SAMPLE_GET_IN_VOL),SampleDataGet(EN_SAMPLE_GET_IN_CURR));
		electric_calculate.single_out_electric = SingleOutElectric(SampleDataGet(EN_SAMPLE_GET_OUT_VOL),SampleDataGet(EN_SAMPLE_GET_OUT_CURR));		
		electric_calculate.minute_in_electric = electric_calculate.minute_in_electric + electric_calculate.single_in_electric;                /*记录输入电量叠加*/
		electric_calculate.minute_out_electric = electric_calculate.minute_out_electric + electric_calculate.single_out_electric;		          /*记录输出电量叠加*/
		ClearIrqElectricFlag();                /*等待下一次电量计算标志*/			
	}
	if(electric_calculate.minute_cont == 58)                   /*记录每分钟的电量*/
	{
		electric_calculate.half_hour_in_electric = electric_calculate.minute_in_electric + electric_calculate.half_hour_in_electric;    
		electric_calculate.half_hour_out_electric = electric_calculate.minute_out_electric + electric_calculate.half_hour_out_electric;
		electric_calculate.minute_in_electric = 0;
		electric_calculate.minute_out_electric = 0;
		electric_calculate.minute_cont = 0;                       /*记录到电量后清0*/
		electric_calculate.half_hour_cont ++;
	}
	if(electric_calculate.half_hour_cont == 30)                 /*记录每小时的电量 KW/H(0.001/bit)*/
	{
		electric_calculate.hour_in_electric = electric_calculate.half_hour_in_electric / 2;
		electric_calculate.hour_out_electric = electric_calculate.half_hour_out_electric / 2;
		value = true;
	}
	return value;
}

void ElectricUpdata(void)                                     /*电量更新*/
{
		 electric_calculate.total_in_electric = electric_calculate.total_in_electric + electric_calculate.hour_in_electric;    /*记录输入总电量*/
		 electric_calculate.total_out_electric = electric_calculate.total_out_electric + electric_calculate.hour_out_electric;	/*记录输出总电量*/                                                                                           /*清除半个小时记录电量标志*/
		
		/*更新给BMS中*/
		 gsysvalue.pvmeter.in_singlmeter = electric_calculate.hour_in_electric;           /*传递至输入电量*/
		 gsysvalue.pvmeter.out_singmeter = electric_calculate.hour_out_electric;           /*传递至输入电量*/		
		 gsysvalue.pvmeter.in_totalmeter = electric_calculate.total_in_electric;           /*传递至输入总电量*/
		 gsysvalue.pvmeter.out_totalmeter = electric_calculate.total_out_electric;           /*传递至输出总电量*/		
	
		 electric_calculate.half_hour_in_electric = 0;
		 electric_calculate.half_hour_out_electric = 0;
		 electric_calculate.half_hour_cont = 0;		
}
void PowerDataWRInfo(bool value)
{
	bool sta = value;
	if(sta)
	{
		//TODO
//		PowerDataSave(&electric_calculate,sizeof(electric_calculate));
		gsysvalue.pvmeter.in_singlmeter  = (int16_t)(0);           /*将本次电量清0*/
		gsysvalue.pvmeter.out_singmeter  = (int16_t)(0);
	}
	else
	{
		//TODO
//		PowerDataRead(&electric_calculate,sizeof(electric_calculate));    /*取出电量信息*/
		gsysvalue.pvmeter.in_singlmeter  = (int16_t)(0);                  /*重新计算单次变量*/
		gsysvalue.pvmeter.out_singmeter  = (int16_t)(0);
	}

}
float IN_Tempture_Get(void)       
{
    static uint32_t MPPTINTemp_ADC = 0; 
		static uint32_t in_tempture_time      = 0;
		static uint32_t in_tempture_cnt      = 0;
		static float mppt_intemp = 25;
		if(TimDifftimeFromNow(in_tempture_time) > 5)
		{
			in_tempture_time = TimGetSystime();
			//TODO
//			MPPTINTemp_ADC = MPPTINTemp_ADC + SampleValueGet(EN_MPPT_IN_TEMP_GET);
				in_tempture_cnt ++;
				if(in_tempture_cnt >= 20)      
				{
            MPPTINTemp_ADC = (MPPTINTemp_ADC/in_tempture_cnt);
            //TODO
//            mppt_intemp = TemperatureGet((int16_t)MPPTINTemp_ADC);
						in_tempture_cnt = 0;					
            MPPTINTemp_ADC =0 ;					
				}  
		}
		return mppt_intemp;
}

float OUT_Tempture_Get(void)       
{
    static uint32_t MPPTOUTTemp_ADC = 0; 
		static uint32_t out_tempture_time      = 0;
		static uint32_t out_tempture_cnt      = 0;
		static float mppt_out_temp = 25;
		if(TimDifftimeFromNow(out_tempture_time) > 5)
		{
			out_tempture_time = TimGetSystime();
			//TODO
//			MPPTOUTTemp_ADC = MPPTOUTTemp_ADC + SampleValueGet(EN_MPPT_OUT_TEMP_GET);
				out_tempture_cnt ++;
				if(out_tempture_cnt >= 20)       
				{
            MPPTOUTTemp_ADC = (MPPTOUTTemp_ADC/out_tempture_cnt);
            //TODO
//            mppt_out_temp = TemperatureGet((int16_t)MPPTOUTTemp_ADC);
						out_tempture_cnt = 0;					
            MPPTOUTTemp_ADC =0 ;					
				}  
		}
		return mppt_out_temp;
}

uint32_t AP12_Voltage_Get(void)       
{
    static uint32_t AP12_ADC_Value = 0; 
		static uint32_t AP12_time      = 0;
		static uint32_t AP12_cnt      = 0;
		static uint32_t AP12_out_value = 12;
		if(TimDifftimeFromNow(AP12_time) > 2)
		{
			AP12_time = TimGetSystime();
			//TODO
//			AP12_ADC_Value = AP12_ADC_Value + SampleValueGet(EN_AP12_VOLTAGE_GET);
				AP12_cnt ++;
				if(AP12_cnt >= 20)      
				{
            AP12_ADC_Value = (AP12_ADC_Value/AP12_cnt);
            AP12_out_value = AP12_ADC_Value;
						AP12_cnt = 0;					
            AP12_ADC_Value =0 ;					
				}  
		}
		return AP12_out_value;
}





