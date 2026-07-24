/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        ethernet_analysis.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-1-17
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-1-17    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef ETHERNET_ANALYSIS_H_
#define ETHERNET_ANALYSIS_H_

#include "F28x_Project.h"

#define MODBUS_INPUT_REG_START_ADDR      (0x6000)
#define MODBUS_INPUT_REG_END_ADDR        (0x61FF)
#define MODBUS_HOLD_REG_START_ADDR       (0x6200)
#define MODBUS_HOLD_REG_END_ADDR         (0x62FF)

#define MODBUS_HOLD_REG_START_ADDR_TEST  (0x8000)
#define MODBUS_HOLD_REG_END_ADDR_TEST    (0x8002)
#define MODBUS_INPUT_REG_START_ADDR_TEST (0x8010)
#define MODBUS_INPUT_REG_END_ADDR_TEST   (0x802C)

typedef enum
{
    EN_ETH_ANALYSIS_ERROR, //解析失败
    EN_ETH_ANALYSIS_OK,     //解析成功
    EN_ETH_ANALYSIS_OK_NORESPONSE,   //解析成功不回复

} EN_ETH_ANALYSIS_STATUS;
    

// 结构体定义
typedef struct _monitor_udp_stru_t
{
    uint8_t DeviceType0;    //[7:0] 2 Byte位宽，设备类型。0x0000：逆变器模块；0x0001；Pack包模块。
    uint8_t DeviceType1;    //[15:8]

    uint8_t Reserved;       //[7:0] 1 Byte位宽，预留字节。
    uint8_t OpCmd;          //[7:0] 1 Byte位宽，操作指令。0x45：写操作；0x43：读操作。

    uint8_t Addr0;          //[7:0] 4 Byte位宽，操作地址。表示读写指令需要操作的寄存器起始地址。
    uint8_t Addr1;          //[15:8]
    uint8_t Addr2;          //[23:16]
    uint8_t Addr3;          //[31:24]

    uint8_t Length0;        //[7:0] 2 Byte位宽，操作长度。表示数据用户数据总长度，该参数对应数据首部和负载有效数据长度总和。
    uint8_t Length1;        //[15:8]
    uint8_t DataBuff[1400]; //数据


    uint16_t Length;         //实际长度
    uint32_t Addr;           //实际地址
} MonitorUdp_t;

typedef struct _network_info_stru_t
{
    uint16_t  arr_broadcast_address[6];    //广播地址
    uint16_t  arr_master_mac_address[6];   //当前主机的MAC地址
    uint16_t  arr_master_ip_address[4];    //当前主机的IP地址
    uint16_t  arr_master_udp_port[2];      //主机端口
    uint16_t  arr_ch390_mac_address[6];    //CH390的MAC地址
    uint16_t  arr_ch390_ip_address[4];     //设定当前CH390的IP地址
    uint16_t  arr_ch390_udp_port[2];
    uint16_t  arr_recv_target_ipaddr[4];   //接收到的访问地址
    uint16_t  arr_recv_udp_ipv_type[2];        //Ipv4 或 Ipv6类型
    
    uint16_t  udp_packet_len;
    uint16_t  udp_data_len;
    uint16_t  protocol_type;           //协议类型
} NetworkInfo_t;


