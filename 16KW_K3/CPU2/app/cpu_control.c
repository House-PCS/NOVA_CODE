/*
 * cpu_control.c
 *
 *  Created on: Nov 22, 2024
 *      Author: Nova007456
 */
#include "cpu_control.h"
#include "ph3_calc_run.h"
#include "interrupt.h"
#include "dual_data_share.h"
#include "ph3_sts_run.h"
#pragma CODE_SECTION(CpuIntServiceFun, ".TI.ramfunc");

static uint32_t s_interruptRunCnt;
static uint32_t s_interruptQIdleCnt;
static uint32_t s_interruptExitTim;

void CpuIntServiceFun(void)
{
    uint32_t intCont;
    uint32_t intContNext;

    intCont = CpuTimer1Regs.TIM.all;

    if(s_interruptExitTim >= intCont)
    {
        s_interruptQIdleCnt = s_interruptExitTim - intCont;
    }
    else if(s_interruptExitTim < intCont)
    {
        s_interruptQIdleCnt = s_interruptExitTim + (CpuTimer1Regs.PRD.all + 1U - intCont);
    }
    DualDataShare_Ph3IfcRunDone();
    Ph3CalcRun_Run();
    Ph3_Sts_Run();
    intContNext = CpuTimer1Regs.TIM.all;

    if(intCont >= intContNext)
    {
        s_interruptRunCnt = intCont - intContNext;

    }
    else if(intCont < intContNext)
    {
        s_interruptRunCnt = intCont + (CpuTimer1Regs.PRD.all + 1U - intContNext);
    }

    s_interruptExitTim = intContNext;

    DualDataShare_SetIsrRunCnt(s_interruptRunCnt);
    DualDataShare_SetIsrQIdleCnt(s_interruptQIdleCnt);
}

uint32_t GetCpuInterruptRunCont(void)
{
    return s_interruptRunCnt;
}

uint32_t GetCpuInterruptQIdleCnt(void)
{
    return s_interruptQIdleCnt;
}

void CpuIntFunInit(void)
{
    RegisterCpuIntCallback(&CpuIntServiceFun);
    Interrupt_Init();
}

void CpuControl_Ph3CalcInit(void)
{
    Ph3CalcRun_Init();
}

void CpuControl_Ph3StsInit(void)
{
    Ph3_Sts_Init();
}

void CpuControl_Ph3DataUpdate(void)
{
    Ph3CalcRun_DataUpdate();
    Ph3_Sts_DataSlowUpdate();
}

void CpuControl_Ph3FlagUpdate(void)
{
    Ph3CalcRun_FlagUpdate();
    Ph3_Sts_FlagUpdate();
}
