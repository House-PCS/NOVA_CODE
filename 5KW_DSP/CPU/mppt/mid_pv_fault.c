/**
******************************************************************************
* @copyright (C) COPYRIGHT Nova
* @file sys_fault.c
* @author Xi'an NovaStar Tech Co.,LTD.
* @version V1.0.0
* @date 2023-8-30
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
#include "mid_pv_fault.h"
#include "modbus_analysis.h"
//#include "sys_info.h"
#include "mid_pv_data_conversion.h"
#include "mid_pv_logic_control.h"
#include "mid_pv_interrupt.h"
ErrorInfo_t         gMpptErrorInfo;
warningparam_t      warningparam;
sysfaultcount_t     faultcount;
EN_TEMP_STATE       gentempstate;
MPPT_alarm_flag gmpptAlarmFlag;
MPPT_alarm_count gmpptAlarmCount;


static bool _AlarmJudge1(int16_t value,int16_t alarm_value,uint8_t *count,uint8_t count_alarm);
static bool _AlarmJudge2(int16_t value,int16_t alarm_value,uint8_t *count,uint8_t count_alarm);
static bool _AlarmJudge3(int16_t value,int16_t alarm_value,int16_t in_curr,int16_t set_curr,uint8_t *count,uint8_t count_alarm);
bool FaultAnalysisProcess(MPPT_alarm_flag *AlarmFlag);
bool MpptAlarmCountComapre(uint8_t actual_count,uint8_t set_count);

bool fault_clear_flag = false;

bool GetFaultClearFlag(void)
{
	return fault_clear_flag;
}

void FaultFlagClear(void)
{
	fault_clear_flag = false;
}


/*******************************************************************************
* @FunctionName _MPPTWarningParmUpdata
* @param para1:       void
* @return
* @note (无)
*
*******************************************************************************/
static void _MPPTWarningParmUpdata(void)
{

    warningparam.mpptwarningparam.mppt_pv_oc = MPPT_PVIN_OC;      /*PV过流阈值*/
    warningparam.mpptwarningparam.mppt_pv_ov = MPPT_IN_OV;      /*PV过压阈值*/
    warningparam.mpptwarningparam.mppt_pv_op = MPPT_PVIN_OP;     /*PV过载阈值 810W*/
    warningparam.mpptwarningparam.mppt_pv_uv = MPPT_IN_UV;      /*PV欠压阈值*/
    
    warningparam.mpptwarningparam.mppt_out_oc = MPPT_OUT_OC;     /*MPPT输出过流阈值*/
    warningparam.mpptwarningparam.mppt_out_ov = MPPT_OUT_OV;     /*MPPT输出过压阈值*/
    warningparam.mpptwarningparam.mppt_out_op = MPPT_OUT_OP;     /*MPPT输出过载阈值*/
    warningparam.mpptwarningparam.mppt_out_uv = MPPT_OUT_UV;     /*MPPT输出欠压阈值*/

    warningparam.mpptwarningparam.mppt_12v_oc = MPPT_DCIN_OC;     /*MPPT12V过流阈值*/
    warningparam.mpptwarningparam.mppt_12v_ov = MPPT_IN_OV;    /*MPPT12V过压阈值*/
    warningparam.mpptwarningparam.mppt_12v_op = MPPT_DCIN_OP;   /*MPPT12V过载阈值*/
    warningparam.mpptwarningparam.mppt_12v_uv = MPPT_IN_UV;    /*MPPT12V欠压阈值*/  
    
}
/*******************************************************************************
* @FunctionName _DCWarningParmUpdata
* @param para1:       void
* @return
* @note (无)
*
*******************************************************************************/
static void _DCWarningParmUpdata(void)
{
    /*所有参数均在原始参数*10*/
  #if DC1_ENABLE 
    warningparam.dc1warningparam.dcin_uv  = 250;          /*DC1输出欠压阈值*/  
    warningparam.dc1warningparam.dcin_ov  = 550;          /*DC1输出过压阈值*/    
    warningparam.dc1warningparam.dcin_oc  = 200;          /*DC1输出过流阈值*/   /*该部分有问题需要修改*/
    warningparam.dc1warningparam.dcin_op  = 8000;          /*DC1输出过载阈值*/

    warningparam.dc1warningparam.dcout_uv = 100; 
    warningparam.dc1warningparam.dcout_ov = 300;
    warningparam.dc1warningparam.dcout_oc = 200;
    warningparam.dc1warningparam.dcout_op = 4800;

    warningparam.dc1warningparam.dc_temp  = 3000;
   #endif     
  #if DC2_ENABLE     
    warningparam.dc2warningparam.dcin_oc  = 100;
    warningparam.dc2warningparam.dcin_ov  = 100;
    warningparam.dc2warningparam.dcin_op  = 100;
    warningparam.dc2warningparam.dcin_uv  = 100;
    warningparam.dc2warningparam.dcout_oc = 100;
    warningparam.dc2warningparam.dcout_ov = 100;
    warningparam.dc2warningparam.dcout_op = 100;
    warningparam.dc2warningparam.dcout_uv = 100;
    warningparam.dc2warningparam.dc_temp  = 100;    
   #endif
    #if DC3_ENABLE 
    warningparam.dc3warningparam.dcin_oc  = 100;
    warningparam.dc3warningparam.dcin_ov  = 100;
    warningparam.dc3warningparam.dcin_op  = 100;
    warningparam.dc3warningparam.dcin_uv  = 100;
    warningparam.dc3warningparam.dcout_oc = 100;
    warningparam.dc3warningparam.dcout_ov = 100;
    warningparam.dc3warningparam.dcout_op = 100;
    warningparam.dc3warningparam.dcout_uv = 100;
    warningparam.dc3warningparam.dc_temp  = 100;
   #endif  
   #if DC4_ENABLE
    warningparam.dc4warningparam.dcin_oc  = 100;
    warningparam.dc4warningparam.dcin_ov  = 100;
    warningparam.dc4warningparam.dcin_op  = 100;
    warningparam.dc4warningparam.dcin_uv  = 100;
    warningparam.dc4warningparam.dcout_oc = 100;
    warningparam.dc4warningparam.dcout_ov = 100;
    warningparam.dc4warningparam.dcout_op = 100;
    warningparam.dc4warningparam.dcout_uv = 100;
    warningparam.dc4warningparam.dc_temp  = 100;
   #endif 
    
}
/*******************************************************************************
* @FunctionName SYSWarningParmUpdata
* @param para1:       void
* @return
* @note (无)
*
*******************************************************************************/
bool SYSFaultUpdata(void)
{
    bool value = false;
    _MPPTWarningParmUpdata();
		if(FaultAnalysisProcess(&gmpptAlarmFlag))
		{
			value = true;
		}
		else
		{
			value = false;
		}
    return value;
}
/*******************************************************************************
* @FunctionName FaultAnalysisProcess
* @param para1:       void
* @return
* @note (无)
*
*******************************************************************************/
bool FaultAnalysisProcess(MPPT_alarm_flag *AlarmFlag)
{
    
    static uint32_t pv_voltage_time      = 0;
    static uint32_t pv_current_time      = 0;
    static uint32_t pv_power_time        = 0;
    static uint32_t mpptout_voltage_time = 0;
    static uint32_t mpptout_current_time = 0;
    static uint32_t mpptout_power_time   = 0;
    static uint32_t mppt12V_voltage_time = 0;
    static uint32_t mppt12V_current_time = 0;
    static uint32_t mppt12V_power_time   = 0;    
    static uint32_t mppt_otp_time        = 0; 
    bool value = false;
  #if DC1_ENABLE
    static uint32_t dc1_in_voltage_time  = 0;
    static uint32_t dc1_in_current_time  = 0;
    static uint32_t dc1_in_power_time    = 0;
    static uint32_t dc1_out_voltage_time = 0;
    static uint32_t dc1_out_current_time = 0;
    static uint32_t dc1_out_power_time   = 0;
    static uint32_t dc1_temp_time        = 0;
    #endif
  #if DC2_ENABLE 
    static uint32_t dc2_in_voltage_time  = 0;
    static uint32_t dc2_in_current_time  = 0;
    static uint32_t dc2_in_power_time    = 0;
    static uint32_t dc2_out_voltage_time = 0;
    static uint32_t dc2_out_current_time = 0;
    static uint32_t dc2_out_power_time   = 0;
    static uint32_t dc2_temp_time        = 0;
    #endif
  #if DC3_ENABLE
    static uint32_t dc3_in_voltage_time  = 0;
    static uint32_t dc3_in_current_time  = 0;
    static uint32_t dc3_in_power_time    = 0;
    static uint32_t dc3_out_voltage_time = 0;
    static uint32_t dc3_out_current_time = 0;
    static uint32_t dc3_out_power_time   = 0;
    static uint32_t dc3_temp_time        = 0;
    #endif
  #if DC4_ENABLE
    static uint32_t dc4_in_voltage_time  = 0;
    static uint32_t dc4_in_current_time  = 0;
    static uint32_t dc4_in_power_time    = 0;
    static uint32_t dc4_out_voltage_time = 0;
    static uint32_t dc4_out_current_time = 0;
    static uint32_t dc4_out_power_time   = 0;
    static uint32_t dc4_temp_time        = 0;
    #endif
    if(GetChargeMode()  == PV_CHARGE)
    {
        /* PV电压 */
        if(TimDifftimeFromNow(pv_voltage_time) > 110)
        {
            pv_voltage_time = TimGetSystime();
            /*PV过压*/
            AlarmFlag -> pv_over_vol_flag =\
            _AlarmJudge1(gsysvalue.mpptvalue.involtage,warningparam.mpptwarningparam.mppt_pv_ov,&(faultcount.mpptfaultcount.pv_ov_count),4);
            /*PV欠压*/        
            AlarmFlag -> pv_under_vol_flag =\
            _AlarmJudge3(gsysvalue.mpptvalue.involtage,warningparam.mpptwarningparam.mppt_pv_uv,gsysvalue.mpptvalue.incurrent,4, &(faultcount.mpptfaultcount.pv_uv_count),4);  
        }    
        /* PV过流 */
        if(TimDifftimeFromNow(pv_current_time) > 120)
        {
            pv_current_time = TimGetSystime();
            AlarmFlag ->pv_over_curr_flag =\
            _AlarmJudge1(gsysvalue.mpptvalue.incurrent,warningparam.mpptwarningparam.mppt_pv_oc,&(faultcount.mpptfaultcount.pv_oc_count),4);      
        }
        /* PV过载 */
        if(TimDifftimeFromNow(pv_power_time) > 130)
        {
            pv_power_time = TimGetSystime();
            AlarmFlag ->pv_over_power_flag =\
            _AlarmJudge1(gsysvalue.mpptvalue.inpower,warningparam.mpptwarningparam.mppt_pv_op,&(faultcount.mpptfaultcount.pv_op_count),4);        
        }         
    }
    else if(GetChargeMode()  == DC_CHARGE)
    {
        /*  MPPT12V电压 */
        if(TimDifftimeFromNow(mppt12V_voltage_time) > 110)
        {
            mppt12V_voltage_time = TimGetSystime();
            /*MPPT_12V过压*/
            AlarmFlag -> dc_over_vol_flag=\
            _AlarmJudge1(gsysvalue.mpptvalue.involtage,warningparam.mpptwarningparam.mppt_12v_ov,&(faultcount.mpptfaultcount.V12_ov_count),4);
            /*MPPT_12V欠压*/        
            AlarmFlag -> dc_under_vol_flag =\
            _AlarmJudge3(gsysvalue.mpptvalue.involtage,warningparam.mpptwarningparam.mppt_12v_uv,gsysvalue.mpptvalue.incurrent,4,&(faultcount.mpptfaultcount.V12_uv_count),4);        
        } 
        /*  MPPT12V过流 */
        if(TimDifftimeFromNow(mppt12V_current_time) > 120)
        {
            mppt12V_current_time = TimGetSystime();
            AlarmFlag ->dc_over_curr_flag =\
            _AlarmJudge1(gsysvalue.mpptvalue.incurrent,warningparam.mpptwarningparam.mppt_12v_oc,&(faultcount.mpptfaultcount.V12_oc_count),4);
        } 
        /*  MPPT12V过载 */
        if(TimDifftimeFromNow(mppt12V_power_time) > 130)
        {
            mppt12V_power_time = TimGetSystime();
            AlarmFlag ->dc_over_power_flag =\
            _AlarmJudge1(gsysvalue.mpptvalue.inpower,warningparam.mpptwarningparam.mppt_12v_op,&(faultcount.mpptfaultcount.V12_op_count),4);       
        }    
    }
    /* MPPT输出电压 */
    if(TimDifftimeFromNow(mpptout_voltage_time) > 140)
    {
        mpptout_voltage_time = TimGetSystime();
        /*输出过压*/
        AlarmFlag ->out_over_vol_flag =\
        _AlarmJudge1(gsysvalue.mpptvalue.outvoltage,warningparam.mpptwarningparam.mppt_out_ov,&(faultcount.mpptfaultcount.out_ov_count),4);
        /*输出欠压*/        
        AlarmFlag ->out_under_vol_flag =\
        _AlarmJudge2(gsysvalue.mpptvalue.outvoltage,warningparam.mpptwarningparam.mppt_out_uv,&(faultcount.mpptfaultcount.out_uv_count),4); 
    }
    /* MPPT输出过流 */
    if(TimDifftimeFromNow(mpptout_current_time) > 150)
    {
        mpptout_current_time = TimGetSystime();
        AlarmFlag ->out_over_curr_flag =\
        _AlarmJudge1(gsysvalue.mpptvalue.outcurrent,warningparam.mpptwarningparam.mppt_out_oc,&(faultcount.mpptfaultcount.out_oc_count),4);       
    }
    /* MPPT输出过载 */
    if(TimDifftimeFromNow(mpptout_power_time) > 160)
    {
        mpptout_power_time = TimGetSystime();
        AlarmFlag ->out_over_power_flag =\
        _AlarmJudge1(gsysvalue.mpptvalue.outpower,warningparam.mpptwarningparam.mppt_out_op,&(faultcount.mpptfaultcount.out_op_count),4);   
    }
    /*  MPPT_MOSotp */
//    if(tim_difftime_from_now(mppt_otp_time) > 3)
//    {
//        mppt_otp_time = tim_get_systime();
//        gMpptErrorInfo.mppt_error1_code.b.mppt_otp_flag =\
//        _AlarmJudge1(gsysvalue.mostemp.q1temp,warningparam.mpptwarningparam.mppt_otp,&(faultcount.mpptfaultcount.otp_count),5);
//        gMpptErrorInfo.mppt_error1_code.b.mppt_otp_flag =\
//        _AlarmJudge1(gsysvalue.mostemp.q2temp,warningparam.mpptwarningparam.mppt_otp,&(faultcount.mpptfaultcount.otp_count),5);        
////        printf("\r\n %d", gMpptErrorInfo.mppt_error1_code.value);         
//    }
    if(GetChargeMode()  == None_InPut)
		{
        /* PV电压 */
        if(TimDifftimeFromNow(pv_voltage_time) > 110)
        {
            pv_voltage_time = TimGetSystime();
            /*PV过压*/
            AlarmFlag -> pv_over_vol_flag =\
            _AlarmJudge1(gsysvalue.mpptvalue.involtage,warningparam.mpptwarningparam.mppt_pv_ov,&(faultcount.mpptfaultcount.pv_ov_count),4);
            /*PV欠压*/        
            AlarmFlag -> pv_under_vol_flag =\
            _AlarmJudge3(gsysvalue.mpptvalue.involtage,warningparam.mpptwarningparam.mppt_pv_uv,gsysvalue.mpptvalue.incurrent,4, &(faultcount.mpptfaultcount.pv_uv_count),4);  
        }    
        /* PV过流 */
        if(TimDifftimeFromNow(pv_current_time) > 120)
        {
            pv_current_time = TimGetSystime();
            AlarmFlag ->pv_over_curr_flag =\
            _AlarmJudge1(gsysvalue.mpptvalue.incurrent,warningparam.mpptwarningparam.mppt_pv_oc,&(faultcount.mpptfaultcount.pv_oc_count),4);      
        }
        /* PV过载 */
        if(TimDifftimeFromNow(pv_power_time) > 130)
        {
            pv_power_time = TimGetSystime();
            AlarmFlag ->pv_over_power_flag =\
            _AlarmJudge1(gsysvalue.mpptvalue.inpower,warningparam.mpptwarningparam.mppt_pv_op,&(faultcount.mpptfaultcount.pv_op_count),4);        
        } 
		}
  #if DC1_ENABLE
    /*  DC1输入电压 */
    if(tim_difftime_from_now(dc1_in_voltage_time) > 500)
    {
        dc1_in_voltage_time = tim_get_systime();
        /*DC1过压*/
        gMpptErrorInfo.dc1_error_code.b.dcin_ov_flag =\
        _AlarmJudge1(gsysvalue.dc1value.invaltage,warningparam.dc1warningparam.dcin_ov,&(faultcount.dc1faultcount.dcin_ov_count),3);
        /*DC1欠压*/        
        gMpptErrorInfo.dc1_error_code.b.dcin_uv_flag =\
        _AlarmJudge2(gsysvalue.dc1value.invaltage,warningparam.dc1warningparam.dcin_uv,&(faultcount.dc1faultcount.dcin_uv_count),5);        
    }
    /*  DC1输入过流 */
    if(tim_difftime_from_now(dc1_in_current_time) > 500)
    {
        dc1_in_current_time = tim_get_systime();
        gMpptErrorInfo.dc1_error_code.b.dcin_oc_flag =\
        _AlarmJudge1(gsysvalue.dc1value.incurrent,warningparam.dc1warningparam.dcin_oc,&(faultcount.dc1faultcount.dcin_oc_count),3);     
    }
    /*  DC1输入过载 */
    if(tim_difftime_from_now(dc1_in_power_time) > 500)
    {
        dc1_in_power_time = tim_get_systime();
        gMpptErrorInfo.dc1_error_code.b.dcin_op_flag =\
        _AlarmJudge1(gsysvalue.dc1value.inpower,warningparam.dc1warningparam.dcin_op,&(faultcount.dc1faultcount.dcin_op_count),3);   
    }
    /*  DC1输出电压 */
    if(tim_difftime_from_now(dc1_out_voltage_time) > 500)
    {
        dc1_out_voltage_time = tim_get_systime();
        /*DC1过压*/
        gMpptErrorInfo.dc1_error_code.b.dcout_ov_flag =\
        _AlarmJudge1(gsysvalue.dc1value.outvaltage,warningparam.dc1warningparam.dcout_ov,&(faultcount.dc1faultcount.dcout_ov_count),3);
        /*DC1欠压*/        
        gMpptErrorInfo.dc1_error_code.b.dcout_uv_flag =\
        _AlarmJudge2(gsysvalue.dc1value.outvaltage,warningparam.dc1warningparam.dcout_uv,&(faultcount.dc1faultcount.dcout_uv_count),3);       
    }
    /*  DC1输出过流 */
    if(tim_difftime_from_now(dc1_out_current_time) > 500)
    {
        dc1_out_current_time = tim_get_systime();
        gMpptErrorInfo.dc1_error_code.b.dcout_oc_flag =\
        _AlarmJudge1(gsysvalue.dc1value.outcurrent,warningparam.dc1warningparam.dcout_oc,&(faultcount.dc1faultcount.dcout_oc_count),3);        
    }
    /*  DC1输出过载 */
    if(tim_difftime_from_now(dc1_out_power_time) > 500)
    {
        dc1_out_power_time = tim_get_systime();
        gMpptErrorInfo.dc1_error_code.b.dcout_op_flag =\
        _AlarmJudge1(gsysvalue.dc1value.outpower,warningparam.dc1warningparam.dcout_op,&(faultcount.dc1faultcount.dcout_op_count),3);         
    }
    /*  DC1_OTP过温 */
    if(tim_difftime_from_now(dc1_temp_time) > 500)        
    {
        dc1_temp_time = tim_get_systime();
        gMpptErrorInfo.dc1_error_code.b.dc_otp_flag =\
        _AlarmJudge1(gsysvalue.dc1value.temp,warningparam.dc1warningparam.dc_temp,&(faultcount.dc1faultcount.dc_temp_count),3);         
    }
    #endif
  #if DC2_ENABLE 
    /*  DC2输入电压 */
    if(tim_difftime_from_now(dc2_in_voltage_time) > 1000)
    {
        dc2_in_voltage_time = tim_get_systime();
        /*DC2过压*/
        gMpptErrorInfo.dc2_error_code.b.dcin_ov_flag =\
        _AlarmJudge1(200,warningparam.dc2warningparam.dcin_ov,&(faultcount.dc2faultcount.dcin_ov_count),5);
        /*DC2欠压*/        
        gMpptErrorInfo.dc2_error_code.b.dcin_uv_flag =\
        _AlarmJudge2(50,warningparam.dc2warningparam.dcin_uv,&(faultcount.dc2faultcount.dcin_uv_count),5);        
    }
    /*  DC2输入过流 */
    if(tim_difftime_from_now(dc2_in_current_time) > 1000)
    {
        dc2_in_current_time = tim_get_systime();
        gMpptErrorInfo.dc2_error_code.b.dcin_oc_flag =\
        _AlarmJudge1(200,warningparam.dc2warningparam.dcin_oc,&(faultcount.dc2faultcount.dcin_oc_count),5);     
    }
    /*  DC2输入过载 */
    if(tim_difftime_from_now(dc2_in_power_time) > 200)
    {
        dc2_in_power_time = tim_get_systime();
        gMpptErrorInfo.dc2_error_code.b.dcin_op_flag =\
        _AlarmJudge1(200,warningparam.dc2warningparam.dcin_op,&(faultcount.dc2faultcount.dcin_op_count),5);   
    }
    /*  DC2输出电压 */
    if(tim_difftime_from_now(dc2_out_voltage_time) > 1000)
    {
        dc2_out_voltage_time = tim_get_systime();
        /*DC2过压*/
        gMpptErrorInfo.dc2_error_code.b.dcout_ov_flag =\
        _AlarmJudge1(200,warningparam.dc2warningparam.dcout_ov,&(faultcount.dc2faultcount.dcout_ov_count),5);
        /*DC2欠压*/        
        gMpptErrorInfo.dc2_error_code.b.dcout_uv_flag =\
        _AlarmJudge2(50,warningparam.dc2warningparam.dcout_uv,&(faultcount.dc2faultcount.dcout_uv_count),5);       
    }
    /*  DC2输出过流 */
    if(tim_difftime_from_now(dc2_out_current_time) > 200)
    {
        dc2_out_current_time = tim_get_systime();
        gMpptErrorInfo.dc2_error_code.b.dcout_oc_flag =\
        _AlarmJudge1(200,warningparam.dc2warningparam.dcout_oc,&(faultcount.dc2faultcount.dcout_oc_count),5);        
    }
    /*  DC2输出过载 */
    if(tim_difftime_from_now(dc2_out_power_time) > 200)
    {
        dc2_out_power_time = tim_get_systime();
        gMpptErrorInfo.dc2_error_code.b.dcout_op_flag =\
        _AlarmJudge1(200,warningparam.dc2warningparam.dcout_op,&(faultcount.dc2faultcount.dcout_op_count),5);         
    }
    /*  DC2_OTP过温 */
    if(tim_difftime_from_now(dc2_temp_time) > 1000)       
    {
        dc2_temp_time = tim_get_systime();
        gMpptErrorInfo.dc2_error_code.b.dc_otp_flag =\
        _AlarmJudge1(gsysvalue.dc2value.temp,warningparam.dc2warningparam.dc_temp,&(faultcount.dc2faultcount.dc_temp_count),5);         
    }
    #endif
  #if DC3_ENABLE 
    /*  DC3输入电压 */
    if(tim_difftime_from_now(dc3_in_voltage_time) > 1000)
    {
        dc3_in_voltage_time = tim_get_systime();
        /*DC3过压*/
        gMpptErrorInfo.dc3_error_code.b.dcin_ov_flag =\
        _AlarmJudge1(200,warningparam.dc3warningparam.dcin_ov,&(faultcount.dc3faultcount.dcin_ov_count),5);
        /*DC3欠压*/        
        gMpptErrorInfo.dc3_error_code.b.dcin_uv_flag =\
        _AlarmJudge2(50,warningparam.dc3warningparam.dcin_uv,&(faultcount.dc3faultcount.dcin_uv_count),5);        
    }
    /*  DC3输入过流 */
    if(tim_difftime_from_now(dc3_in_current_time) > 1000)
    {
        dc3_in_current_time = tim_get_systime();
        gMpptErrorInfo.dc3_error_code.b.dcin_oc_flag =\
        _AlarmJudge1(200,warningparam.dc3warningparam.dcin_oc,&(faultcount.dc3faultcount.dcin_oc_count),5);     
    }
    /*  DC3输入过载 */
    if(tim_difftime_from_now(dc3_in_power_time) > 200)
    {
        dc3_in_power_time = tim_get_systime();
        gMpptErrorInfo.dc3_error_code.b.dcin_op_flag =\
        _AlarmJudge1(200,warningparam.dc3warningparam.dcin_op,&(faultcount.dc3faultcount.dcin_op_count),5);   
    }
    /*  DC3输出电压 */
    if(tim_difftime_from_now(dc3_out_voltage_time) > 1000)
    {
        dc3_out_voltage_time = tim_get_systime();
        /*DC3过压*/
        gMpptErrorInfo.dc3_error_code.b.dcout_ov_flag =\
        _AlarmJudge1(200,warningparam.dc3warningparam.dcout_ov,&(faultcount.dc3faultcount.dcout_ov_count),5);
        /*DC3欠压*/        
        gMpptErrorInfo.dc3_error_code.b.dcout_uv_flag =\
        _AlarmJudge2(50,warningparam.dc3warningparam.dcout_uv,&(faultcount.dc3faultcount.dcout_uv_count),5);       
    }
    /*  DC3输出过流 */
    if(tim_difftime_from_now(dc3_out_current_time) > 200)
    {
        dc3_out_current_time = tim_get_systime();
        gMpptErrorInfo.dc3_error_code.b.dcout_oc_flag =\
        _AlarmJudge1(200,warningparam.dc3warningparam.dcout_oc,&(faultcount.dc3faultcount.dcout_oc_count),5);        
    }
    /*  DC3输出过载 */
    if(tim_difftime_from_now(dc3_out_power_time) > 200)
    {
        dc3_out_power_time = tim_get_systime();
        gMpptErrorInfo.dc3_error_code.b.dcout_op_flag =\
        _AlarmJudge1(200,warningparam.dc3warningparam.dcout_op,&(faultcount.dc3faultcount.dcout_op_count),5);         
    }
    /*  DC3_OTP过温 */
    if(tim_difftime_from_now(dc3_temp_time) > 1000)        
    {
        dc3_temp_time = tim_get_systime();
        gMpptErrorInfo.dc3_error_code.b.dc_otp_flag =\
        _AlarmJudge1(gsysvalue.dc3value.temp,warningparam.dc3warningparam.dc_temp,&(faultcount.dc3faultcount.dc_temp_count),5);         
    }    
    #endif
  #if DC4_ENABLE
    /*  DC4输入电压 */
    if(tim_difftime_from_now(dc4_in_voltage_time) > 1000)
    {
        dc4_in_voltage_time = tim_get_systime();
        /*DC4过压*/
        gMpptErrorInfo.dc4_error_code.b.dcin_ov_flag =\
        _AlarmJudge1(200,warningparam.dc4warningparam.dcin_ov,&(faultcount.dc4faultcount.dcin_ov_count),5);
        /*DC4欠压*/        
        gMpptErrorInfo.dc4_error_code.b.dcin_uv_flag =\
        _AlarmJudge2(50,warningparam.dc4warningparam.dcin_uv,&(faultcount.dc4faultcount.dcin_uv_count),5);        
    }
    /*  DC4输入过流 */
    if(tim_difftime_from_now(dc4_in_current_time) > 1000)
    {
        dc4_in_current_time = tim_get_systime();
        gMpptErrorInfo.dc4_error_code.b.dcin_oc_flag =\
        _AlarmJudge1(200,warningparam.dc4warningparam.dcin_oc,&(faultcount.dc4faultcount.dcin_oc_count),5);     
    }
    /*  DC4输入过载 */
    if(tim_difftime_from_now(dc4_in_power_time) > 200)
    {
        dc4_in_power_time = tim_get_systime();
        gMpptErrorInfo.dc4_error_code.b.dcin_op_flag =\
        _AlarmJudge1(200,warningparam.dc4warningparam.dcin_op,&(faultcount.dc4faultcount.dcin_op_count),5);   
    }
    /*  DC4输出电压 */
    if(tim_difftime_from_now(dc4_out_voltage_time) > 1000)
    {
        dc4_out_voltage_time = tim_get_systime();
        /*DC3过压*/
        gMpptErrorInfo.dc4_error_code.b.dcout_ov_flag =\
        _AlarmJudge1(200,warningparam.dc4warningparam.dcout_ov,&(faultcount.dc4faultcount.dcout_ov_count),5);
        /*DC4欠压*/        
        gMpptErrorInfo.dc4_error_code.b.dcout_uv_flag =\
        _AlarmJudge2(50,warningparam.dc4warningparam.dcout_uv,&(faultcount.dc4faultcount.dcout_uv_count),5);       
    }
    /*  DC4输出过流 */
    if(tim_difftime_from_now(dc4_out_current_time) > 200)
    {
        dc4_out_current_time = tim_get_systime();
        gMpptErrorInfo.dc4_error_code.b.dcout_oc_flag =\
        _AlarmJudge1(200,warningparam.dc4warningparam.dcout_oc,&(faultcount.dc4faultcount.dcout_oc_count),5);        
    }
    /*  DC4输出过载 */
    if(tim_difftime_from_now(dc4_out_power_time) > 200)
    {
        dc4_out_power_time = tim_get_systime();
        gMpptErrorInfo.dc4_error_code.b.dcout_op_flag =\
        _AlarmJudge1(200,warningparam.dc4warningparam.dcout_op,&(faultcount.dc4faultcount.dcout_op_count),5);         
    }
    /*  DC4_OTP过温 */
    if(tim_difftime_from_now(dc4_temp_time) > 1000)      
    {
        dc4_temp_time = tim_get_systime();
        gMpptErrorInfo.dc4_error_code.b.dc_otp_flag =\
        _AlarmJudge1(gsysvalue.dc4value.temp,warningparam.dc4warningparam.dc_temp,&(faultcount.dc4faultcount.dc_temp_count),5);         
    }
    #endif
    
	if(gmpptAlarmFlag.pv_over_vol_flag    || gmpptAlarmFlag.pv_under_vol_flag  || gmpptAlarmFlag.pv_over_curr_flag\
		|| gmpptAlarmFlag.pv_over_power_flag  || gmpptAlarmFlag.dc_over_vol_flag || gmpptAlarmFlag.dc_under_vol_flag\
		|| gmpptAlarmFlag.dc_over_curr_flag || gmpptAlarmFlag.dc_over_power_flag || gmpptAlarmFlag.out_over_vol_flag\
		|| gmpptAlarmFlag.out_under_vol_flag  || gmpptAlarmFlag.out_over_curr_flag  || gmpptAlarmFlag.pv_over_power_flag\
	  || gmpptAlarmFlag.otp_over_temp_alarm_flag)
	{
		value =  true;
	}
return value;
}

