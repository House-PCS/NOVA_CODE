#ifndef __SLAVE_PCS_H_
#define __SLAVE_PCS_H_

#include "dev_modbus.h"
#include "bsp.h"
#include <stdio.h>

#include "dev_udp_comm.h"
#include "udp.h"
#include <string.h>

#define     UDP_SLAVE_TYPE_PCS  (0x01)

#define     DSP_READ_BASE_ADDR  (0X00000000)



#define SLAVE_ARC_ADDR                                          0X05


#define PCS_CHG_CTRL_OFF                                        2          
#define PCS_CHG_CTRL_ON                                         1
#define PCS_CHG_CTRL_ERR                                        0
#define PCS_DSG_CTRL_OFF                                        2          
#define PCS_DSG_CTRL_ON                                         1
#define PCS_DSG_CTRL_ERR                                        0


#define ARC_REG_HOLD_ARC_DETECT_INSULATION_EN_ADDR              0X10C
#define ARC_REG_HOLD_ARC_SYS_UPGRADE_STATUS_ADDR                0X10D


#define PCS_REG_CLEAN_CURRENT_CAP_ADDR                          0x6299

#pragma pack(2)

typedef struct _arc_input_para_stru_t
{
    uint16_t    mode_id;                    //X0000            
    uint16_t    sn[10];                 
    uint16_t    hw_version;                  
    uint16_t    soft_version;  //
    uint16_t    arc_res_err_code;//
    uint16_t    reserve[1];
    uint16_t    channel_num;
    uint16_t    channle_curr_value[8];
    uint16_t    arc_alarm_info;     //0x0018  
    uint16_t    real_time_arc_alarm_info; //0x0019
    uint16_t    channel_arc_strength[8];
    uint16_t    channel_arc_history_max_value[8];
    uint16_t    arc_selftest_result;
    uint16_t    reserve1[5];
    uint16_t    channel_input_volt[8];
    uint16_t    channel_rx[8];
    uint16_t    channel_ry[8];
    uint16_t    impedance_alarm_info;           //0x0048
    uint16_t    inpedance_breakdown_info;       //0x0049 
}ARCInputPara_t;

typedef struct _arc_hold_para_stru_t
{
    uint16_t    slave_addr;                             //0X100
    uint16_t    comm_baud;
    uint16_t    arc_selftest_cmd;
    uint16_t    arc_alarm_threshold[8];
    uint16_t    arc_sensitivity;                        //0X10B
    uint16_t    arc_insulation_detect_en;               //0X10C
    uint16_t    sys_upgrade_status;                     //0X10D
    uint16_t    reserved[2];
    uint16_t    impedance_alarm_threshold_rx[8];
    uint16_t    impedance_alarm_threshold_ry[8];        
}ARCHoldPara_t;

typedef struct _arc_para_stru_t
{
    ARCInputPara_t          arc_input_para;
    ARCHoldPara_t           arc_hold_para;  
} ARCPara_t;


//typedef struct _modbus_pcs_arc_para_stru_t
//{
//    PCSPara_t                       pcs_para;
////    ARCPara_t                       arc_para;
//    uint8_t                         pcs_comm_err_cnt;
////    uint8_t                         arc_comm_err_cnt;
//    uint8_t                         pcs_comm_time_out;
////    uint8_t                         arc_comm_time_out;
//} ModBusPcsArcPara_t;




#pragma pack()

//extern ModBusPcsArcPara_t      gstPcsArcSlavePara;

#define PCS_REG_BATT_CHG_CURR_ADDR                              0X6223
#define PCS_REG_BATT_DSG_CURR_ADDR                              0X6226
#define PCS_REG_SET_CHG_INPUT_TYPE_ADDR                         0X6209 //?????
#define PCS_REG_SELECT_METER_BRAND_ADDR                         0X6268


#define PCS_REG_SET_REMOTE_SHUTDOWN_STATUS_ADDR                 0X6290
#define PCS_REG_OIL_MACHINE_POWRER_ADDR                         0X622F
#define PCS_REG_SET_PCS_DSG_CTRL_ADDR                           0X6294
#define PCS_REG_SET_PCS_CHG_CTRL_ADDR                           0X6295
#define PCS_REG_SET_PCS_USER_TYPE_ADDR                          0X6296
#define PCS_REG_SYS_UPGRADE_STATUS_ADDR                         0X6298
#define PCS_REG_RESET_FACTORY_ADDR                              0X6276
#define PCS_REG_BATT_VOLT_TO_DSP_ADDR                           0X62B8//激活的时候 讲电池电压设置给DSP   ?????
#define PCS_REG_MAX_PV_CHG_POWER_ADDR                           0X6229
#define PCS_REG_SET_CT_ENTER_TEST_ADDR                          0X6297 
#define PCS_REG_SHUTDOWN_ADDR                                   0X629A
#define PCS_REG_DRM_STATUS_ADDR                                 0X625F
#define PCS_REG_LLC_ENABLE_ADDR                                 0X6260     

