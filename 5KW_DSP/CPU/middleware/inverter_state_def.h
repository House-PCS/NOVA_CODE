#ifndef INV_STATE_DEF_H
#define INV_STATE_DEF_H
#include "FSM.h"

typedef enum
{
    EN_INV_STATE_DEF_ERR,
    EN_INV_STATE_DEF_CLOSE,
    EN_INV_STATE_DEF_RELEASE,
    EN_INV_STATE_DEF_OPENING,
    EN_INV_STATE_DEF_DISCONNECTED,
    EN_INV_STATE_DEF_CONNECTING,
    EN_INV_STATE_DEF_CONNECTED,
    EN_INV_STATE_DEF_DISCONNECTING,
    EN_INV_STATE_DEF_DEF = 0xFFFFFFFF,
} EN_INV_STATE_DEF;

extern FsmState_t gStateClose;
extern FsmState_t gStateRelease;
extern FsmState_t gStateOpening;
extern FsmState_t gStateDisConnected;
extern FsmState_t gStateConnecting;
extern FsmState_t gStateConnected;
extern FsmState_t gStateDisConnecting;
extern uint16_t gErrResetFlag;

#endif
