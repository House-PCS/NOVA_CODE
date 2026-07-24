/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sys_info.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-7-16
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-7-16    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef SYS_INFO_H_
#define SYS_INFO_H_

#include "dev_flash.h"

#define MOD_HOLD_REG_FLASH_ADDR             (0x200000)
#define MOD_HOLD_REG_VALID_FLASH_ADDR       (0x200FF8)

#define ELECTRIC_INFO_FLASH_ADDR            (0x240000)
#define ELECTRIC_INFO_VALID_FLASH_ADDR      (0x240FF8)
#define BACK_UP_PAGE_NUM                    (20)

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t upgrade_state                : 1;          /*    0:未升级，1：升级中  */
        uint16_t restate_state                : 1;          /*    0:未复位，1：复位  */
        uint16_t mintor_test_restate          : 1;          /*    监控卡测试复位  */
        uint16_t mintor_restate_state         : 1;          /*    0：监控卡未复位，1：监控卡复位中  */
        uint16_t reserve1                     : 1;
        uint16_t reserve2                     : 1;
        uint16_t reserve3                     : 1;
        uint16_t reserve4                     : 1;
        uint16_t reserve5                     : 1;
        uint16_t reserve6                     : 1;
        uint16_t reserve7                     : 1;
        uint16_t reserve8                     : 1;
        uint16_t reserve9                     : 1;
        uint16_t reserve10                    : 1;
        uint16_t reserve11                    : 1;
        uint16_t reserve12                    : 1;
    } b;

} SysUpgradeState_t;