#define PCS_REG_FRONT_BASE_ADDR                                 0X6000
#define PCS_REG_FRONT_END_ADDR                                  0X621F
#define PCS_REG_IN_FRONT_MAX_NUM                                (PCS_REG_FRONT_END_ADDR - PCS_REG_FRONT_BASE_ADDR+1)  

#define PCS_REG_HOLD_BASE_ADDR                                  0X6220
#define PCS_REG_HOLD_END_ADDR                                   0X63FF  
#define PCS_REG_HOLD_MAX_REG_NUM                                (PCS_REG_HOLD_END_ADDR - PCS_REG_HOLD_BASE_ADDR + 1)

/*拉弧和协处理器一起读*/
#define ARC_REG_BASE_ADDR                                       0X6400
#define PCS_COP_REG_END_ADDR                                    0X64FF   
#define ARC_REG_MAX_REG_NUM                                     (PCS_COP_REG_END_ADDR - ARC_REG_BASE_ADDR + 1)


/*******************************************************************************/
/*****************************PCS结构体**************************************/
#pragma pack(2)
/*----PCS基础信息---*/
typedef struct
{
    uint16_t    model_id;               //6000
    uint16_t    sn[10];                 //6001~600A
    uint16_t    hw_ver;                 //600B
    uint16_t    sw_ver[2];              //600C~600D
    uint16_t    sw_ver_ext[12];         //600E~6019
    uint16_t    reserve[6];             //601D~601F
} PcsBaseInfo_t;

/*----预留基础信息---*/
typedef struct
{
    uint16_t    model_id;               //6040
    uint16_t    sn[10];                 //6041~604A
    uint16_t    hw_ver;                 //604B
    uint16_t    sw_ver[2];              //604C~604D
    uint16_t    sw_ver_ext[12];         //604E~6059
    uint16_t    reserve[6];             //605D~605F
} ReserveBaseInfo_t;

/*----PCS模块状态信息---*/
typedef struct
{
    uint16_t    interface_state;        //6060         接口信息
    uint16_t    inv_run_state;          //6061   
    uint16_t    inv_on_grid_state;      //6062         
    uint16_t    dcdc_run_state;         //6063  
    uint16_t    drm_state;              //6064    
    uint16_t    mppt_run_state[4];      //6065~6058 
    uint16_t    reserve[7];             //6069~605F
    uint16_t    wakeup_status;          //6070
    int16_t     inv_radiator_temp;      //6071
    int16_t     inv_box_temp;           //6072
    int16_t     pv_temp;                //6073
    uint16_t    reserve2[12];           //6074~607F
} PcsModelStatusInfo_t;

/*----PCS电量信息---*/
typedef struct
{
    uint16_t    pv_elec_total[2];              //6080~6081
    uint16_t    buy_elec_total[2];             //6082~6083
    uint16_t    sell_elec_total[2];            //6084~6085
    uint16_t    bat_elec_chg_total[2];         //6086~6087
    uint16_t    bat_elec_dsg_total[2];         //6088~6089
    uint16_t    eps_elec_total[2];             //608A~608B
    uint16_t    remote_load_elec_total[2];     //608C~608D
    uint16_t    gen_elec_total[2];             //608E~608F
    uint16_t    ev_elec_chg_total[2];          //6090~6091
    uint16_t    ev_elec_dsg_total[2];          //6092~6093
    uint16_t    reserve[4];                    //6094~6097
    uint16_t    single_pv_elec_total[8];       //6098~609F
    uint16_t    reserve2[16];                  //60A0~60AF
} PcsElecStatsInfo_t;

