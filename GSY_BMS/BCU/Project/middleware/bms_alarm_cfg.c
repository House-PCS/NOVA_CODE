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
  
  


#include "bms_alarm_cfg.h"
#include "bms_message.h"
#include "bms_sample_res_bsp.h"
#include "fpara_asp.h"
#include "others.h"
#include "bms_sample_hv_bsp.h"
#include "bms_sox.h"



#define ALARM_CLUE_SETTIME      3000
#define ALARM_I_SETTIME         3000
#define ALARM_II_SETTIME        3000
#define ALARM_III_SETTIME       5000
#define ALARM_MAINT_SETTIME     5000

//uint8_t AlarmCode_I[(ALARM_III_UNM+7)/8]={0};     
//uint8_t AlarmCode_II[(ALARM_III_UNM+7)/8]={0};
//uint8_t AlarmCode_III[(ALARM_III_UNM+7)/8]={0};
//uint8_t AlarmCode_Maint[(ALARM_MAINT_UNM+7)/8]={0};
//实时故障码数据
#if (ALARM_III_UNM <= 8)
uint8_t AlarmCode_I=0; 
uint8_t AlarmCode_II=0;
uint8_t AlarmCode_III=0;
#elif (ALARM_III_UNM <= 16)
uint16_t AlarmCode_I=0; 
uint16_t AlarmCode_II=0;
uint16_t AlarmCode_III=0;
#elif (ALARM_III_UNM <= 32)
uint32_t AlarmCode_I=0; 
uint32_t AlarmCode_II=0;
uint32_t AlarmCode_III=0;
#elif (ALARM_III_UNM <= 64)
uint64_t AlarmCode_I=0; 
uint64_t AlarmCode_II=0;
uint64_t AlarmCode_III=0;
#endif

#if (ALARM_MAINT_UNM <= 8)
uint8_t AlarmCode_Maint=0;
#elif (ALARM_MAINT_UNM <= 16)
uint16_t AlarmCode_Maint=0;
#elif (ALARM_MAINT_UNM <= 32)
uint32_t AlarmCode_Maint=0;
#elif (ALARM_MAINT_UNM <= 64)
uint64_t AlarmCode_Maint=0;
#elif (ALARM_MAINT_UNM <= 96)
uint8_t AlarmCode_Maint[12]={0};
#else
uint8_t AlarmCode_Maint[(ALARM_MAINT_UNM+7)/8]={0};
#endif
///////////////

bool gAlarmTest;
bool AlarmClueReserveFun(void){
//    return gAlarmTest;
    return true;
}

