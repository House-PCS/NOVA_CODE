/*
 * sys_info.h
 *
 *  Created on: 2025年3月20日
 *      Author: Nova006352
 */

#ifndef APP_SYS_INFO_H_
#define APP_SYS_INFO_H_

#include "F28x_Project.h"


typedef struct _upload_basic_info_stru_t
{
    /*0x6000-0x601f:逆变器基础信息，32字节*/
    uint16_t    mode_id;                            //0x6000
    uint16_t    sn[10];                             //0x6001-0x600A
    uint16_t    hw_version;                         //0x600b，硬件版本号：bit[15:8]:大版本，0~25代表A~Z，其余值预留。bit[7:0]:小版本，从0开始第一版的硬件版本就是A0
    uint16_t    software_ver[2];                    //0x600C-0x600D，固件版本号
    uint16_t    software_ver_Extend[12];           //0x600E-0x6019，固件版本号预留，12字节
    uint16_t    reserve1[6];                        //0x601A-0x601F，预留

    /*0x6020-0x603f:逆变器基础信息，32字节*/
    uint16_t    arc_mode_id;                         //0x6020
    uint16_t    arc_sn[10];                          //0x6021-0x602A
    uint16_t    arc_hw_version;                      //0x602B
    uint16_t    arc_software_ver[2];                 //0x602C-0x602D，固件版本号
    uint16_t    arc_software_ver_Extend[12];        //0x602E-0x6039，固件版本号预留，12字节
    uint16_t    reserve2[6];                         //0x603A-0x603F，预留

    /*0x6040-0x605f:基础信息预留，32字节*/
    uint16_t    reserve_basic_information[32];      //0x6040-0x605f，基础信息预留

} _upload_basic_info_t;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t grid_input         : 1;          // bit:00    市电输入
        uint16_t gen_input          : 1;          // bit:01    油机输入
        uint16_t pv1_input          : 1;          // bit:02    PV1输入
        uint16_t pv2_input          : 1;          // bit:03    PV2输入
        uint16_t pv3_input          : 1;          // bit:04    PV3输入
        uint16_t pv4_input          : 1;          // bit:05    PV4输入
        uint16_t reverse06_15       :10;
    } bit;
} SysInputState_t;


typedef struct _upload_module_status_t
{
    /*0x6060-0x607F:模块状态*/
    SysInputState_t    sys_input_stat;              //0x6060，PowerInterfaceState，功率接口状态: 每个bit代表一个接口
    
    uint16_t    inv_status;                         //0x6061，InvRunState，逆变器运行状态，逆变器运行状态:00H：待机01H：自检02H：混合并网03H：离网04H：旁路05H: 关机
    uint16_t    invOnGridState;                     //0x6062，并离网状态
    uint16_t    dcdc_status;                        //0x6063，DCDC状态

    uint16_t    DRMStatus;                          //0x6064

    uint16_t    mppt1_status;                       //0x6065
    uint16_t    mppt2_status;                       //0x6066
    uint16_t    mppt3_status;                       //0x6067
    uint16_t    mppt4_status;                       //0x6068
    uint16_t    Module_status_reserve[7];          //0x6069-0x606f,模块状态信息预留

    uint16_t    WakeUpStatus;                       //0x6070
//    uint16_t    SysUpgradeStatus;                   //0x6071

    int16_t     inverter_radiator_temp;             //0x6071，逆变器散热器温度
    int16_t     chassis_internal_temp;              //0x6072，逆变器箱体温度
    int16_t     pv_temp;                            //0x6073，PV温度

    int16_t     temp_reserve[12];                   //0x6074~0x607F，预留温度字段


} _upload_module_status_t;


