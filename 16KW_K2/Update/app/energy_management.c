/*
 * energy_management.c
 *
 *  Created on: 2025年3月18日
 *      Author: Nova006352
 */
#include "string.h"
#include "energy_management.h"
#include "ethernet_analysis.h"
#include "debug_analysis.h"
#include "cla_cpu_share.h"
#include "cpu_timers.h"
#include <algo_interface.h>
 
#include "inv_state.h"
#include "fault.h"
#include "energy_ctrl.h"
#include "sys_info.h"
#include "dev_ntc.h"
#include "AlgoFw/package/cntl/pv/pv.h"

#define MAX_CONNECTING_TIMER   10000
#define MAX_GRID_POWER         18000
#define MAX_INV_POWER          18000
#define MAX_OIL_POWER          18000

#define MAX_PWR_slope          2   //与EnergyScheduling（）函数调用频率相关，1ms调用一次为1s内缓起2kw

#define OverTemp_limit_Temp     80.0f
#define OverTemp_limit_Time  60*5 //5min内限功率
#define OverTemp_limit_Enable  1

#define U16_TO_F32(high_u16, low_u16)  ((float32)((((uint32_t)(high_u16)) << 16) | (((uint32_t)(low_u16)) & 0xFFFF)))

#define PI (3.1415926f)


EN_PARALLEL_OFF_NETWORK_STA gGridState;
EN_WORK_MODE gWorkMode;
DeployInfo_t gstDeployInfo;
PowerGiven_t gstPowerGiven;
uint32_t connecting_time = 0;
uint32_t reconnect_time = 0;

DRM_STATE DRM_Adaptive_STATE;
uint16_t DRM_RELAY_CONTROL_PERMIT;

//todo:以下三个调试用，后续删除
EN_PARALLEL_OFF_NETWORK_STA debug_state;
float32 pv_pwr;
float32 backup_pwr;
uint16_t runstate;

VoltageFilter voltage_filter;
PowerController power_controller;

float32 filtered_bus_voltage;
float32 bus_control_actpwr_out;



/**
* @brief: SetPowerRampUpdate()
* @param: void
* @return:current
* @note:功率调度缓起斜坡函数
*/

//功率缓起的斜坡函数
float SetPowerRampUpdate(float current, float max_slope, float target)
{
    float delta = fabs(target) - fabs(current);

    if (((current >= 0) && (target >= 0)) || ((current <= 0) && (target <= 0)))//防止由充电放电状态直接切换，把系统拉挂
    {
        if (fabs(delta) <= 10) {
            return target;
        } else if (fabs(delta) <= max_slope) {
            current = target;
            return current;
        } else if (delta > 0) {
            current += (target > 0) ? max_slope : -max_slope;
            return current;
        } else {
            return target;
        }
    } else {
        current = 0;
        return current;
    }

}

/**
* @brief: voltage_filter_init
* @param: static inline void
* @return:Nothing
* @note:初始化母线电压滤波器
*/ 
static void voltage_filter_init(VoltageFilter* filter) 
{
    for(int i = 0; i < FILTER_WINDOW_SIZE; i++) {
        filter->voltage_buffer[i] = 0.0f;
    }
    filter->index = 0;
    filter->is_full = 0;
    filter->sum = 0.0f;
}

/**
* @brief: voltage_filter_update
* @param: static inline void
* @return:Nothing
* @note:添加新电压值并返回滤波后的值
*/ 
static float32 voltage_filter_update(VoltageFilter* filter, float new_voltage) 
{
    // 减去即将被替换的值
    if(filter->is_full) {
        filter->sum -= filter->voltage_buffer[filter->index];
    }
    
    // 添加新值
    filter->sum += new_voltage;
    filter->voltage_buffer[filter->index] = new_voltage;
    
    // 更新索引
    filter->index++;
    if(filter->index >= FILTER_WINDOW_SIZE) {
        filter->index = 0;
        filter->is_full = 1;
    }
    
    // 计算平均值
    uint16_t  count = filter->is_full ? FILTER_WINDOW_SIZE : filter->index;
    return filter->sum / count;
}

// 初始化功率控制器
void power_controller_init(PowerController* controller, 
                          float32 set_vol_up, float32 set_vol_low , float32 kp, float32 ki, 
                          float32 vol_theory_up, float32 vol_theory_low ,float32 min_power, float32 max_power) 
{
    controller->busvol_protect_upper_limit = set_vol_up;
    controller->busvol_portect_lower_limit = set_vol_low;
    controller->busvol_theory_upper_limit = vol_theory_up;
    controller->busvol_theory_lower_limit = vol_theory_low;
    controller->kp = kp;
    controller->ki = ki;
    controller->integral = 0.0f;
    controller->max_power = max_power;
    controller->min_power = min_power;
    controller->set_act_power = 0;
    controller->set_react_power = 0;

}

