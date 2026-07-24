#ifndef __SLAVE_PCS_H_
#define __SLAVE_PCS_H_

#include "dev_modbus.h"
#include "bsp.h"
#include <stdio.h>

#define SLAVE_PCS_ADDR                                          0x06
#define SLAVE_ARC_ADDR                                          0X05


#define READ_INPUT_TYPE                                         0X04
#define READ_HOLD_TYPE                                          0X03
#define PCS_ARC_WRITE_QUEUE_DEPTH                               4

#define PCS_REG_IN_ADDR_MODE_ID                                 0X6000
#define PCS_REG_IN_ADDR_LOAD_CONSUM_ON_DAY                      0X6077
#define PCS_REG_HOLD_ADDR_BATT_CHG_VOLT                         0X61E2 
#define ARC_REG_IN_ADDR_MODE_ID                                 0X0000
#define ARC_REG_IN_END_ADDR                                     0X604F
#define ARC_REG_HOLD_SLAVE_ADDR                                 0X0100
#define ARC_REG_HOLD_END_ADDR                                   0X011F

#define ARC_REG_IN_MAX_NUM                                      (ARC_REG_IN_END_ADDR -  ARC_REG_IN_ADDR_MODE_ID + 1)                                                  
#define ARC_REG_HOLD_MAX_NUM                                    (ARC_REG_HOLD_END_ADDR - ARC_REG_HOLD_SLAVE_ADDR + 1)
                                   
#define PCS_REG_IN_FRONT_MAX_NUM                                (PCS_REG_IN_ADDR_LOAD_CONSUM_ON_DAY - PCS_REG_IN_ADDR_MODE_ID)    
#define PCS_REG_IN_BACK_MAX_NUM                                 (PCS_REG_HOLD_ADDR_BATT_CHG_VOLT - PCS_REG_IN_ADDR_LOAD_CONSUM_ON_DAY)
#define PCS_REG_HOLD_MAX_REG_NUM                                (0X6241 - PCS_REG_HOLD_ADDR_BATT_CHG_VOLT + 1)




#define ARC_REG_HOLD_SET_SLAVE_ADDR_ADDR                        0X100
#define ARC_REG_HOLD_SET_COMM_BAUD_ADDR                         0X101
#define ARC_REG_HOLD_ARC_SELFTEST_CMD_ADDR                      0X102

#define ARC_REG_HOLD_ARC_ALARM_BASE_THRESHOLD_ADDR              0x103
#define ARC_REG_HOLD_ARC_ALARM_CH1_THRESHOLD_ADDR               0X103         
#define ARC_REG_HOLD_ARC_ALARM_CH2_THRESHOLD_ADDR               0X104  
#define ARC_REG_HOLD_ARC_ALARM_CH3_THRESHOLD_ADDR               0X105  
#define ARC_REG_HOLD_ARC_ALARM_CH4_THRESHOLD_ADDR               0X106  
#define ARC_REG_HOLD_ARC_ALARM_CH5_THRESHOLD_ADDR               0X107  
#define ARC_REG_HOLD_ARC_ALARM_CH6_THRESHOLD_ADDR               0X108  
#define ARC_REG_HOLD_ARC_ALARM_CH7_THRESHOLD_ADDR               0X109  
#define ARC_REG_HOLD_ARC_ALARM_CH8_THRESHOLD_ADDR               0X10A  

#define ARC_REG_IMPEDANCE_ALARM_BASE_THRESHOLD_RX_ADDR          0X110


#define ARC_REG_IMPEDANCE_ALARM_CH1_THRESHOLD_RX_ADDR           0X110
#define ARC_REG_IMPEDANCE_ALARM_CH2_THRESHOLD_RX_ADDR           0X111
#define ARC_REG_IMPEDANCE_ALARM_CH3_THRESHOLD_RX_ADDR           0X112
#define ARC_REG_IMPEDANCE_ALARM_CH4_THRESHOLD_RX_ADDR           0X113
#define ARC_REG_IMPEDANCE_ALARM_CH5_THRESHOLD_RX_ADDR           0X114
#define ARC_REG_IMPEDANCE_ALARM_CH6_THRESHOLD_RX_ADDR           0X115
#define ARC_REG_IMPEDANCE_ALARM_CH7_THRESHOLD_RX_ADDR           0X116
#define ARC_REG_IMPEDANCE_ALARM_CH9_THRESHOLD_RX_ADDR           0X117

#define ARC_REG_IMPEDANCE_ALARM_BASE_THRESHOLD_RY_ADDR          0X118