typedef struct _upload_Capacity_info_stru_t
{
    /*0x6080-0x60AF: Power Capacity information:电量信息*/
    uint16_t    PV_ElectricGenTotal[2];           //Pv总发电量

    uint16_t    elec_purchase_consum_total[2]    ;//买电总电量 

    uint16_t    elec_sale_consum_total[2]        ;//卖电总电量

    uint16_t    Bat_ElectricChgTotal[2];        //电池总充电量

    uint16_t    Bat_ElectricDsgTotal[2];        //电池总放电量

    uint16_t    backup_total_sonsum_power[2]     ;//本地负载总的用电量    EPS_ElectricTotal 

    uint16_t    load_total_sonsum_power[2]       ;//远端负载总的用电量    KWH
    
    uint16_t    Gen_ElectricTotal[2];              //油机总发电量

    uint16_t    EV_ElectricChgTotal[2];            //充电桩给设备总充电量

    uint16_t    EV_ElectricDsgTotal[2];            //充电桩给设备总放电量

    uint16_t    Power_Information_reserve[4];       //电量信息预留
    
    uint16_t    PV1_ElectricGenTotal[2];               //Pv1总发电量

    uint16_t    PV2_ElectricGenTotal[2];               //Pv2总发电量

    uint16_t    PV3_ElectricGenTotal[2];               //Pv3总发电量

    uint16_t    PV4_ElectricGenTotal[2];               //Pv4总发电量

    uint16_t    pv_Capacity_reserve[16];               //pv预留

}_upload_Capacity_info_t;


