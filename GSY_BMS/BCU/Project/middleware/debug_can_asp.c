/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#include "debug_can_asp.h"
#include "fpara_asp.h"
#include "bms_sample_res_bsp.h"
//#include "CmcManage.h"
//#include "FParaAsp.h"
#include "mcu_bsp.h"
#include "bms_current.h"
#include "bms_sample_hv_bsp.h"
#include "bms_sox.h"
#include "epara_asp.h"
#include "bms_message.h"
#include "others.h"
#include "autorun.h"
#include "bms_alarm_cfg.h"
#include "prot_can_addr_bms.h"
#include "prot_can_pack_para.h"
/////////////////////////////////////////////////////////


static bool StatsInfoReadFun(uint8_t cnt,uint8_t *buff)   //状态信息获取
{
		 uint8_t num = 0;
		
		if(cnt == 0)
		{
				buff[num++] = (uint8_t)(SOFT_VERSION>>8);
				buff[num++] = (uint8_t)SOFT_VERSION;
				buff[num++] = IR_PackParaStateInfo->succeed_flg;
				buff[num++] = IR_EnPowerOn;                                //字段数=4
				buff[num++] = IR_BmsMessage->total_voltage>>8;
				buff[num++] = IR_BmsMessage->total_voltage;
		}
		else if(cnt == 1)
		{
			buff[num++] = IR_BmsMessage->cell_voltage_max>>8;
			buff[num++] = IR_BmsMessage->cell_voltage_max;
			buff[num++] = IR_BmsMessage->cell_max_num;                 //字段数=4
			buff[num++] = IR_BmsMessage->cell_max_pack_num;
			buff[num++] = IR_BmsMessage->cell_voltage_min>>8;
			buff[num++] = IR_BmsMessage->cell_voltage_min;
		}
		else if(cnt == 2)
		{
			buff[num++] = IR_BmsMessage->cell_min_num;
			buff[num++] = IR_BmsMessage->cell_min_pack_num;
			buff[num++] = IR_BmsMessage->cell_voltage_average>>8;      //字段数=4
			buff[num++] = IR_BmsMessage->cell_voltage_average;
			buff[num++] = IR_BmsMessage->cell_voltage_difference>>8;
			buff[num++] = IR_BmsMessage->cell_voltage_difference;
		}
		else if(cnt == 3)
		{
			buff[num++] = IR_BmsMessage->cell_temp_max;
			buff[num++] = IR_BmsMessage->temp_max_num;;
			buff[num++] = IR_BmsMessage->temp_max_pack_num;
			buff[num++] = IR_BmsMessage->cell_temp_min;                //字段数=6
			buff[num++] = IR_BmsMessage->temp_min_num;
			buff[num++] = IR_BmsMessage->temp_min_pack_num;
		}
		else if(cnt == 4)
		{
			buff[num++] = IR_BmsMessage->cell_temp_average;
			buff[num++] = IR_BmsMessage->cell_temp_difference;;
			buff[num++] = IR_AutoRunInfo->autorun_main_state;          //字段数=6
			buff[num++] = IR_AutoRunInfo->autorun_main_errcode;
			buff[num++] = IR_AutoRunInfo->power_off_state;
			buff[num++] = IR_EnPowerOn;
		}
		else if(cnt == 5)
		{
			buff[num++] = IR_SysInfo->current>>8;
			buff[num++] = IR_SysInfo->current;
			buff[num++] = IR_SysInfo->soc>>8;                          //字段数=3
			buff[num++] = IR_SysInfo->soc;
			buff[num++] = IR_SysInfo->soh>>8;
			buff[num++] = IR_SysInfo->soh;
		}
		else if(cnt == 6)
		{
			buff[num++] = IR_SysInfo->sys_state;
			buff[num++] = IR_SysInfo->pcak_max_num;
			buff[num++] = IR_SysInfo->pcak_ready_num;                  //字段数=5
			buff[num++] = IR_SysInfo->BatState;
			buff[num++] = IR_SysInfo->SysRtcTime.year>>8;
			buff[num++] = IR_SysInfo->SysRtcTime.year;
		}
		else if(cnt == 7)
		{
			buff[num++] = IR_SysInfo->SysRtcTime.month>>8;
			buff[num++] = IR_SysInfo->SysRtcTime.month;
			buff[num++] = IR_SysInfo->SysRtcTime.day>>8;              //字段数=3
			buff[num++] = IR_SysInfo->SysRtcTime.day;
			buff[num++] = IR_SysInfo->SysRtcTime.week>>8;
			buff[num++] = IR_SysInfo->SysRtcTime.week;
		}
		else if(cnt == 8)
		{
			buff[num++] = IR_SysInfo->SysRtcTime.hour>>8;
			buff[num++] = IR_SysInfo->SysRtcTime.hour;
			buff[num++] = IR_SysInfo->SysRtcTime.minute>>8;           //字段数=3
			buff[num++] = IR_SysInfo->SysRtcTime.minute;
			buff[num++] = IR_SysInfo->SysRtcTime.seconds>>8;
			buff[num++] = IR_SysInfo->SysRtcTime.seconds;
		}
		else if(cnt == 9)
		{
			buff[num++] = 0;
			buff[num++] = 0;
			buff[num++] = IR_AlarmI_Code>>8;                          //字段数=4
			buff[num++] = IR_AlarmI_Code;
			buff[num++] = IR_AlarmII_Code>>8;
			buff[num++] = IR_AlarmII_Code;
		}
		else if(cnt == 10)
		{
			buff[num++] = IR_AlarmIII_Code>>8;
			buff[num++] = IR_AlarmIII_Code;
			buff[num++] = IR_AlarmMaint_Code>>8;                      //字段数=2
			buff[num++] = IR_AlarmMaint_Code;
			return true;
		}
	return false;
}
static bool CellReadFun(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    
    for(uint8_t i = 0; i < 3; i++)
    {
        uint8_t temp = cnt*3+i;
        if(temp < PACK_CELL_NUM)
        {
            buff[num++] = IR_PackMessage->cell_voltage[temp]>>8;
            buff[num++] = IR_PackMessage->cell_voltage[temp];
        }
        else 
        {
            return true;
        }
    }    
    return false;
}

