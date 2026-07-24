/*
 * dual_data_share.c
 *
 * CPU1 侧共享 RAM 访问（心跳 + CPU2 升级 GS3/GS4）。
 */
#include "dual_data_share.h"
#include <string.h>

volatile DevDct_Cpu1ToCpu2Msg_t *DualDataShare_GetCpu1ToCpu2Msg(void)
{
    return (volatile DevDct_Cpu1ToCpu2Msg_t *)DUAL_DATA_SHARE_CPU1_TO_CPU2_ADDR;
}

volatile DevDct_Cpu2ToCpu1Msg_t *DualDataShare_GetCpu2ToCpu1Msg(void)
{
    return (volatile DevDct_Cpu2ToCpu1Msg_t *)DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR;
}

volatile Cpu2UpgradeCtrl_t *DualDataShare_GetCpu2UpgradeCtrl(void)
{
    return (volatile Cpu2UpgradeCtrl_t *)CPU2_UPG_GS3_ADDR;
}

volatile Cpu2UpgradeData_t *DualDataShare_GetCpu2UpgradeData(void)
{
    return (volatile Cpu2UpgradeData_t *)CPU2_UPG_GS4_ADDR;
}

void DualDataShare_ResetCpu2Upgrade(void)
{
    volatile Cpu2UpgradeCtrl_t *ctrl = DualDataShare_GetCpu2UpgradeCtrl();

    memset((void *)ctrl, 0, sizeof(Cpu2UpgradeCtrl_t));
    ctrl->ack = CPU2_UPG_ACK_INVALID;
}

void DualDataShare_Init(void)
{
    volatile DevDct_Cpu1ToCpu2Msg_t *msg = DualDataShare_GetCpu1ToCpu2Msg();

    msg->reserved = 0U;
    DualDataShare_ResetCpu2Upgrade();
}
