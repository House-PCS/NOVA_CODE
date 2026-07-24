/*
 * control_state.c
 *  State machine
 *  Created on: 2025-3-22
 *      Author: Nova007456
 */

#include "control_state.h"
#include "cla_cpu_share.h"
#include "ph3_calc_ifc_update.h"
#include "debug_analysis.h"
#include "fault.h"
#include "gpio.h"
#include "ph3_sts_ifc_update.h"
#include "relay_ctrl.h"
#include <sample_update.h>
#include "cpu_timers.h"
#include "post.h"
#include "ethernet_analysis.h"
#include "algo_interface.h"
#include "flag_dly_trigger.h"
#include "biz_fsm_interface.h"
#include "dual_core.h"
#include "dev_dual_core_receive.h"
#include "cpu_control.h"
#include "AlgoFw/package/cntl/pv/pv.h"
#include "pwr_ctrl_interface.h"
#include <string.h>
#include "phase_seq.h"

#define INV_BUS_VOLT_DLY_COUNT   (7U)
#define INV_PV_VOLT_DLY_COUNT    (7U)

volatile EN_INV_STATE gstPcsInvRunState;

ActionPermit_t gstControlAction;

static uint16_t connect_delay_cont;
static uint16_t _gMointerConnectType;
void ControlStateInit(void)
{
    gstPcsInvRunState = INV_STATE_STANDBY;
    connect_delay_cont = 0;
    _gMointerConnectType = 0;
    memset((uint16_t*)&gstControlAction, 0, (int)sizeof(ActionPermit_t));
    PhaseSeq_CfgInit(&_gstGridPhaseSeq);
    PhaseSeq_CfgInit(&_gstGenPhaseSeq);
}

static uint8_t _ginvBusPermitOk = 1U;
FLAG_DLY_PAIR_DECLARE(inv_bus_permit, &_ginvBusPermitOk, INV_BUS_VOLT_DLY_COUNT);

static uint8_t _gPvBusPermitOk = 1U;
FLAG_DLY_PAIR_DECLARE(pv_bus_permit, &_gPvBusPermitOk, INV_PV_VOLT_DLY_COUNT);


void BackUpSwitchCtrlFunc(void)
{
    if(BackupSwitchStateGet() && GetBatDsgEnable() && LoadRelayNoneFaultEn() && GetSysSelfTestFlag())
    {
        RelayControl(EN_RELAT_CTRL_TYPE_LOAD, RELAY_ON);
    }
    else
    {
        RelayControl(EN_RELAT_CTRL_TYPE_LOAD, RELAY_OFF);
    }
}


static uint8_t _Inv_BusVoltBad(void)
{
    return (uint8_t)((gstPh3CalcInfo.bus.pos_vol_filter < BUS_VOLT_LOW_FAULT) ||
                     (gstPh3CalcInfo.bus.neg_vol_filter < BUS_VOLT_LOW_FAULT) ||
                     (gstPh3CalcInfo.bus.pos_vol_filter > BUS_VOLT_HIGH) ||
                     (gstPh3CalcInfo.bus.neg_vol_filter > BUS_VOLT_HIGH));
}

static uint8_t _Pv_BusVoltBad(void)
{
    return (uint8_t)((gstPh3CalcInfo.bus.pos_vol_filter < BUS_VOLT_LOW_FAULT) ||
                     (gstPh3CalcInfo.bus.neg_vol_filter < BUS_VOLT_LOW_FAULT) ||
                     (gstPh3CalcInfo.bus.pos_vol_filter > BUS_VOLT_HIGH) ||
                     (gstPh3CalcInfo.bus.neg_vol_filter > BUS_VOLT_HIGH));
}

/**
* @brief: InvWorkPermit_Pv
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, PV section
*/
static void _PvWorkPermit_Bus(void)
{
    char rst = 0;

    FLAG_DLY_CLEAR_RUN(pv_bus_permit, { rst = _Pv_BusVoltBad(); });
    FLAG_DLY_SET_RUN(pv_bus_permit_recover, { rst = !_Pv_BusVoltBad(); });

    gstControlAction.pv_bus_permit = _gPvBusPermitOk;
}

/**
* @brief: InvWorkPermit_Bus
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, bus section
*/
static void _InvWorkPermit_Bus(void)
{
    char rst = 0;

    FLAG_DLY_CLEAR_RUN(inv_bus_permit, { rst = _Inv_BusVoltBad(); });
    FLAG_DLY_SET_RUN(inv_bus_permit_recover, { rst = !_Inv_BusVoltBad(); });

    gstControlAction.inv_bus_disconnect_permit = _ginvBusPermitOk;
    gstControlAction.inv_bus_connect_permit = _ginvBusPermitOk;
}


