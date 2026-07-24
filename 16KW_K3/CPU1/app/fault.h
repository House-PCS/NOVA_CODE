/*
 * fault.h
 *
 *  Created on: 2025年3月22日
 *      Author: Nova007456
 */

#ifndef APP_FAULT_H_
#define APP_FAULT_H_

#include "F28x_Project.h"
#include "system.h"

#ifdef LOW_VOL_TEST
    /*      低压市电部分阈值        */
    #define GRID_VOLT_RMS_LOW_RECOVER       (25.0)//市电电压低恢复阈值
    #define GRID_VOLT_RMS_HIGH              (100.0)//市电电压高阈值
    #define GRID_VOLT_RMS_HIGH_RECOVER      (95.0)//市电电压高恢复阈值
    #define GRID_VOLT_RMS_LOW              (20.0)//市电电压高阈值

    #define GRID_FREQ_LOW                   (45.0)//市电频率低阈值
    #define GRID_FREQ_LOW_RECOVER           (46.0)//市电频率低恢复阈值
    #define GRID_FREQ_HIGH                  (55.0)//市电频率低阈值
    #define GRID_FREQ_HIGH_RECOVER          (54.0)//市电频率低恢复阈值
    /*      低压发电机部分阈值        */
    #define GEN_VOLT_RMS_LOW                (20.0)//发电机电压低阈值
    #define GEN_VOLT_RMS_LOW_RECOVER        (25.0)//发电机电压低恢复阈值
    #define GEN_VOLT_RMS_HIGH               (100.0)//发电机电压高阈值
    #define GEN_VOLT_RMS_HIGH_RECOVER       (95.0)//发电机电压高恢复阈值
    #define GEN_FREQ_LOW                    (45.0)//发电机频率低阈值
    #define GEN_FREQ_LOW_RECOVER            (46.0)//发电机频率低恢复阈值
    #define GEN_FREQ_HIGH                   (55.0)//发电机频率低阈值
    #define GEN_FREQ_HIGH_RECOVER           (54.0)//发电机频率低恢复阈值
    /*      低压逆变部分阈值        */
    #define INV_VOLT_REF                    (70.0)//逆变电压给定
    #define INV_VOLT_RMS_LOW                (20.0)//逆变电压低阈值
    #define INV_VOLT_RMS_LOW_RECOVER        (25.0)//逆变电压低恢复阈值
    #define INV_VOLT_RMS_HIGH               (100.0)//逆变电压高阈值
    #define INV_VOLT_RMS_HIGH_RECOVER       (95.0)//逆变电压高恢复阈值
    /*      低压母线部分阈值        */
    #define BUS_VOLT_HIGH                   (250.0)//Bus电压高阈3值
    #define BUS_VOLT_HIGH_RECOVER           (200.0)//Bus电压高恢复阈值
    #define BUS_VOLT_LOW_FAULT                    (80.0f)//Bus电压低阈值
    #define BUS_VOLT_LOW_RECOVER            (85.0)//Bus电压低恢复阈值
    #define BUS_PEAK_VOLT_HIGH              (260.0)//Bus电压高瞬时阈值
    #define BUS_PEAK_VOLT_HIGH_RECOVER      (240.0)//Bus电压高瞬时恢复阈值
    #define BUS_PEAK_VOLT_LOW               (20.0)//Bus电压低瞬时阈值
    #define BUS_PEAK_VOLT_LOW_RECOVER       (25.0)//Bus电压低恢复瞬时阈值
    /*      PV部分阈值        */
    #define PV_VOLT_HIGH                    (1000.0)//PV电压高阈值
    #define PV_VOLT_HIGH_RECOVER            (990.0)//PV电压高恢复阈值
    #define PV_VOLT_LOW                     (160.0)//PV电压低阈值
    #define PV_VOLT_LOW_RECOVER             (170.0)//PV电压低恢复阈值
    #define PV_CURR_HIGH                    (16.0)//PV电流高阈值
    #define PV_CURR_HIGH_RECOVER            (14.0)//PV电流高恢复阈值

    #define ERROR_THRESHOLD_BUS_CURR_OVER       (10)

    #define ERROR_THRESHOLD_INV_CURR_OVER       (18)
    #define ERROR_THRESHOLD_INV_CURR_OVER_RMS   (10)
    #define ERROR_THRESHOLD_INV_VOLT_OVER       (160)//逆变过压阈值
    #define ERRORDLY_THRESHOLD_INV_VOLT_OVER    (135) //逆变过压阈值
    #define ERRORDLY_THRESHOLD_INV_CURR_OVER    (12)//逆变过流阈值
