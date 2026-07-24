

/* Includes ------------------------------------------------------------------*/
#include "fault_analysis.h"

#define LOG_TAG     "fault_analysis"
#define LIMIT_CHARGE_CURRENT   10
#define LIMIT_DISCHARGE_CURRENT   -10

#define DISCHARGE_OVERCURR1     -5000
#define DISCHARGE_OVERCURR2     -10000
#define DISCHARGE_OVERCURR3     -15000
#define CHARGE_OVERCURR1        5000
#define CHARGE_OVERCURR2        10000
#define CHARGE_OVERCURR3        15000
#define CELL_UV1                2800//gMODBUS_Reg.cell_uv_lvl1
#define CELL_UV2                2700//gMODBUS_Reg.cell_uv_lvl2
#define CELL_UV3                2600//gMODBUS_Reg.cell_uv_lvl3
#define CELL_OV1                5400//gMODBUS_Reg.cell_ov_lvl1
#define CELL_OV2                5500//gMODBUS_Reg.cell_ov_lvl2
#define CELL_OV3                5600//gMODBUS_Reg.cell_ov_lvl3
#define PACK_OV                 56000//gMODBUS_Reg.pack_ov_lvl
#define PACK_UV                 40000//gMODBUS_Reg.pack_uv_lvl
#define CMOS_OT1                4000     /* 充电Mos过温一级 */
#define CMOS_OT2                5000
#define DMOS_OT1                4000     /* 放电Mos过温一级 */
#define DMOS_OT2                5000
#define EMOS_OT1                4000     /* 加电包Mos过温一级 */
#define EMOS_OT2                5000
#define CELL_OT1                5000
#define CELL_OT2                5000
#define CELL_OT3                5000
#define CELL_UT                 5000
#define CELL_LTD                5000    /* large temperature difference */
#define TSENSOR_UT              -400
#define TSENSOR_OT              1200
#define ZERODRIFT_CURRENT       2000


Fault_analysis_flag gFaultAnalysisFlag;
BMSErrorInfo_t  gBmsErrorInfo;