/**
* @brief: InvWorkPermit_Monitor
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, monitor section
*/
static void _InvWorkPermit_Monitor(void)
{
    gstControlAction.monitor_inv_connect_permit = (BizFsm_Interface_GetAllowOffgrid() && \
                                                    BizFsm_Interface_GetAllowGridConnect());
    gstControlAction.monitor_inv_disconnect_permit = BizFsm_Interface_GetAllowOffgrid();
}
/**
* @brief: InvWorkPermit_Monitor
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, monitor section
*/
static void _PvWorkPermit_Monitor(void)
{
    gstControlAction.monitor_pv_permit = BizFsm_Interface_GetAllowPv();
}

/**
* @brief: InvWorkPermit_Parallel
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, parallel section
*/
static inline void _InvWorkPermit_Parallel(void)
{
    gstControlAction.parallel_disconnect_permit = 1;  //Monitor off-grid start permission
    gstControlAction.parallel_connect_permit = 1;     //Monitor grid-connect permission
}
/**
* @brief: InvWorkPermit_Inv_Inside
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, inverter internal section
*/
static inline void _InvWorkPermit_Err_Inside(void)
{
    if(GetErrorFlag() != 0)
    {
        gstControlAction.inv_inside_err_disconnect_permit = 0;
        gstControlAction.inv_inside_err_connect_permit = 0;        //Inverter internal grid-connect permission
    }
    else
    {
        gstControlAction.inv_inside_err_disconnect_permit = 1;
        gstControlAction.inv_inside_err_connect_permit = 1;        //Inverter internal grid-connect permission
    }
}
/**
* @brief: InvWorkPermit_Inv_Inside
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, inverter internal section
*/
static inline void _PvWorkPermit_Err_Inside(void)
{
    if(GetErrorFlag() != 0)
    {
        gstControlAction.pv_inside_err_connect_permit = 0;
    }
    else
    {
        gstControlAction.pv_inside_err_connect_permit = 1;
    }
}

/**
* @brief: InvWorkPermit_Inv_Inside
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, inverter internal section
*/
static inline void _WorkPermit_Cpu2(void)
{

    gstControlAction.cpu2_run_permit = (DualCore_IsCpu2PowerOnOnline() == 1 && DualCore_IsCpu2RunOnline() == 1);
}

/**
* @brief: InvWorkPermit_Inv_Inside
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, inverter internal section
*/
static inline void _InvWorkPermit_StsConnecting(void)
{
    if(Get_StsOutput_IsConnecting() || Get_StsOutput_IsDisconnectd())
    {
        if(!Get_StsOutput_IsWarn())
        {
            gstControlAction.sts_inv_connecting_permit = 1;
        }
        else
        {
            gstControlAction.sts_inv_connecting_permit = 0;
        }
    }
}
static inline void _InvWorkPermit_StsErr(void)
{
    if(!Get_StsOutput_IsErr())
    {
        gstControlAction.sts_err_connected_permit = 1;
    }
    else
    {
        gstControlAction.sts_err_connected_permit = 0;
    }
}

static inline void _InvWorkPermit_StsInsideStop(void)
{
    //对应孤岛检测之类的

    gstControlAction.sts_inside_connected_permit = 1;
}
/**
* @brief: InvWorkPermit_Pv
* @param: inline void
* @return:Nothing
* @note:Inverter work/connect permission, PV section
*/
static void _WorkPermit_InvConnectedChk(void)
{
    gstControlAction.sts_run_connected_permit = Get_StsOutput_IsConnected();
}



static void _WorkPermit_ConectDelayChk(void)
{
    if(Get_StsOutput_IsConnected())
    {
        connect_delay_cont ++;
        gstControlAction.connect_delay_permit = ((connect_delay_cont >= 500) ? 1 : 0);
    }
    else
    {
        connect_delay_cont = 0;
    }
}

static void _WorkPermit_ElseChk(void)
{
//    gstControlAction.connect_time_permit = GetConnectTimeEN();
//    gstControlAction.grid_drm_permit = GetDrmGridConnectEN();

    //TODO 当前代码存在问题
    gstControlAction.connect_time_permit = 1;
    gstControlAction.grid_drm_permit = 1;
}

