/*
 * dual_core.c
 *
 * CPU1: CPUSEL/GS ownership, CPU2 boot (no InitIpc), IPC handshake.
 */
#include "dual_core.h"
#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"
#include "F2837xD_Examples.h"

static bool s_cpu2PowerOnOnline = false;  /* CPU2开机在线标志 */
static bool s_cpu2RunOnline = false;    /* CPU2运行在线标志 */
static uint32_t s_cpu2RunCheckLastValue = 0;
static uint8_t s_cpu2RunUnchangedCnt = 0U;

static bool DualCore_WaitCondition(bool (*cond)(void), uint32_t timeoutMs)
{
    uint32_t elapsedMs = 0U;

    while (cond() == false)
    {
        DELAY_MS(1U);
        elapsedMs++;
        if (elapsedMs >= timeoutMs)
        {
            return false;
        }
    }

    return true;
}

static bool DualCore_IsBootSystemReady(void)
{
    return ((IPCGetBootStatus() & 0x0000000FU) == C2_BOOTROM_BOOTSTS_SYSTEM_READY);
}

static bool DualCore_AreBootIpcFlagsFree(void)
{
    return (IPCLtoRFlagBusy(IPC_FLAG0) == 0U) &&
           (IPCLtoRFlagBusy(IPC_FLAG31) == 0U);
}

void DualCore_ConfigCpuSel(void)
{
    EALLOW;

    DevCfgRegs.CPUSEL0.all  = 0U;
    DevCfgRegs.CPUSEL1.all  = 0U;
    DevCfgRegs.CPUSEL2.all  = 0U;
    DevCfgRegs.CPUSEL4.all  = 0U;
    DevCfgRegs.CPUSEL5.all  = 0U;
    DevCfgRegs.CPUSEL6.all  = 0U;
    DevCfgRegs.CPUSEL7.all  = 0U;
    DevCfgRegs.CPUSEL8.all  = 0U;
    DevCfgRegs.CPUSEL9.all  = 0U;
    DevCfgRegs.CPUSEL11.all = 0U;
    DevCfgRegs.CPUSEL12.all = 0U;
    DevCfgRegs.CPUSEL14.all = 0U;

    EDIS;
}

void DualCore_ConfigGsRamMaster(void)
{
    EALLOW;

    MemCfgRegs.GSxMSEL.all = (GS0_ACCESS | GS1_ACCESS | GS2_ACCESS |
                               GS4_ACCESS);

    MemCfgRegs.GSxMSEL.bit.MSEL_GS5  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS6  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS7  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS8  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS9  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS10 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS11 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS12 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS13 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS14 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS15 = 0;

    EDIS;
}

static void DualCore_ConfigGsRamAccessProtect(void)
{
    EALLOW;

    MemCfgRegs.GSxACCPROT0.bit.CPUWRPROT_GS3 = 0;
    MemCfgRegs.GSxACCPROT0.bit.DMAWRPROT_GS3 = 1;
    MemCfgRegs.GSxACCPROT1.bit.CPUWRPROT_GS4 = 0;
    MemCfgRegs.GSxACCPROT1.bit.DMAWRPROT_GS4 = 1;

    EDIS;
}

void DualCore_InitEarly(void)
{
    DualCore_ConfigCpuSel();
    DualCore_ConfigGsRamMaster();
    DualCore_ConfigGsRamAccessProtect();
    s_cpu2PowerOnOnline = false;  /* CPU2开机在线标志 */
    s_cpu2RunOnline = true;    /* CPU2运行在线标志 */
}

DualCore_Status_t DualCore_BootCpu2WithTimeout(uint32_t timeoutMs)
{
    uint32_t bootStatus;

    bootStatus = IPCGetBootStatus() & 0x0000000FU;
    if (bootStatus == C2_BOOTROM_BOOTSTS_C2TOC1_BOOT_CMD_ACK)
    {
        return DUAL_CORE_ERR_ALREADY_BOOTED;
    }

    if (DualCore_WaitCondition(DualCore_IsBootSystemReady, timeoutMs) == false)
    {
        return DUAL_CORE_ERR_SYSTEM_NOT_READY;
    }

    if (DualCore_WaitCondition(DualCore_AreBootIpcFlagsFree, timeoutMs) == false)
    {
        return DUAL_CORE_ERR_IPC_BUSY;
    }

    if (IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH) != STATUS_PASS)
    {
        return DUAL_CORE_ERR_INVALID_MODE;
    }

    return DUAL_CORE_OK;
}

DualCore_Status_t DualCore_HandshakeCpu1(uint32_t timeoutMs)
{
    uint32_t elapsedMs = 0U;

    IPCLtoRFlagSet(IPC_DUAL_FLAG_CPU1_READY);

    while (IPCLtoRFlagBusy(IPC_DUAL_FLAG_CPU1_READY) != 0U)
    {
        DELAY_MS(1U);
        elapsedMs++;
        if (elapsedMs >= timeoutMs)
        {
            return DUAL_CORE_ERR_CPU2_NOT_ALIVE;
        }
    }

    elapsedMs = 0U;
    while (IPCRtoLFlagBusy(IPC_DUAL_FLAG_CPU2_READY) == 0U)
    {
        DELAY_MS(1U);
        elapsedMs++;
        if (elapsedMs >= timeoutMs)
        {
            return DUAL_CORE_ERR_CPU2_NOT_ALIVE;
        }
    }

    IPCRtoLFlagAcknowledge(IPC_DUAL_FLAG_CPU2_READY);
    s_cpu2PowerOnOnline = true;

    return DUAL_CORE_OK;
}

bool DualCore_IsCpu2PowerOnOnline(void)
{
    return s_cpu2PowerOnOnline;
}

void DualCore_SetCpu2PowerOnOnline(bool online)
{
    s_cpu2PowerOnOnline = online;
}

bool DualCore_IsCpu2RunOnline(void)
{
    return s_cpu2RunOnline;
}

void DualCore_CheckCpu2RunOnline(uint32_t value)
{
    if (value != s_cpu2RunCheckLastValue)
    {
        s_cpu2RunOnline = true;
        s_cpu2RunUnchangedCnt = 0U;
    }
    else
    {
        s_cpu2RunUnchangedCnt++;
        if (s_cpu2RunUnchangedCnt >= 2U)
        {
            s_cpu2RunOnline = false;
        }
    }

    s_cpu2RunCheckLastValue = value;
}

bool DualCore_TriggerCpu2Event(void)
{
    if (s_cpu2PowerOnOnline == false)
    {
        return false;
    }

    if (IPCLtoRFlagBusy(IPC_DUAL_FLAG_CPU1_EVENT) != 0U)
    {
        return false;
    }

    IPCLtoRFlagSet(IPC_DUAL_FLAG_CPU1_EVENT);
    return true;
}