static bool TempReadFun(uint8_t cnt,uint8_t *buff)
{  
    uint8_t num = 0;
    
    for(uint8_t i = 0; i < 6; i++)
    {
        uint8_t temp = cnt*6+i;
        if(temp < PACK_CELL_TEMP_NUM)
        {
            buff[num++] = IR_PackMessage->cell_temp[temp];
        }
        else 
        {
            return true;
        }
    }
    return false;
}
static bool FParaReadFun(uint8_t cnt,uint8_t *buff)
{
		uint8_t num = 0;
	
		if(cnt == 0)
		{
				buff[num++] = IR_PARA_CellVolHigh_I>>8;
				buff[num++] = IR_PARA_CellVolHigh_I;
				buff[num++] = IR_PARA_CellVolHigh_II>>8;
				buff[num++] = IR_PARA_CellVolHigh_II;           						
				buff[num++] = IR_PARA_CellVolHigh_III>>8;
				buff[num++] = IR_PARA_CellVolHigh_III;
		}
		else if(cnt == 1)
		{
				buff[num++] = IR_PARA_CellVolHigh_Fall>>8;
				buff[num++] = IR_PARA_CellVolHigh_Fall;
				buff[num++] = IR_PARA_CellVolLow_I>>8;                 			
				buff[num++] = IR_PARA_CellVolLow_I;
				buff[num++] = IR_PARA_CellVolLow_II>>8;
				buff[num++] = IR_PARA_CellVolLow_II;
		}
		else if(cnt == 2)
		{
				buff[num++] = IR_PARA_CellVolLow_III>>8;
				buff[num++] = IR_PARA_CellVolLow_III;
				buff[num++] = IR_PARA_CellVolLow_Fall>>8;                 	
				buff[num++] = IR_PARA_CellVolLow_Fall;
				buff[num++] = IR_PARA_CellVolDiff_I>>8;
				buff[num++] = IR_PARA_CellVolDiff_I;
		}
		else if(cnt == 3)
		{
				buff[num++] = IR_PARA_CellVolDiff_II>>8;
				buff[num++] = IR_PARA_CellVolDiff_II;
				buff[num++] = IR_PARA_CellVolDiff_III>>8;                 		
				buff[num++] = IR_PARA_CellVolDiff_III;
				buff[num++] = IR_PARA_CellVolDiff_Fall>>8;
				buff[num++] = IR_PARA_CellVolDiff_Fall;
		}
		else if(cnt == 4)
		{
				buff[num++] = IR_PARA_ChargeTempHigh_I;
				buff[num++] = IR_PARA_ChargeTempHigh_II;
				buff[num++] = IR_PARA_ChargeTempHigh_III;
				buff[num++] = IR_PARA_ChargeTempHigh_Fall;
				buff[num++] = IR_PARA_ChargeTempLow_I;
				buff[num++] = IR_PARA_ChargeTempLow_II;
		}		    
		else if(cnt == 5)
		{
				buff[num++] = IR_PARA_ChargeTempLow_III;
				buff[num++] = IR_PARA_ChargeTempLow_Fall;
				buff[num++] = IR_PARA_DisChargeTempHigh_I;
				buff[num++] = IR_PARA_DisChargeTempHigh_II;
				buff[num++] = IR_PARA_DisChargeTempHigh_III;
				buff[num++] = IR_PARA_DisChargeTempHigh_Fall;
		}
		else if(cnt == 6)
		{
				buff[num++] = IR_PARA_DisChargeTempLow_I;                 
				buff[num++] = IR_PARA_DisChargeTempLow_II;
				buff[num++] = IR_PARA_DisChargeTempLow_III;
				buff[num++] = IR_PARA_DisChargeTempLow_Fall;  
				buff[num++] = IR_PARA_CellTempDiff_I;
				buff[num++] = IR_PARA_CellTempDiff_II;
		}
		else if(cnt == 7)
		{

			buff[num++] = IR_PARA_CellTempDiff_III;	
				buff[num++] = IR_PARA_CellTempDiff_Fall;
				buff[num++] = IR_PARA_TotalVolHigh_I>>8;
				buff[num++] = IR_PARA_TotalVolHigh_I;
				buff[num++] = IR_PARA_TotalVolHigh_II>>8;
				buff[num++] = IR_PARA_TotalVolHigh_II;			

		}
		else if(cnt == 8)
		{

				buff[num++] = IR_PARA_TotalVolHigh_III>>8; 
				buff[num++] = IR_PARA_TotalVolHigh_III;
				buff[num++] = IR_PARA_TotalVolHigh_Fall>>8;
				buff[num++] = IR_PARA_TotalVolHigh_Fall;
				buff[num++] = IR_PARA_TotalVolLow_I>>8;
				buff[num++] = IR_PARA_TotalVolLow_I;
		}		
		else if(cnt == 9)
		{
				buff[num++] = IR_PARA_TotalVolLow_II>>8;
				buff[num++] = IR_PARA_TotalVolLow_II;
				buff[num++] = IR_PARA_TotalVolLow_III>>8;
				buff[num++] = IR_PARA_TotalVolLow_III;
				buff[num++] = IR_PARA_TotalVolLow_Fall>>8;
				buff[num++] = IR_PARA_TotalVolLow_Fall;
		}		
		else if(cnt == 10)
		{
				buff[num++] = IR_PARA_ChargeCurrHigh_I>>8; 
				buff[num++] = IR_PARA_ChargeCurrHigh_I;
				buff[num++] = IR_PARA_ChargeCurrHigh_II>>8;
				buff[num++] = IR_PARA_ChargeCurrHigh_II;
				buff[num++] = IR_PARA_ChargeCurrHigh_III>>8;
				buff[num++] = IR_PARA_ChargeCurrHigh_III;
		}		
		else if(cnt == 11)
		{
				buff[num++] = IR_PARA_ChargeCurrHigh_Fall>>8; 
				buff[num++] = IR_PARA_ChargeCurrHigh_Fall;
				buff[num++] = IR_PARA_DisChargeCurrHigh_I>>8;
				buff[num++] = IR_PARA_DisChargeCurrHigh_I; 
				buff[num++] = IR_PARA_DisChargeCurrHigh_II>>8;
				buff[num++] = IR_PARA_DisChargeCurrHigh_II;
		}
		else if(cnt == 12)
		{
				buff[num++] = IR_PARA_DisChargeCurrHigh_III>>8;	
				buff[num++] = IR_PARA_DisChargeCurrHigh_III;
				buff[num++] = IR_PARA_DisChargeCurrHigh_Fall>>8;
				buff[num++] = IR_PARA_DisChargeCurrHigh_Fall;
				buff[num++] = IR_PARA_ResLow_I>>8;
				buff[num++] = IR_PARA_ResLow_I;			

		}
		else if(cnt == 13)
		{

				buff[num++] = IR_PARA_ResLow_II>>8; 
				buff[num++] = IR_PARA_ResLow_II;
				buff[num++] = IR_PARA_ResLow_III>>8;
				buff[num++] = IR_PARA_ResLow_III;
				buff[num++] = IR_PARA_ResLow_Fall>>8;
				buff[num++] = IR_PARA_ResLow_Fall;
		}
		else if(cnt == 14)
		{

				buff[num++] = IR_PARA_PoleTempHigh_I;
				buff[num++] = IR_PARA_PoleTempHigh_II;
				buff[num++] = IR_PARA_PoleTempHigh_III;             		
				buff[num++] = IR_PARA_PoleTempHigh_Fall;
				buff[num++] = IR_PARA_Qmax>>24;
				buff[num++] = IR_PARA_Qmax>>16;
		}
		else if(cnt == 15)
		{

				buff[num++] = IR_PARA_Qmax>>8;                 								
				buff[num++] = IR_PARA_Qmax;
				buff[num++] = IR_PARA_CurrChargeRated>>8;
				buff[num++] = IR_PARA_CurrChargeRated;
				buff[num++] = IR_PARA_CurrChargeMin>>8;
				buff[num++] = IR_PARA_CurrChargeMin;

		}
		else if(cnt == 16)
		{
				buff[num++] = IR_PARA_CurrDisChargeRated>>8;                 				
				buff[num++] = IR_PARA_CurrDisChargeRated;
				buff[num++] = IR_PARA_CurrDisChargeMin>>8;                     
				buff[num++] = IR_PARA_CurrDisChargeMin;
				buff[num++] = IR_PARA_ChargeLowerVol>>8;
				buff[num++] = IR_PARA_ChargeLowerVol;

		}
		else if(cnt == 17)
		{
				buff[num++] = IR_PARA_ChargeStopVol>>8;                 			
				buff[num++] = IR_PARA_ChargeStopVol;
				buff[num++] = IR_PARA_ChargeStopSoc>>8;
				buff[num++] = IR_PARA_ChargeStopSoc;
				buff[num++] = IR_PARA_DisChargeLowerVol>>8;
				buff[num++] = IR_PARA_DisChargeLowerVol;
		}
		else if(cnt == 18)
		{
				buff[num++] = IR_PARA_DisChargeStopVol>>8;                 			
				buff[num++] = IR_PARA_DisChargeStopVol;
				buff[num++] = IR_PARA_DisChargeStopSoc>>8;
				buff[num++] = IR_PARA_DisChargeStopSoc;
				buff[num++] = IR_PARA_BalanceStartVolDiff>>8;
				buff[num++] = IR_PARA_BalanceStartVolDiff;
		}

		else if(cnt == 19)
		{
				buff[num++] = IR_PARA_PasvBalanceStartVol>>8;                 			
				buff[num++] = IR_PARA_PasvBalanceStartVol;
				buff[num++] = IR_PARA_FanStartTemp;
				buff[num++] = IR_PARA_FanStopTemp;
				buff[num++] = IR_PARA_PrechargeVolDiff>>8;
				buff[num++] = IR_PARA_PrechargeVolDiff;
		}
		else if(cnt == 20)
		{
				buff[num++] = IR_PARA_SampleTotalVolDiff>>8;                 				
				buff[num++] = IR_PARA_SampleTotalVolDiff;
				buff[num++] = IR_PARA_PowerOnComEn;
				buff[num++] = IR_PARA_CURR_ZERO;
				buff[num++] = IR_PARA_CURR_LOW;
				buff[num++] = IR_PARA_CURR_LOWER_RATIO;
		}
		else if(cnt == 21)
		{	  
  				buff[num++] = IR_PARA_CURR_LOWER_CYCLE;
			return true;
		}
		return false;
}

