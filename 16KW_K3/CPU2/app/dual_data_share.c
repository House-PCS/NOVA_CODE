/*
 * dual_data_share.c
 *
 * CPU1/CPU2 shared data accessors (CPU2 side).
 */
#include "dual_data_share.h"
#include "ph3_calc_ifc_update.h"
#include "ph3_sts_run.h"
#include "debug_analysis.h"

#pragma CODE_SECTION(DualDataShare_Ph3IfcRunDone, ".TI.ramfunc");
#pragma CODE_SECTION(DualDataShare_SetIsrRunCnt, ".TI.ramfunc");

void DualDataShare_Init(void)
{
    volatile DevDct_Cpu2ToCpu1Msg_t *msg = DualDataShare_GetCpu2ToCpu1Msg();

    msg->cpu2_ph3_ifc_addr = (uintptr_t)Ph3CalcIfcUpdate_Get();
    msg->isr_run_cnt = 0U;
    msg->heartbeat_cnt = 0U;
    msg->isr_idle_cnt = 0U;
    msg->sts_output_addr = (uintptr_t)StsOutputUpdate_Get();
    msg->cpu2_debug_data_addr = (uintptr_t)GetCpuDebugDataPtr();
}

void DualDataShare_Ph3IfcRunDone(void)
{
    volatile DevDct_Cpu2ToCpu1Msg_t *msg = DualDataShare_GetCpu2ToCpu1Msg();

    if(msg->heartbeat_cnt >= 0xFFFFFFFFU)
    {
        msg->heartbeat_cnt = 0U;
    }
    else
    {
        msg->heartbeat_cnt++;
    }
}

void DualDataShare_SetIsrRunCnt(uint32_t cnt)
{
    DualDataShare_GetCpu2ToCpu1Msg()->isr_run_cnt = cnt;
}

void DualDataShare_SetIsrQIdleCnt(uint32_t cnt)
{
    DualDataShare_GetCpu2ToCpu1Msg()->isr_idle_cnt = cnt;
}