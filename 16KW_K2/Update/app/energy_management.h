/*
 * energy_management.h
 *
 *  Created on: 2025年3月18日
 *      Author: Nova006352
 */

#ifndef APP_ENERGY_MANAGEMENT_H_
#define APP_ENERGY_MANAGEMENT_H_
#include "F28x_Project.h"



// #define POWER_VALUE_LIMIT(x,max,min)      ((x>max)?max:((x<min)?min:x))

/********************/
#define FILTER_WINDOW_SIZE 10     // 滤波窗口大小
#define BUSVOL_SET_UP 520.0f        // 母线电压设定保护上限(V)
#define BUSVOL_SET_LOW 430.0f    // 母线电压设定保护下限（V）
#define BUSVOL_THEORY_UP  512.0f   //理论电压上限(V)
#define BUSVOL_THEORY_LOW  440.0f   //理论电压下限(V)
#define MAX_BATTERY_POWER 16500.0f // 最大电池功率(W)
#define MIN_BATTERY_POWER -16500.0f// 最小电池功率(W)
#define KP 50.0f                  // 比例系数
#define KI 0.1f                   // 积分系数
/***********************/






/*并离网状态*/
// typedef enum
// {
//     EN_STAND_BY_MODE,//0
//     EN_OFF_GRID_MODE,
//     EN_OFF_GRID_TO_GRID_CONNECT_MODE,
//     EN_GRID_CONNECT_MODE,
//     EN_GRID_CONNECT_TO_OFF_GRID_MODE,
// } EN_PARALLEL_OFF_NETWORK_STA;

/*工作模式*/
// typedef enum
// {
//     EN_SPONTANEOUS_USE_MODE,
//     EN_BAT_PRIORITY_MODE,
//     EN_PEAK_VALLEY_MODE,
//     EN_OIL_MODE,
// } EN_WORK_MODE;


typedef enum
{
    EN_DRM_DRM0,
    EN_DRM_DRM1,
    EN_DRM_DRM2,
    EN_DRM_DRM3,
    EN_DRM_DRM4,
}DRM_STATE;


/*调度所用信息*/

typedef struct _Deploy_Info_t
{
//调度业务中的使能标志位
    uint16_t    inv_enable;
    uint16_t    grid_enable;
    uint16_t    oil_enable;
//监控卡下发
    uint16_t    dsg_enable;
    uint16_t    chg_enable;
    uint16_t    backup_output_enable;
    uint16_t    antireverseflow_enable;
    uint16_t    charging_input_type;
    uint16_t    work_mode;
    uint16_t    grid_standards;
    uint16_t    cto_meter;
    uint16_t    ct_direction;
    uint16_t    on_grid_reconnect_time;
    uint32_t    set_oil_power;

    int32_t     set_batt_chg_power_input;
    int32_t     set_batt_dsg_power_input;
    int32_t     on_grid_pwr_input;        //将输入目标值进行滤波

    int32_t     set_batt_chg_power;
    int32_t     set_batt_dsg_power;
    int32_t     on_grid_pwr;        //对input值通过母线电压进行输入功率限制
//系统V I PWR的反馈值

    float32     backup_feekback_activerpwr;
    float32     pv_feekback_pwr;
    float32     grid_curr_rms;
} DeployInfo_t;//能量调度所需信息（包括用户设置、反馈信息）

/*电网功率给定*/
typedef struct _Grid_Given_t
{
    float32 active_pwr;
    float32 reactive_pwr;
} GridGiven_t;

/*调度能量功率给定*/
typedef struct _Power_Given_t
{
    float32 bat_pwr;
    float32 pv_pwr;
    float32 oil_pwr;
    GridGiven_t grid_pwr;

    float32 Ramp_gpwroil_active;
    float32 Ramp_gpwroil_reactive;
    float32 Ramp_gpwrgrid_active;
    float32 Ramp_gpwrgrid_reactive;

    float32 Temp_Limit_pwr;

} PowerGiven_t;

// 母线电压滤波结构体
typedef struct {
    float32 voltage_buffer[FILTER_WINDOW_SIZE]; // 电压缓冲区
    uint16_t index;                            // 当前索引
    uint16_t is_full;                          // 缓冲区是否已满
    float32 sum;                                // 当前总和
} VoltageFilter;


typedef enum
{
    Power_Ctrl_Normal,//母线电压未到理论上限
    Power_Ctrl_Hold,  //母线电压在理论上限与保护上限之间
    Power_Ctrl_PI_Chg,    //母线电压超过保护上限进行PI控制
    Power_Ctrl_PI_Dsg,
}Pwr_ctrl_t;

// 功率控制器结构体
typedef struct {
    float32 busvol_protect_upper_limit;  // 母线电压保护上限
    float32 busvol_portect_lower_limit;  // 母线电压保护下限
    float32 busvol_theory_upper_limit;  // 母线电压理论上限
    float32 busvol_theory_lower_limit;  // 母线电压理论下限
    float32 kp, ki;               // PID参数
    float32 integral;             // 积分项
    float32 set_act_power;        // 设定的有功功率
    float32 set_react_power;      // 设定的无功功率
    float32 max_power;            // 最大功率限制
    float32 min_power;            // 最小功率限制
    float32 bus_vol_error;       //母线电压误差
    Pwr_ctrl_t Pwr_ctrl;         //控制模式
//    uint16_t test_flag;
} PowerController;



extern PowerGiven_t gstPowerGiven;
extern float inv_output_power;
extern uint16_t gInvState;
extern uint16_t runstate;
extern DeployInfo_t gstDeployInfo;

extern PowerController power_controller;
extern DRM_STATE DRM_Adaptive_STATE;

//extern uint16_t gEnPv;
bool CtDisConnectCheck(void);
void DeployInfoInit(void);
void EnergyScheduling(void);
void BackUpSwitchCtrlFunc(void);
//uint16_t InvSwichStateGet(void);

#endif /* APP_ENERGY_MANAGEMENT_H_ */