#define ARC_REG_IMPEDANCE_ALARM_CH1_THRESHOLD_RY_ADDR           0X118
#define ARC_REG_IMPEDANCE_ALARM_CH2_THRESHOLD_RY_ADDR           0X119
#define ARC_REG_IMPEDANCE_ALARM_CH3_THRESHOLD_RY_ADDR           0X11A
#define ARC_REG_IMPEDANCE_ALARM_CH4_THRESHOLD_RY_ADDR           0X11B
#define ARC_REG_IMPEDANCE_ALARM_CH5_THRESHOLD_RY_ADDR           0X11C
#define ARC_REG_IMPEDANCE_ALARM_CH6_THRESHOLD_RY_ADDR           0X11D
#define ARC_REG_IMPEDANCE_ALARM_CH7_THRESHOLD_RY_ADDR           0X11E
#define ARC_REG_IMPEDANCE_ALARM_CH9_THRESHOLD_RY_ADDR           0X11F



#define PCS_REG_BATT_CHG_VOLT_ADDR                              0X61E2
#define PCS_REG_BATT_CHG_CURR_ADDR                              0X61E3
#define PCS_REG_BATT_CHG_POWER_ADDR                             0X61E4

//#define PCS_REG_PV_CHG_VOLT_ADDR                                0X60C7
//#define PCS_REG_PV_CHG_CURR_ADDR                                0X60C8
//#define PCS_REG_PV_CHG_POWER_ADDR                               0X60C9
#define PCS_REG_SET_CHG_INPUT_TYPE_ADDR                         0X61F6
#define PCS_REG_SET_AC_OUT_VOLT_LEVEL_ADDR                      0X61F7
#define PCS_REG_SET_AC_OUT_FREQ_ADDR                            0X61F8
#define PCS_REG_SET_GRID_STANDARD_ADDR                          0X61F9
#define PCS_REG_SET_GRID_STANDARD_SEG_ADDR                      0X61FA

#define PCS_REG_BACKUP_OUTPUT_ENABLE_ADDR                       0X61FB
#define PCS_REG_SET_BATT_DSG_CUT_VOLT_ADDR                      0X61FC
#define PCS_REG_SET_BATT_CHG_PROTECT_VOLT_ADDR                  0X61FD
#define PCS_REG_SET_PV_UDP_VOLT_ADDR                            0X61FE
#define PCS_REG_SET_PV_OVP_VOLT_ADDR                            0X61FF

#define PCS_REG_SET_BATT_DSG_POWER_SCHEDULING_ADDR              0X6200
#define PCS_REG_SET_BATT_CHG_POWER_SCHEDULING_ADDR              0X6201
#define PCS_REG_SET_PV_INPUT_MODE_ADDR                          0X6202
#define PCS_REG_SET_DRM_ENABLE_ADDR                             0X6203
#define PCS_REG_SET_ANTI_REVERSE_FLOW_ENABLE_ADDR               0X6204

#define PCS_REG_SET_PORGRAM_CTRL_ENABLE_ADDR                    0X6205
#define PCS_REG_SET_OVER_VOLT_DERATING_ENABLE_ADDR              0X6206
#define PCS_REG_SET_OVER_FREQ_DERATING_ENABLE_ADDR              0X6207

#define PCS_REG_SET_UNDER_VOLT_DERATING_ENABLE_ADDR             0X6208
#define PCS_REG_SET_UNDER_FREQ_DERATING_ENABLE_ADDR             0X6209

#define PCS_REG_SET_ISLAND_ENABLE_ADDR                          0X620A
#define PCS_REG_SET_ACC_COUPLING_ENABLE_ADDR                    0X620B

#define PCS_REG_SET_HIGH_LOW_CROSSOVER_ENABLE_ADDR              0X620C
#define PCS_REG_SELECT_CT_METER_ADDR                            0X620D
#define PCS_REG_SELECT_METER_BRAND_ADDR                         0X620E
#define PCS_REG_SET_CT_DIRECTION_ADDR                           0X620F

#define PCS_REG_SET_ANTI_REVERE_FLOW_THRESHOLD_ADDR             0X6210

#define PCS_REG_SET_REMOTE_SHUTDOWN_STATUS_ADDR                 0X6211

#define PCS_REG_SET_REACTIVE_POWER_VALUE_ADDR                   0X6212
#define PCS_REG_REACTIVE_POWER_CTRL_METHOD_ADDR                 0X6213
#define PCS_REG_POWER_FACTOR_SET_VALUE_ADDR                     0X6214
#define PCS_REG_ACTIVE_POWER_SET_VALUE_ADDR                     0X6215
                       
#define PCS_REG_GRID_VOLT_LOW_LIMIT_ADDR                        0X6216
#define PCS_REG_GRID_VOLT_UPPER_LIMIT_ADDR                      0X6217