// 计算电池功率输出
static void calculate_battery_power(PowerController* controller, 
                             float32 filtered_voltage, float32 dt) 
{
    // 计算电压误差

    if ((filtered_voltage > controller->busvol_theory_lower_limit)&&
        (filtered_voltage < controller->busvol_theory_upper_limit)) //母线电压在理论下限到理论上限之间
    {
        controller->Pwr_ctrl = Power_Ctrl_Normal;

    }
    else if (((filtered_voltage > controller->busvol_theory_upper_limit) && 
              (filtered_voltage < controller->busvol_protect_upper_limit))||
             ((filtered_voltage < controller->busvol_theory_lower_limit) && 
              (filtered_voltage > controller->busvol_portect_lower_limit)))    // 母线在理论值与保护值之间
    {
        controller->Pwr_ctrl = Power_Ctrl_Hold;

    }
    else if (filtered_voltage > controller->busvol_protect_upper_limit)
    {
        controller->Pwr_ctrl = Power_Ctrl_PI_Chg;

    }
    else if (filtered_voltage < controller->busvol_portect_lower_limit)
    {
        controller->Pwr_ctrl = Power_Ctrl_PI_Dsg;

    }
    else
    {
        controller->Pwr_ctrl = Power_Ctrl_Hold;

    }
    
    switch (controller->Pwr_ctrl)
    {
    case Power_Ctrl_Normal:         //母线电压未到理论上限
        {
            bus_control_actpwr_out = controller->set_act_power;
            break;
        }
    case Power_Ctrl_Hold:       //母线电压在理论上限与保护上限之间
        {
            if (bus_control_actpwr_out > 0 && controller->set_act_power > 0)
            {
                bus_control_actpwr_out = bus_control_actpwr_out < controller->set_act_power ? \
                                         bus_control_actpwr_out : controller->set_act_power;
            }
            else if (bus_control_actpwr_out < 0 && controller->set_act_power < 0)
            {
                bus_control_actpwr_out = bus_control_actpwr_out > controller->set_act_power ? \
                                         bus_control_actpwr_out : controller->set_act_power;
            }
            else
            {
                bus_control_actpwr_out = controller->set_act_power;
            }
            
            controller->integral = 0;
            break;
        }
    case Power_Ctrl_PI_Chg:     //母线电压超过保护上限进行PI控制
        {
            controller->bus_vol_error = controller->busvol_protect_upper_limit- filtered_voltage;
            controller->integral += controller->bus_vol_error * dt;
            bus_control_actpwr_out +=  controller->kp * controller->bus_vol_error + controller->ki * controller->integral;
            if(bus_control_actpwr_out > controller->max_power) 
            {
                bus_control_actpwr_out = controller->max_power;
                // 积分抗饱和
                // if(error > 0) 
                // controller->integral -= error * dt;
            } 
            else if(bus_control_actpwr_out < 0) 
            {
                bus_control_actpwr_out = 0;
                // 积分抗饱和
                if(controller->bus_vol_error < 0) 
                //controller->integral -= bus_vol_error * dt;
                controller->integral = 0;
            }
            break;
        }
    case Power_Ctrl_PI_Dsg:     //母线电压超过保护上限进行PI控制
        {
            controller->bus_vol_error  = controller->busvol_portect_lower_limit- filtered_voltage;
            controller->integral += controller->bus_vol_error * dt;
            bus_control_actpwr_out += controller->kp * controller->bus_vol_error + controller->ki * controller->integral;
            if(bus_control_actpwr_out > 0) 
            {
                bus_control_actpwr_out = 0;
                // 积分抗饱和
                if(controller->bus_vol_error > 0) 
                //controller->integral -= bus_vol_error * dt;
                controller->integral = 0;
            } 
            else if(bus_control_actpwr_out < controller->min_power) 
            {
                bus_control_actpwr_out = controller->min_power;
                // 积分抗饱和
                // if(error < 0) 
                // controller->integral -= error * dt;
            }
            break;
        }   
    default:
        bus_control_actpwr_out = controller->set_act_power;
        break;
    }
    
}

// 母线控制电压控制函数
void inverter_busvol_control_loop(float32 bus_voltage, float32 act_pwr, float32 dt) 
{
   static uint16_t is_initialized;
    // 第一次调用时初始化
    if(!is_initialized) {
        voltage_filter_init(&voltage_filter);
        power_controller_init(&power_controller, BUSVOL_SET_UP,
                            BUSVOL_SET_LOW, KP, KI, BUSVOL_THEORY_UP, BUSVOL_THEORY_LOW,
                            MIN_BATTERY_POWER, MAX_BATTERY_POWER);
        is_initialized = 1;
    }
    
    // 1. 母线电压滤波
    filtered_bus_voltage = voltage_filter_update(&voltage_filter, bus_voltage);
    
    power_controller.set_act_power = act_pwr;
    //power_controller.set_react_power = react_pwr;

    // 2. 计算电池功率
    calculate_battery_power(&power_controller, filtered_bus_voltage, dt);
    
    //后续增加无功控制
    //gstInvInterface.reactive_pwr = gstInvInterface.reactive_pwr_ref;

}




// /*该函数主要进行功率给定的更新，以及通过母线电压对功率进行控制*/
// void SetPowerUpdate(void)
// {

//     float last_chg_pwr = 0;
//     float last_grid_pwr = 0;

//     //gstDeployInfo.set_batt_dsg_power = PowerRampUpdate(gstDeployInfo.set_batt_dsg_power, MAX_PWR_slope , gstDeployInfo.set_batt_dsg_power_input );

//     gstDeployInfo.set_batt_dsg_power = gstDeployInfo.set_batt_dsg_power_input;

