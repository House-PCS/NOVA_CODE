# 附录 C：自定义驱动、中间件与链接脚本源码

> 本附录收录 driver/（业务自定义驱动，不含 TI source//include/ 库）、middleware/ 及 cmd/ 链接脚本。


---

## driver/sys_ctrl_cpu2.h

```c
/*
 * sys_ctrl_cpu2.h
 *
 * CPU2 minimal system init (no PLL, peripheral clocks on demand).
 */
#ifndef DRIVER_SYS_CTRL_CPU2_H_
#define DRIVER_SYS_CTRL_CPU2_H_

void SysCtrlCpu2_Init(void);

#endif /* DRIVER_SYS_CTRL_CPU2_H_ */

```

---

## driver/sys_ctrl_cpu2.c

```c
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

```

---

## driver/dual_core_cpu2.h

```c
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

```

---

## driver/dual_core_cpu2.c

```c
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

```

---

## driver/interrupt.h

```c
/*
 * interrupt.h
 *
 * CPU2 interrupt configuration interface.
 */
#ifndef DEVICE_INTERRUPT_H_
#define DEVICE_INTERRUPT_H_

typedef void (*CpuIntCallback)(void);

void RegisterCpuIntCallback(CpuIntCallback callback);
void ClearCpuInterrupt(void);
void Interrupt_Init(void);

#endif /* DEVICE_INTERRUPT_H_ */

```

---

## driver/interrupt.c

```c
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

```

---

## driver/cpu_timers.h

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cpu_timers.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __CPU_TIMERS_H__
#define __CPU_TIMERS_H__
#include "F28x_Project.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EN_CLA_RUN_TIME_MEASURE (0)

/**
 * @brief ��ʱ��״̬�ṹ��
 */
typedef struct timer_state_stru_t
{
    uint32_t Time;                  //����
    uint16_t Timer_1ms_Count;       //��ʱ��1ms
    uint16_t Timer_5ms_Count;       //��ʱ��5ms
    uint16_t Timer_10ms_Count;      //��ʱ��10ms
    uint16_t Timer_20ms_Count;      //��ʱ��20ms
    uint16_t Timer_100ms_Count;     //��ʱ��100ms
    uint16_t Timer_500ms_Count;
    uint16_t Timer_1000ms_Count;    //��ʱ��1000ms

    uint16_t Timer_1ms_Flag;        //��ʱ��1ms��־λ ��1��Ч
    uint16_t Timer_5ms_Flag;        //��ʱ��5ms��־λ ��1��Ч
    uint16_t Timer_10ms_Flag;       //��ʱ��10ms��־λ ��1��Ч
    uint16_t Timer_20ms_Flag;       //��ʱ��20ms��־λ ��1��Ч
    uint16_t Timer_100ms_Flag;      //��ʱ��100ms��־λ ��1��Ч
    uint16_t Timer_500ms_Flag;      //��ʱ��100ms��־λ ��1��Ч
    uint16_t Timer_1000ms_Flag;     //��ʱ��1000ms��־λ ��1��Ч
} TimerState_t;

void CpuTimer_Init(void);
uint32_t TimGetSystime(void);
uint32_t TimDifftimeFromNow(uint32_t time);
void CpuTimerFlag(void);

uint16_t Get_1ms_Flag(void);
uint16_t Get_5ms_Flag(void);
uint16_t Get_10ms_Flag(void);
uint16_t Get_20ms_Flag(void);
uint16_t Get_100ms_Flag(void);
uint16_t Get_500ms_Flag(void);
uint16_t Get_1s_Flag(void);

void Timers_1ms(void);
void Timers_5ms(void);
void Timers_10ms(void);
void Timers_20ms(void);
void Timers_100ms(void);
void Timers_500ms(void);
void Timers_1s(void);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================