#define PCS_REG_GRID_FREQ_LOW_LIMIT_ADDR                        0X6218
#define PCS_REG_GRID_FREQ_UPPER_LIMIT_ADDR                      0X6219
#define PCS_REG_BYPASS_VOLT_LOW_LIMIT_ADDR                      0X621A
#define PCS_REG_BYPASS_VOLT_UPPRR_LIMIT_ADDR                    0X621B

#define PCS_REG_BYPASS_FREQ_LOW_LIMIT_ADDR                      0X621C
#define PCS_REG_BYPASS_FREQ_UPPRR_LIMIT_ADDR                    0X621D
#define PCS_REG_OVER_VOLT_DERATING_VALUE_ADDR                   0X621E
#define PCS_REG_OVER_FREQ_DERATING_VALUE_ADDR                   0X621F
#define PCS_REG_UNDER_VOLT_DERATING_VALUE_ADDR                  0X6220
#define PCS_REG_UNDER_FREQ_DERATING_VALUE_ADDR                  0X6221

#define PCS_REG_R_PHASE_VOLT_CALIBRATION_COEFFICIENT_K_ADDR     0X6222
#define PCS_REG_R_PHASE_VOLT_CALIBRATION_COEFFICIENT_B_ADDR     0X6223
#define PCS_REG_S_PHASE_VOLT_CALIBRATION_COEFFICIENT_K_ADDR     0X6224
#define PCS_REG_S_PHASE_VOLT_CALIBRATION_COEFFICIENT_B_ADDR     0X6225
#define PCS_REG_T_PHASE_VOLT_CALIBRATION_COEFFICIENT_K_ADDR     0X6226
#define PCS_REG_T_PHASE_VOLT_CALIBRATION_COEFFICIENT_B_ADDR     0X6227



#define PCS_REG_R_PHASE_CURR_CALIBRATION_COEFFICIENT_K_ADDR     0X6228
#define PCS_REG_R_PHASE_CURR_CALIBRATION_COEFFICIENT_B_ADDR     0X6229
#define PCS_REG_S_PHASE_CURR_CALIBRATION_COEFFICIENT_K_ADDR     0X622A
#define PCS_REG_S_PHASE_CURR_CALIBRATION_COEFFICIENT_B_ADDR     0X622B
#define PCS_REG_T_PHASE_CURR_CALIBRATION_COEFFICIENT_K_ADDR     0X622C
#define PCS_REG_T_PHASE_CURR_CALIBRATION_COEFFICIENT_B_ADDR     0X622D




#define PCS_REG_PV_CURR_CALIBRATION_BASE_ADDR                   0X622E
#define PCS_REG_PV_CH1_CURR_CALIBRATION_K_ADDR                  0X622E
#define PCS_REG_PV_CH1_CURR_CALIBRATION_B_ADDR                  0X622F
#define PCS_REG_PV_CH2_CURR_CALIBRATION_K_ADDR                  0X6230
#define PCS_REG_PV_CH2_CURR_CALIBRATION_B_ADDR                  0X6231
#define PCS_REG_PV_CH3_CURR_CALIBRATION_K_ADDR                  0X6232
#define PCS_REG_PV_CH3_CURR_CALIBRATION_B_ADDR                  0X6233
#define PCS_REG_PV_CH4_CURR_CALIBRATION_K_ADDR                  0X6234
#define PCS_REG_PV_CH4_CURR_CALIBRATION_B_ADDR                  0X6235



#define PCS_REG_PV_VOLT_CALIBRATION_BASE_ADDR                   0X6100

#define PCS_REG_PV_CH1_VOLT_CALIBRATION_K_ADDR                  0X6236
#define PCS_REG_PV_CH1_VOLT_CALIBRATION_B_ADDR                  0X6237

#define PCS_REG_PV_CH2_VOLT_CALIBRATION_K_ADDR                  0X6238
#define PCS_REG_PV_CH2_VOLT_CALIBRATION_B_ADDR                  0X6239
#define PCS_REG_PV_CH3_VOLT_CALIBRATION_K_ADDR                  0X623A
#define PCS_REG_PV_CH3_VOLT_CALIBRATION_B_ADDR                  0X623B
#define PCS_REG_PV_CH4_VOLT_CALIBRATION_K_ADDR                  0X623C
#define PCS_REG_PV_CH4_VOLT_CALIBRATION_B_ADDR                  0X623D


#define PCS_REG_BATT_VOLT_CALIBRATE_K_ADDR                      0X623E
#define PCS_REG_BATT_VOLT_CALIBRATE_B_ADDR                      0X623F

#define PCS_REG_BATT_CURR_CALIBRATE_K_ADDR                      0X6240
#define PCS_REG_BATT_CURR_CALIBRATE_BADDR                       0X6241

#define PCS_CALIBRATION_PARAM_K                                 0
#define PCS_CALIBRATION_PARAM_B                                 1            

