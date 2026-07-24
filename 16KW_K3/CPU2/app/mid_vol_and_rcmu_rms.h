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
