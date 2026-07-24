#ifndef __FPARA_ASP_H_
#define __FPARA_ASP_H_

#include "flash.h"
#include "stdbool.h"

//GD32F303RBT6 flash128k,64个页
//GD32F407ZE   flash512k,128个页
#define FMC_PAGE_SIZE   ((uint16_t)4096)

#define BOOT_INFO_ADDR      (0x08000000+FMC_PAGE_SIZE*127)
#define FPARA_A_ADDR        (0x08000000+FMC_PAGE_SIZE*126)   //4K    //根据数据量调整分区大小
#define FPARA_B_ADDR        (0x08000000+FMC_PAGE_SIZE*125)   //4K
#define FPARA_A_FLAG_ADD    (FPARA_A_ADDR+FMC_PAGE_SIZE-4)   //四字节写入，字节读出，低字节在前
#define FPARA_B_FLAG_ADD    (FPARA_B_ADDR+FMC_PAGE_SIZE-4)

#define FPARA_MAX   (sizeof(FparaInfoType))

typedef enum
{
    FPARA_WRITE_IDLE = 0,
    FPARA_WRITE_DEBUG,
    FPARA_WRITE_EMS,
}EN_FPARA_WRITE_TYPE;



#pragma pack(1)
typedef struct
{
    uint8_t DeviceType;
    uint16_t HardwareVersion;
    char sn[20];
    char dev_name[20];
}BootInfoType;
#pragma pack()

#pragma pack(1) 
typedef struct _sys_para_stru_t
{
    uint32_t q_max;     //mAH
    
    uint16_t curr_charge_rated;           //0.1A
    uint16_t curr_charge_min;
    uint16_t curr_discharge_rated;
    uint16_t curr_discharge_min;
    
    uint16_t charge_lower_vol;
    uint16_t charge_stop_vol;
    uint16_t charge_stop_soc;
    uint16_t discharge_lower_vol;
    uint16_t discharge_stop_vol;
    uint16_t discharge_stop_soc;
    
    uint16_t balance_start_diff_vol;    //1mV
    uint16_t pasv_balance_start_vol;    //1mV
    
    int8_t fan_start_temp;
    int8_t fan_stop_temp;
    
    uint16_t precharge_diff_vol;    //0.1V
    uint16_t sample_total_vol_diff;     //0.1V
    
    bool powre_on_com_en;
}SysParaType;
#pragma pack()

#pragma pack(1) 
typedef struct _alarm_para_stru_t
{
    uint16_t cell_vol_high_I;           //0.1mV
    uint16_t cell_vol_high_II;
    uint16_t cell_vol_high_III;
    uint16_t cell_vol_high_fall;
    
    uint16_t cell_vol_low_I;
    uint16_t cell_vol_low_II;
    uint16_t cell_vol_low_III;
    uint16_t cell_vol_low_fall;
    
    uint16_t cell_vol_diff_I;
    uint16_t cell_vol_diff_II;
    uint16_t cell_vol_diff_III;
    uint16_t cell_vol_diff_fall;
    
    int8_t charge_temp_high_I;           //
    int8_t charge_temp_high_II;
    int8_t charge_temp_high_III;
    uint8_t charge_temp_high_fall;
    
    int8_t charge_temp_low_I;
    int8_t charge_temp_low_II;
    int8_t charge_temp_low_III;
    uint8_t charge_temp_low_fall;
    
    int8_t discharge_temp_high_I;           //
    int8_t discharge_temp_high_II;
    int8_t discharge_temp_high_III;
    uint8_t discharge_temp_high_fall;
    
    int8_t discharge_temp_low_I;
    int8_t discharge_temp_low_II;
    int8_t discharge_temp_low_III;
    uint8_t discharge_temp_low_fall;
    
    uint8_t cell_temp_diff_I;
    uint8_t cell_temp_diff_II;
    uint8_t cell_temp_diff_III;
    uint8_t cell_temp_diff_fall;
    
    uint16_t total_vol_high_I;           //0.1V
    uint16_t total_vol_high_II;
    uint16_t total_vol_high_III;
    uint16_t total_vol_high_fall;
    
    uint16_t total_vol_low_I;
    uint16_t total_vol_low_II;
    uint16_t total_vol_low_III;
    uint16_t total_vol_low_fall;
    
    uint16_t charge_curr_high_I;           //
    uint16_t charge_curr_high_II;
    uint16_t charge_curr_high_III;
    uint16_t charge_curr_high_fall;
    
    uint16_t discharge_curr_high_I;           //
    uint16_t discharge_curr_high_II;
    uint16_t discharge_curr_high_III;
    uint16_t discharge_curr_high_fall;
    
    uint16_t res_low_I;
    uint16_t res_low_II;
    uint16_t res_low_III;
    uint16_t res_low_fall;
    
    int8_t pole_temp_high_I;           //
    int8_t pole_temp_high_II;
    int8_t pole_temp_high_III;
    uint8_t pole_temp_high_fall;
}AlarmParaType;
#pragma pack()

