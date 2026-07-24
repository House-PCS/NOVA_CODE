#ifndef __PCS_OP_KSTAR_H_
#define __PCS_OP_KSTAR_H_

#include "dev_modbus_h.h"
#include "pcs_op.h"
#include "queue.h"

#define KSTAR_DEV_ADDR  0x01

// kstart 逆变器基本信息读取(多个寄存器)
#define PROTO_UNIFORM_R_KSTAR_REGISTER		0x00FFA005

// kstart 逆变器系统信息
#define PROTO_UNIFORM_R_KSTAR_SYS_INFO		0x00FFA007

// kstart 逆变器基本信息设置(一次一个寄存器)
#define PROTO_UNIFORM_W_KSTAR_REGISTER		0x80FFA008

// kstart 逆变器设置
#define PROTO_UNIFORM_W_KSTAR_SET_SYS		0x80FFA009

// kstart 逆变器执行指令
#define PROTO_UNIFORM_W_KSTAR_OPT_REGISTER	0x80FFA00A

#pragma pack(2)
typedef struct _kstar_pack_info_stru_t
{
    uint16_t            pack_add;
    uint16_t            pack_warn_flag;
    uint16_t            pack_protect_flag;
    uint16_t            pack_status_fault_flag;
    uint16_t            pack_bat_voltage;
    uint16_t            pack_current;
    uint16_t            pack_balance_status;
    uint16_t            pack_bat_cyc_cnt;
    uint16_t            pack_soh;
    uint16_t            pack_soc;
    uint16_t            reserved1;
    uint16_t            pack_online;
    uint16_t            reserved2;
    uint16_t            pack_design_capa;
    uint16_t            pack_residual_capa;
    uint16_t            pack_total_capa;
    uint16_t            pack_dsg_pow_sum;
    uint16_t            pack_chg_pow_sum;
    uint16_t            pack_heating_req;
    uint16_t            pack_cell_ave_temperature;
    uint16_t            pack_cell_max_temperature;
    uint16_t            pack_cell_min_temperature;
    uint16_t            pack_mos_ave_temperature;
    uint16_t            pack_mos_max_temperature;
    uint16_t            pack_mos_min_temperature;
    uint16_t            pack_env_ave_temperature;
    uint16_t            pack_env_max_temperature;
    uint16_t            pack_env_min_temperature;
    uint16_t            ave_cell_voltage;
    uint16_t            max_cell_voltage;
    uint16_t            min_cell_voltage;
    uint16_t            bms_ver[3];
    uint16_t            reserved3[10];
}KstarPackInfo_t;


