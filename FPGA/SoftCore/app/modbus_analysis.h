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
#include "systick.h"
#include "crc.h"
#include "uart.h"
#include "queue.h"
#define CACHE_BMS_BASE       (0x30010000UL)
#define CACHE_DCDC_BASE      (0x30020000UL)

#define gstBmsReg       ((BMSPara_t*)CACHE_BMS_BASE)
#define gstDCDCReg      ((DCDCPara_t*)CACHE_DCDC_BASE)

#define SLAVE_COMM_REV_MAX_TIME         300
#define SLAVE_COMM_INTERVAL_TIME        150
#define SLAVE_COMM_POLLING_TIME         150
#define SLAVE_COMM_BMS_INTERVAL_TIME    150

#define SLAVE_PCS_ADDR                  0x01
#define SLAVE_DCDC_ADDR                 0x02
#define SLAVE_MPPT_ADDR                 0X03
#define SLAVE_BMS_ADDR                  0x04

#define READ_INPUT_TYPE                 0x04
#define READ_HOLD_TYPE                  0x03

typedef enum
{
    EN_MODBUS_SLAVE_COMM_STATUS_OK,
    EN_MODBUS_SLAVE_COMM_STATUS_NOACK,
    EN_MODBUS_SLAVE_COMM_STATUS_ACK_ERROR,
    EN_MODBUS_SLAVE_COMM_STATUS_DISORDER
} EN_MODBUS_SLAVE_COMM_STATUS;

typedef struct _modbus_rev_03_stru_t
{
    uint8_t             data_num;
    uint8_t             *pRev;
} ModbusRev03_t;

typedef struct _modbus_rev_10_stru_t
{
    uint16_t            reg_start_addr;
    uint16_t            reg_num;
} ModbusRev10_t;

typedef struct _modbus_rev_error_stru_t
{
    uint8_t             err_code;
} ModbusRevError_t;

typedef struct _modbus_rev_data_stru_t
{
    uint8_t             slave_addr;
    uint8_t             func_code;
    ModbusRev03_t       rev_03;
    ModbusRev10_t       rev_10;
    ModbusRevError_t    rev_error;
} ModbusRevData_t;

typedef enum
{
    EN_SLAVE_NO_ERROR = 0,
    EN_SLAVE_ERROR_ILLEGAL_FUNC_CODE = 1,
    EN_SLAVE_ERROR_ILLEGAL_REG_ADDR = 2,
    EN_SLAVE_ERROR_ILLEGAL_WRITE_DATA = 3,
    EN_SLAVE_ERROR_HITCH = 4
} EN_SLAVE_ERROR_CODE;

typedef struct _modbus_read_reg_stru_t    //小端数据
{
    uint16_t dsp_wake_up_over               ;
} ModbusInputReg_t;

#pragma pack(1)
typedef struct _dc_dc_only_read_para_stru_t
{
    uint16_t        model_id                                        ;//0x0000
    char            sn[20]                                          ;
    uint16_t        hardware_version                                ;
    uint16_t        software_version_high                           ;
    uint16_t        software_version_low                            ;	
    char            version_extend[24]                              ;
    uint16_t        dc_dc_model_sel                                 ;    
    uint16_t        reserved0[5]                                    ;//bit3~bit0：0x0表示当前是FPGA程序为BOOT；bit3~bit0:  0x01表示当前FPGA程序为APP1；bit15~bit4:预留
    uint16_t        dc_output_voltage                               ;
    uint16_t        dc_output_current                               ;
    uint16_t        dc_output_power                                 ;
    uint16_t        LLC1_output_voltage                             ;
    uint16_t        LLC1_output_current                             ;
    uint16_t        LLC2_output_voltage                             ;
    uint16_t        LLC2_output_current                             ;
    uint16_t        buckboost1_output_voltage                       ;
    uint16_t        buckboost1_output_current                       ;
    uint16_t        buckboost2_output_voltage                       ;
    uint16_t        buckboost2_output_current                       ;
    uint16_t        reserved1[21]                                   ;
    uint16_t        fan1_speed                                      ;
    uint16_t        fan2_speed                                      ;
    uint16_t        reserved2[2]                                    ;
    uint16_t        dc_heatsink_temp1                               ;
    uint16_t        dc_heatsink_temp2                               ;
    uint16_t        reserved3[2]                                    ;
    uint16_t        work_status                                     ;
    uint16_t        pcshardwareid                                   ;    
    uint16_t        reserved4[22]                                   ;
    uint16_t        VoltageError                                    ;                                          
    uint16_t        PowerError                                      ;
    uint16_t        OtherError                                      ;
    uint16_t        reserved5[2]                                    ;
    uint16_t        PowerAlarm                                      ;
    uint16_t        OtherAlarm                                      ;
    uint16_t        reserved6[3]                                    ;
    uint16_t        PowerNotice                                     ;
    uint16_t        OtherNotice                                     ;
    uint16_t        reserved7[4]                                    ;
    uint16_t        recorden                                        ;
    uint16_t        record_addr_high                                ; 
    uint16_t        record_addr_low                                 ;     
    uint16_t        reserved8[29]                                   ;

} DCDCOnlyReadPara_t;
#pragma pack()