static bool SampleResReadFun(uint8_t cnt,uint8_t *buff)
{
		uint8_t num = 0;

		if(cnt == 0)
		{
				buff[num++] = IR_BmsSampleRes->sample_en;
				buff[num++] = IR_BmsSampleRes->sample_over;											//字段数=4
				buff[num++] = IR_BmsSampleRes->res_main>>8;
				buff[num++] = IR_BmsSampleRes->res_main;
				buff[num++] = IR_BmsSampleRes->valid_cnt>>8;  
				buff[num++] = IR_BmsSampleRes->valid_cnt;
		}
		else if(cnt == 1)
		{
				buff[num++] = IR_BmsSampleRes->error_cnt>>8;
				buff[num++] = IR_BmsSampleRes->error_cnt;
				buff[num++] = IR_BmsSampleRes->res_total_voltage>>8;						 //字段数=3
				buff[num++] = IR_BmsSampleRes->res_total_voltage;
				buff[num++] = IR_BmsSampleRes->res_p>>8;
				buff[num++] = IR_BmsSampleRes->res_p;
		}
		else if(cnt == 2)
		{
				buff[num++] = IR_BmsSampleRes->res_n>>8;
				buff[num++] = IR_BmsSampleRes->res_n;
				buff[num++] = IR_BmsSampleRes->volt_kp1>>8;						 //字段数=3
				buff[num++] = IR_BmsSampleRes->volt_kp1;
				buff[num++] = IR_BmsSampleRes->volt_kn1>>8;
				buff[num++] = IR_BmsSampleRes->volt_kn1;
		}
		else if(cnt == 3)
		{
				buff[num++] = IR_BmsSampleRes->volt_kp2>>8;
				buff[num++] = IR_BmsSampleRes->volt_kp2;
				buff[num++] = IR_BmsSampleRes->volt_kn2>>8;						 //字段数=3
				buff[num++] = IR_BmsSampleRes->volt_kn2;
				buff[num++] = IR_BmsSampleHV->valid_cnt>>8;
				buff[num++] = IR_BmsSampleHV->valid_cnt;
		}

		/////////////////////HV////////////
		else if(cnt == 4)
		{
					
				buff[num++] = IR_BmsSampleHV->hv_voltage[0]>>8;						 	
				buff[num++] = IR_BmsSampleHV->hv_voltage[0];
				buff[num++] = IR_BmsSampleHV->hv_voltage[1]>>8;
				buff[num++] = IR_BmsSampleHV->hv_voltage[1];
				buff[num++] = IR_BmsSampleHV->hv_voltage[2]>>8;
				buff[num++] = IR_BmsSampleHV->hv_voltage[2];
		}
		else if(cnt == 5)
		{				
				buff[num++] = IR_BmsSampleHV->error_cnt>>8;						 			
				buff[num++] = IR_BmsSampleHV->error_cnt;
				buff[num++] = IR_BmsCurrent->current>>24;
				buff[num++] = IR_BmsCurrent->current>>16;
				buff[num++] = IR_BmsCurrent->current>>8;						 						
				buff[num++] = IR_BmsCurrent->current;
		}
		/////////////////////CURR////////////
		else if(cnt == 6)
		{
					
				buff[num++] = IR_BmsCurrent->charge_integral>>24;
				buff[num++] = IR_BmsCurrent->charge_integral>>16;
				buff[num++] = IR_BmsCurrent->charge_integral>>8;
				buff[num++] = IR_BmsCurrent->charge_integral;
				buff[num++] = IR_BmsCurrent->discharge_integral>>24;						
				buff[num++] = IR_BmsCurrent->discharge_integral>>16;
		}
		else if(cnt == 7)
		{
					
				buff[num++] = IR_BmsCurrent->discharge_integral>>8;
				buff[num++] = IR_BmsCurrent->discharge_integral;
				return true;
		}	
		return false;
}		

