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
#define MODBUS_INPUT_REG_END_ADDR        (0x621F)
#define MODBUS_HOLD_REG_START_ADDR       (0x6220)
#define MODBUS_HOLD_REG_END_ADDR         (0x631F)

#define MODBUS_ARC_INPUT_REG_START_ADDR  (0x6400)
#define MODBUS_ARC_INPUT_REG_END_ADDR    (0x644F)
#define MODBUS_ARC_HOLD_REG_START_ADDR   (0X6450)
#define MODBUS_ARC_HOLD_REG_END_ADDR     (0X647F)

#define MODBUS_COPRO_INPUT_REG_START_ADDR  (0x6480)
#define MODBUS_COPRO_INPUT_REG_END_ADDR    (0x64CF)
#define MODBUS_COPRO_HOLD_REG_START_ADDR   (0X64D0)
#define MODBUS_COPRO_HOLD_REG_END_ADDR     (0X64FF)



#define MODBUS_HOLD_REG_START_ADDR_TEST  (0x8000)
#define MODBUS_HOLD_REG_END_ADDR_TEST    (0x8002)
#define MODBUS_INPUT_REG_START_ADDR_TEST (0x8010)
#define MODBUS_INPUT_REG_END_ADDR_TEST   (0x802C)

#define FLASH_DspHoldFRAME_START_ADDR      (0x00180000)
#define FLASH_DspHoldPara_START_ADDR      (0x00181000)

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
    uint16_t  protocol_type;               //协议类型
    uint16_t  recv_network_state;           //0:未收到网络数据  1：收到网络数据   2:收到监控卡数据 0xFF：收到数据等待超时
} NetworkInfo_t;




typedef struct UdpDspOut_basic_info_stru_t
{
    /*0x6000-0x601f:逆变器基础信息，32字节*/
    uint16_t    mode_id;                            //0x6000
    uint16_t    sn[10];                             //0x6001-0x600A
    uint16_t    hw_version;                         //0x600b，硬件版本号：bit[15:8]:大版本，0~25代表A~Z，其余值预留。bit[7:0]:小版本，从0开始第一版的硬件版本就是A0
    uint16_t    software_ver[2];                    //0x600C-0x600D，固件版本号
    uint16_t    software_ver_reserve[12];           //0x600E-0x6019，固件版本号预留，12字节
    uint16_t    reserve1[6];                        //0x601A-0x601F，预留

    /*0x6020-0x603f:逆变器基础信息，32字节*/
    uint16_t    arc_mode_id;                         //0x6020
    uint16_t    arc_sn[10];                          //0x6021-0x602A
    uint16_t    arc_hw_version;                      //0x602B
    uint16_t    arc_software_ver[2];                 //0x602C-0x602D，固件版本号
    uint16_t    arc_software_ver_reserve[12];        //0x602E-0x6039，固件版本号预留，12字节
    uint16_t    reserve2[6];                         //0x603A-0x603F，预留

    /*0x6040-0x605f:基础信息预留，32字节*/
    uint16_t    reserve_basic_information[32];      //0x6040-0x605f，基础信息预留

} UdpDspOut_basic_info_t;


typedef struct UdpDspOut_module_status_t
{
    /*0x6060-0x607F:模块状态*/
    uint16_t    sys_input_stat;              //0x6060，PowerInterfaceState，功率接口状态: 每个bit代表一个接口
    
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

} UdpDspOut_module_status_t;


typedef struct UdpDspOut_Capacity_info_stru_t
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

}UdpDspOut_Capacity_info_t;


