/**
******************************************************************************
* @copyright (C) COPYRIGHT Nova
* @file sys_info.h
* @author Xi'an NovaStar Tech Co.,LTD.
* @version V1.0.0
* @date 2022-8-30
* @brief
******************************************************************************
* @attention
* @relation
* @par :(修改日志)
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* </table>
******************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYS_INFO_H_
#define __SYS_INFO_H_
#ifdef __cplusplus
extern "C" {
#endif
/* Inculdes ------------------------------------------------------------------*/
#include "stdint.h"
#include <string.h>
#ifdef __cplusplus
}
#endif

/* Exported macro -----------------------------------------------------------*/
#if     !defined _DC_EN               
#define DC1_ENABLE  0
#define DC2_ENABLE  0
#define DC3_ENABLE  0
#define DC4_ENABLE  0
#endif
/*裁剪DC代码*/
#define K 0.1
  

#define ARC_RES_THRESHOLD_ADDR   0x00032000     //暂定阈值存储

#define ARC_SYSINFO_SAVE_ADDR    0x00030000     //暂定系统信息存储地址
/* Exported types ------------------------------------------------------------*/
typedef struct product_info_stru_t
{
    uint16_t            model_id;
    uint16_t            sn[10];
    uint16_t            hw_version;
    uint16_t            sw_version;   
//    uint16_t            dc_num;
}SysInfo_t;
typedef struct channel_currentvalue_stru_t
{
    uint16_t        channel1_currentvalue;
    uint16_t        channel2_currentvalue;
    uint16_t        channel3_currentvalue;
    uint16_t        channel4_currentvalue;
    uint16_t        channel5_currentvalue;
    uint16_t        channel6_currentvalue;
    uint16_t        channel7_currentvalue;
    uint16_t        channel8_currentvalue;
	
}channel_currentvalue_t;

typedef struct channel_arcstrength_stru_t
{
    uint16_t            channel1_arcstrength;
    uint16_t            channel2_arcstrength;
    uint16_t            channel3_arcstrength;
    uint16_t            channel4_arcstrength;
    uint16_t            channel5_arcstrength;
	uint16_t            channel6_arcstrength;
	uint16_t            channel7_arcstrength;
	uint16_t            channel8_arcstrength;
	
}channel_arcstrength_t;

typedef struct sys_thershold_info_stru_t
{
//    uint16_t            reserve1[2];
//    uint16_t            arc_selfcommand;
    uint16_t            arc_threshold1;
    uint16_t            arc_threshold2;
    uint16_t            arc_threshold3;
    uint16_t            arc_threshold4;
    uint16_t            arc_threshold5;
	uint16_t            arc_threshold6;
	uint16_t            arc_threshold7;
	uint16_t            arc_threshold8;
    
    uint16_t            reserve2[5];
    
    uint16_t            impedance_thersholdrx1;
    uint16_t            impedance_thersholdrx2;
    uint16_t            impedance_thersholdrx3;
    uint16_t            impedance_thersholdrx4;
    uint16_t            impedance_thersholdrx5;
    uint16_t            impedance_thersholdrx6;
    uint16_t            impedance_thersholdrx7;
    uint16_t            impedance_thersholdrx8;
    
    uint16_t            impedance_thersholdry1;
    uint16_t            impedance_thersholdry2;
    uint16_t            impedance_thersholdry3;
    uint16_t            impedance_thersholdry4;
    uint16_t            impedance_thersholdry5;
    uint16_t            impedance_thersholdry6;
    uint16_t            impedance_thersholdry7;
    uint16_t            impedance_thersholdry8;
}sys_thershold_info_t;


extern sys_thershold_info_t thershold_info;
extern SysInfo_t gSysInfo;
extern channel_currentvalue_t channel_currentvalue;
extern channel_arcstrength_t  channel_arcstrength;

/* Exported constants --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void SysInfoInit();
void SysInfoRead(void);

void DeviceBaseInfoWrite(void);
void DeviceBaseInfoRead(void);
void ThresholdInfoWriteToFlash(void);
void ThresholdInfoReadFromFlash(void);
void RestartCount_Init(void);
uint32_t RestartCount_Get(void);


static void _ReadBasicInformation();
float sample_current_value(uint16_t value_voltage);
uint16_t gArcState();
uint16_t gArcRealState();
int Max_valuehistory(uint16_t value,uint16_t* addr);
//int res_judge(uint16_t Rx,uint16_t Ry,float thresholdRx,float thresholdRy);
uint16_t res_judge(float Rx,float Ry,float thresholdRx,float thresholdRy);
uint16_t gResRiskState(float COE);


#endif /* __SYS_INFO_H_ */
/************************ (C) COPYRIGHT Nova *************END OF FILE**********/