#else
    /*      市电部分阈值        */
    #define GRID_VOLT_RMS_LOW               (176.0)//逆变电压低阈值
    #define GRID_VOLT_RMS_LOW_RECOVER       (181.0)//逆变电压低恢复阈值
    #define GRID_VOLT_RMS_HIGH              (265.0)//逆变电压高阈值
    #define GRID_VOLT_RMS_HIGH_RECOVER      (253.0)//逆变电压高恢复阈值
    #define GRID_FREQ_LOW                   (45.0)//市电频率低阈值
    #define GRID_FREQ_LOW_RECOVER           (46.0)//市电频率低恢复阈值
    #define GRID_FREQ_HIGH                  (55.0)//市电频率低阈值
    #define GRID_FREQ_HIGH_RECOVER          (54.0)//市电频率低恢复阈值

    /*      发电机部分阈值        */
    #define GEN_VOLT_RMS_LOW                (176.0)//发电机电压低阈值
    #define GEN_VOLT_RMS_LOW_RECOVER        (181.0)//发电机电压低恢复阈值
    #define GEN_VOLT_RMS_HIGH               (265.0)//发电机电压高阈值
    #define GEN_VOLT_RMS_HIGH_RECOVER       (258.0)//发电机电压高恢复阈值

    #define GEN_FREQ_LOW                    (40.0)//发电机频率低阈值
    #define GEN_FREQ_LOW_RECOVER            (42.0)//发电机频率低恢复阈值
    #define GEN_FREQ_HIGH                   (60.0)//发电机频率低阈值
    #define GEN_FREQ_HIGH_RECOVER           (57.0)//发电机频率低恢复阈值

    /*      高压逆变部分阈值        */
    #define INV_VOLT_REF                    (325.0)//逆变电压给定
    #define INV_VOLT_RMS_LOW                (176.0)//逆变电压低阈值
    #define INV_VOLT_RMS_LOW_RECOVER        (181.0)//逆变电压低恢复阈值
    #define INV_VOLT_RMS_HIGH               (264.0)//逆变电压高阈值
    #define INV_VOLT_RMS_HIGH_RECOVER       (259.0)//逆变电压高恢复阈值


    /*      高压母线部分阈值        */
    #define BUS_VOLT_HIGH                   (450.0)//Bus电压高阈值
    #define BUS_PEAK_VOLT_HIGH                   (550.0)//Bus电压高阈值
//    #define BUS_VOLT_HIGH_RECOVER           (450.0)//Bus电压高恢复阈值
    #define BUS_VOLT_LOW_FAULT                    (320.0)//Bus电压低阈值
//    #define BUS_VOLT_LOW_RECOVER            (330.0)//Bus电压低恢复阈值
//    #define BUS_PEAK_VOLT_HIGH              (530.0)//Bus电压高瞬时阈值
//    #define BUS_PEAK_VOLT_HIGH_RECOVER      (500.0)//Bus电压高瞬时恢复阈值
//    #define BUS_PEAK_VOLT_LOW               (200.0)//Bus电压低瞬时阈值
//    #define BUS_PEAK_VOLT_LOW_RECOVER       (250.0)//Bus电压低恢复瞬时阈值

    /*      PV部分阈值        */
    #define PV_VOLT_HIGH                    (1000.0)//PV电压高阈值
    #define PV_VOLT_HIGH_RECOVER            (990.0)//PV电压高恢复阈值

    #define PV_VOLT_LOW                     (160.0)//PV电压低阈值
    #define PV_VOLT_LOW_RECOVER             (175.0)//PV电压低恢复阈值

    #define PV_CURR_HIGH                    (17.0)//PV电压高阈值
    #define PV_CURR_HIGH_RECOVER            (15.0)//PV电压高恢复阈值

    #define ERROR_THRESHOLD_BUS_CURR_OVER       (48)

    #define ERROR_THRESHOLD_INV_CURR_OVER       (45)
    #define ERROR_THRESHOLD_INV_CURR_OVER_RMS   (32)
    #define ERROR_THRESHOLD_INV_VOLT_OVER       (450)//逆变过压阈值
    #define ERRORDLY_THRESHOLD_INV_VOLT_OVER    (400) //逆变过压阈值
    #define ERRORDLY_THRESHOLD_INV_CURR_OVER    (26)//逆变过流阈值

