/*
 * dual_data_share.h
 *
 * CPU1/CPU2 shared data layout (ADC result + ph3 run flags).
 */
#ifndef APP_DUAL_DATA_SHARE_H_
#define APP_DUAL_DATA_SHARE_H_

#include "F28x_Project.h"
#include <stdint.h>

/** CPU2 -> CPU1 message at CPU2TOCPU1RAM (0x3F800) */
typedef struct
{
    volatile uintptr_t cpu2_ph3_ifc_addr;     /* DualDataShare_Ph3Ifc_t address */
    volatile uint32_t isr_run_cnt;            /* CPU2 IPC ISR runtime in CpuTimer1 ticks */
    volatile uint32_t heartbeat_cnt;          /* CPU2 ph3 run heartbeat for CPU1 */
    volatile uint32_t isr_idle_cnt;           /* CPU2 IPC ISR idle time in CpuTimer1 ticks */
    volatile uintptr_t sts_output_addr;       /* StsOutputStatus_t address */
    volatile uintptr_t cpu2_debug_data_addr;  /* CPU2 debug data address */
} DevDct_Cpu2ToCpu1Msg_t;

#define DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR   0x0003F800UL

/** CPU2->CPU1 message mapped to CPU2TOCPU1RAM, no linker section required */
#define DualDataShare_GetCpu2ToCpu1Msg() \
    ((volatile DevDct_Cpu2ToCpu1Msg_t *)(DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR))

#define gstDevDctCpu2ToCpu1Msg  (*DualDataShare_GetCpu2ToCpu1Msg())

void DualDataShare_Init(void);
void DualDataShare_Ph3IfcRunDone(void);
void DualDataShare_SetIsrRunCnt(uint32_t cnt);
void DualDataShare_SetIsrQIdleCnt(uint32_t cnt);
#endif /* APP_DUAL_DATA_SHARE_H_ */
