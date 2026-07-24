/*
 * sample_update.h
 *
 *  Created on: Oct 13, 2025
 *      Author: Nova006348
 */

#ifndef MID_ADC_H_
#define MID_ADC_H_

#include "F28x_Project.h"
#include "debug_analysis.h"
#include "fault.h"
#include "system.h"

#define ADC_ZERO_MAX        (2100)          /* ADC zero calibration upper limit */
#define ADC_ZERO_MIN        (2000)          /* ADC zero calibration lower limit */
#define ADC_ZERO_ERR_MAX        (2078)          /* ADC zero error detection upper limit */
#define ADC_ZERO_ERR_MIN        (2018)          /* ADC zero error detection lower limit */

#define VALUE_LIMIT(x,max,min)      ((x>max)?max:((x<min)?min:x))

typedef struct adc_calibration_stru_t
{
    uint16_t calibration_time0_finish;           /* ADC Time0 zero calibration finish flag */
    uint16_t calibration_time1_finish;           /* ADC Time1 zero calibration finish flag */
    uint16_t calibration_time2_finish;           /* ADC Time2 zero calibration finish flag */
    uint16_t calibration_time3_finish;           /* ADC Time3 zero calibration finish flag */
    uint16_t calibration_time4_finish;           /* ADC Time4 zero calibration finish flag */

    float32 common_calibration_zero;             /* Common zero reference (1.5V bias) */
    /* Inverter side */
    float32 inv_volt_phase_a_zero;               /* Phase A inverter voltage zero offset */
    float32 inv_volt_phase_b_zero;               /* Phase B inverter voltage zero offset */
    float32 inv_volt_phase_c_zero;               /* Phase C inverter voltage zero offset */
    float32 inv_curr_phase_a_zero;               /* Phase A inverter current zero offset */
    float32 inv_curr_phase_b_zero;               /* Phase B inverter current zero offset */
    float32 inv_curr_phase_c_zero;               /* Phase C inverter current zero offset */
    float32 leak_curr_zero;                      /* Leakage current zero offset */

    float32 relay_middle_volt_phase_a_zero;      /* Relay midpoint voltage phase A zero offset */
    float32 relay_middle_volt_phase_b_zero;      /* Relay midpoint voltage phase B zero offset */
    float32 relay_middle_volt_phase_c_zero;      /* Relay midpoint voltage phase C zero offset */
    /* Grid side */
    float32 grid_volt_phase_a_zero;              /* Phase A grid voltage zero offset */
    float32 grid_volt_phase_b_zero;              /* Phase B grid voltage zero offset */
    float32 grid_volt_phase_c_zero;              /* Phase C grid voltage zero offset */
    float32 grid_curr_phase_a_zero;              /* Phase A grid current zero offset */
    float32 grid_curr_phase_b_zero;              /* Phase B grid current zero offset */
    float32 grid_curr_phase_c_zero;              /* Phase C grid current zero offset */
    float32 ct_a_curr_zero;                      /* CT phase A current zero offset */
    float32 ct_b_curr_zero;                      /* CT phase B current zero offset */
    float32 ct_c_curr_zero;                      /* CT phase C current zero offset */
    /* Load side */
    float32 load_volt_phase_a_zero;              /* Phase A load voltage zero offset */
    float32 load_volt_phase_b_zero;              /* Phase B load voltage zero offset */
    float32 load_volt_phase_c_zero;              /* Phase C load voltage zero offset */
    float32 load_curr_phase_a_zero;             /* Phase A load current zero offset */
    float32 load_curr_phase_b_zero;             /* Phase B load current zero offset */
    float32 load_curr_phase_c_zero;             /* Phase C load current zero offset */
    /* PV side */
    float32 pv1_volt_zero;                       /* PV1 voltage zero offset */
    float32 pv1_curr_zero;                       /* PV1 current zero offset */
    float32 pv2_volt_zero;                       /* PV2 voltage zero offset */
    float32 pv2_curr_zero;                       /* PV2 current zero offset */
    float32 pv3_volt_zero;                       /* PV3 voltage zero offset */
    float32 pv3_curr_zero;                       /* PV3 current zero offset */
    float32 pv4_volt_zero;                       /* PV4 voltage zero offset */
    float32 pv4_curr_zero;                       /* PV4 current zero offset */
    /* BUS side */
    float32 bus_volt_pos_zero;                   /* Positive bus voltage zero offset */
    float32 bus_volt_neg_zero;                   /* Negative bus voltage zero offset */
    float32 bus_curr_zero;                       /* Bus current zero offset */
    /* Generator side */
    float32 gen_volt_phase_a_zero;               /* Phase A generator voltage zero offset */
    float32 gen_volt_phase_b_zero;               /* Phase B generator voltage zero offset */
    float32 gen_volt_phase_c_zero;               /* Phase C generator voltage zero offset */
    float32 gen_curr_phase_a_zero;               /* Phase A generator current zero offset */
    float32 gen_curr_phase_b_zero;               /* Phase B generator current zero offset */
    float32 gen_curr_phase_c_zero;               /* Phase C generator current zero offset */
} CpuAdcCalibration_t;

/** CPU1 GS3 ADC result */
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

/** CPU1 GS3 ADC result */
typedef struct cla_adc_stru_t
{
    float32 inv_volt_phase_a;
    float32 inv_volt_phase_b;
    float32 inv_volt_phase_c;
    float32 inv_curr_phase_a;
    float32 inv_curr_phase_b;
    float32 inv_curr_phase_c;

    float32 bus_volt_pos;
    float32 bus_volt_neg;
} CLAAdc_t;

extern CpuAdc_t  gstAdcResult;              /* ADC sampled physical values */
extern CpuAdc_t  gstAdcCoeff;               /* ADC conversion coefficients */
extern CpuAdcCalibration_t  gstAdcZero;     /* ADC zero calibration offsets */

void Adc_Init(void);
void GetAdcResult(EN_CPU_INTERRUPT_TIME value);
uint16_t Get_Adc_Time0_Calibration_State(void);
uint16_t Get_Adc_Time1_Calibration_State(void);
uint16_t Get_Adc_Time2_Calibration_State(void);
uint16_t Get_Adc_Time3_Calibration_State(void);
uint16_t Get_Adc_Time4_Calibration_State(void);
void TempVolUpdate(void);
#endif /* __ADC_MIDDLE_LAYER_H__ */