static bool Addr_Para_Info(uint8_t cnt,uint8_t *buff)
{
		uint8_t num = 0;
		if(cnt == 0)
		{
				buff[num++] = IR_BmsAddrInfo->rated_num;
				buff[num++] = IR_BmsAddrInfo->check_num;
				buff[num++] = IR_BmsAddrInfo->succeed_flg;
				buff[num++] = IR_BmsAddrInfo->state;
				buff[num++] = IR_BmsAddrInfo->err_flg;
				buff[num++] = IR_BmsAddrInfo->err_location;
		}

		else if(cnt == 1)
		{
				buff[num++] = IR_PackParaStateInfo->succeed_flg;
				buff[num++] = IR_PackParaStateInfo->succeed_num;
				buff[num++] = IR_PackParaStateInfo->fail_num;
				return true;
		}			
	    return false;
}
static bool DO_ReadFun(uint8_t cnt,uint8_t *buff)
{
		uint8_t num = 0;

		for(uint8_t i = 0; i < 6; i++)
		{
			uint8_t temp = cnt*6+i;
			if(temp+1 < DO_NUM)
			{
				buff[num++] = gDO_State[temp+1];
			}
			else 
			{
				return true;
			}
		}
    return false;
}

static bool DI_ReadFun(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    
    for(uint8_t i = 0; i < 6; i++)
    {
        uint8_t temp = cnt*6+i;
        if(temp < DI_NUM)
        {
            buff[num++] = gDI_State[temp];
        }
        else 
        {
            return true;
        }
    }
    return false;
}
static bool ADC_ReadFun(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
        
    if(cnt == 0)
    {
//        buff[num++] = pIR_Adc2Handle->pRValtage[0]>>8;
//        buff[num++] = pIR_Adc2Handle->pRValtage[0];
//        buff[num++] = pIR_Adc2Handle->pRValtage[1]>>8;
//        buff[num++] = pIR_Adc2Handle->pRValtage[1];
//        buff[num++] = pIR_Adc2Handle->pRValtage[2]>>8;
//        buff[num++] = pIR_Adc2Handle->pRValtage[2];
    }
    else if(cnt == 1)
    {
//        buff[num++] = pIR_Adc2Handle->pRValtage[3]>>8;
//        buff[num++] = pIR_Adc2Handle->pRValtage[3];
//        buff[num++] = pIR_Adc2Handle->pRValtage[4]>>8;
//        buff[num++] = pIR_Adc2Handle->pRValtage[4];
//        buff[num++] = pIR_Adc2Handle->pRValtage[5]>>8;
//        buff[num++] = pIR_Adc2Handle->pRValtage[5];
    }
    else if(cnt == 2)
    {
//        buff[num++] = pIR_Adc2Handle->pRValtage[6]>>8;
//        buff[num++] = pIR_Adc2Handle->pRValtage[6];
//        buff[num++] = IR_SysInfo->ntc_temp[0];
//        buff[num++] = IR_SysInfo->ntc_temp[1];
//        buff[num++] = IR_SysInfo->ntc_temp[2];
//        buff[num++] = IR_SysInfo->ntc_temp[3];
        return true;
    }
    return false;
}
static bool Soc_ReadFun(uint8_t cnt,uint8_t *buff)
{
		uint8_t num = 0;
		if(cnt == 0)
		{
			buff[num++] = IR_BmsSoc->soc>>8;
			buff[num++] = IR_BmsSoc->soc;
			buff[num++] = IR_BmsSoc->soh>>8;
			buff[num++] = IR_BmsSoc->soh;
			buff[num++] = IR_BmsSoc->q_max>>24;
			buff[num++] = IR_BmsSoc->q_max>>16;
		}
		else if(cnt == 1)
		{
			buff[num++] = IR_BmsSoc->q_max>>8;
			buff[num++] = IR_BmsSoc->q_max;
			buff[num++] = IR_BmsSoc->q_real>>24;
			buff[num++] = IR_BmsSoc->q_real>>16;
			buff[num++] = IR_BmsSoc->q_real>>8;
			buff[num++] = IR_BmsSoc->q_real;
		}
		else if(cnt == 2)
		{
			buff[num++] = IR_BmsSoc->q_residual>>24;
			buff[num++] = IR_BmsSoc->q_residual>>26;
			buff[num++] = IR_BmsSoc->q_residual>>8;
			buff[num++] = IR_BmsSoc->q_residual>>0;
			buff[num++] = IR_BmsSoc->accum_q_charge>>24;
			buff[num++] = IR_BmsSoc->accum_q_charge>>16;
		}
		else if(cnt == 3)
		{
			buff[num++] = IR_BmsSoc->accum_q_charge>>8;
			buff[num++] = IR_BmsSoc->accum_q_charge;
			buff[num++] = IR_BmsSoc->accum_q_discharge>>24;
			buff[num++] = IR_BmsSoc->accum_q_discharge>>16;
			buff[num++] = IR_BmsSoc->accum_q_discharge>>8;
			buff[num++] = IR_BmsSoc->accum_q_discharge;
		}
		else if(cnt == 4)
		{
			buff[num++] = IR_BmsSoc->cycles>>8;
			buff[num++] = IR_BmsSoc->cycles;
			buff[num++] = IR_BmsSoc->charge_capacity_kwh>>8;
			buff[num++] = IR_BmsSoc->charge_capacity_kwh;
			buff[num++] = IR_BmsSoc->discharge_capacity_kwh>>8;
			buff[num++] = IR_BmsSoc->discharge_capacity_kwh;
		}
		else if(cnt == 5)
		{
			buff[num++] = IR_BmsSoc->once_accum_kwh_charge>>8;
			buff[num++] = IR_BmsSoc->once_accum_kwh_charge;
			buff[num++] = IR_BmsSoc->once_accum_kwh_discharge>>8;
			buff[num++] = IR_BmsSoc->once_accum_kwh_discharge;
			buff[num++] = IR_BmsSoc->day_accum_kwh_charge>>8;
			buff[num++] = IR_BmsSoc->day_accum_kwh_charge;
		}
		else if(cnt == 6)
		{
			buff[num++] = IR_BmsSoc->day_accum_kwh_discharge>>8;
			buff[num++] = IR_BmsSoc->day_accum_kwh_discharge;
			buff[num++] = IR_BmsSoc->total_accum_kwh_charge>>24;
			buff[num++] = IR_BmsSoc->total_accum_kwh_charge>>16;
			buff[num++] = IR_BmsSoc->total_accum_kwh_charge>>8;
			buff[num++] = IR_BmsSoc->total_accum_kwh_charge;
		}
		else if(cnt == 7)
		{
			buff[num++] = IR_BmsSoc->total_accum_kwh_discharge>>24;
			buff[num++] = IR_BmsSoc->total_accum_kwh_discharge>>16;
			buff[num++] = IR_BmsSoc->total_accum_kwh_discharge>>8;
			buff[num++] = IR_BmsSoc->total_accum_kwh_discharge;
			return true;
		}
	return false;
}
static bool Limit_Current(uint8_t cnt,uint8_t *buff)
{
		uint8_t num = 0;
		if(cnt == 1)
		{
			buff[num++] = IR_LimitCurr->limit_current_state;
			buff[num++] = IR_LimitCurr->charge_limit_curr>>8;
			buff[num++] = IR_LimitCurr->charge_limit_curr;
			buff[num++] = IR_LimitCurr->discharge_limit_curr>>8;
			buff[num++] = IR_LimitCurr->discharge_limit_curr;
			buff[num++] = IR_LimitCurr->lower_charge_flag;
		}
		else if(cnt == 2)
		{
			buff[num++] = IR_LimitCurr->lower_discharge_flag;
			buff[num++] = IR_LimitCurr->finish_charge_flag;
			buff[num++] = IR_LimitCurr->finish_discharge_flag;
			buff[num++] = IR_LimitCurr->charge_lower_soc>>8;
			buff[num++] = IR_LimitCurr->charge_lower_soc;
			buff[num++] = IR_LimitCurr->discharge_lower_soc>>8;
		}
		else if(cnt == 3)
		{
			buff[num++] = IR_LimitCurr->discharge_lower_soc;
			return true;
		}
		
	return false;
}
////////////////////////////////////////////////////////////
static bool DO_CtrlFun(uint8_t num,uint8_t *buff)
{
    uint8_t offset = 0;
    
    for(uint8_t j = 0; j < num; j++)
    {
            DO_FORCE_SET(buff[offset]+1,buff[offset+2]);
            offset += (buff[offset+1] +2);
    }
    return true;
}

