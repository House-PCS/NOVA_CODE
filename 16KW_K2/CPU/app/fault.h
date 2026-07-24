/*
 * fault.h
 *
 *  Created on: 2025年3月26日
 *      Author: Nova006352
 */

#ifndef APP_FAULT_H_
#define APP_FAULT_H_

#include "F28x_Project.h"

/*      告警       包含恢复阈值 */
#define WARNING_THRESHOLD_GRID_VOLT_LOW                (175.0)//电网电压低阈值
#define WARNING_THRESHOLD_GRID_VOLT_LOW_RECOVER        (205.0)//电网电压低恢复阈值

#define WARNING_THRESHOLD_GRID_VOLT_OVER               (275)  //电网电压高阈值
#define WARNING_THRESHOLD_GRID_VOLT_OVER_RECOVER       (253)  //电网电压高恢复阈值

#define WARNING_THRESHOLD_GRID_FREQ_LOW                (45)   //电网欠频阈值
#define WARNING_THRESHOLD_GRID_FREQ_LOW_RECOVER        (47.7)   //电网欠频阈值
#define WARNING_THRESHOLD_GRID_FREQ_OVER               (55)   //电网过频阈值
#define WARNING_THRESHOLD_GRID_FREQ_OVER_RECOVER       (51.15)   //电网过频阈值


#define WARNING_THRESHOLD_OIL_VOLT_LOW                (175.0)//油机电压低阈值
#define WARNING_THRESHOLD_OIL_VOLT_LOW_RECOVER        (205.0)//油机电网电压低恢复阈值

#define WARNING_THRESHOLD_OIL_VOLT_OVER               (275)  //油机电网电压高阈值
#define WARNING_THRESHOLD_OIL_VOLT_OVER_RECOVER       (253)  //油机电网电压高恢复阈值


#define WARNING_THERSHOLD_OIL_FREQ_LOW                  (41)    //油机欠频阈值
#define WARNING_THERSHOLD_OIL_FREQ_LOW_RECOVER          (45)  //
#define WARNING_THERSHOLD_OIL_FREQ_OVER                 (60)    //油机过频阈值
#define WARNING_THERSHOLD_OIL_FREQ_OVER_RECOVER         (55)

#define WARNING_THRESHOLD_PV1_VOLT_LOW                 (75)//PV未接用电压低来表示
#define WARNING_THRESHOLD_PV2_VOLT_LOW                 (75)//PV未接用电压低来表示
#define WARNING_THRESHOLD_PV3_VOLT_LOW                 (75)//PV未接用电压低来表示
#define WARNING_THRESHOLD_PV4_VOLT_LOW                 (75)//PV未接用电压低来表示
#define WARNING_THRESHOLD_LOAD_OVER                    (16100)//过载警告
#define WARNING_THRESHOLD_LOAD_OVER_RECOVER            (16000)//过载警告

#define WARNING_THRESHOLD_LEAK_CURR_OVER                (650)    //漏电流传感器的绝对值超过650，即漏电流达到约±180ma触发报警

#define ERRORDLY_RADIATOR_TEMPERATURE_OVER              (100)   //散热器温度过温保护阈值

/*      故障       暂未包含恢复阈值 */
#define ERROR_THRESHOLD_SOFT_TIME_OVER                     (30)  //软起超时阈值（S）
//#define ERROR_THRESHOLD_INV_VOLT_LOW                       (175) //逆变瞬时值欠压阈值
#define ERROR_THRESHOLD_INV_VOLT_OVER                      (420)//逆变瞬时值过压阈值
#define ERRORDLY_THRESHOLD_INV_VOLT_LOW                    (170) //逆变有效值欠压阈值
#define ERRORDLY_THRESHOLD_INV_VOLT_OVER                   (275) //逆变有效值过压阈值
#define ERRORDLY_THRESHOLD_INV_VOLT_LOW_RECOVER           (205) //逆变有效值欠压恢复阈值
#define ERRORDLY_THRESHOLD_INV_VOLT_OVER_RECOVER          (253) //逆变有效值过压恢复阈值
#define ERRORDLY_THRESHOLD_INV_CURR_OVER                   (80)//逆变过流阈值

#define ERROR_THRESHOLD_INV_VOLT_SHORT_CIRCUIT             (175) //逆变短路电压阈值
#define ERROR_THRESHOLD_INV_CURR_SHORT_CIRCUIT             (75)  //逆变短路电流阈值

#define ERRORDLY_THRESHOLD_GRID_PWR_OVER                   (17800)
#define ERROR_THRESHOLD_GRID_PWR_OVER                      (19000)//认证设置为19kw，之前为17.5kw
#define ERROR_THRESHOLD_INV_CURR_OVER                      (160) //逆变过流阈值(17kw)
#define ERRORDLY_THRESHOLD_INV_PWR_OVER                    (17800)
#define ERROR_THRESHOLD_INV_PWR_OVER                       (19000)//认证设置为19kw，之前为17.5kw