/*----PCS功率信息---*/
typedef struct
{
    uint16_t    r_grid_vol;                     //R相电网电压            //60B0
    int16_t     r_meter_current;                //R相电表电流            //60B1
    uint16_t    r_grid_freq;                    //R相电网频率            //60B2
    int16_t     r_meter_act_power[2];           //R相电表有功功率        //60B3~60B4
    int16_t     r_meter_react_power[2];         //R相电表无功功率        //60B5~60B6
    int16_t     r_meter_apparent_power[2];      //R相电表视在功率        //60B7~60B8
    uint16_t    s_grid_vol;                     //S相电网电压            //60B9
    int16_t     s_meter_current;                //S相电表电流            //60BA
    uint16_t    s_grid_freq;                    //S相电网频率            //60BB
    int16_t     s_meter_act_power[2];           //S相电表有功功率        //60BC~60BD
    int16_t     s_meter_react_power[2];         //S相电表无功功率        //60BE~60BF
    int16_t     s_meter_apparent_power[2];      //S相电表视在功率        //60C0~60C1
    uint16_t    t_grid_vol;                     //T相电网电压            //60C2
    int16_t     t_meter_current;                //T相电表电流            //60C3
    uint16_t    t_grid_freq;                    //T相电网频率            //60C4
    int16_t     t_meter_act_power[2];           //T相电表有功功率        //60C5~60C6
    int16_t     t_meter_react_power[2];         //T相电表无功功率        //60C7~60C8
    int16_t     t_meter_apparent_power[2];      //T相电表视在功率        //60C9~60CA
        
    uint16_t    r_on_grid_vol;                  //R相并网口电压           //60CB
    int16_t     r_on_grid_current;              //R相并网口电流           //60CC
    uint16_t    r_on_grid_freq;                 //R相并网口频率           //60CD
    int16_t     r_on_grid_act_power[2];         //R相并网口有功功率       //60CE~60CF
    int16_t     r_on_grid_react_power[2];       //R相并网口无功功率       //60D0~60D1
    int16_t     r_on_grid_apparent_power[2];    //R相并网口视在功率       //60D2~60D3
    uint16_t    s_on_grid_vol;                  //S相并网口电压           //60D4
    int16_t     s_on_grid_current;              //S相并网口电流           //60D5
    uint16_t    s_on_grid_freq;                 //S相并网口频率           //60D6
    int16_t     s_on_grid_act_power[2];         //S相并网口有功功率       //60D7~60D8
    int16_t     s_on_grid_react_power[2];       //S相并网口无功功率       //60D9~60DA
    int16_t     s_on_grid_apparent_power[2];    //S相并网口视在功率       //60DB~60DC
    uint16_t    t_on_grid_vol;                  //T相并网口电压           //60DD
    int16_t     t_on_grid_current;              //T相并网口电流           //60DE
    uint16_t    t_on_grid_freq;                 //T相并网口频率           //60DF
    int16_t     t_on_grid_act_power[2];         //T相并网口有功功率       //60E0~60E1
    int16_t     t_on_grid_react_power[2];       //T相并网口无功功率       //60E2~60E3
    int16_t     t_on_grid_apparent_power[2];    //T相并网口视在功率       //60E4~60E5
        
    uint16_t    r_gen_vol;                      //R相发电机电压           //60E6
    int16_t     r_gen_current;                  //R相发电机电流           //60E7
    uint16_t    r_gen_freq;                     //R相发电机频率           //60E8
    int16_t     r_gen_act_power[2];             //R相发电机有功功率       //60E9~60EA
    int16_t     r_gen_react_power[2];           //R相发电机无功功率       //60EB~60EC
    int16_t     r_gen_apparent_power[2];        //R相发电机视在功率       //60ED~60EE
    uint16_t    s_gen_vol;                      //S相发电机电压           //60EF
    int16_t     s_gen_current;                  //S相发电机电流           //60F0
    uint16_t    s_gen_freq;                     //S相发电机频率           //60F1
    int16_t     s_gen_act_power[2];             //S相发电机有功功率       //60F2~60F3
    int16_t     s_gen_react_power[2];           //S相发电机无功功率       //60F4~60F5
    int16_t     s_gen_apparent_power[2];        //S相发电机视在功率       //60F6~60F7
    uint16_t    t_gen_vol;                      //T相发电机电压           //60F8
    int16_t     t_gen_current;                  //T相发电机电流           //60F9
    uint16_t    t_gen_freq;                     //T相发电机频率           //60FA
    int16_t     t_gen_act_power[2];             //T相发电机有功功率       //60FB~60FC
    int16_t     t_gen_react_power[2];           //T相发电机无功功率       //60FD~60FE
    int16_t     t_gen_apparent_power[2];        //T相发电机视在功率       //60FF~6100
        
    uint16_t    r_backup_vol;                   //R相负载电压             //6101
    int16_t     r_backup_current;               //R相负载电流             //6102
    uint16_t    r_backup_freq;                  //R相负载频率             //6103
    int16_t     r_backup_act_power[2];          //R相负载有功功率         //6104~6105
    int16_t     r_backup_react_power[2];        //R相负载无功功率         //6106~6107
    int16_t     r_backup_apparent_power[2];     //R相负载视在功率         //6108~6109
    uint16_t    s_backup_vol;                   //S相负载电压             //610A
    int16_t     s_backup_current;               //S相负载电流             //610B
    uint16_t    s_backup_freq;                  //S相负载频率             //610C
    int16_t     s_backup_act_power[2];          //S相负载有功功率         //610D~610E
    int16_t     s_backup_react_power[2];        //S相负载无功功率         //610F~6110
    int16_t     s_backup_apparent_power[2];     //S相负载视在功率         //6111~6112
    uint16_t    t_backup_vol;                   //T相负载电压            //6113
    int16_t     t_backup_current;               //T相负载电流            //6114
    uint16_t    t_backup_freq;                  //T相负载频率            //6115
    int16_t     t_backup_act_power[2];          //T相负载有功功率        //6116~6117
    int16_t     t_backup_react_power[2];        //T相负载无功功率        //6118~6119
    int16_t     t_backup_apparent_power[2];     //T相负载视在功率        //611A~611B
        
    uint16_t    r_nload_vol;                    //R相远端负载电压        //611C
    int16_t     r_nload_current;                //R相远端负载电流        //611D
    uint16_t    r_nload_freq;                   //R相远端负载频率        //611E
    int16_t     r_nload_act_power[2];           //R相远端负载有功功率    //611F~6120
    int16_t     r_nload_react_power[2];         //R相远端负载无功功率    //6121~6122
    int16_t     r_nload_apparent_power[2];      //R相远端负载视在功率    //6123~6124
    uint16_t    s_nload_vol;                    //S相远端负载电压        //6125
    int16_t     s_nload_current;                //S相远端负载电流        //6126
    uint16_t    s_nload_freq;                   //S相远端负载频率        //6127
    int16_t     s_nload_act_power[2];           //S相远端负载有功功率    //6128~6129
    int16_t     s_nload_react_power[2];         //S相远端负载无功功率    //612A~612B
    int16_t     s_nload_apparent_power[2];      //S相远端负载视在功率    //612C~612D
    uint16_t    t_nload_vol;                    //T相远端负载电压        //612E
    int16_t     t_nload_current;                //T相远端负载电流        //612F
    uint16_t    t_nload_freq;                   //T相远端负载频率        //6130
    int16_t     t_nload_act_power[2];           //T相远端负载有功功率    //6131~6132
    int16_t     t_nload_react_power[2];         //T相远端负载无功功率    //6133~6134
    int16_t     t_nload_apparent_power[2];      //T相远端负载视在功率    //6135~6136
    
    uint16_t    r_third_party_vol;              //R相三方逆变器电压      //6137
    int16_t     r_third_party_current;          //R相三方逆变器电流      //6138
    uint16_t    r_third_party_freq;             //R相三方逆变器频率      //6139
    int16_t     r_third_party_act_power[2];     //R相三方逆变器有功功率  //613A~613B
    int16_t     r_third_party_react_power[2];   //R相三方逆变器无功功率  //613C~613D
    int16_t     r_third_party_apparent_power[2];//R相三方逆变器视在功率  //613E~613F
    uint16_t    s_third_party_vol;              //S相三方逆变器电压      //6140  
    int16_t     s_third_party_current;          //S相三方逆变器电流      //6141  
    uint16_t    s_third_party_freq;             //S相三方逆变器频率      //6142  
    int16_t     s_third_party_act_power[2];     //S相三方逆变器有功功率  //6143~6144
    int16_t     s_third_party_react_power[2];   //S相三方逆变器无功功率  //6145~6146
    int16_t     s_third_party_apparent_power[2];//S相三方逆变器视在功率  //6147~6148
    uint16_t    t_third_party_vol;              //T相三方逆变器电压      //6149  
    int16_t     t_third_party_current;          //T相三方逆变器电流      //614A  
    uint16_t    t_third_party_freq;             //T相三方逆变器频率      //614B  
    int16_t     t_third_party_act_power[2];     //T相三方逆变器有功功率  //614C~614D
    int16_t     t_third_party_react_power[2];   //T相三方逆变器无功功率  //614E~614F
    int16_t     t_third_party_apparent_power[2];//T相三方逆变器视在功率  //6150~6151
    
    uint16_t    r_inv_vol;                      //R相逆变电压            //6152
    int16_t     r_inv_current;                  //R相逆变电流            //6153
    uint16_t    r_inv_freq;                     //R相逆变频率            //6154
    int16_t     r_inv_act_power[2];             //R相逆变有功功率        //6155~6156
    int16_t     r_inv_react_power[2];           //R相逆变无功功率        //6157~6158
    int16_t     r_inv_apparent_power[2];        //R相逆变视在功率        //6159~615A
    uint16_t    s_inv_vol;                      //S相逆变电压            //615B  
    int16_t     s_inv_current;                  //S相逆变电流            //615C  
    uint16_t    s_inv_freq;                     //S相逆变频率            //615D  
    int16_t     s_inv_act_power[2];             //S相逆变有功功率        //615E~615F
    int16_t     s_inv_react_power[2];           //S相逆变无功功率        //6160~6161
    int16_t     s_inv_apparent_power[2];        //S相逆变视在功率        //6162~6163
    uint16_t    t_inv_vol;                      //T相逆变电压            //6164  
    int16_t     t_inv_current;                  //T相逆变电流            //6165  
    uint16_t    t_inv_freq;                     //T相逆变频率            //6166  
    int16_t     t_inv_act_power[2];             //T相逆变有功功率        //6167~6168
    int16_t     t_inv_react_power[2];           //T相逆变无功功率        //6169~616A
    int16_t     t_inv_apparent_power[2];        //T相逆变视在功率        //616B~616C
    
    uint16_t    reserve[37];                                             //616D~6191
    
    uint16_t    inv_bat_vol;                    //电池侧电压             //6192
    int16_t     inv_bat_current;                //电池侧电流             //6193
    int16_t     inv_bat_power[2];               //电池侧频率             //6194~6195                         
    uint16_t    inv_ev_vol;                     //EV侧电压               //6196
    int16_t     inv_ev_current;                 //EV侧电流               //6197
    int16_t     inv_ev_power[2];                //EV侧频率               //6198~6199
    uint16_t    pv_1_vol;                       //PV1电压                //619A
    int16_t     pv_1_current;                   //PV1电流                //619B
    int16_t     pv_1_power[2];                  //PV1频率                //619C~619D
    uint16_t    pv_2_vol;                       //PV2电压                //619E
    int16_t     pv_2_current;                   //PV2电流                //619F
    int16_t     pv_2_power[2];                  //PV2频率                //61A0~61A1
    uint16_t    pv_3_vol;                       //PV3电压                //61A2
    int16_t     pv_3_current;                   //PV3电流                //61A3
    int16_t     pv_3_power[2];                  //PV3频率                //61A4~61A5
    uint16_t    pv_4_vol;                       //PV4电压                //61A6
    int16_t     pv_4_current;                   //PV4电流                //61A7
    int16_t     pv_4_power[2];                  //PV4频率                //61A8~61A9
    uint16_t    reserve2[13];                                            //61AA~61B6
    uint16_t    bus_vol;                        //母线电压               //61B7
    uint16_t    phase_seq;                      //市电相序               //61B8
    uint16_t    reserve4[7];                                             //61B9~61BF
} PcsPowerInfo_t;