typedef struct _sysinfo_set_stru_t           //小端数据
{
    uint16_t    battery_charging_voltage              ;
    uint16_t    battery_charging_current              ;
    uint16_t    battery_charging_power                ;
    uint16_t    battery_discharging_voltage           ;
    uint16_t    battery_discharging_current           ;
    uint16_t    battery_discharging_power             ;
    uint16_t    x_boost                               ;
    uint16_t    ups_delay0ms                          ;
    int16_t     meter_current                         ;
    int16_t     meter_apparent_power                  ;
    int16_t     meter_active_power                    ;
    int16_t     meter_reactive_power                  ;
    uint16_t    clean_current_cap                     ;
    uint16_t    network_power_limit_enable            ;
    uint16_t    network_power_limit                   ; //入户功率限制（默认15KW)
    uint16_t    parallel_mode_enable                  ; //2025年12月24日新增并机模式使能

    uint16_t    certification_test_switch             ; //2026年3月3日新增并网认证测试开关

    uint16_t    reserve1[3]                           ; //2026年3月3日从4减小到3

    uint16_t    charging_input_typesetting            ;
    uint16_t    ac_output_voltage_range               ;
    uint16_t    ac_output_frequency_range             ;
    uint16_t    grid_standards                        ;
    uint16_t    grids_tandard_segmentation            ;//保留不删
    uint16_t    backup_output_enable                  ;
    uint16_t    battery_discharge_cutoff_vol_point    ;
    uint16_t    battery_charging_vol_protection_point ;
    uint16_t    pv_uv_protection_point                ;
    uint16_t    pv_ov_protection_point                ;
    uint16_t    battery_discharge_power_scheduling    ;
    uint16_t    battery_charging_power_scheduling     ;
    uint16_t    pv_input_mode                         ;
    //uint16_t    drm_enable                            ;
    uint16_t    off_grid_sensitivity                  ;
    uint16_t    antireverseflow_enable                ;
    uint16_t    program_control_enable                ;
    uint16_t    ov_derating_enable                    ;
    uint16_t    of_derating_enable                    ;
    uint16_t    uv_derating_enable                    ;
    uint16_t    uf_derating_enable                    ;
    uint16_t    island_enable                         ;
    uint16_t    ac_coupling_enable                    ;

    uint16_t    hl_crossover_enable                   ;
    uint16_t    pcs_dsg_ctrl                          ;
    uint16_t    pcs_chg_ctrl                          ;
    uint16_t    user_type                             ;
    uint16_t    ignore_ct_err                          ; //ignore_ct_err
    uint16_t    ct_or_meter                           ;
    uint16_t    meter_brand                           ;
    uint16_t    ct_direction                          ;
    uint16_t    antireverseflow_threshold             ;
    uint16_t    remote_standby_cmd                ;
    int16_t    reactive_power_setting_value          ;
    uint16_t    reactive_power_control_method         ;
    int16_t    power_factor_setting_value            ;
    int16_t    active_power_setting_value            ;
    uint16_t    grid_voltage_lower_limit              ;
    uint16_t    grid_voltage_upper_limit              ;
    uint16_t    grid_frequency_lower_limit            ;
    uint16_t    grid_frequency_upper_limit            ;
    uint16_t    bypass_voltage_lower_limit            ;
    uint16_t    bypass_voltage_upper_limit            ;
    uint16_t    bypass_frequency_lower_limit          ;
    uint16_t    bypass_frequency_upper_limit          ;
    uint16_t    ov_derating_point                     ;
    uint16_t    of_derating_point                     ;
    uint16_t    uv_derating_point_charge              ;
    uint16_t    uf_derating_point_charge              ;
    uint16_t    grid_ov_protect_Point                 ;
    uint16_t    inverter_uv_protect_Point             ;
    uint16_t    low_frequency_protect_Point           ;
    uint16_t    over_frequency_protect_Point          ;
    uint16_t    ongrid_reconnect_time                 ;
    uint16_t    ongrid_power                          ;
    uint16_t    reactive_power_response_time          ;
    uint16_t    work_mode                             ;
    uint16_t    reset_factory                         ;
    SysUpgradeState_t sysupgradestatus ;
    uint16_t    r_phase_vol_calibration_coefficient_k ;
    uint16_t    r_phase_vol_calibration_coefficient_b ;
    uint16_t    s_phase_vol_calibration_coefficient_k ;
    uint16_t    s_phase_vol_calibration_coefficient_b ;
    uint16_t    t_phase_vol_calibration_coefficient_k ;
    uint16_t    t_phase_vol_calibration_coefficient_b ;
    uint16_t    r_phase_curr_calibration_coefficient_k;
    uint16_t    r_phase_curr_calibration_coefficient_b;
    uint16_t    s_phase_curr_calibration_coefficient_k;
    uint16_t    s_phase_curr_calibration_coefficient_b;
    uint16_t    t_phase_curr_calibration_coefficient_k;
    uint16_t    t_phase_curr_calibration_coefficient_b;
    uint16_t    pv1_curr_calibrate_k                  ;
    uint16_t    pv1_curr_calibrate_b                  ;
    uint16_t    pv2_curr_calibrate_k                  ;
    uint16_t    pv2_curr_calibrate_b                  ;
    uint16_t    pv3_curr_calibrate_k                  ;
    uint16_t    pv3_curr_calibrate_b                  ;
    uint16_t    pv4_curr_calibrate_k                  ;
    uint16_t    pv4_curr_calibrate_b                  ;
    uint16_t    pv1_vol_calibrate_k                   ;
    uint16_t    pv1_vol_calibrate_b                   ;
    uint16_t    pv2_vol_calibrate_k                   ;
    uint16_t    pv2_vol_calibrate_b                   ;
    uint16_t    pv3_vol_calibrate_k                   ;
    uint16_t    pv3_vol_calibrate_b                   ;
    uint16_t    pv4_vol_calibrate_k                   ;
    uint16_t    pv4_vol_calibrate_b                   ;
    uint16_t    battery_vol_calibrate_k               ;
    uint16_t    battery_vol_calibrate_b               ;
    uint16_t    battery_curr_calibrate_k              ;
    uint16_t    battery_curr_calibrate_b              ;
    uint16_t    pv_switch                             ;
    uint16_t    pf_switch                             ;
    uint16_t    on_grid_power_soft_limit              ;
    uint16_t    on_grid_power_hard_limit              ;
    uint16_t    batt_volt                             ;
    uint16_t    dsp_wake_en                           ;
    uint16_t    pv_max_chg_power                      ;
    uint16_t    max_inv_power                         ;
    uint16_t    dc_source                             ;
    uint16_t    hard_limit_threshold                  ;
    uint16_t    residual_current_protection_threshold ;     //漏电流保护阈值
    uint16_t    residual_current_change_response_time ;     //漏电流响应时间
    uint16_t    firmware_erasure                      ;     //外部Flash固件擦除   xingxiao2026年1月26日    
    uint16_t    reserve4[3]                           ;

} SysInfoSet_t;

