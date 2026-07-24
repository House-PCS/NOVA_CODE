#ifndef COLLECT_HHH
#define COLLECT_HHH

#include "queue.h"
#include "dev_modbus.h"
#include "slave_bms.h"
#include "slave_pcs.h"
#include "dev_modbus.h"
#include "modbus_crc.h"

#define MAX_USER_NUM        4
#define USER_PASSWORD_MAX_LEN   4
typedef enum
{
	ATTR_INVALID = 0,
	ATTR_RO,
	ATTR_WO,
	ATTR_RW,
} teDataAttrRW;

typedef enum
{
	DATA_INVALID = 0,
	DATA_U8,
	DATA_S8,
	DATA_U16,
	DATA_S16,
	DATA_U32,
	DATA_S32,
	DATA_STRING,
} teDataType;
typedef struct
{
	uint16_t base_addr;
	uint16_t num;
	uint16_t data_type;
	uint16_t attr_rw;
	void *pdata;
} tsModbusDataPoint;

typedef enum
{
    EN_COLLECT_POLLING_STATUS_IDLE=0,
    EN_COLLECT_POLLING_STATUS_REV,
    EN_COLLECT_POLLING_STATUS_REV_OVERTIME,
    EN_COLLECT_POLLING_STATUS_WAITING_TX,
    EN_COLLECT_POLLING_STATUS_INTERVAL_WAIT,
}EN_COLLECT_POLLING_STATUS;




#define     NORMAL_USER             0
#define COLLECT_EQUIPEMENT_ID                       0x506B  
#define COLLECT_EQUIPEMENT_ID_NUM                   10
#define COLLECT_MODE_ID                             0x5000
#define MODE_ID_NUM                                 1

#define COLLECT_SLAVE_ADDR                          0X501E
#define COLLECT_SALVE_ADDR_NUM                      1
#define COLLECT_UART_BAUD_ADDR                      0X501F
#define COLLECT_UART_BAUD_NUM                       1













#pragma pack(2)

typedef struct _collect_cap_info
{
    uint16_t    load_cost_day;
    uint16_t    load_cost_month[2];
    uint16_t    load_cost_year[2];
    uint16_t    load_cost_total[2];

    uint16_t    grid_connect_cnt[2];

    uint16_t    electic_purchase_day;
    uint16_t    electic_purchase_month[2];       
    uint16_t    electic_purchase_year[2];    
    uint16_t    electic_purchase_total[2]; 

    uint16_t    electic_sale_day;
    uint16_t    electic_sale_month[2];     
    uint16_t    electic_sale_year[2];   
    uint16_t    electic_sale_total[2]; 

}tsCollectCapInfo;


typedef struct  _peak_load_shift
{
    uint32_t    chg_start_time1;
    uint32_t    chg_end_time1;
    uint32_t    dsg_start_time1;
    uint32_t    dsg_end_time1;

    uint32_t    chg_start_time2;
    uint32_t    chg_end_time2;
    uint32_t    dsg_start_time2;
    uint32_t    dsg_end_time2;
}tsPeakLoadShiftData;

typedef struct _collect_data
{
    uint16_t    mode_id;                                 //0x5000    
    uint16_t    sn[10];
    uint16_t    hw_ver;
    uint16_t    software_ver;   
    uint16_t    user_type;
    uint16_t    normal_password[4];                     //0X500E--5011
    uint16_t    manager_password[4];                    //0X5012--5015
    uint16_t    installer_password[4];                  //0X5016--5019
    uint16_t    internal_password[4];                   //0X501A--501D

    uint16_t    slave_comm_addr;                        //0X501E 设备485通讯地址。

    uint16_t    monitor_485_baud;                       //0X501F 485通讯波特率
    uint16_t    rtc_time[2];                            //0X5020--5021
    uint16_t    backlight_time;                         //0x5022
    uint16_t    backlight_brightness;                   //0x5023
    uint16_t    languange_type;                         //0x5024 
    uint16_t    reserved0[11];                          //0x5025---0x502F

  
    uint16_t    reset_factory;                          //0X5030   
    uint16_t    clean_warning;                          //0X5031    
    uint16_t    clean_history_warning;                  //0X5032

    uint16_t    reserved1[7];                           //0x5033--0x5039
    tsPeakLoadShiftData     peak_load_shift_time;       //0X503A   -- 5059
    uint16_t    net_state;                              //0X505A  
    uint16_t    discharge_depth;                        //0x505B 
    uint16_t    batt_wakeup_time;                       //0x505C           
    uint16_t    reserved2[9];                           //0x505D---0x5065
    uint16_t    device_running_status;                  //0x5066                 
    uint16_t    batt_pack_num;                          //0x5067                            
    uint16_t    batt_reted_cap_value;                   //0x5068
    uint16_t    batt_soh;                               //0x5069
    uint16_t    batt_soc;                               //0x506A
    uint16_t    equipement_id[10];                      //0x506B - 0X5074
    uint16_t    reserved3[30];                          //0X5075--0x508F


    uint32_t    grid_connect_cnt;                       //0x5100
    tsCollectCapInfo   collect_cap_info;                //0x5101

}tsCollectData;

