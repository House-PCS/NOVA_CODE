
#ifndef __MODBUS_ANALYSIS_H_
#define __MODBUS_ANALYSIS_H_


#include "dev_modbus.h"
#include "dev_communication.h"
#include "sox.h"

#define MODBUS_HARDWARE_VERSION                 2000
#define MODBUS_DSG_MOS_CTL                      2140
#define MODBUS_CHG_MOS_CTL                      2141
#define MODBUS_PRE_MOS_CTL                      2142
#define MODBUS_ADD_MOS_CTL1                     2143
#define MODBUS_ADD_MOS_CTL2                     2144
#define MODBUS_RTC_TIMES_HIGH                   2147

#define MODBUS_CELL_UV_LVL1                     2150
#define MODBUS_CELL_UV_LVL2                     2151
#define MODBUS_CELL_UV_LVL3                     2152
#define MODBUS_CELL_OV_LVL1                     2153
#define MODBUS_CELL_OV_LVL2                     2154
#define MODBUS_CELL_OV_LVL3                     2155
#define MODBUS_PACK_OV_LVL_HIGH                 2157
#define MODBUS_PACK_OV_LVL_LOW                  2156
#define MODBUS_PACK_UV_LVL_HIGH                 2159
#define MODBUS_PACK_UV_LVL_LOW                  2158
#define MODBUS_CELL_OT_LVL1                     2160
#define MODBUS_CELL_OT_LVL2                     2161
#define MODBUS_CELL_OT_LVL3                     2162
#define MODBUS_CELL_UT_LVL                      2163
#define MODBUS_MOS_OT_LVL1                      2164
#define MODBUS_MOS_OT_LVL2                      2165
#define MODBUS_MOS_OT_LVL3                      2166
#define MODBUS_DSG_OC_LVL1                      2167
#define MODBUS_DSG_OC_LVL2                      2168
#define MODBUS_DSG_OC_LVL3                      2169
#define MODBUS_CHG_OC_LVL1                      2170
#define MODBUS_CHG_OC_LVL2                      2171
#define MODBUS_CHG_OC_LVL3                      2172
#define MODBUS_CELL_TEMP_DIFF                   2173
#define MODBUS_CELL_VOLTAGE_DIFF                2174
#define MODBUS_SOH_UNDER1                       2175
#define MODBUS_SOH_UNDER2                       2176
#define MODBUS_ERROR_CLEAR                      2177
#define MODBUS_FORCE_OCV                        2178

#define MODBUS_CHG_CURRENT_CALIBRAT1            2192
#define MODBUS_CHG_CURRENT_CALIBRAT2            2193
#define MODBUS_DSG_CURRENT_CALIBRAT1            2194
#define MODBUS_DSG_CURRENT_CALIBRAT2            2195



#define ENTER_TEST_MODE                         8000
#define OUTPU_IO_CTRL                           8001
#define INPUT_IO_DETECT                         8002

typedef enum
{
    EN_MODBUS_RTU,
    EN_MODBUS_TCP
} EN_MODBUS_TYPE;

typedef enum
{
    EN_MODBUS_REV_STATUS_IDLE,
    EN_MODBUS_REV_STATUS_ANALYSIS,
    EN_MODBUS_REV_STATUS_ACK,
    EN_MODBUS_REV_STATUS_ACK_WAIT,
    EN_MODBUS_READ_MPPT,
} EN_MODBUS_REV_STATUS;



typedef enum
{
    EN_MODBUS_FUNC_CODE_READ_INPUT = 0x04,
    EN_MODBUS_FUNC_CODE_READ_REMAIN = 0x03,
    EN_MODBUS_FUNC_CODE_WRITE_SINGLE = 0x06,
    EN_MODBUS_FUNC_CODE_WRITE_MULTI = 0x10
} EN_MODBUS_FUNC_CODE;

typedef enum
{
    EN_MODBUS_ERROR_CODE_INVALID_CMD = 0x01,
    EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR = 0x02,
    EN_MODBUS_ERROR_CODE_INVALID_DATA = 0x03,
} EN_MODBUS_ERROR_CODE;

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

} EN_EXP_SWITCH_STATUS;
#pragma pack(1)
typedef struct _modbus_read_stru_t
{
    uint8_t        dev_addr;
    uint8_t        func_code;
    uint8_t        byte_num;
    uint16_t       *pData;/*包含校验，实际有效长度由byte_num决定*/
} ModbusReadRemainAndInput_t;
#pragma pack()

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
    uint16_t        pack_voltage                    ; //0.01v
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

#pragma pack()

#pragma pack(1)
typedef struct _bms_hold_reg_stru_t                 //小端数据
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
    uint16_t        all_pack_volt_ave               ;//0.01v
    int16_t         current_calibrat_value          ;
    uint16_t        reserved1[14]                   ;    
} BMSHoldPara_t;
#pragma pack()

typedef struct _bms_para_stru_t
{
    BMSOnlyReadPara_t   bms_only_read_para;
    BMSHoldPara_t       bms_hold_para;
} BMSPara_t;

void ResetProcess(void);
void ModbusCmdProcess(void);
bool UpdateModbusReg(void);
void ModbusRegInit(void);
void SendRTCCalibrationEnable(void);
extern uint8_t slave_reg_serial;
extern uint8_t gLimitInfoType;
extern uint8_t gLimitInfoDatasize;
extern BMSPara_t            garrBMSReg;
#endif