typedef struct _udp_dsp_output_para_stru_t
{
    uint16_t    mode_id;                            //0:未定义  1 BSJ3622-03    //6000
    uint16_t    sn[10];                             //6001
    uint16_t    hw_version;                         //0x600b
    uint16_t    software_ver;                       //0x600C
    uint16_t    reserve1;                           //0x600D    预留
    uint16_t    version_0;                          //0x600E
    uint16_t    version_1;                          //0x600F
    uint16_t    version_2;                          //0x6010
    uint16_t    version_3;                          //0x6011
    uint16_t    version_4;                          //0x6012
    uint16_t    version_5;                          //0x6013
    uint16_t    reserve2;                           //0x6014

    uint16_t    inverter_model;                     //601F
    uint16_t    reserve3[2];                        //6020
    uint16_t    sys_input_stat;                     //6021
    uint16_t    sys_stas;                           //6022
    uint16_t    inv_status;                         //6023
    uint16_t    dcdc_status;                        //6024
    uint16_t    mppt1_status;                       //6025
    uint16_t    mppt2_status;                       //6026
    uint16_t    mppt3_status;                       //6027
    uint16_t    mppt4_status;                       //6028
    uint16_t    reserve4[2];                        //6029 - 602A

    uint16_t    dsp_alarm_code[2];                  //602B - 602C
    uint16_t    dsp_error_code[2];                  //602D - 602E
    uint16_t    reserve5[2];                        //601E - 601F

    uint16_t    bus_voltage;                        //6031
    int16_t     inverter_radiator_temp;
    int16_t     chassis_internal_temp;
    int16_t     pv_temp;

    uint16_t    reserve6[12];
    uint16_t    batt_volt;
    uint16_t    batt_current;
    uint16_t    batt_power;
    uint16_t    reserve7[5];

    uint16_t    r_phase_grid_volt;              //R 相电网电压
    uint16_t    r_phase_grid_freq;              //R 相电网频率
    int16_t     r_phase_meter_curr;             //R 相电表电流
    int16_t     r_phase_inv_grid_curr;          //R 相逆变器电网端电流
    int16_t     r_phase_load_cur;               //R 相远端负载电流
    int16_t     r_phase_meter_power;            //R 相电表视在功率
    int16_t     r_phase_meter_active_power;     //R 相电表有功功率
    int16_t     r_phase_meter_reactive_power;   //R 相电表无功功率
    int16_t     r_phase_inv_grid_power;         //R 相逆变电网口视在功率
    int16_t     r_phase_inv_grid_active_power;  //R 相逆变电网口有功功率
    int16_t     r_phase_inv_grid_reactive_power;//R 相逆变电网口无功功率
    int16_t     r_phase_load_power;             //R 相远端负载视在功率
    int16_t     r_phase_load_active_power;      //R 相远端负载有功功率
    int16_t     r_phase_load_reactive_power;    //R 相远端负载无功功率

    uint16_t    s_phase_grid_volt;              //S 相电网电压
    uint16_t    s_phase_grid_freq;              //S 相电网频率
    int16_t     s_phase_meter_curr;             //S 相电表电流
    int16_t     s_phase_inv_grid_curr;          //S 相逆变器电网端电流
    int16_t     s_phase_load_cur;               //S 相远端负载电流
    int16_t     s_phase_meter_power;            //S 相电表视在功率
    int16_t     s_phase_meter_active_power;     //S 相电表有功功率
    int16_t     s_phase_meter_reactive_power;   //S 相电表无功功率
    int16_t     s_phase_inv_grid_power;         //S 相逆变电网口视在功率
    int16_t     s_phase_inv_grid_active_power;  //S 相逆变电网口有功功率
    int16_t     s_phase_inv_grid_reactive_power;//S 相逆变电网口无功功率
    int16_t     s_phase_load_power;             //S 相远端负载视在功率
    int16_t     s_phase_load_active_power;      //S 相远端负载有功功率
    int16_t     s_phase_load_reactive_power;    //S 相远端负载无功功率

    uint16_t    t_phase_grid_volt;              //T 相电网电压
    uint16_t    t_phase_grid_freq;              //T 相电网频率
    int16_t     t_phase_meter_curr;             //T 相电表电流
    int16_t     t_phase_inv_grid_curr;          //T 相逆变器电网端电流
    int16_t     t_phase_load_cur;               //T 相远端负载电流
    int16_t     t_phase_meter_power;            //T 相电表视在功率
    int16_t     t_phase_meter_active_power;     //T 相电表有功功率
    int16_t     t_phase_meter_reactive_power;   //T 相电表无功功率
    int16_t     t_phase_inv_grid_power;         //T 相逆变电网口视在功率
    int16_t     t_phase_inv_grid_active_power;  //T 相逆变电网口有功功率
    int16_t     t_phase_inv_grid_reactive_power;//T 相逆变电网口无功功率
    int16_t     t_phase_load_power;             //T 相远端负载视在功率
    int16_t     t_phase_load_active_power;      //T 相远端负载有功功率
    int16_t     t_phase_load_reactive_power;    //T 相远端负载无功功率
    uint16_t    phase_sequence;                 //0 正序  1负序
    uint16_t    reserve8[13];

    uint16_t    r_phase_oil_volt;               //R 相油机电压
    uint16_t    r_phase_oil_freq;               //R 相油机频率
    int16_t     r_phase_oil_curr;               //R 相油机电流
    int16_t     r_phase_oil_power;              //R 相油机视在功率
    int16_t     r_phase_oil_active_power;       //R 相油机有功功率
    int16_t     r_phase_oil_reactive_power;     //R 相油机无功功率


    uint16_t    s_phase_oil_volt;               //S 相油机电压
    uint16_t    s_phase_oil_freq;               //S 相油机频率
    int16_t     s_phase_oil_curr;               //S 相油机电流
    int16_t     s_phase_oil_power;              //S 相油机视在功率
    int16_t     s_phase_oil_active_power;       //S 相油机有功功率
    int16_t     s_phase_oil_reactive_power;     //S 相油机无功功率

    uint16_t    t_phase_oil_volt;               //T 相油机电压
    uint16_t    t_phase_oil_freq;               //T 相油机频率
    int16_t     t_phase_oil_curr;               //T 相油机电流
    int16_t     t_phase_oil_power;              //T 相油机视在功率
    int16_t     t_phase_oil_active_power;       //T 相油机有功功率
    int16_t     t_phase_oil_reactive_power;     //T 相油机无功功率

    int16_t     reserve9[5];

    uint16_t    r_phase_inverter_volt;          //R 相逆变电压
    uint16_t    r_phase_inverter_freq;          //R 相逆变频率
    int16_t     r_phase_inverter_curr;          //R 相逆变电流
    int16_t     r_phase_inverter_power;         //R 相逆变视在功率
    int16_t     r_phase_inverter_active_power;  //R 相逆变有功功率
    int16_t     r_phase_inverter_reactive_power;//R 相逆变无功功率

    uint16_t    s_phase_inverter_volt;          //S 相逆变电压
    uint16_t    s_phase_inverter_freq;          //S 相逆变频率
    int16_t     s_phase_inverter_curr;          //S 相逆变电流
    int16_t     s_phase_inverter_power;         //S 相逆变视在功率
    int16_t     s_phase_inverter_active_power;  //S 相逆变有功功率
    int16_t     s_phase_inverter_reactive_power;//S 相逆变无功功率

    uint16_t    t_phase_inverter_volt;          //T 相逆变电压
    uint16_t    t_phase_inverter_freq;          //T 相逆变频率
    int16_t     t_phase_inverter_curr;          //T 相逆变电流
    int16_t     t_phase_inverter_power;         //T 相逆变视在功率
    int16_t     t_phase_inverter_active_power;  //T 相逆变有功功率
    int16_t     t_phase_inverter_reactive_power;//T 相逆变无功功率

    uint16_t    reserve10[5];

    uint16_t    r_phase_backup_volt             ;//R相BACKUP电压
    int16_t     r_phase_backup_curr             ;//R相BACKUP电流
    int16_t     r_phase_backup_power            ;//R相BACKUP视在功率
    int16_t     r_phase_backup_active_power     ;//R相BACKUP有用功率
    int16_t     r_phase_backup_reactive_power   ;//R相BACKUP无用功率

    uint16_t    s_phase_backup_volt             ;//S相BACKUP电压
    int16_t     s_phase_backup_curr             ;//S相BACKUP电流
    int16_t     s_phase_backup_power            ;//S相BACKUP视在功率
    int16_t     s_phase_backup_active_power     ;//S相BACKUP有用功率
    int16_t     s_phase_backup_reactive_power   ;//S相BACKUP无用功率


    uint16_t    t_phase_backup_volt             ;//T相BACKUP电压
    int16_t     t_phase_backup_curr             ;//T相BACKUP电流
    int16_t     t_phase_backup_power            ;//T相BACKUP视在功率
    int16_t     t_phase_backup_active_power     ;//T相BACKUP有用功率
    int16_t     t_phase_backup_reactive_power   ;//T相BACKUP无用功率
    uint16_t    reserve11[5];

    uint16_t    on_grid_time;                     //并网时间   //0X60B2
    uint16_t    grid_connect_number[2];           //并网次数
    uint16_t    reserve12[5];
    uint16_t    load_elec_consum_on_day          ;//远端负载当天的用电量  0.1KWH
    uint16_t    load_elec_consum_on_month[2]     ;//远端负载当月的用电量  KWH
    uint16_t    load_elec_consum_on_year[2]      ;//远端负载当年的用电量  KWH
    uint16_t    load_total_sonsum_power[2]       ;//远端负载总的用电量    KWH
    uint16_t    backup_elec_consum_on_day        ;//backup负载当天的用电量  0.1KWH
    uint16_t    backup_elec_consum_on_month[2]   ;//backup负载当月的用电量  KWH
    uint16_t    backup_elec_consum_on_year[2]    ;//backup负载当年的用电量  KWH
    uint16_t    backup_total_sonsum_power[2]     ;//backup负载总的用电量    KWH
    uint16_t    elec_purchase_consum_on_day      ;//买电当天电量
    uint16_t    elec_purchase_consum_on_month[2] ;//买电当月电量
    uint16_t    elec_purchase_consum_on_year[2]  ;//买电当年电量
    uint16_t    elec_purchase_consum_total[2]    ;//买电总电量
    uint16_t    elec_sale_consum_on_day          ;//卖电当天电量
    uint16_t    elec_sale_consum_on_month[2]     ;//卖电当月电量
    uint16_t    elec_sale_consum_on_year[2]      ;//卖电当年电量
    uint16_t    elec_sale_consum_total[2]        ;//卖电总电量
    uint16_t    reserve13[24];

    uint16_t    pv_ch1_input_volt               ;//PV1输入电压
    uint16_t    pv_ch1_input_curr               ;//PV1输入电流
    uint16_t    pv_ch1_input_power               ;//PV1输入功率
    uint16_t    reserve14[2];
    uint16_t    pv_ch2_input_volt               ;//PV2输入电压
    uint16_t    pv_ch2_input_curr               ;//PV2输入电流
    uint16_t    pv_ch2_input_power              ;//PV2输入功率
    uint16_t    reserve15[2];
    uint16_t    pv_ch3_input_volt               ;//PV3输入电压
    uint16_t    pv_ch3_input_curr               ;//PV3输入电流
    uint16_t    pv_ch3_input_power              ;//PV3输入功率
    uint16_t    reserve16[2];
    uint16_t    pv_ch4_input_volt               ;//PV4输入电压
    uint16_t    pv_ch4_input_curr               ;//PV4输入电流
    uint16_t    pv_ch4_input_power              ;//PV4输入功率
    uint16_t    reserve17[10];
    uint16_t    pv1_gennerate_day;
    uint16_t    pv1_gennerate_month[2];
    uint16_t    pv1_gennerate_year[2];
    uint16_t    pv1_gennerate_total[2];

    uint16_t    pv2_gennerate_day;
    uint16_t    pv2_gennerate_month[2];
    uint16_t    pv2_gennerate_year[2];
    uint16_t    pv2_gennerate_total[2];

    uint16_t    pv3_gennerate_day;
    uint16_t    pv3_gennerate_month[2];
    uint16_t    pv3_gennerate_year[2];
    uint16_t    pv3_gennerate_total[2];


    uint16_t    pv4_gennerate_day;
    uint16_t    pv4_gennerate_month[2];
    uint16_t    pv4_gennerate_year[2];
    uint16_t    pv4_gennerate_total[2];

    uint16_t    reserve18[20];
    uint16_t    dsp_wake_up_status;    //0x613A
    uint16_t    reserve19[180];
} UdpDspOutputPara_t;