void FaultAnalysisProcess(void)
{
    static uint32_t dmos_time = 0;
    static uint32_t cmos_time = 0;
    static uint32_t emos_time = 0;
    static uint32_t pmos_time = 0;
    static uint32_t current_time = 0;
    static uint32_t voltage_time = 0;
    static uint32_t temperature_time = 0;
    static uint32_t no_balance_time = 0;    
    static uint32_t vsensor_error_time = 0;  
    static uint32_t tsensor_error_time = 0;  
    static uint32_t tsensor_error_time1 = 0;  
    static uint32_t soc_time = 0;  
    static uint32_t mos_sensor_error_time = 0;  
    static uint32_t zerodrift_time = 0; 
    static uint32_t balance_sensor_error_time = 0; 
    

/************************************MOS故障*************************************/
	/* 放电或预充或充电MOS粘连故障 */
	/* 加热MOS粘连故障 */
	
	/* 预充MOS无法闭合故障 */
	if(gFaultAnalysisFlag.premos_enable_flag)
    {
        /* count_flag = 0开始计时 */
        if(!gFaultAnalysisFlag.pmos_count_flag)
        {
            gFaultAnalysisFlag.pmos_count_flag = 1;
            pmos_time = tim_get_systime();
        }
        
        if(tim_difftime_from_now(pmos_time) > 600 && gFaultAnalysisFlag.chargemos_enable_flag == 0)      /* 预充时间过长故障 */
        {
            gBmsErrorInfo.prec_error_code.b.precharge_timeout = 1;
        }
        else
        {
            gBmsErrorInfo.prec_error_code.b.precharge_timeout = 0;
        }
        
        if(gCell_Info.cell_current > LIMIT_DISCHARGE_CURRENT)       /* 预充MOS无法闭合故障 */    
        {
            
            gBmsErrorInfo.mos_error_code.b.pmos_fail_close = 1;
        }
        else if(gCell_Info.cell_current > LIMIT_CHARGE_CURRENT)     /* 预充电流反向 */
        {
            gBmsErrorInfo.prec_error_code.b.precharge_curr_revers = 1;
        }
        else 
        {
//                gBmsErrorInfo.mos_error_code.b.pmos_fail_close = 0;
        }   
    }
    else
    {
        gFaultAnalysisFlag.pmos_count_flag = 0;
    }
	/* 充电MOS无法闭合故障 */
    if(gFaultAnalysisFlag.chargemos_enable_flag)
    {
        /* 充电电流为正 */
        if(gCell_Info.cell_current < LIMIT_CHARGE_CURRENT)
        {
            gBmsErrorInfo.mos_error_code.b.cmos_fail_close = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.cmos_fail_close = 0;
        }
//        gFaultAnalysisFlag.chargemos_enable_flag = 0;      
    }
	
	/* 放电MOS无法闭合故障 */
    if(gFaultAnalysisFlag.dischargemos_enable_flag)
    {
        /* 放电电流为负 */
        if(gCell_Info.cell_current > LIMIT_DISCHARGE_CURRENT)
        {
            gBmsErrorInfo.mos_error_code.b.dmos_fail_close = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.dmos_fail_close = 0;
        }
//        gFaultAnalysisFlag.dischargemos_enable_flag = 0;       
    }
	
    /* MOS温度传感器故障 */
    if(tim_difftime_from_now(mos_sensor_error_time) > 1000)
    {     
        mos_sensor_error_time = tim_get_systime();        
        if(gCell_Info.dmos_temperature < TSENSOR_UT || gCell_Info.dmos_temperature > TSENSOR_OT)
        {
            gFaultAnalysisFlag.dmos_temperature_sensor_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.dmos_temperature_sensor_error_count = 0;
        }
        if(gCell_Info.cmos_temperature < TSENSOR_UT || gCell_Info.cmos_temperature > TSENSOR_OT)
        {
            gFaultAnalysisFlag.cmos_temperature_sensor_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cmos_temperature_sensor_error_count = 0;
        }
        if(gCell_Info.emos_temperature < TSENSOR_UT || gCell_Info.emos_temperature > TSENSOR_OT)
        {
            gFaultAnalysisFlag.emos_temperature_sensor_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.emos_temperature_sensor_error_count = 0;
        }
        
        if((gFaultAnalysisFlag.dmos_temperature_sensor_error_count > 4) || \
           (gFaultAnalysisFlag.cmos_temperature_sensor_error_count > 4) || \
           (gFaultAnalysisFlag.emos_temperature_sensor_error_count > 4))
        {
            gBmsErrorInfo.mos_error_code.b.mos_ntc_error = 1;
        }
    }
	
    if(tim_difftime_from_now(cmos_time) > 1000)
    {     
        cmos_time = tim_get_systime();
        gCell_Info.cmos_temperature = (TemperatureGet(ADC_CHANNEL_14) * 10);
        /* 充电MOS过温1级 */
        if(gCell_Info.cmos_temperature > CMOS_OT1)
        {
            gFaultAnalysisFlag.cmos_temperature_ov1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cmos_temperature_ov1_count = 0;
        }
        if(gFaultAnalysisFlag.cmos_temperature_ov1_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.cmos_high_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.cmos_high_temper1 = 0;
        }
        
        /* 充电MOS过温2级 */
        if(gCell_Info.cmos_temperature > CMOS_OT2)
        {
            gFaultAnalysisFlag.cmos_temperature_ov2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cmos_temperature_ov2_count = 0;
        }
        if(gFaultAnalysisFlag.cmos_temperature_ov2_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.cmos_high_temper2 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.cmos_high_temper2 = 0;
        }                
    }
			
    if(tim_difftime_from_now(dmos_time) > 1000)
    {     
        dmos_time = tim_get_systime();
        gCell_Info.dmos_temperature = (TemperatureGet(ADC_CHANNEL_8) * 10);
        
        /* 放电MOS过温1级 */
        if(gCell_Info.cmos_temperature > DMOS_OT1)
        {
            gFaultAnalysisFlag.dmos_temperature_ov1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.dmos_temperature_ov1_count = 0;
        }
        if(gFaultAnalysisFlag.dmos_temperature_ov1_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.dmos_high_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.dmos_high_temper1 = 0;
        }
        
        /* 放电MOS过温2级 */
        if(gCell_Info.cmos_temperature > DMOS_OT2)
        {
            gFaultAnalysisFlag.dmos_temperature_ov2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.dmos_temperature_ov2_count = 0;
        }
        if(gFaultAnalysisFlag.dmos_temperature_ov2_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.dmos_high_temper2 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.dmos_high_temper2 = 0;
        }                
    }
	
    if(tim_difftime_from_now(emos_time) > 1000)
    {     
        emos_time = tim_get_systime();
        gCell_Info.emos_temperature = (TemperatureGet(ADC_CHANNEL_15) * 10);
        /* 加电包MOS过温1级 */
        if(gCell_Info.cmos_temperature > EMOS_OT1)
        {
            gFaultAnalysisFlag.emos_temperature_ov1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.emos_temperature_ov1_count = 0;
        }
        if(gFaultAnalysisFlag.emos_temperature_ov1_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.emos_high_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.emos_high_temper1 = 0;
        }
        
        /* 加电包MOS过温2级 */
        if(gCell_Info.cmos_temperature > EMOS_OT2)
        {
            gFaultAnalysisFlag.emos_temperature_ov2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.emos_temperature_ov2_count = 0;
        }
        if(gFaultAnalysisFlag.emos_temperature_ov2_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.emos_high_temper2 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.dmos_high_temper2 = 0;
        }
    }
/********************************************************************************/	
/************************************电流故障*************************************/	
	if(tim_difftime_from_now(current_time) > 500)
    {    
        current_time = tim_get_systime();        
		/* 放电过流1级 */ 
        if(gCell_Info.cell_current < DISCHARGE_OVERCURR1)
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
        if(gCell_Info.cell_current < DISCHARGE_OVERCURR2)
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
        if(gCell_Info.cell_current < DISCHARGE_OVERCURR3)
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

        /* 充电过流1级 */ 
        if(gCell_Info.cell_current > CHARGE_OVERCURR1)
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
        if(gCell_Info.cell_current > CHARGE_OVERCURR2)
        {
            gFaultAnalysisFlag.chg_over_current2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.chg_over_current2_count = 0;
        }
        if(gFaultAnalysisFlag.chg_over_current2_count > 4)
        {
            gBmsErrorInfo.curr_error_code.b.charge_overcurr2 = 1;
        }
        else
        {
            gBmsErrorInfo.curr_error_code.b.charge_overcurr2 = 0;
        }
        
        /* 充电过流3级 */
        if(gCell_Info.cell_current > CHARGE_OVERCURR3)
        {
            gFaultAnalysisFlag.chg_over_current3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.chg_over_current3_count = 0;
        }
        if(gFaultAnalysisFlag.chg_over_current3_count > 4)
        {
            gBmsErrorInfo.curr_error_code.b.charge_overcurr3 = 1;
        }
        else
        {
            gBmsErrorInfo.curr_error_code.b.charge_overcurr3 = 0;
        }
    }  	
	
/********************************************************************************/	
/************************************电芯故障*************************************/	
	if(tim_difftime_from_now(voltage_time) > 1000)
    {    
        voltage_time = tim_get_systime();        
        /* 单体欠压1级 */ 
        if(gCell_Info.cell_max_voltage < CELL_UV1)
        {
            gFaultAnalysisFlag.cell_under_votage1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage1_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.cell_under_vol1 = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.cell_under_vol1 = 0;
        }
        
        /* 单体欠压2级 */
        if(gCell_Info.cell_max_voltage < CELL_UV2)
        {
            gFaultAnalysisFlag.cell_under_votage2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage2_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.cell_under_vol2 = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.cell_under_vol2 = 0;
        }
        
        /* 单体欠压3级 */
        if(gCell_Info.cell_max_voltage < CELL_UV3)
        {
            gFaultAnalysisFlag.cell_under_votage3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage3_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.cell_under_vol3 = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.cell_under_vol3 = 0;
        }
		/* 单体过压1级 */ 
        if(gCell_Info.cell_min_voltage > CELL_OV1)
        {
            gFaultAnalysisFlag.cell_over_votage1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage1_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.cell_over_vol1 = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.cell_over_vol1 = 0;
        }
        
        /* 单体过压2级 */
        if(gCell_Info.cell_min_voltage > CELL_OV2)
        {
            gFaultAnalysisFlag.cell_over_votage2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage2_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.cell_over_vol2 = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.cell_over_vol2 = 0;
        }
        
        /* 单体过压3级 */
        if(gCell_Info.cell_min_voltage > CELL_OV3)
        {
            gFaultAnalysisFlag.cell_over_votage3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage3_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.cell_over_vol3 = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.cell_over_vol3 = 0;
        }

		/* 电池包总电压欠压 */
        if(gCell_Info.total_cell_voltage < PACK_UV)
        {
            gFaultAnalysisFlag.pack_under_votage_count ++;
        }
        else
        {
            gFaultAnalysisFlag.pack_under_votage_count = 0;
        }
        if(gFaultAnalysisFlag.pack_under_votage_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.pack_under_vol = 1;           
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.pack_under_vol = 0;           
        }

        /* 电池包总电压过压 */
        if(gCell_Info.total_cell_voltage > PACK_OV)
        {
            gFaultAnalysisFlag.pack_over_votage_count ++;
        }
        else
        {
            gFaultAnalysisFlag.pack_over_votage_count = 0;
        }
        if(gFaultAnalysisFlag.pack_over_votage_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.pack_over_vol = 1;               
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.pack_over_vol = 0;               
        }		
    }
	
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
            gBmsErrorInfo.cell1_error_code.b.pack_high_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.pack_high_temper1 = 0;
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
            gBmsErrorInfo.cell1_error_code.b.pack_high_temper2 = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.pack_high_temper2 = 0;
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
            gBmsErrorInfo.cell1_error_code.b.pack_high_temper3 = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.pack_high_temper3 = 0;
        }

        /* 电池温度过低故障  */
        if(gCell_Info.cell_min_temperature < CELL_UT)
        {
            gFaultAnalysisFlag.cell_temperature_ut_count ++;        
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ut_count = 0;        
        }
        if(gFaultAnalysisFlag.cell_temperature_ut_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.pack_low_temper = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.pack_low_temper = 0;
        }
        
        /* 电池温差过大  */
        if((gCell_Info.cell_max_temperature - gCell_Info.cell_min_temperature) > CELL_LTD)
        {
            gFaultAnalysisFlag.cell_temperature_ltd_count ++;                   
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ltd_count = 0;       
        }
        
        if(gFaultAnalysisFlag.cell_temperature_ut_count > 4)
        {
            gBmsErrorInfo.cell1_error_code.b.pack_temper_differ = 1;
        }
        else
        {
            gBmsErrorInfo.cell1_error_code.b.pack_temper_differ = 0;
        }             
    }
    

	/* 电芯不均衡  */
