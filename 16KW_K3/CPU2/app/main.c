/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        main.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief       CPU2 main (minimal init + IPC handshake with CPU1)
  ******************************************************************************
  */

#include "F28x_Project.h"
#include "F2837xD_Examples.h"
#include "dual_core_cpu2.h"
#include "sys_ctrl_cpu2.h"
#include "cpu_timers.h"
#include "cpu_control.h"
#include "dev_dual_core_receive.h"
#include "dual_data_share.h"
#include "ph3_calc_ifc_update.h"
#include "mid_vol_and_rcmu_rms.h"
#include "debug_analysis.h"

void main(void)
{
    DualCore_Cpu2Status_t handshakeStatus;

    SysCtrlCpu2_Init();
    DELAY_MS(500);
    DINT;

    IER = 0x0000;
    IFR = 0x0000;

    InitPieCtrl();
    InitPieVectTable();

    handshakeStatus = DualCore_HandshakeCpu2(IPC_DUAL_HANDSHAKE_TIMEOUT_MS);
    if (handshakeStatus != DUAL_CORE_OK)
    {
        for (;;)
        {
        }
    }

    DualCore_ConfigGsRamAccessProtect();
    DualDataShare_Init();
    Ph3CalcIfcUpdate_Init();
    (void)DevDcr_ExchangeAddress(DEV_DCR_ADDR_EXCHANGE_TIMEOUT_MS);
    CpuControl_Ph3CalcInit();
    MidVolAndRcmuRms_Init();
    CpuTimer_Init();
    CpuIntFunInit();
    CpuControl_Ph3StsInit();
    
    EINT;
    ERTM;

    for (;;)
    {
        CpuTimerFlag();
        CpuControl_Ph3DataUpdate();
        if (Get_1ms_Flag() == 1)
        {
//            SetCpuDebugData(0,0);
//            SetCpuDebugData(1,1);
//            SetCpuDebugData(2,2);
//            SetCpuDebugData(3,3);
//            SetCpuDebugData(4,4);
//            SetCpuDebugData(5,5);
//            SetCpuDebugData(6,6);
//            SetCpuDebugData(7,7);
//            SetCpuDebugData(8,8);
//            SetCpuDebugData(9,9);
//            SetCpuDebugData(10,10);
//            SetCpuDebugData(11,11);
//            SetCpuDebugData(12,12);
//            SetCpuDebugData(13,13);
            CpuControl_Ph3FlagUpdate();
            Timers_1ms();
        }
        if (Get_5ms_Flag() == 1)
        {
            Timers_5ms();
        }
        if (Get_10ms_Flag() == 1)
        {
            Timers_10ms();
        }
        if (Get_20ms_Flag() == 1)
        {
            Timers_20ms();
        }
        if (Get_100ms_Flag() == 1)
        {
            Timers_100ms();
        }
        if (Get_500ms_Flag() == 1)
        {
            Timers_500ms();
        }
        if (Get_1s_Flag() == 1)
        {
            Timers_1s();
        }
    }
}
