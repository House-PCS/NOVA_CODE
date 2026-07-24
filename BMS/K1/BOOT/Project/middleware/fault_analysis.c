

/* Includes ------------------------------------------------------------------*/
#include "fault_analysis.h"
#define LOG_TAG     "fault_analysis"

#define LIMIT_CHARGE_CURRENT    BMSWarningParam.limit_charge_current    /* 200 */
#define LIMIT_DISCHARGE_CURRENT BMSWarningParam.limit_discharge_current /* -200 */

#define MOS_OT1                	BMSWarningParam.mos_ot1                /* 750   */
#define MOS_OT2                	BMSWarningParam.mos_ot2                /* 850   */
#define MOS_OT3                	BMSWarningParam.mos_ot3                /* 950   */

#define CELL_UV1                BMSWarningParam.cell_uv1                /* 2700  */
#define CELL_UV2                BMSWarningParam.cell_uv2                /* 2600  */
#define CELL_UV3                BMSWarningParam.cell_uv3                /* 2500  */
#define CELL_OV1                BMSWarningParam.cell_ov1                /* 3550  */
#define CELL_OV2                BMSWarningParam.cell_ov2                /* 3600  */
#define CELL_OV3                BMSWarningParam.cell_ov3                /* 3650  */
#define PACK_OV1                BMSWarningParam.pack_ov1                /* 2700*串数 */
#define PACK_UV1                BMSWarningParam.pack_uv1                /* 3550*串数 */
#define CELL_V_DIFF1            BMSWarningParam.cell_voltage_diff		/* 350 */

#define DISCHARGE_OVERCURR1     BMSWarningParam.discharge_overcurr1     /* 110   */
#define DISCHARGE_OVERCURR2     BMSWarningParam.discharge_overcurr2     /* 120   */
#define DISCHARGE_OVERCURR3     BMSWarningParam.discharge_overcurr3     /* 130   */
#define CHARGE_OVERCURR1        BMSWarningParam.charge_overcurr1        /* 105   */
#define CHARGE_OVERCURR2        BMSWarningParam.charge_overcurr2        /* 110   */
#define CHARGE_OVERCURR3        BMSWarningParam.charge_overcurr3        /* 115   */

#define CELL_UT1                BMSWarningParam.cell_ut1                /* -100  */
#define CELL_OT1                BMSWarningParam.cell_ot1                /* 550   */
#define CELL_OT2                BMSWarningParam.cell_ot2                /* 600   */
#define CELL_OT3                BMSWarningParam.cell_ot3                /* 650   */
#define NTC_T_DIFF              BMSWarningParam.ntc_temp_diff           /* 100   */

#define SOH_UNDER_LVL1          BMSWarningParam.soh_under_lvl1
#define SOH_UNDER_LVL2          BMSWarningParam.soh_under_lvl2
#define SOH_UNDER_LVL3          BMSWarningParam.soh_under_lvl3

#define CELL_UT                 BMSWarningParam.cell_ut                 /*  10  */
#define TSENSOR_UT              BMSWarningParam.tsensor_ut              /* -400 */
#define TSENSOR_OT              BMSWarningParam.tsensor_ot              /* 1200 */
#define ZERODRIFT_CURRENT       BMSWarningParam.zerodrift_current       /* 2000 */
                                                                               
Fault_analysis_flag gFaultAnalysisFlag;
BMSErrorInfo_t  gBmsErrorInfo;
BMSWarningParam_t  BMSWarningParam;

