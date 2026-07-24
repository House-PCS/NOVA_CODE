/**
 * @file pwr_ctrl_interface.h
 * @brief 功率控制接口：输入采集与输出更新
 */

#ifndef PWR_CTRL_INTERFACE_H
#define PWR_CTRL_INTERFACE_H

#include "energy_management_def.h"

typedef struct pwr_ctrl_interface_stru_t
{
    float32 connect_active_pwr;
    float32 connect_reactive_pwr;
    float32 inv_active_pwr;
    float32 inv_reactive_pwr;

    float32 ramp_connect_active_pwr;
    float32 ramp_connect_reactive_pwr;
    float32 ramp_inv_active_pwr;
    float32 ramp_inv_reactive_pwr;

    float32 connect_a_active_pwr;
    float32 connect_a_reactive_pwr;
    float32 inv_a_active_pwr;
    float32 inv_a_reactive_pwr;

    float32 ramp_a_connect_active_pwr;
    float32 ramp_a_connect_reactive_pwr;
    float32 ramp_a_inv_active_pwr;
    float32 ramp_a_inv_reactive_pwr;

    float32 connect_b_active_pwr;
    float32 connect_b_reactive_pwr;
    float32 inv_b_active_pwr;
    float32 inv_b_reactive_pwr;

    float32 ramp_b_connect_active_pwr;
    float32 ramp_b_connect_reactive_pwr;
    float32 ramp_b_inv_active_pwr;
    float32 ramp_b_inv_reactive_pwr;

    float32 connect_c_active_pwr;
    float32 connect_c_reactive_pwr;
    float32 inv_c_active_pwr;
    float32 inv_c_reactive_pwr;

    float32 ramp_c_connect_active_pwr;
    float32 ramp_c_connect_reactive_pwr;
    float32 ramp_c_inv_active_pwr;
    float32 ramp_c_inv_reactive_pwr;

    float32 off_grid_inv_ref;
    float32 pv_pwr;
} PwrCtrlInteface_t;

extern PwrCtrlInteface_t gstPwrCtrlInterface;
void EnergyManagementDataUpdate(void);

uint16_t GetDrmGridConnectEN(void);
uint16_t GetConnectTimeEN(void);

#endif // PWR_CTRL_INTERFACE_H