#define ERROR_THRESHOLD_BUS_CURR_DSG_OVER                   (-50)//母线放电过流阈值,母线放电电流为负
#define ERROR_THRESHOLD_BUS_CURR_CHG_OVER                   (50)//母线充电过流阈值,
#define ERROR_THRESHOLD_BUS_VOLT_OVER                       (550)//母线过压阈值
#define ERROR_THRESHOLD_BUS_VOLT_LOW                        (370)//母线欠压阈值
#define ERROR_THRESHOLD_BUS_VOLT_SHORT_CIRCUIT              (50)//逆变短路电压阈值
#define ERROR_THRESHOLD_BUS_CURR_CHG_SHORT_CIRCUIT           (2)//充电情况下，逆变短路电流阈值
#define ERROR_THRESHOLD_BUS_CURR_DSG_SHORT_CIRCUIT           (-2)//放电情况下，逆变短路电流阈值


//#define ERROR_THRESHOLD_DC_COMPONENT_OVER                     //直流分量高阈值
#define ERRORDLY_THRESHOLD_BOX_TEMPERATURE_OVER              (80.0)//箱内温度高阈值
#define ERRORDLY_THRESHOLD_RADIATOR_TEMPERATURE_OVER         (85.0)    //散热器温度高阈值

#define ERROR_THRESHOLD_CLASSIS_TEMPERATURE_OVER            (60.0) //首次开机，环境温度高阈值
#define ERROR_THRESHOLD_CLASSIS_TEMPERATURE_LOW             (-22.0) //首次开机，环境温度低阈值
#define ERROR_THRESHOLD_RADIATOR_TEMPERATURE_OVER           (85.0)
#define ERROR_THRESHOLD_INV_TEMPERATURE_OVER                (95.5)
#define ERROR_THRESHOLD_PV_TEMPERATURE_OVER                 (95.5)

#define ERROR_THRESHOLD_CLASSIS_TEMPERATURE_OVER_RECOVER   (55.0) //环境温度高恢复阈值
#define ERROR_THRESHOLD_CLASSIS_TEMPERATURE_LOW_RECOVER    (-20.0) //环境温度低恢复阈值
#define ERROR_THRESHOLD_RADIATOR_TEMPERATURE_OVER_RECOVER  (70.0) //散热器温度高恢复阈值
#define ERROR_THRESHOLD_PV_TEMPERATURE_OVER_RECOVER        (80.0) //PV温度高恢复阈值
#define ERROR_THRESHOLD_INV_TEMPERATURE_OVER_RECOVER       (80.0) //逆变温度高恢复阈值

#define ERROR_THRESHOLD_LOAD_OVER_L1                        (17800)//过载等级1阈值，1.1倍过载允许
#define ERROR_THRESHOLD_LOAD_OVER_L2                        (19000)//过载等级2阈值

#define ERROR_THRESHOLD_INS_RES                             (17.0)  //绝缘阻抗阈值


typedef enum
{
    EN_TIME_DIV_1,
    EN_TIME_DIV_2,
    EN_TIME_DIV_3,
    EN_TIME_DIV_4,
    EN_TIME_DIV_5,
}EN_TIME_DIV;


typedef union
{
    uint16_t byte[2];
    struct
    {
        uint32_t reserve0_31            : 32;         // bit:0-31  预留
    } bit;
} InsideFault_t;


//*********************************************ERROR********************************************************/
/*
    ERR Part: Grid ERROR
*/
typedef enum
{
    EN_GRID_ERR_VOL_HIGH,
    EN_GRID_ERR_VOL_LOW,
    EN_GRID_ERR_FRE_HIGH,
    EN_GRID_ERR_FRE_LOW,

}EN_Grid_ERROR;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t grid_volt_high          : 1;          // bit:00    电网电压低
        uint16_t grid_volt_low           : 1;          // bit:01    电网电压高
        uint16_t grid_freq_high          : 1;          // bit:02    电网频率低
        uint16_t grid_freq_low           : 1;          // bit:03    电网频率高
        uint16_t reserve04_15            : 12;         // 预留
    } bit;
} Dsp_GridErrorCode_t;


