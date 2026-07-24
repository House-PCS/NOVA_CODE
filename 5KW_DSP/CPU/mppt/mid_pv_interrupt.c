/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        pid_control.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-19
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-5-19     <td>V1.0.0    <td>zzh         <td>InitialVersion
  * <tr><td>2022-5-19     <td>V1.0.0    <td>zzh         <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "mid_pv_interrupt.h"
#include "math.h"
#include "mid_pv_data_conversion.h"
//#include "dev_misc.h"
#include "mid_pv_logic_control.h"
#include "mid_position_pid.h"
/*******************************************************************************
  * @FunctionName: SetBalanceCurrent
  * @Author:       xingxiao
  * @DateTime:     2023年1月13日T18:19:06+0800
  * @Purpose:      
  * @param:        duty: 0(0%), 1(6.6%), 2(13.2%), 3(19.8%), 4(26.4%), 5(33%), 6(39.6%), 7(46.2%), 
                         8(52.8%), 9(59.4%), 10(66%), 11(72.6%), 12(79.2%), 13(85.8%), 14(92.4%), 15(100%), 
  * @return:
*******************************************************************************/
void MPPT_PV_PI_Ctrl(uint32_t out_u,int32_t set_u);
void SingleMPPTCtrl(int32_t in_u,int32_t in_i,int32_t out_u,int32_t out_i,uint16_t else_invol,uint16_t else_incurr,uint16_t else_outvol,uint16_t else_outcurr);
static void _irq_process(void);

void LoopCtrlTimerInit(void)
{
	//TODO
//    PIDSampleTimeHandle.irq_process = _irq_process;
//    PIDTimerInit();

}

static void _charge_control_function(void);
int64_t MPPTInVoltage[3] = {0, 0, 0};
int64_t MPPTIncurrent[3] = {0, 0, 0};
int64_t MPPTOutVoltage[3] = {0, 0, 0};
int64_t MPPTOutCurrent[3] = {0, 0, 0};
int64_t MPPTPVVoltage[3] = {0, 0, 0};

bool outover_voltage_flag = false;            /*输出超压标志位*/
bool outover_current_flag = false;            /*输出过流标志位*/
bool inover_voltage_flag = false;             /*输入超压标志*/

bool electric_calculate_flag = false;         /*电量计算标志*/ 
   


uint16_t MPPTsetPwm = 0;            /*预设电压值*/
bool first_flag = 0;
bool mppt_Flag = 0;	
bool in_filter_Flag = 0;
bool out_filter_Flag = 0;
uint16_t mppt_cnt=0;
uint16_t mppt_cnt1 = 0;
uint16_t mppt_cnt2 = 0;
bool out_ov_limit_flag = false;


void irq_param_init(void)
{
	first_flag = 0; 
  PidRunInit();
	memset(MPPTInVoltage,0,sizeof(MPPTInVoltage));
	memset(MPPTIncurrent,0,sizeof(MPPTIncurrent));
	memset(MPPTOutVoltage,0,sizeof(MPPTOutVoltage));
	memset(MPPTOutCurrent,0,sizeof(MPPTOutCurrent));
	memset(MPPTPVVoltage,0,sizeof(MPPTPVVoltage));
  MpptNewParmInit(600);
	PidParmInit();            /*PID参数初始化*/	
}

void PidRunInit(void)
{
	mppt_Flag = 0;	
	in_filter_Flag = 0;
	out_filter_Flag = 0;
	mppt_cnt=0;
	mppt_cnt1 = 0;
	mppt_cnt2 = 0;
	electric_calculate_flag = 0;
	out_ov_limit_flag = false;
}

static void _irq_process(void)
{
	_charge_control_function();
}

int16_t out_curr_max_ADC = 0;
int16_t out_vol_max_ADC = 0;
int16_t in_vol_max_ADC = 0;
void irq_mppt_output_limit(void)
{
	out_curr_max_ADC = CurrConverseADC(22 * 10);
	out_vol_max_ADC = VolConverseADC(70 * 10);
	in_vol_max_ADC = VolConverseADC(68 * 10);
}


