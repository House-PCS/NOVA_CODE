/**
******************************************************************************
* @copyright (C) COPYRIGHT Nova
* @file sys_info.c
* @author Xi'an NovaStar Tech Co.,LTD.
* @version V1.0.0
* @date 2023-8-28
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
#include "mid_pv_logic_control.h"
#include "stdio.h"
#include "mid_position_pid.h"
//#include "dev_misc.h"
#include "mid_pv_fault.h"
//#include "dev_powersave.h"
/* Global Variable -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
EN_CHARGE_LOGIC charge_logic;
Loop_Traget_Value_t TragetValue;
/**************输入输出电压阈值***********************/
#define IN_VOL_MIN 		(50)        /*输入电压存在阈值*/
#define OUT_VOL_MIN 	(210)       /*输出电压存在阈值*/

#define SET_CURR_LIMIT (2)      /*该参数由上位机传达，默认上电比该值大于0.2A才会充电*/
#define IN_CURR_LIMIT  (2)      /*输入电流阈值*/

#define BATTERY_CURR_EXISTENCE_THRESHOLD (2)     /*0.2A*/
#define IN_CURR_EXISTENCE_THRESHOLD (20)     /*2A*/

#define CHARGE_END_CURR (2)     /*主机电流设为0.2A时标志充电结束*/

#define PV_MPPT_VOL_DIFF (30)   /*运行时警报压差及开继电器压差*/
#define PV_COVER_STOP_JUDGE_VOL (30)   /*PV光伏板遮挡及移除条件的压差*/
/* Private function  -----------------------------------------------*/
EN_CHARGE_STATE ChargeState(charge_mode chargemode,int16_t in_actual_current,int16_t set_current);
EN_POWER_TYPE PowerSouceJudge(void);
EN_RUN_LOJIC Mppt_Run(bool out_flag ,charge_mode souce_type );

void ChargeParamInit(charge_mode souce_type)
{
	charge_mode sta = souce_type;
	switch (sta)
	{
		case PV_CHARGE:
		{
			/*该值为真实值*10*/
			gloopset.battery_set.charge_current = MPPT_OUT_NOMAL_MAX_CURR;                    		
			gloopset.battery_set.ov_value = MPPT_OUT_OV; 
			gloopset.battery_set.uv_value = MPPT_OUT_UV;     
			gloopset.battery_set.charge_power = MPPT_OUT_NOMAL_MAX_POWER;
			/*该值为ADC值，即环路使用值*/
			//TODO
//			gloop_setadc.battery_adc.charge_current = CurrConverseADC(MPPT_OUT_NOMAL_MAX_CURR);	           /*以最大电流进行充电*/
//			gloop_setadc.battery_adc.ov_adcvalue = VolConverseADC(MPPT_OUT_OV);
//			gloop_setadc.battery_adc.uv_adcvalue = VolConverseADC(MPPT_OUT_UV);
			TragetValue.input_max_current_ADC = CurrConverseADC(MPPT_PVIN_INIT_CURR);	                       /*以设置初始值进行充电*/
			TragetValue.input_max_power = MPPT_PVIN_NOMAL_MAX_POWER;
			TragetValue.output_max_current_ADC = CurrConverseADC(MPPT_OUT_NOMAL_MAX_CURR);
			TragetValue.output_max_power = MPPT_OUT_NOMAL_MAX_POWER;
			break;
		}
		case DC_CHARGE:
		{
			/*该值为真实值*10*/
			gloopset.battery_set.charge_current = MPPT_OUT_NOMAL_MAX_CURR;                    		
			gloopset.battery_set.ov_value = MPPT_OUT_OV; 
			gloopset.battery_set.uv_value = MPPT_OUT_UV;     
			gloopset.battery_set.charge_power = MPPT_OUT_NOMAL_MAX_POWER;
			/*该值为ADC值，即环路使用值*/
			//TODO
//			gloop_setadc.battery_adc.charge_current = CurrConverseADC(MPPT_OUT_NOMAL_MAX_CURR);	           /*以最大电流进行充电*/
//			gloop_setadc.battery_adc.ov_adcvalue = VolConverseADC(MPPT_OUT_OV);
//			gloop_setadc.battery_adc.uv_adcvalue = VolConverseADC(MPPT_OUT_UV);
			TragetValue.input_max_current_ADC = CurrConverseADC(MPPT_DCIN_INIT_CURR);	                       /*以设置初始值进行充电*/
			TragetValue.input_max_power = MPPT_DCIN_NOMAL_MAX_POWER;
			TragetValue.output_max_current_ADC = CurrConverseADC(MPPT_OUT_NOMAL_MAX_CURR);
			TragetValue.output_max_power = MPPT_OUT_NOMAL_MAX_POWER;
			break;
		}
		case None_InPut:
		{
			/*该值为真实值*10*/
			gloopset.battery_set.charge_current = MPPT_OUT_NOMAL_MAX_CURR;                    		
			gloopset.battery_set.ov_value = MPPT_OUT_OV; 
			gloopset.battery_set.uv_value = MPPT_OUT_UV;     
			gloopset.battery_set.charge_power = MPPT_OUT_NOMAL_MAX_POWER;
			/*该值为ADC值，即环路使用值*/
			//TODO
//			gloop_setadc.battery_adc.charge_current = CurrConverseADC(MPPT_OUT_NOMAL_MAX_CURR);	           /*以最大电流进行充电*/
//			gloop_setadc.battery_adc.ov_adcvalue = VolConverseADC(MPPT_OUT_OV);
//			gloop_setadc.battery_adc.uv_adcvalue = VolConverseADC(MPPT_OUT_UV);
			TragetValue.input_max_current_ADC = CurrConverseADC(MPPT_NONEIN_INIT_CURR);	                       /*以设置初始值进行充电*/
			TragetValue.input_max_power = MPPT_NONEIN_NOMAL_MAX_POWER;
			TragetValue.output_max_current_ADC = CurrConverseADC(MPPT_OUT_NOMAL_MAX_CURR);
			TragetValue.output_max_power = MPPT_OUT_NOMAL_MAX_POWER;
			break;
		}
		default :break;
	}
}