static void _WorkPermit_Force_GEnChk(void)
{
//    _gMointerConnectType = GetInvConnectType();                  /*获取监控卡最新信息*/
    _gMointerConnectType = 0;                  /*获取监控卡最新信息*/
    if(_gMointerConnectType == 2)
    {
        if(Get_StsOutput_IsConnected() || Get_StsOutput_IsDisconnecting())
        {
            _gMointerConnectType = gstControlAction.force_gen_connect_permit;
        }
        else
        {
            if(GetStsGenForceState() && (!gstControlAction.gen_phase_chk_permit || Get_StsOutput_IsWarn()))  //当前并网模式选择出现问题
            {
                _gMointerConnectType = 0;
                //进行延时

            }
            else if(!GetStsGenForceState() && (!gstControlAction.grid_phase_chk_permit || Get_StsOutput_IsWarn()))
            {
                _gMointerConnectType = 1;
                //进行延时
            }
        }
    }
    if(_gMointerConnectType != gstControlAction.force_gen_connect_permit)      /*监控卡与本地允许标志不符时,切换并网状态*/
    {
        gstControlAction.mointer_connect_type_chk = false;
    }
}

static uint8_t _SourceRelayIsClosed(void)
{
    if (GetStsGenForceState())
    {
        return (uint8_t)GetRelayState(EN_RELAT_CTRL_TYPE_GEN);
    }
    return (uint8_t)GetRelayState(EN_RELAT_CTRL_TYPE_GRID);
}

uint16_t WorkPermit_GenForceEn(void)
{
    return gstControlAction.force_gen_connect_permit;
}

static void _InvWorkRunControl(void)
{

    bool inv_run_flag = false;
    bool sts_run_en_flag = false;
    bool connecting_en_flag = false;
    bool connect_run_flag = false;

    float volt_adjust = 0;
    float w_adjust = 0;
    static float connecting_volt_adjust;
//    float sts_block_code = 0;
    if(gstControlAction.cpu2_run_permit && gstControlAction.inv_bus_disconnect_permit &&
            gstControlAction.inv_inside_err_disconnect_permit && gstControlAction.monitor_inv_disconnect_permit &&\
            gstControlAction.parallel_disconnect_permit && gstControlAction.sts_err_connected_permit)
    {
        inv_run_flag = true;
    }

    if(inv_run_flag && gstControlAction.inv_bus_connect_permit && gstControlAction.inv_inside_err_connect_permit &&\
            gstControlAction.monitor_inv_connect_permit && gstControlAction.parallel_connect_permit  &&
            gstControlAction.connect_time_permit && gstControlAction.sts_inside_connected_permit &&\
           (GetStsGenForceState() ? gstControlAction.gen_phase_chk_permit : 1) && gstControlAction.sts_inv_connecting_permit &&\
           (!GetStsGenForceState() ? gstControlAction.grid_phase_chk_permit : 1) && \
           (!GetStsGenForceState() ? gstControlAction.grid_drm_permit : 1) && gstControlAction.mointer_connect_type_chk)
    {
        sts_run_en_flag = true;
    }
//    if(!sts_run_en_flag)
//    {
//        if(!inv_run_flag) {sts_block_code = 1;}
//        if(!gstControlAction.inv_bus_connect_permit) {sts_block_code = 2;}
//        if(!gstControlAction.inv_inside_err_connect_permit) {sts_block_code = 3;}
//        if(!gstControlAction.monitor_inv_connect_permit) {sts_block_code = 4;}
//        if(!gstControlAction.parallel_connect_permit) {sts_block_code = 5;}
//        if(!gstControlAction.connect_time_permit) {sts_block_code = 6;}
//        if(!gstControlAction.sts_inside_connected_permit) {sts_block_code = 7;}
//        if(!(GetStsGenForceState() ? gstControlAction.gen_phase_chk_permit : 1)) {sts_block_code = 8;}
//        if(!gstControlAction.sts_inv_connecting_permit) {sts_block_code = 9;}
//        if(!(!GetStsGenForceState() ? gstControlAction.grid_phase_chk_permit : 1)) {sts_block_code = 10;}
//        if(!(!GetStsGenForceState() ? gstControlAction.grid_drm_permit : 1)) {sts_block_code = 11;}
//        if(!gstControlAction.mointer_connect_type_chk) {sts_block_code = 12;}
//    }
    //
    if(sts_run_en_flag && gstControlAction.sts_inv_connecting_permit)
    {
        connecting_en_flag = true;
    }

    if(inv_run_flag && connecting_en_flag && gstControlAction.connect_delay_permit && gstControlAction.sts_run_connected_permit)
    {
        connect_run_flag = true;
    }

    bool source_relay_closed = (_SourceRelayIsClosed() != 0U);
    if(connecting_en_flag && !source_relay_closed)
    {
        volt_adjust = Get_StsOutput_VolRmsAdjust();
        connecting_volt_adjust = volt_adjust;
        w_adjust = Get_StsOutput_wAdjust();        
    }
    else if(!connect_run_flag && source_relay_closed)
    {
        volt_adjust = connecting_volt_adjust;
        float32 src_freq = (GetStsGenForceState() ? gstPh3CalcInfo.gen_vol.freq : gstPh3CalcInfo.grid_vol.freq); 
        Inv3Ph_Set_W0(src_freq * M_2PI);              
    }
    else if(source_relay_closed && connect_run_flag)
    {
        float32 src_theta = (GetStsGenForceState() ? gstPh3CalcInfo.gen_vol.theta : gstPh3CalcInfo.grid_vol.theta);
        Inv3Ph_Set_ConnectedTheta(src_theta);
    }
    else if(!connecting_en_flag)
    {
        gstControlAction.connect_delay_permit = 0;
        connect_delay_cont = 0;
    }

    Inv3Ph_Set_AdjW(w_adjust);
    Inv3Ph_Set_ControlVoltRmsDiff(volt_adjust);

    Inv3PhSetEnRun(inv_run_flag);
    Inv3PhSetEnConnect(connecting_en_flag);
    Inv3PhSetEnConnectRun(connect_run_flag);
    //使能STS运行
    SetStsEnEnConnect(sts_run_en_flag);

    SetCpuDebugData(89,  inv_run_flag);//105
    SetCpuDebugData(90,  connecting_en_flag);//106
    SetCpuDebugData(91,  connect_run_flag);//107
    SetCpuDebugData(92,  sts_run_en_flag);//108

}