void _charge_control_function(void)
{
    static int filter_out_Uadc = 0;
    static int filter_out_Iadc = 0;	
    static int filter_in_Uadc = 0;	
    static int filter_in_Iadc = 0;	
    static uint16_t out_ov_cont = 0;
    static uint16_t out_oc_cont = 0;
    static uint16_t in_ov_cont = 0;

    //TODO
//    MPPTPVVoltage[0] = SampleValueGet(EN_PV_VOLTAGE_GET);
//    MPPTIncurrent[0] =	SampleValueGet(EN_MPPT_IN_CURRENT_GET) ;
//    MPPTOutVoltage[0] = SampleValueGet(EN_MPPT_OUT_VOLTAGE_GET);
//    MPPTOutCurrent[0] = SampleValueGet(EN_MPPT_OUT_CURRENT_GET);

    MPPTPVVoltage[2]  = MPPTPVVoltage[2]  + MPPTPVVoltage[0] ;
    MPPTOutVoltage[2]  = MPPTOutVoltage[2]  + MPPTOutVoltage[0] ;
    MPPTOutCurrent[2]  = MPPTOutCurrent[2]  + MPPTOutCurrent[0] ;
    MPPTIncurrent[2]  = MPPTIncurrent[2]  + MPPTIncurrent[0] ;
	
    MPPTPVVoltage[1]  = MPPTPVVoltage[1]  + MPPTPVVoltage[0] ;
    MPPTIncurrent[1]  = MPPTIncurrent[1]  + MPPTIncurrent[0] ;
    MPPTOutVoltage[1] = MPPTOutVoltage[1] + MPPTOutVoltage[0];
    MPPTOutCurrent[1] = MPPTOutCurrent[1] + MPPTOutCurrent[0];
    mppt_cnt = mppt_cnt + 1;	
    mppt_cnt2 = mppt_cnt2 + 1;
    if((MPPTOutVoltage[0] > out_vol_max_ADC) || outover_voltage_flag)
    {
        out_ov_cont ++;
        if(out_ov_cont >= 2)
        {
        	//TODO
//            _PWMOutputEnable(DISABLE);                     /*失能*/
            outover_voltage_flag = true;		
            out_ov_cont = 0 ;
        }
    }
    else if(MPPTOutVoltage[0] < out_vol_max_ADC)
    {
        out_ov_cont = 0;
    }
    if((MPPTOutCurrent[0] > out_curr_max_ADC) || outover_current_flag)
    {
        out_oc_cont ++;
        if(out_oc_cont >= 2)
        {
        	//TODO
//            _PWMOutputEnable(DISABLE);                     /*失能*/
            outover_current_flag = true;		
            out_oc_cont = 0 ;
        }
    }
    else if(MPPTOutCurrent[0] < out_curr_max_ADC)
    {
        out_oc_cont = 0;
    }
    if((MPPTPVVoltage[0] > in_vol_max_ADC) || inover_voltage_flag)
    {
        in_ov_cont ++;
        if(in_ov_cont >= 2)
        {
        	//TODO
//            _PWMOutputEnable(DISABLE);                     /*失能*/
            inover_voltage_flag = true;		
            in_ov_cont = 0 ;
        }
    }
    else if(MPPTPVVoltage[0] < in_vol_max_ADC)
    {
        in_ov_cont = 0;
    }
    if(mppt_cnt2 >= 200 )                                   /*使用两个cont目的即错开MPPT与PI，使中断运行时间短一些*/
    {
        MPPTPVVoltage[2] =  MPPTPVVoltage[2] / mppt_cnt2;
        MPPTIncurrent[2] =  MPPTIncurrent[2] / mppt_cnt2;
        MPPTOutVoltage[2] =  MPPTOutVoltage[2] / mppt_cnt2;
        MPPTOutCurrent[2] =  MPPTOutCurrent[2] / mppt_cnt2;
        filter_in_Uadc =  MPPTPVVoltage[2];
        filter_in_Iadc   =  MPPTIncurrent[2];
        filter_out_Uadc = MPPTOutVoltage[2];
        filter_out_Iadc = MPPTOutCurrent[2];
        MPPTPVVoltage[2] = 0;
        MPPTIncurrent[2] = 0;
        MPPTOutVoltage[2] = 0; 
        MPPTOutCurrent[2] = 0;
        mppt_cnt2 = 0;
        in_filter_Flag =1;
    }		
    if(mppt_cnt == 1024)
    {
        mppt_cnt = 0;			
        mppt_cnt1++;
        gSampleRawValue.mppt_in_Uadc_process = (MPPTPVVoltage[1]>>10);                                
        gSampleRawValue.mppt_in_Iadc_process = (MPPTIncurrent[1]>>10);
        gSampleRawValue.mppt_out_Uadc_process = (MPPTOutVoltage[1]>>10);
        gSampleRawValue.mppt_out_Iadc_process = (MPPTOutCurrent[1]>>10);
        if(mppt_cnt1 == 10)                                                                            
        {
            mppt_Flag = 1;
            electric_calculate_flag = true;	
            mppt_cnt1 = 0;
        }
        MPPTPVVoltage[1]  = 0;
        MPPTIncurrent[1]  = 0;
        MPPTOutVoltage[1] = 0;
        MPPTOutCurrent[1] = 0;
    }	
		switch(charge_logic)
		{
			case EN_SINGLE_VOLTAGE_LOOP:          
			{
				
					MPPT_PV_PI_Ctrl(MPPTOutVoltage[0],TragetValue.target_voltage_single_loop);      
				break;
			}
			case EN_PV_MPPT_LOOP:                 
			{			
				break;			
			}
			case EN_SINGLE_MPPT_LOOP:                 
			{
                   SingleMPPTCtrl(gSampleRawValue.mppt_in_Uadc_process,gSampleRawValue.mppt_in_Iadc_process, gSampleRawValue.mppt_out_Uadc_process,gSampleRawValue.mppt_out_Iadc_process , \
											filter_in_Uadc,filter_in_Iadc,filter_out_Uadc,filter_out_Iadc	 ); 		                                      /*输入最大电流为8A*/		
				break;			
			}
			case EN_DC_DOUBLE_LOOP:
			{	
				break;				
			}
			case EN_NONE_LOOP:
			{		
				
				break;				
			}
			default:
				break;
		}
}
void MPPT_PV_PI_Ctrl(uint32_t out_u,int32_t set_u)
{

    int out_duty = 0;
    int mppt_target_current = 0;
    out_duty = PidRealize(EN_PID_MPPTPVVOL , out_u , set_u , 0);		
    if(outover_voltage_flag == false && outover_current_flag == false)
    {
    	//TODO
//        MPPTPWMSet( out_duty >> 10 );
    }				
}