```

---

## driver/cpu_timers.c

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cpu_timers.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


//
// Included Files
//
#include "cpu_timers.h"
#include "F28x_Project.h"
//
// Function Prototypes
//
__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void cpu_timer2_isr(void);

static TimerState_t _gTimer;
//
// cpu_timer0_isr - CPU Timer0 ISR with interrupt counter
//
__interrupt void cpu_timer0_isr(void)
{
    CpuTimer0.InterruptCount++;

    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

//
// cpu_timer1_isr - CPU Timer1 ISR
//
#ifdef EN_CLA_RUN_TIME_MEASURE

#else
__interrupt void cpu_timer1_isr(void)
{
    CpuTimer1.InterruptCount++;
}
#endif

//
// cpu_timer2_isr CPU Timer2 ISR
//
__interrupt void cpu_timer2_isr(void)
{
    CpuTimer2.InterruptCount++;
//    ServiceDog();
}

uint32_t TimGetSystime(void)
{
    return CpuTimer2.InterruptCount;
}

/**
  * @brief  Calculate the difference of two time values.
  * This function will handle overflow problem.
  * @param  time[IN]: First time value to calculate
  * @param  new_time[IN]: 2nd time value to calculate
  * @return Time difference of 2 time values.
  */
uint32_t TimDifftime(uint32_t time, uint32_t new_time)
{
    uint32_t diffTime = 0;
    if (new_time >= time) /* not overflow */
    {
        diffTime = (new_time - time);
    }
    else
    {
        diffTime = new_time + (0xffffffffUL - time);
    }
    return (uint32_t)(diffTime);
}

/**
  * @brief  Calculate the time difference from now.
  * @param  time[IN]: Previous recorded time value
  * @return Time difference.
  */
uint32_t TimDifftimeFromNow(uint32_t time)
{
    return TimDifftime(time, CpuTimer2.InterruptCount);
}

void CpuTimerFlag(void)
{
    if(TimDifftimeFromNow(_gTimer.Time) >= 1) //1ms��ʱ��
    {
        _gTimer.Timer_1ms_Count++;
        _gTimer.Timer_5ms_Count++;
        _gTimer.Timer_10ms_Count++;
        _gTimer.Timer_20ms_Count++;
        _gTimer.Timer_100ms_Count++;
        _gTimer.Timer_500ms_Count++;
        _gTimer.Timer_1000ms_Count++;

        if(1)
        {
            _gTimer.Timer_1ms_Flag = 1;
            _gTimer.Timer_1ms_Count = 0;
        }
        if(_gTimer.Timer_5ms_Count >= 5)
        {
            _gTimer.Timer_5ms_Flag = 1;
            _gTimer.Timer_5ms_Count = 0;
        }
        if(_gTimer.Timer_10ms_Count >= 10)
        {
            _gTimer.Timer_10ms_Flag = 1;
            _gTimer.Timer_10ms_Count = 0;
        }
        if(_gTimer.Timer_20ms_Count >= 20)
        {
            _gTimer.Timer_20ms_Flag = 1;
            _gTimer.Timer_20ms_Count = 0;
        }
        if(_gTimer.Timer_100ms_Count >= 100)
        {
            _gTimer.Timer_100ms_Flag = 1;
            _gTimer.Timer_100ms_Count = 0;
        }
        if(_gTimer.Timer_500ms_Count >= 500)
        {
            _gTimer.Timer_500ms_Flag = 1;
            _gTimer.Timer_500ms_Count = 0;
        }
        if(_gTimer.Timer_1000ms_Count >= 1000)
        {
            _gTimer.Timer_1000ms_Flag = 1;
            _gTimer.Timer_1000ms_Count = 0;
        }

        _gTimer.Time = TimGetSystime();
    }
}
uint16_t Get_1ms_Flag(void)
{
    return (_gTimer.Timer_1ms_Flag);
}
uint16_t Get_5ms_Flag(void)
{
    return (_gTimer.Timer_5ms_Flag);
}
uint16_t Get_10ms_Flag(void)
{
    return (_gTimer.Timer_10ms_Flag);
}
uint16_t Get_20ms_Flag(void)
{
    return (_gTimer.Timer_20ms_Flag);
}
uint16_t Get_100ms_Flag(void)
{
    return (_gTimer.Timer_100ms_Flag);
}
uint16_t Get_500ms_Flag(void)
{
    return (_gTimer.Timer_500ms_Flag);
}
uint16_t Get_1s_Flag(void)
{
    return (_gTimer.Timer_1000ms_Flag);
}
void Timers_1ms(void)
{
    _gTimer.Timer_1ms_Flag = 0;
}
void Timers_5ms(void)
{
    _gTimer.Timer_5ms_Flag = 0;
}
void Timers_10ms(void)
{
    _gTimer.Timer_10ms_Flag = 0;
}
void Timers_20ms(void)
{
    _gTimer.Timer_20ms_Flag = 0;
}
void Timers_100ms(void)
{
    _gTimer.Timer_100ms_Flag = 0;
}
void Timers_500ms(void)
{
    _gTimer.Timer_500ms_Flag = 0;
}

void Timers_1s(void)
{
    _gTimer.Timer_1000ms_Flag = 0;
}

void CpuTimer_Init(void)
{
//
// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
//
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
#ifdef EN_CLA_RUN_TIME_MEASURE
#else
    PieVectTable.TIMER1_INT = &cpu_timer1_isr;
#endif

    PieVectTable.TIMER2_INT = &cpu_timer2_isr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

//
// Step 4. Initialize the Device Peripheral. This function can be
//         found in F2837xD_CpuTimers.c
//
    InitCpuTimers();   // For this example, only initialize the Cpu Timers

//
// Configure CPU-Timer 0, 1, and 2 to interrupt every second:
// 200MHz CPU Freq, 1 second Period (in uSeconds)
//
    ConfigCpuTimer(&CpuTimer0, 200, 1000000);
#ifdef EN_CLA_RUN_TIME_MEASURE
    ConfigCpuTimer(&CpuTimer1, 200, 0xFFFFFFFF);
#else
    ConfigCpuTimer(&CpuTimer1, 200, 1000000);
#endif

    ConfigCpuTimer(&CpuTimer2, 200, 1000);

//
// To ensure precise timing, use write-only instructions to write to the
// entire register. Therefore, if any of the configuration bits are changed in
// ConfigCpuTimer and InitCpuTimers (in F2837xD_cputimervars.h), the below
// settings must also be updated.
//
    CpuTimer0Regs.TCR.all = 0x4000;
#ifdef EN_CLA_RUN_TIME_MEASURE
    CpuTimer1Regs.TCR.all = 0x0000;
#else
    CpuTimer1Regs.TCR.all = 0x4000;
#endif

    CpuTimer2Regs.TCR.all = 0x4000;

//
// Step 5. User specific code, enable interrupts:
//

//
// Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
// which is connected to CPU-Timer 1, and CPU int 14, which is connected
// to CPU-Timer 2:
//
    IER |= M_INT1;
    IER |= M_INT13;
    IER |= M_INT14;

//
// Enable TINT0 in the PIE: Group 1 interrupt 7
//
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
}

```