int32_t DutyCalculate(int16_t in_adc, int16_t out_adc)
{
	float in_voltage = 0;
	float out_voltage = 0;
	float F_duty = 0;
	int32_t duty = 0;
	in_voltage = ((float)in_adc * (ADC_REFER) * (V_AF));
	out_voltage = ((float)out_adc * (ADC_REFER) * (V_AF));

	if((in_voltage + out_voltage) >= 0.001f) // 20260429，优化代码
	{
	    F_duty = out_voltage / (in_voltage + out_voltage);
	}
	else
	{
	    F_duty = 0;
	}

	duty = F_duty * 1200;
	return duty;
}

EN_POWER_TYPE PowerSouceJudge(void)
{
	EN_POWER_TYPE sta = EN_POWER_TYPE_NONE;

	if(PVSingalTypeJudge())
	{
		sta = EN_POWER_TYPE_PV;
	}
	else
	{
		sta = EN_POWER_TYPE_NONE;
	}
	return sta;
}


bool RelayControl(bool flag)
{
	uint8_t i = 0;
	bool relag_flag = false;
	if(flag == true)
	{
		while(1)                                                                        /*当MPPT电压与PV电压一致时打开继电器*/                                                 
		{
			SysValue_NormalizeUpdata();
			if(abs(gsysvalue.elsevoltage.pvin_voltage-gsysvalue.mpptvalue.start_involtage) <= PV_MPPT_VOL_DIFF) /*在3V范围内，*/ // 20260429，优化代码
			{
				MpptRelayPull();
				relag_flag = true;
				break;
			}
			i++;
			DELAY_MS(10);
			if(i == 50)
			{
				relag_flag = false;
				break;
			}
		}
	}
	else 
	{
		MpptRelayBreak();
	}
	return relag_flag;
}

