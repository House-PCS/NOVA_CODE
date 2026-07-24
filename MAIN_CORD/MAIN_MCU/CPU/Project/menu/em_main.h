#ifndef _EM_MAIN_H
#define _EM_MAIN_H

#include <stdint.h>


typedef enum
{
    EN_WORK_MODE_SELF_CONSUMP,
    EN_WORK_MODE_BAT_PRIORITY,
    EN_WORK_MODE_PL_BALANCE,
} EN_WORK_MODE_E;

typedef enum
{
    EN_SYS_STAND_BY = 0,
    EN_SYS_OFF_GRID,
    EN_SYS_ON_GRID,
    EN_SYS_SELF_TEST,
    EN_SYS_BY_PASS,
    EN_SYS_SHUTDOWN,
} EN_SYS_STATUS_E;

typedef enum
{
    EN_LED_STAT_NONE = 0,
    EN_LED_STAT_OK,
    EN_LED_STAT_ERR,
} EN_LED_STAT_E;

typedef enum
{
    EN_LED_TYPE_FAULT = 0,
    EN_LED_TYPE_STATUS,
    EN_LED_TYPE_GRID,
} EN_LED_TYPE_E;

typedef enum
{
    EN_CHARGE_TYPE_NONE = 0,
    EN_CHARGE_TYPE_GRID,
    EN_CHARGE_TYPE_PV,
    EN_CHARGE_TYPE_GRID_PV,
} EN_CHARGE_TYPE_E;

typedef enum
{
    EN_WIFI_STAT_OFF = 0,
    EN_WIFI_STAT_ON,
    EN_WIFI_STAT_DISCONN,
    EN_WIFI_STAT_CONN,
} EN_WIFI_STAT_E;

/*故障告警状态*/
typedef enum
{
    EN_NORMAL_STAT = 0,     //无故障
    EN_NOTICE_STAT,         //注意
    EN_WARNNING_STAT,       //警告
    EN_FAULT_STAT,          //故障
} EN_FAULT_STAT_E;

/*电网供电状态*/
typedef enum
{
    EN_GRID_DISABLED,      //设备与电网断开
    EN_GRID_ACTIVE_IN,     //设备馈电至电网
    EN_GRID_ACTIVE_OUT,    //电网向设备发电
    EN_GRID_STAT_NUM,
} EN_GRID_STAT_E;
/*家庭供电状态*/
typedef enum
{
    EN_HOME_DISABLED,  //家庭不使用设备供电
    EN_HOME_ACTIVE,    //家庭使用设备供电
    EN_HOME_STAT_NUM,
} EN_HOME_STAT_E;

/*太阳能供电状态*/
typedef enum
{
    EN_SOLAR_DISABLED, //太阳能板不向设备供电
    EN_SOLAR_ACTIVE,   //太阳能板向设备供电
    EN_SOLAR_DAYTIME,  //白天主题下的太阳能供电线
    EN_SOLAR_STAT_NUM,
} EN_SOLAR_STAT_E;

/**
 * @brief 设置LED灯的状态
 *
 * @param type
 * @param stat
 *
 * @return
 */
int32_t SetLedStat(EN_LED_TYPE_E type, EN_LED_STAT_E stat);

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
int32_t SetWifiStat(EN_WIFI_STAT_E stat);

/**
 * @brief 设置当前时间
 *
 * @param t
 *
 * @return
 */
int32_t SetTime(uint32_t t);

#endif /* EM_MAIN_H */
