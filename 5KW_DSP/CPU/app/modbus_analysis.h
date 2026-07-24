/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __MODBUS_ANALYSIS_H_
#define __MODBUS_ANALYSIS_H_

#include "F28x_Project.h"
#include "mid_comservice.h"
#include "sci.h"
#include "comm_analysis.h"
#include "voltagesourceinvlcfltr.h"
#include "dev_sensor.h"

#define MODBUS_READ_REG_START                   (0x6000)
#define MODBUS_REG_START                        (0x61E2)
#define MODBUS_REG_END                          (0x625F)

#define MODBUS_REG_TEST_START                   (0x8000)
#define MODBUS_REG_TEST_END                     (0x8002)

#define MODBUS_READ_REG_TEST_START              (0x8010)
#define MODBUS_READ_REG_TEST_END                (0x802C)

#define MODBUS_REG_CSIP_START                   (0x6A00)
#define MODBUS_REG_CSIP_END                     (0x6A1F)


extern uint8_t gOpenInv;

typedef enum
{
    EN_MODBUS_RTU,
    EN_MODBUS_TCP
} EN_MODBUS_TYPE;

typedef enum
{
    EN_MODBUS_REV_STATUS_IDLE,
    EN_MODBUS_REV_STATUS_ANALYSIS,
    EN_MODBUS_REV_STATUS_ACK,
    EN_MODBUS_REV_STATUS_ACK_SEND_WAIT,
    EN_MODBUS_REV_STATUS_ACK_WAIT,
    EN_MODBUS_READ_MPPT,
    EN_COM_REV_STATUS_ACK_WAIT,
    EN_COM_SEND,
} EN_MODBUS_REV_STATUS;



typedef enum
{
    EN_MODBUS_FUNC_CODE_READ_INPUT = 0x04,
    EN_MODBUS_FUNC_CODE_READ_REMAIN = 0x03,
    EN_MODBUS_FUNC_CODE_WRITE_SINGLE = 0x06,
    EN_MODBUS_FUNC_CODE_WRITE_MULTI = 0x10
} EN_MODBUS_FUNC_CODE;

typedef enum
{
    EN_MODBUS_ERROR_CODE_INVALID_CMD = 0x01,
    EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR = 0x02,
    EN_MODBUS_ERROR_CODE_INVALID_DATA = 0x03,
} EN_MODBUS_ERROR_CODE;

typedef enum
{
    EN_EXP_NORMAL,
    EN_CHARGING_SLAVE_FIND,
    EN_CHARGING_SWITCH,
    EN_DISCHARGING_SLAVE_FIND,
    EN_DISCHARGING_SWITCH,
    EN_SWITCH_READY_GET,
    EN_SWITCH_READY_TIMEOUT,
    EN_SWITCH_END,

} EN_EXP_SWITCH_STATUS;

typedef struct _modbus_read_stru_t
{
    uint8_t        dev_addr;
    uint8_t        func_code;
    uint8_t        byte_num;
    uint16_t       *pData;/*包含校验，实际有效长度由byte_num决定*/
} ModbusReadRemainAndInput_t;

