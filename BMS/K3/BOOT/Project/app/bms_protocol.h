
#ifndef __BMS_PROTOCOL_H_
#define __BMS_PROTOCOL_H_

#include "bsp.h"
#include "systick.h"
#include "fault_analysis.h"
#include "charge_control.h"
#include "modbus_slave.h"
#include "74hc595.h"


typedef enum
{
	EN_BMS_RUNSTATE_DISABLE = 0,    //无效状态
	EN_BMS_RUNSTATE_OFF,            //下电
    EN_BMS_RUNSTATE_POWER_HOLD,     //高压
	EN_BMS_RUNSTATE_NORMAL,         //正常运行
    EN_BMS_RUNSTATE_STANDBY,        //待机
    
    EN_BMS_RUNSTATE_NUM = 0xff,
}EN_BMS_RUNSTATE;

#pragma pack(1)
typedef struct _bms_system_info_stru_t
{
    uint8_t                 precharge_state;            //0-断开 1-打开
    uint8_t                 charge_state;               //0-断开 1-打开
    uint8_t                 discharge_state;            //0-断开 1-打开
	uint8_t					bms_shutdown;
	uint8_t					bms_sys_fail_sta;
	uint32_t				bms_bat2_drv_train_enb;
    uint32_t				shutdown_delay_times;
}BMSSysInfo_t;
#pragma pack()

/* BMS 故障类别 */
typedef enum
{
    EN_TINY_FAULT = 0,					//系统无故障或者仅有轻微故障，只记录故障码，系统性能不受影响。
    EN_RESERVED1,                       //
    EN_RESERVED2,                       //
    EN_PERFORMANCE_LIMITED1,			//按电芯不同SOC与温度进行限制功率
    EN_RESERVED3,                       //
    EN_PERFORMANCE_LIMITED2,			//放电：1min内当前功率线性降至200W;充电：限制到当前查表电流75%
    EN_OPEN_REQUEST1,					//1. 1min内当前功率线性降至0kw，BMS等待MCU shutdown命令 ，收到指令或故障产生后90s ，BMS走正常下电流程（主回路电流降到5A以下，再切断MOS）。
                                        //2. 充电的时候立即将功率限到0kw，BMS等待MCU shutdown命令 ，收到指令或故障产生后90s ，BMS需要走正常下电流程（主回路电流降到5A以下，再切MOS）。
    EN_OPEN_REQUEST2,					//立即将功率限到0kw，BMS等待MOS shutdown ，收到指令或1s后BMS走正常下电流程（主回路电流降到5A以下，再切断MOS）。
    EN_IMMEDIATE_OPEN_REQUEST,			//立即将功率限到0kw并自行立即断开MOS
    EN_ERROR_CATEGORY = 0XFF,
}EN_BMS_ERROR_CATEGORY;

/* BMS 系统故障等级 */
typedef enum
{
    EN_SYSTEM_ERROR_LEVEL0 = 0,			
	EN_SYSTEM_ERROR_LEVEL1,	
    EN_SYSTEM_ERROR_LEVEL2,	
    EN_SYSTEM_ERROR_LEVEL3,	
    EN_SYSTEM_ERROR = 0XFF,
}EN_BMS_SYSTEM_ERROR;



void BMSRunStateProcess(void);
void WakeUpInit(void);

#endif