static void _PvWorkRunControl(void)
{
    bool run_flag = false;
    if(gstControlAction.monitor_pv_permit && gstControlAction.pv_bus_permit &&\
            gstControlAction.pv_inside_err_connect_permit)
    {
        run_flag = true;
    }
    Pv_Set_EnRun(run_flag);
    PVRunSet(run_flag);
}

static void _InvStateMapUpdate(void)
{
    EN_INV3PH_STATE inv_state = Get3PhRunState();
    switch (inv_state)
    {
        case EN_INV3PH_STATE_IDLE:
        {
            gstPcsInvRunState = INV_STATE_STANDBY;
            break;
        }
        case EN_INV3PH_STATE_ERR:
        {
            gstPcsInvRunState = INV_STATE_STANDBY;
            break;
        }
        case EN_INV3PH_STATE_SELFCHECK:
        {
            gstPcsInvRunState = INV_STATE_SELFTEST;
            break;
        }
        case EN_INV3PH_STATE_STARTING:
        {
            gstPcsInvRunState = INV_STATE_DISCONNECTED;
            break;
        }
        case EN_INV3PH_STATE_DISCONNECTED:
        {
            //进行并网模式设置
            gstControlAction.force_gen_connect_permit = _gMointerConnectType;
            gstControlAction.mointer_connect_type_chk = true;
            //设置STS运行切换模式
            SetStsEnForceGenMode(gstControlAction.force_gen_connect_permit);

            gstPcsInvRunState = INV_STATE_DISCONNECTED;
            break;
        }
        case EN_INV3PH_STATE_CONNECTING:
        {
            if(gstControlAction.force_gen_connect_permit)
            {
                gstPcsInvRunState = INV_STATE_GEN_CONNECTING;
            }
            else
            {
                gstPcsInvRunState = INV_STATE_GRID_CONNECTING;
            }
            break;
        }
        case EN_INV3PH_STATE_CONNECTED:
        {
            if(gstControlAction.force_gen_connect_permit)
            {
                gstPcsInvRunState = INV_STATE_GEN_CONNECTED;
            }
            else
            {
                gstPcsInvRunState = INV_STATE_GRID_CONNECTED;
            }
            break;
        }
        case EN_INV3PH_STATE_DISCONNECTING:
        {
            if(gstControlAction.force_gen_connect_permit)
            {
                gstPcsInvRunState = INV_STATE_GEN_DISCONNECTING;
            }
            else
            {
                gstPcsInvRunState = INV_STATE_GRID_DISCONNECTING;
            }
            break;
        }
        default:
        {
            gstPcsInvRunState = INV_STATE_STANDBY;
            break;
        }
    }
}
static void _InvTotalRelagControl(void)
{
    if(GetInvRelayFlag())
    {
        //TODO    这里有些旁路模式控制策略
        if(0)
        {
            //完成预同步才能开总INV继电器
        }
        else
        {
            RelayControl(EN_RELAT_CTRL_TYPE_INV, true);
        }
    }
    else
    {
        RelayControl(EN_RELAT_CTRL_TYPE_INV, RELAY_OFF);
    }
}


