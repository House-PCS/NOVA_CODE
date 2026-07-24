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