//     if((gstAlgoInterface.bus_vol>525)&&(gstDeployInfo.set_batt_chg_power<-100))
//     {
//         gstDeployInfo.set_batt_chg_power =  (gstDeployInfo.set_batt_chg_power > 0) ? gstDeployInfo.set_batt_chg_power-100 : gstDeployInfo.set_batt_chg_power+100 ;
//     }
//     else if(gstDeployInfo.set_batt_chg_power_input!=last_chg_pwr)
//     {
        
//         //gstDeployInfo.set_batt_chg_power = PowerRampUpdate(gstDeployInfo.set_batt_chg_power, MAX_PWR_slope , gstDeployInfo.set_batt_chg_power_input ) ;
//         gstDeployInfo.set_batt_chg_power = gstDeployInfo.set_batt_chg_power_input;

//     } else {

//     }

//     if((gstAlgoInterface.bus_vol < 430)&&(gstDeployInfo.on_grid_pwr < -100))
//     {
//         gstDeployInfo.on_grid_pwr = (gstDeployInfo.on_grid_pwr > 0) ? gstDeployInfo.on_grid_pwr-100 : gstDeployInfo.on_grid_pwr+100 ;
//     }
//     else if (gstDeployInfo.on_grid_pwr_input != last_grid_pwr)
//     {
//         //gstDeployInfo.on_grid_pwr = PowerRampUpdate(gstDeployInfo.on_grid_pwr, MAX_PWR_slope , gstDeployInfo.on_grid_pwr_input );
//         gstDeployInfo.on_grid_pwr = gstDeployInfo.on_grid_pwr_input;
//     } else {

//     }

//     last_chg_pwr = gstDeployInfo.set_batt_chg_power_input;
//     last_grid_pwr = gstDeployInfo.on_grid_pwr_input;

//     /*满功率放电经常故障停机，送测限制满功率放电*/

//     // if((-16000<=gstDeployInfo.on_grid_pwr)&&(gstDeployInfo.on_grid_pwr<-15000))
//     // {
//     //     gstDeployInfo.on_grid_pwr = -15000;
//     // }

// }



/*剩余功能*/
/*1.标志位*/


/**
* @brief: GetDeployInfo
* @param: void
* @return:Nothing
* @note:获取能量调度所需信息（用户设置、系统反馈值）
*/
void GetDeployInfo()
{
    //todo:成员变量确定后记得要初始化操作

    gstDeployInfo.set_batt_chg_power_input = -1 * U16_TO_F32(gstUdpPcsPara.udp_pcs_hold.set_batt_chg_power[0] , gstUdpPcsPara.udp_pcs_hold.set_batt_chg_power[1]);//todo:充放电符号
    gstDeployInfo.set_batt_dsg_power_input = U16_TO_F32(gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_power[0] , gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_power[1]);
    gstDeployInfo.on_grid_reconnect_time = gstUdpPcsPara.udp_pcs_hold.on_grid_reconnect_time * 1000;//比例关系
    gstDeployInfo.work_mode = gstUdpPcsPara.udp_pcs_hold.work_mode;
    if (gstUdpPcsPara.udp_pcs_hold.charging_input_type == 1) {
        gstDeployInfo.work_mode = EN_OIL_MODE;
    }
    gstDeployInfo.chg_enable = gstUdpPcsPara.udp_pcs_hold.pcs_chg_ctrl;
    gstDeployInfo.dsg_enable = gstUdpPcsPara.udp_pcs_hold.pcs_dsg_ctrl;
    gstDeployInfo.on_grid_pwr_input = -1 * U16_TO_F32(gstUdpPcsPara.udp_pcs_hold.on_grid_power[0] , gstUdpPcsPara.udp_pcs_hold.on_grid_power[1]);//todo:注意买卖电符号
    gstDeployInfo.antireverseflow_enable = gstUdpPcsPara.udp_pcs_hold.anti_reverse_flow_enable;
    gstDeployInfo.pv_feekback_pwr = Pv_Get_Total_Pwr_Ave();
    gstDeployInfo.backup_feekback_activerpwr = -1 * gstAlgoInterface.load_active_power;//负载值为负值，需*-1
    gstDeployInfo.grid_curr_rms = gstAlgoInterface.grid_cur_rms;
    gstDeployInfo.set_oil_power = U16_TO_F32(gstUdpPcsPara.udp_pcs_hold.InvGenMaxPower[0], gstUdpPcsPara.udp_pcs_hold.InvGenMaxPower[1]);
    //gstDeployInfo.set_oil_power = 3000;
    gstDeployInfo.set_batt_dsg_power = gstDeployInfo.set_batt_dsg_power_input;
    gstDeployInfo.set_batt_chg_power = gstDeployInfo.set_batt_chg_power_input;
    gstDeployInfo.on_grid_pwr = gstDeployInfo.on_grid_pwr_input;

    //SetPowerUpdate();
    //    gstDeployInfo.ct_curr = gstAlgoInterface.todo:需要CT电流值接口
    //    gstDeployInfo.pv_feekback_pwr = pv_pwr;todo:调试用，后续删除
    //    gstDeployInfo.backup_feekback_activerpwr = backup_pwr;调试用，后续删除

}

