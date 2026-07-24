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

#define MODBUS_READ_REG_START                   (0x6000)
#define MODBUS_REG_START                      	(0x61E2)
#define MODBUS_REG_END                      	(0x625F)


typedef enum
{
    EN_MODBUS_RTU,
    EN_MODBUS_TCP
}EN_MODBUS_TYPE;

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
}EN_MODBUS_REV_STATUS;



typedef enum
{
    EN_MODBUS_FUNC_CODE_READ_INPUT = 0x04,
    EN_MODBUS_FUNC_CODE_READ_REMAIN = 0x03,
    EN_MODBUS_FUNC_CODE_WRITE_SINGLE = 0x06,
    EN_MODBUS_FUNC_CODE_WRITE_MULTI = 0x10
}EN_MODBUS_FUNC_CODE;

typedef enum
{
    EN_MODBUS_ERROR_CODE_INVALID_CMD = 0x01,
    EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR = 0x02,
    EN_MODBUS_ERROR_CODE_INVALID_DATA = 0x03,
}EN_MODBUS_ERROR_CODE;

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
    
}EN_EXP_SWITCH_STATUS;

typedef struct _modbus_read_stru_t
{
    uint8_t        dev_addr;
    uint8_t        func_code;
    uint8_t        byte_num;
    uint16_t       *pData;/*包含校验，实际有效长度由byte_num决定*/
}ModbusReadRemainAndInput_t;

