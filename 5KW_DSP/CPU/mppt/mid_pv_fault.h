/**
******************************************************************************
* @copyright (C) COPYRIGHT Nova
* @file sys_fault.h
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
#ifndef __SYS_FAULT_H	
#define __SYS_FAULT_H
#ifdef __cplusplus
extern "C" {
#endif
/* Inculdes ------------------------------------------------------------------*/
//#include "debug.h"
#include "F28x_Project.h"
#ifdef __cplusplus
}
#endif



typedef struct MPPT_Fault_analysis_stru_t
{    
    uint8_t pv_uv_count;
    uint8_t pv_ov_count;
    uint8_t pv_oc_count;
    uint8_t pv_op_count;   
    uint8_t out_uv_count;
    uint8_t out_ov_count;
    uint8_t out_oc_count;
    uint8_t out_op_count;
    uint8_t V12_uv_count;
    uint8_t V12_ov_count;
    uint8_t V12_oc_count;
    uint8_t V12_op_count; 
    uint8_t otp_count;    
}MPPT_Fault_analysis_count;

typedef struct DC_Fault_analysis_stru_t
{    
    uint8_t dcin_uv_count;
    uint8_t dcin_ov_count;
    uint8_t dcin_oc_count;
    uint8_t dcin_op_count;   
    uint8_t dcout_uv_count;
    uint8_t dcout_ov_count;
    uint8_t dcout_oc_count;
    uint8_t dcout_op_count;
    uint8_t dc_temp_count;
}DC_Fault_analysis_count;

typedef struct Sys_fault_count_stru_t
{
    MPPT_Fault_analysis_count      mpptfaultcount;               /* mppt故障计数  */
    DC_Fault_analysis_count        dc1faultcount;                /* DC1故障计数  */
    DC_Fault_analysis_count        dc2faultcount;                /* DC2故障计数 */
    DC_Fault_analysis_count        dc3faultcount;                /* DC3故障计数 */
    DC_Fault_analysis_count        dc4faultcount;                /* DC4故障计数   */
}sysfaultcount_t;

typedef union
{
    uint16_t value;
    struct
    {
				uint16_t sys_alarm             : 1;          /* 系统报警  */
        uint16_t software_fault        : 1;          /* 软件故障  */
        uint16_t hardware_fault        : 1;          /* 硬件故障  */
        uint16_t reserved              : 13;         /* 预留故障 */
    } b;

}SYSErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
				uint16_t mppt_pv_uv_flag        : 1;          /* MPPT PV欠压  */
        uint16_t mppt_pv_ov_flag        : 1;          /* MPPT PV过压  */
        uint16_t mppt_pv_oc_flag        : 1;          /* MPPT PV过流  */
        uint16_t mppt_pv_op_flag        : 1;          /* MPPT PV过载 */
				uint16_t mppt_out_uv_flag       : 1;          /* MPPT输出欠压  */
        uint16_t mppt_out_ov_flag       : 1;          /* MPPT输出过压  */
        uint16_t mppt_out_oc_flag       : 1;          /* MPPT输出过流  */
        uint16_t mppt_out_op_flag       : 1;          /* MPPT输出过载 */        
				uint16_t mppt_12_uv_flag        : 1;          /* MPPT 12V欠压  */
        uint16_t mppt_12_ov_flag        : 1;          /* MPPT 12V过压  */
        uint16_t mppt_12_oc_flag        : 1;          /* MPPT 12V过流  */
        uint16_t mppt_12_op_flag        : 1;          /* MPPT 12V过载 */        
        uint16_t mppt_otp_flag          : 1;          /* MPPT OTP过温 */   
        uint16_t reserved                : 3;          /* 预留 */         
    } b;

}MPPTError1Code_t;

typedef union
{
    uint16_t value;
    struct
    {  
				uint16_t mppt_fan_blockage_flag        : 1;          /* 风扇堵转标志  */
        uint16_t mppt_out_fault_flag           : 1;          /* MPPT 输出故障  */
        uint16_t mppt_in_fault_flag            : 1;          /* MPPT 输入故障  */
        uint16_t mppt_relay_fault_flag         : 1;          /* MPPT 继电器故障 */
        uint16_t reserved                : 12;               /* 预留 */         
    } b;

}MPPTError2Code_t;

typedef union
{
    uint16_t value;
    struct
    {
				uint16_t dcin_uv_flag           : 1;          /* DC输入欠压  */
        uint16_t dcin_ov_flag           : 1;          /* DC输入过压  */
        uint16_t dcin_oc_flag           : 1;          /* DC输入过流  */
        uint16_t dcin_op_flag           : 1;          /* DC输入过载 */
				uint16_t dcout_uv_flag          : 1;          /* DC输出欠压  */
        uint16_t dcout_ov_flag          : 1;          /* DC输出过压  */
        uint16_t dcout_oc_flag          : 1;          /* DC输出过流  */
        uint16_t dcout_op_flag          : 1;          /* DC输出过载 */
        uint16_t dc_otp_flag            : 1;          /* DC_OTP过温 */        
        uint16_t reserved               : 7;          /* 预留 */         
    } b;

}DCErrorCode_t;

typedef struct Error_info_stru_t
{
    SYSErrorCode_t      sys_error_code;               /* 系统故障flag  */
    MPPTError1Code_t    mppt_error1_code;             /* MPPT故障flag  */
    MPPTError2Code_t    mppt_error2_code;             /* 保留  */
    DCErrorCode_t       dc1_error_code;               /* DC24V故障flag  */
    DCErrorCode_t       dc2_error_code;               /* DC12V,10A故障flag  */
    DCErrorCode_t       dc3_error_code;               /* DC3V故障flag  */
    DCErrorCode_t       dc4_error_code;               /* DC4V故障flag  */
}ErrorInfo_t;

