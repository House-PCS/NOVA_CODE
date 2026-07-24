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
  * <tr><th>Date          <th>Version   <th>Author                    <th>Description
  * <tr><td>2022-5-19     <td>V1.0.0    <td>xuchengyang 003107        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "mid_position_pid.h"

Pid  gstPidParm[6];
//-------初始化变量，可以通过调节Kp,Ki,Kd控制系统的响应------------
void PidParmInit(void)
{
		memset((uint8_t*)&gstPidParm, 0, sizeof(gstPidParm[0]) * 6);
};

//----------控制算法 需要做最大电流/最大占空比限定----------------在过流阶段需要做抗积分饱和算法
 uint32_t PidRealize(EN_PID_MODE e_pid_mode, uint32_t actual_value, uint32_t target_value ,uint32_t set_i)
{
	float32 max = 0;
	float min = 0;
	float kp  = 0;
	float ki  = 0;
	float kd  = 0;
	uint16_t e_pid_mode1 = e_pid_mode;
	
switch(e_pid_mode1)
    {
        case EN_PID_DC1VOL :
        {
        	//TODO
//            max = gloop_setadc.dc_adc.dc1_currentADC - I_BIAS_ADC;  //最大电流24A
            min = (0.0 - 1*0.002*40)*4096/3.;	 //最小电流设置成负的，可以钳位
            kp  = 0.2f;	
            ki  = 0.1f;	
            kd  = 0.0;	               
            break;
        }
        case EN_PID_DC1CURR :
        {
            max = 1140;  /*最大占空比*/
            min = 10;	
            kp  = 0.1f;	
            ki  = 0.003f;	
            kd  = 0.0;	                 
            break;
        }
        case EN_PID_POWER :
        {
					max = 1010.0 * 1024;
					min = TragetValue.voltage_single_param_min * 1024;
					kp  = 10;	
					ki  = 8;					
					kd  = 0;	            
            break;
        }            
        case EN_PID_MPPTDCVOL :
        { 
            max = set_i * 1024;   		
            min = -100.0 * 1024;
            kp  = 100 ;	
            ki  = 50 ;							
            kd  = 0;					
            break;
        }
        case EN_PID_MPPTDCCURR :
        {
            max = 1010.0 *1024;                         //最大占空比(该值计算方式为最大功率点附近（即电源最低处，升压至60V电压值）)
            min = TragetValue.double_loop_param_min * 1024;	
            kp  = 50 ;	
            ki  = 4;						
            kd  = 0;                
            break;
        }
        case EN_PID_MPPTPVVOL :
        {
            max = 1010.0 * 1024;
            min = 300.0  * 1024;
						/*适合光伏模拟器*/	
            kp  = 100;	
            ki  = 50;					
            kd  = 0;						
            break;
        }
        default: break;
    }  
    gstPidParm[e_pid_mode1].set_value   =   target_value                    ;
    gstPidParm[e_pid_mode1].err         =   gstPidParm[e_pid_mode1].set_value - actual_value ;
    gstPidParm[e_pid_mode1].integral    =   gstPidParm[e_pid_mode1].integral + gstPidParm[e_pid_mode1].err          ;
    if( gstPidParm[e_pid_mode1].integral > max/ki )
    {
        gstPidParm[e_pid_mode1].integral = max/ki ;
    }
    else if(gstPidParm[e_pid_mode1].integral < min/ki)
    {
        gstPidParm[e_pid_mode1].integral = min/ki ;
    }
    else
    {
        gstPidParm[e_pid_mode1].integral = gstPidParm[e_pid_mode1].integral ;
    }

    gstPidParm[e_pid_mode1].differential=   gstPidParm[e_pid_mode1].err-gstPidParm[e_pid_mode1].err_last           ;
    gstPidParm[e_pid_mode1].err_last    =   gstPidParm[e_pid_mode1].err                        ;
    gstPidParm[e_pid_mode1].result      =   kp * gstPidParm[e_pid_mode1].err + ki * gstPidParm[e_pid_mode1].integral + kd * gstPidParm[e_pid_mode1].differential;
    if(gstPidParm[e_pid_mode1].result >= max)
    {
        gstPidParm[e_pid_mode1].actual_value =   max   ;            
    }
    else if(gstPidParm[e_pid_mode1].result <= min)
    {
        gstPidParm[e_pid_mode1].actual_value =   min   ;
    } 
    else
    {
        gstPidParm[e_pid_mode1].actual_value = gstPidParm[e_pid_mode1].result ;
    }
    return gstPidParm[e_pid_mode1].actual_value           ;
}


