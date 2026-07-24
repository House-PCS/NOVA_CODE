#ifndef _EM_MAIN_H
#define _EM_MAIN_H

#include <stdint.h>

typedef enum _PCS_STAT_E
{
	PCS_STAT_SHUTDOWN = 0,
	PCS_STAT_STAND_BY,
	PCS_STAT_SELF_TEST,
	PCS_STAT_BY_PASS,
	PCS_STAT_OFF_GRID,
	PCS_STAT_ON_GRID,
} PCS_STAT_E;

typedef enum _WORK_MODE_E
{
	WORK_MODE_SELF_CONSUMP = 0,
	WORK_MODE_PL_BALANCE,
	WORK_MODE_BAT,
} WORK_MODE_E;

typedef enum _LED_STAT_E
{
	LED_STAT_NONE = 0,
	LED_STAT_OK,
	LED_STAT_ERR,
} LED_STAT_E;

typedef enum _LED_TYPE_E
{
	LED_TYPE_FAULT = 0,
	LED_TYPE_STATUS,
	LED_TYPE_GRID,
} LED_TYPE_E;

typedef enum _CHARGE_TYPE_E
{
	CHARGE_TYPE_NONE = 0,
	CHARGE_TYPE_GRID,
	CHARGE_TYPE_PV,
	CHARGE_TYPE_GRID_PV,
} CHARGE_TYPE_E;

typedef enum _WIFI_STAT_E
{
	WIFI_STAT_OFF = 0,
	WIFI_STAT_ON,
	WIFI_STAT_DISCONN,
	WIFI_STAT_CONN,
} WIFI_STAT_E;

/*故障告警状态*/
typedef enum _FAULT_STAT_E
{
	FAULT_STAT = 0,   	//故障
	WARNNING_STAT,		//警告
	NORMAL_STAT,		//无故障
	FAULT_STAT_NUM,
}FAULT_STAT_E;

/*电网供电状态*/
typedef enum _GRID_STAT_E
{
	GRID_ACTIVE_IN,   	//设备馈电至电网
	GRID_ACTIVE_OUT,	//电网向设备发电
	GRID_DISABLED,		//设备与电网断开
	GRID_STAT_NUM,
}GRID_STAT_E;
/*家庭供电状态*/
typedef enum _HOME_STAT_E
{
	HOME_ACTIVE,    //家庭使用设备供电
	HOME_DISABLED,	//家庭不使用设备供电
	HOME_STAT_NUM,
}HOME_STAT_E;

/*太阳能供电状态*/
typedef enum _SOLAR_STAT_E
{
	SOLAR_ACTIVE,	//太阳能板向设备供电
	SOLAR_DISABLED,	//太阳能板不向设备供电
	SOLAR_DAYTIME,	//白天主题下的太阳能供电线
	SOLAR_STAT_NUM,
}SOLAR_STAT_E;

/**
 * @brief 设置LED灯的状态
 *
 * @param type
 * @param stat
 *
 * @return 
 */
int32_t SetLedStat(LED_TYPE_E type, LED_STAT_E stat);

/**
 * @brief 设置当前电量
 *
 * @param percent
 *
 * @return 
 */
int32_t SetChargePercent(uint32_t percent);

/**
 * @brief 设置WIFI状态
 *
 * @param stat
 *
 * @return 
 */
int32_t SetWifiStat(WIFI_STAT_E stat);

/**
 * @brief 设置当前时间
 *
 * @param t
 *
 * @return 
 */
int32_t SetTime(uint32_t t);

#endif /* EM_MAIN_H */ 