typedef struct UdpDspOut_Power_info_stru_t
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
    uint16_t    r_phase_ongrid_fre;             //R 相并网口频率
    int16_t     r_phase_ongrid_active_power[2]; //R 相并网口有功功率
    int16_t     r_phase_ongrid_reactive_power[2];//R 相并网口无功功率
    int16_t     r_phase_ongrid_power[2];        //R 相并网口视在功率

    uint16_t    s_phase_ongrid_volt;            //S 相并网口电压
    int16_t     s_phase_ongrid_curr;            //S 相并网口电流
    uint16_t    s_phase_ongrid_fre;             //S 相并网口频率
    int16_t     s_phase_ongrid_active_power[2]; //S 相并网口有功功率
    int16_t     s_phase_ongrid_reactive_power[2];//S 相并网口无功功率
    int16_t     s_phase_ongrid_power[2];        //S 相并网口视在功率    

    uint16_t    t_phase_ongrid_volt;            //t 相并网口电压
    int16_t     t_phase_ongrid_curr;            //t 相并网口电流
    uint16_t    t_phase_ongrid_fre;             //t 相并网口频率
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
    uint16_t    r_phase_backup_fre              ;//R相BACKUP频率    /*新增 */
    int16_t     r_phase_backup_active_power[2]  ;//R相BACKUP有用功率
    int16_t     r_phase_backup_reactive_power[2];//R相BACKUP无用功率    
    int16_t     r_phase_backup_power[2]         ;//R相BACKUP视在功率

    uint16_t    s_phase_backup_volt             ;//s相BACKUP电压
    int16_t     s_phase_backup_curr             ;//s相BACKUP电流
    uint16_t    s_phase_backup_fre              ;//s相BACKUP频率    /*新增 */
    int16_t     s_phase_backup_active_power[2]  ;//s相BACKUP有用功率
    int16_t     s_phase_backup_reactive_power[2];//s相BACKUP无用功率    
    int16_t     s_phase_backup_power[2]         ;//s相BACKUP视在功率

    uint16_t    t_phase_backup_volt             ;//t相BACKUP电压
    int16_t     t_phase_backup_curr             ;//t相BACKUP电流
    uint16_t    t_phase_backup_fre              ;//t相BACKUP频率    /*新增 */
    int16_t     t_phase_backup_active_power[2]  ;//t相BACKUP有用功率
    int16_t     t_phase_backup_reactive_power[2];//t相BACKUP无用功率    
    int16_t     t_phase_backup_power[2]         ;//t相BACKUP视在功率

    int16_t     r_phase_load_volt;              //R 相远端负载电压  /*new */
    int16_t     r_phase_load_cur;               //R 相远端负载电流
    uint16_t    r_phase_load_fre;               //R 相远端负载频率  /*new */
    int16_t     r_phase_load_active_power[2];      //R 相远端负载有功功率
    int16_t     r_phase_load_reactive_power[2];    //R 相远端负载无功功率
    int16_t     r_phase_load_power[2];             //R 相远端负载视在功率    

    int16_t     s_phase_load_volt;              //S 相远端负载电压  /*new */
    int16_t     s_phase_load_cur;               //S 相远端负载电流
    uint16_t    s_phase_load_fre;               //S 相远端负载频率  /*new */
    int16_t     s_phase_load_active_power[2];      //S 相远端负载有功功率
    int16_t     s_phase_load_reactive_power[2];    //S 相远端负载无功功率
    int16_t     s_phase_load_power[2];             //S 相远端负载视在功率   

    int16_t     t_phase_load_volt;              //t 相远端负载电压  /*new */
    int16_t     t_phase_load_cur;               //t 相远端负载电流
    uint16_t    t_phase_load_fre;               //t 相远端负载频率  /*new */
    int16_t     t_phase_load_active_power[2];      //t 相远端负载有功功率
    int16_t     t_phase_load_reactive_power[2];    //t 相远端负载无功功率
    int16_t     t_phase_load_power[2];             //t 相远端负载视在功率  

    uint16_t    r_phase_ThirdParty_Volt;                //r 相AC耦合第三方逆变器电压
    int16_t     r_phase_ThirdParty_cur;                 //r 相AC耦合第三方逆变器电流
    uint16_t    r_phase_ThirdParty_fre;                 //r 相AC耦合第三方逆变器频率
    int16_t     r_phase_ThirdParty_active_power[2];      //r 相AC耦合第三方逆变器有功功率
    int16_t     r_phase_ThirdParty_reactive_power[2];    //r 相AC耦合第三方逆变器无功功率
    int16_t     r_phase_ThirdParty_power[2];             //r 相AC耦合第三方逆变器视在功率  

    uint16_t    s_phase_ThirdParty_Volt;                 //s 相AC耦合第三方逆变器电压
    int16_t     s_phase_ThirdParty_cur;                  //s 相AC耦合第三方逆变器电流
    uint16_t    s_phase_ThirdParty_fre;                  //s 相AC耦合第三方逆变器频率
    int16_t     s_phase_ThirdParty_active_power[2];      //s 相AC耦合第三方逆变器有功功率
    int16_t     s_phase_ThirdParty_reactive_power[2];    //s 相AC耦合第三方逆变器无功功率
    int16_t     s_phase_ThirdParty_power[2];             //s 相AC耦合第三方逆变器视在功率  

    uint16_t    t_phase_ThirdParty_Volt;                 //t 相AC耦合第三方逆变器电压
    int16_t     t_phase_ThirdParty_cur;                  //t 相AC耦合第三方逆变器电流
    uint16_t    t_phase_ThirdParty_fre;                  //t 相AC耦合第三方逆变器频率
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
    uint16_t    batt_power[2];                     //电池侧功率1W

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

}UdpDspOut_Power_info_t;