Mpptnew gstMpptNewParm;
Mpptnew MediaParam;
void MpptNewParmInit(uint16_t pwm_old)
{
    
    gstMpptNewParm.v_old     = (int32_t)gSampleRawValue.mppt_in_Uadc_process;//需要提前采样赋值
    gstMpptNewParm.p_old     = 0  ; 	
    gstMpptNewParm.pwm_old   = pwm_old;                                              /*需要实际计算出所需占空比*/                                         
    gstMpptNewParm.p         = 0  ; //实际功率
    gstMpptNewParm.dp        = 0  ; //实际功率变化值
    gstMpptNewParm.dv        = 0; //电压变化值
    gstMpptNewParm.delta_pwm = 1  ; //步进值
    gstMpptNewParm.vref_max  = 60.0f*3*4096/100/3.3f   ; //光伏板最大工作电压
    gstMpptNewParm.vref_min  = 11.5f*3*4096/100/3.3f   ; //光伏板最小工作电压
    gstMpptNewParm.vout_max  = 70.0f*3*4096/100/3.3f   ; /*最大输出，防止轻载模式的时候输出电压过高*/
    gstMpptNewParm.pwm       = 0    ;                     //实时计算的PWM值/**0.8*/
    gstMpptNewParm.i_max     = (23.0f * I_AF * Samp_R + I_BIAS) * ADC_MaxValue / ADC_Ref - I_BIAS_ADC;
    gstMpptNewParm.uoc_pwm   = pwm_old ;
    gstMpptNewParm.uoc_vol   = (int32_t)gSampleRawValue.mppt_in_Uadc_process;//需要提前采样赋值 ;	
    gstMpptNewParm.max_pwm_delta = 1 ;
    gstMpptNewParm.min_pwm_delta = 1 ;	
    gstMpptNewParm.state     = true;
    gstMpptNewParm.updown    = 1   ;
    gstMpptNewParm.time      = 0   ;
    gstMpptNewParm.count     = 0   ;
    gstMpptNewParm.i_old     = 0   ;
		
}
bool input_souce_judge = false;
/*该变量为小周期MPPT的PWM步长标识 0无更新 1减小PWM 2增大PWM*/
uint16_t pwm_delat_update = 0;