/**
* @brief: BackUpSwitchCtrlFunc
* @param: void
* @return:Nothing
* @note:负载继电器控制
*/
void BackUpSwitchCtrlFunc()
{
    if (BackupSwitchStateGet() && \
        (((gstDeployInfo.set_batt_dsg_power !=0)&&(geInvWorkState==INV_STATE_DISCONNECTED))||(geInvWorkState!=INV_STATE_DISCONNECTED)) && \
        ((geInvWorkState!=INV_STATE_STANDBY)&&(geInvWorkState!=INV_STATE_SELFTEST)))//打开负载继电器+允许电池放电+无故障
    {
        if ((gstAlgoInterface.inv_vol_theta > ((9 * PI) / 40)) && (gstAlgoInterface.inv_vol_theta < ((19 * PI) / 40))) {
            RELAY_BACKUP_OPEN;
        }
    } else {
        RELAY_BACKUP_CLOSE;
    }
}

/**9
* @brief: ToDisConnectHandle()
* @param: void
* @return:Nothing
* @note: 断线处理
*/
void ToDisConnectHandle()
{
    static uint16_t cnt_fiter = 0;
    gstPowerGiven.grid_pwr.active_pwr = 0;
    gstPowerGiven.grid_pwr.reactive_pwr = 0;
    if (fabs(gstDeployInfo.grid_curr_rms) < 5) {
        cnt_fiter++;
    } else {
        cnt_fiter = 0;
    }
    if (cnt_fiter > 10) {
        gstDeployInfo.grid_enable = 0; //下发主动脱网指令
        gstDeployInfo.oil_enable = 0;
    }
}

/**
* @brief: ThreadBreakHandle()
* @param: void
* @return:Nothing
* @note:断线检测
*/
void ThreadBreakHandle()
{
    if (GetWarning_Other_R_Ct_Disconnect() == 1) {
        ToDisConnectHandle();
    }
}

/**
* @brief: DeployInfoInit
* @param: void
* @return:Nothing
* @note:调度信息（用户设置、系统反馈值、功率给定）初始化
*/
void DeployInfoInit()
{
    memset((uint16_t*)&gstDeployInfo, 0x00, (int)sizeof(DeployInfo_t));
    memset((uint16_t*)&gstPowerGiven, 0x00, (int)sizeof(PowerGiven_t));
}

/**
* @brief: OffGridFunc
* @param: void
* @return:Nothing
* @note:离网业务
*/
void OffGridFunc(void)
{
    runstate = 0;
    /*PV功率给定*/
    // gstPowerGiven.bat_pwr = gstDeployInfo.set_batt_chg_power;
    // gstPowerGiven.pv_pwr = gstDeployInfo.backup_feekback_activerpwr - gstPowerGiven.bat_pwr;
    // if(gstPowerGiven.pv_pwr < 0)
    // {
    //     gstPowerGiven.pv_pwr = 0;
    // }
    /*并网重连时间*/
    if(TimDifftimeFromNow(reconnect_time) > gstDeployInfo.on_grid_reconnect_time)
    {
        //下发并网指令
        gstDeployInfo.grid_enable = 1;
        gstDeployInfo.oil_enable = 1;
        connecting_time = TimGetSystime();
        runstate = 2;
    }
    else
    {
        gstDeployInfo.grid_enable = 0;
        gstDeployInfo.oil_enable = 0;
    }

}

/**
* @brief: OffToOnGridFunc
* @param: void
* @return:Nothing
* @note:离网-并网业务
*/
void OffToOnGridFunc(void)
{
    if(TimDifftimeFromNow(connecting_time) > MAX_CONNECTING_TIMER)
    {
        //下发离网指令
        gstDeployInfo.grid_enable = 0;
        gstDeployInfo.oil_enable = 0;
        reconnect_time = TimGetSystime();
        runstate = 3;
    }
}

float inv_output_power;