EN_CHARGE_STATE ChargeState(charge_mode chargemode,int16_t in_actual_current,int16_t set_current)        
{
	EN_CHARGE_STATE charge_state = EN_CHARGE;
	static bool pv_signal_flag = false ;
	static uint32_t state_change_time   = 0;
	static uint16_t state_change_cont = 0;

	static uint16_t delay_cont = 0;	

	float 	set_curr = set_current;
	float 	in_curr = in_actual_current ;
	static bool en_state = false; 
	static int16_t in_vol[2] = {0};
	static uint32_t pv_judge_time = 0;
	static uint32_t pv_judge_cont = 0;

	switch (chargemode)
	{
		case PV_CHARGE:
		{
			/*第一种情况，直接检测信号线*/
			if(pv_signal_flag == false)
			{
				if(TimDifftimeFromNow(pv_judge_time) > 10)
				{
					pv_judge_time = TimGetSystime();
					if(PvOrEvEstimate() != true)
					{
						pv_judge_cont++;
					}
					else
					{
						pv_judge_cont = 0;
					}
					if(pv_judge_cont >= 10)
					{
						pv_signal_flag = true;
						pv_judge_cont = 0;
					}
				}
			}
			else if(pv_signal_flag == true)
			{
				if(TimDifftimeFromNow(pv_judge_time) > 10)
				{
					pv_judge_time = TimGetSystime();
					if(PvOrEvEstimate() == true)
					{
						pv_judge_cont++;
					}
					else
					{
						pv_judge_cont = 0;
					}
					if(pv_judge_cont >= 20)
					{
						pv_signal_flag = false;
						charge_state = EN_CHARGE_STOP;     /*通过信号线判断输入源被把，停止充电*/
						pv_judge_cont = 0;
						state_change_cont = 0;
						delay_cont= 0;
						en_state = false;
						return charge_state;
					}
				}
			}
			/*第二种情况检测输入电流*/
			if(TimDifftimeFromNow(state_change_time) > 200)
			{
				state_change_time = TimGetSystime();

				if(en_state)
				{
					delay_cont ++ ;
					if(delay_cont == 3)
					{
						en_state = false;
						delay_cont = 0;
						in_vol[1] = SampleDataGet(EN_SAMPLE_GET_IN_VOL);
						if((in_vol[0] - in_vol[1]) > PV_COVER_STOP_JUDGE_VOL)
						{
							charge_state = EN_CHARGE_STOP;     /*输入源被拔停止充电*/
						}
						else                                  /*阴天情况或被遮挡*/
						{
							charge_state = EN_CHARGE_COVER;
						}
						memset(&in_vol,0,sizeof(in_vol));

					}
					break;
				}
				if(( in_curr < IN_CURR_LIMIT && set_curr > IN_CURR_LIMIT) || set_curr < CHARGE_END_CURR)      /*当输入电流小,设定大,表明被拔;当设定电流小于阈值说明充电结束*/
				{
					state_change_cont++;
				}
				else
				{
					state_change_cont = 0;
					charge_state = EN_CHARGE;     /*正在充电*/
				}
				if(state_change_cont >= 12)                                    /*检测超过2.4S*/
				{
					if(set_curr < CHARGE_END_CURR)
					{
						charge_state = EN_CHARGE_END;     /*充满电停止充电*/
					}
					else
					{
						en_state = true;
						in_vol[0] = SampleDataGet(EN_SAMPLE_GET_IN_VOL);
						//TODO
						//							_PWMOutputEnable(DISABLE);                                  /*失能PWM*/
					}
					state_change_cont = 0;
				}
			}
			break;
		}
		case DC_CHARGE:
		{
			/*DC直接检测充电电流*/
			if(TimDifftimeFromNow(state_change_time) > 200)
			{
				state_change_time = TimGetSystime();
				if((in_curr < IN_CURR_LIMIT && set_curr > IN_CURR_LIMIT) || set_curr < CHARGE_END_CURR)      /*当输入电流小,设定大,表明被拔;当设定电流小于阈值说明充电结束*/
				{
					state_change_cont++;
				}
				else
				{
					state_change_cont = 0;
					charge_state = EN_CHARGE;     /*正在充电*/
				}
				if(state_change_cont >= 6)
				{
					state_change_cont = 0;
					charge_state = EN_CHARGE_STOP;     /*输入源被拔*/
					if(set_curr < CHARGE_END_CURR)
					{
						charge_state = EN_CHARGE_END;     /*充满*/
					}
				}
			}
			break;
		}
		case None_InPut:
		{
			if(TimDifftimeFromNow(state_change_time) > 200)
			{
				state_change_time = TimGetSystime();
				if(en_state)
				{
					delay_cont ++ ;
					if(delay_cont == 4)
					{
						en_state = false;
						delay_cont = 0;
						in_vol[1] = SampleDataGet(EN_SAMPLE_GET_IN_VOL);

						charge_state = EN_CHARGE_STOP;     /*输入源被拔停止充电*/ // 20260429，优化代码
//						if((in_vol[0] - in_vol[1]) > 25)
//						{
//							charge_state = EN_CHARGE_STOP;     /*输入源被拔停止充电*/
//						}
//						else                                  /*阴天情况或被遮挡*/
//						{
//							//									charge_state = EN_CHARGE_COVER;
//							charge_state = EN_CHARGE_STOP;     /*输入源被拔停止充电*/
//						}
						memset(&in_vol,0,sizeof(in_vol));
					}
					break;
				}
				if(( in_curr < IN_CURR_LIMIT && set_curr > IN_CURR_LIMIT) || set_curr < CHARGE_END_CURR)      /*当输入电流小,设定大,表明被拔;当设定电流小于阈值说明充电结束*/
				{
					state_change_cont++;
				}
				else
				{
					state_change_cont = 0;
					charge_state = EN_CHARGE;     /*正在充电*/
				}
				if(state_change_cont >= 12)                                    /*检测超过2.4S*/
				{
					if(set_curr < CHARGE_END_CURR)
					{
						charge_state = EN_CHARGE_END;     /*充满电停止充电*/
					}
					else
					{
						en_state = true;
						in_vol[0] = SampleDataGet(EN_SAMPLE_GET_IN_VOL);
						//TODO
						//							_PWMOutputEnable(DISABLE);                                  /*失能PWM*/
					}
					state_change_cont = 0;
				}
			}
			break;
		}
		default:break;
	}
	return charge_state;
}
/*****************************外部中断停止MPPT运行***********************************/
bool MPPT_STOP_RUN = false;
bool MPPT_Run_State = false;
void MPPTOutCtrl(void)
{
    if(GetMpptOutCtrlState())
    {
    	//TODO
//		_PWMOutputEnable(DISABLE);
        MPPT_STOP_RUN = true;
        MPPT_Run_State = true;
    }
    else if(!GetMpptOutCtrlState())
    {
        MPPT_Run_State = false;
    }
}
void MPPTRunJudge(void)
{
    static uint32_t mppt_run_judge_time = 0;
    static uint32_t mppt_run_judge_cont = 0;
    if(TimDifftimeFromNow(mppt_run_judge_time) > 50)
    {
        mppt_run_judge_time = TimGetSystime();
        if(MPPT_Run_State == false)
        {
            if(!GetMpptOutCtrlState())
            {
                mppt_run_judge_cont ++ ;
            }
			else
			{
				mppt_run_judge_cont = 0;
			}
        }
    }

	if(mppt_run_judge_cont >= 28)
	{
		mppt_run_judge_cont = 0;
		MPPT_STOP_RUN = false;
	}
}
/****返回为true则停止运行****/
bool GetMpptStopRunFlag(void)
{
    return MPPT_STOP_RUN;
}
bool mppt_run_init_flag = false;

