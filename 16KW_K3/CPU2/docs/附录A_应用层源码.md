# 附录 A：应用层源码（app/）

> 本附录收录 CPU2 工程全部业务应用层源码（不含 TI 芯片库）。路径相对工程根目录。


---

## app/main.c

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        main.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief       CPU2 main (minimal init + IPC handshake with CPU1)
  ******************************************************************************
  */

#include "F28x_Project.h"
#include "F2837xD_Examples.h"
#include "dual_core_cpu2.h"
#include "sys_ctrl_cpu2.h"
#include "cpu_timers.h"
#include "cpu_control.h"
#include "dev_dual_core_receive.h"
#include "dual_data_share.h"
#include "ph3_calc_ifc_update.h"
#include "mid_vol_and_rcmu_rms.h"
#include "debug_analysis.h"

void main(void)
{
    DualCore_Cpu2Status_t handshakeStatus;

    SysCtrlCpu2_Init();
    DELAY_MS(500);
    DINT;

    IER = 0x0000;
    IFR = 0x0000;

    InitPieCtrl();
    InitPieVectTable();

    handshakeStatus = DualCore_HandshakeCpu2(IPC_DUAL_HANDSHAKE_TIMEOUT_MS);
    if (handshakeStatus != DUAL_CORE_OK)
    {
        for (;;)
        {
        }
    }

    DualCore_ConfigGsRamAccessProtect();
    DualDataShare_Init();
    Ph3CalcIfcUpdate_Init();
    (void)DevDcr_ExchangeAddress(DEV_DCR_ADDR_EXCHANGE_TIMEOUT_MS);
    CpuControl_Ph3CalcInit();
    MidVolAndRcmuRms_Init();
    CpuTimer_Init();
    CpuIntFunInit();
    CpuControl_Ph3StsInit();
    
    EINT;
    ERTM;

    for (;;)
    {
        CpuTimerFlag();
        CpuControl_Ph3DataUpdate();
        if (Get_1ms_Flag() == 1)
        {
//            SetCpuDebugData(0,0);
//            SetCpuDebugData(1,1);
//            SetCpuDebugData(2,2);
//            SetCpuDebugData(3,3);
//            SetCpuDebugData(4,4);
//            SetCpuDebugData(5,5);
//            SetCpuDebugData(6,6);
//            SetCpuDebugData(7,7);
//            SetCpuDebugData(8,8);
//            SetCpuDebugData(9,9);
//            SetCpuDebugData(10,10);
//            SetCpuDebugData(11,11);
//            SetCpuDebugData(12,12);
//            SetCpuDebugData(13,13);
            CpuControl_Ph3FlagUpdate();
            Timers_1ms();
        }
        if (Get_5ms_Flag() == 1)
        {
            Timers_5ms();
        }
        if (Get_10ms_Flag() == 1)
        {
            Timers_10ms();
        }
        if (Get_20ms_Flag() == 1)
        {
            Timers_20ms();
        }
        if (Get_100ms_Flag() == 1)
        {
            Timers_100ms();
        }
        if (Get_500ms_Flag() == 1)
        {
            Timers_500ms();
        }
        if (Get_1s_Flag() == 1)
        {
            Timers_1s();
        }
    }
}

```

---

## app/cpu_control.h

```c
/*
 * cpu_control.h
 *
 *  Created on: Nov 22, 2024
 *      Author: Nova007456
 */
#ifndef APP_CPU_CONTROL_H_
#define APP_CPU_CONTROL_H_

#include <stdint.h>


void CpuIntFunInit(void);
void CpuIntServiceFun(void);
uint32_t GetCpuInterruptRunCont(void);
uint32_t GetCpuInterruptQIdleCnt(void);

void CpuControl_Ph3CalcInit(void);
void CpuControl_Ph3StsInit(void);
void CpuControl_Ph3DataUpdate(void);
void CpuControl_Ph3FlagUpdate(void);

#endif /* APP_CPU_CONTROL_H_ */

```

---

## app/cpu_control.c

```c
/*
 * cpu_control.c
 *
 *  Created on: Nov 22, 2024
 *      Author: Nova007456
 */
#include "cpu_control.h"
#include "ph3_calc_run.h"
#include "interrupt.h"
#include "dual_data_share.h"
#include "ph3_sts_run.h"
#pragma CODE_SECTION(CpuIntServiceFun, ".TI.ramfunc");

static uint32_t s_interruptRunCnt;
static uint32_t s_interruptQIdleCnt;
static uint32_t s_interruptExitTim;

void CpuIntServiceFun(void)
{
    uint32_t intCont;
    uint32_t intContNext;

    intCont = CpuTimer1Regs.TIM.all;

    if(s_interruptExitTim >= intCont)
    {
        s_interruptQIdleCnt = s_interruptExitTim - intCont;
    }
    else if(s_interruptExitTim < intCont)
    {
        s_interruptQIdleCnt = s_interruptExitTim + (CpuTimer1Regs.PRD.all + 1U - intCont);
    }
    DualDataShare_Ph3IfcRunDone();
    Ph3CalcRun_Run();
    Ph3_Sts_Run();
    intContNext = CpuTimer1Regs.TIM.all;

    if(intCont >= intContNext)
    {
        s_interruptRunCnt = intCont - intContNext;

    }
    else if(intCont < intContNext)
    {
        s_interruptRunCnt = intCont + (CpuTimer1Regs.PRD.all + 1U - intContNext);
    }

    s_interruptExitTim = intContNext;

    DualDataShare_SetIsrRunCnt(s_interruptRunCnt);
    DualDataShare_SetIsrQIdleCnt(s_interruptQIdleCnt);
}

