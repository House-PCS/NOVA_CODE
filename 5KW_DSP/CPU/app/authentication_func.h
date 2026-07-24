/*
 * authentication_function.h
 *
 *  Created on: 2024年9月30日
 *      Author: Nova006352
 */

#ifndef APP_AUTHENTICATION_FUNCTION_H_
#define APP_AUTHENTICATION_FUNCTION_H_

#include "F28x_Project.h"

/* 国家标准 */
typedef enum
{
    EN_AUSTRALIA_A,  //澳洲——A
    EN_AUSTRALIA_B,  //澳洲——B
    EN_AUSTRALIA_C,  //澳洲——C
    EN_NEW_ZEALAND,  //新西兰
    EN_NATIONAL_STANDARDS_NUM,
} EN_NATIONAL_STANDARDS;

/* 电压区间段 */
typedef enum
{
    EN_VOLT_SECTION_1,  //电压区间段1
    EN_VOLT_SECTION_2,  //电压区间段2
    EN_VOLT_SECTION_3,  //电压区间段3
    ENEN_VOLT_SECTION_NUM,
} EN_VOLT_SECTION;

/*升降频区间*/
typedef enum
{
    EN_UNDER_FREQ_SECTION,  //欠频区间
    EN_LOWER_FREQ_SECTION,  //降频区间
    EN_NORMAL_FREQ_SECTION, //正常区间
    EN_UPPER_FREQ_SECTION,  //升频区间
    EN_OVER_FREQ_SECTION,   //过频区间
} EN_FREQUENCY_SECTION;

/*无功模式*/
typedef enum
{
    EN_POWER_FACTOR_MODE,  //功率因数模式
    EN_REATIVE_PWR_MODE,  //无功功率模式
    EN_QV_PWR_MODE, //QV模式
    EN_QP_PWR_MODE,  //QP模式
} EN_REATIVE_PWR_CTRL_MODE;


typedef struct _inv_reactive_coefficient_stru_t
{
    float qv_coefficient_k;
    float qv_coefficient_b;
    float pv_coefficient_k;
    float pv_coefficient_b;
    float pf_coefficient_k;
    float pf_coefficient_b;
} InvReactiveCoefficient_t;
typedef struct _approval_std_info_stru_t
{
    float over_vol_level1;
    float over_vol_level2;
    float under_vol_level1;
    float under_vol_level2;
    float vol_nom_max;
    float over_vol_stop_out_activepwr;
    float under_vol_stop_out_activepwr;
    float recover_over_vol_stop_out_activepwr;
    float recover_under_vol_stop_out_activepwr;
    float over_freq;
    float under_freq;
    float recover_over_freq;
    float recover_under_freq;
    float pf_low_freq;
    float pf_high_freq;
    float pf_stop_under_freq;
    float pf_stop_upper_freq;
    float pf_freq_pwr_max;
    float pf_stop_chg_freq;
    float pf_stop_dsg_freq;
    float pf_freq_hyst;
    float qv_ful_reac_out_Volth;
    float qv_start_out_reac_Volth;
    float qv_start_abs_reac_Volth;
    float qv_ful_reac_abs_Volth;
    float qv_max_prov_Preac;
    float qv_max_abs_Preac;
    float pv_dsg_start_derate_Volth;
    float pv_dsg_min_out_Volth;
    float pv_chg_start_derate_Volth;
    float pv_chg_min_out_Volth;
    float pv_chg_Pmin;
} ApprovalStdInfo_t;

/*认证功率*/
typedef struct inverter_pwr_stru_t
{
    float32 inv_output_act_pwr;
    float32 inv_output_reactive_pwr;
} InverterPwr_t;

extern InverterPwr_t _gInvPwr;
extern EN_NATIONAL_STANDARDS e_national_standards;
extern ApprovalStdInfo_t gApprovalStdInfo;
void ApprovalStdInfoGet(void);
void ReactivepwrMode(void);
//void PowerCtrlModeHandle(EN_NATIONAL_STANDARDS e_national_standards,float inv_active_pwr,float avtive_pwr,float reavtive_pwr);
void PowerCtrlModeHandle(EN_NATIONAL_STANDARDS e_national_standards,float inv_active_pwr,InverterPwr_t* InvPwr);
#endif /* APP_AUTHENTICATION_FUNCTION_H_ */
