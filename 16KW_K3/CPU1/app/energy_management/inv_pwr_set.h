/**
 * @file inv_pwr_set.h
 * @brief 逆变功率设定：斜坡、限幅与并网 P/Q 输出
 */

#ifndef INV_PWR_SET_H
#define INV_PWR_SET_H

#include "energy_management_def.h"


typedef struct inv_pwr_set_output_stru_t
{
    float32 p_power;                  /**< 限幅后逆变有功设定 (W) */
    float32 q_power;                  /**< 限幅后逆变无功设定 (VAR) */
} InvPwrSet_Output_t;

extern InvPwrSet_Output_t gstInvPwrSetOut;
extern InvPwrSet_Output_t gstGridPwrSetOut;
void InvPwrSet_Init(void);
void InvPwrSet_Update(float32 inv_active_pwr, float32 inv_reactive_pwr,\
                      float32 connect_active_pwr, float32 connect_reactive_pwr);

#endif // INV_PWR_SET_H
