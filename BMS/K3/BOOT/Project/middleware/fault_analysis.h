 /**
  ******************************************************************************
  * @file    shell_netif.h
  * @author  NovaStar R&D Team
  * @version V1.0.0
  * @date    2017.09.28
  * @brief   
  ******************************************************************************
  * @attention
  *
  * History<br>
  * version     date        auther      description<br>
  * V1.0.0      2017.11.13  Huyuxin     Initial Version<br>
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/  	
#ifndef __FAULT_ANALYSIS_H	
#define __FAULT_ANALYSIS_H	

/* Inculdes ------------------------------------------------------------------*/

#include "afe_control.h"
#include "debug.h"
#include "modbus_slave.h"


/* Exported typedef -----------------------------------------------------------*/


typedef struct _Fault_analysis_flag_t
{
    uint8_t    cell_under_votage1_count      ;
    uint8_t    cell_under_votage2_count      ;
    uint8_t    cell_under_votage3_count      ;
    uint8_t    cell_over_votage1_count       ;
    uint8_t    cell_over_votage2_count       ;
    uint8_t    cell_over_votage3_count       ;
    uint8_t    dsg_over_current1_count       ;
    uint8_t    dsg_over_current2_count       ;
    uint8_t    dsg_over_current3_count       ;
    uint8_t    chg_over_current1_count       ;
    uint8_t    chg_over_current2_count       ;
    uint8_t    chg_over_current3_count       ;
    uint8_t    cell_temperature_cmp_flag     ;
    uint8_t    cell_temperature_ot1_count    ;
    uint8_t    cell_temperature_ot2_count    ;
    uint8_t    cell_temperature_ot3_count    ;
    uint8_t    cell_temperature_ut_count     ;
    uint8_t    cell_temperature_ltd_count    ;
    uint8_t    premos_enable_flag            ;
    uint8_t    pmos_count_flag               ;
    uint8_t    chargemos_enable_flag         ;
    uint8_t    dischargemos_enable_flag      ;
    uint8_t    pack_under_votage_count       ;
    uint8_t    pack_over_votage_count        ;
    uint8_t    cell_no_balance_flag          ;
    uint8_t    cell_votage_sensor_error_count       ;
    uint8_t    cmos_temperature_sensor_error_count  ;
    uint8_t    dmos_temperature_sensor_error_count  ;
    uint8_t    emos_temperature_sensor_error_count  ;
    uint8_t    cell_temperature_sensor_error_count  ;
    uint8_t    cell_temperature_sensor_error_count1 ;
    uint8_t    cmos_temperature_ov1_count    ;
    uint8_t    cmos_temperature_ov2_count    ;
    uint8_t    dmos_temperature_ov1_count    ;
    uint8_t    dmos_temperature_ov2_count    ;
    uint8_t    emos_temperature_ov1_count    ;
    uint8_t    emos_temperature_ov2_count    ;
    uint8_t    soc_under_error_count         ;
    uint8_t    zerodrift_error_count         ;
    uint8_t    balance_sensor_error_count    ;

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
        uint16_t cmos_high_temper2      : 1;          /* 充电MOS过温2级  */
        uint16_t cmos_high_temper1      : 1;          /* 充电MOS过温1级  */
        uint16_t dmos_high_temper2      : 1;          /* 放电MOS过温2级  */
        uint16_t dmos_high_temper1      : 1;          /* 放电MOS过温1级  */
        uint16_t emos_high_temper2      : 1;          /* 加电包MOS过温2级  */
        uint16_t emos_high_temper1      : 1;          /* 加电包MOS过温1级  */
        uint16_t reserved               : 5;          /* 预留               */
    } b;

}BMSMosErrorCode_t;

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

}BMSCurrErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
		uint16_t cell_under_vol1   : 1;           /* 单体欠压1级 */
        uint16_t cell_under_vol2   : 1;           /* 单体欠压2级 */
        uint16_t cell_under_vol3   : 1;           /* 单体欠压3级	 */
        uint16_t cell_over_vol1    : 1;           /* 单体过压1级 */
        uint16_t cell_over_vol2    : 1;           /* 单体过压2级 */
        uint16_t cell_over_vol3    : 1;           /* 单体过压3级 */
        uint16_t pack_over_vol      : 1;           /* 电池包总电压过压 */
        uint16_t pack_under_vol     : 1;           /* 电池包总电压欠压 */
        uint16_t pack_high_temper1  : 1;           /* 电池温度过高一级报警 */
        uint16_t pack_high_temper2  : 1;           /* 电池温度过高二级报警 */
        uint16_t pack_high_temper3  : 1;           /* 电池温度过高三级报警 */
        uint16_t pack_low_temper    : 1;           /* 电池温度过低故障 */
        uint16_t pack_temper_differ : 1;           /* 电池温差过大 */
        uint16_t pack_no_banlance   : 1;           /* 电芯不均衡 */
        uint16_t pack_low_soh1      : 1;           /* SOH过低一级 */
        uint16_t pack_low_soh2      : 1;           /* SOH过低二级		 */
    } b;

}BMSCell1ErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t reserved            : 16;         /* 预留        */
    } b;

}BMSCell2ErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
		uint16_t precharge_curr_revers    : 1;           /* 预充电流反向 */
        uint16_t precharge_timeout        : 1;           /* 预充时间过长 */
        uint16_t precharge_short_circuit3 : 1;           /* 预充短路3次	 */
        uint16_t precharge_short_circuit  : 1;           /* 预充短路 */
        uint16_t reserved                 : 12;          /* 预留        */
    } b;

}BMSPrecErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {                                                                              
		uint16_t cell_vol_sample_miss   : 1;           /* 电芯电压采样线掉线       */
        uint16_t vol_sensor_error       : 1;           /* 电压传感器故障           */
        uint16_t cell_ntc_error         : 1;           /* 电池温度传感器故障       */
        uint16_t cell_ntc_fatal_error   : 1;           /* 电池温度传感器故障(严重) */  
        uint16_t reserved               : 12;          /* 预留                     */
    } b;

}BMSCellSampleErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {                                                                                    
        uint16_t curr_sensror_zerodrift_error   : 1;           /* 电流传感器零漂过大故障  */
        uint16_t balance_loop_error             : 1;           /* 均衡回路故障            */
        uint16_t soc_over_error                 : 1;           /* SOC过高故障             */
        uint16_t soc_under_error                : 1;           /* SOC过低故障             */
        uint16_t soc_jump_error                 : 1;           /* SOC跳变故障             */
        uint16_t balance_temper_invalid         : 1;           /* 均衡温度无效故障        */
        uint16_t reserved                       : 10;          /* 预留                    */
    } b;

}BMSOther1ErrorCode_t;

typedef struct _bms_com_error_info_t
{
	BMSMosErrorCode_t		    mos_error_code;
	BMSCurrErrorCode_t          curr_error_code;
	BMSCell1ErrorCode_t		    cell1_error_code;
    BMSCell2ErrorCode_t		    cell2_error_code;
	BMSPrecErrorCode_t		    prec_error_code;
	BMSCellSampleErrorCode_t	cellsample_error_code;
	BMSOther1ErrorCode_t		other1_error_code;
}BMSErrorInfo_t;
#pragma pack(1)

extern BMSErrorInfo_t  gBmsErrorInfo;  
extern Fault_analysis_flag gFaultAnalysisFlag;

void FaultAnalysisProcess(void);
#endif /*__MID_SHELL_H */ 
                