---

## driver/system.h

```c
/*
 * system.h
 *
 *  Created on: 2025Äê1ÔÂ6ÈÕ
 *      Author: Nova007456
 */
#ifndef SYSTEM_H_
#define SYSTEM_H_

#define CPU_SYS_CLOCK       (200*1000000)
#define LSPCLK_FREQ         (CPU_SYS_CLOCK/4)
#define PWMSYSCLOCK_FREQ    (100*1000000)

void SysRestart(void);

#endif /* SYSTEM_H_ */

```

---

## driver/system.c

```c
/*
 * system.c
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */

#include "system.h"
#include "F28x_Project.h"

/**
* @brief: SysRestart
* @param: void
* @note: ϵͳ��λ����
*/
void SysRestart(void)
{
    EALLOW;
    WdRegs.WDCR.all = 0x00af;
    WdRegs.WDCR.bit.WDCHK = 2;
    EDIS;
}

```

---

## middleware/mid_filter.h

```c
/**
 * @file mid_filter.h
 * @brief First/second-order Butterworth low-pass filters
 */

#ifndef MID_FILTER_H
#define MID_FILTER_H

#include "F28x_Project.h"

typedef struct mid_filter_1order_butter_stru_t
{
    float32 a0;
    float32 a1;
    float32 b1;
    float32 yn;
    float32 yn_last1;
    float32 xn_last1;
} Mid_Filter1OrderButter_t;

typedef struct mid_filter_2order_butter_stru_t
{
    float32 nb0;
    float32 nb1;
    float32 na1;
    float32 na2;
    float32 xn_last1;
    float32 xn_last2;
    float32 yn;
    float32 yn_last1;
    float32 yn_last2;
} Mid_Filter2OrderButter_t;

void Mid_Filter1OrderButter_Init(Mid_Filter1OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz);
void Mid_Filter1OrderButter_Reset(Mid_Filter1OrderButter_t* filter);
float32 Mid_Filter1OrderButter_Update(Mid_Filter1OrderButter_t* filter, float32 input);

void Mid_Filter2OrderButter_Init(Mid_Filter2OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz);
void Mid_Filter2OrderButter_Reset(Mid_Filter2OrderButter_t* filter);
float32 Mid_Filter2OrderButter_Update(Mid_Filter2OrderButter_t* filter, float32 input);

#endif /* MID_FILTER_H */

```

---

## middleware/mid_filter.c

```c
/**
 * @file mid_filter.c
 * @brief First/second-order Butterworth low-pass filters
 */

#include "mid_filter.h"

#pragma CODE_SECTION(Mid_Filter1OrderButter_Update, ".TI.ramfunc");

#define MID_FILTER_PI           (3.1415926f)
#define MID_FILTER_BUTTER2_ZETA (0.707106781f)

void Mid_Filter1OrderButter_Init(Mid_Filter1OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz)
{
    float32 wt;
    float32 den;

    if ((filter == NULL) || (cutoff_hz <= 0.0f) || (sample_hz <= 0.0f))
    {
        return;
    }

    /* Bilinear transform: yn = a0*x(n) + a1*x(n-1) - b1*y(n-1) */
    wt = 2.0f * MID_FILTER_PI * cutoff_hz / sample_hz;
    den = wt + 2.0f;
    filter->a0 = wt / den;
    filter->a1 = filter->a0;
    filter->b1 = (wt - 2.0f) / den;

    Mid_Filter1OrderButter_Reset(filter);
}

void Mid_Filter1OrderButter_Reset(Mid_Filter1OrderButter_t* filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->yn = 0.0f;
    filter->yn_last1 = 0.0f;
    filter->xn_last1 = 0.0f;
}

float32 Mid_Filter1OrderButter_Update(Mid_Filter1OrderButter_t* filter, float32 input)
{
    if (filter == NULL)
    {
        return input;
    }

    filter->yn = input * filter->a0 + filter->a1 * filter->xn_last1 - filter->b1 * filter->yn_last1;
    filter->xn_last1 = input;
    filter->yn_last1 = filter->yn;

    return filter->yn;
}

void Mid_Filter2OrderButter_Init(Mid_Filter2OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz)
{
    float32 wc;
    float32 ts;
    float32 b0;
    float32 a0;
    float32 a1;
    float32 a2;

    if ((filter == NULL) || (cutoff_hz <= 0.0f) || (sample_hz <= 0.0f))
    {
        return;
    }

    /* Same coefficient design as Second_Order_ButterWorth in cla_control.cla */
    wc = 2.0f * MID_FILTER_PI * cutoff_hz;
    ts = 1.0f / sample_hz;
    b0 = wc * wc * ts * ts;
    a0 = b0 + 4.0f * MID_FILTER_BUTTER2_ZETA * wc * ts + 4.0f;
    a1 = 2.0f * b0 - 8.0f;
    a2 = b0 - 4.0f * MID_FILTER_BUTTER2_ZETA * wc * ts + 4.0f;

    filter->nb0 = b0 / a0;
    filter->nb1 = 2.0f * filter->nb0;
    filter->na1 = a1 / a0;
    filter->na2 = a2 / a0;

    Mid_Filter2OrderButter_Reset(filter);
}

void Mid_Filter2OrderButter_Reset(Mid_Filter2OrderButter_t* filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->xn_last1 = 0.0f;
    filter->xn_last2 = 0.0f;
    filter->yn = 0.0f;
    filter->yn_last1 = 0.0f;
    filter->yn_last2 = 0.0f;
}

float32 Mid_Filter2OrderButter_Update(Mid_Filter2OrderButter_t* filter, float32 input)
{
    if (filter == NULL)
    {
        return input;
    }

    filter->yn = filter->nb0 * (filter->xn_last2 + input)
               + filter->nb1 * filter->xn_last1
               - filter->na1 * filter->yn_last1
               - filter->na2 * filter->yn_last2;

    filter->xn_last2 = filter->xn_last1;
    filter->xn_last1 = input;
    filter->yn_last2 = filter->yn_last1;
    filter->yn_last1 = filter->yn;

    return filter->yn;
}

```

