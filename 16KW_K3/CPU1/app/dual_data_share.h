/*
 * dual_data_share.h
 *
 * CPU1/CPU2 shared data layout (ADC result + ph3 run flags).
 */
#ifndef APP_DUAL_DATA_SHARE_H_
#define APP_DUAL_DATA_SHARE_H_

#include "F28x_Project.h"
#include <stdint.h>

#define CPU_ADC_PH3_RUN_DISABLE           0U
#define CPU_ADC_PH3_RUN_ENABLE            1U



typedef struct cpu_adc_ph3_run_flag_stru_t
{
    volatile uint16_t enable_volt_pll;
    volatile uint16_t enable_curr_pll;
    volatile uint16_t enable_power_calc;
} CpuAdcPh3RunFlag_t;

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

volatile DevDct_Cpu1ToCpu2Msg_t *DualDataShare_GetCpu1ToCpu2Msg(void);

void DualDataShare_Init(void);
void DualDataShare_UpdatePh3Cmd(void);

#endif /* APP_DUAL_DATA_SHARE_H_ */
