/*
 * dual_data_share.c
 *
 * CPU1/CPU2 shared data accessors (CPU1 side).
 */
#include "dual_data_share.h"
#include "sample_update.h"
#include "ph3_sts_ifc_update.h"

static void DualDataShare_SetPh3RunFlag(volatile CpuAdcPh3RunFlag_t *flag, uint16_t en)
{
    flag->enable_volt_pll   = en;
    flag->enable_curr_pll   = en;
    flag->enable_power_calc = en;
}

volatile DevDct_Cpu1ToCpu2Msg_t *DualDataShare_GetCpu1ToCpu2Msg(void)
{
    return (volatile DevDct_Cpu1ToCpu2Msg_t *)DUAL_DATA_SHARE_CPU1_TO_CPU2_ADDR;
}

void DualDataShare_Init(void)
{
    volatile DevDct_Cpu1ToCpu2Msg_t *msg = DualDataShare_GetCpu1ToCpu2Msg();

    msg->cpu1_adc_result_addr = (uintptr_t)&gstAdcResult;
    DualDataShare_SetPh3RunFlag(&msg->grid, CPU_ADC_PH3_RUN_DISABLE);
    DualDataShare_SetPh3RunFlag(&msg->gen, CPU_ADC_PH3_RUN_DISABLE);
    DualDataShare_SetPh3RunFlag(&msg->load, CPU_ADC_PH3_RUN_DISABLE);
    DualDataShare_SetPh3RunFlag(&msg->inv, CPU_ADC_PH3_RUN_DISABLE);
    DualDataShare_SetPh3RunFlag(&msg->ct, CPU_ADC_PH3_RUN_DISABLE);
    msg->sts_input_addr = (uintptr_t)StsEnUpdate_Get();
}

void DualDataShare_UpdatePh3Cmd(void)
{
    volatile DevDct_Cpu1ToCpu2Msg_t *msg = DualDataShare_GetCpu1ToCpu2Msg();

    DualDataShare_SetPh3RunFlag(&msg->grid, CPU_ADC_PH3_RUN_ENABLE);
    DualDataShare_SetPh3RunFlag(&msg->gen, CPU_ADC_PH3_RUN_ENABLE);
    DualDataShare_SetPh3RunFlag(&msg->load, CPU_ADC_PH3_RUN_ENABLE);
    DualDataShare_SetPh3RunFlag(&msg->inv, CPU_ADC_PH3_RUN_ENABLE);
    DualDataShare_SetPh3RunFlag(&msg->ct, CPU_ADC_PH3_RUN_ENABLE);
}
