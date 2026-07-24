/*
 * dev_dual_core_receive.c
 */
#include "dev_dual_core_receive.h"

volatile DevDct_Cpu2ToCpu1Msg_t *DevDcr_GetCpu2ToCpu1MsgPtr(void)
{
    return DevDcr_GetCpu2ToCpu1Msg();
}

uint32_t DevDcr_GetCpu2Heartbeat(void)
{
    return DevDcr_GetCpu2ToCpu1Msg()->heartbeat_cnt;
}