static void _BMSWarningParamUpdate(void)
{
		 BMSWarningParam.limit_charge_current    = 30;
         BMSWarningParam.limit_discharge_current = 30;
		 BMSWarningParam.mos_ot1                 = garrMODBUS_Reg[0].mos_ot_lvl1;
         BMSWarningParam.mos_ot2                 = garrMODBUS_Reg[0].mos_ot_lvl2;
         BMSWarningParam.mos_ot3                 = garrMODBUS_Reg[0].mos_ot_lvl3;
		
         BMSWarningParam.cell_uv1                = garrMODBUS_Reg[0].cell_uv_lvl1;
         BMSWarningParam.cell_uv2                = garrMODBUS_Reg[0].cell_uv_lvl2;
         BMSWarningParam.cell_uv3                = garrMODBUS_Reg[0].cell_uv_lvl3;
         BMSWarningParam.cell_ov1                = garrMODBUS_Reg[0].cell_ov_lvl1;
         BMSWarningParam.cell_ov2                = garrMODBUS_Reg[0].cell_ov_lvl2;
         BMSWarningParam.cell_ov3                = garrMODBUS_Reg[0].cell_ov_lvl3;
         BMSWarningParam.pack_uv1                = ((uint32_t)garrMODBUS_Reg[0].pack_uv_lvl_high << 16) + garrMODBUS_Reg[0].pack_uv_lvl_low;         
		 BMSWarningParam.pack_ov1                = ((uint32_t)garrMODBUS_Reg[0].pack_ov_lvl_high << 16) + garrMODBUS_Reg[0].pack_ov_lvl_low;
         BMSWarningParam.cell_voltage_diff      = garrMODBUS_Reg[0].cell_voltage_diff;

		 	 
         BMSWarningParam.discharge_overcurr1     = garrMODBUS_Reg[0].dsg_oc_lvl1;
         BMSWarningParam.discharge_overcurr2     = garrMODBUS_Reg[0].dsg_oc_lvl2;
         BMSWarningParam.discharge_overcurr3     = garrMODBUS_Reg[0].dsg_oc_lvl3;
         BMSWarningParam.charge_overcurr1        = garrMODBUS_Reg[0].chg_oc_lvl1;
         BMSWarningParam.charge_overcurr2        = garrMODBUS_Reg[0].chg_oc_lvl2;
         BMSWarningParam.charge_overcurr3        = garrMODBUS_Reg[0].chg_oc_lvl3;
		 
         BMSWarningParam.cell_ut1                = (int16_t)garrMODBUS_Reg[0].cell_ut_lvl;
		 BMSWarningParam.cell_ut2                = (int16_t)garrMODBUS_Reg[0].cell_ut_lvl;
		 BMSWarningParam.cell_ut3                = (int16_t)garrMODBUS_Reg[0].cell_ut_lvl;	  
         BMSWarningParam.cell_ot1                = garrMODBUS_Reg[0].cell_ot_lvl1;
         BMSWarningParam.cell_ot2                = garrMODBUS_Reg[0].cell_ot_lvl2;
         BMSWarningParam.cell_ot3                = garrMODBUS_Reg[0].cell_ot_lvl3;
         BMSWarningParam.ntc_temp_diff           = garrMODBUS_Reg[0].cell_temp_diff;

         BMSWarningParam.soh_under_lvl1          = garrMODBUS_Reg[0].soh_under_lvl1;
         BMSWarningParam.soh_under_lvl2          = garrMODBUS_Reg[0].soh_under_lvl2;  
         BMSWarningParam.soh_under_lvl3          = garrMODBUS_Reg[0].soh_under_lvl3;		 
         BMSWarningParam.tsensor_ut              = -400       ;
         BMSWarningParam.tsensor_ot              = 1200       ;
         BMSWarningParam.zerodrift_current       = 2000       ;    
    
}
/****************bms故障故障判断函数***********************/
void FaultAnalysisProcess(void)
{
    static uint32_t mos_time = 0;
    static uint32_t pmos_time = 0;
    static uint32_t current_time = 0;
    static uint32_t voltage_time = 0;
    static uint32_t temperature_time = 0;
    static uint32_t cell_voltage_diff_time = 0;    
    static uint32_t soh_under_time = 0;    
    static uint32_t vsensor_error_time = 0;  
    static uint32_t tsensor_error_time = 0;  
    static uint32_t tsensor_error_time1 = 0;    
    static uint32_t mos_sensor_error_time = 0;  
    static uint32_t zerodrift_time = 0; 
    static uint32_t pmos_fail_close_time = 0; 
    static uint32_t c_dmos_fail_close_time = 0; 
    static uint32_t rtc_error_time = 0; 
    static uint32_t rtc_times = 0;
    static uint32_t mos_adhesion_time = 0;
	static uint32_t	heat_time=0;
    
    int32_t pack_current = 0;
    PackStatus_t *pPackStatus = NULL;
    
    if(gSoxDebugParam.sox_debug_mode)//判断是否是调试模式
    {
        pPackStatus = &gPackStatus_debug;
    }
    else
    {
        pPackStatus = &gPackStatus;
    }
    
    if(pPackStatus->pack_current < 0)//判断电流是正还是负值  负值转换为正值
    {
        pack_current = -pPackStatus->pack_current;
    }
    else
    {
        pack_current = pPackStatus->pack_current;
    }
    
    /* debug Temp */
    _BMSWarningParamUpdate();//更新各种故障类型的阈值参数

/************************************MOS故障*************************************/
	/* 放电 或预充 或充电MOS粘连故障 */ /* 加热MOS粘连故障 */
    if(tim_difftime_from_now(mos_adhesion_time) > 200)
    {
        mos_adhesion_time = tim_get_systime();
        /* 充放电MOs都没打开且放电电流大于2000mA || 充电Mos没有打开且充电电流大于2000mA */
        if(((!gFaultAnalysisFlag.premos_enable_flag && !gFaultAnalysisFlag.dischargemos_enable_flag) && pPackStatus->pack_current > 2000) ||\
           (!gFaultAnalysisFlag.chargemos_enable_flag && pPackStatus->pack_current < -2000))
        {
            gFaultAnalysisFlag.mos_adhesion_count++;
        }
        else
        {
            gFaultAnalysisFlag.mos_adhesion_count = 0;
        }
    }
	
    if(gFaultAnalysisFlag.mos_adhesion_count > 20)
    {
        LOG_DEBUG(LOG_TAG, "mos_adhesion===========================================================================\r\n");
        gBmsErrorInfo.mos_error_code.b.mos_adhesion = 1;
    }
    
	/* 预充MOS无法闭合故障 */
	if(gFaultAnalysisFlag.premos_enable_flag)//premos_enable_flag=1 已打开 premos_enable_flag=0 关闭
    {
        /* count_flag = 0开始计时 */
        if(!gFaultAnalysisFlag.pmos_count_flag)
        {
            gFaultAnalysisFlag.pmos_count_flag = 1;
            pmos_time = tim_get_systime();
        }
        
        /* 出现此故障不再清除，需下电恢复 */
        if(!gBmsErrorInfo.other1_error_code.b.pre_charge_timeout)
        {
            if(tim_difftime_from_now(pmos_time) > 20000)      /* 预充时间过长故障 */
            {
                gBmsErrorInfo.other1_error_code.b.pre_charge_timeout = 1;
            }
            else
            {
                gBmsErrorInfo.other1_error_code.b.pre_charge_timeout = 0;
            }        
        }
    }
    else    
    {
        gFaultAnalysisFlag.pmos_count_flag = 0;
    }
    
    if(tim_difftime_from_now(c_dmos_fail_close_time) > 1000)//
    {
        mos_sensor_error_time = tim_get_systime();
        /* 充电MOS无法闭合故障 */
        if(gFaultAnalysisFlag.chargemos_enable_flag && gFaultAnalysisFlag.premos_enable_flag == 0)
        {
            /* 放电模式下 */
            if(pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE)
            {
                if(gCell_Info.total_cell_voltage - PackVotageGet() > 1000)
                {   
                    gFaultAnalysisFlag.cmos_fail_close_count ++;                
                }
                else
                {
                    gFaultAnalysisFlag.cmos_fail_close_count = 0;
                }            
            }
            
            if(gFaultAnalysisFlag.cmos_fail_close_count > 4)
            {
                gBmsErrorInfo.mos_error_code.b.cmos_fail_close = 1;//次数修改
            }
            else
            {
                gBmsErrorInfo.mos_error_code.b.cmos_fail_close = 0;
            }   
        }
        
        /* 放电MOS无法闭合故障 */
        if(gFaultAnalysisFlag.dischargemos_enable_flag && gFaultAnalysisFlag.premos_enable_flag == 0)
        {
            /* 充电模式下 */
            if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
            {
                if(PackVotageGet() - gCell_Info.total_cell_voltage > 1000)
                {   
                    gFaultAnalysisFlag.dmos_fail_close_count ++;                
                }
                else
                {
                    gFaultAnalysisFlag.dmos_fail_close_count = 0;
                }            
            }
            
            if(gFaultAnalysisFlag.dmos_fail_close_count > 4)
            {
                gBmsErrorInfo.mos_error_code.b.dmos_fail_close = 1;//此处修改
            }
            else
            {
                gBmsErrorInfo.mos_error_code.b.dmos_fail_close = 0;
            }   
        }
    }
	
    /* MOS温度传感器故障 */
    if(tim_difftime_from_now(mos_sensor_error_time) > 1000)
    {     
        mos_sensor_error_time = tim_get_systime();
        // 放电mos温度低于-40  或者   放电mos温度高于120   
        if(gCell_Info.mos_temperature < TSENSOR_UT || gCell_Info.mos_temperature > TSENSOR_OT)
        {
            gFaultAnalysisFlag.mos_temperature_sensor_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.mos_temperature_sensor_error_count = 0;
        }

        if(gFaultAnalysisFlag.mos_temperature_sensor_error_count > 4) 
        {
            gBmsErrorInfo.mos_error_code.b.mos_ntc_error = 1;
        }
    }
	
    if(tim_difftime_from_now(mos_time) > 1000)
    {     
        mos_time = tim_get_systime();
		gCell_Info.mos_temperature = (TemperatureGet(ADC_CHANNEL_12) * 10);
        /* MOS过温1级 */
        if(gCell_Info.mos_temperature > MOS_OT1)
        {
            gFaultAnalysisFlag.mos_temperature_ov1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.mos_temperature_ov1_count = 0;
        }
        if(gFaultAnalysisFlag.mos_temperature_ov1_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper1 = 0;
        }
        
        /* MOS过温2级 */
        if(gCell_Info.mos_temperature > MOS_OT2)
        {
            gFaultAnalysisFlag.mos_temperature_ov2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.mos_temperature_ov2_count = 0;
        }
        if(gFaultAnalysisFlag.mos_temperature_ov2_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper2 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper2 = 0;
        }

        /* MOS过温3级 */
        if(gCell_Info.mos_temperature > MOS_OT3)
        {
            gFaultAnalysisFlag.mos_temperature_ov3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.mos_temperature_ov3_count = 0;
        }
        if(gFaultAnalysisFlag.mos_temperature_ov3_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper3 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper3 = 0;
        }
    }
/********************************************************************************/	
/************************************电芯故障*************************************/	
	if(tim_difftime_from_now(voltage_time) > 1000)
    {    
        voltage_time = tim_get_systime();        
        /* 单体欠压1级 */ 
        if(gCell_Info.cell_min_voltage < CELL_UV1)
        {
            gFaultAnalysisFlag.cell_under_votage1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol1 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol1 = 0;
        }
        
        /* 单体欠压2级 */
        if(gCell_Info.cell_min_voltage < CELL_UV2)
        {
            gFaultAnalysisFlag.cell_under_votage2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage2_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol2 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol2 = 0;
        }
        
        /* 单体欠压3级 */
         if(gCell_Info.cell_min_voltage < CELL_UV3)
        {
            gFaultAnalysisFlag.cell_under_votage3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage3_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol3 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol3 = 0;
        }
		/* 单体过压1级 */ 
        if(gCell_Info.cell_max_voltage > CELL_OV1)
        {
            gFaultAnalysisFlag.cell_over_votage1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol1 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol1 = 0;
        }
        
        /* 单体过压2级 */
        if(gCell_Info.cell_max_voltage > CELL_OV2)
        {
            gFaultAnalysisFlag.cell_over_votage2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage2_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol2 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol2 = 0;
        }
        
        /* 单体过压3级 */
        if(gCell_Info.cell_max_voltage > CELL_OV3)
        {
            gFaultAnalysisFlag.cell_over_votage3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage3_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol3 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol3 = 0;
        }

		/* 电池包总电压欠压故障 */
        if(gCell_Info.total_cell_voltage < PACK_UV1)
        {
            gFaultAnalysisFlag.pack_under_vol1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.pack_under_vol1_count = 0;
        }
        if(gFaultAnalysisFlag.pack_under_vol1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.pack_under_vol1 = 1;           
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.pack_under_vol1 = 0;           
        }
		
        /* 电池包总电压过压故障 */
        if(gCell_Info.total_cell_voltage > PACK_OV1)
        {
            gFaultAnalysisFlag.pack_over_vol1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.pack_over_vol1_count = 0;
        }
        if(gFaultAnalysisFlag.pack_over_vol1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.pack_over_vol1 = 1;               
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.pack_over_vol1 = 0;               
        }

    }

/************************************电流故障*************************************/	
	if(tim_difftime_from_now(current_time) > 500)
    {    
        current_time = tim_get_systime();
        
        if(pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE)
        {
            /* 放电过流1级 */ 
//            if(pack_current > 105000)
            if(pack_current > DISCHARGE_OVERCURR1 * 0.01 * gPackStatus.cur_max_dsg_current_60s && gPackStatus.cur_max_dsg_current_60s >= 100000)
            {
                gFaultAnalysisFlag.dsg_over_current1_count ++;
            }
            else
            {
                gFaultAnalysisFlag.dsg_over_current1_count = 0;
            }
            if(gFaultAnalysisFlag.dsg_over_current1_count > 4)
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr1 = 1;
            }
            else
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr1 = 0;
            }
            
            /* 放电过流2级 */
//            if(pack_current > 110000)
            if(pack_current > DISCHARGE_OVERCURR2 * 0.01 * gPackStatus.cur_max_dsg_current_60s && gPackStatus.cur_max_dsg_current_60s >= 100000)
            {
                gFaultAnalysisFlag.dsg_over_current2_count ++;
            }
            else
            {
                gFaultAnalysisFlag.dsg_over_current2_count = 0;
            }
            if(gFaultAnalysisFlag.dsg_over_current2_count > 4)
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr2 = 1;
            }
            else
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr2 = 0;
            }
            
            /* 放电过流3级 */