---

## cmd/2837xD_FLASH_lnk_cpu2.cmd

```ld

MEMORY
{
PAGE 0 :  /* Program memory */
          /* RAM/FLASH blocks may also be allocated on PAGE 1 for data */
          /* BEGIN is used for "boot to Flash" bootloader mode */

   RAMLS0_4			: origin = 0x008000, length = 0x002800
 /*RAMLS0           : origin = 0x008000, length = 0x000800
   RAMLS1           : origin = 0x008800, length = 0x000800
   RAMLS2           : origin = 0x009000, length = 0x000800
   RAMLS3           : origin = 0x009800, length = 0x000800
   RAMLS4           : origin = 0x00A000, length = 0x000800*/
   RAMLS5			: origin = 0x00A800, length = 0x00800
   RAMD1           : origin = 0x00B800, length = 0x000800     /* D1: reserved (PAGE 0) */

   RESET            : origin = 0x3FFFC0, length = 0x000002     /* Reset vector */
   IQTABLES         : origin = 0x3FE000, length = 0x000b50     /* IQ math lookup tables */
   IQTABLES2        : origin = 0x3FEB50, length = 0x00008c
   FPUTABLES        : origin = 0x3FEBDC, length = 0x0006A0     /* FPU math lookup tables */
   BOOTROM          : origin = 0x3FF27C, length = 0x000D44     /* Boot ROM */

   /* Flash sectors Flash0-FLASH4 reserved for boot */
   //FLASH 0          : origin = 0x080000, length = 0x002000  /* on-chip Flash */
   //FLASH 1          : origin = 0x082000, length = 0x002000  /* on-chip Flash */
   //FLASH 2          : origin = 0x084000, length = 0x002000  /* on-chip Flash */
   //FLASH 3          : origin = 0x086000, length = 0x002000  /* on-chip Flash */
   //FLASH 4          : origin = 0x088000, length = 0x008000  /* on-chip Flash */

//   BEGIN              : origin = 0x098000, length = 0x000002
   BEGIN           	  : origin = 0x080000, length = 0x000002       /* Code entry point */
   PROGRAM_SECTOR     : origin = 0x098010, length = 0x01FFF0  /* CPU application Flash */
   //FLASH 5          : origin = 0x090000, length = 0x008000  /* on-chip Flash */
   //FLASH 6          : origin = 0x098000, length = 0x008000  /* on-chip Flash */
   //FLASH 7          : origin = 0x0A0000, length = 0x008000  /* on-chip Flash */
   //FLASH 8          : origin = 0x0A8000, length = 0x008000  /* on-chip Flash */
   //FLASH 9          : origin = 0x0B0000, length = 0x008000  /* on-chip Flash */
   //FLASH 10         : origin = 0x0B8000, length = 0x002000  /* on-chip Flash */
   //FLASH 11         : origin = 0x0BA000, length = 0x002000  /* on-chip Flash */
   //FLASH 12         : origin = 0x0BC000, length = 0x002000  /* on-chip Flash */
   //FLASH 13         : origin = 0x0BE000, length = 0x002000  /* on-chip Flash */

PAGE 1 : /* Data memory */
         /* RAM/FLASH blocks may also be allocated on PAGE 0 for program code */

   BOOT_RSVD       : origin = 0x000002, length = 0x00007E     /* M0 low region: Boot ROM stack */
   RAMM0           : origin = 0x000122, length = 0x0002DE     /* M0: .cio, Filter_RegsFile (PAGE 1) */
   RAMM1           : origin = 0x000400, length = 0x000400     /* M1: heap (.esysmem) */
   RAMD0           : origin = 0x00B000, length = 0x000800     /* D0: stack only (PAGE 1) */

//   RAMGS0_3    : origin = 0x00C000, length = 0x004000
   RAMGS0      : origin = 0x00C000, length = 0x001000
   RAMGS1      : origin = 0x00D000, length = 0x001000
   RAMGS2      : origin = 0x00E000, length = 0x001000
   RAMGS3      : origin = 0x00F000, length = 0x001000
   RAMGS4      : origin = 0x010000, length = 0x001000
   RAMGS5_15   : origin = 0x011000, length = 0x00B000   /* GS5-GS15, 11 x 4KB */
//   RAMGS5      : origin = 0x011000, length = 0x001000
//   RAMGS6      : origin = 0x012000, length = 0x001000
//   RAMGS7      : origin = 0x013000, length = 0x001000
//   RAMGS8      : origin = 0x014000, length = 0x001000
//   RAMGS9      : origin = 0x015000, length = 0x001000
//   RAMGS10     : origin = 0x016000, length = 0x001000
//   RAMGS11     : origin = 0x017000, length = 0x001000
//   RAMGS12     : origin = 0x018000, length = 0x001000
//   RAMGS13     : origin = 0x019000, length = 0x001000
//   RAMGS14     : origin = 0x01A000, length = 0x001000
//   RAMGS15     : origin = 0x01B000, length = 0x001000

   CPU2TOCPU1RAM   : origin = 0x03F800, length = 0x000400       /* CPU2 to CPU1 IPC message RAM */
   CPU1TOCPU2RAM   : origin = 0x03FC00, length = 0x000400

   CLA1_MSGRAMLOW   : origin = 0x001480,   length = 0x000080    /* CLA to CPU message RAM */
   CLA1_MSGRAMHIGH  : origin = 0x001500,   length = 0x000080    /* CPU to CLA message RAM */
}

SECTIONS
{
   /* Program sections */
   .cinit              : > PROGRAM_SECTOR      PAGE = 0, ALIGN(8)    /* C initialization tables */
   .pinit              : > PROGRAM_SECTOR,     PAGE = 0, ALIGN(8)    /* C++ constructor tables */
   .text               : >> PROGRAM_SECTOR      PAGE = 0, ALIGN(8)   /* Program code */
   codestart           : > BEGIN       PAGE = 0, ALIGN(8)            /* Code startup */

#ifdef __TI_COMPILER_VERSION__
    #if __TI_COMPILER_VERSION__ >= 15009000
    .TI.ramfunc : {} LOAD = PROGRAM_SECTOR,
                         RUN = RAMD1,                         /* Copy from Flash, run in RAM */
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         ALIGN(8)
    #else
   ramfuncs            : LOAD = PROGRAM_SECTOR,
                         RUN = RAMD1,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         ALIGN(8)
    #endif
#endif

   /* Uninitialized data sections */
   .stack              : > RAMD0        PAGE = 1        /* Stack in D0 (2KB) */
   .ebss               : >> RAMGS0 | RAMGS1 | RAMGS2   PAGE = 1    /* Global/static vars in GS0-GS2 */
   .cio				   : > RAMM0 	PAGE = 1        /* C I/O buffers */
   .esysmem            : > RAMM1        PAGE = 1        /* Heap in M1 (1KB) */

   /* Initialized constants in Flash (copied to RAM at startup if needed) */
   .econst             : >> PROGRAM_SECTOR     PAGE = 0, ALIGN(8)   /* const data */
   .switch             : > PROGRAM_SECTOR      PAGE = 0, ALIGN(8)   /* switch jump tables */

   .reset              : > RESET,     	PAGE = 0, TYPE = DSECT /* Placeholder only, not loaded */

   Filter_RegsFile     : > RAMM0,    PAGE = 1

/* Dual-core GS shared sections: GS3/GS4 are reserved for CPU1/CPU2 shared globals */
   SHARERAMGS0      : > RAMGS3,       PAGE = 1
   SHARERAMGS1      : > RAMGS4,       PAGE = 1
   ramgs0           : > RAMGS3,       PAGE = 1
   ramgs1           : > RAMGS4,       PAGE = 1

   /* Required for IPC API drivers */
    GROUP : > CPU1TOCPU2RAM, PAGE = 1
    {
        GETBUFFER :    TYPE = DSECT
        GETWRITEIDX :  TYPE = DSECT
        PUTREADIDX :   TYPE = DSECT
    }

    GROUP : > CPU2TOCPU1RAM, PAGE = 1
    {
        PUTBUFFER
        PUTWRITEIDX
        GETREADIDX
    }

   /* SDFM example sections (disabled) */
//   Filter1_RegsFile : > RAMGS15, PAGE = 1, fill=0x1111
//   Filter2_RegsFile : > RAMGS15, PAGE = 1, fill=0x2222
//   Filter3_RegsFile : > RAMGS15, PAGE = 1, fill=0x3333
//   Filter4_RegsFile : > RAMGS15, PAGE = 1, fill=0x4444
//   Difference_RegsFile : >RAMGS15,   PAGE = 1, fill=0x3333

//   SFRA_F_Data      : > RAMGS15, ALIGN = 64, PAGE = 1      /* SFRA float data */

//   SFRA_Data       : > RAMGS15, ALIGN = 64, PAGE = 1

   FPUmathTables   : > FPUTABLES, PAGE = 0, TYPE = NOLOAD   /* On-chip ROM, do not load */

   /* CLA sections */

   .scratchpad      : > RAMLS5,           PAGE = 0    /* CLA scratchpad */
   .bss_cla         : > RAMLS5,           PAGE = 0    /* CLA uninitialized data */

   Cla1DataRam      : > RAMLS5,           PAGE = 0    /* CLA data RAM */
   cla_shared       : > RAMLS5,           PAGE = 0    /* CPU/CLA shared data */
   CLADataLS1       : > RAMLS5,           PAGE = 0    /* CLA LS1 data */
   CLAFuncs			: > RAMLS5,			PAGE = 0      /* CLA functions */

   Cla1ToCpuMsgRAM  : > CLA1_MSGRAMLOW,   	PAGE = 1   /* CLA to CPU message RAM */
   CpuToCla1MsgRAM  : > CLA1_MSGRAMHIGH,  	PAGE = 1   /* CPU to CLA message RAM */


   .const_cla      :  LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS5,
                      RUN_START(_Cla1ConstRunStart),
                      LOAD_START(_Cla1ConstLoadStart),
                      LOAD_SIZE(_Cla1ConstLoadSize)

	Cla1Prog       :  LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS0_4,
                      LOAD_START(_Cla1funcsLoadStart),
                      LOAD_END(_Cla1funcsLoadEnd),
                      RUN_START(_Cla1funcsRunStart),
                      LOAD_SIZE(_Cla1funcsLoadSize)

   CLA1mathTables   : LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS5,
                      RUN_START(_CLA1mathTablesRunStart),
                      LOAD_START(_CLA1mathTablesLoadStart),
                      LOAD_SIZE(_CLA1mathTablesLoadSize)
}
/*
//===========================================================================
// End of file.
//===========================================================================
*/

```

