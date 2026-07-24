/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        atmos_led.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-25
  * @brief       氛围灯控制业务
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-25    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __ATMOS_LED_H_
#define __ATMOS_LED_H_

#include "dev_atmos_sk68xx.h"




#define PACK_NUMS		(3)


typedef enum
{
	EN_ATMOS_STA_INVALID,
	
    EN_ATMOS_STA_INV_NORMAL,	    	/*逆变器正常运行*/
	EN_ATMOS_STA_INV_CLOSE,				/*停机 灰色常亮*/
	EN_ATMOS_STA_INV_STANDBY,			/*待机   黄色灯常亮*/
	EN_ATMOS_STA_INV_CHECK,				/*自检  黄色呼吸闪烁*/
	EN_ATMOS_STA_INV_OFFGRID,			/*离网运行 绿色呼吸闪烁*/
	EN_ATMOS_STA_INV_BYPASS,			/*旁路模式  蓝色常亮*/
	
	EN_ATMOS_STA_WIFI_NORMAL,			/*采集器正常运行 绿色常亮*/
    EN_ATMOS_STA_WIFI_DISCONN,        	/*未连接路由器 呼吸闪烁*/
	EN_ATMOS_STA_WIFI_DISCONN_CLOUD,   	/*未连接云平台 黄色常亮*/
	EN_ATMOS_STA_WIFI_DISCONN_MONITOR,	/*未连接监控卡 灰色常亮*/
	
	EN_ATMOS_STA_SYS_NORMAL,			/*系统正常运行 绿色常亮*/
	EN_ATMOS_STA_SYS_ERR,				/*系统故障 红色常亮*/	
	EN_ATMOS_STA_SYS_WARNNING,			/*系统告警 黄色常亮*/
	
	EN_ATMOS_STA_INV_ERR,				/*逆变器故障  侧面灯*/
	EN_ATMOS_STA_PACK_ERR,				/*PACK包故障  侧面灯*/
	EN_ATMOS_STA_PACK_NORMAL,			/*PACK包正常  侧面灯*/
	
}EN_ATMOS_STATUS;

typedef enum
{
   	EN_ATMOS_STATUS_WIFI_DISCONN_MONITOR,	/*未连接监控卡 灰色常亮*/
    EN_ATMOS_STATUS_WIFI_DISCONN,        	/*未连接路由器 呼吸闪烁*/
	EN_ATMOS_STATUS_WIFI_DISCONN_CLOUD,   	/*未连接云平台 黄色常亮*/
    EN_ATMOS_STATUS_WIFI_NORMAL,			/*采集器正常运行 绿色常亮*/
} EN_ATMOS_WIFI_STATUS;

typedef enum
{ 
    EN_ATMOS_STATUS_SYS_NORMAL,			    /*系统正常运行 绿色常亮*/
	EN_ATMOS_STATUS_SYS_WARNNING,			/*系统告警 黄色常亮*/
    EN_ATMOS_STATUS_SYS_ERR,				/*系统故障 红色常亮*/	
} EN_ATMOS_SYS_STATUS;

typedef enum
{
	EN_ATMOS_STATUS_INV_STANDBY,			/*待机      黄色灯常亮  */
	EN_ATMOS_STATUS_INV_CHECK,				/*自检      黄色呼吸闪烁*/
    EN_ATMOS_STATUS_INV_NORMAL,	    	    /*并网发电  绿色常亮    */
	EN_ATMOS_STATUS_INV_OFFGRID,			/*离网运行  绿色呼吸闪烁*/
	EN_ATMOS_STATUS_INV_BYPASS,			    /*旁路模式  蓝色常亮    */ 
    EN_ATMOS_STATUS_INV_CLOSE,				/*停机      灰色常亮    */
} EN_ATMOS_INV_STATUS;

typedef enum
{ 
    EN_ATMOS_STATUS_PACK_NORMAL,			/*PACK正常 绿色常亮*/
	EN_ATMOS_STATUS_PACK_ERR,				/*PACK故障 红色常亮*/				

} EN_ATMOS_PACK_STATUS;

typedef struct _work_mode_t
{
	EN_ATMOS_STATUS _AtmostatusWifi;     	 //当前WIFI的工作模式
	EN_ATMOS_STATUS _AtmostatusWifiPre;		//上一次WIFI的工作模式
	
	EN_ATMOS_STATUS _AtmostatusSys;     	 //当前Sys的工作模式
	EN_ATMOS_STATUS _AtmostatusSysPre;		//上一次Sys的工作模式
	
	EN_ATMOS_STATUS _AtmostatusInv;     	 //当前Inv的工作模式
	EN_ATMOS_STATUS _AtmostatusInvPre;		//上一次Inv的工作模式
} WorkMode_t;

typedef struct _pack_sta_t
{
	uint8_t pack_sta;
	
	EN_ATMOS_STATUS AtmostatusPack;     	 //当前Pack的状态
	EN_ATMOS_STATUS AtmostatusPackPre;		//上一次pack的状态
	
}PackSta_t;


typedef struct test_status_t
{
	uint8_t wifi_sta;
	uint8_t sys_sta;
	uint8_t inv_sta;

}TestStatus;

typedef struct system_status_stru_t
{
	uint8_t wifi_sta;
	uint8_t sys_sta;
	uint8_t inv_sta;
    uint8_t pack_sta;

}RunningStatus_t;

typedef struct pack_soc_stru_t
{
    uint16_t soc;
    uint8_t is_charge;
} PackSocSta_t;

void PowerOnAtmosLedInit(void);
void AtmosLedFuncUpdate(void);
void InvStatus(void);
void WifiStatus(void);
void SysStatus(void);
void SideLightSta(void);
void AtmosLedPackUpdateTest(void);

void AtmosLedStartProcess(void);


void AtmosLedErrAlarmPolling(void);
void AtmosLedErrAlarmInit(void);
void SideAtomsPolling(void);
#endif