//            if(pack_current > 120000)
            if(pack_current > DISCHARGE_OVERCURR3 * 0.01 * gPackStatus.cur_max_dsg_current_60s && gPackStatus.cur_max_dsg_current_60s >= 100000)
            {
                gFaultAnalysisFlag.dsg_over_current3_count ++;
            }
            else
            {
                gFaultAnalysisFlag.dsg_over_current3_count = 0;
            }
            if(gFaultAnalysisFlag.dsg_over_current3_count > 4)
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr3 = 1;
            }
            else
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr3 = 0;
            }        
        }
        else if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
        {
            /* 充电过流1级 */ 
//            if(pack_current > 105000)
            if(pack_current > CHARGE_OVERCURR1 * 0.01 * gPackStatus.cur_max_chg_current && gPackStatus.cur_max_chg_current >= 100000)
            {
                gFaultAnalysisFlag.chg_over_current1_count ++;
            }
            else
            {
                gFaultAnalysisFlag.chg_over_current1_count = 0;
            }
            if(gFaultAnalysisFlag.chg_over_current1_count > 4)
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr1 = 1;
            }
            else
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr1 = 0;
            }
            
            /* 充电过流2级 */
//            if(pack_current > 110000)
            if(pack_current > CHARGE_OVERCURR2 * 0.01 * gPackStatus.cur_max_chg_current && gPackStatus.cur_max_chg_current >= 100000)
            {
                gFaultAnalysisFlag.chg_over_current2_count ++;
            }
            else
            {
                gFaultAnalysisFlag.chg_over_current2_count = 0;
            }
            if(gFaultAnalysisFlag.chg_over_current2_count > 10 || gFaultAnalysisFlag.chg_over_current1_5s_flag)
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr2 = 1;
            }
            else
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr2 = 0;
            }
            
            /* 充电过流3级 */