uint32_t GetCpuInterruptRunCont(void)
{
    return s_interruptRunCnt;
}

uint32_t GetCpuInterruptQIdleCnt(void)
{
    return s_interruptQIdleCnt;
}

void CpuIntFunInit(void)
{
    RegisterCpuIntCallback(&CpuIntServiceFun);
    Interrupt_Init();
}

void CpuControl_Ph3CalcInit(void)
{
    Ph3CalcRun_Init();
}

void CpuControl_Ph3StsInit(void)
{
    Ph3_Sts_Init();
}

void CpuControl_Ph3DataUpdate(void)
{
    Ph3CalcRun_DataUpdate();
    Ph3_Sts_DataSlowUpdate();
}

void CpuControl_Ph3FlagUpdate(void)
{
    Ph3CalcRun_FlagUpdate();
    Ph3_Sts_FlagUpdate();
}

```

---

## app/dual_data_share.h

```c
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

```

---

## app/dual_data_share.c

```c
/*
 * dual_data_share.c
 *
 * CPU1/CPU2 shared data accessors (CPU2 side).
 */
#include "dual_data_share.h"
#include "ph3_calc_ifc_update.h"
#include "ph3_sts_run.h"
#include "debug_analysis.h"

#pragma CODE_SECTION(DualDataShare_Ph3IfcRunDone, ".TI.ramfunc");
#pragma CODE_SECTION(DualDataShare_SetIsrRunCnt, ".TI.ramfunc");

void DualDataShare_Init(void)
{
    volatile DevDct_Cpu2ToCpu1Msg_t *msg = DualDataShare_GetCpu2ToCpu1Msg();

    msg->cpu2_ph3_ifc_addr = (uintptr_t)Ph3CalcIfcUpdate_Get();
    msg->isr_run_cnt = 0U;
    msg->heartbeat_cnt = 0U;
    msg->isr_idle_cnt = 0U;
    msg->sts_output_addr = (uintptr_t)StsOutputUpdate_Get();
    msg->cpu2_debug_data_addr = (uintptr_t)GetCpuDebugDataPtr();
}

void DualDataShare_Ph3IfcRunDone(void)
{
    volatile DevDct_Cpu2ToCpu1Msg_t *msg = DualDataShare_GetCpu2ToCpu1Msg();

    if(msg->heartbeat_cnt >= 0xFFFFFFFFU)
    {
        msg->heartbeat_cnt = 0U;
    }
    else
    {
        msg->heartbeat_cnt++;
    }
}

void DualDataShare_SetIsrRunCnt(uint32_t cnt)
{
    DualDataShare_GetCpu2ToCpu1Msg()->isr_run_cnt = cnt;
}

void DualDataShare_SetIsrQIdleCnt(uint32_t cnt)
{
    DualDataShare_GetCpu2ToCpu1Msg()->isr_idle_cnt = cnt;
}
```

---

## app/dev_dual_core_receive.h

```c
/*
 * dev_dual_core_receive.h
 *
 * CPU2 receive CPU1 shared RAM data (GS3/GS4).
 */
#ifndef APP_DEV_DUAL_CORE_RECEIVE_H_
#define APP_DEV_DUAL_CORE_RECEIVE_H_

#include <stdbool.h>
#include <stdint.h>
#include "F28x_Project.h"
#include "ph3_calc/ph3_calc_channel.h"

#include "dual_data_share.h"

#define CPU_ADC_PH3_RUN_DISABLE           0U
#define CPU_ADC_PH3_RUN_ENABLE            1U

/** Matches Ph3Calc_Channel_t enable flags */
typedef struct cpu_adc_ph3_run_flag_stru_t
{
    volatile uint16_t enable_volt_pll;
    volatile uint16_t enable_curr_pll;
    volatile uint16_t enable_power_calc;
} CpuAdcPh3RunFlag_t;

typedef struct sts_input_status_stru_t
{
    volatile uint16_t sts_run_en;
    volatile uint16_t force_gen_mode;
    volatile uint16_t en_connect;
    volatile uint16_t en_disconnect;
    
    volatile uint16_t warn_clear;
    volatile uint16_t err_clear;
} StsInputStatus_t;
/** CPU1 GS3 ADC result, layout must match CPU1 gstAdcResult */
typedef struct cpu_adc_stru_t
{
    float32 inv_volt_phase_a;
    float32 inv_volt_phase_b;
    float32 inv_volt_phase_c;
    float32 inv_curr_phase_a;
    float32 inv_curr_phase_b;
    float32 inv_curr_phase_c;
    float32 load_vol_n_pe;
    float32 leak_curr;

    float32 relay_middle_volt_phase_a;
    float32 relay_middle_volt_phase_b;
    float32 relay_middle_volt_phase_c;

    float32 grid_volt_phase_a;
    float32 grid_volt_phase_b;
    float32 grid_volt_phase_c;
    float32 grid_curr_phase_a;
    float32 grid_curr_phase_b;
    float32 grid_curr_phase_c;
    float32 ct_a_curr;
    float32 ct_b_curr;
    float32 ct_c_curr;

    float32 load_volt_phase_a;
    float32 load_volt_phase_b;
    float32 load_volt_phase_c;
    float32 load_curr_phase_a;
    float32 load_curr_phase_b;
    float32 load_curr_phase_c;

    float32 pv1_volt;
    float32 pv1_curr;
    float32 pv2_volt;
    float32 pv2_curr;
    float32 pv3_volt;
    float32 pv3_curr;
    float32 pv4_volt;
    float32 pv4_curr;

    float32 bus_volt_pos;
    float32 bus_volt_neg;
    float32 bus_curr;

    float32 gen_volt_phase_a;
    float32 gen_volt_phase_b;
    float32 gen_volt_phase_c;
    float32 gen_curr_phase_a;
    float32 gen_curr_phase_b;
    float32 gen_curr_phase_c;

    float32 Temp_A0;
    float32 Temp_A1;
    float32 Temp_A2;
    float32 Temp_A3;
    float32 Temp_A4;
    float32 Temp_A5;
    float32 Temp_A6;
    float32 Temp_A7;
    float32 V1_5;

    float32 inv_temp;
    float32 pv_temp;
    float32 ctr_temp;
} CpuAdc_t;