/*****************************使用单MPPT环***********************************/
void gNewMPPTRunControl(void)
{
	static EN_LOJIC_STALE  sta                = EN_IDLE;    
	static float           mppt_involtage[3]  = {0}    ;
	static float           mppt_outvoltage[3] = {0}    ;
	static float           mppt_intemp[3]  = {0}    ;
	static float           mppt_outtemp[3] = {0}    ;
	static uint32_t        voltage_get_time   = 0      ;
	static uint32_t        voltage_get_cont   = 0      ;
	static bool out_judge = false;
	switch (sta)
	{
		case EN_IDLE:
		{
			/*输入输出电压记录*/
			if(TimDifftimeFromNow(voltage_get_time) > 1000)
			{
                voltage_get_time   = TimGetSystime();
                voltage_get_cont ++;
                mppt_involtage[0]  = mppt_involtage[1];
                mppt_involtage[1]  = mppt_involtage[2];		

                mppt_outvoltage[0] = mppt_outvoltage[1];
                mppt_outvoltage[1] = mppt_outvoltage[2];					
                mppt_involtage[2]  = SampleDataGet(EN_SAMPLE_GET_IN_VOL) ;	
                mppt_outvoltage[2] = SampleDataGet(EN_SAMPLE_GET_OUT_VOL);	
			}
            if(voltage_get_cont >= 3)
            {
                voltage_get_cont = 0;
                if(mppt_involtage[0] > IN_VOL_MIN && mppt_involtage[1] > IN_VOL_MIN && mppt_involtage[2] > IN_VOL_MIN )
                {
                    if(mppt_involtage[2] < MPPT_IN_UV)
                    {
                        gMpptErrorInfo.mppt_error1_code.b.mppt_pv_uv_flag = 1;    /*欠压报警置位*/
                        sta = EN_HARD_FAULT;                                           
                        return;
                    }
                    else if(mppt_involtage[2] > MPPT_IN_OV)
                    {
                        gMpptErrorInfo.mppt_error1_code.b.mppt_pv_ov_flag = 1;    /*过压报警置位*/
                        sta = EN_HARD_FAULT;  
                        return;
                    }
                    sta = EN_OUTPUT_JUDGE;					
                }                
            }
            if(GetMpptStopRunFlag())
            {
                sta = EN_IDLE;
                memset(&mppt_involtage,0,sizeof(mppt_involtage));  
                memset(&mppt_outvoltage,0,sizeof(mppt_outvoltage)); 
            }
			ChargeModeSet(None_InPut);   /*无输入*/
			break;
		}

		case EN_OUTPUT_JUDGE:
		{			/*输出判断*/
			if(mppt_outvoltage[2] < OUT_VOL_MIN && mppt_outvoltage[1] < OUT_VOL_MIN)
			{
				TragetValue.target_voltage_single_loop = (int16_t)((float)55 * 3.0f / 100.0f * ADC_MaxValue / ADC_Ref);
				out_judge = false;
				sta = EN_fIRST_ALARM_JUDGE;
			}
			else if(mppt_outvoltage[2] >= OUT_VOL_MIN && mppt_outvoltage[1] >= OUT_VOL_MIN)                                                 
			{ 
				if(mppt_outvoltage[2] < MPPT_OUT_UV || mppt_outvoltage[1] < MPPT_OUT_UV)
				{
					gMpptErrorInfo.mppt_error1_code.b.mppt_out_uv_flag = 1;    /*欠压报警置位*/
					sta = EN_HARD_FAULT;
					return;
				}
				else if(mppt_outvoltage[2] > MPPT_OUT_OV || mppt_outvoltage[1] > MPPT_OUT_OV)
				{
					gMpptErrorInfo.mppt_error1_code.b.mppt_out_ov_flag = 1;    /*过压报警置位*/
					sta = EN_HARD_FAULT;
					return;
				}
				else if((mppt_outvoltage[2] <= MPPT_OUT_OV && mppt_outvoltage[2] >= MPPT_OUT_UV) && (mppt_outvoltage[1] <= MPPT_OUT_OV && mppt_outvoltage[1] >= MPPT_OUT_UV))
				{
					sta = EN_fIRST_ALARM_JUDGE;
				}
				TragetValue.target_voltage_single_loop = (int16_t)((float)(gloopset.battery_set.ov_value / 10 + 1) * 3.0f / 100.0f * ADC_MaxValue / ADC_Ref);				
				out_judge = true;
			}
			else 
			{
				sta = EN_IDLE;
			}
			break;
		}
		case EN_fIRST_ALARM_JUDGE:       
		{
			if(GetFanState() == EN_FAN_ERROR)   														      /*风扇堵转故障*/
			{
				gMpptErrorInfo.mppt_error2_code.b.mppt_fan_blockage_flag = 1;     /*堵转故障置位*/
				sta = EN_HARD_FAULT;                                             
			}
			if(GetFanState() == EN_TEMP_OVER_HIGH)                                /*过温故障*/
			{
				sta = EN_HARD_FAULT; 
			}
			if( GetFanState() != EN_FAN_ERROR && GetFanState() != EN_TEMP_OVER_HIGH)  /*都不存在问题，会继续往下运行*/		
			{
				sta = EN_INPUT_TYPE_JUDGE;                                        		
			}	
		}
		case EN_INPUT_TYPE_JUDGE:
		{
            static uint32_t input_souce_judge_time = 0;
            static uint8_t pv_souce_judge_cont  = 0;
            static uint8_t input_souce_judge_total_cont  = 0;            
			if(TimDifftimeFromNow(input_souce_judge_time) > 5)
			{
                input_souce_judge_time  = TimGetSystime();
                input_souce_judge_total_cont ++;
                if(PowerSouceJudge() == EN_POWER_TYPE_PV)
                {
                    pv_souce_judge_cont ++ ;
                }
			}
			if(input_souce_judge_total_cont >= 20)
			{
				input_souce_judge_total_cont = 0;
				if(pv_souce_judge_cont >= 17)
				{
					ChargeModeSet(PV_CHARGE);                  /*PV输入*/
				}
				else
				{
					ChargeModeSet(None_InPut);                  /*无输入*/
				}
				pv_souce_judge_cont = 0;
				PowerDataWRInfo(false);                                     /*读取电量信息*/
				mppt_run_init_flag = true;
				sta = EN_MPPT_RUN;
			}
			break;
		}
		case EN_MPPT_RUN:
		{
			EN_RUN_LOJIC RunSta = Mppt_Run(out_judge ,GetChargeMode());
			if(RunSta == EN_RUN_MPPTALARM)   /*MPPT-PV运行触发警报标志*/
			{
				MPPTAlarmState();
				if(GetChargeMode() == PV_CHARGE)                     
				{
					PowerDataWRInfo(true);                                      /*存入电量信息*/
					PowerDataWRInfo(false);                                     /*读取电量信息*/					
				}
				sta = EN_HARD_FAULT;                
			}
			else if(RunSta == EN_RUN_CHARGE_END)
			{

				if(GetChargeMode() == PV_CHARGE)                     
				{
					PowerDataWRInfo(true);                                      /*存入电量信息*/
					PowerDataWRInfo(false);                                     /*读取电量信息*/					
				}
				sta = EN_STOP_CHARGE;                
			}
			else if(RunSta == EN_RUN_CHARGE_FULL)
			{

				if(GetChargeMode() == PV_CHARGE)                     
				{
					PowerDataWRInfo(true);                                      /*存入电量信息*/
					PowerDataWRInfo(false);                                     /*读取电量信息*/					
				}
				sta = EN_FULL_CHARGE;                
			}
			break;
		}	
		case EN_STOP_CHARGE:                                           /*输入源被拔情况*/
		{
			charge_logic = EN_NONE_LOOP;                                 /*不运行环路*/
			//TODO
//			_PWMOutputEnable(DISABLE);                                   /*关闭PWM*/
			RelayControl(false);                                         /*关闭继电器*/
			ChargeModeSet(None_InPut);                                   /*无输入*/				
			ChargeStateSet(STOP_CHARGE);                                 /*停止充电*/
			SysAlarmSet(EN_SYS_NORMAL);                                  /*正常运行*/   
			SysStateSet(SHUTDOWN);                                       /*停止PWM*/
			sta = EN_STOP;                                             /*开启新一轮检测*/
			break;
		}
		case EN_FULL_CHARGE:
		{
			charge_logic = EN_NONE_LOOP;                                 /*不运行环路*/
			//TODO
//			_PWMOutputEnable(DISABLE);                                   /*关闭PWM*/
			RelayControl(false);                                         /*关闭继电器*/
			ChargeModeSet(None_InPut);                                 /*无输入*/				
			ChargeStateSet(END_CHARGE);                                  /*充满*/
			SysAlarmSet(EN_SYS_NORMAL);                                  /*正常运行*/   
			SysStateSet(SHUTDOWN);                                       /*停止PWM*/
            sta = EN_STOP;			                                             /*开启新一轮检测*/
			break;
		}
		case EN_MPPT_ALARM:                                            /*重新上电运行*/
		{
			charge_logic = EN_NONE_LOOP;                                 /*不运行环路*/
			//TODO
//			_PWMOutputEnable(DISABLE);                                   /*关闭PWM*/
			RelayControl(false);                                         /*关闭继电器*/
			ChargeModeSet(None_InPut);                                   /*无输入*/				
			ChargeStateSet(STOP_CHARGE);                                 /*充电结束*/
			SysAlarmSet(EN_SYS_NORMAL);                                  /*正常运行*/   
			SysStateSet(SHUTDOWN);                                       /*停止PWM*/
			sta = EN_STOP;                                               /*开启新一轮检测*/
			break;
		}
		case EN_HARD_FAULT:
		{
			charge_logic = EN_NONE_LOOP;                                 /*不运行环路*/			
			//TODO
//			_PWMOutputEnable(DISABLE);                                   /*关闭PWM*/
			RelayControl(false);                                         /*关闭继电器*/
			ChargeModeSet(None_InPut);                                   /*无输入*/				
			ChargeStateSet(STOP_CHARGE);                                 /*充电结束*/
			SysAlarmSet(EN_HARDEARE_FAULT);                              /*硬件故障*/   
			SysStateSet(FAULT_SHUTDOWN);                                 /*故障停机*/
            sta = EN_MPPT_FAULT_RECOVER;
			break;
		}
		case EN_SOFT_FAULT:
		{
			charge_logic = EN_NONE_LOOP;                                 /*不运行环路*/
			//TODO
//			_PWMOutputEnable(DISABLE);                                   /*关闭PWM*/
			RelayControl(false);                                         /*关闭继电器*/
			ChargeModeSet(None_InPut);                                   /*无输入*/				
			ChargeStateSet(STOP_CHARGE);                                 /*充电结束*/
			SysAlarmSet(EN_SOFTWARE_FAULT);                              /*软件故障*/   
			SysStateSet(FAULT_SHUTDOWN);                                 /*故障停机*/
            sta = EN_MPPT_FAULT_RECOVER;
			break;
		}
        case EN_MPPT_FAULT_RECOVER:
        {
			if(TimDifftimeFromNow(voltage_get_time) > 200)
			{
                voltage_get_time   = TimGetSystime();
                voltage_get_cont ++;
                mppt_involtage[0]  = mppt_involtage[1];
                mppt_involtage[1]  = mppt_involtage[2];
                
                mppt_outvoltage[0] = mppt_outvoltage[1];
                mppt_outvoltage[1] = mppt_outvoltage[2];
                
                mppt_intemp[0]  = mppt_intemp[1];
                mppt_intemp[1]  = mppt_intemp[2];
                
                mppt_outtemp[0]  = mppt_outtemp[1];
                mppt_outtemp[1]  = mppt_outtemp[2];  
                
                mppt_involtage[2]  = SampleDataGet(EN_SAMPLE_GET_IN_VOL) ;	
                mppt_outvoltage[2] = SampleDataGet(EN_SAMPLE_GET_OUT_VOL);
                mppt_intemp[2]  = SampleDataGet(EN_SAMPLE_GET_IN_TEMP) ;	
                mppt_outtemp[2] = SampleDataGet(EN_SAMPLE_GET_OUT_TEMP);	
			}
            /**********************************消除警报*************************************/
            if(voltage_get_cont >= 3)
            {
                voltage_get_cont = 0;
                if( gMpptErrorInfo.mppt_error1_code.b.mppt_pv_uv_flag  || gMpptErrorInfo.mppt_error1_code.b.mppt_12_uv_flag || \
                   gMpptErrorInfo.mppt_error1_code.b.mppt_pv_ov_flag || gMpptErrorInfo.mppt_error1_code.b.mppt_12_ov_flag )
                {
                    if((mppt_involtage[0] < IN_VOL_MIN && mppt_involtage[1] < IN_VOL_MIN && mppt_involtage[2] < IN_VOL_MIN) || \
                        ((mppt_involtage[0] > MPPT_IN_UV_RECOVER && mppt_involtage[0] < MPPT_IN_OV_RECOVER) &&\
                        (mppt_involtage[1] > MPPT_IN_UV_RECOVER && mppt_involtage[1] < MPPT_IN_OV_RECOVER)  \
                        && (mppt_involtage[2] > MPPT_IN_UV_RECOVER && mppt_involtage[2] < MPPT_IN_OV_RECOVER)))
                    {
                        gMpptErrorInfo.mppt_error1_code.b.mppt_pv_uv_flag = false;
                        gMpptErrorInfo.mppt_error1_code.b.mppt_pv_ov_flag = false; 
                        gMpptErrorInfo.mppt_error1_code.b.mppt_12_uv_flag = false;
                        gMpptErrorInfo.mppt_error1_code.b.mppt_12_ov_flag = false; 
                    }
                }
                if(gMpptErrorInfo.mppt_error1_code.b.mppt_out_uv_flag || gMpptErrorInfo.mppt_error1_code.b.mppt_out_ov_flag)
                {
                    if((mppt_outvoltage[0] < OUT_VOL_MIN && mppt_outvoltage[1] < OUT_VOL_MIN && mppt_outvoltage[2] < OUT_VOL_MIN) || \
                        ((mppt_outvoltage[0] > MPPT_OUT_UV_RECOVER && mppt_outvoltage[0] < MPPT_OUT_OV_RECOVER) &&\
                        (mppt_outvoltage[1] > MPPT_OUT_UV_RECOVER && mppt_outvoltage[1] < MPPT_OUT_OV_RECOVER)  \
                        && (mppt_outvoltage[2] > MPPT_OUT_UV_RECOVER && mppt_outvoltage[2] < MPPT_OUT_OV_RECOVER)))
                    {
                        gMpptErrorInfo.mppt_error1_code.b.mppt_out_uv_flag = false;
                        gMpptErrorInfo.mppt_error1_code.b.mppt_out_ov_flag = false; 
                    }
                }
                if(gMpptErrorInfo.mppt_error1_code.b.mppt_otp_flag )
                {
                    if((mppt_intemp[0] < MPPT_OTP_TEMP_RECOVER && mppt_intemp[1] < MPPT_OTP_TEMP_RECOVER && mppt_intemp[2] < MPPT_OTP_TEMP_RECOVER) && \
                        (mppt_outtemp[0] < MPPT_OTP_TEMP_RECOVER && mppt_outtemp[1] < MPPT_OTP_TEMP_RECOVER && mppt_outtemp[2] < MPPT_OTP_TEMP_RECOVER))
                    {
                        gMpptErrorInfo.mppt_error1_code.b.mppt_otp_flag = false;
                    }
                }
            }
            if(gMpptErrorInfo.mppt_error1_code.value == 0 && gMpptErrorInfo.mppt_error2_code.value == 0)                                      /*清除故障状态*/
            {
            	FaultFlagClear();                                        /*清除该标志位*/
            	SysAlarmSet(EN_SYS_NORMAL);                              /*系统正常运行*/
            	MPPTFaultClear();                                        /*清除MPPT故障*/
            	SysStateSet(SHUTDOWN);                                   /*停机状态*/
            	memset(&gmpptAlarmFlag,0,sizeof(gmpptAlarmFlag));
            	memset(&gmpptAlarmCount,0,sizeof(gmpptAlarmCount));
            	sta = EN_STOP;                                           /*开启新一轮检测*/
            }
            if(GetFaultClearFlag())                                      /*清除故障状态*/
            {
            	FaultFlagClear();                                        /*清除该标志位*/
            	SysAlarmSet(EN_SYS_NORMAL);                              /*系统正常运行*/
            	MPPTFaultClear();                                        /*清除MPPT故障*/
            	SysStateSet(SHUTDOWN);                                   /*停机状态*/
            	gOutOcFlagClear();                                     /*清除过流标志*/
            	gInOVFlagClear();
            	gOutOVFlagClear();                                      /*清除超压标志*/
            	sta = EN_STOP;                                           /*开启新一轮检测*/
            }
            break;
        }
		case EN_STOP:
		{
			static uint32_t   run_delay_time   = 0 ;
			static uint8_t run_cont = 0;
			if(TimDifftimeFromNow(run_delay_time) > 200)
			{
				run_delay_time   = TimGetSystime();
				run_cont ++;
				if(run_cont >= 8)
				{
					memset(mppt_involtage,0,sizeof(mppt_involtage));
					memset(mppt_outvoltage,0,sizeof(mppt_outvoltage));
					memset(&mppt_intemp,0,sizeof(mppt_intemp));
					memset(&mppt_outtemp,0,sizeof(mppt_outtemp));
					run_cont = 0;
					sta = EN_IDLE;                                           /*等待一段时间后回到空闲状态*/
				}
			}		
			break;
		}
		default:break;
	}
}