//            if(pack_current > 120000)
            if(pack_current > CHARGE_OVERCURR3 * 0.01 * gPackStatus.cur_max_chg_current && gPackStatus.cur_max_chg_current >= 100000)
            {
                gFaultAnalysisFlag.chg_over_current3_count ++;
            }
            else
            {
                gFaultAnalysisFlag.chg_over_current3_count = 0;
            }
            if(gFaultAnalysisFlag.chg_over_current3_count > 5)
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr3 = 1;
            }
            else
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr3 = 0;
            }    
        }

    }  	
	
/********************************************************************************/	
	
	if(tim_difftime_from_now(temperature_time) > 1000)
    {    
        temperature_time = tim_get_systime();        
        /* 电池温度过高一级报警 */ 
        if(gCell_Info.cell_max_temperature > CELL_OT1)
        {
            gFaultAnalysisFlag.cell_temperature_ot1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ot1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ot1_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper1 = 0;
        }
        
        /* 电池温度过高二级报警 */
        if(gCell_Info.cell_max_temperature > CELL_OT2)
        {
            gFaultAnalysisFlag.cell_temperature_ot2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ot2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ot2_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper2 = 1;
        }
        else
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper2 = 0;
        }
        
        /* 电池温度过高三级报警 */
        if(gCell_Info.cell_max_temperature > CELL_OT3)
        {
            gFaultAnalysisFlag.cell_temperature_ot3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ot3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ot3_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper3 = 1;
        }
        else
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper3 = 0;
        }

        /* 电池温度过低故障  */
        if(gCell_Info.cell_min_temperature < CELL_UT1)
        {
            gFaultAnalysisFlag.cell_temperature_ut1_count ++;        
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ut1_count = 0;        
        }
        if(gFaultAnalysisFlag.cell_temperature_ut1_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.cell_low_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.temper_error_code.b.cell_low_temper1 = 0;
        }
		/* 电池NTC温差大故障  */
		if(gCell_Info.cell_max_temperature-gCell_Info.cell_min_temperature > NTC_T_DIFF)
        {
            gFaultAnalysisFlag.ntc_temperature_ot_count ++;        
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ut1_count = 0;        
        }
        if(gFaultAnalysisFlag.ntc_temperature_ot_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.ntc_high_diff = 1;
        }
        else
        {
            gBmsErrorInfo.temper_error_code.b.ntc_high_diff = 0;
        }
	}
    
    if(tim_difftime_from_now(cell_voltage_diff_time) > 1000)
    {
        cell_voltage_diff_time = tim_get_systime();
		/* 电芯电压不均衡  单体压差大 */
        if((gCell_Info.cell_max_voltage - gCell_Info.cell_min_voltage) > CELL_V_DIFF1)
        {
            gFaultAnalysisFlag.cell_voltage_diff1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_voltage_diff1_count = 0;
            
        }
        
        if(gFaultAnalysisFlag.cell_voltage_diff1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_voltage_diff1 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_voltage_diff1 = 0;
        }

    }  	
	
    if(tim_difftime_from_now(soh_under_time) > 1000)
    {
        soh_under_time = tim_get_systime();
		/* SOH过低一级 */ 
        if(gPackStatus.soh_rpt < SOH_UNDER_LVL1)
        {
            gFaultAnalysisFlag.soh_under_lvl1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.soh_under_lvl1_count = 0;
            
        }
 		/* SOH过低二级 */      
        if(gPackStatus.soh_rpt < SOH_UNDER_LVL2)
        {
            gFaultAnalysisFlag.soh_under_lvl2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.soh_under_lvl2_count = 0;
            
        }
		/* SOH过低3级 */ 		
		if(gPackStatus.soh_rpt < SOH_UNDER_LVL3)
        {
            gFaultAnalysisFlag.soh_under_lvl3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.soh_under_lvl3_count = 0;
            
        }
        
        if(gFaultAnalysisFlag.soh_under_lvl1_count > 4)
        {
            gBmsErrorInfo.state_error_code.b.soh_low_error1 = 1;
        }
        if(gFaultAnalysisFlag.soh_under_lvl2_count > 4)
        {
            gBmsErrorInfo.state_error_code.b.soh_low_error2 = 1;
        }
        if(gFaultAnalysisFlag.soh_under_lvl3_count > 4)
        {
            gBmsErrorInfo.state_error_code.b.soh_low_error3 = 1;
        }
       
    }     
	
