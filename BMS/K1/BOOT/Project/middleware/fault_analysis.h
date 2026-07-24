 /**
  ******************************************************************************
  * @file    FAULT_ANALYSIS.h
  * @author  NovaStar R&D Team
  * @version V1.0.0
  * @date    2023.12.20
  * @brief   
  ******************************************************************************
  * @attention
  *
  * History<br>
  * version     date        auther      description<br>
  * V1.0.0      2023.12.20   LXD       Initial Version<br>
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/  	
#ifndef __FAULT_ANALYSIS_H	
#define __FAULT_ANALYSIS_H	

///* Inculdes ------------------------------------------------------------------*/

#include "afe_control.h"
#include "debug.h"
#include "modbus_analysis.h"
#include "rtc.h"
#include "dev_at24cxx.h"
#include "sensor_control.h"
#include "dev_system_power.h"


/////* Exported typedef -----------------------------------------------------------*/
#define ERROR1_SAVE_ADDR    0x00000000
#define ERROR2_SAVE_ADDR    0x00000008
#define ERROR3_SAVE_ADDR    0x00000010
#define ERROR4_SAVE_ADDR    0x00000018
#define ERROR5_SAVE_ADDR    0x00000020
#define ERROR6_SAVE_ADDR    0x00000028
#define ERROR7_SAVE_ADDR    0x00000030
#define ERROR8_SAVE_ADDR    0x00000038
#define ERROR9_SAVE_ADDR    0x00000040
#define ERROR10_SAVE_ADDR   0x00000048
#define ERROR11_SAVE_ADDR   0x00000050
#define ERROR12_SAVE_ADDR   0x00000058
#define ERROR13_SAVE_ADDR   0x00000060

typedef struct _Fault_analysis_flag_t//故障分析标志位
{
    uint8_t    mos_adhesion_count		 	 		;//mos粘连故障	
    uint8_t    pmos_fail_close_count         		;//预充mos无法闭合故障
    uint8_t    cmos_fail_close_count         		;//充电mos无法闭合的故障
    uint8_t    dmos_fail_close_count         		;//放电mos无法闭合故障	
    uint8_t    mos_temperature_sensor_error_count   ;//mos温度传感器故障
    uint8_t    mos_temperature_ov1_count    		;//mos过温1级故障
    uint8_t    mos_temperature_ov2_count    		;//mos过温2级故障
    uint8_t    mos_temperature_ov3_count    		;//mos过温3级故障
    uint8_t    premos_enable_flag            		;//预充mos打开标志位
	uint8_t	   pmos_count_flag						;//
	
    uint8_t    cell_under_votage1_count      		;//电芯欠压1级故障
    uint8_t    cell_under_votage2_count      		;//电芯欠压2级故障
    uint8_t    cell_under_votage3_count      		;//电芯欠压3级故障
    uint8_t    cell_over_votage1_count       		;//电芯过压1级故障
    uint8_t    cell_over_votage2_count       		;//电芯过压2级故障
    uint8_t    cell_over_votage3_count       		;//电芯过压3级故障
    uint8_t	   pack_under_vol1_count       	 		;//电池包总电压欠压故障
    uint8_t    pack_over_vol1_count        	 		;//电池包总电压过压故障
    uint8_t    cell_voltage_diff1_count      		;//电芯压差大1级
    uint8_t    cell_voltage_diff2_count      		;//电芯压差大2级
    uint8_t    cell_voltage_diff3_count      		;//电芯压差大3级	
	
    uint8_t    dsg_over_current1_count       		;//放电过流1级故障
    uint8_t    dsg_over_current2_count       		;//放电过流2级故障
    uint8_t    dsg_over_current3_count       		;//放电过流3级故障
    uint8_t    chg_over_current1_count       		;//充电过流1级故障
    uint8_t    chg_over_current2_count       		;//充电过流2级故障
    uint8_t    chg_over_current3_count       		;//充电过流3级故障
    uint8_t    chg_over_current1_5s_flag     		;//充电过流1级故障5s标志位
    uint8_t    chargemos_enable_flag         		;//充电mos打开标志位
    uint8_t    dischargemos_enable_flag      		;//放电mos打开标志位
	
    uint8_t    cell_temperature_ut1_count    		;//电芯低温1级故障
    uint8_t    cell_temperature_ot1_count    		;//电芯过温1级故障
    uint8_t    cell_temperature_ot2_count    		;//电芯过温2级故障
    uint8_t    cell_temperature_ot3_count    		;//电芯过温3级故障
    uint8_t    ntc_temperature_ot_count    		    ;//ntc过温故障
	
    uint8_t    soh_under_lvl1_count          		;//SOH过低1级故障
    uint8_t    soh_under_lvl2_count          		;//SOH过低2级故障
    uint8_t    soh_under_lvl3_count          		;//SOH过低3级故障
	
    uint8_t    cell_votage_sensor_error_count       ;//电芯电压传感器故障
    uint8_t    cell_temperature_sensor_error_count  ;//电芯温度传感器故障
    uint8_t    cell_temperature_cmp_flag     		;//电芯温度传感器故障标志位
    uint8_t    e2prom_error_count                   ;//e2prom故障
    uint8_t    rtc_error_count                      ;// rtc故障
    uint8_t    chg_short_protection_count           ;//充电短路保护
    uint8_t    dsg_short_protection_count           ;// 放电短路保护
    uint8_t    flash_error_flag						;//flash故障标志位
    uint8_t    e2prom_error_flag 					;//e2prom故障标志位
	uint8_t    pre_charge_timeout_count   		    ;//预充超时
	uint8_t    heat_error_count						;//加热膜短路故障

}Fault_analysis_flag;