typedef struct _modbus_read_reg_stru_t    //小端数据
{
    uint16_t model_id                       ;
    uint16_t sn[10]                         ;
    uint16_t hardware_version               ;
    uint16_t software_version               ;
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
    uint16_t inverter_radiator_temperature  ;   //逆变散热器温度
    uint16_t chassis_internal_temperature   ;   //机箱内部温度
    uint16_t reserved2[5]                   ;
    uint16_t battery_voltage                ;   //逆变器电池侧电压：0.1V
    int16_t  battery_current                ;   //逆变器电池侧电流：0.1A
    int16_t  battery_power                  ;   //逆变器电池侧功率:1w
    int16_t  battery_active_power           ;   //逆变器电池侧有功功率:1w
    int16_t  battery_reactive_power         ;   //逆变器电池侧无功功率
    uint16_t reserved3                      ;
    uint16_t r_phase_grid_voltage           ;
    uint16_t r_phase_grid_frequency         ;
    uint16_t r_phase_meter_current          ;
    int16_t  r_phase_meter_apparentpower    ;
    int16_t  r_phase_meter_activepower      ;
    int16_t  r_phase_meter_reactivepower    ;
    uint16_t s_phase_grid_voltage           ;
    uint16_t s_phase_grid_frequency         ;
    uint16_t s_phase_meter_current          ;
    int16_t  s_phase_meter_apparentpower    ;
    int16_t  s_phase_meter_activepower      ;
    int16_t  s_phase_meter_reactivepower    ;
    uint16_t t_phase_grid_voltage           ;
    uint16_t t_phase_grid_frequency         ;
    uint16_t t_phase_meter_current          ;
    int16_t  t_phase_meter_apparentpower    ;
    int16_t  t_phase_meter_activepower      ;
    int16_t  t_phase_meter_reactivepower    ;
    uint16_t phase_sequence                 ;   //相序：指交流电系统中三相电压或电流的相对顺序。    0：正序  1：负序
    uint16_t oil_current                    ;
    uint16_t oil_voltage                    ;
    uint16_t oil_frequency                  ;
    uint16_t oil_power                      ;
    uint16_t r_phase_inverter_input_current ;
    uint16_t s_phase_inverter_input_current ;
    uint16_t t_phase_inverter_input_current ;

    uint16_t reserved4[4]                   ;
    uint16_t r_phase_inverter_voltage       ;
    uint16_t r_phase_inverter_frequency     ;
    uint16_t r_phase_inverter_current       ;
    int16_t  r_phase_inverter_apparentpower ;
    int16_t  r_phase_inverter_activepower   ;
    int16_t  r_phase_inverter_reactivepower ;
    uint16_t s_phase_inverter_voltage       ;
    uint16_t s_phase_inverter_frequency     ;
    uint16_t s_phase_inverter_current       ;
    int16_t  s_phase_inverter_apparentpower ;
    int16_t  s_phase_inverter_activepower   ;
    int16_t  s_phase_inverter_reactivepower ;
    uint16_t t_phase_inverter_voltage       ;
    uint16_t t_phase_inverter_frequency     ;
    uint16_t t_phase_inverter_current       ;
    int16_t  t_phase_inverter_apparentpower ;
    int16_t  t_phase_inverter_activepower   ;
    int16_t  t_phase_inverter_reactivepower ;
    uint16_t reserved5[3]                   ;//协议中仍有单独逆变信息
    uint16_t r_phase_backup_voltage         ;
    uint16_t r_phase_backup_current         ;
    int16_t  r_phase_backup_apparentpower   ;
    int16_t  r_phase_backup_activepower     ;
    int16_t  r_phase_backup_reactivepower   ;
    uint16_t s_phase_backup_voltage         ;
    uint16_t s_phase_backup_current         ;
    int16_t  s_phase_backup_apparentpower   ;
    int16_t  s_phase_backup_activepower     ;
    int16_t  s_phase_backup_reactivepower   ;
    uint16_t t_phase_backup_voltage         ;
    uint16_t t_phase_backup_current         ;
    int16_t  t_phase_backup_apparentpower   ;
    int16_t  t_phase_backup_activepower     ;
    int16_t  t_phase_backup_reactivepower   ;

    int16_t  inductor_overcurrent_01;           // 20260414新增，逆变电感过流电流，对应寄存器0x6068
    int16_t  inductor_overcurrent_02;           // 20260414新增，逆变电感过流电流，对应寄存器0x6069
    int16_t  inductor_overcurrent_03;           // 20260414新增，逆变电感过流电流，对应寄存器0x606A
    //uint16_t reserved6[3]                   ;

    int16_t  r_phase_load_apparentpower     ;
    int16_t  r_phase_load_activepower       ;
    int16_t  r_phase_load_reactivepower     ;
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
    uint16_t pv1_input_current              ;
    uint16_t pv2_input_current              ;
    uint16_t pv3_input_current              ;
    uint16_t pv4_input_current              ;
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
} ModbusReadReg_t;


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

} UpgradeState_t;