---

## cmd/F2837xD_Headers_nonBIOS_cpu2.cmd

```ld

MEMORY
{
 PAGE 0:    /* Program Memory */

 PAGE 1:    /* Data Memory */ 

   ADCA_RESULT   : origin = 0x000B00, length = 0x000020
   ADCB_RESULT   : origin = 0x000B20, length = 0x000020
   ADCC_RESULT   : origin = 0x000B40, length = 0x000020
   ADCD_RESULT   : origin = 0x000B60, length = 0x000020

   ADCA          : origin = 0x007400, length = 0x000080
   ADCB          : origin = 0x007480, length = 0x000080
   ADCC          : origin = 0x007500, length = 0x000080
   ADCD          : origin = 0x007580, length = 0x000080
   
   CLA1         : origin = 0x001400, length = 0x000040     /* CLA registers */

   CMPSS1       : origin = 0x005C80, length = 0x000020
   CMPSS2       : origin = 0x005CA0, length = 0x000020
   CMPSS3       : origin = 0x005CC0, length = 0x000020
   CMPSS4       : origin = 0x005CE0, length = 0x000020
   CMPSS5       : origin = 0x005D00, length = 0x000020
   CMPSS6       : origin = 0x005D20, length = 0x000020
   CMPSS7       : origin = 0x005D40, length = 0x000020
   CMPSS8       : origin = 0x005D60, length = 0x000020

   CPU_TIMER0   : origin = 0x000C00, length = 0x000008     /* CPU Timer0 registers */
   CPU_TIMER1   : origin = 0x000C08, length = 0x000008     /* CPU Timer1 registers */
   CPU_TIMER2   : origin = 0x000C10, length = 0x000008     /* CPU Timer2 registers */

   DACA          : origin = 0x005C00, length = 0x000010
   DACB          : origin = 0x005C10, length = 0x000010
   DACC          : origin = 0x005C20, length = 0x000010
   
   CANA          : origin = 0x048000, length = 0x001000		/* DCAN A Registers */
   CANB          : origin = 0x04A000, length = 0x001000		/* DCAN B Registers */

   DMA          : origin = 0x001000, length = 0x000200
   DMACLASRCSEL : origin = 0x007980, length = 0x000040

   ECAP1        : origin = 0x005000, length = 0x000020     /* Enhanced Capture 1 registers */
   ECAP2        : origin = 0x005020, length = 0x000020     /* Enhanced Capture 2 registers */
   ECAP3        : origin = 0x005040, length = 0x000020     /* Enhanced Capture 3 registers */
   ECAP4        : origin = 0x005060, length = 0x000020     /* Enhanced Capture 4 registers */
   ECAP5        : origin = 0x005080, length = 0x000020     /* Enhanced Capture 5 registers */
   ECAP6        : origin = 0x0050A0, length = 0x000020     /* Enhanced Capture 6 registers */
   
   EMIF1        : origin = 0x047000, length = 0x000800
   EMIF2        : origin = 0x047800, length = 0x000800

   EQEP1        : origin = 0x005100, length = 0x000040     /* Enhanced QEP 1 registers */
   EQEP2        : origin = 0x005140, length = 0x000040     /* Enhanced QEP 2 registers */
   EQEP3        : origin = 0x005180, length = 0x000040     /* Enhanced QEP 3 registers */

   EPWM1        : origin = 0x004000, length = 0x000100     /* Enhanced PWM 1 registers */
   EPWM2        : origin = 0x004100, length = 0x000100     /* Enhanced PWM 2 registers */
   EPWM3        : origin = 0x004200, length = 0x000100     /* Enhanced PWM 3 registers */
   EPWM4        : origin = 0x004300, length = 0x000100     /* Enhanced PWM 4 registers */
   EPWM5        : origin = 0x004400, length = 0x000100     /* Enhanced PWM 5 registers */
   EPWM6        : origin = 0x004500, length = 0x000100     /* Enhanced PWM 6 registers */
   EPWM7        : origin = 0x004600, length = 0x000100     /* Enhanced PWM 7 registers */
   EPWM8        : origin = 0x004700, length = 0x000100     /* Enhanced PWM 8 registers */
   EPWM9        : origin = 0x004800, length = 0x000100     /* Enhanced PWM 9 registers */
   EPWM10       : origin = 0x004900, length = 0x000100     /* Enhanced PWM 10 registers */
   EPWM11       : origin = 0x004A00, length = 0x000100     /* Enhanced PWM 11 registers */
   EPWM12       : origin = 0x004B00, length = 0x000100     /* Enhanced PWM 12 registers */

   FLASH0_CTRL  : origin = 0x05F800, length = 0x000300
   FLASH0_ECC   : origin = 0x05FB00, length = 0x000040

   GPIOCTRL     : origin = 0x007C00, length = 0x000180     /* GPIO control registers */
   GPIODAT      : origin = 0x007F00, length = 0x000030     /* GPIO data registers */
   HWBIST       : origin = 0x05E000, length = 0x000100     /* HWBIST registers */
   I2CA         : origin = 0x007300, length = 0x000040     /* I2C-A registers */
   I2CB         : origin = 0x007340, length = 0x000040     /* I2C-B registers */

   IPC          : origin = 0x050000, length = 0x000024

   FLASHPUMPSEMAPHORE   : origin = 0x050024, length = 0x000002

   MEMCFG       : origin = 0x05F400, length = 0x000080     /* Mem Config registers */
   EMIF1CONFIG  : origin = 0x05F480, length = 0x000020     /* Emif-1 Config registers */
   ACCESSPROTECTION  : origin = 0x05F4C0, length = 0x000040     /* Access Protection registers */
   MEMORYERROR  : origin = 0x05F500, length = 0x000040     /* Access Protection registers */
   

   MCBSPA       : origin = 0x006000, length = 0x000040     /* McBSP-A registers */
   MCBSPB       : origin = 0x006040, length = 0x000040     /* McBSP-A registers */

   NMIINTRUPT   : origin = 0x007060, length = 0x000010     /* NMI Watchdog Interrupt Registers */

   PIE_CTRL     : origin = 0x000CE0, length = 0x000020     /* PIE control registers */
   PIE_VECT     : origin = 0x000D00, length = 0x000200     /* PIE Vector Table */
   PBIST        : origin = 0x05E200, length = 0x000200     /* PBIST registers */
   SCIA         : origin = 0x007200, length = 0x000010     /* SCI-A registers */
   SCIB         : origin = 0x007210, length = 0x000010     /* SCI-B registers */
   SCIC         : origin = 0x007220, length = 0x000010     /* SCI-C registers */
   SCID         : origin = 0x007230, length = 0x000010     /* SCI-D registers */
   
   SDFM1		: origin = 0x005E00, length = 0x000080	   /* Sigma delta 1 registers */
   SDFM2		: origin = 0x005E80, length = 0x000080	   /* Sigma delta 2 registers */

   SPIA         : origin = 0x006100, length = 0x000010
   SPIB         : origin = 0x006110, length = 0x000010
   SPIC         : origin = 0x006120, length = 0x000010
   SPID         : origin = 0x006130, length = 0x000010
   CLK_CFG     : origin = 0x05D200, length = 0x000100
   CPU_SYS     : origin = 0x05D300, length = 0x000100

   TRIG         : origin = 0x007900, length = 0x000050
   WD           : origin = 0x007000, length = 0x000040

   XINT         : origin = 0x007070, length = 0x000010
                
   DCSM_Z1      : origin = 0x05F000, length = 0x000030     /* Zone 1 Dual code security module registers */
   DCSM_Z2      : origin = 0x05F040, length = 0x000030     /* Zone 2 Dual code security module registers */
   DCSM_COMMON  : origin = 0x05F070, length = 0x000010     /* Common Dual code security module registers */

   DCSM_Z1_OTP  : origin = 0x078000, length = 0x000020     /* Part of Z1 OTP.  LinkPointer/JTAG lock/ Boot Mode */
   DCSM_Z2_OTP  : origin = 0x078200, length = 0x000020     /* Part of Z2 OTP.  LinkPointer/JTAG lock */ 
}


SECTIONS
{
/*** PIE Vect Table and Boot ROM Variables Structures ***/
  UNION run = PIE_VECT, PAGE = 1
   {
      PieVectTableFile
      GROUP
      {
         EmuKeyVar
         EmuBModeVar
         FlashCallbackVar
         FlashScalingVar
      }
   }

   AdcaResultFile        : > ADCA_RESULT,  PAGE = 1
   AdcbResultFile        : > ADCB_RESULT,  PAGE = 1
   AdccResultFile        : > ADCC_RESULT,  PAGE = 1
   AdcdResultFile        : > ADCD_RESULT,  PAGE = 1

   AdcaRegsFile          : > ADCA,         PAGE = 1
   AdcbRegsFile          : > ADCB,         PAGE = 1
   AdccRegsFile          : > ADCC,         PAGE = 1
   AdcdRegsFile          : > ADCD,         PAGE = 1
   
   Cla1RegsFile          : > CLA1,         PAGE = 1
   Cla1SoftIntRegsFile   : > PIE_CTRL, 	   PAGE = 1, type=DSECT

   Cmpss1RegsFile        : > CMPSS1,      PAGE = 1
   Cmpss2RegsFile        : > CMPSS2,      PAGE = 1
   Cmpss3RegsFile        : > CMPSS3,      PAGE = 1
   Cmpss4RegsFile        : > CMPSS4,      PAGE = 1
   Cmpss5RegsFile        : > CMPSS5,      PAGE = 1
   Cmpss6RegsFile        : > CMPSS6,      PAGE = 1
   Cmpss7RegsFile        : > CMPSS7,      PAGE = 1
   Cmpss8RegsFile        : > CMPSS8,      PAGE = 1

   CpuTimer0RegsFile    : > CPU_TIMER0,  PAGE = 1
   CpuTimer1RegsFile    : > CPU_TIMER1,  PAGE = 1
   CpuTimer2RegsFile    : > CPU_TIMER2,  PAGE = 1

   DacaRegsFile          : > DACA          PAGE = 1
   DacbRegsFile          : > DACB          PAGE = 1
   DaccRegsFile          : > DACC          PAGE = 1
   
   CanaRegsFile		     : > CANA,		   PAGE = 1
   CanbRegsFile		     : > CANB,		   PAGE = 1

   DcsmZ1RegsFile		 : > DCSM_Z1,		   PAGE = 1
   DcsmZ2RegsFile		 : > DCSM_Z2,		   PAGE = 1
   DcsmCommonRegsFile    : > DCSM_COMMON,	   PAGE = 1

   /*** Warning:  Only remove "Type = NOLOAD" to program OTP Locations ***/
   DcsmZ1OtpFile		 : > DCSM_Z1_OTP,	   PAGE = 1, type = NOLOAD
   DcsmZ2OtpFile		 : > DCSM_Z2_OTP,	   PAGE = 1, type = NOLOAD

   DmaRegsFile           : > DMA           PAGE = 1
   DmaClaSrcSelRegsFile  : > DMACLASRCSEL  PAGE = 1

   ECap1RegsFile         : > ECAP1,        PAGE = 1
   ECap2RegsFile         : > ECAP2,        PAGE = 1
   ECap3RegsFile         : > ECAP3,        PAGE = 1
   ECap4RegsFile         : > ECAP4,        PAGE = 1
   ECap5RegsFile         : > ECAP5,        PAGE = 1
   ECap6RegsFile         : > ECAP6,        PAGE = 1
   
   Emif1RegsFile         : > EMIF1         PAGE = 1
   Emif2RegsFile         : > EMIF2         PAGE = 1

   EPwm1RegsFile         : > EPWM1,        PAGE = 1
   EPwm2RegsFile         : > EPWM2,        PAGE = 1
   EPwm3RegsFile         : > EPWM3,        PAGE = 1
   EPwm4RegsFile         : > EPWM4,        PAGE = 1
   EPwm5RegsFile         : > EPWM5,        PAGE = 1
   EPwm6RegsFile         : > EPWM6,        PAGE = 1
   EPwm7RegsFile         : > EPWM7,        PAGE = 1
   EPwm8RegsFile         : > EPWM8,        PAGE = 1
   EPwm9RegsFile         : > EPWM9,        PAGE = 1
   EPwm10RegsFile        : > EPWM10,       PAGE = 1
   EPwm11RegsFile        : > EPWM11,       PAGE = 1
   EPwm12RegsFile        : > EPWM12,       PAGE = 1

   EQep1RegsFile         : > EQEP1,        PAGE = 1
   EQep2RegsFile         : > EQEP2,        PAGE = 1
   EQep3RegsFile         : > EQEP3,        PAGE = 1

   Flash0CtrlRegsFile     : > FLASH0_CTRL    PAGE = 1
   Flash0EccRegsFile      : > FLASH0_ECC     PAGE = 1

   GpioCtrlRegsFile      : > GPIOCTRL,     PAGE = 1
   GpioDataRegsFile      : > GPIODAT,      PAGE = 1

   HwbistRegsFile        : > HWBIST         PAGE = 1
   I2caRegsFile          : > I2CA,          PAGE = 1
   I2cbRegsFile          : > I2CB,          PAGE = 1

   IpcRegsFile           : > IPC,           PAGE = 1

   FlashPumpSemaphoreRegsFile   : > FLASHPUMPSEMAPHORE,    PAGE = 1

   MemCfgRegsFile        : > MEMCFG,       PAGE = 1
   Emif1ConfigRegsFile       : > EMIF1CONFIG,       PAGE = 1
   AccessProtectionRegsFile  : > ACCESSPROTECTION,  PAGE = 1
   MemoryErrorRegsFile       : > MEMORYERROR,       PAGE = 1

   McbspaRegsFile        : > MCBSPA,       PAGE = 1
   McbspbRegsFile        : > MCBSPB,       PAGE = 1

   NmiIntruptRegsFile    : > NMIINTRUPT,   PAGE = 1
   PbistRegsFile         : > PBIST,        PAGE = 1
   PieCtrlRegsFile       : > PIE_CTRL,     PAGE = 1

   SciaRegsFile         : > SCIA,         PAGE = 1
   ScibRegsFile         : > SCIB,         PAGE = 1
   ScicRegsFile         : > SCIC,         PAGE = 1
   ScidRegsFile         : > SCID,         PAGE = 1
   
   Sdfm1RegsFile		 : > SDFM1,		   PAGE = 1
   Sdfm2RegsFile		 : > SDFM2,		   PAGE = 1

   SpiaRegsFile          : > SPIA,        PAGE = 1
   SpibRegsFile          : > SPIB,        PAGE = 1
   SpicRegsFile          : > SPIC,        PAGE = 1
   SpidRegsFile          : > SPID,        PAGE = 1

   ClkCfgRegsFile       : > CLK_CFG,     PAGE = 1
   CpuSysRegsFile       : > CPU_SYS,     PAGE = 1

   WdRegsFile            : > WD,           PAGE = 1

   XintRegsFile          : > XINT          PAGE = 1
   MemCfgRegs		 : > MEMCFG        PAGE = 1

}

/*
//===========================================================================
// End of file.
//===========================================================================
*/

```