int32_t MpptNewPo(int32_t pv_v, int32_t pv_i ,int32_t bat_v, int32_t bat_i , int16_t in_Imax, int16_t out_Imax,int16_t in_maxpower,int16_t actual_in_power,int16_t actual_invol)
{
	
	int16_t in_vol[2] = {0};
	/*改进算法*/
	gstMpptNewParm.p = (pv_v * pv_i) < 0 ? 0 : (pv_v * pv_i);
	if(gstMpptNewParm.state)
	{ 
		 if(GetChargeMode() == None_InPut  )
		{
				in_Imax = 1000;
				if(pv_i < 50)                   /*无输入且为0.5A*/
				{
					gstMpptNewParm.max_pwm_delta = 2 ;
				}
				else
				{
					gstMpptNewParm.max_pwm_delta = 1 ;
				}
		} 
		else if(GetChargeMode() == PV_CHARGE)
		{
					gstMpptNewParm.max_pwm_delta = 2 ;
		}
		if(pv_i > in_Imax || bat_i > out_Imax || actual_in_power > in_maxpower || actual_invol < MPPT_IN_UV_RECOVER)
		{
				gstMpptNewParm.updown = 0;
		}
		if(gstMpptNewParm.p < gstMpptNewParm.p_old )
		{
			gstMpptNewParm.updown ^=1;                   /*改变扰动方向*/	
			gstMpptNewParm.time ++;	                     /*记录改变方向的次数*/	
			if(gstMpptNewParm.time >= 2)               /*若改变超过3次则进入小步长状态*/
			{
				gstMpptNewParm.pwm_old_array[gstMpptNewParm.count ++ ] = gstMpptNewParm.pwm_old;
			}			
			if(GetChargeMode() == None_InPut && gstMpptNewParm.count >= 2)
			{
				input_souce_judge = true;				
			}
		}
		/*找最大功率点*/
		gstMpptNewParm.pwm = (gstMpptNewParm.updown) ? ((gstMpptNewParm.count > 0) ? gstMpptNewParm.pwm_old + gstMpptNewParm.min_pwm_delta : gstMpptNewParm.pwm_old + gstMpptNewParm.max_pwm_delta) \
												: ((gstMpptNewParm.count > 0) ? gstMpptNewParm.pwm_old - gstMpptNewParm.min_pwm_delta : gstMpptNewParm.pwm_old - gstMpptNewParm.max_pwm_delta);
		gstMpptNewParm.p_old = gstMpptNewParm.p;
		gstMpptNewParm.pwm_old = gstMpptNewParm.pwm;	
		/*DC源到达最大功率点后关闭MPPT*/
		if(gstMpptNewParm.count >= 6)
		{
			if(GetChargeMode() == PV_CHARGE)
			{
				gstMpptNewParm.count = 1;
				gstMpptNewParm.time  = 3;
			}
		}
		gstMpptNewParm.i_old = pv_i;
	}
	else
	{
		switch (pwm_delat_update)
		{
			case 0 :
			{
			
				break;
			}
			case 1 :
			{
				gstMpptNewParm.pwm = gstMpptNewParm.pwm_old - gstMpptNewParm.delta_pwm;
				MediaParam.pwm_old = mppt_middle_duty_set;
				MediaParam.pwm = mppt_middle_duty_set; 
				MediaParam.updown = 0;
				break;
			}
			case 2 :
			{
				gstMpptNewParm.pwm = gstMpptNewParm.pwm_old + gstMpptNewParm.delta_pwm;
				MediaParam.pwm = 0; 
				MediaParam.pwm_old = 0;
				MediaParam.updown = 1;
				break;
			}
		}
		pwm_delat_update = 0;
	}
	if(input_souce_judge)
	{
		input_souce_judge = false;
		in_vol[0] = (int16_t)((float)gstMpptNewParm.uoc_vol * (ADC_REFER) * (V_AF) * 10);
		in_vol[1] = SampleDataGet(EN_SAMPLE_GET_IN_VOL);
		if(abs(in_vol[0] - in_vol[1]) > mppt_pv_dc_souce_judge_vol)            /*超过1.5V即为PV充电*/
		{
			ChargeModeSet(PV_CHARGE);
			gstMpptNewParm.count = 0;
			gstMpptNewParm.time  = 0;
			gstMpptNewParm.max_pwm_delta = 2 ;
			gstMpptNewParm.min_pwm_delta = 1 ;
			gstMpptNewParm.state = true;			
		}
		else
		{
			ChargeModeSet(DC_CHARGE); 
			gstMpptNewParm.count = 0;
			gstMpptNewParm.time  = 3;
			gstMpptNewParm.max_pwm_delta = 1 ;
			gstMpptNewParm.min_pwm_delta = 1 ;	
			gstMpptNewParm.state = false;
		}
		
	}
	gstMpptNewParm.pwm_old = gstMpptNewParm.pwm;
	return gstMpptNewParm.pwm;
}

