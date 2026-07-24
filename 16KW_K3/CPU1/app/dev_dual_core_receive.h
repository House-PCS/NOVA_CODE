/*
 * dev_dual_core_receive.h
 *
 * CPU1 receive CPU2 shared RAM data (GS3/GS4).
 */
#ifndef APP_DEV_DUAL_CORE_RECEIVE_H_
#define APP_DEV_DUAL_CORE_RECEIVE_H_

#include <stdbool.h>
#include <stdint.h>
#include "F28x_Project.h"
#include "dual_data_share.h"
#include "AlgoFw/package/cntl/sts/sts_interface.h"

/** 三相电压接口量 */
typedef struct ph3_calc_ifc_vol_stru_t
{
    float32 a_phase_instant;
    float32 b_phase_instant;
    float32 c_phase_instant;
    float32 a_phase_rms;
    float32 b_phase_rms;
    float32 c_phase_rms;
    float32 freq_a;
    float32 freq_b;
    float32 freq_c;
    float32 theta;
} Ph3Calc_IfcVol_t;

/** 三相电流接口量 */
typedef struct ph3_calc_ifc_curr_stru_t
{
    float32 a_phase_instant;
    float32 b_phase_instant;
    float32 c_phase_instant;
    float32 a_phase_rms;
    float32 b_phase_rms;
    float32 c_phase_rms;
    float32 freq_a;
    float32 freq_b;
    float32 freq_c;
    float32 theta;
} Ph3Calc_IfcCurr_t;

/** 三相功率接口量 */
typedef struct ph3_calc_ifc_power_stru_t
{
    float32 a_phase_active;
    float32 a_phase_reactive;
    float32 a_phase_apperant;

    float32 b_phase_active;
    float32 b_phase_reactive;
    float32 b_phase_apperant;

    float32 c_phase_active;
    float32 c_phase_reactive;
    float32 c_phase_apperant;

    float32 total_active;
    float32 total_reactive;
    float32 total_apperant;

    float32 power_factor;
} Ph3Calc_IfcPower_t;

/** 正负序分离接口量 */
typedef struct ph3_calc_ifc_posneg_stru_t
{
    float32 alfa_pos;
    float32 beta_pos;
    float32 alfa_neg;
    float32 beta_neg;
    float32 d_pos;
    float32 q_pos;
    float32 d_neg;
    float32 q_neg;
} Ph3Calc_IfcPosNeg_t;

/** 单通道三相电量对外接口 */
typedef struct ph3_calc_interface_stru_t
{
    Ph3Calc_IfcVol_t vol;
    Ph3Calc_IfcCurr_t curr;
    Ph3Calc_IfcPower_t power;
    float32 vol_a_phase_count;
    float32 vol_b_phase_count;
    float32 vol_c_phase_count;
    Ph3Calc_IfcPosNeg_t volt_posneg;
    Ph3Calc_IfcPosNeg_t curr_posneg;
} Ph3Calc_Interface_t;

/** 母线滤波量 */
typedef struct ph3_calc_ifc_bus_stru_t
{
    float32 vol_filter;
    float32 curr_filter;
    float32 power_filter;
    float32 pos_vol_filter;
    float32 neg_vol_filter;
} Ph3CalcIfcBus_t;

/** CPU2 ph3 interface: grid / gen / load / inv / ct */
typedef struct dual_data_share_ph3_ifc_stru_t
{
    Ph3Calc_Interface_t grid;
    Ph3Calc_Interface_t gen;
    Ph3Calc_Interface_t load;
    Ph3Calc_Interface_t inv;
    Ph3Calc_Interface_t ct;

    Ph3CalcIfcBus_t bus;
    float32 rcmu_curr_rms;
    float32 mid_vol_a_rms;
    float32 mid_vol_b_rms;
    float32 mid_vol_c_rms;
} DualDataShare_Ph3Ifc_t;

typedef struct sts_output_status_stru_t
{
    Sts_Output_t sts_output;
    Sts_Err_Un_t sts_err;
    Sts_Warn_Un_t sts_warn;
    EN_STS_STATE  sts_state;
    uint8_t  sts_is_disconnectd;
    uint8_t  sts_is_disconnecting;
    uint8_t  sts_is_connecting;
    uint8_t  sts_is_connected;
    uint8_t  sts_is_gen_mode;
} StsOutputStatus_t;


/** CPU2 -> CPU1 message at CPU2TOCPU1RAM (0x3F800) */
typedef struct
{
    volatile uintptr_t cpu2_ph3_ifc_addr;     /* DualDataShare_Ph3Ifc_t address */
    volatile uint32_t isr_run_cnt;            /* CPU2 IPC ISR runtime in CpuTimer1 ticks */
    volatile uint32_t heartbeat_cnt;          /* CPU2 ph3 run heartbeat for CPU1 */
    volatile uint32_t isr_idle_cnt;           /* CPU2 IPC ISR idle time in CpuTimer1 ticks */
    volatile uintptr_t sts_output_addr;         /* Sts_Output_t address */
    volatile uintptr_t cpu2_debug_data_addr;    /* CPU2 debug data address */
} DevDct_Cpu2ToCpu1Msg_t;

#define DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR   0x0003F800UL

/** CPU2->CPU1 message mapped to CPU2TOCPU1RAM, no linker section required */
#define DualDataShare_GetCpu2ToCpu1Msg() \
    ((volatile DevDct_Cpu2ToCpu1Msg_t *)(DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR))

#define gstDevDctCpu2ToCpu1Msg  (*DualDataShare_GetCpu2ToCpu1Msg())

#define DEV_DCR_CPU2_TO_CPU1_ADDR         DUAL_DATA_SHARE_CPU2_TO_CPU1_ADDR
#define DEV_DCR_CPU1_TO_CPU2_ADDR         DUAL_DATA_SHARE_CPU1_TO_CPU2_ADDR
#define DEV_DCR_ADDR_EXCHANGE_TIMEOUT_MS  100U

#define DevDcr_GetCpu1ToCpu2Msg()         DualDataShare_GetCpu1ToCpu2Msg()
#define DevDcr_GetCpu2ToCpu1Msg()         DualDataShare_GetCpu2ToCpu1Msg()

bool DevDcr_ExchangeAddress(uint32_t timeoutMs);
void DevDcr_ResetPh3Ifc(void);
volatile DualDataShare_Ph3Ifc_t *DevDcr_GetPh3Ifc(void);
volatile DevDct_Cpu2ToCpu1Msg_t *DevDcr_GetCpu2ToCpu1MsgPtr(void);
volatile StsOutputStatus_t *DevDcr_GetStsOutput(void);
uint32_t DevDcr_GetCpu2Heartbeat(void);
uint32_t DevDcr_GetCpu2IsrRunCnt(void);
uint32_t DevDcr_GetCpu2IsrIdleCnt(void);
volatile float32 *DevDcr_GetCpu2DebugData(void);
#endif /* APP_DEV_DUAL_CORE_RECEIVE_H_ */