#endif

#define ERRORDLY_INV_TEMPERATURE_OVER                   (110)
#define ERRORDLY_PV_TEMPERATURE_OVER                    (108)

#define ERRORDLY_THRESHOLD_BOX_TEMPERATURE_OVER              (105.0)//箱内温度高阈值

#define ERROR_THRESHOLD_LOAD_OVER_L2                        (19500)//过载等级2阈值
#define ERROR_THRESHOLD_INV_PWR_OVER    (19000)
#define ERROR_THRESHOLD_GRID_PWR_OVER   (19000)

#define ERROR_THRESHOLD_LOAD_OVER_L1                        (18000)//过载等级1阈值，1.1倍过载允许

#define WARNDLY_THRESHOLD_PWR_OVER                          (16500)

typedef enum
{
    EN_WARNING_GRID_VOL_LOW,
    EN_WARNING_GRID_VOL_OVER,
    EN_WARNING_GRID_FREQ_LOW,
    EN_WARNING_GRID_FREQ_OVER,
    EN_WARNING_PV1_VOL_LOW,
    EN_WARNING_PV2_VOL_LOW,
    EN_WARNING_PV3_VOL_LOW,
    EN_WARNING_PV4_VOL_LOW,
    EN_WARNING_PV_LOSS_CONTROL,
    EN_WARNING_OVER_LOAD,
    EN_WARNING_LEAK_CURR_HIGH,                        //直接更改为故障
    EN_WARNING_FAN_FAULT,

    EN_WARNING_CT_R_REVERSE,                          //需补充
    EN_WARNING_CT_S_REVERSE,
    EN_WARNING_CT_T_REVERSE,

    EN_WARNING_GEN_VOL_LOW,
    EN_WARNING_GEN_VOL_OVER,
    EN_WARNING_GEN_FREQ_LOW,
    EN_WARNING_GEN_FREQ_OVER,
}EN_WARNING;


typedef union
{
    uint16_t byte[2];
    struct
    {
        uint16_t grid_volt_low          : 1;          // bit:00    电网电压低
        uint16_t grid_volt_high         : 1;          // bit:01    电网电压高
        uint16_t grid_freq_low          : 1;          // bit:02    电网频率低
        uint16_t grid_freq_high         : 1;          // bit:03    电网频率高
        uint16_t pv1_no_connected       : 1;          // bit:04    PV1未接
        uint16_t pv2_no_connected       : 1;          // bit:05    PV2未接
        uint16_t pv3_no_connected       : 1;          // bit:06    PV3未接
        uint16_t pv4_no_connected       : 1;          // bit:07    PV4未接
        uint16_t pv_loss_control        : 1;          // bit:08    PV失控
        uint16_t overload               : 1;          // bit:09    过载报警
        uint16_t leak_curr_high         : 1;          // bit:10    漏电流高
        uint16_t fan_error              : 1;          // bit:11    风扇故障
        uint16_t r_ct_reverse           : 1;          // bit:12    R_CT接反
        uint16_t s_ct_reverse           : 1;          // bit:13    S_CT接反
        uint16_t t_ct_reverse           : 1;          // bit:14    T_CT接反
        uint16_t gen_volt_low           : 1;          // bit:15    电网电压低
        uint16_t gen_volt_high          : 1;          // bit:16    电网电压高
        uint16_t gen_freq_low           : 1;          // bit:17    电网频率低
        uint16_t gen_freq_high          : 1;          // bit:18    电网频率高
        uint32_t reserve19_31           : 13;         // bit:19-31 保留
    } bit;
} DspWarningCode_t;

