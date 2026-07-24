#ifndef __ALARM_ASPCFG_H_
#define __ALARM_ASPCFG_H_

#include <stdint.h>
#include "bms_alarm.h"




#define ALARM_CLUE_NUM      0
#define ALARM_III_UNM		14
#define ALARM_MAINT_UNM     22

enum AlarmType{
#if ALARM_CLUE_NUM
	ALARM_CLUE,         //提示报警信息，一般不做保护处理
#endif
#if ALARM_III_UNM
    ALARM_I,
	ALARM_II,
	ALARM_III,          //三级报警
#endif
#if ALARM_MAINT_UNM
	ALARM_MAINT,        //维护报警
#endif
	
	ALARM_TOTAL,
};


////////////////////////////报警码定义////////////////

//////提示报警码///////
#define ALARM_CLUE_RESERVE			0x00000001

//////三级报警报警码///////
//#define ALARM_I_CELL_VOL_HIGH			0			//单体电压高
//#define ALARM_I_CELL_VOL_LOW			1			//单体电压低
//#define ALARM_I_CELL_VOL_DIFF			2           //单体压差大

//#define ALARM_I_TEMP_CHARGE_HIGH        3
//#define ALARM_I_TEMP_CHARGE_LOW			4
//#define ALARM_I_TEMP_DISCHARGE_HIGH     5
//#define ALARM_I_TEMP_DISCHARGE_LOW      6
//#define ALARM_I_CELL_TEMP_DIFF          7           //单体温差大

//#define ALARM_I_TOTAL_VOL_HIGH          8
//#define ALARM_I_TOTAL_VOL_LOW			9

//#define ALARM_I_CHARGE_CURR_HIGH        10
//#define ALARM_I_DISCHARGE_CURR_HIGH     11

//#define ALARM_I_RES_LOW                 12           //绝缘低
//#define ALARM_I_POLE_TEMP_HIGH          13           //极柱温度高

////////二级报警码///////
//#define ALARM_II_CELL_VOL_HIGH              0			//单体电压高
//#define ALARM_II_CELL_VOL_LOW               1			//单体电压低
//#define ALARM_II_CELL_VOL_DIFF              2           //单体压差大

//#define ALARM_II_TEMP_CHARGE_HIGH           3
//#define ALARM_II_TEMP_CHARGE_LOW            4
//#define ALARM_II_TEMP_DISCHARGE_HIGH        5
//#define ALARM_II_TEMP_DISCHARGE_LOW         6
//#define ALARM_II_CELL_TEMP_DIFF             7           //单体温差大

//#define ALARM_II_TOTAL_VOL_HIGH             8
//#define ALARM_II_TOTAL_VOL_LOW              9

//#define ALARM_II_CHARGE_CURR_HIGH           10
//#define ALARM_II_DISCHARGE_CURR_HIGH        11

//#define ALARM_II_RES_LOW                    12           //绝缘低
//#define ALARM_II_POLE_TEMP_HIGH          13           //极柱温度高


//////三级报警报警码///////
#define ALARM_III_CELL_VOL_HIGH              0			//单体电压高
#define ALARM_III_CELL_VOL_LOW               1			//单体电压低
#define ALARM_III_CELL_VOL_DIFF              2           //单体压差大

#define ALARM_III_TEMP_CHARGE_HIGH           3
#define ALARM_III_TEMP_CHARGE_LOW            4
#define ALARM_III_TEMP_DISCHARGE_HIGH        5
#define ALARM_III_TEMP_DISCHARGE_LOW         6
#define ALARM_III_CELL_TEMP_DIFF             7           //单体温差大

#define ALARM_III_TOTAL_VOL_HIGH             8
#define ALARM_III_TOTAL_VOL_LOW              9

#define ALARM_III_CHARGE_CURR_HIGH           10
#define ALARM_III_DISCHARGE_CURR_HIGH       11

#define ALARM_III_RES_LOW                   12           //绝缘低
#define ALARM_III_POLE_TEMP_HIGH            13           //极柱温度高



//////维护报警码///////
#define ALARM_MAINT_PACK_HARDWARE_FAULT         0			//从控硬件故障
#define ALARM_MAINT_PACK_COM_FAULT              1			//从控通讯异常
#define ALARM_MAINT_SAMPLE_VOL_FAULT            2			//电压采集故障
#define ALARM_MAINT_SAMPLE_TEMP_FAULT           3			//温度采集故障
#define ALARM_MAINT_BALANCE_PASV_FAULT          4			//被动均衡故障
#define ALARM_MAINT_BALANCE_ACTIVE_FAULT        5			//主动均衡故障
#define ALARM_MAINT_CELL_VOL_HIGH               6			//单体电芯过压
#define ALARM_MAINT_CELL_VOL_LOW                7			//单体电芯欠压