/**
* @brief: OnGridFunc
* @param: void
* @return:Nothing
* @note:并网业务
*/
void OnGridFunc(void)
{
    // gWorkMode = (EN_WORK_MODE)gstDeployInfo.work_mode;
    // /*1.电池功率限充放电幅值*/
    // switch(gWorkMode)
    // {
    //     case EN_SPONTANEOUS_USE_MODE:
    //     {
    //         runstate = 4;
    //         /*自发自用电池功率（充/放）给定*/
    //         gstPowerGiven.bat_pwr = gstDeployInfo.backup_feekback_activerpwr - gstDeployInfo.pv_feekback_pwr;
    //         gstPowerGiven.bat_pwr = POWER_VALUE_LIMIT(gstPowerGiven.bat_pwr,gstDeployInfo.set_batt_dsg_power,gstDeployInfo.set_batt_chg_power);
    //         break;
    //     }
    //     case EN_BAT_PRIORITY_MODE:
    //     {
    //         runstate = 5;
    //         /*电池优先电池功率给定*/
    //         gstPowerGiven.bat_pwr = gstDeployInfo.set_batt_chg_power;
    //         break;
    //     }
    //     case EN_PEAK_VALLEY_MODE:
    //     {
    //         runstate = 6;
    //         /*峰谷模式充电时间段 电池功率给定*/
    //         if(gstDeployInfo.chg_enable == 1)
    //         {
    //             gstPowerGiven.bat_pwr = gstDeployInfo.set_batt_chg_power;
    //         }
    //         /*峰谷模式放电时间段 电池功率给定*/
    //         else if(gstDeployInfo.dsg_enable == 1)
    //         {
    //             runstate = 7;
    //             inv_output_power = gstDeployInfo.backup_feekback_activerpwr - gstDeployInfo.on_grid_pwr;//并网功率为负，前面符号为"-"
    //             if(inv_output_power > MAX_INV_POWER)
    //             {
    //                 inv_output_power = MAX_INV_POWER;
    //             }
    //             gstPowerGiven.bat_pwr = inv_output_power - gstDeployInfo.pv_feekback_pwr;
    //             gstPowerGiven.bat_pwr = POWER_VALUE_LIMIT(gstPowerGiven.bat_pwr,gstDeployInfo.set_batt_dsg_power,gstDeployInfo.set_batt_chg_power);
    //         }
    //         /*峰谷模式不充不放时间段 电池功率给定*/
    //         else
    //         {
    //             runstate = 8;
    //             inv_output_power = gstDeployInfo.backup_feekback_activerpwr - gstDeployInfo.on_grid_pwr;
    //             if(inv_output_power > MAX_INV_POWER)
    //             {
    //                 inv_output_power = MAX_INV_POWER;
    //             }
    //             gstPowerGiven.bat_pwr = inv_output_power - gstDeployInfo.pv_feekback_pwr;
    //             gstPowerGiven.bat_pwr = POWER_VALUE_LIMIT(gstPowerGiven.bat_pwr,0,gstDeployInfo.set_batt_chg_power);
    //         }
    //         break;
    //     }
    //     case EN_OIL_MODE:
    //     {
    //         runstate = 9;
    //         /*油机模式电池功率所需给定*/
    //         gstPowerGiven.bat_pwr = gstDeployInfo.set_batt_chg_power;
    //         break;
    //     }
    //     default:
    //     {
    //         gstPowerGiven.bat_pwr = 0;
    //         break;
    //     }
    // }


//     if(gWorkMode == EN_OIL_MODE)//油机 PV功率给定
//     {
//         gstPowerGiven.oil_pwr = gstDeployInfo.backup_feekback_activerpwr -  gstPowerGiven.bat_pwr - gstDeployInfo.pv_feekback_pwr;
//         gstPowerGiven.oil_pwr = POWER_VALUE_LIMIT(gstPowerGiven.oil_pwr,MAX_OIL_POWER,0);
//         gstPowerGiven.grid_pwr.active_pwr = 0;
//         gstPowerGiven.pv_pwr = gstDeployInfo.backup_feekback_activerpwr - gstPowerGiven.bat_pwr - gstPowerGiven.oil_pwr + 100;
//     }
//     else//电网 PV功率给定
//     {
//         gstPowerGiven.grid_pwr.active_pwr = gstDeployInfo.backup_feekback_activerpwr -  gstPowerGiven.bat_pwr - gstDeployInfo.pv_feekback_pwr;
//         gstPowerGiven.grid_pwr.active_pwr = POWER_VALUE_LIMIT(gstPowerGiven.grid_pwr.active_pwr,MAX_GRID_POWER,gstDeployInfo.on_grid_pwr);
//         if((gstDeployInfo.antireverseflow_enable == 1) && (gstPowerGiven.grid_pwr.active_pwr < 0))
//         {
//             gstPowerGiven.grid_pwr.active_pwr = 0;//todo:暂时写0，后续用远端负载功率值
// //            gstPowerGiven.grid_pwr.active_pwr = ;远端负载，如果远端负载通讯故障，则有功功率为0
//         }
//         /*CT断线检测、电表*/
//         ThreadBreakHandle();

//         /*TODO:并网认证功能
//          */
//         gstPowerGiven.oil_pwr = 0;
//         gstPowerGiven.pv_pwr = gstDeployInfo.backup_feekback_activerpwr - gstPowerGiven.bat_pwr - gstPowerGiven.grid_pwr.active_pwr + 100;
//     }
//     if(gstPowerGiven.pv_pwr < 0)
//     {
//         gstPowerGiven.pv_pwr = 0;
//     }
    reconnect_time = TimGetSystime();
}


/**
* @brief: OnToOffGridFunc
* @param: void
* @return:Nothing
* @note:并网->离网业务
*/
void OnToOffGridFunc(void)
{
    // gstPowerGiven.grid_pwr.active_pwr = 0;
    // gstPowerGiven.grid_pwr.reactive_pwr = 0;
    // gstPowerGiven.oil_pwr = 0;
    // gstPowerGiven.pv_pwr = 0;
    gstDeployInfo.grid_enable = 0;
    gstDeployInfo.oil_enable = 0;
    reconnect_time = TimGetSystime();
    runstate = 10;
}


/**
* @brief: DRM_RELAY_CONTROL
* @param: void
* @return:Nothing
* @note:DRM继电器控制
*/

void DRM_RELAY_CONTROL(void)
{
    if (DRM_Adaptive_STATE == EN_DRM_DRM0)
    {
        DRM_RELAY_CONTROL_PERMIT = 0;
    }
    else 
     {
        DRM_RELAY_CONTROL_PERMIT = 1;        
     }    
}



/**
* @brief: DRM_Adaptive_Recognition
* @param: void
* @return:Nothing
* @note:DRM自适应识别功率限制
*/