/** CPU1 -> CPU2 message at CPU1TOCPU2RAM */
typedef struct
{
    volatile uintptr_t cpu1_adc_result_addr;  /* CPU1 GS3 ADC result buffer address */
    CpuAdcPh3RunFlag_t grid;                  /* Grid ph3 run flags */
    CpuAdcPh3RunFlag_t gen;                   /* Generator ph3 run flags */
    CpuAdcPh3RunFlag_t load;                  /* Load ph3 run flags */
    CpuAdcPh3RunFlag_t inv;                   /* Inverter ph3 run flags */
    CpuAdcPh3RunFlag_t ct;                    /* CT ph3 run flags */
    volatile uintptr_t sts_input_addr;         /* Sts_Input_t address */

} DevDct_Cpu1ToCpu2Msg_t;

#define DUAL_DATA_SHARE_CPU1_TO_CPU2_ADDR   0x0003FC00UL

/** CPU1->CPU2 message mapped to CPU1TOCPU2RAM, no linker section required */
#define DualDataShare_GetCpu1ToCpu2Msg() \
    ((volatile DevDct_Cpu1ToCpu2Msg_t *)(DUAL_DATA_SHARE_CPU1_TO_CPU2_ADDR))

#define DEV_DCR_CPU2_TO_CPU1_ADDR         DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR
#define DEV_DCR_CPU1_TO_CPU2_ADDR         DUAL_DATA_SHARE_CPU1_TO_CPU2_ADDR
#define DEV_DCR_ADDR_EXCHANGE_TIMEOUT_MS  100U

#define DevDcr_GetCpu1ToCpu2Msg()         DualDataShare_GetCpu1ToCpu2Msg()
#define DevDcr_GetCpu2ToCpu1Msg()         DualDataShare_GetCpu2ToCpu1Msg()

bool DevDcr_ExchangeAddress(uint32_t timeoutMs);
volatile CpuAdc_t *DevDcr_GetCpuAdcResult(void);
volatile DevDct_Cpu1ToCpu2Msg_t *DevDcr_GetCpu1ToCpu2MsgPtr(void);
void DevDcr_ApplyPh3RunFlag(Ph3Calc_Channel_t *ch,
                            const volatile CpuAdcPh3RunFlag_t *flag);
volatile StsInputStatus_t *DevDcr_GetStsInput(void);
#endif /* APP_DEV_DUAL_CORE_RECEIVE_H_ */

```

---

## app/dev_dual_core_receive.c

```c
/*
 * dev_dual_core_receive.c
 */
#include "dev_dual_core_receive.h"
#include "F2837xD_Examples.h"

#pragma CODE_SECTION(DevDcr_GetCpuAdcResult, ".TI.ramfunc");
#pragma CODE_SECTION(DevDcr_GetCpu1ToCpu2MsgPtr, ".TI.ramfunc");

static volatile CpuAdc_t *s_adc_result = (volatile CpuAdc_t *)0;
static volatile StsInputStatus_t *s_sts_input = (volatile StsInputStatus_t *)0;

static volatile void *DevDcr_GetCpu1AdcAddress(void)
{
    volatile DevDct_Cpu1ToCpu2Msg_t *msg = DevDcr_GetCpu1ToCpu2Msg();

    return (volatile void *)(msg->cpu1_adc_result_addr);
}

static volatile void *DevDcr_GetCpu1StsInputAddress(void)
{
    volatile DevDct_Cpu1ToCpu2Msg_t *msg = DevDcr_GetCpu1ToCpu2Msg();

    return (volatile void *)(msg->sts_input_addr);
}

bool DevDcr_ExchangeAddress(uint32_t timeoutMs)
{
    uint32_t waitMs = 0U;

    while (waitMs < timeoutMs)
    {
        s_adc_result = (volatile CpuAdc_t *)DevDcr_GetCpu1AdcAddress();
        s_sts_input = (volatile StsInputStatus_t *)DevDcr_GetCpu1StsInputAddress();
        if ((s_adc_result != (volatile CpuAdc_t *)0) && (s_sts_input != (volatile StsInputStatus_t *)0))
        {
            return true;
        }

        DELAY_MS(1U);
        waitMs++;
    }

    return false;
}

