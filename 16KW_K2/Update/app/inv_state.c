/*
 * inv_state.c
 *
 *  Created on: 2025年3月26日
 *      Author: Nova006352
 */

#include "inv_state.h"
#include "cla_cpu_share.h"
#include "fault.h"
#include "ethernet_analysis.h"
#include <algo_interface.h>
#include "cla_control.h"
#include "cla_control_cpu.h"
#include "product_specification.h"
#include "debug_analysis.h"
#include "SysSelfTest.h"
#include "algo_interface.h" //TODO:调试接口

#pragma CODE_SECTION(InvWorkStateSwitch,".TI.ramfunc");

uint32_t gGridConnectCount;    //计数器
extern uint16_t DRM_RELAY_CONTROL_PERMIT;
uint32_t gFaultRecoverCheckTime = 0; //故障恢复检测时间
uint32_t gFaultRecoverCheckCount = 0; //故障恢复检测时间
uint16_t gFaultRecoverCountEnable = 0; //故障恢复计数使能
uint8_t  gFaultRecoverFlag = 0; //故障恢复标志

EN_INV_STATE geInvWorkState;    //逆变运行状态
InvActionPermit_t gstInvAction; //逆变动作权利许可
#define GRID_CONNECTED_VOL_THRESHOLD 3.0f
#define GRID_CONNECTED_VOLRMS_THRESHOLD 1.5f
#define GRID_CONNECTED_THETA_THRESHOLD 0.5f
#define GRID_CONNECTED_COUNT_THRESHOLD 2000
#define GEN_CONNECTED_VOL_THRESHOLD 7.0f
#define GEN_CONNECTED_VOLRMS_THRESHOLD 1.0f
#define GEN_CONNECTED_THETA_THRESHOLD 0.2f
#define GEN_CONNECTED_COUNT_THRESHOLD 2000
#define CPU_RUN_TS                          INV_CONTROL_TS  // 控制周期
//继电器打开延迟时间
#define REALY_DELAY_TIME 0.04f/CPU_RUN_TS  //继电器关断时间为0.04s
/**
* @brief: InvWorkPermit_Pv
* @param: inline void
* @return:Nothing
* @note:逆变工作和并网许可,PV部分
*/
static inline void InvWorkPermit_Pv(void)
{
    gstInvAction.pv_disconnect_permit = 1;  //PV离网开机许可
    gstInvAction.pv_connect_permit = 1;     //PV并网许可
}
/**
* @brief: InvWorkPermit_Bus
* @param: inline void
* @return:Nothing
* @note:逆变工作和并网许可,Bus部分
*/
static inline void InvWorkPermit_Bus(void)
{
    gstInvAction.bus_disconnect_permit = 1;  //BUS离网开机许可
    gstInvAction.bus_connect_permit = 1;     //BUS并网许可
}
/**
* @brief: InvWorkPermit_Grid
* @param: inline void
* @return:Nothing
* @note:逆变工作和并网许可,Grid部分
*/
static inline void InvWorkPermit_Grid(void)
{
    //举例
    GetRemoteShutdownStatus() == 0 ? (gstInvAction.grid_disconnect_permit = 1) : (gstInvAction.grid_disconnect_permit = 0);     //GRID离网开机许可
    if((GetWarning_Grid_VoltLow() == 1) || (GetWarning_Grid_VoltHigh() == 1) || (GetWarning_Grid_FreqLow() == 1) || (GetWarning_Grid_FreqHigh() == 1) || (DRM_RELAY_CONTROL_PERMIT==0)|| GetRemoteShutdownStatus()) 
    {
        gstInvAction.grid_connect_permit = 0;     //GRID并网许可
    }
    else
    {
        gstInvAction.grid_connect_permit = 1;     //GRID并网许可
    }
}
/**
* @brief: InvWorkPermit_Gen
* @param: inline void
* @return:Nothing
* @note:逆变工作和并网许可,Gen部分
*/
static inline void InvWorkPermit_Gen(void)
{
    gstInvAction.gen_disconnect_permit = 1;     
    if((GetWarning_Gen_VolLow() == 1) || (GetWarning_Gen_VolHigh() == 1) || (GetWarning_Gen_FreqLow() == 1) || (GetWarning_Gen_FreqHigh() == 1)) 
    {
        gstInvAction.gen_connect_permit = 0;     //GEN并网许可
    }
    else
    {
        gstInvAction.gen_connect_permit = 1;     //GEN并网许可
    }
}
/**
* @brief: InvWorkPermit_Monitor
* @param: inline void
* @return:Nothing
* @note:逆变工作和并网许可,Monitor部分
*/
void InvWorkPermit_Monitor(uint16_t en_inv, uint16_t en_grid, uint16_t gen_enable)
{
    gstInvAction.monitor_connect_permit = ((en_grid && en_inv ) || (gstInvInterface.en_oning_grid_cmd))&&(GSTATE_RELAY_GEN_START == 1);     //监控并网许可
    gstInvAction.monitor_disconnect_permit = ((en_inv || gstInvInterface.en_inv_cmd == 1))&&( GetPcsLLCEnable() == 1);  //监控离网开机许可
    gstInvAction.gen_enable = gen_enable;
}
/**
* @brief: InvWorkPermit_SelfTest
* @param: inline void
* @return:Nothing
* @note:逆变工作和并网许可,逆变器自检部分
*/
static inline void InvWorkPermit_SelfTest(void)
{
//    gstInvAction.self_test_disconnect_permit = 1;  //自检离网开机许可
}
/**
* @brief: InvWorkPermit_Parallel
* @param: inline void
* @return:Nothing
* @note:逆变工作和并网许可,并机部分
*/
static inline void InvWorkPermit_Parallel(void)
{
    gstInvAction.parallel_disconnect_permit = 1;  //监控离网开机许可
    gstInvAction.parallel_connect_permit = 1;     //监控并网许可
}
/**
* @brief: InvWorkPermit_Inv_Inside
* @param: inline void
* @return:Nothing
* @note:逆变工作和并网许可,逆变器内部部分
*/
static inline void InvWorkPermit_Inv_Inside(void)
{
    if(GetErrorFlag() != 0)//故障判断
//    if(GetError_InvTzFault() != 0)
    {
        gstInvAction.inv_inside_disconnect_permit = 0;
        gstInvAction.inv_inside_connect_permit = 0;        //逆变内部并网许可
    }
    else
    {
        gstInvAction.inv_inside_disconnect_permit = 1;
        gstInvAction.inv_inside_connect_permit = 1;        //逆变内部并网许可
    }
}
/**
* @brief: InvRelayPermit
* @param: inline void
* @return:Nothing
* @note:逆变工作和并网许可,并机部分
*/
static inline void InvRelayPermit(void)
{
//    gstInvAction.main_relay_permit = 1;     //主继电器闭合许可
//    gstInvAction.load_relay_permit = 1;     //负载继电器闭合许可
//    gstInvAction.grid_relay_permit = 1;     //市电继电器闭合许可
//    gstInvAction.gen_relay_permit = 1;      //发电机继电器闭合许可
//    if(BackupSwitchStateGet() == true)//后续加入放电深度等条件判断
//    {
//        gstInvAction.load_relay_permit = 1;
//    }
//    else
//    {
//        gstInvAction.load_relay_permit = 0;
//    }
}