typedef union
{
    uint16_t value;
    struct
    {
		uint16_t mos_adhesion           : 1;          /* 放电或预充或充电MOS粘连故障  */
        uint16_t pmos_fail_close        : 1;          /* 预充MOS无法闭合故障  */
        uint16_t cmos_fail_close        : 1;          /* 充电MOS无法闭合故障  */
        uint16_t dmos_fail_close        : 1;          /* 放电MOS无法闭合故障  */
        uint16_t mos_ntc_error          : 1;          /* MOS传感器故障  */
        uint16_t mos_high_temper3       : 1;          /* MOS过温3级  */
        uint16_t mos_high_temper2       : 1;          /* MOS过温2级  */
        uint16_t mos_high_temper1       : 1;          /* MOS过温1级  */
        uint16_t reserved               : 8;          /* 预留               */
    } b;

}BMSMosErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
		uint16_t cell_under_vol1       	: 1;           /* 单体欠压1级*/
        uint16_t cell_under_vol2       	: 1;           /* 单体欠压2级*/
        uint16_t cell_under_vol3       	: 1;           /* 单体欠压3级*/
        uint16_t cell_over_vol1        	: 1;           /* 单体过压1级*/
        uint16_t cell_over_vol2        	: 1;           /* 单体过压2级*/
        uint16_t cell_over_vol3        	: 1;           /* 单体过压3级*/
        uint16_t pack_under_vol1       	: 1;           /* 电池包总电压欠压故障*/
        uint16_t pack_over_vol1        	: 1;           /* 电池包总电压过压故障*/
        uint16_t cell_voltage_diff1     : 1;           /* 电芯压差大1级*/
        uint16_t reserved               : 7;           /* 预留 */

    } b;

}BMSVoltage1ErrorCode_t;//BMS电压故障1

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t reserved            : 16;         /* 预留        */
    } b;

}BMSVoltage2ErrorCode_t;//BMS电压故障2 预留

typedef union
{
    uint16_t value;
    struct
    {
		uint16_t discharge_overcurr1 : 1;          /* 放电过流1级 */
        uint16_t discharge_overcurr2 : 1;          /* 放电过流2级 */
        uint16_t discharge_overcurr3 : 1;          /* 放电过流3级 */
        uint16_t charge_overcurr1    : 1;          /* 充电过流3级 */
        uint16_t charge_overcurr2    : 1;          /* 充电过流2级 */
        uint16_t charge_overcurr3    : 1;          /* 充电过流1级 */
        uint16_t reserved            : 10;         /* 预留        */
    } b;

}BMSCurrErrorCode_t;//BMS电流故障码

typedef union
{
    uint16_t value;
    struct
    {
		uint16_t cell_low_temper1       		: 1;           /* 电池温度过低1级故障*/
		uint16_t cell_high_temper1       	    : 1;           /* 电池温度过高1级故障*/
		uint16_t cell_high_temper2       		: 1;           /* 电池温度过高2级故障*/
		uint16_t cell_high_temper3       		: 1;           /* 电池温度过高3级故障*/
		uint16_t ntc_high_diff       		    : 1;           /* ntc温差大故障*/		
        uint16_t reserved               		: 11;          /* 预留  */
    } b;

}BMSTemperErrorCode_t;//BMS温度故障码