/*
    ERR Part: PV ERROR
*/
typedef enum
{
    EN_PV_ERR_PV1_SHORT,
    EN_PV_ERR_PV2_SHORT,
    EN_PV_ERR_PV3_SHORT,
    EN_PV_ERR_PV4_SHORT,
    EN_PV_ERR_PV1_REVERSE, 
    EN_PV_ERR_PV2_REVERSE,
    EN_PV_ERR_PV3_REVERSE,
    EN_PV_ERR_PV4_REVERSE,
    EN_PV_ERR_PV1_LOSE_CONTROL,
    EN_PV_ERR_PV2_LOSE_CONTROL, 
    EN_PV_ERR_PV3_LOSE_CONTROL, 
    EN_PV_ERR_PV4_LOSE_CONTROL,     
    EN_PV_ERR_PV1_OVER_CURR,
    EN_PV_ERR_PV2_OVER_CURR,
    EN_PV_ERR_PV3_OVER_CURR,
    EN_PV_ERR_PV4_OVER_CURR,
    EN_PV_ERR_PV1_INS_FAULT,
    EN_PV_ERR_PV2_INS_FAULT,
    EN_PV_ERR_PV3_INS_FAULT,
    EN_PV_ERR_PV4_INS_FAULT,
    EN_PV_ERR_PV1_ARC_FAULT,
    EN_PV_ERR_PV2_ARC_FAULT,
    EN_PV_ERR_PV3_ARC_FAULT,
    EN_PV_ERR_PV4_ARC_FAULT,
    EN_PV_ERR_PV1_VOLT_HIGH,
    EN_PV_ERR_PV2_VOLT_HIGH,
    EN_PV_ERR_PV3_VOLT_HIGH,
    EN_PV_ERR_PV4_VOLT_HIGH,

}EN_PV_ERROR;

typedef union
{
    uint16_t byte[2];
    struct
    {
        uint16_t pv1_short              : 1;          // bit:00    PV1短路
        uint16_t pv2_short              : 1;          // bit:01    PV2短路
        uint16_t pv3_short              : 1;          // bit:02    PV3短路
        uint16_t pv4_short              : 1;          // bit:03    PV4短路
        uint16_t pv1_reverse            : 1;          // bit:04    PV1反接
        uint16_t pv2_reverse            : 1;          // bit:05    PV2反接
        uint16_t pv3_reverse            : 1;          // bit:06    PV3反接
        uint16_t pv4_reverse            : 1;          // bit:07    PV4反接
        uint16_t pv1_lose_control       : 1;          // bit:08    PV1失控
        uint16_t pv2_lose_control       : 1;          // bit:09    PV2失控
        uint16_t pv3_lose_control       : 1;          // bit:10    PV3失控
        uint16_t pv4_lose_control       : 1;          // bit:11    PV4失控
        uint16_t pv1_over_curr          : 1;          // bit:12    PV1过流
        uint16_t pv2_over_curr          : 1;          // bit:13    PV2过流
        uint16_t pv3_over_curr          : 1;          // bit:14    PV3过流
        uint16_t pv4_over_curr          : 1;          // bit:15    PV4过流
        uint16_t pv1_ins_fault          : 1;          // bit:0     PV1绝缘阻抗小
        uint16_t pv2_ins_fault          : 1;          // bit:1     PV2绝缘阻抗小
        uint16_t pv3_ins_fault          : 1;          // bit:2     PV3绝缘阻抗小
        uint16_t pv4_ins_fault          : 1;          // bit:3     PV4绝缘阻抗小
        uint16_t pv1_arc_fault          : 1;          // bit:4     PV1拉弧故障
        uint16_t pv2_arc_fault          : 1;          // bit:5     PV2拉弧故障
        uint16_t pv3_arc_fault          : 1;          // bit:6     PV3拉弧故障
        uint16_t pv4_arc_fault          : 1;          // bit:7     PV4拉弧故障
        uint16_t pv1_volt_high          : 1;          // bit:8     PV1过压告警 
        uint16_t pv2_volt_high          : 1;          // bit:9     PV2过压告警 
        uint16_t pv3_volt_high          : 1;          // bit:10    PV3过压告警 
        uint16_t pv4_volt_high          : 1;          // bit:11    PV4过压告警 
        uint16_t reserve12_15           : 4;         // 预留
    } bit;
} Dsp_PvErrorCode_t;


/*
    ERR Part: Bus ERROR
*/
typedef enum
{
    EN_BUS_ERR_VOL_LOW,
    EN_BUS_ERR_VOL_OVER,
    EN_BUS_ERR_SHORT_CIRCUIT,
    EN_BUS_ERR_OUT_FAULT,
    EN_BUS_ERR_CHG_OVERCURR,
    EN_BUS_ERR_DSG_OVERCURR,

}EN_Bus_ERROR;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t bus_volt_low          : 1;          // bit:00    母线电压低
        uint16_t bus_volt_over         : 1;          // bit:01    母线电压高
        uint16_t bus_short_circut      : 1;          // bit:02    母线短路
        uint16_t bus_out_fault         : 1;          // bit:03    母线连接异常
        uint16_t bus_chg_overcurr      : 1;          // bit:04    电池充电过流
        uint16_t bus_dsg_overcurr      : 1;          // bit:05    电池放电过流
        uint16_t reserve06_15          : 10;         // 预留
    } bit;
} Dsp_BusErrorCode_t;


