
#ifndef _MID_POSITION_PID_H_
#define _MID_POSITION_PID_H_
#include "stdint.h"
//#include "bsp.h"
//#include "dev_sample.h"
#include "mid_pv_data_conversion.h"
#include "mid_pv_interrupt.h"
#include "mid_pv_logic_control.h"
#include "string.h"
//-------------定义PID变量结构体----------------
typedef struct _pid_stru_t{
    int32_t set_value   ;
    int32_t actual_value;
    int32_t err         ;        //定义偏差值
    int32_t err_last    ;        //定义上一个偏差值
    int32_t result      ;        //定义结果值 【电压外环的结果是电流内环的给定，电流内环的结果是占空比】
    int32_t integral    ;        //定义积分值
    int32_t differential;        //定义微分值
}Pid;



typedef struct _MpptNew_stru_t{
		int32_t v_old     ;
		int32_t p_old     ;
		int32_t pwm_old  ;             /*MPPT计算N-1次PID设定PWM*/  
		int32_t p         ;
		int32_t dp        ;
		int32_t dv        ;
		int32_t delta_pwm ;
		int32_t vref_max  ;
		int32_t vref_min  ;
		int32_t vout_max  ;
		int32_t pwm       ;
		int32_t uoc_pwm   ;
		int32_t uoc_vol   ;		
		int32_t i_max  ;
		/*此处为改进算法新增变量*/
		int32_t max_pwm_delta ;
		int32_t min_pwm_delta ;	
		bool    state     ;
		uint8_t updown    ;
		uint8_t time      ;
		int32_t pwm_old_array[6];
	  uint8_t count     ;
	  int32_t i_old      ;
}Mpptnew;

typedef enum
{
	EN_PID_DC1VOL = 0,
	EN_PID_DC1CURR   ,
	EN_PID_POWER     ,
	EN_PID_MPPTDCVOL   ,
	EN_PID_MPPTDCCURR  ,
	EN_PID_MPPTPVVOL
}EN_PID_MODE;

extern Pid  gstPidParm[6];
extern Mpptnew gstMpptNewParm;
void   PidParmInit(void);
uint32_t PidRealize(EN_PID_MODE e_pid_mode, uint32_t actual_value, uint32_t target_value, uint32_t set_i);


void MpptNewParmInit(uint16_t pwm_old);
int32_t MpptNewPo(int32_t pv_v, int32_t pv_i ,int32_t bat_v, int32_t bat_i , int16_t in_Imax, int16_t out_Imax,int16_t in_maxpower,int16_t actual_in_power,int16_t actual_invol);
uint16_t* MpptPoMiddle(int32_t pv_v , int32_t pv_i, int32_t batter_v , int32_t batter_i ,bool clear_flag, int16_t in_Imax, int16_t out_Imax,int16_t in_maxpower,int16_t actual_in_power,int16_t actual_invol);
void MpptParamClear(void);
#endif