volatile CpuAdc_t *DevDcr_GetCpuAdcResult(void)
{
    return s_adc_result;
}
volatile StsInputStatus_t *DevDcr_GetStsInput(void)
{
    return s_sts_input;
}
volatile DevDct_Cpu1ToCpu2Msg_t *DevDcr_GetCpu1ToCpu2MsgPtr(void)
{
    return DevDcr_GetCpu1ToCpu2Msg();
}

void DevDcr_ApplyPh3RunFlag(Ph3Calc_Channel_t *ch,
                            const volatile CpuAdcPh3RunFlag_t *flag)
{
    if((ch == NULL) || (flag == NULL))
    {
        return;
    }

    ch->enable_volt_pll = flag->enable_volt_pll;
    ch->enable_curr_pll = flag->enable_curr_pll;
    ch->enable_power_calc = flag->enable_power_calc;
}

```

---

## app/ph3_calc_run.h

```c
/*
 * ph3_calc_run.h
 *
 * CPU2 three-phase calculation runtime.
 */
#ifndef APP_PH3_CALC_RUN_H_
#define APP_PH3_CALC_RUN_H_

#include "ph3_calc/ph3_calc_channel.h"

extern Ph3Calc_Channel_t s_grid;
extern Ph3Calc_Channel_t s_gen;
extern Ph3Calc_Channel_t s_load;
extern Ph3Calc_Channel_t s_inv;
extern Ph3Calc_Channel_t s_ct;

void Ph3CalcRun_Init(void);
void Ph3CalcRun_Run(void);
void Ph3CalcRun_FlagUpdate(void);
void Ph3CalcRun_DataUpdate(void);

#endif /* APP_PH3_CALC_RUN_H_ */

```

---

## app/ph3_calc_run.c

```c
/*
 * ph3_calc_run.c
 *
 * CPU2 three-phase calculation runtime.
 */
#include "ph3_calc_run.h"
#include "dev_dual_core_receive.h"
#include "dual_data_share.h"
#include "ph3_calc_ifc_update.h"
#include "ph3_calc/ph3_calc_filter.h"
#include "ph3_calc/ph3_calc_channel.h"
#include "mid_vol_and_rcmu_rms.h"

#pragma CODE_SECTION(Ph3CalcRun_Run, ".TI.ramfunc");

Ph3Calc_Channel_t s_grid;
Ph3Calc_Channel_t s_gen;
Ph3Calc_Channel_t s_load;
Ph3Calc_Channel_t s_inv;
Ph3Calc_Channel_t s_ct;

static Ph3Calc_Filter_t s_busCurrFilter;
static volatile bool s_dataUpdateFlag = false;

void Ph3CalcRun_Init(void)
{
    Ph3Calc_ChannelInit(&s_grid, PH3_CALC_PF_MODE_NORMAL, PH3_CALC_PLL_MODE_POSSEQ);
    Ph3Calc_ChannelInit(&s_gen, PH3_CALC_PF_MODE_NORMAL, PH3_CALC_PLL_MODE_POSSEQ);
    Ph3Calc_ChannelInit(&s_load, PH3_CALC_PF_MODE_NORMAL, PH3_CALC_PLL_MODE_POSSEQ);
    Ph3Calc_ChannelInit(&s_inv, PH3_CALC_PF_MODE_INV, PH3_CALC_PLL_MODE_POSSEQ);
    Ph3Calc_ChannelInit(&s_ct, PH3_CALC_PF_MODE_NORMAL, PH3_CALC_PLL_MODE_POSSEQ);

    Ph3Calc_Filter_Init(&s_busCurrFilter, PH3_CALC_FILTER_DEFAULT_WIN);
}

void Ph3CalcRun_Run(void)
{
    volatile CpuAdc_t *adc = DevDcr_GetCpuAdcResult();

    if(adc == (volatile CpuAdc_t *)0)
    {
        return;
    }

    Ph3Calc_ChannelSetSample(&s_grid,
                             adc->grid_volt_phase_a, adc->grid_volt_phase_b, adc->grid_volt_phase_c,
                             adc->grid_curr_phase_a, adc->grid_curr_phase_b, adc->grid_curr_phase_c);
    Ph3Calc_ChannelRun(&s_grid);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.grid, &s_grid);
    Ph3Calc_ChannelSetSample(&s_gen,
                             adc->gen_volt_phase_a, adc->gen_volt_phase_b, adc->gen_volt_phase_c,
                             adc->gen_curr_phase_a, adc->gen_curr_phase_b, adc->gen_curr_phase_c);
    Ph3Calc_ChannelRun(&s_gen);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.gen, &s_gen);
    Ph3Calc_ChannelSetSample(&s_load,
                             adc->load_volt_phase_a, adc->load_volt_phase_b, adc->load_volt_phase_c,
                             adc->load_curr_phase_a, adc->load_curr_phase_b, adc->load_curr_phase_c);
    Ph3Calc_ChannelRun(&s_load);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.load, &s_load);
    Ph3Calc_ChannelSetSample(&s_inv,
                             adc->inv_volt_phase_a, adc->inv_volt_phase_b, adc->inv_volt_phase_c,
                             adc->inv_curr_phase_a, adc->inv_curr_phase_b, adc->inv_curr_phase_c);
    Ph3Calc_ChannelRun(&s_inv);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.inv, &s_inv);
    Ph3Calc_ChannelSetSample(&s_ct,
                             adc->grid_volt_phase_a, adc->grid_volt_phase_b, adc->grid_volt_phase_c,
                             adc->ct_a_curr, adc->ct_b_curr, adc->ct_c_curr);
    Ph3Calc_ChannelRun(&s_ct);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.ct, &s_ct);
    gstPh3CalcIfcUpdate.bus.curr_filter = Ph3Calc_Filter_Run(&s_busCurrFilter, adc->bus_curr);
    gstPh3CalcIfcUpdate.bus.pos_vol_filter = adc->bus_volt_pos;
    gstPh3CalcIfcUpdate.bus.neg_vol_filter = adc->bus_volt_neg;
    gstPh3CalcIfcUpdate.bus.vol_filter  = gstPh3CalcIfcUpdate.bus.pos_vol_filter + gstPh3CalcIfcUpdate.bus.neg_vol_filter;
    gstPh3CalcIfcUpdate.bus.power_filter = gstPh3CalcIfcUpdate.bus.vol_filter * gstPh3CalcIfcUpdate.bus.curr_filter;
    DualDataShare_Ph3IfcRunDone();
    MidVolAndRcmuRms_Run(adc->relay_middle_volt_phase_a, adc->relay_middle_volt_phase_b, adc->relay_middle_volt_phase_c,
                         adc->leak_curr, gstPh3CalcIfcUpdate.inv.vol.freq_a);
    s_dataUpdateFlag = true;
}

