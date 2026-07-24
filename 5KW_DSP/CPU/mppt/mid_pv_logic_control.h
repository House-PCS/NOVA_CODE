/**
******************************************************************************
* @copyright (C) COPYRIGHT Nova
* @file sys_info.h
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
#ifndef __LOGIC_CONTROL_H_
#define __LOGIC_CONTROL_H_
#ifdef __cplusplus
extern "C" {
#endif
/* Inculdes ------------------------------------------------------------------*/
#include "stdint.h"
#include <string.h>
#include "mid_pv_data_conversion.h"
#include "mid_pv_interrupt.h"
#include "cpu_timers.h"
#include "F28x_Project.h"
#ifdef __cplusplus
}
#endif

/* Exported macro -----------------------------------------------------------*/
#define EN_MAX_POWER_CHARGE          (0)       /*不与主机配合，最大功率充电*/

/******************MPPT算法参数设置*******************/
#define mppt_middle_duty_set 40   /*小周期MPPT分辨率*/
#define mppt_middle_pwm_change_limit 2
#define mppt_middle_pwm_change_number 5
#define mppt_pv_dc_souce_judge_vol 15

/************报警阈值***************/
#define MPPT_OUT_OV 560
#define MPPT_OUT_UV 410
#define MPPT_OUT_OP 8500
#define MPPT_OUT_OC 215

#define MPPT_IN_OV 620                 /*******包含PV和DC*******/
#define MPPT_IN_UV 110

#define MPPT_DCIN_OP 6000
#define MPPT_DCIN_OC 100

#define MPPT_PVIN_OP 8500
#define MPPT_PVIN_OC 215
/************报警阈值恢复***************/
#define MPPT_IN_OV_RECOVER 610                 
#define MPPT_IN_UV_RECOVER 115

#define MPPT_OUT_OV_RECOVER 545                 
#define MPPT_OUT_UV_RECOVER 425
#define MPPT_OTP_TEMP_RECOVER 600
/************功率设置限制参数***************/
#define MPPT_OUT_NOMAL_MAX_CURR 200
#define MPPT_PVIN_NOMAL_MAX_CURR 200
#define MPPT_DCIN_NOMAL_MAX_CURR 80
#define MPPT_NONEIN_NOMAL_MAX_CURR 70

#define MPPT_OUT_NOMAL_MAX_POWER 8000
#define MPPT_PVIN_NOMAL_MAX_POWER 8000
#define MPPT_DCIN_NOMAL_MAX_POWER 5200
#define MPPT_NONEIN_NOMAL_MAX_POWER 4500

#define MPPT_HIGE_TEMP_MAX_POWER 2000
/************初始参数***************/
#define MPPT_PVIN_INIT_CURR 100
#define MPPT_DCIN_INIT_CURR 80
#define MPPT_NONEIN_INIT_CURR 70
/*****************END******************/
typedef enum{
	EN_NONE_LOOP,                  /*无闭环*/	
	EN_SINGLE_VOLTAGE_LOOP,				 /*PV电压环*/
	EN_PV_MPPT_LOOP,						   /*PVMPPT*/
	EN_SINGLE_MPPT_LOOP,					 /*PVMPPT*/
	EN_DC_DOUBLE_LOOP              /*DC双闭环*/
}EN_CHARGE_LOGIC;			 

typedef enum{
	EN_IDLE = 0,                  		/*空闲状态*/   	
	EN_OUTPUT_JUDGE,									/*输出判断*/
	EN_fIRST_ALARM_JUDGE,             /*起始警报判断*/    
	EN_INPUT_TYPE_JUDGE,							/*输入类型判断*/
	EN_MPPT_RUN,				              /*运行MPPT*/             
	EN_STOP_CHARGE,                   /*停止充电*/
	EN_FULL_CHARGE,                   /*充满*/
	EN_HARD_FAULT,                    /*硬件故障*/
	EN_SOFT_FAULT,                    /*软件故障*/
	EN_MPPT_ALARM,                    /*MPPT警报*/
    EN_MPPT_FAULT_RECOVER,            /*MPPT故障恢复*/
	EN_STOP,						          		  /*停止状态*/
}EN_LOJIC_STALE;	

typedef enum{
	EN_RUN_INIT = 0,                      /*运行初始化*/  
	EN_RUN_READY,                 		     /*运行准备工作*/ 
	EN_RUN_ALARM_CHECK,                   /*开PWM检查输入输出端是否存在问题*/
	EN_RUN_OUT_EXIST,								     	/*输出电压存在*/	
	EN_RUN_OUT_NOEXIST,							      /*输出电压不存在*/
	EN_RUN_MPPTALARM,                     /*MPPT告警*/	
	EN_RUN_NORMAL,                        /*运行正常*/
	EN_RUN_CHARGE_END,                    /*充电结束*/
	EN_RUN_CHARGE_COVER,                  /*PV充电遮挡*/ 	
	EN_RUN_CHARGE_FULL,                   /*充满*/	
	EN_RUN_STOP				                    /*运行停止*/
}EN_RUN_LOJIC;	


typedef enum{
	EN_CHARGE_STOP = 0,                  		/*停止充电*/   	
	EN_CHARGE,								     	        /*充电中*/	
	EN_CHARGE_END,							            /*充电结束*/
	EN_CHARGE_COVER                         /*PV充电时光伏板被全部遮挡*/
}EN_CHARGE_STATE;	



typedef struct{
	int16_t target_voltage_double_loop;
	int16_t target_voltage_single_loop;
	int16_t double_loop_param_min;
	int16_t voltage_single_param_min;
	int16_t input_max_current_ADC;
	int16_t input_max_power;
	int16_t output_max_current_ADC;
	int16_t output_max_power;
}Loop_Traget_Value_t;

typedef enum
{
	EN_POWER_TYPE_NONE,						/*无电源*/
	EN_POWER_TYPE_PV,						  /*电源类型光伏板*/
	EN_POWER_TYPE_DC, 					  /*电源类型DC电源*/
	EN_POWER_TYPE_MAX_NUM					/*类型最大数*/
}EN_POWER_TYPE;

extern EN_CHARGE_LOGIC charge_logic;

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
extern Loop_Traget_Value_t TragetValue;
/* Exported functions ------------------------------------------------------- */
bool RelayControl(bool flag);

void gNewMPPTRunControl(void);

void MPPTOutCtrl(void);
void MPPTRunJudge(void);



#endif /* __SYS_INFO_H_ */
/************************ (C) COPYRIGHT Nova *************END OF FILE**********/