typedef struct _kstar_inv_base_info_stru_t
{
    /*3000 - 3043*/
    uint16_t            pv1_input_voltage;
    uint16_t            pv2_input_voltage;
    uint16_t            reserved1[10];
    uint16_t            pv1_input_current;
    uint16_t            pv2_input_current;
    uint16_t            reserved2[10];
    uint16_t            pv1_input_power;
    uint16_t            pv2_input_power;
    uint16_t            reserved3[10];
    uint16_t            pv_capa_gen_day;
    uint16_t            pv_capa_gen_month[2];
    uint16_t            pv_capa_gen_year[2];
    uint16_t            pv_capa_gen_total[2];
    uint16_t            reserved4;
    /*3044 - 3061*/ 
    uint16_t            sys_scene;
    uint16_t            inv_model;
    uint16_t            sys_status;
    uint16_t            inv_status;
    uint16_t            dcdc_status;
    uint16_t            dsp_warning_code[2];
    uint16_t            dsp_error_code[2];
    uint16_t            bus_voltage;
    uint16_t            dcbus_voltage;
    uint16_t            inv_radiator_temperature;
    uint16_t            battery_radiator_temperature;
    uint16_t            dev_inside_temperature;
    uint16_t            reserved5;
    uint16_t            dod_enable;
    uint16_t            bms_dod_enable;
    uint16_t            off_grid_dod;
    /*3062 - 3096*/ 
    uint16_t            battery_type;
    uint16_t            battery_voltage;
    uint16_t            battery_current;
    uint16_t            battery_chg_dsg_power;
    uint16_t            battery_remain_capa;
    uint16_t            battery_temperature;
    uint16_t            on_grid_dod;
    uint16_t            battery_chg_voltage;
    uint16_t            battery_chg_max_current;
    uint16_t            battery_dsg_max_current;
    uint16_t            battery_dsg_end_voltage;
    uint16_t            battery_chg_protect_voltage;
    uint16_t            battery_rated_capa;
    uint16_t            battery_soh;
    uint16_t            reserved6;
    uint16_t            battery_pack_num;
    uint16_t            battery_dsg_power_disp;
    uint16_t            battery_chg_power_disp;
    uint16_t            battery_floating_chg_voltage_set;
    uint16_t            battery_average_chg_voltage_set;
    uint16_t            battery_chg_current_set;
    uint16_t            bms_wakeup_enable;
    uint16_t            bms_wakeup_time_set;
    uint16_t            reserved7;
    uint16_t            force_chg_dsg_enable;
    uint16_t            reserved8[10];
    /*3097 - 3155*/ 
    uint16_t            grid_voltage_r;
    uint16_t            grid_freq_r;
    uint16_t            meter_current_r;
    uint16_t            meter_power_r;
    uint16_t            grid_voltage_s;
    uint16_t            grid_freq_s;
    uint16_t            meter_current_s;
    uint16_t            meter_power_s;
    uint16_t            grid_voltage_t;
    uint16_t            grid_freq_t;
    uint16_t            meter_current_t;
    uint16_t            meter_power_t;
    uint16_t            buy_capa_day;
    uint16_t            buy_capa_month[2];
    uint16_t            buy_capa_year[2];
    uint16_t            buy_capa_total[2];
    uint16_t            sell_capa_day;
    uint16_t            sell_capa_month[2];
    uint16_t            sell_capa_year[2];
    uint16_t            sell_capa_total[2];
    uint16_t            inv_voltage_r;
    uint16_t            inv_current_r;
    uint16_t            inv_freq_r;
    uint16_t            inv_power_r;
    uint16_t            inv_voltage_s;
    uint16_t            inv_current_s;
    uint16_t            inv_freq_s;
    uint16_t            inv_power_s;
    uint16_t            inv_voltage_t;
    uint16_t            inv_current_t;
    uint16_t            inv_freq_t;
    uint16_t            inv_power_t;
    uint16_t            backup_voltage_r;
    uint16_t            backup_current_r;
    uint16_t            backup_power_r;
    uint16_t            backup_voltage_s;
    uint16_t            backup_current_s;
    uint16_t            backup_power_s;
    uint16_t            backup_voltage_t;
    uint16_t            backup_current_t;
    uint16_t            backup_power_t;
    uint16_t            load_power_r;
    uint16_t            load_power_s;
    uint16_t            load_power_t;
    uint16_t            load_capa_use_day;
    uint16_t            load_capa_use_month[2];
    uint16_t            load_capa_use_year[2];
    uint16_t            load_capa_use_total[2];
    uint16_t            grid_connected_num[2];
    /*3156 - 3226*/ 
    uint16_t            pv_temperature;
    uint16_t            llc_temperature;
    uint16_t            reserved9[2];
    uint16_t            pls_chg_start_time1[2];
    uint16_t            pls_chg_end_time1[2];
    uint16_t            pls_dsg_start_time1[2];
    uint16_t            pls_dsg_end_time1[2];
    uint16_t            pls_chg_start_time2[2];
    uint16_t            pls_chg_end_time2[2];
    uint16_t            pls_dsg_start_time2[2];
    uint16_t            pls_dsg_end_time2[2];
    uint16_t            timing_chg_dsg_status;
    uint16_t            reserved10[2];
    uint16_t            pv_input_mode;
    uint16_t            reserved11;
    uint16_t            drm_enable;
    uint16_t            anti_reflux_enable;
    uint16_t            anti_reflux_threshold;
    uint16_t            backup_out_enable;
    uint16_t            prog_ctl_enable;
    uint16_t            start_up_delay;
    uint16_t            remote_shut_down_status;
    uint16_t            reserved12[5];
    uint16_t            grid_standard;
    uint16_t            arm_error_code;
    uint16_t            reactive_power_set;
    uint16_t            reserved13;
    uint16_t            reactive_power_ctl_mode;
    uint16_t            reserved14;
    uint16_t            power_factor_set;
    uint16_t            active_power_set;
    uint16_t            grid_voltage_lower;
    uint16_t            grid_voltage_upper;
    uint16_t            grid_freq_lower;
    uint16_t            grid_freq_upper;
    uint16_t            bypass_voltage_lower;
    uint16_t            bypass_voltage_upper;
    uint16_t            bypass_freq_lower;
    uint16_t            bypass_freq_upper;
    uint16_t            over_voltage_derating_enable;
    uint16_t            over_freq_derating_enable;
    uint16_t            under_voltage_derating_enable;
    uint16_t            under_freq_derating_enable;
    uint16_t            over_voltage_derating_point;
    uint16_t            over_freq_derating_point;
    uint16_t            under_voltage_derating_point;
    uint16_t            under_freq_derating_point;
    uint16_t            voltage_cal_factor_r;
    uint16_t            voltage_cal_factor_s;
    uint16_t            voltage_cal_factor_t;
    uint16_t            current_cal_factor_r;
    uint16_t            current_cal_factor_s;
    uint16_t            current_cal_factor_t;
    uint16_t            battery_voltage_cal_factor;
    uint16_t            reserved15[3];
    /*3227 - 3440*/ 
    uint16_t            dsp_arm_ver;
    uint16_t            inv_sn[11];
    uint16_t            real_time_year;
    uint16_t            real_time_month;
    uint16_t            real_time_day;
    uint16_t            real_time_hour;
    uint16_t            real_time_minute;
    uint16_t            real_time_sencond;
    uint16_t            real_time_week;
    uint16_t            reserved16[5];
    uint16_t            dynamo_mode_enable;
    uint16_t            iso_island_enable;
    uint16_t            pack_force_wake_up;
    uint16_t            open_heating_film_manual;
    uint16_t            ct_dir;
    uint16_t            pv1_current_cal_factor;
    uint16_t            pv2_current_cal_factor;
    uint16_t            grid_standard_sub_divide;
    uint16_t            ac_couple_enable;
    uint16_t            grid_chg_max_soc;
    uint16_t            maintain_min_soc_enable;
    uint16_t            gfci_check_enable;
    uint16_t            vrt_enable;
    uint16_t            ct_or_meter;
    uint16_t            meter_brand; //3265
    uint16_t            block0[8];//协议文档空缺
    uint16_t            reserved17[6];
    uint16_t            internal_use[5];
    uint16_t            block1[7];//协议文档空缺
    uint16_t            battery_dsg_capa_total[2];//3292
    uint16_t            battery_dsg_capa_day;
    uint16_t            reserved18[4];
    uint16_t            battery_chg_capa_total[2];
    uint16_t            battery_chg_capa_day;
    uint16_t            reserved19[11];
    uint16_t            block2;//协议文档空缺
    uint16_t            self_comsumption_chg_enable;//3314
    uint16_t            block3[2];//协议文档空缺
    uint16_t            self_comsumption_chg_start_time;
    uint16_t            self_comsumption_chg_end_time;//3318
    uint16_t            block4[70];//协议文档空缺
    uint16_t            reserved20[52];
    KstarPackInfo_t     pack_info[8];
}KstarInvBaseInfo_t;