void Ph3CalcRun_FlagUpdate(void)
{
    volatile DevDct_Cpu1ToCpu2Msg_t *msg = DevDcr_GetCpu1ToCpu2Msg();

    if(msg == (volatile DevDct_Cpu1ToCpu2Msg_t *)0)
    {
        return;
    }

    DevDcr_ApplyPh3RunFlag(&s_grid, &msg->grid);
    DevDcr_ApplyPh3RunFlag(&s_gen, &msg->gen);
    DevDcr_ApplyPh3RunFlag(&s_load, &msg->load);
    DevDcr_ApplyPh3RunFlag(&s_inv, &msg->inv);
    DevDcr_ApplyPh3RunFlag(&s_ct, &msg->ct);
}

void Ph3CalcRun_DataUpdate(void)
{
    if(s_dataUpdateFlag)
    {
        s_dataUpdateFlag = false;
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.grid, &s_grid);
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.gen, &s_gen);
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.load, &s_load);
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.inv, &s_inv);
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.ct, &s_ct);
        MidVolAndRcmuRms_Update(&gstPh3CalcIfcUpdate);
    }
}

```

---

## app/ph3_calc_ifc_update.h

```c
/*
 * ph3_calc_ifc_update.h
 *
 * CPU2 ph3 calc interface update (GS4 shared RAM, consumed by CPU1).
 */
#ifndef APP_PH3_CALC_IFC_UPDATE_H_
#define APP_PH3_CALC_IFC_UPDATE_H_

#include "ph3_calc/ph3_calc_interface.h"

/** 母线滤波量 */
typedef struct ph3_calc_ifc_bus_stru_t
{
    float32 vol_filter;
    float32 curr_filter;
    float32 power_filter;
    float32 pos_vol_filter;
    float32 neg_vol_filter;
} Ph3CalcIfcBus_t;

/** CPU2 ph3 interface: grid / gen / load / inv / ct (placed in GS4 shared RAM) */
typedef struct dual_data_share_ph3_ifc_stru_t
{
    Ph3Calc_Interface_t grid;
    Ph3Calc_Interface_t gen;
    Ph3Calc_Interface_t load;
    Ph3Calc_Interface_t inv;
    Ph3Calc_Interface_t ct;
    Ph3CalcIfcBus_t bus;
    float32 rcmu_curr_rms;
    float32 mid_vol_a_rms;
    float32 mid_vol_b_rms;
    float32 mid_vol_c_rms;
} DualDataShare_Ph3Ifc_t;

extern DualDataShare_Ph3Ifc_t gstPh3CalcIfcUpdate;

void Ph3CalcIfcUpdate_Init(void);
void Ph3CalcIfcUpdate(Ph3Calc_Interface_t *ifc, const Ph3Calc_Channel_t *ch);
void Ph3CalcIfcAngleUpdate(Ph3Calc_Interface_t *ifc, const Ph3Calc_Channel_t *ch);
DualDataShare_Ph3Ifc_t *Ph3CalcIfcUpdate_Get(void);

#endif /* APP_PH3_CALC_IFC_UPDATE_H_ */

```

---

## app/ph3_calc_ifc_update.c

```c
/*
 * ph3_calc_ifc_update.c
 *
 * CPU2 ph3 calc interface update (GS4 shared RAM, consumed by CPU1).
 */
#include "ph3_calc_ifc_update.h"

#pragma CODE_SECTION(Ph3CalcIfcUpdate, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3CalcIfcAngleUpdate, ".TI.ramfunc");

#pragma DATA_SECTION(gstPh3CalcIfcUpdate, "ramgs1")
DualDataShare_Ph3Ifc_t gstPh3CalcIfcUpdate;

static void Ph3CalcIfcBus_Init(Ph3CalcIfcBus_t *bus)
{
    bus->vol_filter = 0.0f;
    bus->curr_filter = 0.0f;
    bus->power_filter = 0.0f;
    bus->pos_vol_filter = 0.0f;
    bus->neg_vol_filter = 0.0f;
}

