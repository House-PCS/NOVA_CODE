
#ifndef __SOX_H_
#define __SOX_H_

#include "bsp.h"
#include "systick.h"
#include "afe_control.h"
#include "mid_shell.h"
#include "dev_spiflash.h"


#define SOX_FIRST_FIX_ADDR      0x100

typedef enum
{
	EN_PACK_CURRENT_IDLE_MODE,
	EN_PACK_CURRENT_DISCHARGE_MODE,
	EN_PACK_CURRENT_CHARGE_MODE
}EN_PACK_CURRENT_MODE;

#pragma pack(1)
typedef struct _soc_stru_t
{
    float       soc_max;    /* 最大允许放电深度 */
    float       soc_min;    /* 最小允许放电深度 */
    float       soc_real;   /* 真实soc */
    float       soc_actl;   /* 计算续航soc */
    float       soc_disp;   /* 仪表显示soc */
	float       soh;        /*初始值100,按常温额定容量计算，不考虑温度*/
}SOX_t;
#pragma pack()

#pragma pack(1)
typedef struct _pack_status_stru_t
{
	int16_t		pack_min_temperature;/*单位:0.1摄氏度*/
	int16_t		pack_max_temperature;/*单位:0.1摄氏度*/
	int16_t		pack_average_temperature;
	int32_t		pack_current;       /*单位:mA, 放电>0, 充电<0*/
	uint16_t	cell_min_voltage;
	uint16_t	cell_max_voltage;
	uint16_t	cell_average_voltage;
	uint32_t	cell_total_voltage;
	
	uint16_t	full_charge_voltage;
	bool		full_charge_flag;
	uint16_t	empty_charge_voltage;
	bool		empty_charge_flag;
	
	uint32_t	cr;/*当前充放电可用容量, cr = ce * soh, ce为电池包额定容量，受温度和电流倍率影响(查表) mAh*/
	uint32_t 	cur_max_dsg_current_10s;
	uint32_t 	cur_max_dsg_current_30s;
	uint32_t 	cur_max_dsg_current_60s;
	uint32_t    cur_max_chg_current;
	// uint8_t		ocv_calibration_flag;/*需要校准时置1*/
	uint32_t	ocv_time_count;/*掉电需要保存,每当EN_PACK_CURRENT_IDLE_MODE模式时，赋值RTC的值，当RTC的值-ocv_time_count > 阈值(如2小时), ocv_calibration_flag置1*/
	
	float	    chg_accumulate_capa_ah;/*单位:Ah*/
	float	    dsg_accumulate_capa_ah;/*单位:Ah*/
        
    float	    chg_accumulate_capa_wh;/*单位:Wh*/
	float	    dsg_accumulate_capa_wh;/*单位:Wh*/
	
	SOX_t		sox_info;
	EN_PACK_CURRENT_MODE mode;
    uint16_t    pack_cycles;        /* 电芯循环次数 */
    uint16_t    current_power;      /* 当前功率 */
    float       current_power_percentage;      /* 当前功率比例系数（0~1，默认1） */
    uint32_t    recharge_times;     /* 预计充满时长/剩余可用时长 */
    uint16_t    soh_rpt;        /*初始值100,按常温额定容量计算，不考虑温度 0.1%*/
}PackStatus_t;
#pragma pack()

#pragma pack(1)
typedef struct _sox_debug_param_stru_t
{
    uint8_t sox_debug_mode;
    int32_t cell_current_debug;
}SoxDebugParam_t;
#pragma pack()

extern PackStatus_t	gPackStatus;
extern PackStatus_t	gPackStatus_debug;
extern SoxDebugParam_t gSoxDebugParam;
extern uint8_t gFlagClear;

void OCVCalibraiton(PackStatus_t *pPackStatus);
void FullChargeCalibration(void);

void CurrentCapaCalc(PackStatus_t *pPackStatus);
void SOCAmpereHourIntegral(uint32_t ms, PackStatus_t *pPackStatus);
void FullChargeRatedVoltageCalc(void);
void PackCurrentModeJudge(PackStatus_t *pPackStatus);

void SoxInit(PackStatus_t *pPackStatus);
void SoxProcess(PackStatus_t *pPackStatus);

void SoxFlashInfoSave(void);
void SoxFlashInfoLoad(void);
void FlagClear(void);

#endif




















