
#ifndef __FAULT_ANALYSIS_H
#define __FAULT_ANALYSIS_H

#include "debug.h"
#include "modbus_analysis.h"
#include "rtc.h"
#include "dev_24c32.h"
#include "sensor_control.h"
#include "dev_system_power.h"

typedef struct _Fault_analysis_flag_t
{
    uint8_t    cell_under_votage1_count             ;
    uint8_t    cell_under_votage2_count             ;
    uint8_t    cell_under_votage3_count             ;
    uint8_t    cell_over_votage1_count              ;
    uint8_t    cell_over_votage2_count              ;
    uint8_t    cell_over_votage3_count              ;
    uint16_t   dsg_over_current1_count              ;
    uint16_t   dsg_over_current2_count              ;
    uint16_t   dsg_over_current3_count              ;
    uint16_t   chg_over_current1_count              ;
    uint16_t   chg_over_current2_count              ;
    uint16_t   chg_over_current3_count              ;
    uint8_t    cell_temperature_cmp_flag            ;
    uint8_t    cell_temperature_ot1_count           ;
    uint8_t    cell_temperature_ot2_count           ;
    uint8_t    cell_temperature_ot3_count           ;
    uint8_t    cell_temperature_ut1_count           ;
    uint8_t    cell_temperature_ut2_count           ;
    uint8_t    cell_temperature_ut3_count           ;    
    uint8_t    cell_temperature_ltd_count           ;
    uint8_t    premos_enable_flag                   ;
    uint8_t    pmos_count                           ;
    uint8_t    chargemos_enable_flag                ;
    uint8_t    dischargemos_enable_flag             ;
    uint8_t    prechargemos_enable_flag             ;
    uint8_t    pack_under_votage_count              ;
    uint8_t    pack_over_votage_count               ;
    uint8_t    cell_voltage_diff_count              ;
    uint8_t    soh_under_lvl1_count                 ;
    uint8_t    soh_under_lvl2_count                 ;
    uint8_t    soh_under_lvl3_count                 ;    
    uint8_t    cell_votage_sensor_error_count       ;
    uint8_t    cmos_temperature_sensor_error_count  ;
    uint8_t    dmos_temperature_sensor_error_count  ;
    uint8_t    cell_temperature_sensor_error_count  ;
    uint8_t    cell_temperature_sensor_error_count1 ;
    uint8_t    mos_fail_close_count                 ;
    uint8_t    precharge_curr_revers_count          ;
    uint8_t    soc_under_error_count                ;
    uint8_t    zerodrift_error_count                ;
    uint8_t    balance_sensor_error_count           ;
    uint8_t    flash_error_flag                     ;
    uint8_t    e2prom_error_flag                    ;
    uint8_t    mos_adhesion_count                   ;
    uint8_t    dsg_short_count                      ;
    uint8_t    chg_short_count                      ;
    uint8_t    heat_error_count                     ;
} Fault_analysis_flag;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t mos_adhesion           : 1;          /*  三级故障 放电或预充或充电MOS粘连故障  */ 
        uint16_t mos_fail_close         : 1;          /*  三级故障 预充MOS无法闭合故障、充电MOS无法闭合故障、放电MOS无法闭合故障（mos闭合故障）  */
        uint16_t reserved               : 14;          /* 预留               */
    } b;

} BMSMosErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t discharge_overcurr1 : 1;          /* 放电过流1级 一级故障*/
        uint16_t discharge_overcurr2 : 1;          /* 放电过流2级 二级故障*/
        uint16_t discharge_overcurr3 : 1;          /* 放电过流3级 三级故障*/
        uint16_t charge_overcurr1    : 1;          /* 充电过流1级 一级故障*/
        uint16_t charge_overcurr2    : 1;          /* 充电过流2级 二级故障*/
        uint16_t charge_overcurr3    : 1;          /* 充电过流3级 三级故障*/
        uint16_t reserved            : 10;         /* 预留        */
    } b;

} BMSCurrErrorCode_t;