static uint16_t Inv_grid_connecting_to_connected_condition(void)
{
    static uint32_t check_count = 0;
    // SetCpuDebugData(8,check_count);
    // SetCpuDebugData(9,fabs(gstAlgoInterface.inv_vol - gstAlgoInterface.grid_vol));
    // SetCpuDebugData(10,fabs(gstAlgoInterface.inv_vol_rms - gstAlgoInterface.grid_vol_rms));
    // SetCpuDebugData(11,fabs(gstAlgoInterface.inv_vol_theta - gstAlgoInterface.grid_vol_theta));
    if((fabs(gstAlgoInterface.inv_vol_rms - gstAlgoInterface.grid_vol_rms) < GRID_CONNECTED_VOLRMS_THRESHOLD) && (fabs(gstAlgoInterface.inv_vol_theta - gstAlgoInterface.grid_vol_theta) < GRID_CONNECTED_THETA_THRESHOLD))
    {
        check_count++;
    }
    else
    {
        if(check_count > 2)
        {
            check_count -= 2;
        }
        else
        {
            check_count = 0;
        }
    }
    if((check_count >= GRID_CONNECTED_COUNT_THRESHOLD) && (fabs(gstAlgoInterface.inv_vol - gstAlgoInterface.grid_vol) < GRID_CONNECTED_VOL_THRESHOLD))
    {
        check_count = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}
static uint16_t Inv_gen_connecting_to_connected_condition(void)
{
    static uint32_t check_count = 0;
    // SetCpuDebugData(8,check_count);
    // SetCpuDebugData(9,fabs(gstAlgoInterface.inv_vol - gstAlgoInterface.grid_vol));
    // SetCpuDebugData(10,fabs(gstAlgoInterface.inv_vol_rms - gstAlgoInterface.grid_vol_rms));
    // SetCpuDebugData(11,fabs(gstAlgoInterface.inv_vol_theta - gstAlgoInterface.grid_vol_theta));
    if((fabs(gstAlgoInterface.inv_vol_rms - gstAlgoInterface.generator_vol_rms) < GEN_CONNECTED_VOLRMS_THRESHOLD) && (fabs(gstAlgoInterface.inv_vol_theta - gstAlgoInterface.generator_vol_theta) < GEN_CONNECTED_THETA_THRESHOLD))
    {
        check_count++;
    }
    else
    {
        if(check_count > 2)
        {
            check_count -= 2;
        }
        else
        {
            check_count = 0;
        }
    }
    if((check_count >= GEN_CONNECTED_COUNT_THRESHOLD) && (fabs(gstAlgoInterface.inv_vol - gstAlgoInterface.generator_vol) < GEN_CONNECTED_VOL_THRESHOLD))
    {
        check_count = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}
static uint16_t Inv_grid_disconnecting_to_disconnected_condition(void)
{
    static uint32_t disconnecting_check_count = 0;
    if(gstAlgoInterface.grid_cur_rms < 1.0f)
    {
        disconnecting_check_count++;
    }
    else
    {
        disconnecting_check_count--;
    }
    if(disconnecting_check_count >= 200)
    {
        disconnecting_check_count = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}
static uint16_t Inv_gen_disconnecting_to_disconnected_condition(void)
{
    static uint32_t disconnecting_check_count = 0;
    if(gstAlgoInterface.generator_cur_rms < 1.0f)
    {
        disconnecting_check_count++;
    }
    else
    {
        disconnecting_check_count--;
    }
    if(disconnecting_check_count >= 200)
    {
        disconnecting_check_count = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}
static uint16_t Inv_disconnection_permit_check(void)
{
    return (gstInvAction.pv_disconnect_permit == 1) &&
           (gstInvAction.bus_disconnect_permit == 1) &&
           (gstInvAction.grid_disconnect_permit == 1) &&
           (gstInvAction.gen_disconnect_permit == 1) &&
           (gstInvAction.parallel_disconnect_permit == 1) &&
           (gstInvAction.inv_inside_disconnect_permit == 1);

}
static uint16_t Monitor_disconnect_check(void)
{
    return (gstInvAction.monitor_disconnect_permit == 1);
}

static uint16_t _IsletCheckLow()
{
    float limit_vol_low = 170.0f * 1.414f * gstAlgoInterface.grid_vol_sin;
    float limit_vol_low_abs = fabsf(limit_vol_low);
    float limit_vol_h = 270.0f * 1.414f * gstAlgoInterface.grid_vol_sin;
    float limit_vol_h_abs = fabsf(limit_vol_h);
    float grid_vol_abs = fabsf(gstAlgoInterface.grid_vol);
    float theta = gstAlgoInterface.grid_vol_theta;

    float check_theta = theta > 3.1415926 ? (theta - 3.1415926) : theta;
    static uint16_t check_count = 0;
    uint16_t check_err = 0;

    if(check_theta > 3.1415926 / 6 && check_theta < 3.1415926 * 5 / 6.0)
    {
        if(gstAlgoInterface.grid_vol > 0)
        {
            if(limit_vol_low > gstAlgoInterface.grid_vol || limit_vol_h < gstAlgoInterface.grid_vol)
            {
                check_err = 1;
            }
        }
        else
        {
            if(limit_vol_low < gstAlgoInterface.grid_vol || limit_vol_h > gstAlgoInterface.grid_vol)
            {
                check_err = 1;
            }
        }
    }
    else if(gstAlgoInterface.grid_vol > limit_vol_h + 100.0f)
    {
        check_err = 1;
        check_count += 10;
    }
    else if(gstAlgoInterface.grid_vol < limit_vol_low - 100.0f)
    {
        check_err = 1;
        check_count += 10;
    }

    if(check_count < 1)
    {
        check_count = 1;
    }
    if(check_err == 1)
    {
        check_count++;
    }
    else
    {
        check_count--;
    }
    if (check_count > 30)
    {
        check_count = 30;
        return 1;
    }
    return 0;
}

static uint16_t Inv_grid_connection_permit_check(void)
{
    return (gstInvAction.pv_connect_permit == 1) &&
           (gstInvAction.bus_connect_permit == 1) &&
           (gstInvAction.grid_connect_permit == 1) &&
           (gstInvAction.monitor_connect_permit == 1) &&
           (gstInvAction.gen_enable == 0) &&               //强制油机模式关闭时并电网
           (gstInvAction.parallel_connect_permit == 1) &&
           (gstInvAction.inv_inside_connect_permit == 1);

}
static uint16_t Inv_gen_connection_permit_check(void)
{
    return (gstInvAction.pv_connect_permit == 1) &&
           (gstInvAction.bus_connect_permit == 1) &&
           (gstInvAction.gen_connect_permit == 1) &&
           (gstInvAction.monitor_connect_permit == 1) &&
           (gstInvAction.gen_enable == 1) &&               //强制油机模式启动时并油机
           (gstInvAction.parallel_connect_permit == 1) &&
           (gstInvAction.inv_inside_connect_permit == 1);

}

// static uint8_t _FaultFlagCheck()
// {
//     uint8_t fault_flag = 0;

//     fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_volt_low;
//     fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_volt_over;
//     fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_short_circut;
//     fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_chg_overcurr;
//     fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_dsg_overcurr;
//     fault_flag |= gstFaultInfo.inside_err_inv.bit.inv_out_short;
//     fault_flag |= gstFaultInfo.inside_err_inv.bit.inv_vol_low;
//     fault_flag |= gstFaultInfo.inside_err_inv.bit.inv_vol_over;
//     fault_flag |= gstFaultInfo.inside_err_inv.bit.inv_over_curr;
//     fault_flag |= gstFaultInfo.inside_err_load.bit.over_load1;
//     fault_flag |= gstFaultInfo.inside_err_load.bit.over_load2;
//     fault_flag |= gstFaultInfo.inside_err_temp.bit.chassis_over_temp;
//     fault_flag |= gstFaultInfo.inside_err_temp.bit.inv_over_temp;
//     fault_flag |= gstFaultInfo.inside_err_temp.bit.pv_over_temp;
//     fault_flag |= gstFaultInfo.inside_err_temp.bit.radiator_over_temp;
    
//     return fault_flag;
// }

static void _FaultFlagClear()
{
    gstFaultInfo.inside_err_bus.bit.bus_volt_low = 0;
    gstFaultInfo.inside_err_bus.bit.bus_volt_over = 0;
    gstFaultInfo.inside_err_bus.bit.bus_short_circut = 0;
    gstFaultInfo.inside_err_bus.bit.bus_out_fault = 0;
    gstFaultInfo.inside_err_bus.bit.bus_chg_overcurr = 0;
    gstFaultInfo.inside_err_bus.bit.bus_dsg_overcurr = 0;
    gstFaultInfo.inside_err_inv.bit.inv_out_short = 0;
    gstFaultInfo.inside_err_inv.bit.inv_vol_low = 0;
    gstFaultInfo.inside_err_inv.bit.inv_vol_over = 0;
    gstFaultInfo.inside_err_inv.bit.inv_over_curr = 0;
    gstFaultInfo.inside_err_inv.bit.inv_out_fault = 0;
    gstFaultInfo.inside_err_load.bit.over_load = 0;
}

uint16_t GetRecoverError()
{
    uint16_t fault_flag = 0;

    fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_volt_low;
    fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_volt_over;
    fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_short_circut;
    fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_out_fault;
    fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_chg_overcurr;
    fault_flag |= gstFaultInfo.inside_err_bus.bit.bus_dsg_overcurr;
    fault_flag |= gstFaultInfo.inside_err_inv.bit.inv_out_short;
    fault_flag |= gstFaultInfo.inside_err_inv.bit.inv_vol_low;
    fault_flag |= gstFaultInfo.inside_err_inv.bit.inv_vol_over;
    fault_flag |= gstFaultInfo.inside_err_inv.bit.inv_over_curr;
    fault_flag |= gstFaultInfo.inside_err_inv.bit.inv_out_fault;
    fault_flag |= gstFaultInfo.inside_err_load.bit.over_load;
    return fault_flag;
}

uint8_t _FaultRecover()
{
    if(GetErrorFlag() == 0)
    {
        gFaultRecoverCheckTime = 0;
        return 1;
    }
    
    //恢复次数超过2次不再恢复
    if(gFaultRecoverCheckCount >= 2)
    {
        return 0;
    }
    
    gFaultRecoverCheckTime++;
    
    //计数一分钟，清除可恢复故障
    if(gFaultRecoverCheckTime == (uint32_t)(20000UL * 60UL))
    {
        if (GetRecoverError())
        {
            _FaultFlagClear();
            gFaultRecoverCountEnable = 1;
        }
        else
        {
            gFaultRecoverCountEnable = 0;
        }
    }

    //一分钟后如果还有故障，重新计时
    if(gFaultRecoverCheckTime > (uint32_t)(20000UL * 60UL))
    {
        if(gstFaultInfo.inside_err_grid.byte != 0 ||
            gstFaultInfo.inside_err_pv.byte[0] != 0 ||
            gstFaultInfo.inside_err_pv.byte[1] != 0 ||
            gstFaultInfo.inside_err_bus.byte != 0 ||
            gstFaultInfo.inside_err_inv.byte != 0 ||
            gstFaultInfo.inside_err_load.byte != 0 ||
            gstFaultInfo.inside_err_temp.byte != 0 ||
            gstFaultInfo.inside_err_gen.byte != 0 ||
            gstFaultInfo.inside_err_other.byte != 0)
        {
            gFaultRecoverCheckTime = 0;
        }
    }

    //两分钟后无故障，允许恢复
    if(gFaultRecoverCheckTime >= (uint32_t)(20000UL * 120UL))
    {
        if(gFaultRecoverCountEnable) 
        {
            gFaultRecoverCheckCount++;
            gFaultRecoverCountEnable = 0;
        }
        gFaultRecoverCheckTime = 0;
        gstFaultInfo.err_flag = 0;
        return 1;
    }

    return 0;
}

/**
* @brief: InvWorkStateSwitch
* @param: inline void
* @return:Nothing
* @note:逆变运行状态切换
*/
static inline void InvWorkStateSwitch(void)
{
    static uint32_t connected_time = 0;
    static uint32_t disconnected_time = 0;
    static uint32_t timeout = 0;
//    gstInvAction.load_relay_permit = 1; //TODO:调试用
    gstInvAction.main_relay_permit = 1; //TODO:调试用
    switch (geInvWorkState)
    {
        case INV_STATE_STANDBY://逆变待机状态
        {
            RELAY_TOTAL_CLOSE;            
            if(GetSysTimeOutFlag() || GetSysSelfTestFlag())    //正常情况由自检代码接管继电器控制
            {
                RELAY_GRID_CLOSE;
            }
            RELAY_GEN_CLOSE;
            gFaultRecoverFlag = _FaultRecover();
            if(Inv_disconnection_permit_check() && Monitor_disconnect_check() && gFaultRecoverFlag)
            {
                geInvWorkState = INV_STATE_SELFTEST;
                geInvControlState = INV_STATE_SELFTEST;
                timeout = TimGetSystime();
            }
            else
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
            }
            break;
        }
        case INV_STATE_SELFTEST://逆变自检状态
        {
            //自检是否通过
            if (Inv_disconnection_permit_check() != 1)
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
                /* code */
            }
            
            if(gstInvAction.self_test_disconnect_permit == 1)
            {
                if(gstInvInterface.openloop_inv_cmd == 1) // TODO:调试用，用完删除
                {
                    geInvWorkState = INV_STATE_OPENING;
                    geInvControlState = INV_STATE_OPENING;
                }
                else
                {
                     if(TimDifftimeFromNow(timeout)  < 4000)
                    {
                        break;
                    }
                    geInvWorkState = INV_STATE_DISCONNECTED;
                    geInvControlState = INV_STATE_DISCONNECTED;
                    timeout = TimGetSystime();
                }
            }
            else
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
            }
            break;
        }
        case INV_STATE_OPENING://逆变开环状态
        {
            if(Inv_disconnection_permit_check() && Monitor_disconnect_check())
            {
                RELAY_TOTAL_OPEN;
            }
            else
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
                RELAY_TOTAL_CLOSE;
                // RELAY_BACKUP_CLOSE;
            }

            break;
        }
        case INV_STATE_DISCONNECTED://逆变闭环状态
        {
            /*      继电器动作部分        */
            if(gstInvAction.main_relay_permit == 1)
            {
                //判断一下离网模式下电压符合要求,监控等也允许闭合继电器,闭合主继电器
                RELAY_TOTAL_OPEN;
            }
            else
            {
                //断开主继电器
                RELAY_TOTAL_CLOSE;
            }
            if(disconnected_time < 20000)
            {
                disconnected_time++;
            }

            /*      并网判断        */
            if(Inv_grid_connection_permit_check() && disconnected_time >= 20000)
            {
                gGridConnectCount = 0;
                geInvWorkState = INV_STATE_GRID_CONNECTING;
                geInvControlState = INV_STATE_GRID_CONNECTING;
                disconnected_time = 0;
            }
            else if(Inv_gen_connection_permit_check() && disconnected_time >= 20000)
            {
                gGridConnectCount = 0;
                geInvWorkState = INV_STATE_GEN_CONNECTING;
                geInvControlState = INV_STATE_GEN_CONNECTING;
                disconnected_time = 0;
            }
            /*      故障退出        */
            else if(Inv_disconnection_permit_check() != 1 || Monitor_disconnect_check() != 1)
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
                RELAY_TOTAL_CLOSE;
                disconnected_time = 0;
                // RELAY_BACKUP_CLOSE;
            }
            break;
        }
        case INV_STATE_GRID_CONNECTING://逆变并网中状态
        {
            /*      故障退出        */
            if(Inv_disconnection_permit_check() != 1)
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
                RELAY_TOTAL_CLOSE;
                // RELAY_BACKUP_CLOSE;
            }
            else if(Inv_grid_connection_permit_check() != 1)
            {
                geInvWorkState = INV_STATE_DISCONNECTED;
                geInvControlState = INV_STATE_DISCONNECTED;

            }
            else if(Inv_grid_connecting_to_connected_condition() == 1)
            {
                geInvWorkState = INV_STATE_GRID_CONNECTED;
                geInvControlState = INV_STATE_GRID_CONNECTING;
                RELAY_GRID_OPEN;
                /* code */
            }
            connected_time = 0;
            break;
        }
        case INV_STATE_GEN_CONNECTING://逆变并网中状态
        {
            /*      故障退出        */
            if(Inv_disconnection_permit_check() != 1)
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
                RELAY_TOTAL_CLOSE;
                // RELAY_BACKUP_CLOSE;
            }
            else if(Inv_gen_connection_permit_check() != 1)
            {
                geInvWorkState = INV_STATE_DISCONNECTED;
                geInvControlState = INV_STATE_DISCONNECTED;

            }
            // else if(Inv_grid_connecting_to_connected_condition() == 1 || gstInvInterface.en_on_grid_cmd == 1)
            else if(Inv_gen_connecting_to_connected_condition() == 1)
            {
                geInvWorkState = INV_STATE_GEN_CONNECTED;
                geInvControlState = INV_STATE_GEN_CONNECTING;
                RELAY_GEN_OPEN;
                /* code */
            }
            connected_time = 0;
            break;
        }
        case INV_STATE_GRID_CONNECTED://逆变已并网状态
        {
            if(connected_time<20000)
            {
                connected_time++;
            }
            /*      故障退出        */
            if(Inv_disconnection_permit_check() != 1)
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
                gGridConnectCount = 0;
            }
            else if((Inv_grid_connection_permit_check() != 1) || (Monitor_disconnect_check() != 1))
            {
                geInvWorkState = INV_STATE_GRID_DISCONNECTING;
                geInvControlState = INV_STATE_GRID_DISCONNECTING;
            }
            else if((connected_time >= 20000 && _IsletCheckLow() == 1))
            {
                if (GetUps0msEnable())
                {
                    geInvWorkState = INV_STATE_GRID_DISCONNECTING;
                    geInvControlState = INV_STATE_GRID_DISCONNECTING;
                }
            }
            else if(gGridConnectCount >= REALY_DELAY_TIME )
            {   
                gGridConnectCount = 0;
                geInvControlState = INV_STATE_GRID_CONNECTED;
            }
            if(gGridConnectCount < REALY_DELAY_TIME )
            {
                gGridConnectCount ++;
            }
            break;
        }
        case INV_STATE_GEN_CONNECTED://逆变已并网状态
        {
            /*      故障退出        */
            if(Inv_disconnection_permit_check() != 1)
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
                gGridConnectCount = 0;
            }
            else if((Inv_gen_connection_permit_check() != 1) || (Monitor_disconnect_check() != 1))
            {
                geInvWorkState = INV_STATE_GEN_DISCONNECTING;
                geInvControlState = INV_STATE_GEN_DISCONNECTING;
            }
            else if(gGridConnectCount >= REALY_DELAY_TIME )
            {   
                gGridConnectCount = 0;
                geInvControlState = INV_STATE_GEN_CONNECTED;
            }
            if(gGridConnectCount < REALY_DELAY_TIME )
            {
                gGridConnectCount ++;
            }

            break;
        }
        case INV_STATE_GRID_DISCONNECTING://逆变离网中状态
        {
            if(Inv_disconnection_permit_check() != 1)
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
                gGridConnectCount = 0;
            }
            else if(Inv_grid_disconnecting_to_disconnected_condition() == 1)
            {
                RELAY_GRID_CLOSE;
                geInvWorkState = INV_STATE_DISCONNECTED;
                geInvControlState = INV_STATE_DISCONNECTED;
            }

            break;
        }
        case INV_STATE_GEN_DISCONNECTING://逆变离网中状态
        {
            if(Inv_disconnection_permit_check() != 1)
            {
                geInvWorkState = INV_STATE_STANDBY;
                geInvControlState = INV_STATE_STANDBY;
                gGridConnectCount = 0;
            }
            else if(Inv_gen_disconnecting_to_disconnected_condition() == 1)
            {
                RELAY_GEN_CLOSE;
                geInvWorkState = INV_STATE_DISCONNECTED;
                geInvControlState = INV_STATE_DISCONNECTED;
            }

            break;
        }
        default:
        {
            geInvWorkState = INV_STATE_STANDBY;
            geInvControlState = INV_STATE_STANDBY;
            break;
        }
    }
}

/**
* @brief: Inv_State_Int1
* @param: static inline void
* @return:Nothing
* @note:逆变状态机函数
*/
void Inv_State_Int1(void)
{
    //各个模块状态位许可判断
    InvWorkPermit_Pv();
    InvWorkPermit_Bus();
    InvWorkPermit_Grid();
    InvWorkPermit_Gen();
//    InvWorkPermit_Monitor(0,0);
    InvWorkPermit_SelfTest();
    InvWorkPermit_Parallel();
    InvWorkPermit_Inv_Inside();
    //继电器状态位许可判断
    InvRelayPermit();
    //逆变状态机切换函数
    InvWorkStateSwitch();
    // CPU侧同步CLA实际状态（由CLA迁移）
    ClaControlCpuRunStateSync();
}




