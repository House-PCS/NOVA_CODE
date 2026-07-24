/**
 * @file temp_pwr_derate.h
 * @brief 温度降额：光伏 / 逆变 / 控制板功率限制
 */

#ifndef TEMP_PWR_DERATE_H
#define TEMP_PWR_DERATE_H

#include "energy_management_def.h"

typedef struct temp_pwr_derate_output_stru_t
{
    float32 pv_pwr;                       /**< 光伏功率限制 (W) */
    float32 inv_active_pwr;               /**< 逆变有功降额限制 (W)，并网/离网 */
    float32 disconnect_volt_ref;          /**< 离网逆变电压峰值参考 (V) */
} TempPwrDerate_Output_t;

extern TempPwrDerate_Output_t gstTempPwrDerateOut;

void TempPwrDerate_Init(void);
void TempPwrDerate_Update(void);

#endif // TEMP_PWR_DERATE_H