void SingleMPPTCtrl(int32_t in_u,int32_t in_i,int32_t out_u,int32_t out_i,uint16_t else_invol,uint16_t else_incurr,uint16_t else_outvol,uint16_t else_outcurr)    /*PWM越低，输入端口电压越高*/
{
		static uint16_t pwm_duty = 0;
		static uint16_t* arr;
		static uint16_t  arr_number[3] = {0};
		static bool mppt_update_flag = false;                                                    /*标志大周期MPPT更新代码*/
		
/****************输出超压变量处理*******************/
		static uint16_t PWM_Old = 0;
		static uint16_t out_ov_cont = 0;
		static uint16_t out_pwm_use_cont = 0;
		//TODO
//		if((MPPTOutVoltage[0] >= gloop_setadc.battery_adc.ov_adcvalue) && GetChargeMode() == PV_CHARGE && gstMpptNewParm.state && out_ov_limit_flag)
		if(1)
		{
				out_pwm_use_cont ++;
				if(out_pwm_use_cont >= 5)
				{
					out_pwm_use_cont = 0;
					MPPTsetPwm = PWM_Old - 1;
					//TODO
//					MPPTPWMSet( MPPTsetPwm );
					PWM_Old = MPPTsetPwm;	
					gstMpptNewParm.pwm_old	= MPPTsetPwm;
					return;						
				}
		}	
		if(mppt_Flag == 1)                                                                       /*MPPT更新占空比标志*/
		{
				MPPTsetPwm = MpptNewPo( in_u,in_i,out_u,out_i,TragetValue.input_max_current_ADC, \
																TragetValue.output_max_current_ADC,TragetValue.input_max_power, \
																gsysvalue.mpptvalue.inpower,gsysvalue.mpptvalue.involtage);                 /*最大电流20A*/	
				if(gstMpptNewParm.state)                                                            /*MPPT不运行时即不改变占空比*/
				{
					//TODO
//					MPPTPWMSet( MPPTsetPwm );
				}	
				mppt_update_flag = true;				
				mppt_Flag = 0;
/****************PV输出超压变量处理*******************/
				PWM_Old = MPPTsetPwm;
				if(!out_ov_limit_flag)
				{
					out_ov_cont++;
					if(out_ov_cont >= 4)
					{
						out_ov_cont = 0;
						out_ov_limit_flag = true;
					}
				}
				
		}		
		if((GetChargeMode() != PV_CHARGE) && first_flag && in_i > 40)                                                      /*表明为DC需运行寻找最大电流代码*/
		{
				if(in_filter_Flag)                                                                                                                                /*在一次MPPT运行周期内运行*/
				{
						arr = MpptPoMiddle(else_invol,else_incurr,else_outvol , else_outcurr ,mppt_update_flag, \
																TragetValue.input_max_current_ADC,TragetValue.output_max_current_ADC ,\
																TragetValue.input_max_power,gsysvalue.mpptvalue.inpower,gsysvalue.mpptvalue.involtage);
						in_filter_Flag = 0;
						mppt_update_flag = false;
						arr_number[0] = arr[0];
						arr_number[1] = arr[1];
						arr_number[2] = arr[2];
				}
				if(arr_number[2] > 0)
				{
						if(arr_number[0] > 0)
						{
								pwm_duty = (uint16_t)MPPTsetPwm;
								arr_number[0] -- ;
						}
						else 
						{
								if(arr_number[1] > 0)
								{
								    if(MPPTsetPwm < PWM_MAX_LIMIT) // 20260429，优化代码
								    {
								        pwm_duty = (uint16_t)MPPTsetPwm + 1;
								    }
								    else
								    {
								        pwm_duty = PWM_MAX_LIMIT;
								    }

								    arr_number[1] -- ;
								}				
						}
						arr_number[2]--;				
				}
				else if(arr_number[2] <= 0)
				{
						arr_number[0] = arr[0];
						arr_number[1] = arr[1];
						arr_number[2] = arr[2];
				}	
				//TODO
//				MPPTPWMSet( pwm_duty );
		}	
		if(first_flag == 0)                             
		{
				MPPTsetPwm = gstMpptNewParm.uoc_pwm;	
				//TODO
//				MPPTPWMSet( MPPTsetPwm );
				first_flag = 1;
		}		
}

void gOutOVFlagClear (void)
{
		outover_voltage_flag = 0;             /*清除超压标志位*/
}	

bool gout_over_voltage_get (void)
{
	return outover_voltage_flag;
}	

void gOutOcFlagClear (void)
{
		outover_current_flag = 0;             /*清除超压标志位*/
}	

bool gout_over_current_get (void)
{
	return outover_current_flag;
}	

bool GetIrqElectricFlag (void)
{
	return electric_calculate_flag;
}	

void ClearIrqElectricFlag (void)
{
	electric_calculate_flag = 0;
}	

void gInOVFlagClear (void)
{
		inover_voltage_flag = 0;             /*清除超压标志位*/
}	

bool gin_over_voltage_get (void)
{
	return inover_voltage_flag;
}	