typedef enum
{
    EN_ARC_READ_INPUT_ARC_EVENT,
    EN_ARC_READ_HOLD_EVENT,
    EN_PCS_READ_FRONT_EVENT,
    EN_PCS_READ_BACK_EVENT,
    EN_PCS_HOLD_EVENT,
    EN_PCS_ARC_READ_EVENT_NUM
}EN_PCS_ARC_READ_EVENT;


typedef enum
{
    EN_PCS_ARC_POLLING_STATUS_IDLE=0,
    EN_PCS_ARC_POLLING_STATUS_WAITING_REV,
    EN_PCS_ARC_POLLING_STATUS_REV_OVERTIME,
    EN_PCS_ARC_POLLING_STATUS_REV_INTERVAL_WAIT,
}EN_PCS_ARC_POLLING_STATUS;


#pragma pack(2)

typedef struct _arc_input_para_stru_t
{
    uint16_t    mode_id;
    uint16_t    sn[10];
    uint16_t    hw_version;   
    uint16_t    soft_version;  
    uint16_t    reserve[2];
    uint16_t    channel_num;
    uint16_t    channle_curr_value[8];
    uint16_t    arc_alarm_info;
    uint16_t    real_time_arc_alarm_info;
    uint16_t    channel_arc_strength[8];
    uint16_t    channel_arc_history_max_value[8];
    uint16_t    arc_selftest_result;
    uint16_t    reserve1[5];
    uint16_t    channel_input_volt[8];
    uint16_t    channel_rx[8];
    uint16_t    channel_ry[8];
    uint16_t    impedance_alarm_info;
    uint16_t    inpedance_breakdown_info;     
}ARCInputPara_t;




typedef struct _arc_hold_para_stru_t
{
    uint16_t    slave_addr;
    uint16_t    comm_baud;
    uint16_t    arc_selftest_cmd;
    uint16_t    arc_alarm_threshold[8];
    uint16_t    reserved[5];
    uint16_t    impedance_alarm_threshold_rx[8];
    uint16_t    impedance_alarm_threshold_ry[8];        
}ARCHoldPara_t;







