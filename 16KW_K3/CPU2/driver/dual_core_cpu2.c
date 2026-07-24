/*
 * dual_core_cpu2.c
 *
 * CPU2: wait for CPU1 IPC barrier, GS ownership, signal ready.
 */
#include "dual_core_cpu2.h"
#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"
#include "F2837xD_Examples.h"

void DualCore_WaitGsRamMaster(void)
{
    while ((MemCfgRegs.GSxMSEL.all & IPC_DUAL_GS_CPU2_MASK) != IPC_DUAL_GS_CPU2_MASK)
    {
    }
}

void DualCore_ConfigGsRamAccessProtect(void)
{
    EALLOW;

    MemCfgRegs.GSxACCPROT0.bit.CPUWRPROT_GS3 = 0;
    MemCfgRegs.GSxACCPROT0.bit.DMAWRPROT_GS3 = 1;
    MemCfgRegs.GSxACCPROT1.bit.CPUWRPROT_GS4 = 0;
    MemCfgRegs.GSxACCPROT1.bit.DMAWRPROT_GS4 = 1;

    EDIS;
}

DualCore_Cpu2Status_t DualCore_HandshakeCpu2(uint32_t timeoutMs)
{
    uint32_t elapsedMs = 0U;

    while (IPCRtoLFlagBusy(IPC_DUAL_FLAG_CPU1_READY) == 0U)
    {
        DELAY_MS(1U);
        elapsedMs++;
        if (elapsedMs >= timeoutMs)
        {
            return DUAL_CORE_ERR_CPU1_NOT_READY;
        }
    }

    IPCRtoLFlagAcknowledge(IPC_DUAL_FLAG_CPU1_READY);
    DualCore_WaitGsRamMaster();
    IPCLtoRFlagSet(IPC_DUAL_FLAG_CPU2_READY);

    return DUAL_CORE_OK;
}