/*
    ERR Part: Inv ERROR
*/
typedef enum
{
    EN_INV_ERR_OUT_SHORT,
    EN_INV_ERR_VOL_LOW,
    EN_INV_ERR_VOL_OVER,
    EN_INV_ERR_OVER_CURR,
    EN_INV_ERR_OUT_FAULT,
}EN_INV_ERROR;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t inv_out_short         : 1;          // bit:00    逆变输出短路
        uint16_t inv_vol_low           : 1;          // bit:01    逆变电压低
        uint16_t inv_vol_over          : 1;          // bit:02    逆变电压高
        uint16_t inv_over_curr         : 1;          // bit:03    逆变过流  
        uint16_t inv_out_fault         : 1;          // bit:04    逆变输出异常
        uint16_t reserve05_15          : 9;         // 预留
    } bit;
} Dsp_InvErrorCode_t;


/*
    ERR Part: Load ERROR
*/
typedef enum
{
    EN_LOAD_ERR_OVER_LOAD,
}EN_LOAD_ERROR;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t over_load            : 1;          // bit:00    逆变输出短路     
        uint16_t reserve01_15         : 15;         // 预留
    } bit;
} Dsp_LoadErrorCode_t;


/*
    ERR Part: Temp ERROR
*/
typedef enum
{
    EN_TEMP_ERR_RADIATOR_OVER_TEMP,                  //TEMP_A2
    EN_TEMP_ERR_CHASSIS_OVER_TEMP,                   //TEMP_A3
    EN_TEMP_ERR_CHASSIS_LOW_TEMP,                    //TEMP_A3
    EN_TEMP_ERR_PV_OVER_TEMP,                        //TEMP_A0、TEMP_A1
}EN_TEMP_ERROR;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t radiator_over_temp     : 1;          // bit:00    散热器过温,TEMP_A2
        uint16_t chassis_over_temp      : 1;          // bit:01    腔体过温,TEMP_A3
        uint16_t chassis_low_temp       : 1;          // bit:02    环境欠温，TEMP_A3
        uint16_t pv_over_temp          : 1;          // bit:03     Pv过温，TEMP_A0、TEMP_A1       
        uint16_t reserve04_15          : 12;         // 预留
    } bit;
} Dsp_TempErrorCode_t;



/*
    ERR Part: GEN ERROR
*/
typedef enum
{
    EN_GEN_ERR_FAULT,

}EN_GEN_ERROR;

typedef union
{
    uint16_t byte;
    struct
    {      
        uint16_t reserve00_15          : 16;         // 预留
    } bit;
} Dsp_GENErrorCode_t;

/*
    ERR Part: Other ERROR
*/
typedef enum
{
    EN_OTHER_ERR_START_TIMEOUT,                  
    EN_OTHER_ERR_TZ_FAULT,                
    EN_OTHER_ERR_LEAKCURR_HIGH,                      
    EN_OTHER_ERR_ARC_FAULT,   
    EN_OTHER_ERR_INS_FAULT,
    EN_OTHER_ERR_DALAY_FAULT,
    EN_OTHER_ERR_SAMPING_FAULT,
    EN_OTHER_ERR_TEMP_SENSOR_FAULT,
    EN_OTHER_ERR_COMM_FAULT,
    EN_OTHER_ERR_BYPASS_FAULT,
    EN_OTHER_ERR_COMM_FAULT_WITHE_SALVE,
    EN_OTHER_ERR_SALVE_FAULT,
    EN_OTHER_ERR_GROUND,
    EN_OTHER_ERR_COMM_FAULT_WITHE_ARC,

}EN_OTHER_ERROR;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t start_timeout         : 1;          // bit:00，软起超时   
        uint16_t inv_tz_fault          : 1;          // bit:01，硬件过流  
        uint16_t leakcurr_high         : 1;          // bit:02，漏电流高    
        uint16_t arc_fault             : 1;          // bit:03，拉弧故障 
        uint16_t ins_fault             : 1;          // bit:04，绝缘阻抗故障    
        uint16_t dalay_fault           : 1;          // bit:05，继电器短路故障    
        uint16_t sampling_fault        : 1;          // bit:06，采样传感器故障    
        uint16_t temp_sensor_fault     : 1;          // bit:07，温度传感器故障         
        uint16_t comm_fault            : 1;          // bit:08，主从通讯故障 
        uint16_t bypass_fault          : 1;          // bit:09，旁路故障
        uint16_t comm_fault_with_slave : 1;          // bit:10，从机通讯故障
        uint16_t salve_fault           : 1;          // bit:11，并机从机故障
        uint16_t ground_fault          : 1;          // bit:12，接地故障
        uint16_t comm_fault_with_arc   : 1;          // bit:13，拉弧通讯故障      
        uint16_t reserve14_15          : 2;          // 预留
    } bit;
} Dsp_OtherErrorCode_t;