typedef union
{
    uint32_t value;
    struct
    {
        uint16_t cell_under_vol1     : 1;           /* 单体欠压1级 一级故障*/
        uint16_t cell_under_vol2     : 1;           /* 单体欠压2级 二级故障*/
        uint16_t cell_under_vol3     : 1;           /* 单体欠压3级 三级故障    */
        uint16_t cell_over_vol1      : 1;           /* 单体过压1级 一级故障 */
        uint16_t cell_over_vol2      : 1;           /* 单体过压2级 二级故障 */
        uint16_t cell_over_vol3      : 1;          /* 单体过压3级  三级故障*/
        uint16_t pack_over_vol       : 1;          /* 电池包总电压过压 三级故障*/
        uint16_t pack_under_vol      : 1;          /* 电池包总电压欠压 三级故障*/
        uint16_t pack_high_temper1   : 1;          /* 电池温度过高一级报警 一级故障*/
        uint16_t pack_high_temper2   : 1;          /* 电池温度过高二级报警 二级故障*/
        uint16_t pack_high_temper3   : 1;          /* 电池温度过高三级报警 三级故障*/
        uint16_t pack_low_temper1    : 1;          /* 电池温度过低一级故障 */
        uint16_t pack_low_temper2    : 1;          /* 电池温度过低二级故障 */
        uint16_t pack_low_temper3    : 1;          /* 电池温度过低三级故障 */        
        uint16_t pack_temper_differ  : 1;          /* 电池温差过大 三级故障*/
        uint16_t cell_voltage_diff   : 1;          /* 电芯不均衡 三级故障*/
        uint16_t pack_low_soh1       : 1;          /* SOH过低一级 一级故障*/
        uint16_t pack_low_soh2       : 1;          /* SOH过低二级 二级故障       */
        uint16_t pack_low_soh3       : 1;          /* SOH过低三级 三级故障       */        
    } b;

} BMSCell1ErrorCode_t;


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

} BMSCellSampleErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t balance_loop_error             : 1;           /* 均衡回路故障            */
        uint16_t spi_flash_error                : 1;           /* spi fash故障            */
        uint16_t e2prom_error                   : 1;           /* e2prom故障              */
        uint16_t rtc_error                      : 1;           /* rtc故障                 */
        uint16_t heat_error                     : 1;           /* 加热膜故障              */
        uint16_t reserved                       : 11;           /* 预留                   */
    } b;

} BMSOther1ErrorCode_t;

typedef struct _bms_error_num_code_t
{
    uint16_t    cell_under_vol1_error_num      ;                    /* 单体欠压1级 */
    uint16_t    cell_under_vol2_error_num      ;                    /* 单体欠压2级 */
    uint16_t    cell_under_vol3_error_num      ;                    /* 单体欠压3级     */
    uint16_t    cell_over_vol1_error_num       ;                    /* 单体过压1级 */
    uint16_t    cell_over_vol2_error_num       ;                    /* 单体过压2级 */
    uint16_t    cell_over_vol3_error_num       ;                    /* 单体过压3级 */
    uint16_t    pack_high_temper1_error_num    ;                    /* 电池温度过高一级报警 */
    uint16_t    pack_high_temper2_error_num    ;                    /* 电池温度过高二级报警 */
    uint16_t    pack_high_temper3_error_num    ;                    /* 电池温度过高三级报警 */
    uint16_t    pack_low_temper_error_num      ;                    /* 电池温度过低故障 */
    uint16_t    discharge_overcurr1_error_num  ;                    /* 放电过流1级 */
    uint16_t    discharge_overcurr2_error_num  ;                    /* 放电过流2级 */
    uint16_t    discharge_overcurr3_error_num  ;                    /* 放电过流3级 */
    uint16_t    charge_overcurr1_error_num     ;                    /* 充电过流1级 */
    uint16_t    charge_overcurr2_error_num     ;                    /* 充电过流2级 */
    uint16_t    charge_overcurr3_error_num     ;                    /* 充电过流3级 */
    uint16_t    pack_over_votage_error_num     ;                    /* 总压过压 */
    uint16_t    pack_under_votage_error_num    ;                    /* 总压欠压 */
    uint16_t    pack_temper_differ_error_num   ;                    /* 温差过大 */

} BMSErrorNumCode_t;

