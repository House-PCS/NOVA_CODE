/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        mppt_process.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-3-19
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-3-19    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __MPPT_PROCESS_H__
#define __MPPT_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dev_sensor.h"
#include "voltagesourceinvlcfltr.h"
#include "algo_info.h"


#pragma CODE_SECTION(MpptInverter,".TI.ramfunc");
#pragma CODE_SECTION(PvSendStateWave,".TI.ramfunc");
#define DUAL_PV_MAX_POWER gMaxPvPwr //双路PV功率最大值
#define SINGLE_PV_MAX_POWER 5000 //单路PV功率最大值
extern uint16_t gPvPowerHoursComputedFlag;

extern uint32_t gPv1PowerTotalHours ;
extern uint32_t gPv2PowerTotalHours ;
extern uint32_t gPurchasePowerTotalHours;
extern uint32_t gSalePowerTotalHours;
extern uint32_t gLoadPowerTotalHours;
extern uint32_t gNorLoadPowerTotalHours;
extern uint32_t gBatChgPowerTotalHours ;
extern uint32_t gBatDsgPowerTotalHours ;
void MpptInverter(void);
void PvSendStateWave();
void MpptSetPv1Pwr(float pwr);
uint16_t Read_Mppt_state();
void PVCntlInit();
void* PvGetStateWave();
void SetSourceMode(uint16_t mode);

extern float gPv1_Voltage_Ave;
extern float gPv1_Current_Ave;
extern float gPv2_Voltage_Ave;
extern float gPv2_Current_Ave;
extern float gBat_Voltage_Ave;
extern float gBat_Current_Ave;
extern float gBus_Voltage_Ave;
extern float gGrid_Power_Ave;
extern float gPvPwrRef;

typedef struct _PV_stru_t 
{
    float32  pv_vol;               //PV电压
    float32  pv_cur;               //PV电流
    float32  pv_vol_old;               //上一次的PV电压
    float32  pv_cur_old;                //上一次的PV电流

    float32  pv_pwr;              //pv的功率
    float32  pv_pwr_old;                //上一次的PV功率
    float32  mppt_duty;                //mppt计算占空比
    float32  duty_eff;                //生效占空比

    float32  mppt_duty_add; //步长增加量
    float32  mppt_duty_subtract;                //步长减小量
    float32  mppt_duty_add_old;                //步长减小量
    float32  mppt_duty_subtract_old;                //步长减小量

    uint16_t mppt_state;                //导纳增量法运行状态位
    uint16_t pv_pwr_state;                //功率运行状态位
    float32  dir;                //爬坡方向
    float32  dV; //VOL变化量

    float32  dP; //P变化量
    float32  dI; //I变化量
    float32  dI_divid_dV; //斜率
    float32  pvI_divid_pvV; //斜率

    float32  pv_pwr_limit_count; //限功率运行计数器                
    float32  pv_pwr_nlimit_count;    //满功率运行计数器            
    float32  pv_pwr_set;                //设定功率
    float32  mppt_duty_high_threshold;                //步长阈值
    float32  mppt_duty_low_threshold;                //步长阈值
    float32  pv_pwr_out;
} PvData_t;

extern PvData_t gstPv1Data;
extern PvData_t gstPv2Data;

typedef enum
{
    EN_PV_STATE_DEF_READY,
    EN_PV_STATE_DEF_ACTIVATIONBOOST,
    EN_PV_STATE_DEF_NORMAL,
} EN_PV_STATE_DEF;
typedef enum
{
    EN_PV_SOURCE, //PV源类型
    EN_DC_SOURCE, //直流源类型
} EN_SOURCE_MODE_DEF;
void PvRunStateSet(uint16_t pvrunstate);
EN_PV_STATE_DEF PvRunStateRead();
#define PV_READY 3
#define PV_NORMAL 2
#define PV_BOOST 1
#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