typedef struct _kstar_inv_sys_info_stru_t
{
    uint16_t            dev_model[8];
    uint16_t            battery_name[8];
    uint16_t            arm_ver;
    uint16_t            dsp_ver;
    uint16_t            dsp2_ver;
}KstarInvSysInfo_t;


typedef struct _kstar_dev_set_stru_t
{
    uint16_t            pls_chg_start_time1_set[2];
    uint16_t            pls_chg_end_time1_set[2];
    uint16_t            pls_dsg_start_time1_set[2];
    uint16_t            pls_dsg_end_time1_set[2];
    uint16_t            pls_chg_start_time2_set[2];
    uint16_t            pls_chg_end_time2_set[2];
    uint16_t            pls_dsg_start_time2_set[2];
    uint16_t            pls_dsg_end_time2_set[2];
    
    uint16_t            dev_model_set[8];
    uint16_t            battery_name_set[8];
    uint16_t            inv_sn_set[11];
    
    uint16_t            real_time_year_set;
    uint16_t            real_time_month_set;
    uint16_t            real_time_day_set;
    uint16_t            real_time_hour_set;
    uint16_t            real_time_minute_set;
    uint16_t            real_time_sencond_set;
    uint16_t            real_time_week_set;          
}KstarDevSet_t;

typedef struct _kstar_remote_cmd_stru_t
{
    uint16_t            clear_stat_info;
    uint16_t            remote_shut_down;
    uint16_t            remove_remote_shut_down;
}KstarRemoteCmd_t;


#pragma pack()

extern KstarInvBaseInfo_t gKstarInvBaseInfo;
extern KstarInvSysInfo_t  gKstarInvSysInfo;
extern KstarDevSet_t      gKstarDevSet;
extern KstarRemoteCmd_t   gKstarRemoteCmd;

int32_t KstarDevCheckReq(Queue_t *queue);

int32_t KstarDevCheck(DevOp_t *dev_op, ModbusRTURevData_t *modbus_rev);

int32_t KstarDevReadBaseInfoReqPush(Queue_t *queue, DevOpInfo_t *p_dev_op_info);

int32_t KstarDevReadBaseInfoReqPop(DevOpCtx_t *p_dev_op_ctx, DevOpInfo_t *p_op_info, 
								   Queue_t *op_queue, Queue_t *modbus_queue, 
								   DevOp_t *p_dev_op_get, 
								   ModbusRTURevData_t *p_modbus_dev_rev);


int32_t KstarBaseSet(uint16_t reg_addr, uint16_t reg_data);

int32_t KstarInvSet(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data);

int32_t KstarRemoteSet(uint16_t reg_addr, uint16_t reg_data);

#endif
