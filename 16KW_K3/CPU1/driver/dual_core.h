/*
 * dual_core.h
 *
 * CPU1 dual-core resource ownership and CPU2 boot/handshake (TI dma_ex1 / ram_ex1 style).
 */
#ifndef DRIVER_DUAL_CORE_H_
#define DRIVER_DUAL_CORE_H_

#include <stdbool.h>
#include <stdint.h>

/* IPC flags for startup barrier (FLAG4/5 avoid PIE IPC0~3) */
#define IPC_DUAL_FLAG_CPU1_READY    0x00000010U  /* IPC_FLAG4: CPU1 peripherals ready */
#define IPC_DUAL_FLAG_CPU2_READY    0x00000020U  /* IPC_FLAG5: CPU2 init ready */
#define IPC_DUAL_FLAG_CPU1_EVENT    0x00000001U  /* IPC_FLAG0: CPU1 event interrupt to CPU2 */
#define IPC_DUAL_GS_CPU2_MASK       0x00000017U  /* GS0|GS1|GS2|GS4 master -> CPU2 */
#define IPC_DUAL_BOOT_TIMEOUT_MS         1000U
#define IPC_DUAL_HANDSHAKE_TIMEOUT_MS    3000U

typedef enum
{
    DUAL_CORE_OK = 0,
    DUAL_CORE_ERR_ALREADY_BOOTED,
    DUAL_CORE_ERR_INVALID_MODE,
    DUAL_CORE_ERR_SYSTEM_NOT_READY,
    DUAL_CORE_ERR_IPC_BUSY,
    DUAL_CORE_ERR_CPU2_NOT_ALIVE
} DualCore_Status_t;

void DualCore_ConfigCpuSel(void);
void DualCore_ConfigGsRamMaster(void);
void DualCore_InitEarly(void);

DualCore_Status_t DualCore_BootCpu2WithTimeout(uint32_t timeoutMs);
DualCore_Status_t DualCore_HandshakeCpu1(uint32_t timeoutMs);

bool DualCore_IsCpu2PowerOnOnline(void);
void DualCore_SetCpu2PowerOnOnline(bool online);

bool DualCore_IsCpu2RunOnline(void);
void DualCore_CheckCpu2RunOnline(uint32_t value);

bool DualCore_TriggerCpu2Event(void);

#endif /* DRIVER_DUAL_CORE_H_ */