//  if(soc_max - soc_min >= 30)
//	{
//		gFaultAnalysisFlag.cell_no_balance_flag = 1;
//	}
//	else
//	{
//		gFaultAnalysisFlag.cell_no_balance_flag = 0;
//		no_balance_time = tim_get_systime();
//	}
//	
//	if(tim_difftime_from_now(no_balance_time) >= 250000)
//	{
//        gBmsErrorInfo.cell1_error_code.b.pack_no_banlance = 1;
//	}
	/* SOH过低一级 */ 
	/* SOH过低二级 */     
//	if(soc < a && count < 40)   /* SOH≤80%的循环次数小于<40次 */
//	{
//        gBmsErrorInfo.cell1_error_code.b.pack_low_soh1 = 1;
//	}
//	else if(soc < b && count < 40)        /* SOH≤70%的循环次数小于<40次 */
//	{
//        gBmsErrorInfo.cell1_error_code.b.pack_low_soh2 = 1;
//	}
	
/********************************************************************************/	
/************************************预充故障*************************************/	

	/* 预充电流反向（见预充MOS无法闭合故障） */
	/* 预充时间过长（见预充MOS无法闭合故障） */
	/* 预充短路3次(需从PCS获取外侧高压值)x  */
	/* 预充短路 (需从PCS获取外侧高压值)x    */
	
