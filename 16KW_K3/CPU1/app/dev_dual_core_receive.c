/*
 * dev_dual_core_receive.c
 */
#include "dev_dual_core_receive.h"
#include "F2837xD_Examples.h"

static volatile DualDataShare_Ph3Ifc_t *s_ph3Ifc = (volatile DualDataShare_Ph3Ifc_t *)0;
static volatile StsOutputStatus_t *s_stsOutput = (volatile StsOutputStatus_t *)0;
static volatile float32 *s_cpu2DebugData = (volatile float32 *)0;

static volatile void *DevDcr_GetCpu2StsOutputAddress(void)
{
    volatile DevDct_Cpu2ToCpu1Msg_t *msg = DevDcr_GetCpu2ToCpu1Msg();

    return (volatile void *)(msg->sts_output_addr);
}

static volatile void *DevDcr_GetCpu2Ph3IfcAddress(void)
{
    volatile DevDct_Cpu2ToCpu1Msg_t *msg = DevDcr_GetCpu2ToCpu1Msg();

    return (volatile void *)(msg->cpu2_ph3_ifc_addr);
}

static volatile void *DevDcr_GetCpu2DebugDataAddress(void)
{
    volatile DevDct_Cpu2ToCpu1Msg_t *msg = DevDcr_GetCpu2ToCpu1Msg();

    return (volatile void *)(msg->cpu2_debug_data_addr);
}

void DevDcr_ResetPh3Ifc(void)
{
    s_ph3Ifc = (volatile DualDataShare_Ph3Ifc_t *)0;
    s_stsOutput = (volatile StsOutputStatus_t *)0;
    s_cpu2DebugData = (volatile float32 *)0;
}

bool DevDcr_ExchangeAddress(uint32_t timeoutMs)
{
    uint32_t waitMs = 0U;

    while (waitMs < timeoutMs)
    {
        s_ph3Ifc = (volatile DualDataShare_Ph3Ifc_t *)DevDcr_GetCpu2Ph3IfcAddress();
        s_stsOutput = (volatile StsOutputStatus_t *)DevDcr_GetCpu2StsOutputAddress();
        s_cpu2DebugData = (volatile float32 *)DevDcr_GetCpu2DebugDataAddress();
        if (s_ph3Ifc != (volatile DualDataShare_Ph3Ifc_t *)0 && s_stsOutput != (volatile StsOutputStatus_t *)0 && s_cpu2DebugData != (volatile float32 *)0)
        {
            return true;
        }

        DELAY_MS(1U);
        waitMs++;
    }

    return false;
}

volatile DualDataShare_Ph3Ifc_t *DevDcr_GetPh3Ifc(void)
{
    return s_ph3Ifc;
}

volatile StsOutputStatus_t *DevDcr_GetStsOutput(void)
{
    return s_stsOutput;
}

volatile float32 *DevDcr_GetCpu2DebugData(void)
{
    return s_cpu2DebugData;
}

volatile DevDct_Cpu2ToCpu1Msg_t *DevDcr_GetCpu2ToCpu1MsgPtr(void)
{
    return DevDcr_GetCpu2ToCpu1Msg();
}

uint32_t DevDcr_GetCpu2Heartbeat(void)
{
    return DevDcr_GetCpu2ToCpu1Msg()->heartbeat_cnt;
}

uint32_t DevDcr_GetCpu2IsrRunCnt(void)
{
    return DevDcr_GetCpu2ToCpu1Msg()->isr_run_cnt;
}

uint32_t DevDcr_GetCpu2IsrIdleCnt(void)
{
    return DevDcr_GetCpu2ToCpu1Msg()->isr_idle_cnt;
}