typedef struct _upload_Power_info_stru_t
{
    /*0x60B0-0x61BF:Power_info，功率信息*/
    uint16_t    r_phase_grid_volt;              //R 相电网电压
    int16_t     r_phase_meter_curr;             //R 相电表电流
    uint16_t    r_phase_grid_freq;              //R 相电网频率
    int16_t     r_phase_meter_active_power[2];     //R 相电表有功功率    
    int16_t     r_phase_meter_reactive_power[2];   //R 相电表无功功率    
    int16_t     r_phase_meter_power[2];            //R 相电表视在功率

    uint16_t    s_phase_grid_volt;              //s 相电网电压
    int16_t     s_phase_meter_curr;             //s 相电表电流
    uint16_t    s_phase_grid_freq;              //s 相电网频率
    int16_t     s_phase_meter_active_power[2];     //s 相电表有功功率    
    int16_t     s_phase_meter_reactive_power[2];   //s 相电表无功功率    
    int16_t     s_phase_meter_power[2];            //s 相电表视在功率 
    
    uint16_t    t_phase_grid_volt;              //t 相电网电压
    int16_t     t_phase_meter_curr;             //t 相电表电流
    uint16_t    t_phase_grid_freq;              //t 相电网频率
    int16_t     t_phase_meter_active_power[2];     //t 相电表有功功率    
    int16_t     t_phase_meter_reactive_power[2];   //t 相电表无功功率    
    int16_t     t_phase_meter_power[2];            //t 相电表视在功率  
    
    
    uint16_t    r_phase_ongrid_volt;            //R 相并网口电压
    int16_t     r_phase_ongrid_curr;            //R 相并网口电流
    uint16_t    r_phase_ongrid_freq;             //R 相并网口频率
    int16_t     r_phase_ongrid_active_power[2]; //R 相并网口有功功率
    int16_t     r_phase_ongrid_reactive_power[2];//R 相并网口无功功率
    int16_t     r_phase_ongrid_power[2];        //R 相并网口视在功率

    uint16_t    s_phase_ongrid_volt;            //S 相并网口电压
    int16_t     s_phase_ongrid_curr;            //S 相并网口电流
    uint16_t    s_phase_ongrid_freq;             //S 相并网口频率
    int16_t     s_phase_ongrid_active_power[2]; //S 相并网口有功功率
    int16_t     s_phase_ongrid_reactive_power[2];//S 相并网口无功功率
    int16_t     s_phase_ongrid_power[2];        //S 相并网口视在功率    

    uint16_t    t_phase_ongrid_volt;            //t 相并网口电压
    int16_t     t_phase_ongrid_curr;            //t 相并网口电流
    uint16_t    t_phase_ongrid_freq;             //t 相并网口频率
    int16_t     t_phase_ongrid_active_power[2]; //t 相并网口有功功率
    int16_t     t_phase_ongrid_reactive_power[2];//t 相并网口无功功率
    int16_t     t_phase_ongrid_power[2];        //t 相并网口视在功率
    
    uint16_t    r_phase_oil_volt;               //R 相油机电压 
    int16_t     r_phase_oil_curr;               //R 相油机电流
    uint16_t    r_phase_oil_freq;               //R 相油机频率
    int16_t     r_phase_oil_active_power[2];       //R 相油机有功功率
    int16_t     r_phase_oil_reactive_power[2];     //R 相油机无功功率
    int16_t     r_phase_oil_power[2];              //R 相油机视在功率

    uint16_t    s_phase_oil_volt;               //R 相油机电压 
    int16_t     s_phase_oil_curr;               //R 相油机电流
    uint16_t    s_phase_oil_freq;               //R 相油机频率
    int16_t     s_phase_oil_active_power[2];       //R 相油机有功功率
    int16_t     s_phase_oil_reactive_power[2];     //R 相油机无功功率
    int16_t     s_phase_oil_power[2];              //R 相油机视在功率

    uint16_t    t_phase_oil_volt;               //R 相油机电压 
    int16_t     t_phase_oil_curr;               //R 相油机电流
    uint16_t    t_phase_oil_freq;               //R 相油机频率
    int16_t     t_phase_oil_active_power[2];       //R 相油机有功功率
    int16_t     t_phase_oil_reactive_power[2];     //R 相油机无功功率
    int16_t     t_phase_oil_power[2];              //R 相油机视在功率

    uint16_t    r_phase_backup_volt             ;//R相BACKUP电压
    int16_t     r_phase_backup_curr             ;//R相BACKUP电流
    uint16_t    r_phase_backup_freq              ;//R相BACKUP频率    /*新增 */
    int16_t     r_phase_backup_active_power[2]  ;//R相BACKUP有用功率
    int16_t     r_phase_backup_reactive_power[2];//R相BACKUP无用功率    
    int16_t     r_phase_backup_power[2]         ;//R相BACKUP视在功率

    uint16_t    s_phase_backup_volt             ;//s相BACKUP电压
    int16_t     s_phase_backup_curr             ;//s相BACKUP电流
    uint16_t    s_phase_backup_freq              ;//s相BACKUP频率    /*新增 */
    int16_t     s_phase_backup_active_power[2]  ;//s相BACKUP有用功率
    int16_t     s_phase_backup_reactive_power[2];//s相BACKUP无用功率    
    int16_t     s_phase_backup_power[2]         ;//s相BACKUP视在功率

    uint16_t    t_phase_backup_volt             ;//t相BACKUP电压
    int16_t     t_phase_backup_curr             ;//t相BACKUP电流
    uint16_t    t_phase_backup_freq              ;//t相BACKUP频率    /*新增 */
    int16_t     t_phase_backup_active_power[2]  ;//t相BACKUP有用功率
    int16_t     t_phase_backup_reactive_power[2];//t相BACKUP无用功率    
    int16_t     t_phase_backup_power[2]         ;//t相BACKUP视在功率

    int16_t     r_phase_load_volt;              //R 相远端负载电压  /*new */
    int16_t     r_phase_load_cur;               //R 相远端负载电流
    uint16_t    r_phase_load_freq;               //R 相远端负载频率  /*new */
    int16_t     r_phase_load_active_power[2];      //R 相远端负载有功功率
    int16_t     r_phase_load_reactive_power[2];    //R 相远端负载无功功率
    int16_t     r_phase_load_power[2];             //R 相远端负载视在功率    

    int16_t     s_phase_load_volt;              //S 相远端负载电压  /*new */
    int16_t     s_phase_load_cur;               //S 相远端负载电流
    uint16_t    s_phase_load_freq;               //S 相远端负载频率  /*new */
    int16_t     s_phase_load_active_power[2];      //S 相远端负载有功功率
    int16_t     s_phase_load_reactive_power[2];    //S 相远端负载无功功率
    int16_t     s_phase_load_power[2];             //S 相远端负载视在功率   

    int16_t     t_phase_load_volt;              //t 相远端负载电压  /*new */
    int16_t     t_phase_load_cur;               //t 相远端负载电流
    uint16_t    t_phase_load_freq;               //t 相远端负载频率  /*new */
    int16_t     t_phase_load_active_power[2];      //t 相远端负载有功功率
    int16_t     t_phase_load_reactive_power[2];    //t 相远端负载无功功率
    int16_t     t_phase_load_power[2];             //t 相远端负载视在功率  

    uint16_t    r_phase_ThirdParty_Volt;                //r 相AC耦合第三方逆变器电压
    int16_t     r_phase_ThirdParty_cur;                 //r 相AC耦合第三方逆变器电流
    uint16_t    r_phase_ThirdParty_freq;                 //r 相AC耦合第三方逆变器频率
    int16_t     r_phase_ThirdParty_active_power[2];      //r 相AC耦合第三方逆变器有功功率
    int16_t     r_phase_ThirdParty_reactive_power[2];    //r 相AC耦合第三方逆变器无功功率
    int16_t     r_phase_ThirdParty_power[2];             //r 相AC耦合第三方逆变器视在功率  

    uint16_t    s_phase_ThirdParty_Volt;                 //s 相AC耦合第三方逆变器电压
    int16_t     s_phase_ThirdParty_cur;                  //s 相AC耦合第三方逆变器电流
    uint16_t    s_phase_ThirdParty_freq;                  //s 相AC耦合第三方逆变器频率
    int16_t     s_phase_ThirdParty_active_power[2];      //s 相AC耦合第三方逆变器有功功率
    int16_t     s_phase_ThirdParty_reactive_power[2];    //s 相AC耦合第三方逆变器无功功率
    int16_t     s_phase_ThirdParty_power[2];             //s 相AC耦合第三方逆变器视在功率  

    uint16_t    t_phase_ThirdParty_Volt;                 //t 相AC耦合第三方逆变器电压
    int16_t     t_phase_ThirdParty_cur;                  //t 相AC耦合第三方逆变器电流
    uint16_t    t_phase_ThirdParty_freq;                  //t 相AC耦合第三方逆变器频率
    int16_t     t_phase_ThirdParty_active_power[2];      //t 相AC耦合第三方逆变器有功功率
    int16_t     t_phase_ThirdParty_reactive_power[2];    //t 相AC耦合第三方逆变器无功功率
    int16_t     t_phase_ThirdParty_power[2];             //t 相AC耦合第三方逆变器视在功率  
   
    uint16_t    r_phase_inverter_volt;          //R 相逆变电压
    int16_t     r_phase_inverter_curr;          //R 相逆变电流
    uint16_t    r_phase_inverter_freq;          //R 相逆变频率
    int16_t     r_phase_inverter_active_power[2];  //R 相逆变有功功率
    int16_t     r_phase_inverter_reactive_power[2];//R 相逆变无功功率
    int16_t     r_phase_inverter_power[2];         //R 相逆变视在功率

    uint16_t    s_phase_inverter_volt;          //s 相逆变电压
    int16_t     s_phase_inverter_curr;          //s 相逆变电流
    uint16_t    s_phase_inverter_freq;          //s 相逆变频率
    int16_t     s_phase_inverter_active_power[2];  //s 相逆变有功功率
    int16_t     s_phase_inverter_reactive_power[2];//s 相逆变无功功率
    int16_t     s_phase_inverter_power[2];         //s 相逆变视在功率

    uint16_t    t_phase_inverter_volt;          //t 相逆变电压
    int16_t     t_phase_inverter_curr;          //t 相逆变电流
    uint16_t    t_phase_inverter_freq;          //t 相逆变频率
    int16_t     t_phase_inverter_active_power[2];  //t 相逆变有功功率
    int16_t     t_phase_inverter_reactive_power[2];//t 相逆变无功功率
    int16_t     t_phase_inverter_power[2];         //t 相逆变视在功率

    uint16_t    inv_info_reserve[37];           //预留

    uint16_t    batt_volt;                      //电池侧电压0.1V
    uint16_t    batt_current;                   //电池侧电流0.1A
    int16_t     batt_power[2];                  //电池侧功率1W

    uint16_t    InvEV_volt;                     //EV侧电压
    uint16_t    InvEV_cur;                      //EV侧电流
    int16_t     InvEV_power[2];                    //EV侧功率

    uint16_t    pv_ch1_input_volt               ;//PV1输入电压
    int16_t     pv_ch1_input_curr               ;//PV1输入电流
    uint16_t    pv_ch1_input_power[2]           ;//PV1输入功率   
    
    uint16_t    pv_ch2_input_volt               ;//PV2输入电压
    int16_t     pv_ch2_input_curr               ;//PV2输入电流
    uint16_t    pv_ch2_input_power[2]           ;//PV2输入功率

    uint16_t    pv_ch3_input_volt               ;//PV3输入电压
    int16_t     pv_ch3_input_curr               ;//PV3输入电流
    uint16_t    pv_ch3_input_power[2]           ;//PV3输入功率

    uint16_t    pv_ch4_input_volt               ;//PV4输入电压
    int16_t     pv_ch4_input_curr               ;//PV4输入电流
    uint16_t    pv_ch4_input_power[2]           ;//PV4输入功率

    uint16_t    pv_info_reserve[13];           //预留
    
    uint16_t    bus_voltage;  
    uint16_t    PhaseSequence;                  //市电相序

    uint16_t    Power_info_reserve[7];                  //预留

}_upload_Power_info_t;


