#ifndef LLC_DCDC_CNTL_H
#define LLC_DCDC_CNTL_H

#define LLC_MAX_PWM 0.5

#pragma CODE_SECTION(LlcLoopCntl,".TI.ramfunc");
#pragma CODE_SECTION(LlcSendStateWave,".TI.ramfunc");
void LlcSendStateWave();
void LlcLoopCntl(void);
void LlcSetMaxPwm(float limit);
float LlcGetMaxPwm();
void LlcCntlInit();
void* LlcGetStateWave();
void LlcRunStateSet(uint16_t llcrunstate);

extern uint8_t gIsLlcStartupOk;

typedef enum
{
    EN_LLC_STATE_DEF_READY,
    EN_LLC_STATE_DEF_SOFTSTART_PV,
    EN_LLC_STATE_DEF_SOFTSTARTOK_PV,
    EN_LLC_STATE_DEF_SOFTSTART_BAT,
    EN_LLC_STATE_DEF_NORMAL,
} EN_LLC_STATE_DEF;
EN_LLC_STATE_DEF LlcRunStateRead();
#define LLC_READY 3
#define LLC_SOFTSTART 2
#define LLC_NORMAL 1
#endif