/*----PCS故障信息---*/
typedef struct
{
    uint16_t    grid_error;           //61C0
    uint16_t    pv_error[2];          //61C1~61C2
    uint16_t    bus_error;            //61C3
    uint16_t    inv_error;            //61C4
    uint16_t    load_error;           //61C5
    uint16_t    temp_error;           //61C6
    uint16_t    gen_error;            //61C7
    uint16_t    other_error;          //61C8
    uint16_t    reserve_error;        //61C9
    
    uint16_t    grid_alarm;           //61CA
    uint16_t    pv_alarm[2];          //61CB~61CC
    uint16_t    bus_alarm;            //61CD
    uint16_t    inv_alarm;            //61CE
    uint16_t    load_alarm;           //61CF
    uint16_t    temp_alarm;           //61D0
    uint16_t    gen_alarm;            //61D1
    uint16_t    other_alarm;          //61D2
    uint16_t    reserve_alarm;        //61D3
    
    uint16_t    grid_notice;          //61D4
    uint16_t    pv_notice[2];         //61D5~61D6
    uint16_t    bus_notice;           //61D7
    uint16_t    inv_notice;           //61D8
    uint16_t    load_notice;          //61D9
    uint16_t    temp_notice;          //61DA
    uint16_t    gen_notice;           //61DB
    uint16_t    other_notice;         //61DC
    uint16_t    reserve_notice[3];    //61DD~61DF

} PcsErrorInfo_t;