typedef struct UdpDspOut_fault_info_stru_t
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

}UdpDspOut_fault_info_t;


typedef struct _udp_dsp_output_para_stru_t
{
    UdpDspOut_basic_info_t        UdpDspOut_basic_info;
    UdpDspOut_module_status_t     UdpDspOut_module_status;
    UdpDspOut_Capacity_info_t     UdpDspOut_Capacity_info;
    UdpDspOut_Power_info_t        UdpDspOut_Power_info;
    UdpDspOut_fault_info_t        UdpDspOut_fault_info;

} UdpDspOutputPara_t;



typedef struct _udp_dsp_hold_para_stru_t
{
    uint16_t    Second_ID;                                  //0x6220 功率段设别ID 0：默认1：6kw功率段 2：8kw功率段 3：10kw功率段 4：12kw功率段 5：14kw功率段 6 ：16kw功率段
    uint16_t    work_mode;                                  //0x6221 设置工作模式    2:削峰填谷 1:电池优先 0:自发自用
    uint16_t    charging_input_type;                        //0x6222 设置强制发电机使能    1:强制为发电机 0:强制为市电 2: 有市电用市电无市电用发电机   
    uint16_t    set_batt_chg_curr;                          //0x6223 设置电池充电最大电流    0.1A
    uint16_t    set_batt_chg_power[2];                      //0x6224-0x6225 设置电池放电最大功率    1.0W
    uint16_t    set_batt_dsg_curr;                          //0x6226 设置电池放电最大电流    0.1A   
    uint16_t    set_batt_dsg_power[2];                      //0x6227-0x6228 设置电池放电最大功率    1.0W
    uint16_t    InvPVMaxPower[2];                           //0x6229-0x622A 逆变器最大允许的光伏功率(默认值为设备根据自身型号预置的最大值)
    uint16_t    InvOnGridIFInMaxPower[2];                   //0x622B~0x622C 并网口最大输入功率(默认值为设备根据自身型号预置最大值)
    uint16_t    InvOnGridIFOutMaxPower[2];                  //0x622D~0x622E 并网口最大输出功率(默认值为设备根据自身型号预置最大值)
    uint16_t    InvGenMaxPower[2];                          //0x622F~0x6230 发电机最大功率(默认为0，首次使用需要先设置)
    uint16_t    InvEVMaxInPower[2];                         //0x6231~0x6232 设备吸收充电桩最大功率
    uint16_t    InvEVMaxOutPower[2];                        //0x6233~0x6234 设备供给充电桩最大功率
    uint16_t    NetworkPowerMaxPowerLimit[2];               //0x6235~0x6236 入户功率限制：根据用户实际电力布局设定，一般要大于我们设备+总负载的承载功率
    uint16_t    on_grid_power[2];                           //0x6237~0x6238 最大并网功率 不同产品默认值不同

    uint16_t    reserve1[23];                               //0x6239~0x624F 预留
    uint16_t    pv_input_mode;                              //0x6250 设置PV输入模式 00:输入独立模式 01:输入并联模式 02:输入恒压模式
    uint16_t    anti_reverse_flow_enable;                   //0x6251 设置防逆流使能 00:禁止PV给电网送电 01:允许PV给电网送电

    uint16_t    reserve2[2];                                //0x6252~0x6253 预留

    uint16_t    x_boost;                                    //0x6254 设置X-BOOST功能使能   1:使能 0:禁止
    uint16_t    ups_delay_0ms;                              //0x6255 设置Ups的0毫秒切换功能使能    1:使能 0:禁止       
    uint16_t    backup_output_enable;                       //0x6256 设置本地负载输出使能    1:使能 0:禁止
    uint16_t    program_ctrl_enable;                        //0x6257 设置程序控制使能 00:禁止 01:使能
    uint16_t    over_volt_derating_enable;                  //0x6258 设置过压降额使能 00:禁止 01:使能
    uint16_t    over_freq_derating_enable;                  //0x6259 设置过频降额使能 00:禁止 01:使能
    uint16_t    under_volt_derating_enable;                 //0x625A 设置欠压降额使能 00:禁止 01:使能
    uint16_t    under_freq_derating_enable;                 //0x625B 设置欠频降额使能 00:禁止 01:使能
    uint16_t    island_enable;                              //0x625C 设置孤岛使能    00:禁止 01:使能
    uint16_t    ac_coupling_enable;                         //0x625D 设置AC耦合使能  00:禁止 01:使能
    uint16_t    high_low_crossover_enable;                  //0x625E 设置高低穿使能  00:禁止 01:使能       
    uint16_t    drm_state;                                  //0x625F 每个bit代表一路DRM 0-关闭，1-打开   默认关闭 BIT0:DRM0  BIT1:DRM1  BIT2:DRM2  BIT3:DRM3  BIT4:DRM4
    uint16_t    LLCEnable;                                  //0x6260 （Pack预充动作完成后开启）0-关闭，1-打开   默认关闭  

    uint16_t    reserve3[5];                                //0x6261~0x6265 预留

    uint16_t    cto_meter;                                  //0x6266 设置CT或电表   00:CT 01:meter
    uint16_t    ct_direction;                               //0x6267 设置CT方向   00:正方向 01:负方向  
    uint16_t    meter_brand;                                //0x6268 设置电表品牌   详细见协议 不常用变量 00:正泰（CHNT） 01：安科瑞（Acrel）      
    uint16_t    grid_standards;                             //0x6269 设置市场协商电网标准 00:中国 01:德国 02:澳洲A 03:澳洲B 04:澳洲C 05:新西兰 06H：意大利
    uint16_t    grad_standards_segment;                     //0x626A 设置电网标准细分 详细见协议 不常用变量
    uint16_t    BatteryChgMaxSOC;                           //0x626B 电池充电截止SOC：0% ~ 100%   默认：100%
    uint16_t    OnGridBatteryDsgMinSoc;                     //0x626C 并网电池放电截止SOC：0% ~ 100%   默认：10%
    uint16_t    OffGridBatteryDsgMinSoc;                    //0x626D 离网电池放电截止SOC：0% ~ 100%   默认：10%

    uint16_t    reserve4[8];                                //0x626E~0x6275 预留

    uint16_t    reset_factory;                              //0x6276 设置恢复出厂设置 写入 0X5AA5 则恢复出厂设置    
    uint16_t    on_grid_reconnect_time;                     //0x6277 设置并网重连时间(S)  5 - 300 
    uint16_t    reactive_power_respones_time;               //0x6278 设置无功响应时间(S) 
    int16_t     reactiveSet;                                //0x6279 无功功率设定，-100~100
    uint16_t    reactive_power_ctrl_method;                 //0x627A 设置无功控制方式 00:按照功率因数控制 01:按照无功功率控制 02:按照QU曲线控制 03:按照QP曲线控制
    uint16_t    ActiveSet;                                  //0x627B 有功设置值：0-100	限定最大功率百分比  
    int16_t     FactorSet;                                  //0x627C 工因设置值：//80-100	无功为负，超前功因 0.8-1 //-(80-100)	无功为正，滞后功因 0.8-1

    uint16_t    reserve5[19];                               //0x627D~0x628F 预留

    uint16_t    remote_shutdown_status;                     //0x6290 设置远程待机   0x01待机，0x00为正常运行
    uint16_t    PV_Switch;                                  //0x6291 00H：Disable01H：Enable 默认：Enable
    uint16_t    PF_Switch;                                  //0x6292 00H：Disable01H：Enable 默认：Enable
    uint16_t    dsp_wake_en;                                //0X6293 PV WakeUp Status 设置激活使能 01:PV激活使能 02:PV激活完成 03:正常功能 
    uint16_t    pcs_dsg_ctrl;                               //0x6294 设置放电控制 00:监控卡异常禁止放电 01:允许放电 02:禁止放电
    uint16_t    pcs_chg_ctrl;                               //0x6295 设置充电控制 00:监控卡异常禁止充电 01:允许充电 02:禁止充电
    uint16_t    pcs_user_type;                              //0x6296 设置用户类型 00:普通用户 01:管理员 02:安装调试 03:内部  
    uint16_t    ignor_ct_err;                               //0x6297 设置消除CT故障 00:不消除CT告警 01:消除CT告警 
    uint16_t    sys_upgrade_status;                         //0x6298 系统升级
    uint16_t    ElecClear;                                  //0x6299 清除累计电量，0：不清除，1：清除
    uint16_t    ShutDown;                                   //0x629A 关机，0：不关机，1：关机 

    uint16_t    reserve6[21];                               //0x629A~0x62AF 预留

    uint16_t    batt_dsg_volt_point;                        //0x62B0 设置电池放电截止电压   0.1V 范围 43.2 - 57.6V
    uint16_t    batt_chg_volt_point;                        //0x62B1 设置电池充电电压保护电  0.1V 范围 43.2 - 57.6V
    uint16_t    pv_uvp_point;                               //0x62B2 设置PV输入欠压保护点   0.1V
    uint16_t    pv_ovp_point;                               //0x62B3 设置PV输入过压保护点   0.1V   
    uint16_t    ev_uvp_point;                               //0x62B4 设置EV输入欠压保护点   0.1V
    uint16_t    ev_ovp_point;                               //0x62B5 设置EV输入过压保护点   0.1V  
    uint16_t    grid_volt_low_limit;                        //0x62B6 设置电网电压下限  0.1V
    uint16_t    grid_volt_upper_limit;                      //0x62B7 设置电网电压上限  0.1V
    uint16_t    grid_freq_low_limit;                        //0x62B8 设置电网频率下限  0.01HZ
    uint16_t    grid_freq_upper_limit;                      //0x62B8 设置电网频率上限  0.01HZ
    uint16_t    bypass_volt_low_limit;                      //0x62BA 设置电网频率下限  0.01HZ
    uint16_t    bypass_volt_upper_limit;                    //0x62BB 设置旁路电压上限  0.1V
    uint16_t    bypass_freq_low_limit;                      //0x62BC 设置旁路频率下限  0.01HZ
    uint16_t    bypass_freq_upper_limit;                    //0x62BD 设置旁路频率上限  0.01HZ
    uint16_t    over_volt_derating_point;                   //0x62BE 设置过压降额点    1%
    uint16_t    over_freq_derating_point;                   //0x62BF 设置过频降额点    1%
    uint16_t    under_volt_derating_point;                  //0x62C0 设置欠压降额点(充电)  1%
    uint16_t    under_freq_derating_point;                  //0x62C1 设置欠频降额点(充电)  1%
    
    uint16_t    reserve7[14];                               //0x62C2~0x62CF 预留

    uint16_t    CalibrationReset;                           //0x62D0 校准复位，即恢复出厂设置
    uint16_t    reserve8[4];                                //0x62D1~0x62D4 预留
    uint16_t    r_phase_volt_calibration_k_parm;            //0x62D5 R(U/A)相电压校准系数K: 默认值 4096
    uint16_t    r_phase_volt_calibration_b_parm;            //0x62D6 R(U/A)相电压校准偏置B: 默认值 4096
    uint16_t    s_phase_volt_calibration_k_parm;            //0x62D7 S(V/B)相电压校准系数K: 默认值 4096
    uint16_t    s_phase_volt_calibration_b_parm;            //0x62D8 S(V/B)相电压校准偏置B: 默认值 4096
    uint16_t    t_phase_volt_calibration_k_parm;            //0x62D9 T(W/C)相电压校准系数K: 默认值 4096
    uint16_t    t_phase_volt_calibration_b_parm;            //0x62DA T(W/C)相电压校准偏置B: 默认值 4096
    uint16_t    r_phase_cur_calibration_k_parm;             //0x62DB R(U/A)相电流校准系数K: 默认值 4096
    uint16_t    r_phase_cur_calibration_b_parm;             //0x62DC R(U/A)相电流校准偏置B: 默认值 4096
    uint16_t    s_phase_cur_calibration_k_parm;             //0x62DD S(V/B)相电流校准系数K: 默认值 4096
    uint16_t    s_phase_cur_calibration_b_parm;             //0x62DE S(V/B)相电流校准偏置B: 默认值 4096
    uint16_t    t_phase_cur_calibration_k_parm;             //0x62DF T(W/C)相电流校准系数K: 默认值 4096
    uint16_t    t_phase_cur_calibration_b_parm;             //0x62F0 T(W/C)相电流校准偏置B: 默认值 4096
    
    uint16_t    Inv_Battery_Volt_Calibration_CoefK;         //0x62F1 逆变电池侧电压校准系数
    uint16_t    Inv_Battery_Volt_Calibration_CoefB;         //0x62F2 逆变电池侧电压校准偏置
    uint16_t    Inv_Battery_Cur_Calibration_CoefK;          //0x62F3 逆变电池侧电流校准系数
    uint16_t    Inv_Battery_Cur_Calibration_CoefB;          //0x62F4 逆变电池侧电流校准偏置
    uint16_t    Inv_EV_Volt_Calibration_CoefK;              //0x62F5 逆变EV侧电压校准系数
    uint16_t    Inv_EV_Volt_Calibration_CoefB;              //0x62F6 逆变EV侧电压校准偏置
    uint16_t    Inv_EV_Cur_Calibration_CoefK;               //0x62F7 逆变EV侧电流校准系数
    uint16_t    Inv_EV_Cur_Calibration_CoefB;               //0x62F8 逆变EV侧电流校准偏置
    
    uint16_t    pv_ch1_volt_k_calibrate;                    //0x62f9 PV1电压校准系数K: 默认值 4096
    uint16_t    pv_ch1_volt_b_calibrate;                    //0x62fa PV1电压校准偏置B: 默认值 4096
    uint16_t    pv_ch1_curr_k_calibrate;                    //0x62fb PV1电流校准系数K: 默认值 4096
    uint16_t    pv_ch1_curr_b_calibrate;                    //0x62fc PV1电流校准偏置B: 默认值 4096
        
    uint16_t    pv_ch2_volt_k_calibrate;                    //0x62fd PV2电压校准系数K: 默认值 4096
    uint16_t    pv_ch2_volt_b_calibrate;                    //0x62fe PV2电压校准偏置B: 默认值 4096
    uint16_t    pv_ch2_curr_k_calibrate;                    //0x62ff PV2电流校准系数K: 默认值 4096
    uint16_t    pv_ch2_curr_b_calibrate;                    //0x6300 PV2电流校准偏置B: 默认值 4096

    uint16_t    pv_ch3_volt_k_calibrate;                    //0x6301 PV3电压校准系数K: 默认值 4096
    uint16_t    pv_ch3_volt_b_calibrate;                    //0x6302 PV3电压校准偏置B: 默认值 4096
    uint16_t    pv_ch3_curr_k_calibrate;                    //0x6303 PV3电流校准系数K: 默认值 4096
    uint16_t    pv_ch3_curr_b_calibrate;                    //0x6304 PV3电流校准偏置B: 默认值 4096
        
    uint16_t    pv_ch4_volt_k_calibrate;                    //0x6305 PV4电压校准系数K: 默认值 4096
    uint16_t    pv_ch4_volt_b_calibrate;                    //0x6306 PV4电压校准偏置B: 默认值 4096
    uint16_t    pv_ch4_curr_k_calibrate;                    //0x6307 PV4电流校准系数K: 默认值 4096
    uint16_t    pv_ch4_curr_b_calibrate;                    //0x6308 PV4电流校准偏置B: 默认值 4096
    
    uint16_t    reserve9[23];                               //0x6309~0x631F 预留

    /*0x6320~0x63FF 224*U16 预留*/

}UdpDspHoldPara_t;