typedef struct _bms_com_error_info_t
{
    BMSMosErrorCode_t           mos_error_code;
    BMSCurrErrorCode_t          curr_error_code;
    BMSCell1ErrorCode_t         cell1_error_code;
    BMSCellSampleErrorCode_t    cellsample_error_code;
    BMSOther1ErrorCode_t        other1_error_code;
} BMSErrorInfo_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t mos_adhesion                   : 1;        /* 放电或预充或充电（MOS粘连故障 ） */
        uint16_t mos_fail_close                 : 1;        /* 预充MOS无法闭合故障  、充电MOS无法闭合故障、放电MOS无法闭合故障（mos闭合故障）*/
        uint16_t precharge_fail                 : 1;        /* 预充电流反向、预充时间过长、预充短路（预充失败）  */
        uint16_t charge_short_circuit           : 1;        /* 充电短路 */
        uint16_t discharge_short_circuit        : 1;        /* 放电短路 */
    } b;

} PackMosErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t mos_ntc_error              : 1;          /* MOS NTC传感器故障  */
        uint16_t cell_ntc_error             : 1;          /* 电池NTC传感器故障       */
        uint16_t cell_ntc_fatal_error       : 1;          /* 电池温度传感器故障(严重) */
        uint16_t mos_high_temper3           : 1;          /* 充电MOS过温3级/放电MOS过温3级  */
        uint16_t mos_high_temper2           : 1;          /* 充电MOS过温3级/放电MOS过温2级  */        
        uint16_t cell_high_temper3          : 1;          /* 电池温度过高三级故障 */
        uint16_t cell_high_temper2          : 1;          /* 电池温度过高二级故障 */        
        uint16_t cell_low_temper3           : 1;          /* 电池温度过低三级故障 */
        uint16_t cell_low_temper2           : 1;          /* 电池温度过低二级故障  */
        uint16_t cellpost_low_temper3       : 1;          /* 电池极柱温度过高三级故障 */
        uint16_t cellpost_low_temper2       : 1;          /* 电池极柱温度过高二级故障  */       
        uint16_t cell_temper_differ         : 1;          /* 电池温差过大 */
    } b;

} PackTempErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t current_sensor_error               : 1;          /* 电流传感器故障 */		
        uint16_t discharge_overcurr3                : 1;          /* 放电过流3级 */
        uint16_t discharge_overcurr2                : 1;          /* 放电过流2级 */        
        uint16_t charge_overcurr3                   : 1;          /* 充电过流3级 */
        uint16_t charge_overcurr2                   : 1;          /* 充电过流2级 */        
    } b;

} PackCurrErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t vol_sensor_error           : 1;           /* 电压传感器故障*/
        uint16_t cell_over_vol3             : 1;           /* 单体过压3级 */
        uint16_t cell_over_vol2             : 1;           /* 单体过压2级 */        
        uint16_t cell_under_vol3            : 1;           /* 单体欠压3级     */
        uint16_t cell_under_vol2            : 1;           /* 单体欠压2级     */        
        uint16_t pack_over_vol3             : 1;           /* 电池包总电压过压3级 */
        uint16_t pack_over_vol2             : 1;           /* 电池包总电压过压2级 */        
        uint16_t pack_under_vol3            : 1;           /* 电池包总电压欠压3级 */
        uint16_t pack_under_vol2            : 1;           /* 电池包总电压欠压2级 */        
        uint16_t cell_voltage_diff          : 1;           /* 电芯压差过大 */
        uint16_t cell_vol_sample_miss       : 1;           /* 电芯电压采样线掉线*/
    } b;    

} PackCell1ErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t pack_low_soh3       : 1;           /* SOH过低3级*/
        uint16_t pack_low_soh2       : 1;           /* SOH过低2级*/
    } b;

} PackSOXErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t spi_flash_error                : 1;           /* spi fash故障            */
        uint16_t e2prom_error                   : 1;           /* e2prom故障              */
        uint16_t afe_comm__error                : 1;           /* e2prom故障              */
        uint16_t rtc_error                      : 1;           /* rtc故障                 */
        uint16_t balance_error                  : 1;           /* rtc故障                 */
        uint16_t heat_error                     : 1;           /* 加热膜故障               */        
    } b;

} PackOtherErrorCode_t;

typedef union
{
    uint16_t value;
    struct
    {

    } b;

} PackMosAlarmCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t cell_high_temper1                  : 1;            /* 电池温度过高1级报警 */
        uint16_t cell_low_temper1                   : 1;            /* 电池温度过低1级故障  */
        uint16_t cellpost_low_temper1               : 1;            /* 电池极柱温度过高1级故障  */       
        uint16_t cell_temper_differ_waring          : 1;            /* 电池温差过大一级 */ 
        uint16_t mos_high_temper1                   : 1;            /* 充电MOS、放电mos过温1级  */  
    } b;

} PackTempAlarmCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t discharge_overcurr1 : 1;          /* 放电过流1级 */
        uint16_t charge_overcurr1    : 1;          /* 充电过流1级 */
    } b;

} PackCurrAlarmCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t cell_over_vol1             : 1;           /* 单体过压1级 */
        uint16_t cell_under_vol1            : 1;           /* 单体欠压1级 */ 
        uint16_t pack_over_vol1             : 1;           /* 电池包总电压过压1级 */
        uint16_t pack_under_vol1            : 1;           /* 电池包总电压欠压1级 */       
        uint16_t cell_voltage_diff          : 1;           /* 电芯压差过大 */        
    } b;

} PackCellAlarmCode_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t pack_low_soh1       : 1;           /* SOH过低1级*/

    } b;

} PackSOXAlarmCode_t;