void Ph3CalcIfcUpdate_Init(void)
{
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.grid);
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.gen);
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.load);
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.inv);
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.ct);
    Ph3CalcIfcBus_Init(&gstPh3CalcIfcUpdate.bus);
    gstPh3CalcIfcUpdate.rcmu_curr_rms = 0.0f;
    gstPh3CalcIfcUpdate.mid_vol_a_rms = 0.0f;
    gstPh3CalcIfcUpdate.mid_vol_b_rms = 0.0f;
    gstPh3CalcIfcUpdate.mid_vol_c_rms = 0.0f;
}

void Ph3CalcIfcUpdate(Ph3Calc_Interface_t *ifc, const Ph3Calc_Channel_t *ch)
{
    if((ifc == NULL) || (ch == NULL))
    {
        return;
    }

    Ph3Calc_InterfaceUpdate(ifc, ch);
}

void Ph3CalcIfcAngleUpdate(Ph3Calc_Interface_t *ifc, const Ph3Calc_Channel_t *ch)
{
    if((ifc == NULL) || (ch == NULL))
    {
        return;
    }

    Ph3Calc_InterfaceAngleUpdate(ifc, ch);
}

DualDataShare_Ph3Ifc_t *Ph3CalcIfcUpdate_Get(void)
{
    return &gstPh3CalcIfcUpdate;
}

```

---

## app/ph3_sts_run.h

```c
/*
 * ph3_sts_run.h
 *
 * CPU2 three-phase status runtime.
 */
#ifndef APP_PH3_STS_RUN_H_
#define APP_PH3_STS_RUN_H_

#include <stdint.h>
#include <stdbool.h>
#include "F28x_Project.h"
#include "AlgoFw/package/cntl/sts/sts_interface.h"
#include "AlgoFw/package/cntl/sts/sts_adjust.h"



/* STS algorithm framework interfaces (external AlgoFw) */

typedef struct sts_output_status_stru_t
{
    Sts_Output_t sts_output;
    Sts_Err_Un_t sts_err;
    Sts_Warn_Un_t sts_warn;
    EN_STS_STATE  sts_state;
    uint8_t  sts_is_disconnectd;
    uint8_t  sts_is_disconnecting;
    uint8_t  sts_is_connecting;
    uint8_t  sts_is_connected;
    uint8_t  sts_is_gen_mode;
} StsOutputStatus_t;





void Ph3_Sts_Init(void);
void Ph3_Sts_Run(void);
StsOutputStatus_t *StsOutputUpdate_Get(void);
void Ph3_Sts_FlagUpdate(void);
void Ph3_Sts_DataSlowUpdate(void);
#endif /* APP_PH3_STS_RUN_H_ */


```

---

## app/ph3_sts_run.c

```c
/*
 * ph3_sts_run.c
 *
 * CPU2 three-phase status runtime.
 */
#include "ph3_sts_run.h"
#include "ph3_calc_run.h"
#include "dev_dual_core_receive.h"
#include <string.h>

static Sts_Input_t _gstStsInput;
#pragma DATA_SECTION(_gstStsOutput, "ramgs1")
StsOutputStatus_t _gstStsOutput;

StsInputStatus_t _gstStsInputStatus;

Sts_Adjust_Input_t _gstStsAdjustInput;

static uint8_t _gstStsFlagUpdateFlag = 0;
StsOutputStatus_t *StsOutputUpdate_Get(void)
{
    return &_gstStsOutput;
}

void Ph3_Sts_FlagUpdate(void)
{
    volatile StsInputStatus_t *sts_input = DevDcr_GetStsInput();

    if(sts_input == (volatile StsInputStatus_t *)0)
    {
        return;
    }
    _gstStsInputStatus.sts_run_en = sts_input->sts_run_en;
    _gstStsInputStatus.force_gen_mode = sts_input->force_gen_mode;
    _gstStsInputStatus.en_connect = sts_input->en_connect;
    _gstStsInputStatus.en_disconnect = sts_input->en_disconnect;
    _gstStsInputStatus.warn_clear = sts_input->warn_clear;
    _gstStsInputStatus.err_clear = sts_input->err_clear;
}