typedef struct _modbus_read_reg_stru_t    //小端数据
{
	uint16_t model_id                       ;
    uint16_t sn[10]                         ;
    uint16_t hardware_version               ;
    uint16_t software_version               ;
    uint16_t inverter_model					;
    uint16_t reserved1                      ;
    uint16_t sys_input_stat                 ;
    uint16_t system_status                  ;
    uint16_t inv_status               		;
    uint16_t dcdc_status               		;
    uint16_t mppt_status           			;
    uint16_t dsp_alarm_code_high            ;
    uint16_t dsp_alarm_code_low             ;
    uint16_t dsp_error_code_high            ;
    uint16_t dsp_error_code_low             ;
    uint16_t bus_voltage               		;	//母线电压
    uint16_t inverter_radiator_temperature  ;	//逆变散热器温度
    uint16_t chassis_internal_temperature   ;	//机箱内部温度
    uint16_t reserved2[5]                   ;
    uint16_t battery_voltage                ;	//逆变器电池侧电压：0.1V
    uint16_t battery_current                ;	//逆变器电池侧电流：0.1A
    uint16_t battery_power              	;	//逆变器电池侧功率:1w
    uint16_t battery_active_power           ;   //逆变器电池侧有功功率:1w
    uint16_t battery_reactive_power         ;   //逆变器电池侧无功功率
    uint16_t reserved3                      ;
    uint16_t r_phase_grid_voltage           ;
    uint16_t r_phase_grid_frequency         ;
    uint16_t r_phase_meter_current          ;
    uint16_t r_phase_meter_apparentpower    ;
    uint16_t r_phase_meter_activepower      ;
    uint16_t r_phase_meter_reactivepower    ;
    uint16_t s_phase_grid_voltage           ;
    uint16_t s_phase_grid_frequency         ;
    uint16_t s_phase_meter_current          ;
    uint16_t s_phase_meter_apparentpower    ;
    uint16_t s_phase_meter_activepower      ;
    uint16_t s_phase_meter_reactivepower    ;
    uint16_t t_phase_grid_voltage           ;
    uint16_t t_phase_grid_frequency         ;
    uint16_t t_phase_meter_current          ;
    uint16_t t_phase_meter_apparentpower    ;
    uint16_t t_phase_meter_activepower      ;
    uint16_t t_phase_meter_reactivepower    ;
    uint16_t phase_sequence        			;	//相序：指交流电系统中三相电压或电流的相对顺序。	0：正序  1：负序
    uint16_t oil_current					;
    uint16_t oil_voltage					;
    uint16_t oil_frequency					;
    uint16_t oil_power						;
    uint16_t reserved4[7]                   ;
    uint16_t r_phase_inverter_voltage   	;
    uint16_t r_phase_inverter_frequency 	;
    uint16_t r_phase_inverter_current		;
    uint16_t r_phase_inverter_apparentpower ;
    uint16_t r_phase_inverter_activepower	;
    uint16_t r_phase_inverter_reactivepower ;
    uint16_t s_phase_inverter_voltage   	;
    uint16_t s_phase_inverter_frequency 	;
    uint16_t s_phase_inverter_current		;
    uint16_t s_phase_inverter_apparentpower ;
    uint16_t s_phase_inverter_activepower	;
    uint16_t s_phase_inverter_reactivepower ;
    uint16_t t_phase_inverter_voltage   	;
    uint16_t t_phase_inverter_frequency 	;
    uint16_t t_phase_inverter_current		;
    uint16_t t_phase_inverter_apparentpower ;
    uint16_t t_phase_inverter_activepower	;
    uint16_t t_phase_inverter_reactivepower ;
    uint16_t reserved5[3]                   ;
    uint16_t r_phase_backup_voltage         ;
    uint16_t r_phase_backup_current        	;
    uint16_t r_phase_backup_apparentpower   ;
    uint16_t r_phase_backup_activepower     ;
    uint16_t r_phase_backup_reactivepower   ;
    uint16_t s_phase_backup_voltage         ;
    uint16_t s_phase_backup_current        	;
    uint16_t s_phase_backup_apparentpower   ;
    uint16_t s_phase_backup_activepower     ;
    uint16_t s_phase_backup_reactivepower   ;
    uint16_t t_phase_backup_voltage         ;
    uint16_t t_phase_backup_current        	;
    uint16_t t_phase_backup_apparentpower   ;
    uint16_t t_phase_backup_activepower     ;
    uint16_t t_phase_backup_reactivepower   ;
    uint16_t reserved6[3]                   ;

    uint16_t r_phase_load_apparentpower     ;
    uint16_t r_phase_load_activepower       ;
    uint16_t r_phase_load_reactivepower     ;
    uint16_t s_phase_load_apparentpower     ;
    uint16_t s_phase_load_activepower       ;
    uint16_t s_phase_load_reactivepower     ;
    uint16_t t_phase_load_apparentpower     ;
    uint16_t t_phase_load_activepower       ;
    uint16_t t_phase_load_reactivepower     ;
    uint16_t reserved7[8]                   ;

    uint16_t load_electric_cost_total_high  ;
    uint16_t load_electric_cost_total_low   ;
    uint16_t reserved8[7]                    ;

    uint16_t electric_purchase_total_high   ;
    uint16_t electric_purchase_total_low    ;
    uint16_t reserved9[5]                   ;
    uint16_t electric_sale_total_high       ;
    uint16_t electric_sale_total_low        ;
    uint16_t reserved10[17]                  ;

    uint16_t pv1_input_voltage              ;
    uint16_t pv2_input_voltage              ;
    uint16_t pv3_input_voltage              ;
    uint16_t pv4_input_voltage              ;
    uint16_t reserved11                   	;
    uint16_t pv1_input_current              ;
    uint16_t pv2_input_current              ;
    uint16_t pv3_input_current              ;
    uint16_t pv4_input_current              ;
    uint16_t reserved12                   	;
    uint16_t pv1_input_power                ;
    uint16_t pv2_input_power                ;
    uint16_t pv3_input_power                ;
    uint16_t pv4_input_power                ;
    uint16_t reserved13               		;
    uint16_t pv_power_generation_day        ;
    uint16_t pv_power_generation_month_high ;
    uint16_t pv_power_generation_month_low  ;
    uint16_t pv_power_generation_year_high  ;
    uint16_t pv_power_generation_year_low   ;
    uint16_t pv_power_generation_total_high ;
    uint16_t pv_power_generation_total_low  ;
    uint16_t pv1_power_generation_day       ;
    uint16_t pv1_power_generation_month_high;
    uint16_t pv1_power_generation_month_low ;
    uint16_t pv1_power_generation_year_high ;
    uint16_t pv1_power_generation_year_low  ;
    uint16_t pv1_power_generation_total_high;
    uint16_t pv1_power_generation_total_low ;
    uint16_t pv2_power_generation_day       ;
    uint16_t pv2_power_generation_month_high;
    uint16_t pv2_power_generation_month_low ;
    uint16_t pv2_power_generation_year_high ;
    uint16_t pv2_power_generation_year_low  ;
    uint16_t pv2_power_generation_total_high;
    uint16_t pv2_power_generation_total_low ;
    uint16_t pv3_power_generation_day       ;
    uint16_t pv3_power_generation_month_high;
    uint16_t pv3_power_generation_month_low ;
    uint16_t pv3_power_generation_year_high ;
    uint16_t pv3_power_generation_year_low  ;
    uint16_t pv3_power_generation_total_high;
    uint16_t pv3_power_generation_total_low ;
    uint16_t pv4_power_generation_day       ;
    uint16_t pv4_power_generation_month_high;
    uint16_t pv4_power_generation_month_low ;
    uint16_t pv4_power_generation_year_high ;
    uint16_t pv4_power_generation_year_low  ;
    uint16_t pv4_power_generation_total_high;
    uint16_t pv4_power_generation_total_low ;
    uint16_t pv_temperature                 ;
    uint16_t on_grid_time                   ;

    uint16_t reserved14[271]                ;
}ModbusReadReg_t;

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
    uint16_t	battery_charging_current              ;
    uint16_t    battery_charging_power                ;
    uint16_t    battery_discharging_voltage           ;
    uint16_t    battery_discharging_current           ;
    uint16_t    battery_discharging_power             ;
    uint16_t    reserve1[14]                           ;
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
    uint16_t    drm_enable                            ;
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

    uint16_t    reserve2[2]                           ;
    uint16_t    ct_or_meter                           ;
    uint16_t    meter_brand                           ;
    uint16_t    ct_direction                          ;
    uint16_t    antireverseflow_threshold             ;
    uint16_t    remote_shutdown_status                ;
    uint16_t    reactive_power_setting_value          ;
    uint16_t    reactive_power_control_method         ;
    uint16_t    power_factor_setting_value            ;
    uint16_t    active_power_setting_value            ;
    uint16_t    grid_voltage_lower_limit              ;
    uint16_t    grid_voltage_upper_limit              ;
    uint16_t    grid_frequency_lower_limit            ;
    uint16_t    grid_frequency_upper_limit            ;
    uint16_t	bypass_voltage_lower_limit            ;
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
    UpgradeState_t    sysupgradestatus                              ;
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
}ModbusReg_t;

extern ModbusReadReg_t ModbusReadReg;
extern ModbusReg_t     ModbusReg;
extern uint16_t gUpgraderRestartFlag;
void ModbusCmdProcess();
void ModbusRegInit(void);
#endif








