/*
 * dual_data_share.h
 *
 * CPU1/CPU2 共享数据：心跳 + CPU2 升级 GS3/GS4。
 */
#ifndef APP_DUAL_DATA_SHARE_H_
#define APP_DUAL_DATA_SHARE_H_

#include "F28x_Project.h"
#include "cpu2_upgrade_share.h"
#include <stdint.h>

/** CPU1 -> CPU2 message at CPU1TOCPU2RAM */
typedef struct
{
    volatile uint32_t reserved;
} DevDct_Cpu1ToCpu2Msg_t;

/** CPU2 -> CPU1 message at CPU2TOCPU1RAM (0x3F800) */
typedef struct
{
    volatile uint32_t heartbeat_cnt;
} DevDct_Cpu2ToCpu1Msg_t;

#define DUAL_DATA_SHARE_CPU1_TO_CPU2_ADDR   0x0003FC00UL
#define DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR   0x0003F800UL

volatile DevDct_Cpu1ToCpu2Msg_t *DualDataShare_GetCpu1ToCpu2Msg(void);
volatile DevDct_Cpu2ToCpu1Msg_t *DualDataShare_GetCpu2ToCpu1Msg(void);

volatile Cpu2UpgradeCtrl_t *DualDataShare_GetCpu2UpgradeCtrl(void);
volatile Cpu2UpgradeData_t *DualDataShare_GetCpu2UpgradeData(void);

void DualDataShare_Init(void);
void DualDataShare_ResetCpu2Upgrade(void);

#endif /* APP_DUAL_DATA_SHARE_H_ */