static void _GridAndGenPhaseJudge(void)
{
    float grid_a_theta,grid_b_theta,grid_c_theta;
    float gen_a_theta,gen_b_theta,gen_c_theta;

    grid_a_theta = PHASE_CNT_TO_THETA_RAD(gstPh3CalcInfo.grid_a_phase_count, gstPh3CalcInfo.grid_vol.freq);
    grid_b_theta = PHASE_CNT_TO_THETA_RAD(gstPh3CalcInfo.grid_b_phase_count, gstPh3CalcInfo.grid_vol.freq);
    grid_c_theta = PHASE_CNT_TO_THETA_RAD(gstPh3CalcInfo.grid_c_phase_count, gstPh3CalcInfo.grid_vol.freq);

    gen_a_theta = PHASE_CNT_TO_THETA_RAD(gstPh3CalcInfo.gen_a_phase_count, gstPh3CalcInfo.gen_vol.freq);
    gen_b_theta = PHASE_CNT_TO_THETA_RAD(gstPh3CalcInfo.gen_b_phase_count, gstPh3CalcInfo.gen_vol.freq);
    gen_c_theta = PHASE_CNT_TO_THETA_RAD(gstPh3CalcInfo.gen_c_phase_count, gstPh3CalcInfo.gen_vol.freq);

    if(Get_StsOutput_IsDisconnectd())
    {
        Sts_PhaseSeq_Analyze(grid_a_theta, grid_b_theta, grid_c_theta,
            gstPh3CalcInfo.grid_vol.a_phase_rms, gstPh3CalcInfo.grid_vol.b_phase_rms, gstPh3CalcInfo.grid_vol.c_phase_rms,
            &_gstGridPhaseSeq, &_gstGridPhaseSeqResult, 1);
        Sts_PhaseSeq_Analyze(gen_a_theta, gen_b_theta, gen_c_theta,
            gstPh3CalcInfo.gen_vol.a_phase_rms, gstPh3CalcInfo.gen_vol.b_phase_rms, gstPh3CalcInfo.gen_vol.c_phase_rms,
            &_gstGenPhaseSeq, &_gstGenPhaseSeqResult, 1);
    }
    gstControlAction.gen_phase_chk_permit = ((GetStsGenForceState() && _gstGenPhaseSeqResult.is_normal) ? 1 : 0);
    gstControlAction.grid_phase_chk_permit = ((!GetStsGenForceState() && _gstGridPhaseSeqResult.is_normal) ? 1 : 0);
}

static void _GridAndGenRelagControl(void)
{
    if(GetStsGenForceState())
    {
        RelayControl(EN_RELAT_CTRL_TYPE_GRID, false);
    }
    else
    {
        RelayControl(EN_RELAT_CTRL_TYPE_GEN, false);
    }
    if(Get_StsOutput_RelagState())
    {
        if(GetStsGenForceState() && !GetRelayState(EN_RELAT_CTRL_TYPE_GEN))
        {
            RelayControl(EN_RELAT_CTRL_TYPE_GEN, true);
        }
        else if(!GetStsGenForceState() && !GetRelayState(EN_RELAT_CTRL_TYPE_GRID))
        {
            RelayControl(EN_RELAT_CTRL_TYPE_GRID, true);
        }
    }
    else
    {
        RelayControl(EN_RELAT_CTRL_TYPE_GEN, false);
        RelayControl(EN_RELAT_CTRL_TYPE_GRID, false);
    }
    if(GetErrorFlag() != 0)
    {
        RelayControl(EN_RELAT_CTRL_TYPE_GEN, false);
        RelayControl(EN_RELAT_CTRL_TYPE_GRID, false);
    }
}
/**
* @brief: Inv_State_Int1
* @param: static inline void
* @return:Nothing
* @note:Inverter state machine
*/
void Inv_State_Int1(void)
{
    StsOutputUpdate();
    _InvWorkPermit_StsErr();
    _InvWorkPermit_StsInsideStop();
    _InvWorkPermit_Err_Inside();
    _PvWorkPermit_Err_Inside();
    _WorkPermit_Cpu2();

    _WorkPermit_InvConnectedChk();
    _WorkPermit_ConectDelayChk();    //VF切PQ使用,延时

    _GridAndGenRelagControl();

   Inv3PhDataUpdate();
   Inv3PhWorkRun();
   _PvWorkRunControl();

   _InvWorkRunControl();
   Inv3PhCpuTransforClaData();
}