void DRM_Adaptive_Recognition(float32 active_pwr,float32 reactive_pwr)
{
    uint8_t drm_err_state;

    if(gstUdpPcsPara.udp_pcs_hold.drm_state & 0x0001)
    {
        DRM_Adaptive_STATE = EN_DRM_DRM0;
    }

    else if(gstUdpPcsPara.udp_pcs_hold.drm_state & 0x0002)
    {
        DRM_Adaptive_STATE = EN_DRM_DRM1;
    }

    else if(gstUdpPcsPara.udp_pcs_hold.drm_state & 0x0004)
    {
        DRM_Adaptive_STATE = EN_DRM_DRM2;
    }

    else if(gstUdpPcsPara.udp_pcs_hold.drm_state & 0x0008)
    {
        DRM_Adaptive_STATE = EN_DRM_DRM3;
    }

    else if(gstUdpPcsPara.udp_pcs_hold.drm_state & 0x0010)
    {
        DRM_Adaptive_STATE = EN_DRM_DRM4;
    }
    else
    {
        DRM_Adaptive_STATE = EN_DRM_DRM4;
    }

    /*DRM误码检测在监控卡中进行,DSP中的检测可加可不加*/
     drm_err_state = 0;

     for (uint8_t i = 0 ; i < 5; i++)//如果DRM发多个1，则认为发送错误
     {

         if ((gstUdpPcsPara.udp_pcs_hold.drm_state & (0x0001 << i)) != 0)
         {
             drm_err_state++;
         }

         if (drm_err_state>=2) //收到两个以上的1，设置DRM为0
         {
             DRM_Adaptive_STATE = EN_DRM_DRM0 ;
             drm_err_state = 0;
         }
                
      }

     
    switch (DRM_Adaptive_STATE)
    {
    // case EN_DRM_EN:
    //     SetPwrRef(gstPowerGiven.Ramp_gpwrgrid_active , gstPowerGiven.Ramp_gpwrgrid_reactive);
    //     break;
        case EN_DRM_DRM0:
            SetPwrRef(0 , 0);
            break;
        case EN_DRM_DRM1:
            SetPwrRef(0 , reactive_pwr);
            break;
        case EN_DRM_DRM2:
            SetPwrRef(active_pwr*0.50 , reactive_pwr);
            break;
        case EN_DRM_DRM3:
            SetPwrRef(active_pwr*0.75 , reactive_pwr);
            break;
        case EN_DRM_DRM4:
            SetPwrRef(active_pwr , reactive_pwr);
            break;
        
        default:
            SetPwrRef(0 , 0);
            break;
    }
    DRM_RELAY_CONTROL();

}


/* 过温降额上下文：每个被限功率通道（并网/油机）独立维护，避免 static 共用导致互相污染 */
typedef struct {
    uint32_t OverTemp_count;
    uint32_t LowTemp_count;
    uint8_t  is_overheat;
    bool     is_outpower_init;
    float32  out_power;
} OverTempLimitCtx_t;

/* 并网通道与油机通道各自独立的过温降额状态 */
static OverTempLimitCtx_t gOverTempCtxGrid;
static OverTempLimitCtx_t gOverTempCtxOil;

/**
* @brief: OverTemp_Limit
* @param: ctx          通道独立的过温降额状态上下文
* @param: Temp_limp    过温触发温度阈值
* @param: Temp_limp_Time 过温触发/恢复计数基准时间
* @param: set_power    上游设定功率
* @return: 经过过温降额处理后的输出功率
* @note: 过温功率限制；并网与油机各传入独立 ctx，互不影响
*/
float32 OverTemp_Limit(OverTempLimitCtx_t *ctx, float32 Temp_limp, uint16_t Temp_limp_Time, float32 set_power)
{
    const float32 POWER_MIN_RATIO = 0.05f;
    const float32 TEMP_RECOVER_HYS = 2.0f;
    float32 temp_recover_th = Temp_limp - TEMP_RECOVER_HYS;

    float32 Tempmax = GetNTCMaxTemp();

    if (Tempmax > Temp_limp) {
        ctx->OverTemp_count = (ctx->OverTemp_count < UINT32_MAX) ? (ctx->OverTemp_count + 1) : ctx->OverTemp_count;
        ctx->LowTemp_count = (ctx->LowTemp_count > 0) ? (ctx->LowTemp_count - 1) : 0;
    } else if (Tempmax < temp_recover_th) {
        ctx->LowTemp_count = (ctx->LowTemp_count < UINT32_MAX) ? (ctx->LowTemp_count + 1) : ctx->LowTemp_count;
        ctx->OverTemp_count = (ctx->OverTemp_count > 0) ? (ctx->OverTemp_count - 1) : 0;
    }

    if (ctx->OverTemp_count >= (200 * Temp_limp_Time)) {
        ctx->is_overheat = 1;
        ctx->OverTemp_count = 0;
    }

    if (ctx->LowTemp_count >= (10 * Temp_limp_Time)) {
        ctx->is_overheat = 0;
        ctx->LowTemp_count = 0;
    }

    /* A+ 保护：上游设定为 0（如市电模式下油机给定恒为 0）时，直接返回 0，
     * 不锁存 out_power，避免下次非零设定被旧的 0 钳死无法回升。*/
    if (set_power == 0.0f) {
        return 0.0f;
    }

    if (!ctx->is_outpower_init) {
        ctx->out_power = set_power;
        ctx->is_outpower_init = true;
    }

    if ((!ctx->is_overheat) && (Tempmax < temp_recover_th)) {
        ctx->out_power = set_power;
        return ctx->out_power;
    }

    if (fabsf(set_power) < fabsf(ctx->out_power)) {
        ctx->out_power = set_power;
    }

    if (ctx->is_overheat) {
        float32 power_reduction = (Tempmax - Temp_limp) * 0.1f;
        float32 target_power = 0.0f;
        float32 target_abs = 0.0f;
        float32 out_abs = 0.0f;

        power_reduction = (power_reduction > 0.95f) ? 0.95f : power_reduction;
        target_power = set_power * (1.0f - power_reduction);

        if (set_power >= 0.0f) {
            if (target_power < set_power * POWER_MIN_RATIO) {
                target_power = set_power * POWER_MIN_RATIO;
            }
        } else {
            if (target_power > set_power * POWER_MIN_RATIO) {
                target_power = set_power * POWER_MIN_RATIO;
            }
        }

        target_abs = fabsf(target_power);
        out_abs = fabsf(ctx->out_power);

        if (target_abs <= out_abs) {
            ctx->out_power = target_power;
        }
    } else {
        ctx->out_power = set_power;
    }

    return ctx->out_power;
}





