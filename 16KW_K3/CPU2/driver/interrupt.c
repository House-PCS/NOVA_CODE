/*
 * interrupt.c
 *
 * CPU2 interrupt configuration. CLA is intentionally not enabled here.
 */
#include "interrupt.h"
#include "dual_core_cpu2.h"
#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"

static CpuIntCallback pCallbackCpuInt = NULL;

void RegisterCpuIntCallback(CpuIntCallback callback)
{
    pCallbackCpuInt = callback;
}

interrupt void CPU_Interrupt(void)
{
    if (pCallbackCpuInt != NULL)
    {
        pCallbackCpuInt();
    }

    ClearCpuInterrupt();
}

void ClearCpuInterrupt(void)
{
    IPCRtoLFlagAcknowledge(IPC_DUAL_FLAG_CPU1_EVENT);
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

void Interrupt_Init(void)
{
    EALLOW;
    PieVectTable.IPC0_INT = &CPU_Interrupt;
    PieCtrlRegs.PIEIER1.bit.INTx13 = 1U;
    IER |= M_INT1;
    EDIS;

    if (IPCRtoLFlagBusy(IPC_DUAL_FLAG_CPU1_EVENT) != 0U)
    {
        IPCRtoLFlagAcknowledge(IPC_DUAL_FLAG_CPU1_EVENT);
    }
}