typedef struct _udp_arc_uphold_para_stru_t
{
    uint16_t    arc_model_id;                               //0x6400 
    uint16_t    arc_sn[10];                                 //0x6401-0x640A SN号
    uint16_t    arc_hw_version;                             //0x640B 硬件版本号
    uint16_t    arc_sw_version[2];                          //0x640C~0x640D 固件版本号
    uint16_t    arc_sw_version_extend[12];                   //0x640E~0x6419 固件版本号扩展
    uint16_t    arc_reserve[6];                             //0x641A~0x641F 预留
    
    /*ARC 检测状态信息*/
    uint16_t    arc_occur;                           //0x6420 各通道电弧是否触发阈值，每个bit代表一个通道 0代表未发生，1代表发生 
    uint16_t    ins_res_low_occur;                     //0x6421 各通道绝缘阻抗低触发阈值，每个bit代表一个通道 0代表未发生，1代表发生
    uint16_t    arc_detect_sensitivity;               //0x6422 各通道拉弧检测灵敏度
    uint16_t    ins_res_threshold;                    //0x6423 绝缘检测阻抗阈值
    
    uint16_t    pv1_channel_vol;
    uint16_t    pv2_channel_vol;
    uint16_t    pv3_channel_vol;
    uint16_t    pv4_channel_vol;

    uint16_t    pv1_channel_Rp;
    uint16_t    pv2_channel_Rp;
    uint16_t    pv3_channel_Rp;
    uint16_t    pv4_channel_Rp;

    uint16_t    pv1_channel_Rn;
    uint16_t    pv2_channel_Rn;
    uint16_t    pv3_channel_Rn;
    uint16_t    pv4_channel_Rn;

    uint16_t    pv1_arc_threshold_num;
    uint16_t    pv2_arc_threshold_num;
    uint16_t    pv3_arc_threshold_num;
    uint16_t    pv4_arc_threshold_num;

    uint16_t    rescheckoverflag;                 //0x6434 绝缘自检完成标志位，0代表未完成，1代表完成   

    uint16_t    arc_status_reserve[27];             //0x6435~0x644F
}UdpArcUpHoldPara_t;