static bool FParaSetFun(uint8_t num,uint8_t *buff)
{
    uint8_t offset = 0;
    
    for(uint8_t j = 0; j < num; j++)
    {
		
            switch(buff[offset])
            {
                case 0:
                {		
                    IR_Write_BmsPara->alarm_para.cell_vol_high_I = (buff[offset+2]<<8) + buff[offset+3];
                    break;
                } 
                case 1:
                {		
                    IR_Write_BmsPara->alarm_para.cell_vol_high_II = (buff[offset+2]<<8) + buff[offset+3];
                    break;
                }
                case 2:
                {		
                    IR_Write_BmsPara->alarm_para.cell_vol_high_III = (buff[offset+2]<<8) + buff[offset+3];
                    break;
                }
                case 3:
                {		
                    IR_Write_BmsPara->alarm_para.cell_vol_high_fall = (buff[offset+2]<<8) + buff[offset+3];
                    break;
                }
                case 4:
                {		
                    IR_Write_BmsPara->alarm_para.cell_vol_low_I = (buff[offset+2]<<8) + buff[offset+3];
                    break;
                }
                case 5:
                {		
                        IR_Write_BmsPara->alarm_para.cell_vol_low_II = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 6:
                {		
                        IR_Write_BmsPara->alarm_para.cell_vol_low_III = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 7:
                {		
                        IR_Write_BmsPara->alarm_para.cell_vol_low_fall = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 8:
                {		
                        IR_Write_BmsPara->alarm_para.cell_vol_diff_I = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 9:
                {		
                        IR_Write_BmsPara->alarm_para.cell_vol_diff_II = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 10:
                {		
                        IR_Write_BmsPara->alarm_para.cell_vol_diff_III = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 11:
                {		
                        IR_Write_BmsPara->alarm_para.cell_vol_diff_fall = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 12:
                {		
                        IR_Write_BmsPara->alarm_para.charge_temp_high_I = buff[offset+2];
                        break;
                }
                case 13:
                {		
                        IR_Write_BmsPara->alarm_para.charge_temp_high_II = buff[offset+2];
                        break;
                }
                case 14:
                {		
                        IR_Write_BmsPara->alarm_para.charge_temp_high_III = buff[offset+2];
                        break;
                }
                case 15:
                {		
                        IR_Write_BmsPara->alarm_para.charge_temp_high_fall = buff[offset+2];
                        break;
                }
                case 16:
                {		
                        IR_Write_BmsPara->alarm_para.charge_temp_low_I = buff[offset+2];
                        break;
                }
                case 17:
                {		
                        IR_Write_BmsPara->alarm_para.charge_temp_low_II = buff[offset+2];
                        break;
                }
                case 18:
                {		
                        IR_Write_BmsPara->alarm_para.charge_temp_low_III = buff[offset+2];
                        break;
                }
                case 19:
                {		
                        IR_Write_BmsPara->alarm_para.charge_temp_low_fall = buff[offset+2];
                        break;
                }
                case 20:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_temp_high_I = buff[offset+2];
                        break;
                }
                case 21:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_temp_high_II = buff[offset+2];
                        break;
                }
                case 22:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_temp_high_III = buff[offset+2];
                        break;
                }
                case 23:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_temp_high_fall = buff[offset+2];
                        break;
                }
                case 24:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_temp_low_I = buff[offset+2];
                        break;
                }
                case 25:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_temp_low_II = buff[offset+2];
                        break;
                }
                case 26:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_temp_low_III = buff[offset+2];
                        break;
                }
                case 27:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_temp_low_fall = buff[offset+2];
                        break;
                }
                case 28:
                {		
                        IR_Write_BmsPara->alarm_para.cell_temp_diff_I = buff[offset+2];
                        break;
                }
                case 29:
                {		
                        IR_Write_BmsPara->alarm_para.cell_temp_diff_II = buff[offset+2];
                        break;
                }
                case 30:
                {		
                        IR_Write_BmsPara->alarm_para.cell_temp_diff_III = buff[offset+2];
                        break;
                }
                case 31:
                {		
                        IR_Write_BmsPara->alarm_para.cell_temp_diff_fall = buff[offset+2];
                        break;
                }
                case 32:
                {		
                        IR_Write_BmsPara->alarm_para.total_vol_high_I = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 33:
                {		
                        IR_Write_BmsPara->alarm_para.total_vol_high_II = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 34:
                {		
                        IR_Write_BmsPara->alarm_para.total_vol_high_III = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 35:
                {		
                        IR_Write_BmsPara->alarm_para.total_vol_high_fall = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 36:
                {		
                        IR_Write_BmsPara->alarm_para.total_vol_low_I = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 37:
                {		
                        IR_Write_BmsPara->alarm_para.total_vol_low_II = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 38:
                {		
                        IR_Write_BmsPara->alarm_para.total_vol_low_III = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 39:
                {		
                        IR_Write_BmsPara->alarm_para.total_vol_low_fall = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 40:
                {		
                        IR_Write_BmsPara->alarm_para.charge_curr_high_I = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 41:
                {		
                        IR_Write_BmsPara->alarm_para.charge_curr_high_II = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 42:
                {		
                        IR_Write_BmsPara->alarm_para.charge_curr_high_III = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 43:
                {		
                        IR_Write_BmsPara->alarm_para.charge_curr_high_fall = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 44:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_curr_high_I = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 45:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_curr_high_II = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 46:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_curr_high_III = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 47:
                {		
                        IR_Write_BmsPara->alarm_para.discharge_curr_high_fall = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 48:
                {		
                        IR_Write_BmsPara->alarm_para.res_low_I = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 49:
                {		
                        IR_Write_BmsPara->alarm_para.res_low_II = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 50:
                {		
                        IR_Write_BmsPara->alarm_para.res_low_III = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 51:
                {		
                        IR_Write_BmsPara->alarm_para.res_low_fall = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 52:
                {		
                        IR_Write_BmsPara->alarm_para.pole_temp_high_I = buff[offset+2];
                        break;
                }
                case 53:
                {		
                        IR_Write_BmsPara->alarm_para.pole_temp_high_II = buff[offset+2];
                        break;
                }
                case 54:
                {		
                        IR_Write_BmsPara->alarm_para.pole_temp_high_III = buff[offset+2];
                        break;
                }
                case 55:
                {		
                        IR_Write_BmsPara->alarm_para.pole_temp_high_fall = buff[offset+2];
                        break;
                }
                
                
                ///////////////SYS_PARA//////////////////
                case 56:
                {		
                        IR_Write_BmsPara->sys_para.q_max = (buff[offset+2]<<24) + (buff[offset+3]<<16) + (buff[offset+4]<<8) + buff[offset+5];
                        BmsQmaxSet(IR_PARA_Qmax);
                        break;
                }
                case 57:
                {		
                        IR_Write_BmsPara->sys_para.curr_charge_rated = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 58:
                {		
                        IR_Write_BmsPara->sys_para.curr_charge_min = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 59:
                {		
                        IR_Write_BmsPara->sys_para.curr_discharge_rated = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 60:
                {		
                         IR_Write_BmsPara->sys_para.curr_discharge_min = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 61:
                {		
                        IR_Write_BmsPara->sys_para.charge_lower_vol = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 62:
                {		
                        IR_Write_BmsPara->sys_para.charge_stop_vol = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 63:
                {		
                        IR_Write_BmsPara->sys_para.charge_stop_soc = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 64:
                {		
                        IR_Write_BmsPara->sys_para.discharge_lower_vol = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 65:
                {		
                        IR_Write_BmsPara->sys_para.discharge_stop_vol = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 66:
                {		
                        IR_Write_BmsPara->sys_para.discharge_stop_soc = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 67:
                {		
                        IR_Write_BmsPara->sys_para.balance_start_diff_vol = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 68:
                {		
                        IR_Write_BmsPara->sys_para.pasv_balance_start_vol = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 69:
                {		
                        IR_Write_BmsPara->sys_para.fan_start_temp = buff[offset+2];
                        break;
                }
                case 70:
                {		
                        IR_Write_BmsPara->sys_para.fan_stop_temp = buff[offset+2];
                        break;
                }
                case 71:
                {		
                        IR_Write_BmsPara->sys_para.precharge_diff_vol = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 72:
                {		
                        IR_Write_BmsPara->sys_para.sample_total_vol_diff = (buff[offset+2]<<8) + buff[offset+3];
                        break;
                }
                case 73:
                {		
                        IR_Write_BmsPara->sys_para.powre_on_com_en = buff[offset+2];
                        break;
                }
                
                
                ///////////////////bms_run_para//////////
                case 74:
                {		
                        IR_Write_BmsPara->bms_run_para.curr_zero = buff[offset+2];
                        break;
                }
                case 75:
                {		
                        IR_Write_BmsPara->bms_run_para.curr_low = buff[offset+2];
                        break;
                }
                case 76:
                {		
                        IR_Write_BmsPara->bms_run_para.curr_lower_ratio = buff[offset+2];
                        break;
                }
                case 77:
                {		
                        IR_Write_BmsPara->bms_run_para.curr_lower_cycle = buff[offset+2];
                        break;
                }
                default :   break;
            }

    }
    FParaSaveSet(FPARA_WRITE_DEBUG);
    return true;
}


static bool EParaReadFun(uint8_t cnt,uint8_t *buff)
{
	uint8_t num = 0;
	static uint8_t temp = 0;
	
	if(cnt%2 == 0)
	{
		buff[num++] = IR_EParaData[temp]>>24;
		buff[num++] = IR_EParaData[temp]>>16;
		buff[num++] = IR_EParaData[temp]>>8;
		buff[num++] = IR_EParaData[temp];
		if(++temp >= EPARA_ASP_NUM)
		{
			temp = 0;
			return true;
		}
		buff[num++] = IR_EParaData[temp]>>24;
		buff[num++] = IR_EParaData[temp]>>16;
	}
	else 
	{
		buff[num++] = IR_EParaData[temp]>>8;
		buff[num++] = IR_EParaData[temp];
		if(++temp >= EPARA_ASP_NUM)
		{
			temp = 0;
			return true;
		}
		buff[num++] = IR_EParaData[temp]>>24;
		buff[num++] = IR_EParaData[temp]>>16;
		buff[num++] = IR_EParaData[temp]>>8;
		buff[num++] = IR_EParaData[temp];
		if(++temp >= EPARA_ASP_NUM)
		{
			temp = 0;
			return true;
		}
	}		
	return false;
}
static bool EParaSetFun(uint8_t num,uint8_t *buff)
{
    uint8_t offset = 0;
    
    for(uint8_t j = 0; j < num; j++)
    {
		EParaWrite(buff[offset], (buff[offset+2]<<24) + (buff[offset+3]<<16) + (buff[offset+4]<<8) + buff[offset+5]);
		offset += (buff[offset+1] +2);
    }
    return true;
}

char SN[20] = "SN123456789123456789";
uint8_t DevType = 4;
char DevName[20] = "CMU64T68";
static bool BootInfo_ReadFun(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    
    for(uint8_t i = 0; i < 6; i++)
    {
        uint8_t temp = cnt*6+i;
        if(temp < 20)
        {
            buff[num++] = IR_BootInfo->sn[temp];
        }
        else if(temp < 21)
        {
            buff[num++] = IR_BootInfo->DeviceType;
        }
        else if(temp < 29)
        {
            buff[num++] = IR_BootInfo->dev_name[temp - 21];
        }
        else
        {
            return true;
        }
    }
    
    return false;
}
static bool BootInfoSetFun(uint8_t num,uint8_t *buff)
{
    uint8_t offset = 0;
    
    for(uint8_t j = 0; j < num; j++)
    {
		switch(buff[offset])
		{
			case 0:
			{
                memcpy(IR_Write_BootInfo->sn,&buff[offset+2],buff[offset+1]);
			}
			case 1:
			{
			
			}
			case 2:
			{
			
			}
			default : break;
		}      
//            else if(size_z = size,size += sizeof(IR_BmsPara->EnParaInfo.OtherTempValid),buff[offset] < size)
//            {
//                IR_Write_BmsPara->EnParaInfo.OtherTempValid[buff[offset]-size_z]= buff[offset+2];
//            }
//            else if(size += 1,buff[offset] < size)
//            {
//                IR_Write_BmsPara->EnParaInfo.TempErrAllowNum= buff[offset+2];
//            }
            
            
            offset += (buff[offset+1] +2);

    }
    BootInfoSaveSet(FPARA_WRITE_DEBUG);
    return true;
}
//////////////////////////////PACK_INFO//////////////////////////////////
#if (PACK_NUM>=1)
static bool PackInfoReadFun_01(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    
    if(cnt == 0)
    {
        buff[num++] = IR_PackMessage[0].com_state;
        buff[num++] = IR_PackMessage[0].total_voltage>>8;
        buff[num++] = IR_PackMessage[0].total_voltage;
        buff[num++] = IR_PackMessage[0].cell_voltage_max>>8;
        buff[num++] = IR_PackMessage[0].cell_voltage_max;
        buff[num++] = IR_PackMessage[0].cell_max_num;
    }
    else if(cnt == 1)
    {
        buff[num++] = IR_PackMessage[0].cell_voltage_min>>8;
        buff[num++] = IR_PackMessage[0].cell_voltage_min;
        buff[num++] = IR_PackMessage[0].cell_min_num;
        buff[num++] = IR_PackMessage[0].cell_voltage_average>>8;
        buff[num++] = IR_PackMessage[0].cell_voltage_average;
        buff[num++] = IR_PackMessage[0].cell_temp_max;
    }
    else if(cnt == 2)
    {
        buff[num++] = IR_PackMessage[0].temp_max_num;
        buff[num++] = IR_PackMessage[0].cell_temp_min;
        buff[num++] = IR_PackMessage[0].temp_min_num;
        buff[num++] = IR_PackMessage[0].cell_temp_average;
        buff[num++] = IR_PackMessage[0].pole_temp_max_num;
        buff[num++] = IR_PackMessage[0].pole_temp_min;
    }
    else if(cnt == 3)
    {
        buff[num++] = IR_PackMessage[0].pole_temp_min_num;
        buff[num++] = IR_PackMessage[0].balance_temp_max;
        
        return true;
    }
    
    return false;
}
static bool PackCellReadFun_01(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    
    for(uint8_t i = 0; i < 3; i++)
    {
        if(cnt*3+i < PACK_CELL_NUM)
        {
            buff[num++] = IR_PackMessage[0].cell_voltage[cnt*3+i]>>8;
            buff[num++] = IR_PackMessage[0].cell_voltage[cnt*3+i];
        }
        else
        {
            return true;
        }
    }
    
    return false;
}
static bool PackTempReadFun_01(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    
    for(uint8_t i = 0; i < 6; i++)
    {
        if(cnt*6+i < PACK_CELL_TEMP_NUM)
        {
            buff[num++] = IR_PackMessage[0].cell_voltage[cnt*6+i]>>8;
            buff[num++] = IR_PackMessage[0].cell_voltage[cnt*6+i];
        }
        else
        {
            return true;
        }
    }
    
    return false;
}
#endif
#if (PACK_NUM>=2)
static bool PackInfoReadFun_02(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    
    if(cnt == 0)
    {
        buff[num++] = IR_PackMessage[1].com_state;
        buff[num++] = IR_PackMessage[1].total_voltage>>8;
        buff[num++] = IR_PackMessage[1].total_voltage;
        buff[num++] = IR_PackMessage[1].cell_voltage_max>>8;
        buff[num++] = IR_PackMessage[1].cell_voltage_max;
        buff[num++] = IR_PackMessage[1].cell_max_num;
    }
    else if(cnt == 1)
    {
        buff[num++] = IR_PackMessage[1].cell_voltage_min>>8;
        buff[num++] = IR_PackMessage[1].cell_voltage_min;
        buff[num++] = IR_PackMessage[1].cell_min_num;
        buff[num++] = IR_PackMessage[1].cell_voltage_average>>8;
        buff[num++] = IR_PackMessage[1].cell_voltage_average;
        buff[num++] = IR_PackMessage[1].cell_temp_max;
    }
    else if(cnt == 2)
    {
        buff[num++] = IR_PackMessage[1].temp_max_num;
        buff[num++] = IR_PackMessage[1].cell_temp_min;
        buff[num++] = IR_PackMessage[1].temp_min_num;
        buff[num++] = IR_PackMessage[1].cell_temp_average;
        buff[num++] = IR_PackMessage[1].pole_temp_max_num;
        buff[num++] = IR_PackMessage[1].pole_temp_min;
    }
    else if(cnt == 3)
    {
        buff[num++] = IR_PackMessage[1].pole_temp_min_num;
        buff[num++] = IR_PackMessage[1].balance_temp_max;
        
        return true;
    }
    
    return false;
}
#endif
#if (PACK_NUM>=3)
static bool PackInfoReadFun_03(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    
    if(cnt == 0)
    {
        buff[num++] = IR_PackMessage[2].com_state;
        buff[num++] = IR_PackMessage[2].total_voltage>>8;
        buff[num++] = IR_PackMessage[2].total_voltage;
        buff[num++] = IR_PackMessage[2].cell_voltage_max>>8;
        buff[num++] = IR_PackMessage[2].cell_voltage_max;
        buff[num++] = IR_PackMessage[2].cell_max_num;
    }
    else if(cnt == 1)
    {
        buff[num++] = IR_PackMessage[2].cell_voltage_min>>8;
        buff[num++] = IR_PackMessage[2].cell_voltage_min;
        buff[num++] = IR_PackMessage[2].cell_min_num;
        buff[num++] = IR_PackMessage[2].cell_voltage_average>>8;
        buff[num++] = IR_PackMessage[2].cell_voltage_average;
        buff[num++] = IR_PackMessage[2].cell_temp_max;
    }
    else if(cnt == 2)
    {
        buff[num++] = IR_PackMessage[2].temp_max_num;
        buff[num++] = IR_PackMessage[2].cell_temp_min;
        buff[num++] = IR_PackMessage[2].temp_min_num;
        buff[num++] = IR_PackMessage[2].cell_temp_average;
        buff[num++] = IR_PackMessage[2].pole_temp_max_num;
        buff[num++] = IR_PackMessage[2].pole_temp_min;
    }
    else if(cnt == 3)
    {
        buff[num++] = IR_PackMessage[2].pole_temp_min_num;
        buff[num++] = IR_PackMessage[2].balance_temp_max;
        
        return true;
    }
    return false;
}
#endif
#if (PACK_NUM>=4)
static bool PackInfoReadFun_04(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    
    if(cnt == 0)
    {
        buff[num++] = IR_PackMessage[3].com_state;
        buff[num++] = IR_PackMessage[3].total_voltage>>8;
        buff[num++] = IR_PackMessage[3].total_voltage;
        buff[num++] = IR_PackMessage[3].cell_voltage_max>>8;
        buff[num++] = IR_PackMessage[3].cell_voltage_max;
        buff[num++] = IR_PackMessage[3].cell_max_num;
    }
    else if(cnt == 1)
    {
        buff[num++] = IR_PackMessage[3].cell_voltage_min>>8;
        buff[num++] = IR_PackMessage[3].cell_voltage_min;
        buff[num++] = IR_PackMessage[3].cell_min_num;
        buff[num++] = IR_PackMessage[3].cell_voltage_average>>8;
        buff[num++] = IR_PackMessage[3].cell_voltage_average;
        buff[num++] = IR_PackMessage[3].cell_temp_max;
    }
    else if(cnt == 2)
    {
        buff[num++] = IR_PackMessage[3].temp_max_num;
        buff[num++] = IR_PackMessage[3].cell_temp_min;
        buff[num++] = IR_PackMessage[3].temp_min_num;
        buff[num++] = IR_PackMessage[3].cell_temp_average;
        buff[num++] = IR_PackMessage[3].pole_temp_max_num;
        buff[num++] = IR_PackMessage[3].pole_temp_min;
    }
    else if(cnt == 3)
    {
        buff[num++] = IR_PackMessage[3].pole_temp_min_num;
        buff[num++] = IR_PackMessage[3].balance_temp_max;
        
        return true;
    }
    return false;
}
#endif
////////////////////////////////////////////////////////////////
char dd[20] = {"中文"};
ProductionTestType ProductionTest = {0};
static bool ProductionTEST_ReadFun(uint8_t cnt,uint8_t *buff)
{
    uint8_t num = 0;
    bool di_state = true;
    
    if(cnt == 0)
    {
        buff[num++] = ProductionTest.do_ctrl;
        for(uint8_t i = 2; i < 4; i++)
        {
            if(gDI_State[i] != 1)
            {
                di_state = false;
                break;
            }
        }
        for(uint8_t i = 4; i < DI_NUM; i++)
        {
            if(gDI_State[i])
            {
                di_state = false;
                break;
            }
        }
        buff[num++] = di_state;
        buff[num++] = 0;
        buff[num++] = 0;
        buff[num++] = 0;
        buff[num++] = 0;
        
        return true;
    }
    return false;
}
static bool ProductionTEST_CtrlFun(uint8_t cnt,uint8_t *buff)
{
    uint8_t offset = 0;
    
    for(uint8_t j = 0; j < cnt; j++)
    {
        switch(buff[offset])
        {
            case 0:
            {		
                ProductionTest.do_ctrl = buff[offset+2];
                if(ProductionTest.do_ctrl)
                {
                    for(uint8_t i = 1; i < DO_NUM; i++)
                    {
                        DO_FORCE_SET(i,true);
                    }
                }
                else
                {
                    for(uint8_t i = 1; i < DO_NUM; i++)
                    {
                        DO_FORCE_SET(i,false);
                    }
                }
                break;
            } 
            
            default :
                break;
        }
        offset += (buff[offset+1] +2);
    }
    ProductionTest.test_mode = true;
    return true;
}
////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_ASP_NUM  (PACK_NUM*3+31)
static DebugMessageAspType DebugMessage_Asp[DEBUG_ASP_NUM]=
{
//    {101, 64, StatsInfoReadFun,NULL,0},
//    {102, 44, SampleResReadFun,NULL,0},
//	{103, 46, Soc_ReadFun,NULL,0},
//	{104, 13, Limit_Current,NULL,0},
//	{105, 9, Addr_Para_Info,NULL,0},
//    
//    {111, 64, StatsInfoReadFun,NULL,0},
//    {112, 44, SampleResReadFun,NULL,0},
//	{113, 46, Soc_ReadFun,NULL,0},
//	{114, 13, Limit_Current,NULL,0},
//	{115, 9, Addr_Para_Info,NULL,0},
//    
//    {121, 64, StatsInfoReadFun,NULL,0},
//    {122, 44, SampleResReadFun,NULL,0},
//	{123, 46, Soc_ReadFun,NULL,0},
//	{124, 13, Limit_Current,NULL,0},
//	{125, 9, Addr_Para_Info,NULL,0},
//    
//    {131, 64, StatsInfoReadFun,NULL,0},
//    {132, 44, SampleResReadFun,NULL,0},
//	{133, 46, Soc_ReadFun,NULL,0},
//	{134, 13, Limit_Current,NULL,0},
//	{135, 9, Addr_Para_Info,NULL,0},
//    
//    {141, 64, StatsInfoReadFun,NULL,0},
//    {142, 44, SampleResReadFun,NULL,0},
//	{143, 46, Soc_ReadFun,NULL,0},
//	{144, 13, Limit_Current,NULL,0},
//	{145, 9, Addr_Para_Info,NULL,0},
//    
//    {151, 64, StatsInfoReadFun,NULL,0},
//    {152, 44, SampleResReadFun,NULL,0},
//	{153, 46, Soc_ReadFun,NULL,0},
//	{154, 13, Limit_Current,NULL,0},
//	{155, 9, Addr_Para_Info,NULL,0},
    
    {1,2,ProductionTEST_ReadFun,ProductionTEST_CtrlFun,2},
    
    {11,DI_NUM,DI_ReadFun,NULL,0},
	{12,DO_NUM-1,DO_ReadFun,DO_CtrlFun,DO_NUM-1},
    {13,ADC_CHANNEL_NUM*2+TEMP_NUM,ADC_ReadFun,NULL,0},

	{20,127,FParaReadFun,FParaSetFun,78},
	{21,EPARA_ASP_NUM*4,EParaReadFun,EParaSetFun,EPARA_ASP_NUM},

    {30, 44, SampleResReadFun,NULL,0},
	{31, 9, Addr_Para_Info,NULL,0},
	{32, 46, Soc_ReadFun,NULL,0},
	{33, 13, Limit_Current,NULL,0},
	{40, 64, StatsInfoReadFun,NULL,0},
	
	{10,29,BootInfo_ReadFun,BootInfoSetFun,3},
#if (PACK_NUM>=1)    
    {51, 20, PackInfoReadFun_01,NULL,0},
//    {101, PACK_CELL_NUM*2, PackCellReadFun_01,NULL,0},
//    {151, PACK_CELL_TEMP_NUM, PackTempReadFun_01,NULL,0},
#endif
#if (PACK_NUM>=2)    
    {52, 20, PackInfoReadFun_02,NULL,0},
#endif
#if (PACK_NUM>=3)    
    {53, 20, PackInfoReadFun_03,NULL,0},
#endif
#if (PACK_NUM>=4)    
    {54, 20, PackInfoReadFun_04,NULL,0},
#endif
    
    
};
/** 
* @brief  CAN_DEBUG协议Asp初始化
* @param  none
* @return  none
* @note:
*
*/
void DebugAspCfgInit()      //初始化得在CAN协议栈初始化之前
{
    DebugReadType *p_read = NULL;
    DebugWriteType *p_write = NULL;
        
    for(uint8_t i = 0; i < DEBUG_ASP_NUM; i++)
    {
        if(DebugMessage_Asp[i].read_data_fun != NULL)
        {
            DebugReadType *next = p_read;
            p_read = (DebugReadType*)m_malloc(sizeof(DebugReadType));
            p_read->next = next;
            p_read->ch_num =  DebugMessage_Asp[i].ch_num;
            p_read->frames = (DebugMessage_Asp[i].total_bytes + 5)/6;
            p_read->read_fun = DebugMessage_Asp[i].read_data_fun;
        }
        if(DebugMessage_Asp[i].write_data_fun != NULL)
        {
            DebugWriteType *next = p_write;
            p_write = (DebugWriteType*)m_malloc(sizeof(DebugWriteType));
            p_write->next = next;
            p_write->buff = (uint8_t*)m_malloc(DebugMessage_Asp[i].data_num*2 + DebugMessage_Asp[i].total_bytes +1);    //写数据格式：T_NUM +TLV...
            p_write->ch_num = DebugMessage_Asp[i].ch_num;
            p_write->write_fun = DebugMessage_Asp[i].write_data_fun;
        }
    }
    DebugCanCfgInit(p_read,p_write);
}