/******************************************WARNING****************************************************/

/*
    WARNING Part: Grid WARNING
*/
typedef enum
{
    EN_GRID_WARNING_VOL_LOW,
    EN_GRID_WARNING_VOL_HIGH,
    EN_GRID_WARNING_FRE_LOW,
    EN_GRID_WARNING_FRE_HIGH,

}EN_Grid_WARNING;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t grid_volt_low          : 1;          // bit:00    电网电压低,小于176V，205V恢复
        uint16_t grid_volt_high         : 1;          // bit:01    电网电压高,大于265V，253V恢复
        uint16_t grid_freq_low           : 1;          // bit:02    电网频率低,小于47Hz，47.7恢复
        uint16_t grid_freq_high          : 1;          // bit:03    电网频率高,大于52Hz，50.15Hz恢复
        uint16_t reserve04_15           : 12;         // 预留
    } bit;
} Dsp_GridWARNINGCode_t;


/*
    WARNING Part: PV WARNING
*/
typedef enum
{

    EN_PV_WARNING_PV1_VOLT_LOW,
    EN_PV_WARNING_PV2_VOLT_LOW,
    EN_PV_WARNING_PV3_VOLT_LOW,
    EN_PV_WARNING_PV4_VOLT_LOW,
    EN_PV_WARNING_PV1_VOLT_HIGH, 
    EN_PV_WARNING_PV2_VOLT_HIGH,
    EN_PV_WARNING_PV3_VOLT_HIGH,
    EN_PV_WARNING_PV4_VOLT_HIGH,
    EN_PV_WARNING_PV1_CURR_HIGH,
    EN_PV_WARNING_PV2_CURR_HIGH, 
    EN_PV_WARNING_PV3_CURR_HIGH, 
    EN_PV_WARNING_PV4_CURR_HIGH,     
    EN_PV_WARNING_PV1_OVER_PWR,
    EN_PV_WARNING_PV2_OVER_PWR,
    EN_PV_WARNING_PV3_OVER_PWR,
    EN_PV_WARNING_PV4_OVER_PWR,
    EN_PV_WARNING_TOTAL_OVER_PWR,

}EN_PV_WARNING;

typedef union
{
    uint16_t byte[2];
    struct
    {
        uint16_t pv1_vol_low            : 1;          // bit:00    PV1欠压
        uint16_t pv2_vol_low            : 1;          // bit:01    PV2欠压
        uint16_t pv3_vol_low            : 1;          // bit:02    PV3欠压
        uint16_t pv4_vol_low            : 1;          // bit:03    PV4欠压
        uint16_t pv1_vol_high           : 1;          // bit:04    PV1过压
        uint16_t pv2_vol_high           : 1;          // bit:05    PV2过压
        uint16_t pv3_vol_high           : 1;          // bit:06    PV3过压
        uint16_t pv4_vol_high           : 1;          // bit:07    PV4过压
        uint16_t pv1_cur_high           : 1;          // bit:08    PV1过流
        uint16_t pv2_cur_high           : 1;          // bit:09    PV2过流
        uint16_t pv3_cur_high           : 1;          // bit:10    PV3过流
        uint16_t pv4_cur_high           : 1;          // bit:11    PV4过流
        uint16_t pv1_over_pwr           : 1;          // bit:12    PV1过功率
        uint16_t pv2_over_pwr           : 1;          // bit:13    PV2过功率
        uint16_t pv3_over_pwr           : 1;          // bit:14    PV3过功率
        uint16_t pv4_over_pwr           : 1;          // bit:15    PV4过功率
        uint16_t total_over_pwr         : 1;          // bit:16    总功率过功率

        uint16_t reserve17_31           : 15;         // 预留
    } bit;
} Dsp_PvWARNINGCode_t;


/*
    WARNING Part: Bus WARNING
*/
typedef enum
{
    EN_BUS_WARNING,

}EN_Bus_WARNING;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t reserve00_15          : 16;         // 预留
    } bit;
} Dsp_BusWARNINGCode_t;


/*
    ERR Part: Inv WARNING
*/
typedef enum
{
    EN_INV_WARNING_FAULT,
} EN_INV_WARNING;

typedef union
{
    uint16_t byte;
    struct
    {    
        uint16_t reserve00_15          : 16;         // 预留
    } bit;
} Dsp_InvWARNINGCode_t;


/*
    WARNING Part: Load WARNING
*/
typedef enum
{
    EN_LOAD_WARNING_OVER_LOAD,
}EN_LOAD_WARNING;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t over_load             : 1;          // bit:00    过载   
        uint16_t reserve01_15          : 15;         // 预留
    } bit;
} Dsp_LoadWARNINGCode_t;