typedef struct _udp_arc_hold_para_stru_t
{
    uint16_t    arc_detect_enable;                            //0x6450 拉弧检测使能
    uint16_t    ins_res_enable;                               //0x6451 绝缘检测使能
    uint16_t    arc_ctrlstate_reserve1[14];                 //0x6452~0x645F 预留
    uint16_t    sys_upgrade_status;                           //0x6460 系统升级状态
    uint16_t    arc_ctrlstate_reserve2[31];                 //0x6461~0x647F 预留
}UdpArcHoldPara_t;



typedef struct _udp_copro_uphold_para_stru_t
{
    uint16_t    pcs_copro_mode_id;                          //0x6480
    uint16_t    sn[10];                                     //0x6481-0x648A SN号
    uint16_t    hw_version;                                 //0x648B 硬件版本号
    uint16_t    soft_version[2];                            //0x648C~0x648D 固件版本号  
    //uint16_t    Copro_res_err_code;//
    uint16_t    soft_version_reserve[12];                   //0x648E~0x6499 固件版本号扩展
    uint16_t    pcs_copro_basic_reserve[6];                 //0x649A~0x649F 预留

    uint16_t    pcs_copro_run_status;                       //0x64A0 PCS协处理器运行状态
    uint16_t    pcs_copro_state_reserve[47];                //0x64A1- 0x64CF

}UdpCoproupHoldPara_t;