void MPPTAlarmState(void)
{
	
	if(gmpptAlarmFlag.relay_first_fault_flag)            
	{
		gmpptAlarmCount.mppt_relay_first_alarm_count ++ ;
		gmpptAlarmFlag.relay_first_fault_flag = false;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_relay_first_alarm_count,1))
		{
			gMpptErrorInfo.mppt_error2_code.b.mppt_relay_fault_flag = true;
		}
	}
	if(gmpptAlarmFlag.relay_run_alarm_flag)           
	{
		gmpptAlarmCount.mppt_relay_run_alarm_count ++ ;
		gmpptAlarmFlag.relay_run_alarm_flag = false;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_relay_run_alarm_count,1))
		{
			gMpptErrorInfo.mppt_error2_code.b.mppt_relay_fault_flag = true;
		}
	}
	if(gmpptAlarmFlag.fan_blockage_alarm_flag)              /*风扇堵转标志*/
	{
		gmpptAlarmCount.mppt_fan_blockage_alarm_count ++;
		gmpptAlarmFlag.fan_blockage_alarm_flag = false;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_fan_blockage_alarm_count,1))
		{
			gMpptErrorInfo.mppt_error2_code.b.mppt_fan_blockage_flag = true;
		}
	}
	if(gmpptAlarmFlag.in_alarm_flag)              
	{
		gmpptAlarmCount.mppt_in_alarm_count ++;
		gmpptAlarmFlag.in_alarm_flag = false;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_in_alarm_count,1))
		{
			gMpptErrorInfo.mppt_error2_code.b.mppt_in_fault_flag = true;
		}
	}
	if(gmpptAlarmFlag.out_alarm_flag)             
	{
		gmpptAlarmCount.mppt_out_alarm_count ++;
		gmpptAlarmFlag.out_alarm_flag = false;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_out_alarm_count,1))
		{
			gMpptErrorInfo.mppt_error2_code.b.mppt_out_fault_flag = true;
		}
	}
	if(gmpptAlarmFlag.otp_over_temp_alarm_flag)              /*超温报警*/
	{
		gmpptAlarmCount.mppt_otp_over_temp_count ++;
		gmpptAlarmFlag.otp_over_temp_alarm_flag = false;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_otp_over_temp_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_otp_flag = true;
		}
	}
	/*输入PV告警处理（运行两次有问题后发送主机）*/
	if(gmpptAlarmFlag.pv_over_vol_flag)                /*PV超压标志*/
	{
		gmpptAlarmCount.mppt_pv_over_vol_count ++;       /*记录总警报次数*/
		gmpptAlarmFlag.pv_over_vol_flag = false;
		faultcount.mpptfaultcount.pv_ov_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_pv_over_vol_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_pv_ov_flag = true;
		}
	}
	if(gmpptAlarmFlag.pv_under_vol_flag)                /*PV欠压标志*/
	{
		gmpptAlarmCount.mppt_pv_under_vol_count ++;
		gmpptAlarmFlag.pv_under_vol_flag = false;
		faultcount.mpptfaultcount.pv_uv_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_pv_under_vol_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_pv_uv_flag = true;
		}
	}
	if(gmpptAlarmFlag.pv_over_curr_flag)                /*PV过流标志*/
	{
		gmpptAlarmCount.mppt_pv_over_curr_count ++;
		gmpptAlarmFlag.pv_over_curr_flag = false;
		faultcount.mpptfaultcount.pv_oc_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_pv_over_curr_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_pv_oc_flag = true;
		}
	}
	if(gmpptAlarmFlag.pv_over_power_flag)               /*PV过载标志*/
	{
		gmpptAlarmCount.mppt_pv_over_power_count ++;
		gmpptAlarmFlag.pv_over_power_flag = false;
		faultcount.mpptfaultcount.pv_op_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_pv_over_power_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_pv_op_flag = true;
		}
	}
	/*输出告警处理（运行两次有问题后发送主机）*/
	if(gmpptAlarmFlag.out_over_vol_flag)               /*输出过压标志*/
	{
		gmpptAlarmCount.mppt_out_over_vol_count ++;
		gmpptAlarmFlag.out_over_vol_flag = false;
		faultcount.mpptfaultcount.out_ov_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_out_over_vol_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_out_ov_flag = true;
		}
	}
	if(gmpptAlarmFlag.out_under_vol_flag)               /*输出欠压标志*/
	{
		gmpptAlarmCount.mppt_out_under_vol_count ++;
		gmpptAlarmFlag.out_under_vol_flag = false;
		faultcount.mpptfaultcount.out_uv_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_out_under_vol_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_out_uv_flag = true;
		}
	}
	if(gmpptAlarmFlag.out_over_curr_flag)               /*输出过流标志*/
	{
		gmpptAlarmCount.mppt_out_over_curr_count ++;
		gmpptAlarmFlag.out_over_curr_flag = false;
		faultcount.mpptfaultcount.out_oc_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_out_over_curr_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_out_oc_flag = true;
		}
	}
	if(gmpptAlarmFlag.out_over_power_flag)               /*输出过载标志*/
	{
		gmpptAlarmCount.mppt_out_over_power_count ++;
		gmpptAlarmFlag.out_over_power_flag = false;
		faultcount.mpptfaultcount.out_op_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_out_over_power_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_out_op_flag = true;
		}
	}
	/*DC源输入报警处理*/
	if(gmpptAlarmFlag.dc_over_vol_flag)                /*DC超压标志*/
	{
		gmpptAlarmCount.mppt_dc_over_vol_count ++;       /*记录总警报次数*/
		gmpptAlarmFlag.dc_over_vol_flag = false;
		faultcount.mpptfaultcount.V12_ov_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_dc_over_vol_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_12_ov_flag = true;
		}
	}
	if(gmpptAlarmFlag.dc_under_vol_flag)                /*DC欠压标志*/
	{
		gmpptAlarmCount.mppt_dc_under_vol_count ++;
		gmpptAlarmFlag.dc_under_vol_flag = false;
		faultcount.mpptfaultcount.V12_uv_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_dc_under_vol_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_12_uv_flag = true;
		}
	}
	if(gmpptAlarmFlag.dc_over_curr_flag)                /*DC过流标志*/
	{
		gmpptAlarmCount.mppt_dc_over_curr_count ++;
		gmpptAlarmFlag.dc_over_curr_flag = false;
		faultcount.mpptfaultcount.V12_oc_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_dc_over_curr_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_12_oc_flag = true;
		}
	}
	if(gmpptAlarmFlag.dc_over_power_flag)               /*DC过载标志*/
	{
		gmpptAlarmCount.mppt_dc_over_power_count ++;
		gmpptAlarmFlag.dc_over_power_flag = false;
		faultcount.mpptfaultcount.V12_op_count = 0;
		if(MpptAlarmCountComapre(gmpptAlarmCount.mppt_dc_over_power_count,1))
		{
			gMpptErrorInfo.mppt_error1_code.b.mppt_12_op_flag = true;
		}
	}
}

