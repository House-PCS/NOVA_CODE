/**
 * @file biz_fsm_interface.h
 * @brief Sole interaction interface between biz_fsm and external modules
 * @details External modules shall access biz_fsm only through this header.
 *          Internal biz_fsm modules shall access external data/functions only through this header.
 */

#ifndef BIZ_FSM_INTERFACE_H
#define BIZ_FSM_INTERFACE_H

#include "biz_fsm_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Module lifecycle (called from external main loop)                          */
/* -------------------------------------------------------------------------- */

void BizFsm_Interface_Init(void);
int BizFsm_Interface_Update(void* context);

/* -------------------------------------------------------------------------- */
/* External -> biz_fsm: command writes (comm layer / monitor card, etc.)      */
/* -------------------------------------------------------------------------- */

void BizFsm_Interface_SetUpgradeCmd(uint8_t cmd);
void BizFsm_Interface_SetPowerOnReset(uint8_t cmd);
void BizFsm_Interface_SetIsErr(uint8_t err);
void BizFsm_Interface_SetFaultClass(EN_RUN_FAULT_CLASS fault_class);
void BizFsm_Interface_SetFaultCleared(uint8_t cleared);

/* -------------------------------------------------------------------------- */
/* biz_fsm -> external: status reads (for external module queries)            */
/* -------------------------------------------------------------------------- */

EN_RUN_STATE BizFsm_Interface_GetRunState(void);
EN_RUN_FAULT_CLASS BizFsm_Interface_GetFaultClass(void);
uint8_t BizFsm_Interface_GetPowerOnCmd(void);

/** @brief Operation permission flags (1 = allowed, 0 = not allowed) */
uint8_t BizFsm_Interface_GetAllowOffgrid(void);      /**< Off-grid inverter */
uint8_t BizFsm_Interface_GetAllowGridConnect(void);  /**< Grid-tied inverter */
uint8_t BizFsm_Interface_GetAllowPv(void);           /**< PV boost / MPPT */

/* -------------------------------------------------------------------------- */
/* Internal use: read external module data / call external functions          */
/* -------------------------------------------------------------------------- */

void BizFsm_Interface_ExtSelfTestRun(void);
uint8_t BizFsm_Interface_ExtGetSelfTestPass(void);
uint8_t BizFsm_Interface_ExtGetSelfTestTimeout(void);
uint8_t BizFsm_Interface_ExtGetPcsLLCEnable(void);
float BizFsm_Interface_ExtGetBusVoltNeg(void);
float BizFsm_Interface_ExtGetBusVoltPos(void);

#ifdef __cplusplus
}
#endif

#endif /* BIZ_FSM_INTERFACE_H */