uint16_t* MpptPoMiddle(int32_t pv_v , int32_t pv_i, int32_t batter_v , int32_t batter_i ,bool clear_flag, int16_t in_Imax, int16_t out_Imax,int16_t in_maxpower,int16_t actual_in_power,int16_t actual_invol)
{
	static uint16_t number_arr[3];
	static uint16_t input_judge = 0;
	
	uint16_t x = 0; 
	uint16_t y = 0;

	if(clear_flag && gstMpptNewParm.state)                          /*若大周期MPPT正在运行则需改变占空比*/
	{
		MediaParam.pwm = 0;
		MediaParam.pwm_old = 0;
	}
	if(true)
	{
			MediaParam.p   = (pv_v * pv_i) < 0 ? 0 : (pv_v * pv_i);	
			/**************超过最大电流判定输入源类型**************/
			if((pv_i <= in_Imax + 100 && pv_i >= in_Imax - 30) && GetChargeMode() == None_InPut)        
			{
				gstMpptNewParm.state     = false;                     /*关闭大周期MPPT*/
				input_judge ++ ;
			}
			if(input_judge == 20)
			{
				input_souce_judge = true;	
				input_judge = 0;
			}
			/**************超过最大电流判定输入源类型(END)**************/
			/**************MPPT更新占空比代码**************/
			if(pv_i > in_Imax || batter_i > out_Imax|| actual_in_power > in_maxpower || actual_invol < MPPT_IN_UV_RECOVER)
			{
				MediaParam.updown = 0;
			}
			else if(MediaParam.p < MediaParam.p_old)
			{
				MediaParam.updown ^=1;                   /*改变扰动方向*/	
			}
			MediaParam.pwm = (MediaParam.updown) ?  MediaParam.pwm_old + 1 : MediaParam.pwm_old - 1 ;
			MediaParam.pwm = (MediaParam.pwm <= 0) ? 0 : ((MediaParam.pwm >= mppt_middle_duty_set) ? mppt_middle_duty_set : MediaParam.pwm);
			/**************MPPT更新占空比代码（END）**************/
			
			/***************小周期MPPT更新占空比代码***************/
			
			if(gstMpptNewParm.state == false)    /*小周期MPPT自主改变占空比数值*/
			{
				if(MediaParam.pwm <= 0 + mppt_middle_pwm_change_limit)
				{
					MediaParam.min_pwm_delta ++;
				}
				else
				{
					MediaParam.min_pwm_delta = 0;
				}
				if(MediaParam.pwm >= (mppt_middle_duty_set - mppt_middle_pwm_change_limit))
				{
					MediaParam.max_pwm_delta ++;
				}
				else
				{
					MediaParam.max_pwm_delta = 0;
				}
				if(MediaParam.max_pwm_delta > mppt_middle_pwm_change_number || MediaParam.min_pwm_delta > mppt_middle_pwm_change_number)
				{
					if(MediaParam.max_pwm_delta > mppt_middle_pwm_change_number)                   /*需增加占空比*/
					{
						pwm_delat_update = 2;
						MediaParam.max_pwm_delta = 0;
					}
					else if(MediaParam.min_pwm_delta > mppt_middle_pwm_change_number)
					{
						pwm_delat_update = 1;
						MediaParam.min_pwm_delta = 0;                             /*以中间占空比形式输出*/
					}
				}								
			}
			/**************小周期MPPT更新占空比代码（END）**************/            
            if(MediaParam.pwm == 0)
            {
                x = mppt_middle_duty_set / 2;
                y = 0;
            }
            else if(MediaParam.pwm > 0 && MediaParam.pwm <= mppt_middle_duty_set / 2 )
            {
                x = (mppt_middle_duty_set / 2 + 1) - MediaParam.pwm;
                y = 1;
            }
            else if(MediaParam.pwm > mppt_middle_duty_set / 2  && MediaParam.pwm < mppt_middle_duty_set)
            {
                y = MediaParam.pwm - (mppt_middle_duty_set / 2) + 1 ;
                x = 1;
            }
            else if(MediaParam.pwm == mppt_middle_duty_set)
            {
                y = mppt_middle_duty_set / 2;
                x = 0;
            }
			MediaParam.pwm_old = MediaParam.pwm;
			MediaParam.p_old = MediaParam.p;	
			number_arr[0] = x;
			number_arr[1] = y;	
			number_arr[2] = x + y;		
	}
	return number_arr;
}


void MpptParamClear(void)
{
	memset(&gstMpptNewParm,0,sizeof(gstMpptNewParm));
	memset(&MediaParam,0,sizeof(MediaParam));
}