bool MpptAlarmCountComapre(uint8_t actual_count,uint8_t set_count)
{
	bool value = false;
	if(actual_count >= set_count)
	{
		value = true;
	}
	else
	{
		value = false;
	}
	return value;
}
void SysAlarmSet(EN_SYS_ALARM sta)
{
	if(sta == EN_SYS_NORMAL)
	{
		gMpptErrorInfo.sys_error_code.value = 0x00;
	}
	else if(sta == EN_HARDEARE_FAULT)
	{
    gMpptErrorInfo.sys_error_code.b.hardware_fault = true;
	}
	else if(sta == EN_SOFTWARE_FAULT)
	{
    gMpptErrorInfo.sys_error_code.b.software_fault = true;
	}
	else if(sta == EN_ALARM)
	{
    gMpptErrorInfo.sys_error_code.b.sys_alarm = true;
	}
}

/*******************************************************************************
* @FunctionName _AlarmJudge1
* @param para1:       (value)
* @param para2:       (alarm_value)
* @param para3:       (count)
* @param para4:       (count_alarm)
* @return
* - 1 报警
* - 0 正常
* @note (无)
*
*******************************************************************************/
static bool _AlarmJudge1(int16_t value,int16_t alarm_value,uint8_t *count,uint8_t count_alarm)
{
    
     uint8_t i = *count;
    
    if(value > alarm_value)
    {
        i++;
    }
    else
    {
        i = 0;
    }     
   
    *count = i;
    if(i > count_alarm)
    {
        return 1;
    }
    else
    {
        return 0;
    }   
    
}
/*******************************************************************************
* @FunctionName _AlarmJudge2
* @param para1:       (value)
* @param para2:       (alarm_value)
* @param para3:       (count)
* @param para4:       (count_alarm)
* @return
* - 1 报警
* - 0 正常
* @note (无)
*
*******************************************************************************/
static bool _AlarmJudge2(int16_t value,int16_t alarm_value,uint8_t *count,uint8_t count_alarm)
{
    
     uint8_t i = *count;
    
    if(value < alarm_value )
    {
        i++;
    }
    else
    {
        i = 0;
    }     
   
    *count = i;
    if(i > count_alarm)
    {
        return 1;
    }
    else
    {
        return 0;
    }  
    
}
/*******************************************************************************
* @FunctionName _AlarmJudge3
* @param para1:       (value)
* @param para2:       (alarm_value)
* @param para3:       (count)
* @param para4:       (count_alarm)
* @return
* - 1 报警
* - 0 正常
* @note (无)
*
*******************************************************************************/
static bool _AlarmJudge3(int16_t value,int16_t alarm_value,int16_t in_curr,int16_t set_curr,uint8_t *count,uint8_t count_alarm)
{
    
     uint8_t i = *count;
    
    if(value < alarm_value && in_curr > set_curr)        
    {
        i++;
    }
    else
    {
        i = 0;
    }     
   
    *count = i;
    if(i > count_alarm)
    {
        return 1;
    }
    else
    {
        return 0;
    }  
    
}
/*检测输入MOS/拉载检测是否存在故障*/
EN_FIRST_DETECT PowerOnDetectFault(int16_t in_alarm_curr , int16_t out_alarm_curr)                       /*上电检测故障*/
{
	static int16_t in_curr[3] = {0};
	static int16_t out_curr[3] = {0};
	static  uint32_t timer_delay_time = 0;
	EN_FIRST_DETECT in_value = 	EN_DETECT_NONE;
	EN_FIRST_DETECT out_value = EN_DETECT_NONE;
	EN_FIRST_DETECT value = EN_DETECT_NONE;
	static uint8_t cont = 0;
//	if(!GetMpptOutCtrlState())                         /*未开启拉载检测*/
//	{
//		if(tim_difftime_from_now(timer_delay_time) > 2)                   
//		{
//				timer_delay_time  = tim_get_systime();
//				cont++;		
//				in_curr[0]  = in_curr[1];
//				in_curr[1]  = in_curr[2];
//				in_curr[2]  = (int16_t)((((float)SampleValueGet(EN_MPPT_IN_CURRENT_GET)  ) / I_CM) * 10);
//			
//				out_curr[0]  = out_curr[1];
//				out_curr[1]  = out_curr[2];
//				out_curr[2]  = (int16_t)((((float)SampleValueGet(EN_MPPT_OUT_CURRENT_GET)  ) / I_CM) * 10);
//		}
//		if(cont >= 3)
//		{
//			if(in_curr[0] >= in_alarm_curr && in_curr[1] >= in_alarm_curr  && in_curr[2] >= in_alarm_curr)                               /*输入故障（包含MOS及拉载检测故障）*/
//			{
//				in_value = EN_IN_DETECT_FAULT;
//			}
//			else if(in_curr[0] <= in_alarm_curr && in_curr[1] <= in_alarm_curr && in_curr[2] <= in_alarm_curr)
//			{
//				in_value = EN_DETECT_CONTINUE_RUN;
//			}
//			else
//			{
//				in_value = EN_DETECT_NONE;
//			}
//			
//			if(out_curr[0] >= out_alarm_curr && out_curr[1] >= out_alarm_curr  && out_curr[2] >= out_alarm_curr)                               /*输入故障（包含MOS及拉载检测故障）*/
//			{
//				out_value = EN_OUT_DETECT_FAULT;
//			}
//			else if(out_curr[0] <= out_alarm_curr && out_curr[1] <= out_alarm_curr && out_curr[2] <= out_alarm_curr)
//			{
//				out_value = EN_DETECT_CONTINUE_RUN;
//			}
//			else
//			{
//				out_value = EN_DETECT_NONE;
//			}
//			
//			if(in_value == EN_DETECT_CONTINUE_RUN && out_value == EN_DETECT_CONTINUE_RUN)
//			{
//				value = EN_DETECT_CONTINUE_RUN;
//			}
//			else if(in_value == EN_IN_DETECT_FAULT || out_value == EN_OUT_DETECT_FAULT)
//			{
//				if(in_value == EN_IN_DETECT_FAULT)
//				{
//					value = EN_IN_DETECT_FAULT;
//				}
//				if(out_value == EN_OUT_DETECT_FAULT)
//				{
//					value = EN_OUT_DETECT_FAULT;
//				}
//				if(in_value == EN_IN_DETECT_FAULT && out_value == EN_OUT_DETECT_FAULT)
//				{
//					value = EN_IN_OUT_DETECT_FAULT;
//				}
//			}
//			else
//			{
//				value = EN_DETECT_NONE;
//			}
//			cont = 0;
//			memset(in_curr,0,sizeof(in_curr));
//			memset(out_curr,0,sizeof(out_curr));
//		}
//	}
	return value;
}
/*运行检测，检测继电器是否存在故障*/
bool RelayFault(int16_t pv_voltage,int16_t mppt_in_voltage,int16_t alarm_voltage)                       /*上电检测故障*/
{
	static uint32_t relay_time = 0;
  bool value = 0;
	static uint16_t relay_cont = 0;
	/*600ms更新一次充电电流*/
	if(TimDifftimeFromNow(relay_time) > 120)
	{
			relay_time = TimGetSystime();
			if(abs(pv_voltage - mppt_in_voltage )> alarm_voltage)
			{
				relay_cont ++;				
			}
			else
			{
				relay_cont = 0;
			}
			if(relay_cont >= 8)       
			{
				relay_cont = 0;
				value = true;
			}  
	}	
	return value;
}

void MPPTFaultClear(void)
{
		gMpptErrorInfo.mppt_error1_code.value = 0x00;   /*清除所有故障*/
		gMpptErrorInfo.mppt_error2_code.value = 0x00;   /*清除所有故障*/
}

void Fault_Clear(void)                              /*故障清除函数*/
{
		fault_clear_flag = true;                         /*MPPT重新检测标志*/
}


void MpptFanControl(void)
{
		bool fan_break_flag = 0;
		//TODO
//		EN_TEMP_LEVEL sta = FanControl(&fan_break_flag,NULL);
		EN_TEMP_LEVEL sta = 0;
		if(fan_break_flag)
		{
			gentempstate = EN_FAN_ERROR; 			
		}
		else if(sta == TEMP_LEVEL_OVER_HIGH)     /*温度报警,系统停机*/
		{
			gentempstate = EN_TEMP_OVER_HIGH; 
			gMpptErrorInfo.mppt_error1_code.b.mppt_otp_flag = true;
		}	
		else if(sta == TEMP_LEVEL_HIGH)     /*风扇转速最大限制功率*/
		{
			gentempstate = EN_TEMP_HIGH; 
		}
}  

EN_TEMP_STATE GetFanState(void)
{
	return gentempstate;
}