/*
    WARNING Part: Temp WARNING
*/
typedef enum
{
//    EN_TEMP_WARNING_CHASSIS_OVER_TEMP,                  //TEMP_A3
    EN_TEMP_WARNING_RADIATOR_OVER_TEMP,                 //TEMP_A2
    // EN_TEMP_WARNING_INV_OVER_TEMP,                      //TEMP_A1
    // EN_TEMP_WARNING_PV_OVER_TEMP,                       //TEMP_A0
}EN_TEMP_WARNING;

typedef union
{
    uint16_t byte;
    struct
    {
        // uint16_t chassis_over_temp     : 1;          // bit:00    腔体过温,TEMP_A3
        uint16_t radiator_over_temp    : 1;          // bit:01    散热器过温,TEMP_A2
        // uint16_t inv_over_temp         : 1;          // bit:02    逆变过温，TEMP_A1
        // uint16_t pv_over_temp          : 1;          // bit:03    逆变过流，TEMP_A0       
        uint16_t reserve01_15          : 15;         // 预留
    } bit;
} Dsp_TempWARNINGCode_t;



/*
    WARNING Part: GEN WARNING
*/
typedef enum
{
    EN_GEN_WARNING_VOL_LOW,
    EN_GEN_WARNING_VOL_HIGH,
    EN_GEN_WARNING_FRE_LOW,
    EN_GEN_WARNING_FRE_HIGH,

}EN_GEN_WARNING;

typedef union
{
    uint16_t byte;
    struct
    {    
        uint16_t gen_vol_low           : 1;
        uint16_t gen_vol_high          : 1;
        uint16_t gen_freq_low           : 1;
        uint16_t gen_freq_high          : 1;  
        uint16_t reserve04_15          : 12;         // 预留
    } bit;
} Dsp_GENWARNINGCode_t;

/*
    WARNING Part: Other WARNING
*/
typedef enum
{
    EN_OTHER_WARNING_FAN_FAULT,  
    EN_OTHER_WARNING_R_CT_DISCONNECT,
    // EN_OTHER_WARNING_S_CT_DISCONNECT,
    // EN_OTHER_WARNING_T_CT_DISCONNECT, 
    // EN_OTHER_WARNING_R_CT_REVERSE, 
    // EN_OTHER_WARNING_S_CT_REVERSE,
    // EN_OTHER_WARNING_T_CT_REVERSE,     
    // EN_OTHER_WARNING_FIRMWARE_PACK_FAULT,
    // EN_OTHER_WARNING_MODLE_ID_FAULT,

}EN_OTHER_WARNING;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t fan_fault             : 1;          // bit:00 
        uint16_t r_ct_disconnect       : 1;          // bit:01
        // uint16_t s_ct_disconnect       : 1;          // bit:02
        // uint16_t t_ct_disconnect       : 1;          // bit:03
        // uint16_t r_ct_reverse          : 1;          // bit:04
        // uint16_t s_ct_reverse          : 1;          // bit:05
        // uint16_t t_ct_reverse          : 1;          // bit:06
        // uint16_t firmware_pack_fault   : 1;          // bit:07
        // uint16_t modle_id_fault        : 1;          // bit:08
        uint16_t reserve03_15          : 14;          // 预留
    } bit;
} Dsp_OtherWARNINGCode_t;


///*********************************WARNING END*****************************************/


//**********************************NOTICE BEGIN**********************************/
typedef enum
{
    EN_PV_NOTICE_PV1_MISSING,  
    EN_PV_NOTICE_PV2_MISSING,
    EN_PV_NOTICE_PV3_MISSING,  
    EN_PV_NOTICE_PV4_MISSING,    
}EN_PV_NOTICE;

typedef union
{
    uint16_t byte[2];
    struct
    {
        uint16_t pv1_missing           : 1;          // bit:00 
        uint16_t pv2_missing           : 1;          // bit:01
        uint16_t pv3_missing           : 1;          // bit:02
        uint16_t pv4_missing           : 1;          // bit:03

        uint16_t reserve04_15          : 12;          // 预留
    } bit;
} Dsp_PVNOTICECode_t;

typedef enum
{
    EN_OTHER_NOTICE_FIRMWARE_DSP_FAULT,
    EN_OTHER_NOTICE_FIRMWARE_ARC_FAULT,
    EN_OTHER_NOTICE_FIRMWARE_COPRO_FAULT,
}EN_OTHER_NOTICE;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t firmware_dsp_fault    : 1;          // bit:00 
        uint16_t firmware_arc_fault    : 1;          // bit:01
        uint16_t firmware_copro_fault  : 1;          // bit:02
        uint16_t reserve03_15          : 13;          // 预留
    } bit;
} Dsp_OtherNOTICECode_t;