/*----PCS预留信息---*/
typedef struct
{
    uint16_t    reserve[48];    //61F0~621F
} PcsReserveInfo_t;

/*----PCS配置信息---*/
typedef struct
{
    uint16_t    second_id;                      //6220
    uint16_t    work_mode;                      //6221
    uint16_t    gen_mode;                       //6222
    uint16_t    inv_bat_max_chg_current;        //6223
    uint16_t    inv_bat_max_chg_power[2];       //6224~6225
    uint16_t    inv_bat_max_dsg_current;        //6226
    uint16_t    inv_bat_max_dsg_power[2];       //6227~6228
    uint16_t    inv_pv_max_power[2];            //6229~622A
    uint16_t    inv_on_grid_in_max_power[2];    //622B~622C
    uint16_t    inv_on_grid_out_max_power[2];   //622D~622E
    uint16_t    inv_gen_max_power[2];           //622F~6230
    uint16_t    inv_ev_in_max_power[2];         //6231~6232
    uint16_t    inv_ev_out_max_power[2];        //6233~6234
    uint16_t    network_max_power_limit[2];     //6235~6236
    uint16_t    on_grid_max_power[2];           //6237~6238
    uint16_t    reserve[23];                    //6239~624F
    uint16_t    pv_input_mode;                  //6250
    uint16_t    prohibit_reverse_flow_enable;   //6251
    uint16_t    reserve2[2];                    //6252~6253
    uint16_t    x_boost_enable;                 //6254
    uint16_t    ups_0_delay_enable;             //6255
    uint16_t    backup_enable;                  //6256
    uint16_t    program_ctrl_enable;            //6257
    uint16_t    over_vol_derating_enable;       //6258
    uint16_t    over_freq_derating_enable;      //6259
    uint16_t    under_vol_derating_enable;      //625A
    uint16_t    under_freq_derating_enable;     //625B
    uint16_t    island_enable;                  //625C
    uint16_t    ac_coupling_enable;             //625D
    uint16_t    high_low_crossover_enable;      //625E
    uint16_t    drm_enable;                     //625F
    uint16_t    llc_enable;                     //6260
    uint16_t    reserve3[5];                    //6261~6265
    uint16_t    ct_or_meter;                    //6266
    uint16_t    ct_dir;                         //6267
    uint16_t    meter_brand;                    //6268
    uint16_t    grid_standard;                  //6269
    uint16_t    grid_sub_standard;              //626A
    uint16_t    bat_chg_max_soc;                //626B
    uint16_t    on_grid_bat_dsg_min_soc;        //626C
    uint16_t    off_grid_bat_dsg_min_soc;       //626D
    uint16_t    reserve4[8];                    //626E~6275
    uint16_t    inv_factory_reset;              //6276
    uint16_t    on_grid_reconnect_time;         //6277
    uint16_t    reactive_response_time;         //6278
    int16_t     reactive_set;                   //6279
    uint16_t    reactive_ctrl_type;             //627A
    uint16_t    active_set;                     //627B
    int16_t     power_factor_set;               //627C
    uint16_t    reserve5[3];                    //627D~627F
    uint16_t    certification_test_switch;      //6280
    uint16_t    reserve6[15];                   //6281~638F
} PcsSetInfo_t;