typedef struct _modbus_reg_stru_t           //小端数据
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
    uint16_t    network_power_limit                   ;//入户功率限制（默认15kW）
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
    uint16_t    off_grid_sensitivity                  ; // 20260415，原drm_enable寄存器内容改为off_grid_sensitivity，用于选择0ms脱网的响应时间
    uint16_t    antireverseflow_enable                ;
    uint16_t    program_control_enable                ;
    uint16_t    ov_derating_enable                    ;
    uint16_t    of_derating_enable                    ;
    uint16_t    uv_derating_enable                    ;
    uint16_t    uf_derating_enable                    ;
    uint16_t    island_enable                         ;
    uint16_t    ac_coupling_enable                    ;

    uint16_t    hl_crossover_enable                   ;
    uint16_t    pcs_dsg_ctrl                          ;//监控卡下发给DSP的允许放电使能信号
    uint16_t    pcs_chg_ctrl                          ;//监控卡下发给DSP的允许充电使能信号
    uint16_t    user_type                             ;
    uint16_t    ignore_ct_err                         ; //ignore_ct_err
    uint16_t    ct_or_meter                           ;
    uint16_t    meter_brand                           ;
    uint16_t    ct_direction                          ;
    uint16_t    antireverseflow_threshold             ;
    uint16_t    remote_standby_cmd                    ; // 20260514, 新增待机寄存器，原remote_shutdown_status改为remote_standby_cmd

    int16_t     reactive_power_setting_value          ;
    uint16_t    reactive_power_control_method         ;
    int16_t     power_factor_setting_value            ;
    int16_t     active_power_setting_value            ;
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
    uint16_t    ongrid_power                          ; // 20260519，补充备注：操作界面设置的CT端最大并网卖电功率，恒定正数，例：4000表示设定的从CT端流向电网的最大卖电功率为4000W
    uint16_t    reactive_power_response_time          ;
    uint16_t    work_mode                             ;
    uint16_t    reset_factory                         ;
    UpgradeState_t    sysupgradestatus                      ;//升级状态
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
} ModbusReg_t;

typedef struct _modbus_reg_test_stru_t    //小端数据
{
    uint16_t    enter_test_mode                       ;
    uint16_t    pwm_relay_out_ctrl                    ;
    uint16_t    io_out_ctrl                           ;
} ModbusRegTest_t;

typedef struct _modbus_read_reg_test_stru_t           //小端数据
{
    uint16_t    io_input_status                       ;
    uint16_t    flash_test_result                     ;
    uint16_t    adc_bus_voltage                       ;
    uint16_t    adc_bat_voltage                       ;
    uint16_t    adc_load_voltage                      ;
    uint16_t    adc_microgrid_voltage                 ;
    uint16_t    adc_powergrid_voltage                 ;
    uint16_t    adc_invert_voltage                    ;
    uint16_t    adc_pv2_voltage                       ;
    uint16_t    adc_pv1_voltage                       ;
    uint16_t    adc_pv2_current                       ;
    uint16_t    adc_pv1_current                       ;
    uint16_t    adc_bat_current                       ;
    uint16_t    adc_load_current                      ;
    uint16_t    adc_microgrid_current                 ;
    uint16_t    adc_powergrid_current                 ;
    uint16_t    adc_leak_current                      ;
    uint16_t    adc_inductor_current                  ;
    uint16_t    adc_transformer_current               ;
    uint16_t    adc_dc_component                      ;
    uint16_t    adc_ct_current                        ;
    uint16_t    ref_1_5v                              ;
    uint16_t    temp_a1                               ;
    uint16_t    temp_a2                               ;
    uint16_t    temp_a3                               ;
    uint16_t    temp_a4                               ;
    uint16_t    temp_a5                               ;
    uint16_t    temp_a6                               ;
    uint16_t    temp_a7                               ;

} ModbusReadRegTest_t;

