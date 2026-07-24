#ifndef INVERTER_FUNC_DEF_H
#define INVERTER_FUNC_DEF_H
#include "F2837xD_device.h"

extern uint32_t gReleaseCount; 

extern float32 gDutyCom;
void InvStateClosedFunc();
void InvStateReleaseFunc();
void InvStateOpeningFunc();
void InvStateDisConnectedFunc();
void InvStateConnectingFunc();
void InvStateConnectedFunc();
void InvStateDisConnectingFunc();
uint16_t GetInvRunstate(void);

#endif