typedef struct _udp_copro_hold_para_stru_t
{

    uint16_t    sys_upgrade_status;                           //0x64D0 系统升级状态
    uint16_t    dsp_heart_beat;                               //0x64D1 DSP心跳
    uint16_t    mcu_rly_cntl;                                 //0x64D2 MCU继电器控制
    uint16_t    mcu_rly_state;                                //0x64D3 MCU继电器状态反馈

    uint16_t    pcs_copro_ctrl_reserve[46];                 //0x64D2~0x64FF 预留

}UdpCoproHoldPara_t;


typedef struct _udp_pcs_para_stru_t
{
    UdpDspOutputPara_t     udp_pcs_upload;
    UdpDspHoldPara_t       udp_pcs_hold;
    UdpArcUpHoldPara_t     udp_arc_upload;
    UdpArcHoldPara_t       udp_arc_hold;
    UdpCoproupHoldPara_t   udp_copro_upload;
    UdpCoproHoldPara_t     udp_copro_hold;
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


bool InvSwitchStateGet(void);
bool BackupSwitchStateGet(void);
bool GetBatDsgEnable();
bool GetBatChgEnable();

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
bool GetIgnorCtOrMeter(void);
uint16_t GetWorkMode(void);
uint16_t GetUps0msEnable(void);
uint16_t GetElecClear(void);
uint16_t GetPcsLLCEnable(void);
uint16_t GetCtDirection(void);
uint16_t GetRemoteShutdownStatus(void);
void ClearUpdateFlag(void);
bool Save_System_Parameter(UdpDspHoldPara_t* para);
#endif /* APP_CH390_ANALYSIS_H_ */