/********************************************************************************/	
/************************************预充故障*************************************/	

	/* 预充电流反向（见预充MOS无法闭合故障） */
	/* 预充时间过长（见预充MOS无法闭合故障） */
	/* 预充短路3次(需从PCS获取外侧高压值)x  */
	/* 预充短路 (需从PCS获取外侧高压值)x    */
	
/********************************************************************************/	

/************************************电芯采样故障*************************************/

	/* 电压传感器故障 */
    if(tim_difftime_from_now(vsensor_error_time) > 1000)
    {
        vsensor_error_time = tim_get_systime();
        if(gCell_Info.cell_max_voltage > 4000 || gCell_Info.cell_min_voltage < 2000)
        {
            gFaultAnalysisFlag.cell_votage_sensor_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_votage_sensor_error_count = 0;
            
        }
        
        if(gFaultAnalysisFlag.cell_votage_sensor_error_count > 5)
        {
            gBmsErrorInfo.other1_error_code.b.vol_sensor_error = 1;            
        }       
    }
    
    if(tim_difftime_from_now(tsensor_error_time) >= 2000)
    {
        tsensor_error_time = tim_get_systime();
        /* 电池温度传感器故障 */
        if(gCell_Info.cell_max_temperature > TSENSOR_OT || gCell_Info.cell_min_temperature < TSENSOR_UT)
        {
            gFaultAnalysisFlag.cell_temperature_sensor_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_sensor_error_count = 0;
            
        }
        if(gFaultAnalysisFlag.cell_temperature_sensor_error_count > 10)
        {
            gBmsErrorInfo.other1_error_code.b.cell_ntc_error = 1;
        }
        
    }	