typedef struct _upload_fault_info_stru_t
{
    /*故障信息：0x61C0-0x621F*/
    uint16_t    GridError;                          //0x61C0，电网故障
    uint16_t    PVError[2];                         //0x61C1-0x61C2，Pv故障
    uint16_t    BusError;                           //0x61C3，母线故障
    uint16_t    InvError;                           //0x61C4，逆变故障
    uint16_t    LoadError;                          //0x61C5，负载故障
    uint16_t    TempError;                          //0x61C6，温度故障
    uint16_t    GenError;                           //0x61C7，油机故障
    uint16_t    OtherError;                         //0x61C8，其他故障
    uint16_t    Error_reserve;                      //0x61C9，故障预留

    uint16_t    GridAlarm;                          //0x61CA，电网警告
    uint16_t    PVAlarm[2];                         //0x61CB-0x61CC，Pv警告
    uint16_t    BusAlarm;                           //0x61CD，母线警告
    uint16_t    InvAlarm;                           //0x61CE，逆变警告
    uint16_t    LoadAlarm;                          //0x61CF，负载警告
    uint16_t    TempAlarm;                          //0x61D0，温度警告
    uint16_t    GenAlarm;                           //0x61D1，油机警告
    uint16_t    OtherAlarm;                         //0x61D2，其他警告
    uint16_t    Alarm_reserve;                      //0x61D3，警告预留

    uint16_t    GridNotice;                          //0x61D4，电网通知
    uint16_t    PVNotice[2];                         //0x61D5-0x61D6，Pv通知
    uint16_t    BusNotice;                           //0x61D7，母线通知
    uint16_t    InvNotice;                           //0x61D8，逆变通知
    uint16_t    LoadNotice;                          //0x61D9，负载通知
    uint16_t    TempNotice;                          //0x61DA，温度通知
    uint16_t    GenNotice;                           //0x61DB，油机通知
    uint16_t    OtherNotice;                         //0x61DC，其他通知
    uint16_t    Notice_reserve[3];                   //0x61DF，通知预留

    uint16_t    fault_info_reserve[48];              //预留

}_upload_fault_info_t;



typedef struct _upload_info_stru_t
{
    _upload_basic_info_t        upload_basic_info;
    _upload_module_status_t     upload_module_status;
    _upload_Capacity_info_t     upload_Capacity_info;
    _upload_Power_info_t        upload_Power_info;
    _upload_fault_info_t        upload_fault_info;

} UploadInfo_t;


extern UploadInfo_t  gstUploadInfo;

void UploadInfoInit(void);
void SysInfoUpload(void);


#endif /* APP_SYS_INFO_H_ */