typedef struct _sysinfo_get_stru_t    //小端数据
{
    uint16_t inverter_model                 ;
    uint16_t parallel_mode_stat             ;   //2025年12月24日新增并机状态
    uint16_t sys_input_stat                 ;
    uint16_t system_status                  ;
    uint16_t inv_status                     ;
    uint16_t dcdc_status                    ;
    uint16_t mppt_status                    ;
    uint16_t dsp_alarm_code_high            ;
    uint16_t dsp_alarm_code_low             ;
    uint16_t dsp_error_code_high            ;
    uint16_t dsp_error_code_low             ;
    uint16_t bus_voltage                    ;   //母线电压
    int16_t inverter_radiator_temperature  ;   //逆变散热器温度
    int16_t chassis_internal_temperature   ;   //机箱内部温度
    uint16_t reserved2[5]                   ;
    uint16_t battery_voltage                ;   //逆变器电池侧电压：0.1V
    int16_t  battery_current                ;   //逆变器电池侧电流：0.1A
    int16_t  battery_power                  ;   //逆变器电池侧功率:1w
    int16_t  battery_active_power           ;   //逆变器电池侧有功功率:1w
    int16_t  battery_reactive_power         ;   //逆变器电池侧无功功率
    uint16_t reserved3                      ;
    uint16_t r_phase_grid_voltage           ;
    uint16_t r_phase_grid_frequency         ;
    int16_t  r_phase_meter_current          ;
    int16_t  r_phase_meter_apparentpower    ;
    int16_t  r_phase_meter_activepower      ;
    int16_t  r_phase_meter_reactivepower    ;
    uint16_t s_phase_grid_voltage           ;
    uint16_t s_phase_grid_frequency         ;
    int16_t  s_phase_meter_current          ;
    int16_t  s_phase_meter_apparentpower    ;
    int16_t  s_phase_meter_activepower      ;
    int16_t  s_phase_meter_reactivepower    ;
    uint16_t t_phase_grid_voltage           ;
    uint16_t t_phase_grid_frequency         ;
    int16_t  t_phase_meter_current          ;
    int16_t  t_phase_meter_apparentpower    ;
    int16_t  t_phase_meter_activepower      ;
    int16_t  t_phase_meter_reactivepower    ;
    uint16_t phase_sequence                 ;   //相序：指交流电系统中三相电压或电流的相对顺序。    0：正序  1：负序
    int16_t  oil_current                    ;
    uint16_t oil_voltage                    ;
    uint16_t oil_frequency                  ;
    int16_t  oil_power                      ;
    int16_t  r_phase_inverter_input_current ;
    int16_t  s_phase_inverter_input_current ;
    int16_t  t_phase_inverter_input_current ;

    uint16_t reserved4[4]                   ;
    uint16_t r_phase_inverter_voltage       ;
    uint16_t r_phase_inverter_frequency     ;
    int16_t  r_phase_inverter_current       ;
    int16_t  r_phase_inverter_apparentpower ;
    int16_t  r_phase_inverter_activepower   ;
    int16_t  r_phase_inverter_reactivepower ;
    uint16_t s_phase_inverter_voltage       ;
    uint16_t s_phase_inverter_frequency     ;
    int16_t  s_phase_inverter_current       ;
    int16_t  s_phase_inverter_apparentpower ;
    int16_t  s_phase_inverter_activepower   ;
    int16_t  s_phase_inverter_reactivepower ;
    uint16_t t_phase_inverter_voltage       ;
    uint16_t t_phase_inverter_frequency     ;
    int16_t  t_phase_inverter_current       ;
    int16_t  t_phase_inverter_apparentpower ;
    int16_t  t_phase_inverter_activepower   ;
    int16_t  t_phase_inverter_reactivepower ;
    uint16_t reserved5[3]                   ;//协议中仍有单独逆变信息
    uint16_t r_phase_backup_voltage         ;
    int16_t  r_phase_backup_current         ;
    int16_t  r_phase_backup_apparentpower   ;
    int16_t  r_phase_backup_activepower     ;
    int16_t  r_phase_backup_reactivepower   ;
    uint16_t s_phase_backup_voltage         ;
    int16_t  s_phase_backup_current         ;
    int16_t  s_phase_backup_apparentpower   ;
    int16_t  s_phase_backup_activepower     ;
    int16_t  s_phase_backup_reactivepower   ;
    uint16_t t_phase_backup_voltage         ;
    int16_t  t_phase_backup_current         ;
    int16_t  t_phase_backup_apparentpower   ;
    int16_t  t_phase_backup_activepower     ;
    int16_t  t_phase_backup_reactivepower   ;

    int16_t  inductor_overcurrent_01;           // 20260414新增，逆变电感过流电流，对应寄存器0x6068
    int16_t  inductor_overcurrent_02;           // 20260414新增，逆变电感过流电流，对应寄存器0x6069
    int16_t  inductor_overcurrent_03;           // 20260414新增，逆变电感过流电流，对应寄存器0x606A
//    uint16_t reserved6[3]                   ;

    int16_t  r_phase_load_apparentpower     ; //20260226,补充说明：代码中所有的r_phase_load_apparentpower实际上包含了第三方光伏的功率，真实含义是远端负载+第三方光伏的总功率
    int16_t  r_phase_load_activepower       ; //20260226,补充说明：代码中所有的r_phase_load_activepower实际上包含了第三方光伏的功率，真实含义是远端负载+第三方光伏的总功率
    int16_t  r_phase_load_reactivepower     ; //20260226,补充说明：代码中所有的r_phase_load_reactivepower实际上包含了第三方光伏的功率，真实含义是远端负载+第三方光伏的总功率
    int16_t  s_phase_load_apparentpower     ;
    int16_t  s_phase_load_activepower       ;
    int16_t  s_phase_load_reactivepower     ;
    int16_t  t_phase_load_apparentpower     ;
    int16_t  t_phase_load_activepower       ;
    int16_t  t_phase_load_reactivepower     ;
    uint16_t reserved7[8]                   ;

    uint16_t load_electric_cost_total_high  ;
    uint16_t load_electric_cost_total_low   ;
    uint16_t grid_connect_count_high        ;
    uint16_t grid_connect_count_low         ;
    uint16_t reserved8[5]                   ;

    uint16_t electric_purchase_total_high   ;
    uint16_t electric_purchase_total_low    ;
    uint16_t reserved9[5]                   ;
    uint16_t electric_sale_total_high       ;
    uint16_t electric_sale_total_low        ;
    int16_t  r_phase_ac_apparentpower       ;
    int16_t  r_phase_ac_activepower         ;
    int16_t  r_phase_ac_reactivepower       ;
    int16_t  s_phase_ac_apparentpower       ;
    int16_t  s_phase_ac_activepower         ;
    int16_t  s_phase_ac_reactivepower       ;
    int16_t  t_phase_ac_apparentpower       ;
    int16_t  t_phase_ac_activepower         ;
    int16_t  t_phase_ac_reactivepower       ;

    uint16_t battery_discharge_total_high   ;
    uint16_t battery_discharge_total_low    ;
    uint16_t battery_charge_total_high      ;
    uint16_t battery_charge_total_low       ;
    uint16_t remote_load_power_total_high   ;
    uint16_t remote_load_power_total_low    ;

    uint16_t reserved10[2]                  ;

    uint16_t pv1_input_voltage              ;
    uint16_t pv2_input_voltage              ;
    uint16_t pv3_input_voltage              ;
    uint16_t pv4_input_voltage              ;
    uint16_t reserved11                     ;
    int16_t  pv1_input_current              ;
    int16_t  pv2_input_current              ;
    int16_t  pv3_input_current              ;
    int16_t  pv4_input_current              ;
    uint16_t reserved12                     ;
    uint16_t pv1_input_power                ;
    uint16_t pv2_input_power                ;
    uint16_t pv3_input_power                ;
    uint16_t pv4_input_power                ;
    uint16_t reserved13[13]                 ;
    uint16_t pv1_power_generation_total_high;
    uint16_t pv1_power_generation_total_low ;
    uint16_t reserved14[5];
    uint16_t pv2_power_generation_total_high;
    uint16_t pv2_power_generation_total_low ;
    uint16_t reserved15[5]                  ;
    uint16_t pv3_power_generation_total_high;
    uint16_t pv3_power_generation_total_low ;
    uint16_t reserved16[5]                  ;
    uint16_t pv4_power_generation_total_high;
    uint16_t pv4_power_generation_total_low ;
    uint16_t pv_temperature                 ;
    uint16_t on_grid_time                   ;
    uint16_t dsp_wake_up_over               ;
    uint16_t reserved17[270]                ;
} SysInfoGet_t;