typedef struct _collect_para_stru_t
{
    tsCollectData       collect_data;
}tsCollectPara_t;



typedef struct _pv_input_info
{
    uint16_t    pv_ch1_cur_value;
    uint16_t    pv_ch2_cur_value; 
    uint16_t    pv_ch1_volt_value;
    uint16_t    pv_ch2_volt_value;
    uint16_t    pv_ch1_power_value;
    uint16_t    pv_ch2_power_value;  
}tsPvInputInfo;

typedef struct _gird_output_info
{
    uint16_t    gird_output_cur_value;
    uint16_t    gird_output_volt_value;
    uint16_t    gird_output_freq_value;
    int16_t     gird_output_active_power_value;
    uint16_t    gird_output_reactive_power_value;
    uint16_t    gird_output_apparent_power_value;

}tsGridOutputInfo;

typedef struct _oil_input_info
{
    uint16_t    oil_cur_value;
    uint16_t    oil_volt_value;
    uint16_t    oil_freq_value;
    uint16_t    oil_power_value;
}tsOilInputInfo;


typedef struct _terminal_load_info
{
    uint16_t    terminal_load_cur_value;
    uint16_t    terminal_load_volt_value;
    uint16_t    terminal_load_freq_value;
    uint16_t    terminal_load_power_value;
}tsTerminalLoadInfo;



typedef struct _inver_info
{
    uint16_t    inver_cur_value;
    uint16_t    inver_volt_value;
    uint16_t    inver_freq_value;
    int16_t     inver_active_power_value;
    uint16_t    inver_reactive_power_value;
    uint16_t    inver_apparent_power_value;
}tsInverInfo;


typedef struct _batt_info
{
    uint16_t    batt_volt_value;
    int16_t     batt_cur_value;
    uint16_t    cap_value;
    uint8_t     batt_soc_value;
    uint8_t     batt_soh_value;
}tsBattInfo;

typedef struct _temp_info
{
    int16_t     inverter_radiator_temp;             
    int16_t     chassis_internal_temp;            
    int16_t     batt_max_temp;                      

}tsTempInfo;


typedef struct _pcs_status_info
{
    uint16_t    system_status;                      //
    uint16_t    invs_status;
    uint16_t    mppt_status;
    uint16_t    dcdc_status;  
}tsPcsStatusInfo;

typedef struct _sys_setting_info
{

    uint16_t    work_mode;
    uint16_t    grid_standards;
    uint16_t    grid_standard_seg;                         
}tsSysSettingInfo;

typedef struct _collcet_info
{
    tsPvInputInfo           pv_input_info;
    tsGridOutputInfo        grid_output_info;
    tsOilInputInfo          oil_input_info;
    tsTerminalLoadInfo      terminal_load_info;
    tsInverInfo             inver_info;
    tsBattInfo              batt_info;
    int16_t                 dc_bus_volt_value;
    tsTempInfo              temp_info;
    tsPcsStatusInfo         pcs_status_info;
    tsSysSettingInfo        sys_setting_status_info;
}tsCollectDisplayInfo;





typedef struct _user_password
{
    uint16_t    passwrod[MAX_USER_NUM][USER_PASSWORD_MAX_LEN];
}tsUserPassword;

typedef struct _collect_ctrl_info
{
    tsCollectDisplayInfo        CollectDiplayInfo; 
    tsUserPassword              user_password;
    uint16_t                    cur_user_type;
    tsPeakLoadShiftData         peak_load_shift_data; 

}tsCollectCtrlInfo;





#pragma pack()

extern  tsCollectCtrlInfo   _gCollectCtrlInfo;
extern  tsCollectPara_t     gCollectParaData;

const char* GetPcsModelByModelId(uint16_t model_id);
bool    CommCollectInit(void);
void    SlaveCollectModbusCommProcess(void);
bool    GetCollectData(uint16_t data_start_addr,uint16_t reg_len,uint8_t *p_data);   //
bool    SetCollectData(uint8_t *pdata,uint16_t len);   //

uint8_t GetCorrectRtcStatus(void);
void    CleanCorrectRtcFlag(void);
uint16_t    GetCollectBaud(void);

void ModbusReadInput(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num);
void ModbusReadRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num);
void ModbusWriteMultiRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_value);
void ModbusWriteSingleRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t value);

//tsPvInputInfo       GetPvInputInfo(void);
//tsGridOutputInfo    GetGridOutputInfo(void);
//tsOilInputInfo      GetOilInputInfo(void);
//tsTerminalLoadInfo  GetTerminalLoadInfo(void);
//tsInverInfo         GetInverInfo(void);
//tsBattInfo          GetBattInfo(void);
//int16_t             GetBusVoltInfo(void);
//tsTempInfo          GetTempInfo(void);
//tsPcsStatusInfo     GetPcsStatusInfo(void);
//tsSysSettingInfo    GetSysSettintInfo(void);


#endif