/*----PCS控制信息---*/
typedef struct
{
    uint16_t    remote_shutdown;                //6290
    uint16_t    pv_switch;                      //6291
    uint16_t    pf_switch;                      //6292
    uint16_t    pv_wakeup_status;               //6293
    uint16_t    dsg_ctrl;                       //6294
    uint16_t    chg_ctrl;                       //6295
    uint16_t    user_type;                      //6296
    uint16_t    ignore_ct_error;                //6297
    uint16_t    upgrade_status;                 //6298
    uint16_t    elec_clear;                     //6299  
    uint16_t    shutdown_signal;                //629A
    uint16_t    reserve[21];                    //629B~62AF
} PcsControlInfo_t;

/*----功率阈值信息---*/
typedef struct
{
    uint16_t    bat_under_vol_threshold;        //62B0
    uint16_t    bat_over_vol_threshold;         //62B1
    uint16_t    pv_under_vol_threshold;         //62B2
    uint16_t    pv_over_vol_threshold;          //62B3
    uint16_t    ev_under_vol_threshold;         //62B4
    uint16_t    ev_over_vol_threshold;          //62B5
    uint16_t    grid_under_vol_threshold;       //62B6
    uint16_t    grid_over_vol_threshold;        //62B7
    uint16_t    grid_under_freq_threshold;      //62B8
    uint16_t    grid_over_freq_threshold;       //62B9
    uint16_t    bypass_under_vol_threshold;     //62BA
    uint16_t    bypass_over_vol_threshold;      //62BB
    uint16_t    bypass_under_freq_threshold;    //62BC
    uint16_t    bypass_over_freq_threshold;     //62BD
    uint16_t    over_vol_derating_point;        //62BE
    uint16_t    over_freq_derating_point;       //62BF
    uint16_t    under_vol_derating_point;       //62C0
    uint16_t    under_freq_derating_point;      //62C1
    uint16_t    reserve[14];                   //62C2~62CF
} PcsLimitInfo_t;