typedef enum
{
    EN_ERR_SOFT_START_OVERTIME,
    EN_ERR_INV_SHORT,                                 //需补充
    EN_ERR_INV_VOL_LOW,
    EN_ERR_INV_VOLT_OVER,
    EN_ERR_BUS_DSG_CURR_OVER,
    EN_ERR_BUS_CHG_CURR_OVER,

    EN_ERR_BUS_VOL_LOW,
    EN_ERR_BUS_VOL_OVER,
    EN_ERR_DCDC_SHORT_ERR,
    EN_ERR_HW_CURR_ERR,
    EN_ERR_INV_CUR_OVER,
    EN_ERR_INV_VOL_DC_OFFSET_OVER,

    EN_ERR_INV_CASE_OVERHEAT,
    EN_ERR_INV_RADIATOR_OVERHEAT,
    EN_ERR_HARDWARE_CMPSS_OVER_CUR,
    EN_ERR_INV_OUTPUT_ERR,

    EN_ERR_OVERLOAD_1,
    EN_ERR_OVERLOAD_2,

    EN_ERR_LEAKCURR_HIGH,

    EN_ERR_CT_R_DISCONNECT,//19-25检测功能暂未开发
    EN_ERR_CT_S_DISCONNECT,
    EN_ERR_CT_T_DISCONNECT,
    EN_ERR_RELAY_FAULT,
    EN_ERR_SAMPLING_SENSOR,
    EN_ERR_TEM_SENSOR,
    EN_ERR_PV_REVERSE_CONNECT,
    EN_ERR_ARCINS_FAULT,

    EN_ERR_RESERVE_0_FAULT,
    EN_ERR_RESERVE_1_FAULT,
    EN_ERR_RESERVE_2_FAULT,
    EN_ERR_RESERVE_3_FAULT,
    EN_ERR_COM_FAULT,
}EN_ERROR;


typedef enum
{
    EN_TIME_DIV_1,
    EN_TIME_DIV_2,
    EN_TIME_DIV_3,
}EN_TIME_DIV;