/********************************************************************************/	

/************************************电芯采样故障*************************************/

	/* 电芯电压采样线掉线 */
	/* 电压传感器故障 */
    if(tim_difftime_from_now(vsensor_error_time) > 1000)
    {
        vsensor_error_time = tim_get_systime();
        if(gCell_Info.cell_max_voltage > 4800 || gCell_Info.cell_min_voltage < 250)
        {
            gFaultAnalysisFlag.cell_votage_sensor_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_votage_sensor_error_count = 0;
            
        }
        
        if(gFaultAnalysisFlag.cell_votage_sensor_error_count > 5)
        {
            gBmsErrorInfo.cellsample_error_code.b.vol_sensor_error = 1;
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
            gBmsErrorInfo.cellsample_error_code.b.cell_ntc_error = 1;
        }
        
    }
	/* 电池温度传感器故障(严重) */
    if(gBmsErrorInfo.cellsample_error_code.b.cell_ntc_error && (tim_difftime_from_now(tsensor_error_time1) >= 2000))
    {
        uint8_t i = 0;
        uint8_t count = 0;
        tsensor_error_time1 = tim_get_systime();
        for(i = 0; i < 4; i++)
        {
            if(gCell_Info.cell_temperature[i] > TSENSOR_OT || gCell_Info.cell_temperature[i] < TSENSOR_UT)
            {
                count ++;
            }
        }

        if(count > 1)
        {
            gFaultAnalysisFlag.cell_temperature_sensor_error_count1 ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_sensor_error_count1 = 0;
            
        }
        
        if(gFaultAnalysisFlag.cell_temperature_sensor_error_count1 > 10)
        {
            gBmsErrorInfo.cellsample_error_code.b.cell_ntc_fatal_error = 1;
        }
    }
	