#define ALARM_MAINT_FAN_FAULT                   11			//风扇故障
#define ALARM_MAINT_TOTAL_VOL_SAMPLE_FAULT      12			//总压采集故障
#define ALARM_MAINT_RELAY_KN_FAULT              13			//主负继电器故障
#define ALARM_MAINT_RELAY_KM_FAULT              14			//主正继电器故障
#define ALARM_MAINT_PRECHARGE_FAULT             15			//预充故障

#define ALARM_MAINT_BAT_NOT_ENABLE              16			//电池未使能
#define ALARM_MAINT_COM_PCS_FAULT               17			//PCS通讯故障
#define ALARM_MAINT_COM_EMS_FAULT               18			//EMS通讯故障
#define ALARM_MAINT_EMERGENCY_STOP              19			//急停
#define ALARM_MAINT_STWRONG_CONTROL_PASV        20			//EMS/PCS强控继电器
#define ALARM_MAINT_STWRONG_CONTROL_ACTIVE      21			//手动强控继电器

/////对外数据接口//////
////故障码读取
#if (ALARM_III_UNM <= 8)
#define IR_AlarmI_Code          (*IR_AlarmInfo->AlarmPacket[ALARM_I].alarmCode)
#define IR_AlarmII_Code         (*IR_AlarmInfo->AlarmPacket[ALARM_II].alarmCode)
#define IR_AlarmIII_Code        (*IR_AlarmInfo->AlarmPacket[ALARM_III].alarmCode)
#elif (ALARM_III_UNM <= 16)
#define IR_AlarmI_Code          (*(uint16_t*)IR_AlarmInfo->AlarmPacket[ALARM_I].alarmCode)
#define IR_AlarmII_Code         (*(uint16_t*)IR_AlarmInfo->AlarmPacket[ALARM_II].alarmCode)
#define IR_AlarmIII_Code        (*(uint16_t*)IR_AlarmInfo->AlarmPacket[ALARM_III].alarmCode)
#elif (ALARM_III_UNM <= 32)
#define IR_AlarmI_Code          (*(uint32_t*)IR_AlarmInfo->AlarmPacket[ALARM_I].alarmCode)
#define IR_AlarmII_Code         (*(uint32_t*)IR_AlarmInfo->AlarmPacket[ALARM_II].alarmCode)
#define IR_AlarmIII_Code        (*(uint32_t*)IR_AlarmInfo->AlarmPacket[ALARM_III].alarmCode)
#elif (ALARM_III_UNM <= 64)
#define IR_AlarmI_Code          (*(uint64_t*)IR_AlarmInfo->AlarmPacket[ALARM_I].alarmCode)
#define IR_AlarmII_Code         (*(uint64_t*)IR_AlarmInfo->AlarmPacket[ALARM_II].alarmCode)
#define IR_AlarmIII_Code        (*(uint64_t*)IR_AlarmInfo->AlarmPacket[ALARM_III].alarmCode)
#endif

#if (ALARM_MAINT_UNM <= 8)
#define IR_AlarmMaint_Code      (*IR_AlarmInfo->AlarmPacket[ALARM_MAINT].alarmCode)
#elif (ALARM_MAINT_UNM <= 16)
#define IR_AlarmMaint_Code      (*(uint16_t*)IR_AlarmInfo->AlarmPacket[ALARM_MAINT].alarmCode)
#elif (ALARM_MAINT_UNM <= 32)
#define IR_AlarmMaint_Code      (*(uint32_t*)IR_AlarmInfo->AlarmPacket[ALARM_MAINT].alarmCode)
#elif (ALARM_MAINT_UNM <= 64)
#define IR_AlarmMaint_Code      (*(uint64_t*)IR_AlarmInfo->AlarmPacket[ALARM_MAINT].alarmCode)
#endif

#define AlarmMaintCode(x)   ((IR_AlarmInfo->AlarmPacket[ALARM_MAINT].alarmCode[x/8]>>(x%8))&0x1)
typedef struct {
	uint8_t AlarmCode;		//故障位码
	bool (*AlarmParseFunc)(void);		//故障条件判断函数
	uint16_t EnabledTime;			//故障产生时间
	uint16_t DisenabledTime;		//故障消除时间
	
}ALARM_ASPCFG_INFO;

void BmsAlarmAspInit(void);

#endif