typedef union
{
    uint16_t value;
    struct
    {

    } b;

} PackOtherAlarmCode_t;



//通知
typedef union
{
    uint16_t value;
    struct
    {

    } b;

} PackMosNoticeCode_t;

typedef union
{
    uint16_t value;
    struct
    {

    } b;

} PackTempNoticeCode_t;

typedef union
{
    uint16_t value;
    struct
    {

    } b;

} PackCurrNoticeCode_t;

typedef union
{
    uint16_t value;
    struct
    {
      
    } b;

} PackCellNoticeCode_t;

typedef union
{
    uint16_t value;
    struct
    {

    } b;

} PackSOXNoticeCode_t;

typedef union
{
    uint16_t value;
    struct
    {

    } b;

} PackOtherNoticeCode_t;

typedef struct _bms_error_info_t
{
    PackMosErrorCode_t              mos_error_code;
    PackTempErrorCode_t             temp_error_code;
    PackCurrErrorCode_t             curr_error_code;
    PackCell1ErrorCode_t            cell1_error_code;
    PackSOXErrorCode_t              sox_error_code;
    PackOtherErrorCode_t            other_error_code;

    PackMosAlarmCode_t              mos_alarm_code;
    PackTempAlarmCode_t             temp_alarm_code;
    PackCurrAlarmCode_t             curr_alarm_code;
    PackCellAlarmCode_t             cell1_alarm_code;
    PackSOXAlarmCode_t              sox_alarm_code;
    PackOtherAlarmCode_t            other_alarm_code;

    PackMosNoticeCode_t             mos_notice_code;
    PackTempNoticeCode_t            temp_notice_code;
    PackCurrNoticeCode_t            curr_notice_code;
    PackCellNoticeCode_t            cell1_notice_code;
    PackSOXNoticeCode_t             sox_notice_code;
    PackOtherNoticeCode_t           other_notice_code;

} PackErrorInfo_t;


typedef struct _pack_error_stru_t
{
    uint16_t     mos_history_error_code;
    uint16_t     curr_history_error_code;
    uint16_t     cell1_history_error_code;
    uint16_t     cell2_history_error_code;
    uint16_t     pre_history_error_code;
    uint16_t     cellsample_history_error_code;
    uint16_t     other_history_error_code;
    uint16_t     rtc_high_history_error_code;
    uint16_t     rtc_low_history_error_code;
} HistoryErrorStatus_t;

#pragma pack(1)

typedef struct _bms_warning_param_t
{
    int32_t  discharge_overcurr1     ;
    int32_t  discharge_overcurr2     ;
    int32_t  discharge_overcurr3     ;
    int32_t  charge_overcurr1        ;
    int32_t  charge_overcurr2        ;
    int32_t  charge_overcurr3        ;
    uint32_t cell_uv1                ;
    uint32_t cell_uv2                ;
    uint32_t cell_uv3                ;
    uint32_t cell_ov1                ;
    uint32_t cell_ov2                ;
    uint32_t cell_ov3                ;
    uint32_t pack_ov                 ;
    uint32_t pack_uv                 ;
    int32_t  cmos_ot1                ;
    int32_t  cmos_ot2                ;
    int32_t  cmos_ot3                ;

    int32_t  cell_ot1                ;
    int32_t  cell_ot2                ;
    int32_t  cell_ot3                ;
    int16_t  cell_ut1                ;
    int16_t  cell_ut2                ;
    int16_t  cell_ut3                ;    
    int32_t  cell_temp_diff          ;
    int32_t  cell_voltage_diff       ;
    uint32_t soh_under_lvl1          ;
    uint32_t soh_under_lvl2          ;
    uint32_t soh_under_lvl3          ;   
    int32_t  tsensor_ut              ;
    int32_t  tsensor_ot              ;
    int32_t  zerodrift_current       ;
} BMSWarningParam_t;

extern BMSErrorInfo_t   gBmsErrorInfo;
extern PackErrorInfo_t  gPackErrorInfo;
extern HistoryErrorStatus_t gHistoryErrorStatus;
extern Fault_analysis_flag gFaultAnalysisFlag;
extern BMSErrorNumCode_t gBMSErrorNumCode;
void FaultAnalysisProcess(void);
void ErrorClear(void);
void HistoryErrorClear(void);
#endif