typedef struct _modbus_reg_csip_stru_t  // 20260529，新增CSIP结构体，注意所有下发的带max的最大值都为正数，没有符号，功率方向需要由DSP设定
{
    uint16_t    CSIP_enable                     ;   // 20260529，新增CSIP状态使能，1：使能；其他值：无效
    
    uint16_t    CSIP_import_power_source        ;   // 2026年5月26日 CT口输入功率来源  0：来自队列    1：来自默认值
    uint16_t    CSIP_import_power_ramp_up_type  ;   // 20260529，新增CT口输入功率爬坡类型，0：按爬坡时间，1：爬坡率，0xFFFF表示不限制
    uint16_t    CSIP_import_power_ramp_up_value ;   // 20260529，新增CT口输入功率爬坡参数，如果爬坡类型为0，则表示爬坡时间，定义如下：输入功率爬坡时间(爬坡类型为0有效)，单位：s，0xFFFF表示不限制；
                                                    // 如果爬坡类型为1，则表示爬坡率，定义如下：输入功率爬坡率(爬坡类型为1有效)，表示每秒变化额定功率的百分比，单位0.01%，例如：该值为27，则表示功率按照每秒0.27% * 5kW变化，0xFFFF表示不限制
    uint16_t    CSIP_import_max_power[2]        ;   // 2*uint16 20260529，新增CT口最大入户功率，单位：W，0xFFFFFFFF表示不限制，例真实数据为0x12345678，CSIP_import_max_power[0] = 0x1234，CSIP_import_max_power[1] = 0x5678

    uint16_t    CSIP_export_power_source        ;   // 2026年5月26日 CT口输出功率来源  0：来自队列    1：来自默认值
    uint16_t    CSIP_export_power_ramp_up_type  ;   // 20260529，新增CT口输出功率爬坡类型，0：按爬坡时间，1：爬坡率，0xFFFF表示不限制
    uint16_t    CSIP_export_power_ramp_up_value ;   // 20260529，新增CT口输出功率爬坡参数，如果爬坡类型为0，则表示爬坡时间，定义如下：输出功率爬坡时间(爬坡类型为0有效)，单位：s，0xFFFF表示不限制；
                                                    // 如果爬坡类型为1，则表示爬坡率，定义如下：输出功率爬坡率(爬坡类型为1有效)，表示每秒变化额定功率的百分比，单位0.01%，例如：该值为27，则表示功率按照每秒0.27% * 5kW变化，0xFFFF表示不限制
    uint16_t    CSIP_export_max_power[2]        ;   // 2*uint16 20260529，新增CT口最大卖电功率，单位：W，0xFFFFFFFF表示不限制，例真实数据为0x12345678，CSIP_export_max_power[0] = 0x1234，CSIP_export_max_power[1] = 0x5678

    uint16_t    CSIP_ac_in_power_source         ;   // 2026年5月26日 AC口输入功率来源  0：来自队列    1：来自默认值
    uint16_t    CSIP_ac_in_power_ramp_up_type   ;   // 20260529，新增AC口输入功率爬坡类型，0：按爬坡时间，1：爬坡率，0xFFFF表示不限制
    uint16_t    CSIP_ac_in_power_ramp_up_value  ;   // 20260529，新增AC口输入功率爬坡参数，如果爬坡类型为0，则表示爬坡时间，定义如下：输入功率爬坡时间(爬坡类型为0有效)，单位：s，0xFFFF表示不限制；
                                                    // 如果爬坡类型为1，则表示爬坡率，定义如下：输入功率爬坡率(爬坡类型为1有效)，表示每秒变化额定功率的百分比，单位0.01%，例如：该值为27，则表示功率按照每秒0.27% * 5kW变化，0xFFFF表示不限制
    uint16_t    CSIP_ac_in_max_power[2]         ;   // 2*uint16 20260529，新增AC口最大输入功率，单位：W，0xFFFFFFFF表示不限制，例真实数据为0x12345678，CSIP_ac_in_max_power[0] = 0x1234，CSIP_ac_in_max_power[1] = 0x5678

    uint16_t    CSIP_ac_out_power_source        ;   // 2026年5月26日 AC口输出功率来源  0：来自队列    1：来自默认值
    uint16_t    CSIP_ac_out_power_ramp_up_type  ;   // 20260529，新增AC口输出功率爬坡类型，0：按爬坡时间，1：爬坡率，0xFFFF表示不限制
    uint16_t    CSIP_ac_out_power_ramp_up_value ;   // 20260529，新增AC口输出功率爬坡参数，如果爬坡类型为0，则表示爬坡时间，定义如下：输出功率爬坡时间(爬坡类型为0有效)，单位：s，0xFFFF表示不限制；
                                                    // 如果爬坡类型为1，则表示爬坡率，定义如下：输出功率爬坡率(爬坡类型为1有效)，表示每秒变化额定功率的百分比，单位0.01%，例如：该值为27，则表示功率按照每秒0.27% * 5kW变化，0xFFFF表示不限制
    uint16_t    CSIP_ac_out_max_power[2]        ;   // 2*uint16 20260529，新增AC口最大输出功率，单位：W，0xFFFFFFFF表示不限制，例真实数据为0x12345678，CSIP_ac_out_max_power[0] = 0x1234，CSIP_ac_out_max_power[1] = 0x5678
    uint16_t    reserve[10]                     ;   // 2026年5月26日 预留
    uint16_t    CSIP_run_ctrl                   ;   // 2026年5月26日 CSIP工作状态控制  0：正常运行(并网)    1：切离网   2：静态(并网，AC口禁止输入输出)
} ModbusRegCSIP_t;

extern ModbusReadReg_t      ModbusReadReg;
extern ModbusReg_t          ModbusReg;
extern ModbusReadRegTest_t  ModbusReadRegTest;
extern ModbusRegTest_t      ModbusRegTest;
extern uint8_t Grid_flag;
extern uint8_t remove_open_ctrl;
extern uint8_t Antireverseflow_flag;
extern uint16_t gUpgraderRestartFlag;
void ModbusCmdProcess();
void ModbusRegInit(void);

void ModbusRegSave(void);
void ModbusRegRead(void);
#endif