#pragma pack(1) 
typedef struct _bms_run_para_stru_t
{   
    uint8_t curr_zero;
    uint8_t curr_low;
    uint8_t curr_lower_ratio;           //降流比率 
    uint8_t curr_lower_cycle;       //降流周期
    
}BmsRunParaType;
#pragma pack()

#pragma pack(1) 
typedef struct _device_para_stru_t
{
    uint8_t     enet_mac[6];
    
    bool        enet_dhcp_en;
    uint8_t     enet_ip[4];
    uint8_t     enet_mask[4];
    uint8_t     enet_gw[4];
    uint16_t    enet_port;
}DeviceParaType;
#pragma pack()

typedef struct _fpara_para_stru_t
{
    uint16_t            versions;  //参数本地配置版本号，不能为0xFFFF.
    SysParaType         sys_para;
    AlarmParaType       alarm_para;
    BmsRunParaType      bms_run_para;
    DeviceParaType      device_para;
}FparaInfoType;    



////////////////////////////读参数///////////////////////////////////////////////
#define IR_PARA_Qmax         IR_BmsPara->sys_para.q_max 
#define IR_PARA_CurrChargeRated         IR_BmsPara->sys_para.curr_charge_rated 
#define IR_PARA_CurrChargeMin         IR_BmsPara->sys_para.curr_charge_min 
#define IR_PARA_CurrDisChargeRated      IR_BmsPara->sys_para.curr_discharge_rated 
#define IR_PARA_CurrDisChargeMin      IR_BmsPara->sys_para.curr_discharge_min 

#define IR_PARA_ChargeLowerVol         IR_BmsPara->sys_para.charge_lower_vol 
#define IR_PARA_ChargeStopVol      IR_BmsPara->sys_para.charge_stop_vol 
#define IR_PARA_ChargeStopSoc      IR_BmsPara->sys_para.charge_stop_soc 
#define IR_PARA_DisChargeLowerVol         IR_BmsPara->sys_para.discharge_lower_vol 
#define IR_PARA_DisChargeStopVol      IR_BmsPara->sys_para.discharge_stop_vol 
#define IR_PARA_DisChargeStopSoc      IR_BmsPara->sys_para.discharge_stop_soc 

#define IR_PARA_BalanceStartVolDiff      IR_BmsPara->sys_para.balance_start_diff_vol
#define IR_PARA_PasvBalanceStartVol      IR_BmsPara->sys_para.pasv_balance_start_vol

#define IR_PARA_FanStartTemp            IR_BmsPara->sys_para.fan_start_temp
#define IR_PARA_FanStopTemp             IR_BmsPara->sys_para.fan_stop_temp
#define IR_PARA_PrechargeVolDiff        IR_BmsPara->sys_para.precharge_diff_vol
#define IR_PARA_SampleTotalVolDiff      IR_BmsPara->sys_para.sample_total_vol_diff