/**
* @brief: EnergyScheduling()
* @param: void
* @return:Nothing
* @note:能量调度
*/
void EnergyScheduling()
{
    // float gpwroil_active = 0;
    // float gpwroil_reactive = 0;
    // float gpwrgrid_active = 0;
    // float gpwrgrid_reactive = 0;

    gstDeployInfo.inv_enable = InvSwitchStateGet();
    if (gstDeployInfo.inv_enable)
    {
        /*获取能量调度所需信息*/
        GetDeployInfo();

        EnergyCtrlInput_t* energy_ctrl_input = GetEnergyInput();
        EnergyCtrlOutput_t* energy_ctrl_output = GetEnergyOutput();

        if(Get_cla_Inv_state() == INV_STATE_STANDBY)
        {
            energy_ctrl_input->inv_stat = EN_STAND_BY_MODE;
        }
        else if(Get_cla_Inv_state() == INV_STATE_DISCONNECTED)
        {
            energy_ctrl_input->inv_stat = EN_OFF_GRID_MODE;
            OffGridFunc();
        }
        else if(Get_cla_Inv_state() == INV_STATE_GRID_CONNECTING || Get_cla_Inv_state() == INV_STATE_GEN_CONNECTING)
        {
            if(Get_cla_Inv_state() == INV_STATE_GRID_CONNECTING)
            {
                energy_ctrl_input->inv_stat = EN_OFF_GRID_TO_GRID_CONNECT_MODE;
            }
            OffToOnGridFunc();
        }
        else if(Get_cla_Inv_state() == INV_STATE_GRID_CONNECTED || Get_cla_Inv_state() == INV_STATE_GEN_CONNECTED)
        {
            if(Get_cla_Inv_state() == INV_STATE_GRID_CONNECTED)
            {
                energy_ctrl_input->inv_stat = EN_GRID_CONNECT_MODE;
            }
            OnGridFunc();
        }
        else if(Get_cla_Inv_state() == INV_STATE_GRID_DISCONNECTING || Get_cla_Inv_state() == INV_STATE_GEN_DISCONNECTING)
        {
            if(Get_cla_Inv_state() == INV_STATE_GRID_DISCONNECTING)
            {
                 energy_ctrl_input->inv_stat = EN_GRID_CONNECT_TO_OFF_GRID_MODE;
            }
            OnToOffGridFunc();
        }

        // energy_ctrl_input->inv_stat = Get_cla_Inv_state();
        energy_ctrl_input->work_mode = (EN_WORK_MODE)gstDeployInfo.work_mode;

        if (gstDeployInfo.chg_enable == 1)
            energy_ctrl_input->peak_shift_stat = 1;
        else if (gstDeployInfo.dsg_enable == 1)
            energy_ctrl_input->peak_shift_stat = 2;
        else
            energy_ctrl_input->peak_shift_stat = 0;

        energy_ctrl_input->ev_mode = 0;
        energy_ctrl_input->antireverseflow_enable = gstDeployInfo.antireverseflow_enable;
        // energy_ctrl_input->power_max_household_get = gstUdpPcsPara.udp_pcs_hold.network_power_limit_enable ?
        //     (float)gstUdpPcsPara.udp_pcs_hold.network_power_limit : MAX_GRID_POWER;
        energy_ctrl_input->power_max_household_get = fabsf(gstDeployInfo.on_grid_pwr);
        energy_ctrl_input->power_max_ongrid_get = fabsf(gstDeployInfo.on_grid_pwr);
        energy_ctrl_input->power_max_gen_get = gstDeployInfo.set_oil_power;
        energy_ctrl_input->power_max_bat_chg_get = fabsf(gstDeployInfo.set_batt_chg_power);
        energy_ctrl_input->power_max_bat_dsg_get = GetWarning_Other_R_Ct_Disconnect() ?  0.0f : fabsf(gstDeployInfo.set_batt_dsg_power);
        energy_ctrl_input->power_pv2bus_get = Pv_Get_Total_Pwr_Ave();
        energy_ctrl_input->power_ev2bus_get = 0;
        energy_ctrl_input->power_bat2bus_get = (-1)*gstAlgoInterface.bus2batpower;
        energy_ctrl_input->power_act_ct2grid_get = gstAlgoInterface.ct_active_power;
        energy_ctrl_input->power_act_norload_get = fabsf(gstAlgoInterface.normalload_active_power);
        energy_ctrl_input->power_act_grid2pcs_get = gstAlgoInterface.grid_active_power;
        energy_ctrl_input->power_act_inv2pcs_get = gstAlgoInterface.inv_active_power;
        energy_ctrl_input->power_act_gen2pcs_get = gstAlgoInterface.generator_active_power;
        energy_ctrl_input->power_act_eps_get = fabsf(gstAlgoInterface.load_active_power);
        energy_ctrl_input->power_react_ct2grid_get = gstAlgoInterface.ct_reactive_power;
        energy_ctrl_input->power_react_norload_get = gstAlgoInterface.normalload_reactive_power;
        energy_ctrl_input->power_react_grid2pcs_get = gstAlgoInterface.grid_reactive_power;
        energy_ctrl_input->power_react_inv2pcs_get = gstAlgoInterface.inv_reactive_power;
        energy_ctrl_input->power_react_gen2pcs_get = gstAlgoInterface.generator_reactive_power;
        energy_ctrl_input->power_react_eps_get = gstAlgoInterface.load_reactive_power;
        energy_ctrl_input->power_appar_ct2grid_get = gstAlgoInterface.ct_apperant_power;
        energy_ctrl_input->power_appar_norload_get = gstAlgoInterface.normalload_apperant_power;
        energy_ctrl_input->power_appar_grid2pcs_get = gstAlgoInterface.grid_apperant_power;
        energy_ctrl_input->power_appar_inv2pcs_get = gstAlgoInterface.inv_apperant_power;
        energy_ctrl_input->power_appar_gen2pcs_get = gstAlgoInterface.generator_apperant_power;
        energy_ctrl_input->power_appar_eps_get = fabsf(gstAlgoInterface.load_apperant_power);
        EnergyCtrlProcess();

        inverter_busvol_control_loop(gstAlgoInterface.bus_vol , energy_ctrl_output->power_act_grid2pcs_set , 0.005);
        gstPowerGiven.grid_pwr.active_pwr = OverTemp_limit_Enable ? OverTemp_Limit(&gOverTempCtxGrid, OverTemp_limit_Temp , OverTemp_limit_Time , bus_control_actpwr_out) : bus_control_actpwr_out;
        //gstPowerGiven.grid_pwr.active_pwr = bus_control_actpwr_out;
        gstPowerGiven.pv_pwr = energy_ctrl_output->power_pv2bus_set;
        //gstPowerGiven.grid_pwr.active_pwr = energy_ctrl_output->power_act_grid2pcs_set;
        //gstPowerGiven.oil_pwr = energy_ctrl_output->power_act_gen2pcs_set;
        gstPowerGiven.oil_pwr = OverTemp_limit_Enable ? OverTemp_Limit(&gOverTempCtxOil, OverTemp_limit_Temp , OverTemp_limit_Time , energy_ctrl_output->power_act_gen2pcs_set) :energy_ctrl_output->power_act_gen2pcs_set;

        Pv_Set_Pwr_Limit(gstPowerGiven.pv_pwr + gstPvPwrTotalNoiseVar.noise_std * 1.2f);
        
        // Pv_Set_Pwr_Limit(gstPowerGiven.pv_pwr);

        gstPowerGiven.Ramp_gpwroil_active = SetPowerRampUpdate(gstPowerGiven.Ramp_gpwroil_active, MAX_PWR_slope, gstPowerGiven.oil_pwr);
        //gstPowerGiven.Ramp_gpwroil_reactive = SetPowerRampUpdate(gstPowerGiven.Ramp_gpwroil_reactive, MAX_PWR_slope, 50.0f);
        gstPowerGiven.Ramp_gpwroil_reactive = -gstAlgoInterface.load_reactive_power;
        gstPowerGiven.Ramp_gpwrgrid_active = SetPowerRampUpdate(gstPowerGiven.Ramp_gpwrgrid_active, MAX_PWR_slope, gstPowerGiven.grid_pwr.active_pwr);
        //gstPowerGiven.Ramp_gpwrgrid_reactive = SetPowerRampUpdate(gstPowerGiven.Ramp_gpwrgrid_reactive, MAX_PWR_slope, gstPowerGiven.grid_pwr.reactive_pwr);
        gstPowerGiven.Ramp_gpwrgrid_reactive = -gstAlgoInterface.load_reactive_power;

        if(gstDeployInfo.work_mode == EN_OIL_MODE)//强制油机
        {
            //DRM_Adaptive_Recognition(gstPowerGiven.Ramp_gpwroil_active,gstPowerGiven.Ramp_gpwroil_reactive);
            SetPwrRef(gstPowerGiven.Ramp_gpwroil_active , gstPowerGiven.Ramp_gpwroil_reactive);
            //SetPwrRef(gstPowerGiven.oil_pwr,50.0f);
            InvWorkPermit_Monitor(gstDeployInfo.inv_enable , gstDeployInfo.oil_enable, 1);
        }
        else //市电
        {
            DRM_Adaptive_Recognition(gstPowerGiven.Ramp_gpwrgrid_active , gstPowerGiven.Ramp_gpwrgrid_reactive);
            //SetPwrRef(gstPowerGiven.Ramp_gpwrgrid_active , gstPowerGiven.Ramp_gpwrgrid_reactive);
            //SetPwrRef(gstPowerGiven.grid_pwr.active_pwr , gstPowerGiven.grid_pwr.reactive_pwr);
            InvWorkPermit_Monitor(gstDeployInfo.inv_enable , gstDeployInfo.grid_enable, 0);
        }

    }
}







