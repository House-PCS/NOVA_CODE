/*
 * dual_core_cpu2.h
 *
 * CPU2 startup handshake (TI dma_ex1 / ram_ex1 style).
 */
#ifndef DRIVER_DUAL_CORE_CPU2_H_
#define DRIVER_DUAL_CORE_CPU2_H_

#include <stdint.h>

#define IPC_DUAL_FLAG_CPU1_READY    0x00000010U
#define IPC_DUAL_FLAG_CPU2_READY    0x00000020U
#define IPC_DUAL_FLAG_CPU1_EVENT    0x00000001U
#define IPC_DUAL_GS_CPU2_MASK       0x00000017U
#define IPC_DUAL_BOOT_TIMEOUT_MS         1000U
#define IPC_DUAL_HANDSHAKE_TIMEOUT_MS    3000U

typedef enum
{
    DUAL_CORE_OK = 0,
    DUAL_CORE_ERR_CPU1_NOT_READY
} DualCore_Cpu2Status_t;

void DualCore_WaitGsRamMaster(void);
void DualCore_ConfigGsRamAccessProtect(void);
DualCore_Cpu2Status_t DualCore_HandshakeCpu2(uint32_t timeoutMs);

#endif /* DRIVER_DUAL_CORE_CPU2_H_ */