void Ph3_Sts_DataSlowUpdate(void)
{
    if(_gstStsFlagUpdateFlag == 0)
    {
        return;
    }
    _gstStsFlagUpdateFlag = 0;
    if(_gstStsInputStatus.force_gen_mode != _gstStsOutput.sts_is_gen_mode)
    {
        _gstStsOutput.sts_is_gen_mode = _gstStsInputStatus.force_gen_mode;
    }
    if(_gstStsOutput.sts_is_gen_mode)
    {
        _gstStsInput.connected_volt_rms_a = s_gen.phase_a.vol.rms;
        _gstStsInput.connected_volt_rms_b = s_gen.phase_b.vol.rms;
        _gstStsInput.connected_volt_rms_c = s_gen.phase_c.vol.rms;

        _gstStsInput.connected_curr_rms_a = s_gen.phase_a.cur.rms;
        _gstStsInput.connected_curr_rms_b = s_gen.phase_b.cur.rms;
        _gstStsInput.connected_curr_rms_c = s_gen.phase_c.cur.rms;
    }
    else
    {
        _gstStsInput.connected_volt_rms_a = s_grid.phase_a.vol.rms;
        _gstStsInput.connected_volt_rms_b = s_grid.phase_b.vol.rms;
        _gstStsInput.connected_volt_rms_c = s_grid.phase_c.vol.rms;

        _gstStsInput.connected_curr_rms_a = s_grid.phase_a.cur.rms;
        _gstStsInput.connected_curr_rms_b = s_grid.phase_b.cur.rms;
        _gstStsInput.connected_curr_rms_c = s_grid.phase_c.cur.rms;
    }

    _gstStsInput.inv_volt_rms_a = s_inv.phase_a.vol.rms;
    _gstStsInput.inv_volt_rms_b = s_inv.phase_b.vol.rms;
    _gstStsInput.inv_volt_rms_c = s_inv.phase_c.vol.rms;

    Sts_WarnCheck_Run();
    Sts_FaultCheck_Run();

    _gstStsOutput.sts_err = *Sts_Get_Err_Obj();
    _gstStsOutput.sts_warn = *Sts_Get_Warn_Obj();
    _gstStsOutput.sts_is_connecting = Sts_Get_IsConnecting();
    _gstStsOutput.sts_is_disconnecting = Sts_Get_IsDisconnecting();
    _gstStsOutput.sts_is_disconnectd = Sts_Get_IsDisconnected();
    _gstStsOutput.sts_is_connected = Sts_Get_IsConnected();
    _gstStsOutput.sts_state = Sts_Get_State();
}

void Ph3_Sts_InstDateUpdate(void)
{
    volatile CpuAdc_t *adc = DevDcr_GetCpuAdcResult();

    if(adc == (volatile CpuAdc_t *)0)
    {
        return;
    }
    if(_gstStsOutput.sts_is_gen_mode)
    {
        _gstStsInput.connected_volt_inst_a = adc->gen_volt_phase_a;
        _gstStsInput.connected_volt_inst_b = adc->gen_volt_phase_b;
        _gstStsInput.connected_volt_inst_c = adc->gen_volt_phase_c;

        _gstStsInput.connected_theta = s_gen.volt_pll.theta_sin;

        _gstStsInput.connected_w0 = s_gen.freq_a * 2.0f * PH3_CALC_PI;

    }
    else
    {
        _gstStsInput.connected_volt_inst_a = adc->grid_volt_phase_a;
        _gstStsInput.connected_volt_inst_b = adc->grid_volt_phase_b;
        _gstStsInput.connected_volt_inst_c = adc->grid_volt_phase_c;

        _gstStsInput.connected_theta = s_grid.volt_pll.theta_sin;

        _gstStsInput.connected_w0 = s_grid.freq_a * 2.0f * PH3_CALC_PI;

    }

    _gstStsInput.inv_volt_inst_a = adc->inv_volt_phase_a;
    _gstStsInput.inv_volt_inst_b = adc->inv_volt_phase_b;
    _gstStsInput.inv_volt_inst_c = adc->inv_volt_phase_c;

    _gstStsInput.inv_theta = s_inv.volt_pll.theta_sin;

    _gstStsInput.inv_w0 = s_inv.freq_a * 2.0f * PH3_CALC_PI;

    Sts_Set_InputData(&_gstStsInput);
    _gstStsFlagUpdateFlag = 1;
}

void ph3Sts_Adjust_DataUpdate(void)
{

    //STS相位调整器运行
    _gstStsAdjustInput.inv_w0 = _gstStsInput.inv_w0;
    _gstStsAdjustInput.inv_theta = _gstStsInput.inv_theta;
    _gstStsAdjustInput.inv_volt_rms = (_gstStsInput.inv_volt_rms_a + _gstStsInput.inv_volt_rms_b +_gstStsInput.inv_volt_rms_c)/3;
    _gstStsAdjustInput.grid_w0 = _gstStsInput.connected_w0;
    _gstStsAdjustInput.grid_theta = _gstStsInput.connected_theta;
    _gstStsAdjustInput.grid_volt_rms = (_gstStsInput.connected_volt_rms_a + _gstStsInput.connected_volt_rms_b +_gstStsInput.connected_volt_rms_c)/3;
    gStsAdjust.state = Sts_Get_State();
    Sts_Adjust_SetInput(&_gstStsAdjustInput);
}

void Ph3_Sts_Init(void)
{
    Sts_Init();
    memset(&_gstStsInput, 0, sizeof(Sts_Input_t));
    memset(&_gstStsOutput, 0, sizeof(StsOutputStatus_t));
    Sts_Adjust_Init();
}

void Ph3_Sts_Run(void)
{
    if(_gstStsInputStatus.sts_run_en)
    {
        Sts_Set_EnConnect(_gstStsInputStatus.en_connect);
        Sts_Set_Warn_clear(_gstStsInputStatus.warn_clear);
        Sts_Set_Err_clear(_gstStsInputStatus.err_clear);
        Ph3_Sts_InstDateUpdate();
        Sts_Run();
        ph3Sts_Adjust_DataUpdate();
        Sts_Adjust_Run();
        gStsData.adjust_w = gStsAdjust.o_adj_w;
        gStsData.adjust_volt = gStsAdjust.o_adj_volt;
        Sts_Get_OutputData(&_gstStsOutput.sts_output);
    }
    else
    {
        Ph3_Sts_Init();
    }
}


```

---

## app/mid_vol_and_rcmu_rms.h

```c
/*
 * mid_vol_and_rcmu_rms.h
 *
 * Relay middle voltage (3-phase) and RCMU leak current RMS calculation on CPU2.
 */