void PcsStatePermit(void)
{
    volatile float32 *cpu2Debug = DevDcr_GetCpu2DebugData();
    if (cpu2Debug != (volatile float32 *)0)
    {
        SetCpuDebugData(61, cpu2Debug[0]);   // ch77
        SetCpuDebugData(62, cpu2Debug[1]);   // ch78
        SetCpuDebugData(63, cpu2Debug[2]);   // ch79
        SetCpuDebugData(64, cpu2Debug[3]);   // ch80

        SetCpuDebugData(65,cpu2Debug[7]);//81
        SetCpuDebugData(66,cpu2Debug[8]);//82

        SetCpuDebugData(67,cpu2Debug[9]);//83
        SetCpuDebugData(68,cpu2Debug[10]);//84
        SetCpuDebugData(69,cpu2Debug[11]);//85

        SetCpuDebugData(86,cpu2Debug[4]);//102
        SetCpuDebugData(87,cpu2Debug[5]);//103
        SetCpuDebugData(88,cpu2Debug[6]);//104

    }    
    StsOutputUpdate();

//    SetCpuDebugData(61,gstControlAction.inv_bus_connect_permit);//77
//    SetCpuDebugData(62,gstControlAction.inv_inside_err_connect_permit);   //78
//    SetCpuDebugData(63,gstControlAction.monitor_inv_connect_permit);//79
//    SetCpuDebugData(64,gstControlAction.parallel_connect_permit);//80   并网预同步标志（后面该部分需要修改）
//    SetCpuDebugData(65,gstControlAction.connect_time_permit);//81
//    SetCpuDebugData(66,gstControlAction.sts_inside_connected_permit);//82
//
//    SetCpuDebugData(67,(GetStsGenForceState() ? gstControlAction.gen_phase_chk_permit : 1) );//83
//    SetCpuDebugData(68,(!GetStsGenForceState() ? gstControlAction.grid_phase_chk_permit : 1));//84
//    SetCpuDebugData(69,(!GetStsGenForceState() ? gstControlAction.grid_drm_permit : 1));//85
    SetCpuDebugData(70,gstControlAction.mointer_connect_type_chk);//86
    SetCpuDebugData(71,gstControlAction.sts_inv_connecting_permit);//87

    SetCpuDebugData(72,gstControlAction.connect_delay_permit);//88
    SetCpuDebugData(73,gstControlAction.sts_run_connected_permit);//89

    SetCpuDebugData(80,Get_StsOutput_IsDisconnectd());//96
    SetCpuDebugData(81,Get_StsOutput_IsDisconnecting());//97
    SetCpuDebugData(82,Get_StsOutput_IsConnecting());//98
    SetCpuDebugData(83,Get_StsOutput_IsConnected());//99
    SetCpuDebugData(84,Get_StsOutput_State());//100
    SetCpuDebugData(85,Get_StsOutput_Warn()->bytes[0]);//101

    SetCpuDebugData(93,  Get_StsOutput_RelagState());//109
    SetCpuDebugData(94,  Get_StsOutput_VolRmsAdjust());//110
    SetCpuDebugData(95,  Get_StsOutput_wAdjust());//111

    _PvWorkPermit_Bus();
    _InvWorkPermit_Bus();

    _InvWorkPermit_StsConnecting();
    _GridAndGenPhaseJudge();
    _WorkPermit_Force_GEnChk();
    _InvWorkPermit_Monitor();
    _PvWorkPermit_Monitor();
    _InvWorkPermit_Parallel();
    _WorkPermit_ElseChk();
    _InvStateMapUpdate();
    //TODO 后需需考虑电网继电器先进行旁路供电，此时INV总继电器合闸需谨慎
    _InvTotalRelagControl();

}