/********************************************************************************/	
/************************************其他故障*************************************/
	
	/* 加热膜故障  */
    if(tim_difftime_from_now(heat_time) >= 500)
    {
        heat_time= tim_get_systime();
        /* 充放电Mos没有打开且电流大于2A */
        if(HeatshortCheck() == true)
        {
            gFaultAnalysisFlag.heat_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.heat_error_count = 0;
        }
        
                if(gFaultAnalysisFlag.heat_error_count > 4)
        {
            //TODO
            gBmsErrorInfo.other1_error_code.b.heat_error = 1;
        }
    }
	
    /* 均衡温度无效故障 */
    
    /*RTC 故障*/
    if(tim_difftime_from_now(rtc_error_time) > 5000)
    {     
        rtc_error_time = tim_get_systime();
        if(RtcGetTime() < rtc_times)
        {
            //TODO
//            gBmsErrorInfo.other1_error_code.b.rtc_error = 1;
        }
        rtc_times = RtcGetTime();
    }    
	
    /*flash 故障*/
    if(gFaultAnalysisFlag.flash_error_flag)
    {
        gBmsErrorInfo.other1_error_code.b.spi_flash_error = 1;
    }
    
    /*e2prom 故障*/
    if(gFaultAnalysisFlag.e2prom_error_flag)
    {
        gBmsErrorInfo.other1_error_code.b.e2prom_error = 1;
    }
    
    /* 短路保护 */
    if(ChgOCCheck() == true)
    {
        gBmsErrorInfo.other1_error_code.b.chg_short_protection = 1;
    }
    
    if(DsgOCCheck() == true)
    {
        gBmsErrorInfo.other1_error_code.b.dsg_short_protection = 1;
    }