#ifndef APP_MID_VOL_AND_RCMU_RMS_H_
#define APP_MID_VOL_AND_RCMU_RMS_H_

#include "F28x_Project.h"
#include "ph3_calc_ifc_update.h"
#include <stdbool.h>

/** Relay middle voltage + RCMU leak current RMS outputs */
typedef struct mid_vol_and_rcmu_rms_stru_t
{
    float32 mid_vol_a_rms;
    float32 mid_vol_b_rms;
    float32 mid_vol_c_rms;
    float32 rcmu_curr_rms;
} MidVolAndRcmuRms_t;

extern MidVolAndRcmuRms_t gstMidVolAndRcmuRms;

void MidVolAndRcmuRms_Init(void);
void MidVolAndRcmuRms_Run(float32 mid_vol_a, float32 mid_vol_b, float32 mid_vol_c,
                          float32 leak_curr, float32 freq);
void MidVolAndRcmuRms_Update(DualDataShare_Ph3Ifc_t *ph3_ifc);
#endif /* APP_MID_VOL_AND_RCMU_RMS_H_ */

```

---

## app/mid_vol_and_rcmu_rms.c

```c
/*
 * mid_vol_and_rcmu_rms.c
 *
 * Relay middle voltage (3-phase) and RCMU leak current RMS calculation on CPU2.
 */
#include "mid_vol_and_rcmu_rms.h"
#include "mid_filter.h"

#pragma CODE_SECTION(MidVolAndRcmuRms_Run, ".TI.ramfunc");
#pragma CODE_SECTION(MidVolAndRcmuRms_Update, ".TI.ramfunc");

MidVolAndRcmuRms_t gstMidVolAndRcmuRms;

static Mid_Filter1OrderButter_t _gLowfilter_1order;

#define LEAK_CURRENT_LPF_CUTOFF_HZ (500.0f)
#define LEAK_CURRENT_SAMPLE_FREQ_HZ (20000.0f)
static uint16_t s_cycle_sample_count = 0;
static float32 s_leak_curr_filter = 0.0f;
void MidVolAndRcmuRms_Init(void)
{
    s_cycle_sample_count = 0;
    gstMidVolAndRcmuRms.mid_vol_a_rms = 0.0f;
    gstMidVolAndRcmuRms.mid_vol_b_rms = 0.0f;
    gstMidVolAndRcmuRms.mid_vol_c_rms = 0.0f;
    gstMidVolAndRcmuRms.rcmu_curr_rms = 0.0f;
    Mid_Filter1OrderButter_Init(&_gLowfilter_1order, LEAK_CURRENT_LPF_CUTOFF_HZ, LEAK_CURRENT_SAMPLE_FREQ_HZ);
}

void MidVolAndRcmuRms_Run(float32 mid_vol_a, float32 mid_vol_b, float32 mid_vol_c,
                          float32 leak_curr, float32 freq)
{
    static uint16_t s_cycle_count = 0;
    static float32 leak_rms_sum = 0.0f;
    static float32 mid_vol_a_rms_sum = 0.0f;
    static float32 mid_vol_b_rms_sum = 0.0f;
    static float32 mid_vol_c_rms_sum = 0.0f;
    s_leak_curr_filter = Mid_Filter1OrderButter_Update(&_gLowfilter_1order, leak_curr);
    leak_rms_sum += s_leak_curr_filter * s_leak_curr_filter;
    mid_vol_a_rms_sum += mid_vol_a * mid_vol_a;
    mid_vol_b_rms_sum += mid_vol_b * mid_vol_b;
    mid_vol_c_rms_sum += mid_vol_c * mid_vol_c;
    s_cycle_count++;
    if (s_cycle_count >= s_cycle_sample_count)
    {
        gstMidVolAndRcmuRms.rcmu_curr_rms = leak_rms_sum / s_cycle_sample_count;
        gstMidVolAndRcmuRms.mid_vol_a_rms = mid_vol_a_rms_sum / s_cycle_sample_count;
        gstMidVolAndRcmuRms.mid_vol_b_rms = mid_vol_b_rms_sum / s_cycle_sample_count;
        gstMidVolAndRcmuRms.mid_vol_c_rms = mid_vol_c_rms_sum / s_cycle_sample_count;
        leak_rms_sum = 0.0f;
        mid_vol_a_rms_sum = 0.0f;
        mid_vol_b_rms_sum = 0.0f;
        mid_vol_c_rms_sum = 0.0f;
        s_cycle_count = 0;
        s_cycle_sample_count = LEAK_CURRENT_SAMPLE_FREQ_HZ / ((freq > 0) ? ((freq < 30) ? 30 : ((freq > 70) ? 70 : freq)) :50);
    }
}

void MidVolAndRcmuRms_Update(DualDataShare_Ph3Ifc_t *ph3_ifc)
{
    ph3_ifc->rcmu_curr_rms = gstMidVolAndRcmuRms.rcmu_curr_rms;
    ph3_ifc->mid_vol_a_rms = gstMidVolAndRcmuRms.mid_vol_a_rms;
    ph3_ifc->mid_vol_b_rms = gstMidVolAndRcmuRms.mid_vol_b_rms;
    ph3_ifc->mid_vol_c_rms = gstMidVolAndRcmuRms.mid_vol_c_rms;
}

```