typedef struct _pcs_input_para_front_stru_t
{
    uint16_t    mode_id; //0:未定义  1 BSJ3622-03    //6000
    uint16_t    sn[10];
    uint16_t    hw_version;                         //0x600b   
    uint16_t    software_ver;
    uint16_t    inverter_model;
    uint16_t    reserve1[2];                        //600f
    uint16_t    system_status;                      //6010
    uint16_t    invs_status;
    uint16_t    dcdc_status;
    uint16_t    mppt_status;
    uint16_t    dsp_alarm_code[2];
    uint16_t    dsp_error_code[2];
    uint16_t    bus_voltage;                        //6018
    uint16_t    inverter_radiator_temp;
    uint16_t    chassis_internal_temp;
    uint16_t    temp_reserve[5];
    uint16_t    batt_volt;                          //6020
    uint16_t    batt_current;
    uint16_t    batt_chg_dsg_power;             //逆变器电池侧电压
    uint16_t    reserve2[3];

    uint16_t    r_phase_grid_volt;              //R 相电网电压    //6026
    uint16_t    r_phase_grid_freq;              //R 相电网频率 
    uint16_t    r_phase_meter_curr;             //R 相电表电流
    uint16_t    r_phase_meter_power;            //R 相电表功率
    uint16_t    r_phase_meter_active_power;     //R 相电表有功功率
    uint16_t    r_phase_meter_reactive_power;   //R 相电表无功功率
    uint16_t    s_phase_grid_volt;              //S 相电网电压
    uint16_t    s_phase_grid_freq;              //S 相电网频率 
    uint16_t    s_phase_meter_curr;             //S 相电表电流
    uint16_t    s_phase_meter_power;            //S 相电表功率
    uint16_t    s_phase_meter_active_power;     //S 相电表有功功率
    uint16_t    s_phase_meter_reactive_power;   //S 相电表无功功率
    uint16_t    t_phase_grid_volt;              //T 相电网电压
    uint16_t    t_phase_grid_freq;              //T 相电网频率 
    uint16_t    t_phase_meter_curr;             //T 相电表电流
    uint16_t    t_phase_meter_power;            //T 相电表功率
    uint16_t    t_phase_meter_active_power;     //T 相电表有功功率
    uint16_t    t_phase_meter_reactive_power;   //T 相电表无功功率
    uint16_t    phase_sequence;                 //相序  0-正序 1-负序        //6038


    uint16_t    oil_curr_value;                 //油机电流 0x6039
    uint16_t    oil_volt_value;                 //油机电压 0x603A 
    uint16_t    oil_freq_value;                 //油机频率 603B
    uint16_t    oil_power_value;                //油机功率 603C

    uint16_t    reserver3[7];

    uint16_t    r_phase_inverter_volt;          //R 相逆变电压               //6044      
    uint16_t    r_phase_inverter_freq;          //R 相逆变频率 
    uint16_t    r_phase_inverter_curr;          //R 相逆变电流
    uint16_t    r_phase_inverter_power;         //R 相逆变视在功率
    uint16_t    r_phase_inverter_active_power;  //R 相逆变有功功率
    uint16_t    r_phase_inverter_reactive_power;//R 相逆变无功功率

    uint16_t    s_phase_inverter_volt;          //S 相逆变电压
    uint16_t    s_phase_inverter_freq;          //S 相逆变频率 
    uint16_t    s_phase_inverter_curr;          //S 相逆变电流
    uint16_t    s_phase_inverter_power;         //S 相逆变视在功率
    uint16_t    s_phase_inverter_active_power;  //S 相逆变有功功率
    uint16_t    s_phase_inverter_reactive_power;//S 相逆变无功功率


    uint16_t    t_phase_inverter_volt;          //T 相逆变电压
    uint16_t    t_phase_inverter_freq;          //T 相逆变频率 
    uint16_t    t_phase_inverter_curr;          //T 相逆变电流
    uint16_t    t_phase_inverter_power;         //T 相逆变视在功率
    uint16_t    t_phase_inverter_active_power;  //T 相逆变有功功率
    uint16_t    t_phase_inverter_reactive_power;//T 相逆变无功功率

    uint16_t    inverter_apparent_power;        //逆变视在功率               //6056
    uint16_t    inverter_active_power;          //逆变有功功率
    uint16_t    inverter_reactive_power;        //逆变无功功率  


    uint16_t    r_phase_backup_volt             ;//R相BACKUP电压             //6059
    uint16_t    r_phase_backup_curr             ;//R相BACKUP电流    
    uint16_t    r_phase_backup_apparent_power   ;//R相BACKUP视在功率
    uint16_t    r_phase_backup_active_power     ;//R相BACKUP有用功率
    uint16_t    r_phase_backup_reactive_power   ;//R相BACKUP无用功率


    uint16_t    s_phase_backup_volt             ;//S相BACKUP电压             //605E
    uint16_t    s_phase_backup_curr             ;//S相BACKUP电流    
    uint16_t    s_phase_backup_apparent_power   ;//S相BACKUP视在功率
    uint16_t    s_phase_backup_active_power     ;//S相BACKUP有用功率
    uint16_t    s_phase_backup_reactive_power   ;//S相BACKUP无用功率


    uint16_t    t_phase_backup_volt             ;//T相BACKUP电压             //6063
    uint16_t    t_phase_backup_curr             ;//T相BACKUP电流    
    uint16_t    t_phase_backup_apparent_power   ;//T相BACKUP视在功率
    uint16_t    t_phase_backup_active_power     ;//T相BACKUP有用功率
    uint16_t    t_phase_backup_reactive_power   ;//T相BACKUP无用功率

    uint16_t    backup_apparent_power           ;//backup 视在功率          //6068
    uint16_t    backup_active_power             ;//backup 有功功率
    uint16_t    backup_reactive_power           ;//backip 无功功率

    uint16_t    r_phase_load_apparent_power     ;//R相负载视在功率
    uint16_t    r_phase_load_active_power       ;//R相负载有用功率
    uint16_t    r_phase_load_reactive_power     ;//R相负载无用功率


    uint16_t    s_phase_load_apparent_power     ;//S相负载视在功率
    uint16_t    s_phase_load_active_power       ;//S相负载有用功率
    uint16_t    s_phase_load_reactive_power     ;//S相负载无用功率

    uint16_t    t_phase_load_apparent_power     ;//T相负载视在功率
    uint16_t    t_phase_load_active_power       ;//T相负载有用功率
    uint16_t    t_phase_load_reactive_power     ;//T相负载无用功率

    uint16_t    load_apparent_power             ;//负载视在功率             //6074
    uint16_t    load_active_power               ;//负载有功功率             //6075
    uint16_t    load_reactive_power             ;//负载无功功率             //6076
}PCSInputFrontPara_t;




