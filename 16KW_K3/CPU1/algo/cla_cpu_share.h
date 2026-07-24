/*
 * cla_cpu_share.h
 *
 *  Created on: 2025-03-25
 *      Author: Nova007456
 */
#ifndef APP_CLA_CPU_SHARE_H_
#define APP_CLA_CPU_SHARE_H_

#include "F28x_Project.h"
#include "cap.h"
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_def.h"
typedef struct cpu_transfor_cla_data_t
{
    float32 inv_vol_a;
    float32 inv_vol_b;
    float32 inv_vol_c;

    float32 inv_curr_L1_a;
    float32 inv_curr_L1_b;
    float32 inv_curr_L1_c;

    float32 inv_curr_L2_a;
    float32 inv_curr_L2_b;
    float32 inv_curr_L2_c;

    float32 bus_vol;
    float32 bus_vol_pos;
    float32 bus_vol_neg;

    float32 theta;
    float32 theta_offset;

    float32 w0;
    float32 w_offset;

    float32 vol_amp_ref;
    float32 vol_amp_ref_offset;

    EN_INV3PH_STATE ClaInv3PhState;

} CpuTransforClaData_t;

extern volatile CpuTransforClaData_t   gstCpuTransforClaData;
extern volatile float garrDebugDataCla[16];

#endif /* APP_CLA_CPU_SHARE_H_ */