//////////////////////I级////////////////////////////////////
bool AlarmICellVolHighFun(void){
	if(IR_BmsMessage->cell_voltage_max >= IR_PARA_CellVolHigh_I)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_voltage_max + IR_PARA_CellVolHigh_Fall <= IR_PARA_CellVolHigh_I)
    {
        return false;
    }
    return IR_AlarmI_Code&(1<<ALARM_III_CELL_VOL_HIGH);
}
bool AlarmICellVolLowFun (void){
	if(IR_BmsMessage->cell_voltage_min <= IR_PARA_CellVolLow_I)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_voltage_min - IR_PARA_CellVolLow_Fall >= IR_PARA_CellVolLow_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_CELL_VOL_LOW);
}
bool AlarmICellVolDiffFun()
{
	if(IR_BmsMessage->cell_voltage_difference >= IR_PARA_CellVolDiff_I)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_voltage_difference + IR_PARA_CellVolDiff_Fall <= IR_PARA_CellVolDiff_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_CELL_VOL_DIFF);
}
bool AlarmIChargeTempHighFun()
{
	if(IR_BmsMessage->cell_temp_max >= IR_PARA_ChargeTempHigh_I)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_max + IR_PARA_ChargeTempHigh_Fall <= IR_PARA_ChargeTempHigh_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_TEMP_CHARGE_HIGH);
}
bool AlarmIChargeTempLowFun()
{
	if(IR_BmsMessage->cell_temp_min <= IR_PARA_ChargeTempLow_I)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_min - IR_PARA_ChargeTempLow_Fall >= IR_PARA_ChargeTempLow_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_TEMP_CHARGE_LOW);
}
bool AlarmIDisChargeTempHighFun()
{
	if(IR_BmsMessage->cell_temp_max >= IR_PARA_DisChargeTempHigh_I)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_max +IR_PARA_DisChargeTempHigh_Fall <= IR_PARA_DisChargeTempHigh_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_TEMP_DISCHARGE_HIGH);
}
bool AlarmIDisChargeTempLowFun()
{
	if(IR_BmsMessage->cell_temp_min <= IR_PARA_DisChargeTempLow_I)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_min - IR_PARA_DisChargeTempLow_Fall >= IR_PARA_DisChargeTempLow_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_TEMP_DISCHARGE_LOW);
}
bool AlarmITempDiffFun()
{
	if(IR_BmsMessage->cell_temp_difference >= IR_PARA_CellTempDiff_I)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_difference + IR_PARA_CellTempDiff_Fall <= IR_PARA_CellTempDiff_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_CELL_TEMP_DIFF);
}
bool AlarmITotalVolHighFun(void){
	if(IR_BmsMessage->total_voltage >= IR_PARA_TotalVolHigh_I)
    {
		return true;
	}
    else if(IR_BmsMessage->total_voltage + IR_PARA_TotalVolHigh_Fall <= IR_PARA_TotalVolHigh_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_TOTAL_VOL_HIGH);
}
bool AlarmITotalVolLowFun (void){
	if(IR_BmsMessage->total_voltage <= IR_PARA_TotalVolLow_I)
    {
		return true;
	}
    else if(IR_BmsMessage->total_voltage - IR_PARA_TotalVolLow_Fall >= IR_PARA_TotalVolLow_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_TOTAL_VOL_LOW);
}
bool AlarmIChargeCurrHighFun()
{
//	if(IR_SysInfo->current <= -(IR_LimitCurr->charge_limit_curr + 50))
//    {
//        return true;
//    }
//    else if(IR_SysInfo->current >= -IR_LimitCurr->charge_limit_curr)
//    {
//        return false;
//    }
	return IR_AlarmI_Code&(1<<ALARM_III_CHARGE_CURR_HIGH);
}
bool AlarmIDisChargeCurrHighFun()
{
//	if(IR_SysInfo->current >= (IR_LimitCurr->discharge_limit_curr + 50))
//    {
//        return true;
//    }
//    else if(IR_SysInfo->current <= IR_LimitCurr->discharge_limit_curr)
//    {
//        return false;
//    }
	return IR_AlarmI_Code&(1<<ALARM_III_DISCHARGE_CURR_HIGH);
}
bool AlarmIResLowFun()
{
	if(IR_BmsSampleRes->res_main <= IR_PARA_ResLow_I)
    {
		return true;
	}
    else if(IR_BmsSampleRes->res_main - IR_PARA_ResLow_Fall >= IR_PARA_ResLow_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_RES_LOW);
}
bool AlarmIPoleTempHighFun()
{
	if(IR_BmsMessage->pole_temp_max >= IR_PARA_PoleTempHigh_I)
    {
		return true;
	}
    else if(IR_BmsMessage->pole_temp_max + IR_PARA_PoleTempHigh_Fall <= IR_PARA_PoleTempHigh_I)
    {
        return false;
    }
	return IR_AlarmI_Code&(1<<ALARM_III_POLE_TEMP_HIGH);
}
static ALARM_ASPCFG_INFO AlarmIAspCfg[ALARM_III_UNM]=
{
    {
        ALARM_III_CELL_VOL_HIGH,
        AlarmICellVolHighFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_CELL_VOL_LOW,
        AlarmICellVolLowFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
	{
        ALARM_III_CELL_VOL_DIFF,
        AlarmICellVolDiffFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_TEMP_CHARGE_HIGH,
        AlarmIChargeTempHighFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_TEMP_CHARGE_LOW,
        AlarmIChargeTempLowFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_TEMP_DISCHARGE_HIGH,
        AlarmIDisChargeTempHighFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_TEMP_DISCHARGE_LOW,
        AlarmIDisChargeTempLowFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_CELL_TEMP_DIFF,
        AlarmITempDiffFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_TOTAL_VOL_HIGH,
        AlarmITotalVolHighFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_TOTAL_VOL_LOW,
        AlarmITotalVolLowFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_CHARGE_CURR_HIGH,
        AlarmIChargeCurrHighFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_DISCHARGE_CURR_HIGH,
        AlarmIDisChargeCurrHighFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_RES_LOW,
        AlarmIResLowFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
    {
        ALARM_III_POLE_TEMP_HIGH,
        AlarmIPoleTempHighFun,
        ALARM_I_SETTIME,
        ALARM_I_SETTIME
    },
};

//////////////////////II级////////////////////////////////////
bool AlarmIICellVolHighFun(void){
	if(IR_BmsMessage->cell_voltage_max >= IR_PARA_CellVolHigh_II)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_voltage_max + IR_PARA_CellVolHigh_Fall <= IR_PARA_CellVolHigh_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_CELL_VOL_HIGH);
}
bool AlarmIICellVolLowFun (void){
	if(IR_BmsMessage->cell_voltage_min <= IR_PARA_CellVolLow_II)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_voltage_min - IR_PARA_CellVolLow_Fall >= IR_PARA_CellVolLow_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_CELL_VOL_LOW);
}
bool AlarmIICellVolDiffFun()
{
	if(IR_BmsMessage->cell_voltage_difference >= IR_PARA_CellVolDiff_II)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_voltage_difference + IR_PARA_CellVolDiff_Fall <= IR_PARA_CellVolDiff_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_CELL_VOL_DIFF);
}
bool AlarmIIChargeTempHighFun()
{
	if(IR_BmsMessage->cell_temp_max >= IR_PARA_ChargeTempHigh_II)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_max + IR_PARA_ChargeTempHigh_Fall <= IR_PARA_ChargeTempHigh_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_TEMP_CHARGE_HIGH);
}
bool AlarmIIChargeTempLowFun()
{
	if(IR_BmsMessage->cell_temp_min <= IR_PARA_ChargeTempLow_II)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_min - IR_PARA_ChargeTempLow_Fall >= IR_PARA_ChargeTempLow_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_TEMP_CHARGE_LOW);
}
bool AlarmIIDisChargeTempHighFun()
{
	if(IR_BmsMessage->cell_temp_max >= IR_PARA_DisChargeTempHigh_II)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_max +IR_PARA_DisChargeTempHigh_Fall <= IR_PARA_DisChargeTempHigh_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_TEMP_DISCHARGE_HIGH);
}
bool AlarmIIDisChargeTempLowFun()
{
	if(IR_BmsMessage->cell_temp_min <= IR_PARA_DisChargeTempLow_II)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_min - IR_PARA_DisChargeTempLow_Fall >= IR_PARA_DisChargeTempLow_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_TEMP_DISCHARGE_LOW);
}
bool AlarmIITempDiffFun()
{
	if(IR_BmsMessage->cell_temp_difference >= IR_PARA_CellTempDiff_II)
    {
		return true;
	}
    else if(IR_BmsMessage->cell_temp_difference + IR_PARA_CellTempDiff_Fall <= IR_PARA_CellTempDiff_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_CELL_TEMP_DIFF);
}
bool AlarmIITotalVolHighFun(void){
	if(IR_BmsMessage->total_voltage >= IR_PARA_TotalVolHigh_II)
    {
		return true;
	}
    else if(IR_BmsMessage->total_voltage +IR_PARA_TotalVolHigh_Fall <= IR_PARA_TotalVolHigh_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_TOTAL_VOL_HIGH);
}
bool AlarmIITotalVolLowFun (void){
	if(IR_BmsMessage->total_voltage <= IR_PARA_TotalVolLow_II)
    {
		return true;
	}
    else if(IR_BmsMessage->total_voltage - IR_PARA_TotalVolLow_Fall >= IR_PARA_TotalVolLow_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_TOTAL_VOL_LOW);
}
bool AlarmIIChargeCurrHighFun()
{
//    if(IR_SysInfo->current <= -(IR_LimitCurr->charge_limit_curr + 100))
//    {
//        return true;
//    }
//    else if(IR_SysInfo->current >= -IR_LimitCurr->charge_limit_curr)
//    {
//        return false;
//    }
	return IR_AlarmII_Code&(1<<ALARM_III_CHARGE_CURR_HIGH);
}
bool AlarmIIDisChargeCurrHighFun()
{
//	if(IR_SysInfo->current >= (IR_LimitCurr->discharge_limit_curr + 100))
//    {
//        return true;
//    }
//    else if(IR_SysInfo->current <= IR_LimitCurr->discharge_limit_curr)
//    {
//        return false;
//    }
	return IR_AlarmII_Code&(1<<ALARM_III_DISCHARGE_CURR_HIGH);
}
bool AlarmIIResLowFun()
{
	if(IR_BmsSampleRes->res_main <= IR_PARA_ResLow_II)
    {
		return true;
	}
    else if(IR_BmsSampleRes->res_main - IR_PARA_ResLow_Fall >= IR_PARA_ResLow_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_RES_LOW);
}
bool AlarmIIPoleTempHighFun()
{
	if(IR_BmsMessage->pole_temp_max >= IR_PARA_PoleTempHigh_II)
    {
		return true;
	}
    else if(IR_BmsMessage->pole_temp_max - IR_PARA_PoleTempHigh_Fall <= IR_PARA_PoleTempHigh_II)
    {
        return false;
    }
	return IR_AlarmII_Code&(1<<ALARM_III_POLE_TEMP_HIGH);
}
static ALARM_ASPCFG_INFO AlarmIIAspCfg[ALARM_III_UNM]=
{
    {
        ALARM_III_CELL_VOL_HIGH,
        AlarmIICellVolHighFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_CELL_VOL_LOW,
        AlarmIICellVolLowFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
	{
        ALARM_III_CELL_VOL_DIFF,
        AlarmIICellVolDiffFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_TEMP_CHARGE_HIGH,
        AlarmIIChargeTempHighFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_TEMP_CHARGE_LOW,
        AlarmIIChargeTempLowFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_TEMP_DISCHARGE_HIGH,
        AlarmIIDisChargeTempHighFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_TEMP_DISCHARGE_LOW,
        AlarmIIDisChargeTempLowFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_CELL_TEMP_DIFF,
        AlarmIITempDiffFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_TOTAL_VOL_HIGH,
        AlarmIITotalVolHighFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_TOTAL_VOL_LOW,
        AlarmIITotalVolLowFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_CHARGE_CURR_HIGH,
        AlarmIIChargeCurrHighFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_DISCHARGE_CURR_HIGH,
        AlarmIIDisChargeCurrHighFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
    {
        ALARM_III_RES_LOW,
        AlarmIIResLowFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
	{
        ALARM_III_POLE_TEMP_HIGH,
        AlarmIIPoleTempHighFun,
        ALARM_II_SETTIME,
        ALARM_II_SETTIME
    },
};
//////////////////////III级////////////////////////////////////
bool AlarmIIICellVolHighFun(void){
	if(IR_BmsMessage->cell_voltage_max >= IR_PARA_CellVolHigh_III)
    {
		return true;
	}
#ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->cell_voltage_max + IR_PARA_CellVolHigh_Fall <= IR_PARA_CellVolHigh_III)
    {
        return false;
    }
#endif
    return IR_AlarmIII_Code&(1<<ALARM_III_CELL_VOL_HIGH);
}
bool AlarmIIICellVolLowFun (void){
	if(IR_BmsMessage->cell_voltage_min <= IR_PARA_CellVolLow_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->cell_voltage_min - IR_PARA_CellVolLow_Fall >= IR_PARA_CellVolLow_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_CELL_VOL_LOW);
}
bool AlarmIIICellVolDiffFun()
{
	if(IR_BmsMessage->cell_voltage_difference >= IR_PARA_CellVolDiff_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->cell_voltage_difference + IR_PARA_CellVolDiff_Fall <= IR_PARA_CellVolDiff_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_CELL_VOL_DIFF);
}
bool AlarmIIIChargeTempHighFun()
{
	if(IR_BmsMessage->cell_temp_max >= IR_PARA_ChargeTempHigh_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->cell_temp_max + IR_PARA_ChargeTempHigh_Fall <= IR_PARA_ChargeTempHigh_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_TEMP_CHARGE_HIGH);
}
bool AlarmIIIChargeTempLowFun()
{
	if(IR_BmsMessage->cell_temp_min <= IR_PARA_ChargeTempLow_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->cell_temp_min - IR_PARA_ChargeTempLow_Fall >= IR_PARA_ChargeTempLow_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_TEMP_CHARGE_LOW);
}
bool AlarmIIIDisChargeTempHighFun()
{
	if(IR_BmsMessage->cell_temp_max >= IR_PARA_DisChargeTempHigh_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->cell_temp_max +IR_PARA_DisChargeTempHigh_Fall <= IR_PARA_DisChargeTempHigh_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_TEMP_DISCHARGE_HIGH);
}
bool AlarmIIIDisChargeTempLowFun()
{
	if(IR_BmsMessage->cell_temp_min <= IR_PARA_DisChargeTempLow_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->cell_temp_min - IR_PARA_DisChargeTempLow_Fall >= IR_PARA_DisChargeTempLow_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_TEMP_DISCHARGE_LOW);
}
bool AlarmIIITempDiffFun()
{
	if(IR_BmsMessage->cell_temp_difference >= IR_PARA_CellTempDiff_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->cell_temp_difference + IR_PARA_CellTempDiff_Fall <= IR_PARA_CellTempDiff_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_CELL_TEMP_DIFF);
}
bool AlarmIIITotalVolHighFun(void){
	if(IR_BmsMessage->total_voltage >= IR_PARA_TotalVolHigh_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->total_voltage +IR_PARA_TotalVolHigh_Fall <= IR_PARA_TotalVolHigh_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_TOTAL_VOL_HIGH);
}
bool AlarmIIITotalVolLowFun (void){
	if(IR_BmsMessage->total_voltage <= IR_PARA_TotalVolLow_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->total_voltage - IR_PARA_TotalVolLow_Fall >= IR_PARA_TotalVolLow_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_TOTAL_VOL_LOW);
}
bool AlarmIIIChargeCurrHighFun()
{
//    if(IR_SysInfo->current <= -(IR_LimitCurr->charge_limit_curr + 250))
//    {
//        return true;
//    }
    #ifdef FAULT_RESUMABLE
    else if(IR_SysInfo->current >= -IR_LimitCurr->charge_limit_curr)
    {
        return false;
    }
    #endif
	if(IR_SysInfo->current <= -IR_PARA_ChargeCurrHigh_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_SysInfo->current - IR_PARA_ChargeCurrHigh_Fall >= -IR_PARA_ChargeCurrHigh_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_CHARGE_CURR_HIGH);
}
bool AlarmIIIDisChargeCurrHighFun()
{
//    if(IR_SysInfo->current >= (IR_LimitCurr->discharge_limit_curr + 250))
//    {
//        return true;
//    }
    #ifdef FAULT_RESUMABLE
    else if(IR_SysInfo->current <= IR_LimitCurr->discharge_limit_curr)
    {
        return false;
    }
    #endif
	if(IR_SysInfo->current >= IR_PARA_DisChargeCurrHigh_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_SysInfo->current +IR_PARA_DisChargeCurrHigh_Fall <= IR_PARA_DisChargeCurrHigh_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_DISCHARGE_CURR_HIGH);
}
bool AlarmIIIResLowFun()
{
	if(IR_BmsSampleRes->res_main <= IR_PARA_ResLow_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsSampleRes->res_main - IR_PARA_ResLow_Fall >= IR_PARA_ResLow_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_RES_LOW);
}
bool AlarmIIIPoleTempHighFun()
{
	if(IR_BmsMessage->pole_temp_max >= IR_PARA_PoleTempHigh_III)
    {
		return true;
	}
    #ifdef FAULT_RESUMABLE
    else if(IR_BmsMessage->pole_temp_max - IR_PARA_PoleTempHigh_Fall <= IR_PARA_PoleTempHigh_III)
    {
        return false;
    }
    #endif
	return IR_AlarmIII_Code&(1<<ALARM_III_POLE_TEMP_HIGH);
}
static const ALARM_ASPCFG_INFO AlarmIIIAspCfg[ALARM_III_UNM]=
{
    {
        ALARM_III_CELL_VOL_HIGH,
        AlarmIIICellVolHighFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_CELL_VOL_LOW,
        AlarmIIICellVolLowFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
	{
        ALARM_III_CELL_VOL_DIFF,
        AlarmIIICellVolDiffFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_TEMP_CHARGE_HIGH,
        AlarmIIIChargeTempHighFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_TEMP_CHARGE_LOW,
        AlarmIIIChargeTempLowFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_TEMP_DISCHARGE_HIGH,
        AlarmIIIDisChargeTempHighFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_TEMP_DISCHARGE_LOW,
        AlarmIIIDisChargeTempLowFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_CELL_TEMP_DIFF,
        AlarmIIITempDiffFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_TOTAL_VOL_HIGH,
        AlarmIIITotalVolHighFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_TOTAL_VOL_LOW,
        AlarmIIITotalVolLowFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_CHARGE_CURR_HIGH,
        AlarmIIIChargeCurrHighFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_DISCHARGE_CURR_HIGH,
        AlarmIIIDisChargeCurrHighFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
    {
        ALARM_III_RES_LOW,
        AlarmIIIResLowFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
	{
        ALARM_III_POLE_TEMP_HIGH,
        AlarmIIIPoleTempHighFun,
        ALARM_III_SETTIME,
        ALARM_III_SETTIME
    },
};
//////////////////////MAINT////////////////////////////////////
bool AlarmMaintPackComFun()
{
    uint8_t i = 0;
    for(i = 0; i < PACK_NUM; i++)
    {
        if((gPackMessage[i].com_state&PACK_COM_READY) != PACK_COM_READY)
        {
            return true;
        }
    }
//    return false;   //恢复
	return IR_AlarmMaint_Code&(1<<ALARM_MAINT_PACK_COM_FAULT);
}
bool AlarmMaintCellSampleFaultFun()
{
    uint8_t i = 0;
    for(i = 0; i < PACK_NUM; i++)
    {
        if(gPackMessage[i].alarm_info.cell_sample_open || gPackMessage[i].alarm_info.cell_sample_short)
        {
            return true;
        }
    }
//    return false;   //恢复
	return IR_AlarmMaint_Code&(1<<ALARM_MAINT_SAMPLE_VOL_FAULT);
}
bool AlarmMaintTempSampleFaultFun()
{
    uint8_t i = 0;
    for(i = 0; i < PACK_NUM; i++)
    {
        if(gPackMessage[i].alarm_info.temp_sample_open || gPackMessage[i].alarm_info.temp_sample_short)
        {
            return true;
        }
    }
//    return false;   //恢复
	return IR_AlarmMaint_Code&(1<<ALARM_MAINT_SAMPLE_TEMP_FAULT);
}
bool AlarmMaintBalanceFaultFun()
{
    uint8_t i = 0;
    for(i = 0; i < PACK_NUM; i++)
    {
        if(gPackMessage[i].alarm_info.balance_open || gPackMessage[i].alarm_info.balance_short)
        {
            return true;
        }
    }
//    return false;   //恢复
	return IR_AlarmMaint_Code&(1<<ALARM_MAINT_BALANCE_PASV_FAULT);
}
bool AlarmMaintTotalVolSampleFaultFun()
{
    if(ABS_DIFF(IR_BmsSampleHV->hv_voltage[GPV], IR_BmsMessage->total_voltage) >= IR_PARA_SampleTotalVolDiff)
    {
        return true;
    }
    else
    {
        return false;
    }
	return IR_AlarmMaint_Code&(1<<ALARM_MAINT_TOTAL_VOL_SAMPLE_FAULT);
}
bool AlarmMaintRelayKnFaultFun()
{
    if(IR_RelayKnCtrl != IR_RelayKnState)
    {
        return true;
    }
	return IR_AlarmMaint_Code&(1<<ALARM_MAINT_RELAY_KN_FAULT);
}
bool AlarmMaintRelayKmFaultFun()
{
    if(IR_RelayKmCtrl != IR_RelayKmState)
    {
        return true;
    }
	return IR_AlarmMaint_Code&(1<<ALARM_MAINT_RELAY_KM_FAULT);
}
bool AlarmMaintPreChargeFaultFun()
{
    if(IR_PreChargeErr)
    {
        return true;
    }
	return IR_AlarmMaint_Code&(1<<ALARM_MAINT_PRECHARGE_FAULT);
}
bool AlarmMaintPcsComFaultFun()
{
    if(IR_PcsComTimeOut)
    {
        return true;
    }
    else
    {
        return false;
    }
	return IR_AlarmMaint_Code&(1<<ALARM_MAINT_COM_PCS_FAULT);
}
static ALARM_ASPCFG_INFO AlarmMaintAspCfg[ALARM_MAINT_UNM]=
{
    {
        ALARM_MAINT_PACK_COM_FAULT,
        AlarmMaintPackComFun,
        ALARM_MAINT_SETTIME,
        ALARM_MAINT_SETTIME,
    },
    {
        ALARM_MAINT_SAMPLE_VOL_FAULT,
        AlarmMaintCellSampleFaultFun,
        ALARM_MAINT_SETTIME,
        ALARM_MAINT_SETTIME,
    },
    {
        ALARM_MAINT_SAMPLE_TEMP_FAULT,
        AlarmMaintTempSampleFaultFun,
        ALARM_MAINT_SETTIME,
        ALARM_MAINT_SETTIME,
    },
    {
        ALARM_MAINT_BALANCE_PASV_FAULT,
        AlarmMaintBalanceFaultFun,
        ALARM_MAINT_SETTIME,
        ALARM_MAINT_SETTIME,
    },
	{
        ALARM_MAINT_TOTAL_VOL_SAMPLE_FAULT,
        AlarmMaintTotalVolSampleFaultFun,
        ALARM_MAINT_SETTIME,
        ALARM_MAINT_SETTIME,
    },
    {
        ALARM_MAINT_RELAY_KN_FAULT,
        AlarmMaintRelayKnFaultFun,
        ALARM_MAINT_SETTIME,
        ALARM_MAINT_SETTIME,
    },
    {
        ALARM_MAINT_RELAY_KM_FAULT,
        AlarmMaintRelayKmFaultFun,
        ALARM_MAINT_SETTIME,
        ALARM_MAINT_SETTIME,
    },
    {
        ALARM_MAINT_PRECHARGE_FAULT,
        AlarmMaintPreChargeFaultFun,
        ALARM_MAINT_SETTIME,
        ALARM_MAINT_SETTIME,
    },
    {
        ALARM_MAINT_COM_PCS_FAULT,
        AlarmMaintPcsComFaultFun,
        ALARM_MAINT_SETTIME,
        ALARM_MAINT_SETTIME,
    },
};

//////////////////////////////////////////\\\\\\\\///////////////////////////////////////
static const ALARM_ASPCFG_INFO*	AlarmAspCfgTab[ALARM_TOTAL]={
//	AlarmClueAspCfg,
    AlarmIAspCfg,AlarmIIAspCfg,AlarmIIIAspCfg,AlarmMaintAspCfg
};

//static AlarmPartsType AlarmClueParts[ALARM_CLUE_NUM]={0};
static AlarmPartsType AlarmIParts[ALARM_III_UNM]={0};
static AlarmPartsType AlarmIIParts[ALARM_III_UNM]={0};
static AlarmPartsType AlarmIIIParts[ALARM_III_UNM]={0};
static AlarmPartsType AlarmMaintParts[ALARM_MAINT_UNM]={0};

static AlarmPacketType alarmPacket[ALARM_TOTAL]={
//	{
//		ALARM_CLUE_NUM,
//		AlarmClueParts,
//		&Alarm_AspCude[ALARM_CLUE]
//	},
	{
		ALARM_III_UNM,
		AlarmIParts,
		(uint8_t*)&AlarmCode_I,
	},
	{
		ALARM_III_UNM,
		AlarmIIParts,
        (uint8_t*)&AlarmCode_II,
	},
	{
		ALARM_III_UNM,
		AlarmIIIParts,
        (uint8_t*)&AlarmCode_III,
	},
	{
		ALARM_MAINT_UNM,
		AlarmMaintParts,
        (uint8_t*)&AlarmCode_Maint,
	},
};
	
/** 
* @brief  告警模块初始化
* @param  none
* @return  none
* @note:
*
*/
void BmsAlarmAspInit( ){
	uint8_t i=0,j=0;
	
	for(i=0;i< ALARM_TOTAL;i++){
		for(j=0;j<alarmPacket[i].AlarmNum;j++){
			alarmPacket[i].AlarmParts[j].AlarmCode          =AlarmAspCfgTab[i][j].AlarmCode;
			alarmPacket[i].AlarmParts[j].AlarmParseFunc     =AlarmAspCfgTab[i][j].AlarmParseFunc;
			alarmPacket[i].AlarmParts[j].EnabledTime		=AlarmAspCfgTab[i][j].EnabledTime;
			alarmPacket[i].AlarmParts[j].DisenabledTime     =AlarmAspCfgTab[i][j].DisenabledTime;
		}
//        AlarmInfo.AlarmPacket[i].alarmCode = (uint8_t *)malloc(AlarmCodeBytesSizeof(alarmPacket[i].AlarmNum));
	}
	AlarmInfo.AlarmTypeNum=ALARM_TOTAL;
	AlarmInfo.AlarmPacket=alarmPacket;	
	

}
