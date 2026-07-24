/*
 * electric_calculate.h
 *
 *  Created on: Nov 19, 2025
 *      Author: Nova006348
 */

#ifndef APP_ELECTRIC_CALCULATE_H_
#define APP_ELECTRIC_CALCULATE_H_

#include "F28x_Project.h"

#define NON_NEGATIVE(x) (((x) > 0) ? (x) : 0)

/* Actual sample period is 100ms in electric_calculate.c (600 samples = 60s) */
#define SAMPLE_INTERVAL_MS   100 * 20
#define SAMPLES_PER_MINUTE   600

/* Per-channel energy accumulator */
typedef struct {
    float power_accumulate_ws;      /* Running sum (W??s) */
    float power_minute_ws;          /* Completed 60s window sum (W??s) */
    uint32_t total_01kwh;           /* Total energy in 0.1 kWh units */
    float remainder_wh;             /* Fractional remainder (Wh) */

    uint16_t sample_count;          /* Sample count in current window */
    bool calc_active_flag;          /* Set when a window is ready to convert */
} ElectricCalculate_t;


typedef struct {

    ElectricCalculate_t pv_energr_meter[3];
    ElectricCalculate_t buy_electric_meter;
    ElectricCalculate_t sell_electric_meter;
    ElectricCalculate_t bat_chg_electric_meter;
    ElectricCalculate_t bat_dsg_electric_meter;
    ElectricCalculate_t eps_electric_meter;
    ElectricCalculate_t remote_load_electric_meter;
    ElectricCalculate_t gen_electric_meter;
    ElectricCalculate_t ev_chg_electric_meter;
    ElectricCalculate_t ev_dsg_electric_meter;
}SysElectricMeter_t;

typedef struct {

    float pv1_power;
    float pv2_power;
    float pv3_power;
    float gird_active_power;
    float ct_active_power;
    float bat_power;
    float eps_active_power;
    float gen_active_power;
    float ev_power;
}ElectricMeterInValue_t;

void ElectricPowerAccumulate(void);
ElectricMeterInValue_t* GetElectricPowerInfo(void);

/* Energy readback (0.1 kWh units) */
uint16_t GetPvTotalElectric(void);
uint16_t GetPv1Electric(void);
uint16_t GetPv2Electric(void);
uint16_t GetPv3Electric(void);
uint16_t GetBuyElectric(void);
uint16_t GetSellElectric(void);
uint16_t GetBatChgElectric(void);
uint16_t GetBatDsgElectric(void);
uint16_t GetEpsElectric(void);
uint16_t GetGenElectric(void);
uint16_t GetRemoteLoadElectric(void);
uint16_t GetEvChgElectric(void);
uint16_t GetEvDsgElectric(void);
void ElectricParamClear(uint16_t *flag);
#endif /* APP_ELECTRIC_CALCULATE_H_ */
