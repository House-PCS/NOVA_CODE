/*
 * sys_ctrl_cpu2.c
 *
 * CPU2 minimal system init, replaces full InitSysCtrl() for dual-core app.
 */
#include "sys_ctrl_cpu2.h"
#include "F28x_Project.h"
#include "F2837xD_Examples.h"
#include <string.h>

extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadSize;
extern Uint16 RamfuncsRunStart;

static void SysCtrlCpu2_InitPeripheralClocks(void)
{
    EALLOW;

    CpuSysRegs.PCLKCR0.bit.CPUTIMER0 = 1;
    CpuSysRegs.PCLKCR0.bit.CPUTIMER1 = 1;
    CpuSysRegs.PCLKCR0.bit.CPUTIMER2 = 1;

    EDIS;
}

void SysCtrlCpu2_Init(void)
{
    DisableDog();

#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
    InitFlash();
#endif

    SysCtrlCpu2_InitPeripheralClocks();
}