typedef struct Mppt_warning_param_stru_t
{
    int16_t mppt_pv_uv ;                                       
    int16_t mppt_pv_ov ;
    int16_t mppt_pv_oc ;
    int16_t mppt_pv_op ;    
    
    int16_t mppt_out_uv ;
    int16_t mppt_out_ov ;
    int16_t mppt_out_oc ;
    int16_t mppt_out_op ;    
    
    int16_t mppt_12v_uv ;
    int16_t mppt_12v_ov ;
    int16_t mppt_12v_oc ;
    int16_t mppt_12v_op ; 
    
    int16_t mppt_otp ;    
}mpptwarningparam_t;

typedef struct DC_warning_param_stru_t
{
    int16_t dcin_uv ;
    int16_t dcin_ov ;
    int16_t dcin_oc ;
    int16_t dcin_op ;    
    
    int16_t dcout_uv ;
    int16_t dcout_ov ;
    int16_t dcout_oc ;
    int16_t dcout_op ;     
    int16_t dc_temp ;        
}dcwarningparam_t;

typedef struct Warning_param_stru_t
{
    mpptwarningparam_t      mpptwarningparam;               /* mppt报警阈值  */
    dcwarningparam_t        dc1warningparam;                /* DC1报警阈值  */
    dcwarningparam_t        dc2warningparam;                /* DC2报警阈值 */
    dcwarningparam_t        dc3warningparam;                /* DC3报警阈值 */
    dcwarningparam_t        dc4warningparam;                /* DC4报警阈值   */
}warningparam_t;

typedef enum{
	EN_DETECT_NONE = 0,                  		                 /*不执行动作*/   		
	EN_IN_DETECT_FAULT,							                         /*输入故障*/
	EN_OUT_DETECT_FAULT,                                     /*输出故障*/
	EN_IN_OUT_DETECT_FAULT,                                  /*输入输出故障*/
	EN_DETECT_CONTINUE_RUN                                   /*继续运行*/
}EN_FIRST_DETECT;	

typedef enum{
	EN_SYS_NORMAL = 0,                  		              /*系统正常运行*/   		
	EN_HARDEARE_FAULT,							                      /*系统硬件故障*/
	EN_SOFTWARE_FAULT,                                    /*系统软件故障*/
	EN_ALARM                                              /*系统报警*/
}EN_SYS_ALARM;	

typedef enum{
	EN_TEMP_NORMAL  = 0,                                  /*温度正常*/                    
	EN_TEMP_HIGH,                  		                    /*温度高，降功率*/   		
	EN_TEMP_OVER_HIGH,							                      /*温度告警*/
	EN_FAN_ERROR,                                         /*风扇错误*/                                           /*系统报警*/
}EN_TEMP_STATE;

/*警报发生后重新运行又触发该警报次数*/
typedef struct MPPT_alarm_count_stru_t
{    
    uint8_t mppt_relay_first_alarm_count;
    uint8_t mppt_relay_run_alarm_count;
    uint8_t mppt_fan_blockage_alarm_count;
    uint8_t mppt_in_alarm_count;
    uint8_t mppt_out_alarm_count;
    uint8_t mppt_otp_over_temp_count;
    uint8_t mppt_pv_over_vol_count;   
    uint8_t mppt_pv_under_vol_count;
    uint8_t mppt_pv_over_curr_count;
    uint8_t mppt_pv_over_power_count;
    uint8_t mppt_out_over_vol_count;
    uint8_t mppt_out_under_vol_count;
    uint8_t mppt_out_over_curr_count;
    uint8_t mppt_out_over_power_count;
    uint8_t mppt_dc_over_vol_count;   
    uint8_t mppt_dc_under_vol_count;
    uint8_t mppt_dc_over_curr_count;
    uint8_t mppt_dc_over_power_count;
}MPPT_alarm_count;
/*单次触发警报标志*/
typedef struct MPPT_alarm_flag_stru_t
{    
    bool relay_first_fault_flag;
    bool relay_run_alarm_flag;
    bool fan_blockage_alarm_flag;
    bool in_alarm_flag;
    bool out_alarm_flag;	
    bool otp_over_temp_alarm_flag;
    bool pv_over_vol_flag;   
    bool pv_under_vol_flag;
    bool pv_over_curr_flag;
    bool pv_over_power_flag;
    bool out_over_vol_flag;
    bool out_under_vol_flag;
    bool out_over_curr_flag;
    bool out_over_power_flag;
    bool dc_over_vol_flag;   
    bool dc_under_vol_flag;
    bool dc_over_curr_flag;
    bool dc_over_power_flag;
}MPPT_alarm_flag;



extern MPPT_alarm_flag gmpptAlarmFlag;
extern MPPT_alarm_count gmpptAlarmCount;
extern ErrorInfo_t gMpptErrorInfo;
extern EN_TEMP_STATE       gentempstate;

bool SYSFaultUpdata(void);
void Fault_Clear(void);
uint8_t InputJudgeAlarm(int16_t alarm_curr);
bool GetFaultClearFlag(void);
void FaultFlagClear(void);
void MpptFanControl(void);
bool RelayFault(int16_t pv_voltage,int16_t mppt_in_voltage,int16_t alarm_voltage) ;
EN_FIRST_DETECT PowerOnDetectFault(int16_t in_alarm_curr , int16_t out_alarm_curr) ;                       /*上电检测故障*/ 
void MPPTFaultClear(void);
EN_TEMP_STATE GetFanState(void);
void SysAlarmSet(EN_SYS_ALARM sta);
void MPPTAlarmState(void);
#endif /* __SYS_FAULT_H */
/************************ (C) COPYRIGHT Nova *************END OF FILE**********/ 