typedef union
{
    uint16_t byte[2];
    struct
    {
        uint16_t start_timeout          : 1;          // bit:00    软启超时
        uint16_t inv_out_short          : 1;          // bit:01    逆变器输出短路
        uint16_t inv_volt_low           : 1;          // bit:02    逆变电压低
        uint16_t inv_volt_high          : 1;          // bit:03    逆变电压高
        uint16_t bus_dsg_over_curr      : 1;          // bit:04    bus放电过流
        uint16_t bus_chg_over_curr      : 1;          // bit:05    bus充电过流
        uint16_t bus_volt_low           : 1;          // bit:06    bus欠压
        uint16_t bus_volt_high          : 1;          // bit:07    bus过压
        uint16_t bus_short              : 1;          // bit:08    bus短路
        uint16_t hardware_fault         : 1;          // bit:09    硬件故障       /*需补充该处*/
        uint16_t inv_over_curr          : 1;          // bit:10    逆变过流
        uint16_t dc_component_high      : 1;          // bit:11    直流分量高
        uint16_t box_over_temp          : 1;          // bit:12    箱体内环境过温
        uint16_t radiator_over_temp     : 1;          // bit:13    散热器过温
        uint16_t inv_tz_fault           : 1;          // bit:14    TZ保护故障(CMPSS故障)
        uint16_t inv_out_fault          : 1;          // bit:15    逆变输出异常

        uint16_t always_over_load1      : 1;          // bit:16    逆变输出过载1
        uint16_t always_over_load2      : 1;          // bit:17    逆变输出过载2
        uint16_t leakcurr_high          : 1;          // bit:18    漏电流高
        uint16_t r_ct_disconnect        : 1;          // bit:19    R相CT断线
        uint16_t s_ct_disconnect        : 1;          // bit:20    S相CT断线
        uint16_t t_ct_disconnect        : 1;          // bit:21    T相CT断线
        uint16_t delay_fault            : 1;          // bit:22    继电器故障
        uint16_t samping_sensor         : 1;          // bit:23    采样传感器故障
        uint16_t tem_sensor             : 1;          // bit:24    温度传感器故障
        uint16_t pv_reverse             : 1;          // bit:25    PV反接
        uint16_t arcins_fault           : 1;          // bit:26    拉弧绝缘检测故障
        uint16_t reserve27_30           : 4;          // bit:28-30 预留
        uint16_t always_comm_error      : 1;          // bit:31    主从通讯错误         /*需补充*/

    } bit;
} DspErrorCode_t;
typedef enum
{
    EN_ERR_GRID_VOL_HIGH_FAULT,
    EN_ERR_GRID_VOL_LOW_FAULT,
    EN_ERR_GRID_CURR_OVER_FAULT,

    EN_ERR_EPS_VOL_HIGH_FAULT,
    EN_ERR_EPS_VOL_LOW_FAULT,
    EN_ERR_EPS_CURR_OVER_FAULT,

    EN_ERR_GEN_VOL_HIGH_FAULT,
    EN_ERR_GEN_VOL_LOW_FAULT,
    EN_ERR_GEN_CURR_OVER_FAULT,
    EN_ERR_EARTH_FAULT,
}EN_ERROR1;
typedef union
{
    uint16_t byte[2];
    struct
    {
        uint16_t grid_volt_high          : 1;
        uint16_t grid_volt_low           : 1;
        uint16_t grid_over_curr           : 1;
        uint16_t eps_volt_high          : 1;
        uint16_t eps_volt_low           : 1;
        uint16_t eps_over_curr           : 1;
        uint16_t gen_volt_high          : 1;
        uint16_t gen_volt_low           : 1;
        uint16_t gen_over_curr           : 1;
        uint16_t earth_fault           : 1;
        uint16_t reserve1                : 6;
        uint32_t reserve16_31            : 16;         // bit:0-31  预留
    } bit;
} InsideFault_t;


typedef struct _fault_info_stru_t
{
    DspWarningCode_t        inside_warn;         //DSP不上报的内部告警
    DspErrorCode_t          inside_err;          //DSP不上报的内部故障
    InsideFault_t           inside_add_fault;    //DSP不上报的内部故障
    uint16_t                delay_count;         //连续检测计数
    uint16_t                err_flag;            //故障标志
    uint16_t                warning_flag;        //警告标志

} FaultInfo_t;


#include "cond_cum_countTrigger.h"

typedef CondCum_CountTrigger_t CountTrigger_t;

typedef struct
{
    bool grid_volt_low_flag;
    bool grid_volt_high_flag;
    bool grid_freq_low_flag;
    bool grid_freq_high_flag;
    bool pv1_no_connected_flag;
    bool pv2_no_connected_flag;
    bool pv3_no_connected_flag;
    bool pv4_no_connected_flag;
    bool pv_loss_control_flag;
    bool gen_volt_low_flag;
    bool gen_volt_high_flag;
    bool gen_freq_low_flag;
    bool gen_freq_high_flag;
} ElseModuleWarningFlag_t;

typedef struct
{
    bool inv_out_short_flag;
    bool inv_volt_low_flag;
    bool inv_volt_high_flag;
    bool inv_over_curr_flag;
    bool dc_component_high_flag;
    bool inv_tz_fault_flag;
    bool inv_out_fault_flag;
    bool pv_reverse_flag;
} ElseModuleErrFlag_t;