typedef struct _fault_info_stru_t
{
    //DspWarningCode_t        inside_warn;         //DSP不上报的内部告警
    Dsp_GridErrorCode_t     inside_err_grid;     //DSP电网相关故障
    Dsp_PvErrorCode_t       inside_err_pv;       //DSPpv相关故障
    Dsp_BusErrorCode_t      inside_err_bus;      //DSP母线相关故障
    Dsp_InvErrorCode_t      inside_err_inv;      //DSP逆变相关故障
    Dsp_LoadErrorCode_t     inside_err_load;     //DSP负载相关故障
    Dsp_TempErrorCode_t     inside_err_temp;     //DSP温度相关故障
    Dsp_GENErrorCode_t      inside_err_gen;      //DSP油机相关故障
    Dsp_OtherErrorCode_t    inside_err_other;    //DSP其他故障

    Dsp_GridWARNINGCode_t   inside_warn_grid;     //DSP电网相关警告
    Dsp_PvWARNINGCode_t     inside_warn_pv;       //DSPpv相关警告
    Dsp_BusWARNINGCode_t    inside_warn_bus;      //DSP母线相关警告
    Dsp_InvWARNINGCode_t    inside_warn_inv;      //DSP逆变相关警告
    Dsp_LoadWARNINGCode_t   inside_warn_load;     //DSP负载相关警告
    Dsp_TempWARNINGCode_t   inside_warn_temp;     //DSP温度相关警告
    Dsp_GENWARNINGCode_t    inside_warn_gen;      //DSP油机相关警告
    Dsp_OtherWARNINGCode_t  inside_warn_other;    //DSP其他警告   
         
    Dsp_PVNOTICECode_t      inside_notice_pv;      //DSP PV相关通知
    Dsp_OtherNOTICECode_t   inside_notice_other;   //DSP其他相关通知

    InsideFault_t           inside_add_fault;    //DSP不上报的内部故障
    uint16_t                delay_count;         //连续检测计数
    uint16_t                err_flag;            //故障标志
    uint16_t                warning_flag;        //警告标志

} FaultInfo_t;


typedef struct count_trigger_stru_t {
    int32_t count;
    int32_t count_interval;
    uint32_t trig_threshold;
    uint8_t trigged;
    void (*trig_hook)(void *);
    void *hook_param;
} CountTrigger_t;

void CountTriggerInit(CountTrigger_t *p_trigger, uint32_t threshold);
void CountTriggerRun(CountTrigger_t *p_trigger, uint8_t value);
void CountTriggerReset(CountTrigger_t* p_trigger);
void ErrWarningCheckInit(void);
void ErrWarningHandler(void);

uint16_t GetGridErrorValue(void);
uint16_t GetPvErrorHighValue(void);
uint16_t GetPvErrorLowValue(void);
uint16_t GetBusErrorValue(void);
uint16_t GetInvErrorValue(void);
uint16_t GetLoadErrorValue(void);
uint16_t GetTempErrorValue(void);
uint16_t GetGenErrorValue(void);
uint16_t GetOtherErrorValue(void);

uint16_t GetGridWarningHighValue(void);
uint16_t GetPvWarningHighValue(void);
uint16_t GetPvWarningLowValue(void);
uint16_t GetBusWarningValue(void);
uint16_t GetInvWarningValue(void);
uint16_t GetLoadWarningValue(void);
uint16_t GetTempWarningValue(void);
uint16_t GetGenWarningValue(void);
uint16_t GetOtherWarningValue(void);

/* Error Functions Declarations */
uint16_t GetError_Grid_VoltLow(void);
uint16_t GetError_Grid_VoltHigh(void);
uint16_t GetError_Grid_FreqLow(void);
uint16_t GetError_Grid_FreqHigh(void);

uint16_t GetError_Pv1_Short(void);
uint16_t GetError_Pv2_Short(void);
uint16_t GetError_Pv3_Short(void);
uint16_t GetError_Pv4_Short(void);
uint16_t GetError_Pv1_Reverse(void);
uint16_t GetError_Pv2_Reverse(void);
uint16_t GetError_Pv3_Reverse(void);
uint16_t GetError_Pv4_Reverse(void);
uint16_t GetError_Pv1_LoseControl(void);
uint16_t GetError_Pv2_LoseControl(void);
uint16_t GetError_Pv3_LoseControl(void);
uint16_t GetError_Pv4_LoseControl(void);
uint16_t GetError_Pv1_OverCurr(void);
uint16_t GetError_Pv2_OverCurr(void);
uint16_t GetError_Pv3_OverCurr(void);
uint16_t GetError_Pv4_OverCurr(void);
uint16_t GetError_Pv1_InsFault(void);
uint16_t GetError_Pv2_InsFault(void);
uint16_t GetError_Pv3_InsFault(void);
uint16_t GetError_Pv4_InsFault(void);
uint16_t GetError_Pv1_ArcFault(void);
uint16_t GetError_Pv2_ArcFault(void);
uint16_t GetError_Pv3_ArcFault(void);
uint16_t GetError_Pv4_ArcFault(void);