/********************************************************************************/	
}

void LatchErrorCheck(void)//此函数的作用：
{
    uint8_t rdata[ERROR11_SAVE_ADDR + 0x08] = {0};
    uint8_t *p = NULL;
//    memset(rdata, 0xAA, 0x40);
//    memset(rdata, 0, ERROR11_SAVE_ADDR + 0x08);
//    EepromWrite(rdata,ERROR1_SAVE_ADDR, 0x50);
    
    EepromRead(rdata, ERROR1_SAVE_ADDR, ERROR11_SAVE_ADDR + 0x08);
    p = rdata;
    
    /* 0x05防止指针溢出 */
    for(int i = 0; i < ERROR11_SAVE_ADDR + 0x05; i ++)
    {
        if(rdata[i] != 0)
        {
            break;
        }
        else
        {
            p++;
        }
    }
    
    if(strchr((char*)p, 0xAA) != 0)
    {
//        garrMODBUS_Reg[0].bms_shutdown = 1;
//        gBmsErrorInfo.cell1_error_code.b.cell_under_vol3 = (rdata[0x00] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cell1_error_code.b.cell_over_vol3 = (rdata[0x08] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cell1_error_code.b.pack_low_soh1 = (rdata[0x10] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cell1_error_code.b.pack_low_soh2 = (rdata[0x18] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cell1_error_code.b.cell_voltage_diff = (rdata[0x20] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cellsample_error_code.b.cell_vol_sample_miss = (rdata[0x28] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cellsample_error_code.b.vol_sensor_error = (rdata[0x30] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cellsample_error_code.b.cell_ntc_error = (rdata[0x38] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.other1_error_code.b.chg_short_protection = (rdata[0x40] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.other1_error_code.b.dsg_short_protection = (rdata[0x48] ^ 0xAA) ? 0 : 1;
    }
}

void ErrorClear(void)
{
    uint8_t rdata[ERROR11_SAVE_ADDR + 0x08] = {0};
    
    memset(rdata, 0, ERROR11_SAVE_ADDR + 0x08);
    EepromWrite(rdata,ERROR1_SAVE_ADDR, ERROR11_SAVE_ADDR + 0x08);
}

MSH_CMD_EXPORT(ErrorClear, ---);