typedef struct _pcs_input_para_back_stru_t
{
    uint16_t    load_elec_consum_on_day         ;//负载当天的用电量  0.1KWH   //6077
    uint16_t    load_elec_consum_on_month[2]    ;//负载当月的用电量  KWH    
    uint16_t    load_elec_consum_on_year[2]     ;//负载当年的用电量  KWH 
    uint16_t    load_total_sonsum_power[2]      ;//负载总的用电量    KWH
    uint16_t    grid_connection_num[2]          ;//并网次数          KWH

    uint16_t    elec_purchase_consum_on_day     ;//买电当天电量               //6080
    uint16_t    elec_purchase_consum_on_month[2];//买电当月电量  
    uint16_t    elec_purchase_consum_on_year[2] ;//买电当年电量 
    uint16_t    elec_purchase_consum_total[2]   ;//买电总电量 

      
    uint16_t    elec_sale_consum_on_day         ;//卖电当天电量               //6087
    uint16_t    elec_sale_consum_on_month[2]    ;//卖电当月电量  
    uint16_t    elec_sale_consum_on_year[2]     ;//卖电当年电量 
    uint16_t    elec_sale_consum_total[2]       ;//卖电总电量 


    uint16_t    reserve[16];
    uint16_t    pv_ch1_input_volt               ;//PV1输入电压              //609F
    uint16_t    pv_ch2_input_volt               ;//PV2输入电压
    uint16_t    pv_ch3_input_volt               ;//PV3输入电压
    uint16_t    pv_ch4_input_volt               ;//PV4输入电压
    uint16_t    reserve1;

    uint16_t    pv_ch1_input_curr               ;//PV1输入电流
    uint16_t    pv_ch2_input_curr               ;//PV2输入电流
    uint16_t    pv_ch3_input_curr               ;//PV3输入电流
    uint16_t    pv_ch4_input_curr               ;//PV4输入电流
    uint16_t    reserve2;

    uint16_t    pv_ch1_input_power               ;//PV1输入功率
    uint16_t    pv_ch2_input_power               ;//PV2输入功率
    uint16_t    pv_ch3_input_power               ;//PV3输入功率
    uint16_t    pv_ch4_input_power               ;//PV4输入功率
    uint16_t    reserve3;                                                 //60AD




    uint16_t    pv_sum_generation_on_day        ;//PV 当天发电量  0.1kwh        //60AE
    uint16_t    pv_sum_generation_on_month[2]   ;//PV 当月发电量  kwh
    uint16_t    pv_sum_generation_on_year[2]    ;//PV 当年发电量  kwh
    uint16_t    pv_sum_generation_total[2]      ;//PV 总发电量

    uint16_t    pv_ch1_generation_on_day        ;//PV1 当天发电量
    uint16_t    pv_ch1_generation_on_month[2]   ;//PV1 当月发电量
    uint16_t    pv_ch1_generation_on_year[2]    ;//PV1 当年发电量
    uint16_t    pv_ch1_generation_on_total[2]   ;//PV1 总发电量


    uint16_t    pv_ch2_generation_on_day        ;//PV2 当天发电量
    uint16_t    pv_ch2_generation_on_month[2]   ;//PV2 当月发电量
    uint16_t    pv_ch2_generation_on_year[2]    ;//PV2 当年发电量
    uint16_t    pv_ch2_generation_on_total[2]   ;//PV2 总发电量


    uint16_t    pv_ch3_generation_on_day        ;//PV3 当天发电量
    uint16_t    pv_ch3_generation_on_month[2]   ;//PV3 当月发电量
    uint16_t    pv_ch3_generation_on_year[2]    ;//PV3 当年发电量
    uint16_t    pv_ch3_generation_on_total[2]   ;//PV3 总发电量

    uint16_t    pv_ch4_generation_on_day        ;//PV4 当天发电量
    uint16_t    pv_ch4_generation_on_month[2]   ;//PV4 当月发电量
    uint16_t    pv_ch4_generation_on_year[2]    ;//PV4 当年发电量
    uint16_t    pv_ch4_generation_on_total[2]   ;//PV4 总发电量

    uint16_t    pv_tempture                     ;//
    uint16_t    reserve4[16]                    ;//
}PCSInputbackPara_t;