uint16_t GetError_Bus_VoltLow(void);
uint16_t GetError_Bus_VoltOver(void);
uint16_t GetError_Bus_ShortCircut(void);
uint16_t GetError_Bus_OutFault(void);
uint16_t GetError_Bus_ChgOvercurr(void);
uint16_t GetError_Bus_DsgOvercurr(void);

uint16_t GetError_Inv_OutShort(void);
uint16_t GetError_Inv_VolLow(void);
uint16_t GetError_Inv_VolOver(void);
uint16_t GetError_Inv_OverCurr(void);

uint16_t GetError_Load_OverLoad1(void);
uint16_t GetError_Load_OverLoad2(void);

uint16_t GetError_Temp_ChassisOverTemp(void);
uint16_t GetError_Temp_RadiatorOverTemp(void);
uint16_t GetError_Temp_InvOverTemp(void);
uint16_t GetError_Temp_PvOverTemp(void);

uint16_t GetError_Other_StartTimeout(void);
uint16_t GetError_Other_InvTzFault(void);
uint16_t GetError_Other_LeakcurrHigh(void);
uint16_t GetError_Other_ArcFault(void);
uint16_t GetError_Other_InsFault(void);
uint16_t GetError_Other_DalayFault(void);
uint16_t GetError_Other_SamplingFault(void);
uint16_t GetError_Other_TempSensorFault(void);
uint16_t GetError_Other_CommFault(void);
uint16_t GetError_Other_BypassFault(void);

/* Warning Functions Declarations */
uint16_t GetWarning_Grid_VoltLow(void);
uint16_t GetWarning_Grid_VoltHigh(void);
uint16_t GetWarning_Grid_FreqLow(void);
uint16_t GetWarning_Grid_FreqHigh(void);

uint16_t GetWarning_Pv1_VolLow(void);
uint16_t GetWarning_Pv2_VolLow(void);
uint16_t GetWarning_Pv3_VolLow(void);
uint16_t GetWarning_Pv4_VolLow(void);
uint16_t GetWarning_Pv1_VolHigh(void);
uint16_t GetWarning_Pv2_VolHigh(void);
uint16_t GetWarning_Pv3_VolHigh(void);
uint16_t GetWarning_Pv4_VolHigh(void);
uint16_t GetWarning_Pv1_CurHigh(void);
uint16_t GetWarning_Pv2_CurHigh(void);
uint16_t GetWarning_Pv3_CurHigh(void);
uint16_t GetWarning_Pv4_CurHigh(void);
uint16_t GetWarning_Pv1_OverPwr(void);
uint16_t GetWarning_Pv2_OverPwr(void);
uint16_t GetWarning_Pv3_OverPwr(void);
uint16_t GetWarning_Pv4_OverPwr(void);
uint16_t GetWarning_Pv_TotalOverPwr(void);

uint16_t GetWarning_Bus_VoltLow(void);
uint16_t GetWarning_Bus_VoltHigh(void);

uint16_t GetWarning_Inv_OutFault(void);
uint16_t GetWarning_Inv_OutVolOver(void);
uint16_t GetWarning_Inv_OutVolLow(void);

uint16_t GetWarning_Load_OverLoad(void);

uint16_t GetWarning_Temp_ChassisOverTemp(void);
uint16_t GetWarning_Temp_RadiatorOverTemp(void);
uint16_t GetWarning_Temp_InvOverTemp(void);
uint16_t GetWarning_Temp_PvOverTemp(void);

uint16_t GetWarning_Gen_VolLow(void);
uint16_t GetWarning_Gen_VolHigh(void);
uint16_t GetWarning_Gen_FreqLow(void);
uint16_t GetWarning_Gen_FreqHigh(void);

uint16_t GetWarning_Other_FanFault(void);
uint16_t GetWarning_Other_R_Ct_Disconnect(void);
uint16_t GetWarning_Other_S_Ct_Disconnect(void);
uint16_t GetWarning_Other_T_Ct_Disconnect(void);
uint16_t GetWarning_Other_R_Ct_Reverse(void);
uint16_t GetWarning_Other_S_Ct_Reverse(void);
uint16_t GetWarning_Other_T_Ct_Reverse(void);
uint16_t GetWarning_Other_FirmwarePackFault(void);
uint16_t GetWarning_Other_ModleIdFault(void);

uint16_t GetPvNoticeHighValue(void);
uint16_t GetPvNoticeLowValue(void);
uint16_t GetOtherNoticeValue(void);


uint16_t GetErrorFlag(void);
uint16_t GetWarningFlag(void);

void Fault_Int1(void);
void Fault_20ms(void);
void ErrWarningHandler();
extern uint32_t GetErrorValue(void);
extern FaultInfo_t gstFaultInfo;
extern bool CT_DisConnectCheck;
void ErrWarningHandler_100ms();

#endif /* APP_FAULT_H_ */