/********************************************************************************/	
/************************************其他故障*************************************/

	/* 电流传感器零漂过大故障  */
    if(tim_difftime_from_now(zerodrift_time) >= 500)
    {
        zerodrift_time = tim_get_systime();
        if(abs(gCell_Info.cell_current) > ZERODRIFT_CURRENT)
        {
            gFaultAnalysisFlag.zerodrift_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.zerodrift_error_count = 0;
        }
        
                if(gFaultAnalysisFlag.zerodrift_error_count > 4)
        {
            gBmsErrorInfo.other1_error_code.b.curr_sensror_zerodrift_error = 1;
        }
    }
	/* 均衡回路故障            */
	/* SOC过高故障             */
    
    
	/* SOC过低故障             */
    if(tim_difftime_from_now(soc_time) >= 1000)
    {
        soc_time = tim_get_systime();
        if(gPackStatus.sox_info.soc_real <= 0)
        {
            gFaultAnalysisFlag.soc_under_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.soc_under_error_count = 0;
        }
        
        if(gFaultAnalysisFlag.cell_temperature_sensor_error_count1 > 10)
        {
            gBmsErrorInfo.other1_error_code.b.soc_under_error = 1;
        }
        else
        {
            gBmsErrorInfo.other1_error_code.b.soc_under_error = 0;
        }
    }

	/* SOC跳变故障             */
    /* 均衡温度无效故障 */
    if(tim_difftime_from_now(balance_sensor_error_time) > 1000)
    {     
        balance_sensor_error_time = tim_get_systime();        
        if(gCell_Info.afe_net_temperature < TSENSOR_UT || gCell_Info.afe_net_temperature > TSENSOR_OT)
        {
            gFaultAnalysisFlag.balance_sensor_error_count ++;
        }
        else
        {
            gFaultAnalysisFlag.balance_sensor_error_count = 0;
        }
        
        if(gFaultAnalysisFlag.balance_sensor_error_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.mos_ntc_error = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.mos_ntc_error = 0;
        }
    }
	
/********************************************************************************/	
}
    