typedef struct _pcs_hold_para_stru_t
{
    uint16_t    set_batt_chg_volt;         //0.1V            //61E2
    uint16_t    set_batt_chg_curr;          //0.01A
    uint16_t    set_batt_chg_power;        //1W
    uint16_t    reserved[17];                               //61e5--61f5

    uint16_t    set_chg_input_type;
    uint16_t    set_ac_output_volt_range;
    uint16_t    set_ac_oupput_freq_range;
    uint16_t    grid_standards;
    uint16_t    grid_standard_seg;                          //61FA
    uint16_t    backup_output_enable;
    uint16_t    batt_dsg_cutoff_volt;
    uint16_t    batt_chg_ovp_volt;
    uint16_t    pv_uvp_value;
    uint16_t    pv_ovp_value;                               //61FF
    uint16_t    batt_dsg_power_scheduling;                  //6200
    uint16_t    batt_chg_power_scheduling;                  //6201
    uint16_t    pv_input_mode;
    uint16_t    drm_enable;
    uint16_t    anti_reverse_flow_enable;
    uint16_t    program_ctrl_enable;
    uint16_t    over_volt_derating_enable;
    uint16_t    over_freq_derating_enable;
    uint16_t    under_volt_derating_enable;
    uint16_t    under_freq_derating_enable;
    uint16_t    island_enable;                              //602A
    uint16_t    acc_coupling_enable;
    uint16_t    high_low_crossover_enable;
    uint16_t    cto_meter;
    uint16_t    meter_brand;
    uint16_t    ct_direction;
    uint16_t    anti_reverse_flow_threshold;                //6210
    uint16_t    remote_shutdown_status;
    uint16_t    reactive_power_setting_value;
    uint16_t    reactive_power_ctrl_method;
    uint16_t    power_factor_setting_value;
    uint16_t    active_power_setting_value;
    uint16_t    grid_volt_low_limit;
    uint16_t    grid_volt_upper_limit;
    uint16_t    grid_freq_low_limit;
    uint16_t    grid_freq_upper_limit;
    uint16_t    bypass_volt_low_limit;
    uint16_t    bypass_volt_upper_limit;
    uint16_t    bypass_freq_low_limit;
    uint16_t    bypass_freq_upper_limit;
    uint16_t    over_volt_derating_point;
    uint16_t    over_freq_derating_point;
    uint16_t    under_volt_derating_point;                  //6220
    uint16_t    under_freq_derating_point;                  //6221

    uint16_t    r_phase_volt_calibration_k_parm;
    uint16_t    r_phase_volt_calibration_b_parm;
    uint16_t    s_phase_volt_calibration_k_parm;
    uint16_t    s_phase_volt_calibration_b_parm;
    uint16_t    t_phase_volt_calibration_k_parm;
    uint16_t    t_phase_volt_calibration_b_parm;

    uint16_t    r_phase_cur_calibration_k_parm;
    uint16_t    r_phase_cur_calibration_b_parm;
    uint16_t    s_phase_cur_calibration_k_parm;
    uint16_t    s_phase_cur_calibration_b_parm;
    uint16_t    t_phase_cur_calibration_k_parm;
    uint16_t    t_phase_cur_calibration_b_parm;


    uint16_t    pv_ch1_curr_k_calibrate;
    uint16_t    pv_ch1_curr_b_calibrate;
    uint16_t    pv_ch2_curr_k_calibrate;                    //6230
    uint16_t    pv_ch2_curr_b_calibrate;
    uint16_t    pv_ch3_curr_k_calibrate;
    uint16_t    pv_ch3_curr_b_calibrate;
    uint16_t    pv_ch4_curr_k_calibrate;
    uint16_t    pv_ch4_curr_b_calibrate;

    uint16_t    pv_ch1_volt_k_calibrate;
    uint16_t    pv_ch1_volt_b_calibrate;
    uint16_t    pv_ch2_volt_k_calibrate;
    uint16_t    pv_ch2_volt_b_calibrate;
    uint16_t    pv_ch3_volt_k_calibrate;
    uint16_t    pv_ch3_volt_b_calibrate;
    uint16_t    pv_ch4_volt_k_calibrate;
    uint16_t    pv_ch4_volt_b_calibrate;


    uint16_t    batt_volt_k_calibrate;                     //623E
    uint16_t    batt_volt_b_calibrate;

    uint16_t    batt_curr_k_calibrate;
    uint16_t    batt_curr_b_calibrate;
}PCSHoldPara_t;

typedef struct _pcs_para_stru_t
{
    PCSInputFrontPara_t     pcs_input_front_para;
    PCSInputbackPara_t      pcs_input_back_para;  
    PCSHoldPara_t           pcs_hold_para;
}PCSPara_t;


typedef struct _arc_para_stru_t
{
    ARCInputPara_t          arc_input_para;
    ARCHoldPara_t           arc_hold_para;  
}ARCPara_t;


typedef struct _modbus_pcs_arc_para_stru_t
{
    PCSPara_t                       pcs_para;
    ARCPara_t                       arc_para;
    uint8_t                         pcs_comm_err_cnt;
    uint8_t                         arc_comm_err_cnt;
    uint8_t                         pcs_comm_time_out;
    uint8_t                         arc_comm_time_out;
}ModBusPcsArcPara_t;



#pragma pack()

void    SlaveArcModbusCommProcess(void);
void    SlavePcsArcCommInit(void);
//bool    SetSlaveAddr(uint16_t val);
//bool    SetCommBaud(uint16_t val);
//bool    SetSelfTestCmd(uint16_t cmd);
//bool    SetArcAlarmThreshold(uint8_t channel,uint16_t val);
//bool    SetImpedanceAlarmRxThreshold(uint8_t channel,uint16_t val);
//bool    SetImpedanceAlarmRyThreshold(uint8_t channel,uint16_t val);


