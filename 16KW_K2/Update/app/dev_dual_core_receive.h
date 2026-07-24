/*
 * dev_dual_core_receive.h
 *
 * CPU1 receive CPU2 shared RAM data (heartbeat only).
 */
#ifndef APP_DEV_DUAL_CORE_RECEIVE_H_
#define APP_DEV_DUAL_CORE_RECEIVE_H_

#include <stdint.h>
#include "dual_data_share.h"

#define DualDataShare_GetCpu2ToCpu1Msg() \
    ((volatile DevDct_Cpu2ToCpu1Msg_t *)(DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR))

#define gstDevDctCpu2ToCpu1Msg  (*DualDataShare_GetCpu2ToCpu1Msg())

#define DEV_DCR_CPU2_TO_CPU1_ADDR         DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR
#define DEV_DCR_CPU1_TO_CPU2_ADDR         DUAL_DATA_SHARE_CPU1_TO_CPU2_ADDR

#define DevDcr_GetCpu1ToCpu2Msg()         DualDataShare_GetCpu1ToCpu2Msg()
#define DevDcr_GetCpu2ToCpu1Msg()         DualDataShare_GetCpu2ToCpu1Msg()

volatile DevDct_Cpu2ToCpu1Msg_t *DevDcr_GetCpu2ToCpu1MsgPtr(void);
uint32_t DevDcr_GetCpu2Heartbeat(void);

#endif /* APP_DEV_DUAL_CORE_RECEIVE_H_ */