#define IR_PARA_PowerOnComEn            IR_BmsPara->sys_para.powre_on_com_en 
/////////////////////////alarm_para//////////////////////////////////////////
#define IR_PARA_CellVolHigh_I       IR_BmsPara->alarm_para.cell_vol_high_I
#define IR_PARA_CellVolHigh_II      IR_BmsPara->alarm_para.cell_vol_high_II
#define IR_PARA_CellVolHigh_III     IR_BmsPara->alarm_para.cell_vol_high_III
#define IR_PARA_CellVolHigh_Fall    IR_BmsPara->alarm_para.cell_vol_high_fall

#define IR_PARA_CellVolLow_I       IR_BmsPara->alarm_para.cell_vol_low_I
#define IR_PARA_CellVolLow_II      IR_BmsPara->alarm_para.cell_vol_low_II
#define IR_PARA_CellVolLow_III     IR_BmsPara->alarm_para.cell_vol_low_III
#define IR_PARA_CellVolLow_Fall    IR_BmsPara->alarm_para.cell_vol_low_fall

#define IR_PARA_CellVolDiff_I       IR_BmsPara->alarm_para.cell_vol_diff_I
#define IR_PARA_CellVolDiff_II      IR_BmsPara->alarm_para.cell_vol_diff_II
#define IR_PARA_CellVolDiff_III     IR_BmsPara->alarm_para.cell_vol_diff_III
#define IR_PARA_CellVolDiff_Fall    IR_BmsPara->alarm_para.cell_vol_diff_fall

#define IR_PARA_ChargeTempHigh_I       IR_BmsPara->alarm_para.charge_temp_high_I
#define IR_PARA_ChargeTempHigh_II      IR_BmsPara->alarm_para.charge_temp_high_II
#define IR_PARA_ChargeTempHigh_III     IR_BmsPara->alarm_para.charge_temp_high_III
#define IR_PARA_ChargeTempHigh_Fall    IR_BmsPara->alarm_para.charge_temp_high_fall

#define IR_PARA_ChargeTempLow_I       IR_BmsPara->alarm_para.charge_temp_low_I
#define IR_PARA_ChargeTempLow_II      IR_BmsPara->alarm_para.charge_temp_low_II
#define IR_PARA_ChargeTempLow_III     IR_BmsPara->alarm_para.charge_temp_low_III
#define IR_PARA_ChargeTempLow_Fall    IR_BmsPara->alarm_para.charge_temp_low_fall

#define IR_PARA_DisChargeTempHigh_I       IR_BmsPara->alarm_para.discharge_temp_high_I
#define IR_PARA_DisChargeTempHigh_II      IR_BmsPara->alarm_para.discharge_temp_high_II
#define IR_PARA_DisChargeTempHigh_III     IR_BmsPara->alarm_para.discharge_temp_high_III
#define IR_PARA_DisChargeTempHigh_Fall    IR_BmsPara->alarm_para.discharge_temp_high_fall

#define IR_PARA_DisChargeTempLow_I       IR_BmsPara->alarm_para.discharge_temp_low_I
#define IR_PARA_DisChargeTempLow_II      IR_BmsPara->alarm_para.discharge_temp_low_II
#define IR_PARA_DisChargeTempLow_III     IR_BmsPara->alarm_para.discharge_temp_low_III
#define IR_PARA_DisChargeTempLow_Fall    IR_BmsPara->alarm_para.discharge_temp_low_fall

#define IR_PARA_CellTempDiff_I       IR_BmsPara->alarm_para.cell_temp_diff_I
#define IR_PARA_CellTempDiff_II      IR_BmsPara->alarm_para.cell_temp_diff_II
#define IR_PARA_CellTempDiff_III     IR_BmsPara->alarm_para.cell_temp_diff_III
#define IR_PARA_CellTempDiff_Fall    IR_BmsPara->alarm_para.cell_temp_diff_fall

#define IR_PARA_TotalVolHigh_I       IR_BmsPara->alarm_para.total_vol_high_I
#define IR_PARA_TotalVolHigh_II      IR_BmsPara->alarm_para.total_vol_high_II
#define IR_PARA_TotalVolHigh_III     IR_BmsPara->alarm_para.total_vol_high_III
#define IR_PARA_TotalVolHigh_Fall    IR_BmsPara->alarm_para.total_vol_high_fall