typedef union
{
    uint16_t value;
    struct
    {
		uint16_t soh_low_error1       			: 1;           /* SOH过低1级故障*/
		uint16_t soh_low_error2       			: 1;           /* SOH过低2级故障*/
		uint16_t soh_low_error3       			: 1;           /* SOH过低3级故障*/
        uint16_t reserved               		: 13;          /* 预留               */        
    } b;

}BMSStateErrorCode_t;//电芯/电池包状态故障

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t vol_sensor_error       		: 1;           /* 电压传感器故障           */
        uint16_t cell_ntc_error         		: 1;           /* 电池温度传感器故障       */
        uint16_t spi_flash_error                : 1;           /* spi fash故障             */
        uint16_t e2prom_error                   : 1;           /* e2prom故障               */
        uint16_t rtc_error                      : 1;           /* rtc故障                  */
        uint16_t chg_short_protection           : 1;           /* 充电短路保护             */
        uint16_t dsg_short_protection           : 1;           /* 放电短路保护             */
        uint16_t pre_charge_timeout   			: 1;           /* 预充超时故障  */	
        uint16_t heat_error   					: 1;           /* 加热膜短路故障   */			
        uint16_t reserved                 		: 7;           /* 预留        */
    } b;

}BMSOther1ErrorCode_t;//其他故障

typedef struct _bms_com_error_info_t
{
	BMSMosErrorCode_t		    mos_error_code;//mos故障
	BMSVoltage1ErrorCode_t		voltage1_error_code;//电压故障1
	BMSVoltage2ErrorCode_t		voltage2_error_code;//电压故障2 预留
	BMSCurrErrorCode_t          curr_error_code;//电流故障
	BMSTemperErrorCode_t		temper_error_code;//温度故障
	BMSStateErrorCode_t			state_error_code;//状态故障
	BMSOther1ErrorCode_t		other1_error_code;//其他故障	
}BMSErrorInfo_t;
#pragma pack(1)

typedef struct _bms_warning_param_t//定义故障阈值
{
	int32_t  mos_ot1                 ;//充电MOS过温1级
    int32_t  mos_ot2                 ;//充电MOS过温2级
    int32_t  mos_ot3                 ;//充电MOS过温3级
	
	uint32_t cell_uv1                ;//单体电芯欠压1级
    uint32_t cell_uv2                ;//单体电芯欠压2级
    uint32_t cell_uv3                ;//单体电芯欠压3级
    uint32_t cell_ov1                ;//单体电芯过压1级
    uint32_t cell_ov2                ;//单体电芯过压2级
    uint32_t cell_ov3                ;//单体电芯过压3级
    uint32_t pack_uv1                ;//电池包总压欠压级
    uint32_t pack_ov1                ;//电池包总压过压级
    uint32_t cell_voltage_diff       ;//电芯压差大
	
    int32_t  limit_charge_current    ;//充电限制电流
    int32_t  limit_discharge_current ;//放电限制电流
    int32_t  discharge_overcurr1     ;//放电过流1级
    int32_t  discharge_overcurr2     ;//放电过流2级
    int32_t  discharge_overcurr3     ;//放电过流3级
    int32_t  charge_overcurr1        ;//充电过流1级
    int32_t  charge_overcurr2        ;//充电过流2级
    int32_t  charge_overcurr3        ;//充电过流3级
	
    int32_t  cell_ut1             	 ;//电芯温度低1级
    int32_t  cell_ut2             	 ;//电芯温度低2级
    int32_t  cell_ut3             	 ;//电芯温度低3级
    int32_t  cell_ot1             	 ;//电芯温度高1级
    int32_t  cell_ot2             	 ;//电芯温度高2级
    int32_t  cell_ot3             	 ;//电芯温度高3级
    int32_t  ntc_temp_diff           ;//ntc温差大1级
	
    uint32_t soh_under_lvl1          ;//SOH过低1级
    uint32_t soh_under_lvl2          ;//SOH过低2级
	uint32_t soh_under_lvl3          ;//SOH过低3级

    int32_t  tsensor_ut              ;//
    int32_t  tsensor_ot              ;//
    int32_t  zerodrift_current       ;
}BMSWarningParam_t;

extern BMSErrorInfo_t  gBmsErrorInfo;  
extern Fault_analysis_flag gFaultAnalysisFlag;

void FaultAnalysisProcess(void);
void LatchErrorCheck(void);
void ErrorClear(void);
#endif /*__MID_SHELL_H */ 
                