#pragma pack(1)
typedef struct _dc_dc_hold_para_stru_t
{
    uint16_t        ip_address_high                                 ; 
    uint16_t        ip_address_low                                  ;    
    uint16_t        pcs_device_type                                 ;
    uint16_t        led_status_led_r_value                          ;
    uint16_t        led_g_value_b_value                             ;
    uint16_t        led_mode                                        ;
    uint16_t        led_period                                      ;
    uint16_t        softreboot                                      ;//0x50:进APP0 x51:进BOOT
    uint16_t        recorden_status                                 ;//0x50:进APP0 x51:进BOOT	
    uint16_t        reserved0[7]                                    ;
    uint16_t        Shutdown                                        ;
    uint16_t        down_link_communication_enabled                 ;
    uint16_t        DCDC_OutputEn                                   ;
    int16_t         PackBalanceEn                                   ;//均衡功能
    uint16_t        updata_status                                   ;
    uint16_t        error_clear                                     ;
    uint16_t        FanForceEn                                      ;    
    uint16_t        reserved1[9]                                    ;
    uint16_t        reserved2[80]                                   ;

} DCDCHoldPara_t;
#pragma pack()

typedef struct _dc_dc_para_stru_t
{
    DCDCOnlyReadPara_t    dc_dc_only_read_para;
    DCDCHoldPara_t        dc_dc_hold_para;
} DCDCPara_t;

#pragma pack(1)
typedef struct _bms_read_reg_stru_t    //小端数据
{
    uint16_t        model_id                        ;//绝对地址0x7000
    char            sn[20]                          ;
    uint16_t        hardware_version                ;   
    uint16_t        software_version_high           ;
    uint16_t        software_version_low            ; 
    char            version_extend[24]              ;
    uint16_t        reserved1[6]                    ;
    uint16_t        packaddr                        ;//绝对地址0x7020
    uint16_t        packworkstatus                  ;
    uint16_t        pack_voltage                    ; //0.1v
    int16_t         cell_current                    ; //0.1A
    uint16_t        current_power                   ; //1w
    uint16_t        soc_real                        ;//0.1%
    uint16_t        soc_rpt                         ;//0.1%
    uint16_t        pack_health                     ;//0.1%
    int16_t         pack_max_chg_current            ;//0.1A
    int16_t         pack_max_dsg_current            ;//0.1A
    int16_t         reserved2[1]                    ;//预留可用于增加瞬态最大放电电流信息
    uint16_t        cell_std_capa_ah                ;//0.1Ah
    uint16_t        capacity_ah                     ;//0.1Ah
    uint16_t        pack_res_capa_ah                ;//0.1Ah
    uint16_t        pack_cycles                     ;  
    uint16_t        charging_energy_high            ;//0.1Wh    
    uint16_t        charging_energy_low             ;//0.1Wh    
    uint16_t        discharge_energy_high           ;//0.1Wh     
    uint16_t        discharge_energy_low            ;//0.1Wh  
    uint16_t        heat_status                     ;
    uint16_t        mos_status                      ;//电池包MOS(继电器状态)bit0- 放电MOS bit1- 充电MOS bit2-预放电MOS
    uint16_t        cell_max_voltage                ;//mv
    uint16_t        cell_min_voltage                ;//mv
    uint16_t        cell_average_voltage            ;//mv
    int16_t         cell_max_temp                   ;//0.1℃
    int16_t         cell_min_temp                   ;//0.1℃
    int16_t         cell_ave_temp                   ;//0.1℃
    int16_t         mos_max_temp                    ;//0.1℃
    int16_t         mos_min_temp                    ;//0.1℃
    int16_t         mos_ave_temp                    ;//0.1℃
    int16_t         pack_max_temp                   ;//0.1℃ 环境最高温度
    int16_t         pack_min_temp                   ;//0.1℃
    int16_t         pack_ave_temp                   ;//0.1℃    
    uint16_t        balance_status_high             ;     
    uint16_t        balance_status_low              ;  
    uint16_t        fltLevel                        ;
    uint16_t        reserved3[12]                   ;  
    uint16_t        pack_ov_lvl_high                ;
    uint16_t        pack_ov_lvl_low                 ;  
    uint16_t        pack_uv_lvl_high                ;
    uint16_t        pack_uv_lvl_low                 ;
    uint16_t        cell_ov_lvl1                    ;
    uint16_t        cell_ov_lvl2                    ;
    uint16_t        cell_ov_lvl3                    ;
    uint16_t        cell_uv_lvl1                    ;
    uint16_t        cell_uv_lvl2                    ;
    uint16_t        cell_uv_lvl3                    ;
    int16_t         cell_ot_lvl1                    ;
    int16_t         cell_ot_lvl2                    ;
    int16_t         cell_ot_lvl3                    ;
    int16_t         cell_ut_lvl                     ;
    int16_t         cell_ut_lv2                     ;
    int16_t         cell_ut_lv3                     ;    
    int16_t         mos_ot_lvl1                     ;
    int16_t         mos_ot_lvl2                     ;
    int16_t         mos_ot_lvl3                     ;
    int16_t         reserved4                       ;
    uint16_t        dsg_oc_lvl1                     ;
    uint16_t        dsg_oc_lvl2                     ;
    uint16_t        dsg_oc_lvl3                     ;
    uint16_t        chg_oc_lvl1                     ;
    uint16_t        chg_oc_lvl2                     ;
    uint16_t        chg_oc_lvl3                     ;
    uint16_t        reserved5[3]                    ;
    uint16_t        cell_voltage_diff               ;
    uint16_t        cell_temp_diff                  ;
    uint16_t        soh_under_lvl1                  ;
    uint16_t        soh_under_lvl2                  ;
    uint16_t        soh_under_lvl3                  ;    
    uint16_t        reserved6[14]                   ;
    uint16_t        cell_vdc[16]                    ;//mV
    int16_t         cell_temp[16]                   ;//0.1℃
    uint16_t        reserved7[16]                   ;
    uint16_t        mos_error                       ;
    uint16_t        temp_error                      ;
    uint16_t        curr_error                      ;
    uint16_t        volt_error                      ;
    uint16_t        sox_error                       ;
    uint16_t        other_error                     ;
    uint16_t        reserved8[3]                    ;
    uint16_t        mos_alarm                       ;
    uint16_t        temp_alarm                      ;
    uint16_t        curr_alarm                      ;
    uint16_t        volt_alarm                      ;
    uint16_t        sox_alarm                       ;
    uint16_t        other_alarm                     ;
    uint16_t        reserved9[3]                    ;
    uint16_t        mos_notice                      ;
    uint16_t        temp_notice                     ;
    uint16_t        curr_notice                     ;
    uint16_t        volt_notice                     ;
    uint16_t        sox_notice                      ;
    uint16_t        other_notice                    ;
    uint16_t        reserved10[3]                   ;
    uint16_t        reserved11[21]                  ;

} BMSOnlyReadPara_t;
#pragma pack()