typedef struct _udp_dsp_hold_para_stru_t
{
    uint16_t    set_batt_chg_volt;                          //0x6200 设置电池充电截止电压    0.1V
    uint16_t    set_batt_chg_curr;                          //0x6201 设置电池充电最大电流    0.1A
    uint16_t    set_batt_chg_power;                         //0x6202 设置电池充电最大功率    1.0W
    uint16_t    set_batt_dsg_volt;                          //0x6203 设置电池放电截止电压    0.1V
    uint16_t    set_batt_dsg_curr;                          //0x6204 设置电池放电最大电流    0.1A
    uint16_t    set_batt_dsg_power;                         //0x6205 设置电池放电最大功率    1.0W
    uint16_t    x_boost;                                    //0x6206 设置X-BOOST功能使能   1:使能 0:禁止
    uint16_t    ups_delay_0ms;                              //0x6207 设置Ups的0毫秒切换功能使能    1:使能 0:禁止
    uint16_t    backup_output_enable;                       //0x6208 设置本地负载输出使能    1:使能 0:禁止
    uint16_t    charging_input_type;                        //0x6209 设置强制发电机使能    1:强制为发电机 0:强制为市电
    uint16_t    work_mode;                                  //0x620A 设置工作模式    2:削峰填谷 1:电池优先 0:自发自用
    uint16_t    reserve[21];
    uint16_t    ac_output_volt;                             //0x6220 设置AC输出电压档位 00:电压不变 01:100V 02:120V 03:220V 04:230V
    uint16_t    ac_output_freq;                             //0x6221 设置AC输出频率档位 00:频率不变 01:60HZ 02:50HZ
    uint16_t    grid_standards;                             //0x6222 设置市场协商电网标准 00:中国 01:德国 02:澳洲A 03:澳洲B 04:澳洲C 05:新西兰
    uint16_t    grad_standards_segment;                     //0x6223 设置电网标准细分 详细见协议 不常用变量
    uint16_t    batt_dsg_power_schedule;                    //0x6224 设置电池放电最大功率百分比 0% - 100%
    uint16_t    batt_chg_power_schedule;                    //0x6225 设置电池充电最大功率百分比 0% - 100%
    uint16_t    pv_input_mode;                              //0x6226 设置PV输入模式 00:输入独立模式 01:输入并联模式 02:输入恒压模式
    uint16_t    anti_reverse_flow_enable;                   //0x6227 设置防逆流使能 00:禁止PV给电网送电 01:允许PV给电网送电
    uint16_t    program_ctrl_enable;                        //0x6228 设置程序控制使能 00:禁止 01:使能
    uint16_t    over_volt_derating_enable;                  //0x6229 设置过压降额使能 00:禁止 01:使能
    uint16_t    over_freq_derating_enable;                  //0x622A 设置过频降额使能 00:禁止 01:使能
    uint16_t    under_volt_derating_enable;                 //0x622B 设置欠压降额使能 00:禁止 01:使能
    uint16_t    under_freq_derating_enable;                 //0x622C 设置欠频降额使能 00:禁止 01:使能
    uint16_t    island_enable;                              //0x622D 设置孤岛使能    00:禁止 01:使能
    uint16_t    ac_coupling_enable;                         //0x622E 设置AC耦合使能  00:禁止 01:使能
    uint16_t    high_low_crossover_enable;                  //0x622F 设置高低穿使能  00:禁止 01:使能
    uint16_t    cto_meter;                                  //0x6230 设置CT或电表   00:CT 01:meter
    uint16_t    meter_brand;                                //0x6231 设置电表品牌   详细见协议 不常用变量
    uint16_t    ct_direction;                               //0x6232 设置CT方向   00:正方向 01:负方向
    uint16_t    anti_reverse_flow_threshold;                //0x6233 设置防逆流阈值(不允许并网) 0 - 5000W
    uint16_t    remote_shutdown_status;                     //0x6234 设置远程开关机 00:正常 01:关机 02:开机
    uint16_t    reactive_power_setting_value;               //0x6235 设置无功功率值
    uint16_t    reactive_power_ctrl_method;                 //0x6236 设置无功控制方式 00:按照功率因数控制 01:按照无功功率控制 02:按照QU曲线控制 03:按照QP曲线控制
    uint16_t    power_factor_setting_value;                 //0x6237 设置功率因数值 80-100 无功为负,超前功因 0.8-1  默认:100-(80-100)    无功为正,滞后功因 0.8-1  默认:100
    uint16_t    pv_switch;                                  //0x6238 设置PV切换 00:禁止 01:使能
    uint16_t    pf_switch;                                  //0x6239 设置PF切换 00:禁止 01:使能
    uint16_t    on_grid_power_soft_limit_en;                //0x623A 设置市电功率软件限制 00:禁止 01:使能
    uint16_t    on_grid_power_hard_limit_en;                //0x623B 设置市电功率硬件限制 00:禁止 01:使能


    uint16_t    reserve1[20];                               //0x623C 预留
    uint16_t    batt_dsg_volt_point;                        //0x6250 设置电池放电截止电压   0.1V 范围 43.2 - 57.6V
    uint16_t    batt_chg_volt_point;                        //0x6251 设置电池充电电压保护电  0.1V 范围 43.2 - 57.6V
    uint16_t    pv_uvp_point;                               //0x6252 设置PV输入欠压保护点   0.1V
    uint16_t    pv_ovp_point;                               //0x6253 设置PV输入过压保护点   0.1V
    uint16_t    active_power_setting_value;                 //0x6254 设置有功设置值   0% - 100%
    uint16_t    grid_volt_low_limit;                        //0x6255 设置电网电压下限  0.1V
    uint16_t    grid_volt_upper_limit;                      //0x6256 设置电网电压上限  0.1V
    uint16_t    grid_freq_low_limit;                        //0x6257 设置电网频率下限  0.01HZ
    uint16_t    grid_freq_upper_limit;                      //0x6258 设置电网频率上限  0.01HZ
    uint16_t    bypass_volt_low_limit;                      //0x6259 设置电网频率下限  0.01HZ
    uint16_t    bypass_volt_upper_limit;                    //0x625A 设置旁路电压上限  0.1V
    uint16_t    bypass_freq_low_limit;                      //0x625B 设置旁路频率下限  0.01HZ
    uint16_t    bypass_freq_upper_limit;                    //0x625C 设置旁路频率上限  0.01HZ
    uint16_t    over_volt_derating_point;                   //0x625D 设置过压降额点    1%
    uint16_t    over_freq_derating_point;                   //0x625E 设置过频降额点    1%
    uint16_t    under_volt_derating_point;                  //0x625F 设置欠压降额点(充电)  1%
    uint16_t    under_freq_derating_point;                  //0x6260 设置欠频降额点(充电)  1%、
    uint16_t    grid_ov_point;                              //0x6261 设置电网过压保护 1%
    uint16_t    inv_uvp_point;                              //0x6262 设置逆变低压保护点 1%
    uint16_t    low_freq_protect_point;                     //0x6263 设置低频保护点 1%
    uint16_t    over_freq_protect_point;                    //0x6264 设置过频保护点 1%
    uint16_t    on_grid_reconnect_time;                     //0x6265 设置并网重连时间(S)  5 - 300
    uint16_t    on_grid_power;                              //0x6266 设置并网最大功率(W)  0 - 5000
    uint16_t    reactive_power_respones_time;               //0x6267 设置无功响应时间(S)
    uint16_t    reset_factory;                              //0X6268 设置恢复出厂设置 写入 0X5AA5 则恢复出厂设置


    uint16_t    reserved2[23];                              //0x6269 - 627f 预留
    uint16_t    r_phase_volt_calibration_k_parm;            //0x6280 R(U/A)相电压校准系数K: 默认值 4096
    uint16_t    r_phase_volt_calibration_b_parm;            //0x6281 R(U/A)相电压校准系数B: 默认值 4096
    uint16_t    s_phase_volt_calibration_k_parm;            //0x6282 S(V/B)相电压校准系数K: 默认值 4096
    uint16_t    s_phase_volt_calibration_b_parm;            //0x6283 S(V/B)相电压校准系数B: 默认值 4096
    uint16_t    t_phase_volt_calibration_k_parm;            //0x6284 T(W/C)相电压校准系数K: 默认值 4096
    uint16_t    t_phase_volt_calibration_b_parm;            //0x6285 T(W/C)相电压校准系数B: 默认值 4096
    uint16_t    r_phase_cur_calibration_k_parm;             //0x6286 R(U/A)相电流校准系数K: 默认值 4096
    uint16_t    r_phase_cur_calibration_b_parm;             //0x6287 R(U/A)相电流校准系数B: 默认值 4096
    uint16_t    s_phase_cur_calibration_k_parm;             //0x6288 S(V/B)相电流校准系数K: 默认值 4096
    uint16_t    s_phase_cur_calibration_b_parm;             //0x6289 S(V/B)相电流校准系数B: 默认值 4096
    uint16_t    t_phase_cur_calibration_k_parm;             //0x628A T(W/C)相电流校准系数K: 默认值 4096
    uint16_t    t_phase_cur_calibration_b_parm;             //0x628B T(W/C)相电流校准系数B: 默认值 4096


    uint16_t    pv_ch1_curr_k_calibrate;                    //0x628C PV1电流校准系数K: 默认值 4096
    uint16_t    pv_ch1_curr_b_calibrate;                    //0x628D PV1电流校准系数B: 默认值 4096
    uint16_t    pv_ch2_curr_k_calibrate;                    //0x628E PV2电流校准系数K: 默认值 4096
    uint16_t    pv_ch2_curr_b_calibrate;                    //0x628F PV2电流校准系数B: 默认值 4096
    uint16_t    pv_ch3_curr_k_calibrate;                    //0x6290 PV3电流校准系数K: 默认值 4096
    uint16_t    pv_ch3_curr_b_calibrate;                    //0x6291 PV3电流校准系数B: 默认值 4096
    uint16_t    pv_ch4_curr_k_calibrate;                    //0x6292 PV4电流校准系数K: 默认值 4096
    uint16_t    pv_ch4_curr_b_calibrate;                    //0x6293 PV4电流校准系数B: 默认值 4096

    uint16_t    pv_ch1_volt_k_calibrate;                    //0x6294 PV1电压校准系数K: 默认值 4096
    uint16_t    pv_ch1_volt_b_calibrate;                    //0x6295 PV1电压校准系数B: 默认值 4096
    uint16_t    pv_ch2_volt_k_calibrate;                    //0x6296 PV2电压校准系数K: 默认值 4096
    uint16_t    pv_ch2_volt_b_calibrate;                    //0x6297 PV2电压校准系数B: 默认值 4096
    uint16_t    pv_ch3_volt_k_calibrate;                    //0x6298 PV3电压校准系数K: 默认值 4096
    uint16_t    pv_ch3_volt_b_calibrate;                    //0x6299 PV3电压校准系数B: 默认值 4096
    uint16_t    pv_ch4_volt_k_calibrate;                    //0x629A PV4电压校准系数K: 默认值 4096
    uint16_t    pv_ch4_volt_b_calibrate;                    //0x629B PV4电压校准系数B: 默认值 4096
    uint16_t    batt_volt_k_calibrate;                      //0x629C 电池电压校准系数K: 默认值 4096
    uint16_t    batt_volt_b_calibrate;                      //0x629D 电池电压校准系数B: 默认值 4096
    uint16_t    batt_curr_k_calibrate;                      //0x629E 电池电流校准系数K: 默认值 4096
    uint16_t    batt_curr_b_calibrate;                      //0x629F 电池电流校准系数B: 默认值 4096
    uint16_t    reserved3[24];                              //0x62A0 预留

    uint16_t    batt_volt_to_dsp;                           //0X62B8 设置唤醒电池电压 0.1V
    uint16_t    dsp_wake_en;                                //0X62B9 设置激活使能 01:PV激活使能 02:PV激活完成 03:正常功能
    uint16_t    pv_max_chg_power;                           //0X62BA 设置PV最大功率 单位 W
    uint16_t    max_inv_power;                              //0X62BB 设置最大逆变功率 单位W
    uint16_t    pcs_dsg_ctrl;                               //0x62BC 设置放电控制 00:监控卡异常禁止放电 01:允许放电 02:禁止放电
    uint16_t    pcs_chg_ctrl;                               //0x62BD 设置充电控制 00:监控卡异常禁止充电 01:允许充电 02:禁止充电
    uint16_t    pcs_user_type;                              //0x62BE 设置用户类型 00:普通用户 01:管理员 02:安装调试 03:内部
    uint16_t    ignor_ct_err;                               //0x62BF 设置消除CT故障 00:不消除CT告警 01:消除CT告警
    uint16_t    sys_upgrade_status;                         //0x62C0 下发当前系统状态 (BIT0: 00:未升级 01:升级中 02)(BIT1: 00:DSP未复位 01:DSP复位 02)(BIT2:监控卡测试未付状态位)(BIT3: 00:监控卡未复位 01: 监控卡复位中)
    uint16_t    drm_state;                                  //0:禁止   1：使能  BIT0:DRM0  BIT1:DRM1  BIT2:DRM2  BIT3:DRM3  BIT4:DRM4
    uint16_t    reserved4[62];                              //0x62C1 预留
} UdpDspHoldPara_t;


typedef struct _udp_pcs_para_stru_t
{
    UdpDspOutputPara_t     udp_pcs_upload;
    UdpDspHoldPara_t       udp_pcs_hold;
} UdpPCSPara_t;


typedef struct _udp_pcs_batt_info_stru_t
{
    uint16_t    set_batt_chg_volt;                          //0.1V
    uint16_t    set_batt_chg_curr;                          //0.01A
    uint16_t    set_batt_chg_power;                         //1W

    uint16_t    set_batt_dsg_volt;                          //
    uint16_t    set_batt_dsg_curr;                          //
    uint16_t    set_batt_dsg_power;                         //
} PcsBattSetInfo;

extern UdpPCSPara_t  gstUdpPcsPara;

void gstMonitorDataUpload(void);

void GetCH390MacAddress(void);
void Send_ARP_Protocol(void);

void EthernetProcess(void);
void NetworkInit(void);
void UpLoadStatus(uint8_t state);
uint16_t UpgradeRegRead(void);
bool MeterModeGet(void);
uint16_t MeterBrandGet(void);
bool GetEthSysUpgradeState(void);
#endif /* APP_CH390_ANALYSIS_H_ */