//bool PcsSetBattChgVolt(uint16_t val);
//bool PcsSetBattChgCurr(uint16_t val);
//bool PcsSetBattChgPower(uint16_t val);
//bool PcsSetChgInputType(uint16_t type);
//bool PcsSetAcOutputVoltRange(uint16_t volt_range);
//bool PcsSetAcOutputFreqRange(uint16_t freq_range);
//bool PcsSetGridStandard(uint16_t grid_standard);
//bool PcsSetGridStandardSegmentation(uint16_t grid_standard_seg);
//bool PcsSetBackupOutputStatus(uint16_t status);
//bool PcsSetBattDsgCutOfVoltValue(uint16_t val);
//bool PcsSetBattChgVoltProtectValue(uint16_t val);
//bool PcsSetPvUdpValue(uint16_t val);
//bool PcsSetPvOvpValue(uint16_t val);
//bool PcsSetBattDsgPowerSchedulingValue(uint16_t val);
//bool PcsSetBattChgPowerSchedulingValue(uint16_t val);
//bool PcsSetPvInputMode(uint16_t mode);
//bool PcsSetDrmStatus(uint16_t status);
//bool PcsSetAntiReverseFlowStatus(uint16_t status);
//bool PcsSetProgramCtrlStatus(uint16_t status);
//bool PcsSetOverVoltDeratingStatus(uint16_t status);
//bool PcsSetOverFreqDeratingStatus(uint16_t status);
//bool PcsSetUnderVoltDeratingStatus(uint16_t status);
//bool PcsSetUnderFreqDeratingStatus(uint16_t status);
//bool PcsSetIslandStatus(uint16_t status);
//bool PcsSetAcCouplingStatus(uint16_t status);
//bool PcsSetHighLowCrossoverStatus(uint16_t status);
//bool PcsSelectCtOrMeter(uint16_t status);
//bool PcsSetMeterBrand(uint16_t status);
//bool PcsSetCtDirection(uint16_t status);
//bool PcsSetAntiReverseFlowThreshold(uint16_t val);
//bool PcsSetRomoteShutdownStatus(uint16_t status);
//bool PcsSetReactivePowerValue(uint16_t val);
//bool PcsSetReactivePowerCtrlMethod(uint16_t method);
//bool PcsSetPowerFactorValue(uint16_t val);
//bool PcsSetActivePowerPersentValue(uint16_t val);
//bool PcsSetGridVoltLowLimitValue(uint16_t val);
//bool PcsSetGridVoltUpperLimitValue(uint16_t val);
//bool PcsSetGridFreqLowLimitValue(uint16_t val);
//bool PcsSetGridFreqUpperLimitValue(uint16_t val);
//bool PcsSetBypassVoltLowLimitValue(uint16_t val);
//bool PcsSetBypassVoltUpperLimitValue(uint16_t val);
//bool PcsSetBypassFreqLowLimitValue(uint16_t val);
//bool PcsSetBypassFreqUpperLimitValue(uint16_t val);
//bool PcsSetOverVoltDeratingValue(uint16_t val);
//bool PcsSetOverFreqDeratingValue(uint16_t val);
//bool PcsSetUnderVoltDeratingValue(uint16_t val);
//bool PcsSetUnderFreqDeratingValue(uint16_t val);
//bool PcsSetRphaseVoltCalibrationCoefficient(uint16_t val);
//bool PcsSetSphaseVoltCalibrationCoefficient(uint16_t val);
//bool PcsSetTphaseVoltCalibrationCoefficient(uint16_t val);
//bool PcsSetRphaseCurrCalibrationCoefficient(uint16_t val);
//bool PcsSetSphaseCurrCalibrationCoefficient(uint16_t val);
//bool PcsSetTphaseCurrCalibrationCoefficient(uint16_t val);
//bool PcsSetPvCurrCalibration(uint8_t param_type,uint8_t channel,uint16_t val);
//bool PcsSetPvVoltCalibration(uint8_t param_type,uint8_t channel,uint16_t val);
//bool PcsSetBattVoltCalibration(uint8_t param_type,uint16_t val);
//bool PcsSetBattCurrCalibration(uint8_t param_type,uint16_t val);
bool CollectWritePcsFun(uint16_t reg_addr,uint16_t reg_num,uint8_t *pdata,uint16_t slave_addr);


void GetDataFromArc(uint8_t *pdata,uint16_t data_num);
void GetPcsInputFrontData(uint8_t *pdata,uint16_t data_num);
void GetPcsInputBackData(uint8_t *pdata,uint16_t data_num);
void GetPcsHoldData(uint8_t *pdata,uint16_t data_num);

#endif