EN_RUN_LOJIC Mppt_Run(bool out_flag ,charge_mode souce_type )
{
	static EN_RUN_LOJIC    run_lojic          = EN_RUN_INIT;  /*默认运行初始状态*/
	static uint32_t set_time = 0;
	static uint16_t cont = 0;	
	static uint32_t ready_time = 0;
	static uint32_t ready_cont = 0;
	int32_t duty_set = 0; 
	if(mppt_run_init_flag)
	{
		mppt_run_init_flag = false;
		run_lojic = EN_RUN_STOP;                                    /*下次直接从PV或DC的ready运行*/
	}
	/****************警报处理***********************/

    if(gout_over_voltage_get() || gout_over_current_get() || gin_over_voltage_get())
    {
        if(gout_over_voltage_get())
        {
            gmpptAlarmFlag.out_over_vol_flag = true;
            gOutOVFlagClear ();
       
        }
        if(gout_over_current_get())
        {
        	gmpptAlarmFlag.out_over_curr_flag = true;
        	gOutOcFlagClear ();
        }   
        if(gin_over_voltage_get())
        {
        	if(GetChargeMode() == DC_CHARGE)
        	{
        		gmpptAlarmFlag.dc_over_vol_flag = true;
        	}
        	else
        	{
        		gmpptAlarmFlag.pv_over_vol_flag = true;
        	}
        	gInOVFlagClear();
        }
        run_lojic = EN_RUN_MPPTALARM;
    }
    /****************关闭MPPT***********************/
    if(GetMpptStopRunFlag() && run_lojic != EN_RUN_MPPTALARM)
    {
    	run_lojic = EN_RUN_CHARGE_END;
    }
    else if(run_lojic == EN_RUN_MPPTALARM)
    {
    	run_lojic = EN_RUN_MPPTALARM;
    }
/****************MPPT正常运行代码***********************/ 
	switch (run_lojic)
	{
		case EN_RUN_INIT:
		{
			if(RelayControl(true))
			{
				//TODO
//				timer_interrupt_disable(TIMER1,TIMER_INT_UP);                      /*关闭定时器中断*/
				irq_param_init();	                                                 /*初始化控制器参数*/
//				timer_interrupt_enable(TIMER1,TIMER_INT_UP);                       /*开启定时器中断*/
				run_lojic = EN_RUN_READY;                                          /*切换至运行准备状态*/
			}
			else
			{
				gmpptAlarmFlag.relay_first_fault_flag = true;                    /*继电器错误标志置位*/
				run_lojic = EN_RUN_MPPTALARM;
			}
			break;
		}
		case EN_RUN_READY:
		{
			/*延时，等待中断采样值更新*/
			if(TimDifftimeFromNow(ready_time) > 20)
			{
				ready_time = TimGetSystime();
				ready_cont ++;
				if(ready_cont >= 14)
				{
					ChargeParamInit(souce_type);                          /*初始化默认类型*/
					if(out_flag == true)
					{
						run_lojic = EN_RUN_OUT_EXIST;                       /*输出存在电压*/
					}
					else
					{
						//TODO
						//								MPPTPWMSet(600);                                    /*初始占空比*/
						if(!gout_over_voltage_get() && !gout_over_current_get() && !GetMpptStopRunFlag() &&!gin_over_voltage_get())
						{
							//TODO
							//									_PWMOutputEnable(ENABLE);                           /*使能PWM*/
						}
						run_lojic = EN_RUN_ALARM_CHECK;                     /*开PWM阶段检查输入输出*/
					}
					/*End*/
					ready_cont = 0;
				}
			}
			break;
		}
		case EN_RUN_ALARM_CHECK:                                                 /*警报检查*/
		{
			run_lojic = EN_RUN_OUT_NOEXIST;
			charge_logic = EN_SINGLE_VOLTAGE_LOOP;
			break;
		}
		case EN_RUN_OUT_EXIST:                                                   /*输出有电压情况*/
		{
			duty_set = DutyCalculate(gSampleRawValue.mppt_in_Uadc_process,gSampleRawValue.mppt_out_Uadc_process);				                     	                                      /*判断出有源输入后初始占空比为计算值*/
			MpptNewParmInit(duty_set);
			//TODO
			//			MPPTPWMSet(duty_set);
			if(!gout_over_voltage_get() && !gout_over_current_get() && !GetMpptStopRunFlag() && !gin_over_voltage_get())
			{
				//TODO
				//					_PWMOutputEnable(ENABLE);                           /*使能PWM*/
			}
			PidRunInit();                                                        /*保证环路不会抢先运行*/
			charge_logic = EN_SINGLE_MPPT_LOOP;                                  /*mppt单环*/
			run_lojic = EN_RUN_NORMAL;                                           /*切换至正常充电状态*/
			break;
		}
		case EN_RUN_OUT_NOEXIST:                                               /*输出无电压情况*/
		{
			static bool  MPPT_BMS_MUTUAL = true;
			if(gloopset.battery_set.ov_value < MPPT_OUT_OV_RECOVER && gloopset.battery_set.ov_value > MPPT_OUT_UV_RECOVER && MPPT_BMS_MUTUAL)
			{
				TragetValue.target_voltage_single_loop = (int16_t)(((float)gloopset.battery_set.ov_value / 10 + 0.7f) * 3.0f / 100.0f * ADC_MaxValue / ADC_Ref);
				MPPT_BMS_MUTUAL = false;
			}
			if((float)SampleDataGet(EN_SAMPLE_GET_OUT_CURR) > BATTERY_CURR_EXISTENCE_THRESHOLD)     /*输出电流大于阈值，说明MOS已开*/
			{
				charge_logic = EN_NONE_LOOP;                                          /*不运行环路*/
				//TODO
				//					_PWMOutputEnable(DISABLE);                                            /*失能PWM，切换控制环路*/
				//					soft_delayms(200);
				DELAY_MS(200);
				duty_set = DutyCalculate(gSampleRawValue.mppt_in_Uadc_process,gSampleRawValue.mppt_out_Uadc_process);				                     	                                      /*判断出有源输入后初始占空比为计算值*/
				MpptNewParmInit(duty_set);
				//TODO
				//					MPPTPWMSet(duty_set);
				if(!gout_over_voltage_get() && !gout_over_current_get() && !GetMpptStopRunFlag() &&!gin_over_voltage_get())
				{
					//TODO
					//							_PWMOutputEnable(ENABLE);                           /*使能PWM*/
				}
				PidRunInit();                                                        /*保证环路不会抢先运行*/
				charge_logic = EN_SINGLE_MPPT_LOOP;                                  /*mppt单环*/
				MPPT_BMS_MUTUAL = true;
				run_lojic = EN_RUN_NORMAL;                                           /*切换至正常充电状态*/
			}
			break;
		}
		case EN_RUN_NORMAL:                                 
		{
			if(TimDifftimeFromNow(set_time) > 60)          /*更新充电电流*/
			{
				set_time = TimGetSystime();
				cont ++;
				if(cont >= 15)
				{
					Normalize_ADC(souce_type);
					cont = 0;
				}
			}
			/*充满及输入源被拔判断*/
			switch (ChargeState(souce_type,gsysvalue.mpptvalue.incurrent,gloopset.battery_set.charge_current))
			{
			case EN_CHARGE_STOP:
			{
				run_lojic = EN_RUN_CHARGE_END;
				break;
			}
			case EN_CHARGE:
			{
				ChargeStateSet(CHARGE);                                 /*充电*/
				SysAlarmSet(EN_SYS_NORMAL);                                  /*正常运行*/
				SysStateSet(NORMAL_OPERATION);                                       /*正常运行*/
				break;
			}
			case EN_CHARGE_COVER:                                   /*PV充电时光伏板被遮挡*/
			{
				run_lojic = EN_RUN_CHARGE_COVER;
				break;
			}
			case EN_CHARGE_END:
			{
				run_lojic = EN_RUN_CHARGE_FULL;
				break;
			}
			}
			/*故障判断*/
			if(RelayFault(gsysvalue.elsevoltage.pvin_voltage,gsysvalue.mpptvalue.start_involtage,PV_MPPT_VOL_DIFF))        /*运行中继电器存在压差*/
			{
				gmpptAlarmFlag.relay_run_alarm_flag = true;
				run_lojic = EN_RUN_MPPTALARM;
			}
			if(GetFanState() == EN_FAN_ERROR)   														
			{
				gmpptAlarmFlag.fan_blockage_alarm_flag = true;                                                 /*风扇堵转错误置位*/
				run_lojic = EN_RUN_MPPTALARM;
			}
			if(SYSFaultUpdata()|| gout_over_voltage_get() || gMpptErrorInfo.mppt_error1_code.b.mppt_otp_flag || gout_over_current_get() || gin_over_voltage_get())																					/*提交警报*/
			{
				if(gout_over_voltage_get())
				{
					gmpptAlarmFlag.out_over_vol_flag = true;
					gOutOVFlagClear ();
				}
				if(gout_over_current_get())
				{
					gmpptAlarmFlag.out_over_curr_flag = true;
					gOutOcFlagClear ();
				}
				if(gin_over_voltage_get())
				{
					if(GetChargeMode() == DC_CHARGE)
					{
						gmpptAlarmFlag.dc_over_vol_flag = true;
					}
					else
					{
						gmpptAlarmFlag.pv_over_vol_flag = true;
					}
					gInOVFlagClear();
				}
				run_lojic = EN_RUN_MPPTALARM;
			}
			/*电量计算*/
			if(souce_type == PV_CHARGE)
			{
				if(MinuteElectric(GetIrqElectricFlag()))
				{
					ElectricUpdata();
				}			
			}
			break;
		}
		/*对应不同工况*/
		case EN_RUN_MPPTALARM:
		{
			break;
		}
		case EN_RUN_CHARGE_END:
		{

			break;
		}
		case EN_RUN_CHARGE_FULL:
		{                                   
			break;
		}
		case EN_RUN_CHARGE_COVER:                                      /*PV充电遮挡*/
		{
			charge_logic = EN_NONE_LOOP;                                 /*不运行环路*/
			static uint32_t charge_cover_time = 0;
			static uint16_t charge_cover_cont = 0;
			if(TimDifftimeFromNow(charge_cover_time) > 50)          /*更新充电电流，即执行MPPT时间*/
			{
				charge_cover_time = TimGetSystime();
				charge_cover_cont ++;
				if(charge_cover_cont >= 40)
				{
					irq_param_init();	                                     /*初始化控制器参数*/
					if(SampleDataGet(EN_SAMPLE_GET_OUT_VOL) > OUT_VOL_MIN)
					{
						run_lojic = EN_RUN_OUT_EXIST;		                     /*重新运行MPPT*/
					}
					else
					{
						run_lojic = EN_RUN_CHARGE_END;	                     /*本次充电结束*/
					}
					charge_cover_cont = 0;
				}
			}
			break;
		}
		/*END*/
		case EN_RUN_STOP:
		{
			cont = 0;
			run_lojic = EN_RUN_INIT;
			MpptParamClear();
			break;
		}
		default:break;
	}
	return run_lojic;
}






/************************ (C) COPYRIGHT Nova *************END OF FILE**********/