/*----功率采样校准信息---*/
typedef struct
{
    uint16_t    calibration_reset;              //62D0
    uint16_t    reserve[4];                     //62D1~62D4
    uint16_t    inv_R_vol_calib_k;              //62D5
    uint16_t    inv_R_vol_calib_b;              //62D6
    uint16_t    inv_S_vol_calib_k;              //62D7
    uint16_t    inv_S_vol_calib_b;              //62D8
    uint16_t    inv_T_vol_calib_k;              //62D9
    uint16_t    inv_T_vol_calib_b;              //62DA
    uint16_t    inv_R_current_calib_k;          //62DB
    uint16_t    inv_R_current_calib_b;          //62DC
    uint16_t    inv_S_current_calib_k;          //62DD
    uint16_t    inv_S_current_calib_b;          //62DE
    uint16_t    inv_T_current_calib_k;          //62DF
    uint16_t    inv_T_current_calib_b;          //62E0
    uint16_t    inv_bat_vol_calib_k;            //62E1
    uint16_t    inv_bat_vol_calib_b;            //62E2
    uint16_t    inv_bat_current_calib_k;        //62E3
    uint16_t    inv_bat_current_calib_b;        //62E4
    uint16_t    inv_ev_vol_calib_k;             //62E5
    uint16_t    inv_ev_vol_calib_b;             //62E6
    uint16_t    inv_ev_current_calib_k;         //62E7
    uint16_t    inv_ev_current_calib_b;         //62E8
    uint16_t    inv_pv_1_vol_calib_k;           //62E9
    uint16_t    inv_pv_1_vol_calib_b;           //62EA
    uint16_t    inv_pv_1_current_calib_k;       //62EB
    uint16_t    inv_pv_1_current_calib_b;       //62EC
    uint16_t    inv_pv_2_vol_calib_k;           //62ED
    uint16_t    inv_pv_2_vol_calib_b;           //62EE
    uint16_t    inv_pv_2_current_calib_k;       //62EF
    uint16_t    inv_pv_2_current_calib_b;       //62F0
    uint16_t    inv_pv_3_vol_calib_k;           //62F1
    uint16_t    inv_pv_3_vol_calib_b;           //62F2
    uint16_t    inv_pv_3_current_calib_k;       //62F3
    uint16_t    inv_pv_3_current_calib_b;       //62F4
    uint16_t    inv_pv_4_vol_calib_k;           //62F5
    uint16_t    inv_pv_4_vol_calib_b;           //62F6
    uint16_t    inv_pv_4_current_calib_k;       //62F7
    uint16_t    inv_pv_4_current_calib_b;       //62F8
    uint16_t    reserve2[23];                   //62F9~630F
                                                
} PcsPowerCalibrateInfo_t;    

/*----PCS预留信息---*/
typedef struct
{
    uint16_t    reserve[240];    //6310~63FF
} PcsReserve2Info_t;

/*----Arc基础信息---*/
typedef struct
{
    uint16_t    model_id;               //6400
    uint16_t    sn[10];                 //6401~640A
    uint16_t    hw_ver;                 //640B
    uint16_t    sw_ver[2];              //640C~640D
    uint16_t    sw_ver_ext[12];         //640E~6419
    uint16_t    reserve[6];             //641A~641F
} ArcBaseInfo_t;

/*----Arc状态信息---*/
typedef struct
{
    uint16_t    arc_occur;              //6420
    uint16_t    ins_res_low_ocur;       //6421
    uint16_t    detect_sensitivity;     //6422
    uint16_t    ins_res_threshold;      //6423
    uint16_t    pv1_volt;               //6424
    uint16_t    pv2_volt;               //6425
    uint16_t    pv3_volt;               //6426
    uint16_t    pv4_volt;               //6427
    uint16_t    pv1_res_p;              //6428
    uint16_t    pv2_res_p;              //6429
    uint16_t    pv3_res_p;              //642A
    uint16_t    pv4_res_p;              //642B
    uint16_t    pv1_res_n;              //642C
    uint16_t    pv2_res_n;              //642D
    uint16_t    pv3_res_n;              //642E
    uint16_t    pv4_res_n;              //642F
    uint16_t    pv1_othreshold_num;     //6430  //过阈值总数
    uint16_t    pv2_othreshold_num;     //6431
    uint16_t    pv3_othreshold_num;     //6432
    uint16_t    pv4_othreshold_num;     //6433
    uint16_t    res_check_over_flag;    //6434
    uint16_t    reserve2[27];           //6435~644F
} ArcStatusInfo_t;

/*----Arc控制信息---*/
typedef struct
{
    uint16_t    arc_enable;             //6450
    uint16_t    ins_res_enable;         //6451
    uint16_t    reserve[14];            //6452~645F
    uint16_t    upgrade_status;         //6460
    uint16_t    reserve2[31];           //6461~647F
} ArcCtrlInfo_t;

/*----PCS协处理器基础信息---*/
typedef struct
{
    uint16_t    model_id;               //6480
    uint16_t    sn[10];                 //6481~648A
    uint16_t    hw_ver;                 //648B
    uint16_t    sw_ver[2];              //648C~648D
    uint16_t    sw_ver_ext[12];         //648E~6499
    uint16_t    reserve[6];             //649A~649F
} PcsCopBaseInfo_t;

/*----PCS协处理器状态信息---*/
typedef struct
{
    uint16_t    run_status;             //64A0
    uint16_t    reserve[47];            //64A1~64CF
} PcsCopStatusInfo_t;

