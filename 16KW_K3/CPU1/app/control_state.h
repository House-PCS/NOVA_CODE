/*
 * control_state.h
 *
 *  Created on:
 *      Author: Nova007456
 */

#ifndef APP_CONTROL_STATE_H_
#define APP_CONTROL_STATE_H_

#include "F28x_Project.h"
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_interface.h"

#define PHASE_CNT_TO_THETA_RAD(cnt, freq_hz) \
    (((freq_hz) > 1.0e-3f) ? ((cnt) * (freq_hz) * (6.28318530717958647692f / 20000.0f)) : (0.0f))

typedef struct inv_action_permit_stru_t
{

    uint16_t pv_bus_permit;

    uint16_t inv_bus_disconnect_permit;
    uint16_t inv_bus_connect_permit;

    uint16_t grid_phase_chk_permit;
    uint16_t gen_phase_chk_permit;

    uint16_t monitor_inv_disconnect_permit;
    uint16_t monitor_inv_connect_permit;

    uint16_t force_gen_connect_permit;

    uint16_t monitor_pv_permit;


    uint16_t parallel_disconnect_permit;
    uint16_t parallel_connect_permit;

    uint16_t inv_inside_err_disconnect_permit;
    uint16_t inv_inside_err_connect_permit;
    uint16_t pv_inside_err_connect_permit;


    uint16_t cpu2_run_permit;

    uint16_t sts_inv_connecting_permit;
    uint16_t sts_run_connected_permit;
    uint16_t sts_err_connected_permit;
    uint16_t sts_inside_connected_permit;

    uint16_t mointer_connect_type_chk;

    uint16_t connect_delay_permit;
    uint16_t connect_time_permit;
    uint16_t grid_drm_permit;
} ActionPermit_t;

typedef enum
{
    INV_STATE_STANDBY,              /* Inverter standby state */
    INV_STATE_SELFTEST,             /* Inverter self-test state */
    INV_STATE_OPENING,              /* Inverter open-loop state */
    INV_STATE_DISCONNECTED,         /* Inverter closed-loop (off-grid) state */
    INV_STATE_GRID_CONNECTING,      /* Grid connecting state */
    INV_STATE_GEN_CONNECTING,       /* Generator connecting state */
    INV_STATE_GRID_CONNECTED,       /* Grid connected state */
    INV_STATE_GEN_CONNECTED,        /* Generator connected state */
    INV_STATE_GRID_DISCONNECTING,   /* Grid disconnecting state */
    INV_STATE_GEN_DISCONNECTING,    /* Generator disconnecting state */
    INV_REMAIN = 0xFFFFFFFF,
} EN_INV_STATE;

extern volatile EN_INV_STATE gstPcsInvRunState;

void Inv_State_Int1(void);
void InvWorkPermit_Monitor(void);
void BackUpSwitchCtrlFunc(void);
void ControlStateInit(void);
void PcsStatePermit(void);
uint16_t WorkPermit_GenForceEn(void);
#endif /* APP_CONTROL_STATE_H_ */