typedef struct
{
    float max_dsg_curr;
    float max_chg_curr;
    float max_bus_short_curr;

    float max_total_inv_warning_power;
    float max_total_grid_warning_power;
    float max_total_gen_warning_power;
    float max_total_load_warning_power;
    float max_total_ct_warning_power;

    float max_total_inv_fault_power_level1;
    float max_total_grid_fault_power_level1;
    float max_total_gen_fault_power_level1;
    float max_total_load_fault_power_level1;
    float max_total_ct_fault_power_level1;

    float max_total_inv_fault_power_level2;
    float max_total_grid_fault_power_level2;
    float max_total_gen_fault_power_level2;
    float max_total_load_fault_power_level2;
    float max_total_ct_fault_power_level2;

    float bus_curr;
    float inv_total_power;
    float inv_a_power;
    float inv_b_power;
    float inv_c_power;

    float load_total_power;
    float load_a_power;
    float load_b_power;
    float load_c_power;

    float grid_total_power;
    float grid_a_power;
    float grid_b_power;
    float grid_c_power;

    float gen_total_power;
    float gen_a_power;
    float gen_b_power;
    float gen_c_power;

    float ct_total_power;
    float ct_r_power;
    float ct_s_power;
    float ct_t_power;

    float bus_pos;
    float bus_neg;
    float leak_current;
    uint16_t iso_gpio_state;
    uint16_t  inv_state_ok;
    uint16_t  gird_state_ok;
    uint16_t  gen_state_ok;
} ElseModuleData_t;


typedef struct _module_fault_info_stru_t
{
    ElseModuleWarningFlag_t warning_flag;
    ElseModuleErrFlag_t   err_flag;
    ElseModuleData_t      in_data;
} ModuleFaultAndDataInfo_t;


uint16_t GetWarn_GridVoltLow(void);
uint16_t GetWarn_GridVoltHigh(void);
uint16_t GetWarn_GridFreqLow(void);
uint16_t GetWarn_GridFreqHigh(void);
uint16_t GetWarn_Pv1NoConnected(void);
uint16_t GetWarn_Pv2NoConnected(void);
uint16_t GetWarn_Pv3NoConnected(void);
uint16_t GetWarn_Pv4NoConnected(void);
uint16_t GetWarn_PvLoseControl(void);
uint16_t GetWarn_OverLoad(void);
uint16_t GetWarn_LeakCurrHigh(void);
uint16_t GetWarn_FanError(void);
uint16_t GetWarn_R_Ct_Reverse(void);
uint16_t GetWarn_S_Ct_Reverse(void);
uint16_t GetWarn_T_Ct_Reverse(void);
uint16_t GetWarn_GenVoltLow(void);
uint16_t GetWarn_GenVoltHigh(void);
uint16_t GetWarn_GenFreqLow(void);
uint16_t GetWarn_GenFreqHigh(void);

uint16_t GetError_StartTimeout(void);
uint16_t GetError_InvOutShort(void);
uint16_t GetError_InvVoltLow(void);
uint16_t GetError_InvVoltHigh(void);
uint16_t GetError_BusDsgOverCurr(void);
uint16_t GetError_BusChgOverCurr(void);
uint16_t GetError_BusVoltLow(void);
uint16_t GetError_BusVoltHigh(void);
uint16_t GetError_BusShort(void);
uint16_t GetError_HardwareFault(void);
uint16_t GetError_InvOverCurr(void);
uint16_t GetError_DcComponentHigh(void);
uint16_t GetError_BoxOverTemp(void);
uint16_t GetError_RadiatorOverTemp(void);
uint16_t GetError_InvTzFault(void);
uint16_t GetError_InvOutFault(void);
uint16_t GetError_InvOverLoad1(void);
uint16_t GetError_InvOverLoad2(void);
uint16_t GetError_R_Ct_Disconnect(void);
uint16_t GetError_S_Ct_Disconnect(void);
uint16_t GetError_T_Ct_Disconnect(void);
uint16_t GetError_IsoAndArcFault(void);
uint16_t GetError_Adc_Calibration_Fault(void);
uint16_t GetError_Comm_Error(void);

extern FaultInfo_t gstFaultInfo;
/*初始化*/
void ErrWarningCheckInit();
/*输入接口*/
void FastErrWarningHandler(void);
void SlowErrWarningHandler(void);
uint16_t GetErrorFlag(void);
uint16_t LoadRelayNoneFaultEn(void);
#endif /* APP_FAULT_H_ */