typedef enum
{
    EN_INPUT_STANDBY,
    EN_INPUT_SELF_CHECK,
    EN_INPUT_MIX_BRID,
    EN_INPUT_OFF_BRID,
    EN_INPUT_BYPASS,
    EN_INPUT_SHUTDOWN
} EN_SYS_STAT;
typedef enum
{
    EN_INV_STANDBY,
    EN_INPUT_OFF_GRID,
    EN_INPUT_ON_GRID,
    EN_INPUT_OFF_TO_ON_GRID,
    EN_INPUT_ON_TO_OFF_GRID,
} EN_INV_STAT;
typedef enum
{
    EN_DCDC_STANDBY,
    EN_DCDC_BREAKDOWN,
    EN_DCDC_CHG,
    EN_DCDC_DSG,
} EN_DCDC_STAT;
typedef enum
{
    EN_MPPT_STANDBY,
    EN_MPPT_SS,
    EN_MPPT_RUN,
    EN_MPPT_BREAKDOWN,
} EN_MPPT_STAT;


extern SysInfoGet_t gSysInfoGet;
extern SysInfoSet_t gSysSetInfo;
extern float gAntiReFlowValue;

bool ModHoldRegSave(void);
bool ModHoldRegRead(void);
bool ElectricInfoSave(void);
bool ElectricInfoRead(void);
void TimingStorage(void);

void ElectricInfoGet(void);
void SysInfoGet(void);
void SysInfoSet(void);
void DefaultInfoGet(void);

void UsageInfoGet(void);
void AdminInfoGet(void);

void UsageInfoSave(void);
void AdminInfoSave(void);
void ResetFactory();
void ApprovalStdInfoGet(void);
uint16_t ThreadBreakDetect(void);

#endif /* SYSINFO_H_ */