#define IR_PARA_TotalVolLow_I       IR_BmsPara->alarm_para.total_vol_low_I
#define IR_PARA_TotalVolLow_II      IR_BmsPara->alarm_para.total_vol_low_II
#define IR_PARA_TotalVolLow_III     IR_BmsPara->alarm_para.total_vol_low_III
#define IR_PARA_TotalVolLow_Fall    IR_BmsPara->alarm_para.total_vol_low_fall

#define IR_PARA_ChargeCurrHigh_I       IR_BmsPara->alarm_para.charge_curr_high_I
#define IR_PARA_ChargeCurrHigh_II      IR_BmsPara->alarm_para.charge_curr_high_II
#define IR_PARA_ChargeCurrHigh_III     IR_BmsPara->alarm_para.charge_curr_high_III
#define IR_PARA_ChargeCurrHigh_Fall    IR_BmsPara->alarm_para.charge_curr_high_fall

#define IR_PARA_DisChargeCurrHigh_I       IR_BmsPara->alarm_para.discharge_curr_high_I
#define IR_PARA_DisChargeCurrHigh_II      IR_BmsPara->alarm_para.discharge_curr_high_II
#define IR_PARA_DisChargeCurrHigh_III     IR_BmsPara->alarm_para.discharge_curr_high_III
#define IR_PARA_DisChargeCurrHigh_Fall    IR_BmsPara->alarm_para.discharge_curr_high_fall


#define IR_PARA_ResLow_I       IR_BmsPara->alarm_para.res_low_I
#define IR_PARA_ResLow_II      IR_BmsPara->alarm_para.res_low_II
#define IR_PARA_ResLow_III     IR_BmsPara->alarm_para.res_low_III
#define IR_PARA_ResLow_Fall    IR_BmsPara->alarm_para.res_low_fall

#define IR_PARA_PoleTempHigh_I       IR_BmsPara->alarm_para.pole_temp_high_I
#define IR_PARA_PoleTempHigh_II      IR_BmsPara->alarm_para.pole_temp_high_II
#define IR_PARA_PoleTempHigh_III     IR_BmsPara->alarm_para.pole_temp_high_III
#define IR_PARA_PoleTempHigh_Fall    IR_BmsPara->alarm_para.pole_temp_high_fall 


//////////////////////////BmsRunPara/////////////////////////
#define IR_PARA_CURR_ZERO           IR_BmsPara->bms_run_para.curr_zero 
#define IR_PARA_CURR_LOW            IR_BmsPara->bms_run_para.curr_low 
#define IR_PARA_CURR_LOWER_RATIO        IR_BmsPara->bms_run_para.curr_lower_ratio 
#define IR_PARA_CURR_LOWER_CYCLE        IR_BmsPara->bms_run_para.curr_lower_cycle 



#define IR_PARA_ENET_MAC        IR_BmsPara->device_para.enet_mac 
#define IR_PARA_ENET_DHCP_EN        IR_BmsPara->device_para.enet_dhcp_en 
#define IR_PARA_ENET_IP        IR_BmsPara->device_para.enet_ip 
#define IR_PARA_ENET_MASK        IR_BmsPara->device_para.enet_mask 
#define IR_PARA_ENET_GW        IR_BmsPara->device_para.enet_gw 
#define IR_PARA_ENET_Port        IR_BmsPara->device_para.enet_port 

////////////////////////////写参数///////////////////////////////////////////////




//////////////////////////////////////////////////////
#define ENEN_PARA_FPARA     1   //1:使能ENET参数由Fpara设定,否则使用默认参数。



extern const FparaInfoType* const IR_BmsPara;
extern  FparaInfoType* const IR_Write_BmsPara;


extern const BootInfoType* const IR_BootInfo;
extern BootInfoType* const IR_Write_BootInfo;
////////////////////////////////////////////////////////////
void FParaAspInit(void);
void FParaWriteMain(void);
void FParaSaveSet(EN_FPARA_WRITE_TYPE type);
void BootInfoSaveSet(EN_FPARA_WRITE_TYPE type);


#endif
