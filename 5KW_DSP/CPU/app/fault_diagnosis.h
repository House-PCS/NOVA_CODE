/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        fault_diagnosis.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-5-31
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-5-31    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __FAULT_DIAGNOSUS_H
#define __FAULT_DIAGNOSUS_H

/* Inculdes ------------------------------------------------------------------*/

#include "F28x_Project.h"
#include "cpu_timers.h"


/* Exported typedef -----------------------------------------------------------*/


typedef struct _diagnosis_info_stru_t
{
    uint16_t grid_vol_low_count;
    uint16_t grid_vol_high_count;
    uint16_t bat_vol_low_count;
    uint16_t bat_vol_high_count;
    uint16_t bus_vol_low_count;
    uint16_t bus_vol_high_count;
    uint16_t over_load_count;
    uint16_t inv_overcur_count;
    uint16_t inv_overtempe_count;
    uint16_t leak_current_over_count;

} DiagnosisInfo_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t grid_vol_low           : 1;          /* W00    电网电压低  */
        uint16_t grid_vol_high          : 1;          /* W01    电网电压高  */
        uint16_t grid_freq_low          : 1;          /* W02    电网频率低  */
        uint16_t grid_freq_high         : 1;          /* W03    电网频率高  */
        uint16_t pv_no_connected        : 1;          /* W04    PV未接  */
        uint16_t reserve1               : 1;          /* W05    保留  */
        uint16_t reserve2               : 1;          /* W06    保留  */
        uint16_t bat_vol_low            : 1;          /* W07    电池电压低  */
        uint16_t bat_vol_high           : 1;          /* W08    电池电压高  */
        uint16_t overload               : 1;          /* W09    过载报警  */
        uint16_t leak_curr_high         : 1;          /* W10    漏电流高  */
        uint16_t reserve3               : 1;          /* W11    保留  */
        uint16_t fan_error              : 1;          /* W12    风扇故障  */
        uint16_t r_ct_reverse           : 1;          /* W13    R_CT接反  */
        uint16_t s_ct_reverse           : 1;          /* W14    S_CT接反 */
        uint16_t t_ct_reverse           : 1;          /* W15    T_CT接反 */
    } b;

} DspWorningCode_t;

typedef union
{
    uint32_t value;
    struct
    {
        uint32_t start_timeout          : 1;          /* F00    软启超时  */
        uint32_t inv_out_short          : 1;          /* F01    逆变器输出短路  */
        uint32_t battery_undervoltage   : 1;          /* F02    电池欠压2.6*16 = 41.6V  */
        uint32_t reserved               : 1;          /* F03      保留*/
        uint32_t bus_vol_low            : 1;          /* F04    母线电压低  */
        uint32_t bus_vol_high           : 1;          /* F05    母线电压高  */
        uint32_t bus_short              : 1;          /* F06    母线短路  */
        uint32_t reserved1              : 1;          /* F07    保留*/
        uint32_t reserved2              : 1;          /* F07    保留*/
        uint32_t hardware_failure       : 1;          /* F09    硬件故障  */
        uint32_t inv_over_curr          : 1;          /* F10    逆变过流  */
        uint32_t dc_component_high      : 1;          /* F11    直流分量高  */
        uint32_t box_over_temp          : 1;          /* F12    箱体内环境过温85℃  */
        uint32_t radiator_over_temp     : 1;          /* F13    散热器过温85℃  */
        uint32_t reserved3              : 1;          /* F14    保留 */
        uint32_t bat_dsg_over_curr      : 1;          /* F15    电池放电过流120A */
        uint32_t bat_chg_over_curr      : 1;          /* F16    电池充电过流120A */
        uint32_t reserved4              : 1;          /* F17    保留 */
        uint32_t inv_out_error          : 1;          /* F18    逆变输出异常 */
        uint32_t reserved5              : 1;          /* F19    保留 */
        uint32_t always_over_load1      : 1;          /* F20    过载1（>5.5KW）20s */
        uint32_t always_over_load2      : 1;          /*        过载2（>6KW）     */
        uint32_t reserved6              : 1;          /* F19    保留 */
        uint32_t always_comm_error      : 1;          /* F31    主从通讯错误 */

    } b;

} DspErrorCode_t;


typedef struct _fault_info_stru_t
{
    DspWorningCode_t        worning_code;
    DspErrorCode_t          error_code;
} FaultInfo_t;

extern FaultInfo_t gstFaultInfo;
typedef struct _fault_param_stru_t
{
    int32_t  limit_charge_current    ;
} FaultParam_t;

extern FaultInfo_t gstFaultInfo;
extern DiagnosisInfo_t gstDiagnosisInfo;
extern uint16_t gComFail;
extern uint16_t gDeiceComFlag;

extern uint16_t arc_fault_flag;
void WarnningDiagnosis(void);
void ErrorDiagnosis(void);
void FaultDiagnosisProcess(void);
void ErrorHandling(void);
void LeakCurrCheck(void);
void ArcInsFaultDetect(void);

#endif /*__MID_SHELL_H */