#pragma pack(1)
typedef struct _bms_hold_reg_stru_t           //小端数据
{
    uint16_t        heat_ctl                        ;
    uint16_t        dsg_mos_ctl                     ;
    uint16_t        chg_mos_ctl                     ;
    uint16_t        pre_mos_ctl                     ;
    uint16_t        limit_curr_ctl                  ;
    uint16_t        reserved0[4]                    ;
    uint16_t        sys_upgrade_state               ;
    uint16_t        sys_reset                       ;
    uint16_t        reset_factory                   ; //5A A5
    uint16_t        bms_shutdown                    ;
    uint16_t        rtc_times_high                  ;    
    uint16_t        rtc_times_low                   ;    
    uint16_t        force_ocv                       ;
    uint16_t        all_pack_volt_ave               ;
    int16_t        current_calibrat_value           ;
    uint16_t        reserved1[14]                   ;  

} BMSHoldPara_t;
#pragma pack()

typedef struct _bms_para_stru_t
{
    BMSOnlyReadPara_t   bms_only_read_para;
    BMSHoldPara_t       bms_hold_para;
} BMSPara_t;

typedef struct _modbus_reg_stru_t           //小端数据
{
    uint16_t    battery_charging_voltage              ;
    uint16_t    dsp_wake_en                           ;
} ModbusHoldReg_t;

typedef enum
{
    EN_SLAVE_POLLING_STATUS_IDLE = 0,
    EN_SLAVE_POLLING_STATUS_WAITING_REV,
    EN_SLAVE_POLLING_STATUS_REV_OVERTIME,
    EN_SLAVE_POLLING_STATUS_REV_OK,
    EN_SLAVE_POLLING_STATUS_REV_INTERVAL_WAIT,
    EN_SLAVE_POLLING_STATUS_REV_ADD_BMS_INTERVAL_WAIT,
    EN_SLAVE_POLLING_STATUS_REV_ERROR,
} EN_SLAVE_POLLING_STATUS;

typedef struct _bms_slave_write_stru_t
{
    uint8_t             data_type;
    uint8_t             slave_addr;
    uint16_t            reg_addr;
    uint8_t             reg_num;
    uint16_t            word_data[10];
    uint8_t             byte_data[10];
} SlaveBmsWrite_t;

typedef struct _bms_private_write_stru_t
{
    uint32_t            reg_addr;
    uint16_t            reg_num;
    uint8_t             *p;
} PrivateBmsWrite_t;

extern BMSPara_t    gSlaveBMSPara;
extern DCDCPara_t   gTempSlaveDCDCPara;
void BmsRegInit(void);
void DCDCRegInit(void);
void BmsModBusHostWrite10(uint8_t slave_addr, uint16_t reg_start_addr, uint16_t reg_num, uint8_t data_num, uint16_t *pReg_value);
void ModbusCommProcess(void);
extern char  extend_version[24];
#endif

