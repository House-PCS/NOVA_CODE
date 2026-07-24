/**
 * @file biz_fsm_func.h
 * @brief Periodic business state handler functions
 */

#ifndef BIZ_FSM_FUNC_H
#define BIZ_FSM_FUNC_H

void BizStateIdle_Function(void* context);
void BizStateSelfTest_Function(void* context);
void BizStateReady_Function(void* context);
void BizStateGenStart_Function(void* context);
void BizStatePvStart_Function(void* context);
void BizStateNomalStart_Function(void* context);
void BizStateNomalRun_Function(void* context);
void BizStateTestMode_Function(void* context);
void BizStateFault_Function(void* context);
void BizStateShutDown_Function(void* context);
void BizStateBoot_Function(void* context);
void BizStateInitParam_Function(void* context);
void BizStatePowerOff_Function(void* context);

void BizFsm_Ready_BusVoltDlyEntryReset(void);
void BizFsm_NomalRun_OnExit(void* context);
void BizFsm_TestMode_OnExit(void* context);

#endif /* BIZ_FSM_FUNC_H */