/*----PCS协处理器控制信息---*/
typedef struct
{
    uint16_t    upgrade_status;         //64D0
    uint16_t    dsp_heart_beat;         //64D1
    uint16_t    mvu_rly_ctrl;           //64D2
    uint16_t    mvu_rly_state;          //64D3
    uint16_t    reserve[43];            //64D4~64FF
} PcsCopCtrlInfo_t;




/*----PCS信息---*/
typedef struct
{
    PcsBaseInfo_t               pcs_base_info       ;    //基础信息
    ReserveBaseInfo_t           reserve_base_info[2];    //预留基础信息
    PcsModelStatusInfo_t        model_status        ;    //模块状态信息
    PcsElecStatsInfo_t          elec_stats          ;    //电量统计
    PcsPowerInfo_t              power_info          ;    //功率信息
    PcsErrorInfo_t              error_info          ;    //故障信息
    PcsReserveInfo_t            reserve_info        ;    //预留信息
    PcsSetInfo_t                set_info            ;    //设置信息
    PcsControlInfo_t            control_info        ;    //控制信息
    PcsLimitInfo_t              limit_info          ;    //阈值信息
    PcsPowerCalibrateInfo_t     calibrate_info      ;    //校准信息
    PcsReserve2Info_t           reserve_2_info      ;    //预留信息2
    ArcBaseInfo_t               arc_base_info       ;    //拉弧基础信息
    ArcStatusInfo_t             arc_status_info     ;    //拉弧状态信息
    ArcCtrlInfo_t               arc_ctrl_info       ;    //拉弧控制信息
    PcsCopBaseInfo_t            pcs_cop_base_info   ;    //PCS协处理器基础信息
    PcsCopStatusInfo_t          pcs_cop_status_info ;    //PCS协处理器状态信息
    PcsCopCtrlInfo_t            pcs_cop_ctrl_info   ;    //PCS协处理器控制信息  
} PcsInfo_t;


/*******************************************************************************/
/*******************************************************************************/

typedef struct _udp_pcs_para_stru_t
{
    PcsInfo_t             pcs_info;
    uint8_t               pcs_comm_err_cnt;
    uint8_t               pcs_comm_time_out;
} UdpPCSPara_t;


typedef struct _udp_pcs_ctrl_stru_t
{
    uint8_t     pcs_comm_err_cnt;
    union   
    {
        struct
        {
            unsigned char recv_flag:1;
            
        }bit_value;
        uint16_t byte_value;
    }ctrl_flag;
    
} UdpPcsCtrl_t;
#pragma pack()




extern  UdpPCSPara_t  gstUdpPcsSlavePara;
void DspCommProc(void);
void DspUdpParaInit(void);
void SendDspUdpData(void);
void ColseDspUdpConnect(void);
void OpenDspUdpConnect(void);

bool SetPcsChgCtrlStatus(uint8_t chg_ctrl);
bool SetPcsDsgCtrlStatus(uint8_t dsg_ctrl);
bool SetPcsRemoteShutdown(uint16_t status);
bool SetPcsDsgVoltCurPowerInfo(uint16_t batt_max_dsg_curr, uint32_t batt_max_dsg_power);
bool SetPcsChgVoltCurPowerInfo(uint16_t, uint32_t);
bool SetPcsChgDsgCurPowerInfo(uint16_t max_chg_cur, uint32_t max_chg_power, uint16_t max_dsg_cur, uint32_t max_dsg_power);
bool SetPcsMaxPvPower(uint32_t max_pv_power, uint32_t max_inv_power);
bool SetChgInputType(uint8_t type);
bool SetPcsUserType(uint16_t user_type,uint8_t max_user_type_num);
bool SetPcsResetFactory(void);
bool SetBattVoltToDsp(uint16_t batt_volt,uint8_t dsp_wake_status);
bool SetPcsSysUpgradeStatus(uint16_t status);
bool SetPcsElecMeterType(uint16_t meter_type);
bool SetArcInsulationDetect(uint8_t status);
bool SetArcSysUpgradeStatus(uint8_t status);
bool CollectWritePcsFun(uint16_t reg_addr,uint16_t reg_num,uint8_t *pdata,uint8_t using_mode);
void GetPcsInputFrontData(uint8_t *pdata,uint16_t data_num);
void GetPcsHoldData(uint8_t *pdata,uint16_t data_num);
bool SetDspEnterCtTestMode(uint8_t status);
void SendByteDataToPcs(uint32_t data_addr,uint16_t data_len,uint8_t *pdata);
bool SetDspDrmStatus(uint16_t status);
bool SetDspLLCEnable(void);
bool SetPcsCapInfoToZero(uint16_t status);
bool SetPcsActivePowerVoltCtrl(uint16_t pv_switch);
bool SetPcsActivePowerFreqCtrl(uint16_t pf_switch);
bool PCSShutDownSend(void);
bool SetPcsBackEnCtrl(uint16_t backup_en);

#endif