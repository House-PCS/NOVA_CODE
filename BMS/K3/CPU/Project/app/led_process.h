
#ifndef __LED_PROCESS_H_
#define __LED_PROCESS_H_


#include "bsp.h"
#include "dev_led.h"
void EXPMcuLedProcess(void);
void LedAppFun(void);
void LedInit(void);
void TurnOffLedProcess(void);
void TurnOnLedProcess(void);
void HostMcuLedProcess(void);
void TestLedProcess(void);
void _SystemErrorLedProcess(void);
void _DSGLedProcess(float PowerData1);
void _CHGLedProcess(float PowerData2);
void _MpptErrorAlarmLedProcess(void);
void _ExpMosErrorLedProcess(void);
void _ExpCurrentErrorLedProcess(void);
void _ExpCellErrorLedProcess(void);
void _ExpSampleErrorLedProcess(void);
void _ExpOtherErrorLedProcess(void);
void SetAtmosLed(EN_LED_STATUS status, uint8_t brightness, uint32_t time, uint8_t r_data, uint8_t g_data, uint8_t b_data, uint8_t num);
void DSGAtmosLampSet(uint8_t brightness, uint8_t r_data, uint8_t g_data, uint8_t b_data, uint8_t num);
void CHGAtmosLampSet(uint8_t brightness, uint8_t r_data, uint8_t g_data, uint8_t b_data, uint8_t num);
void TurnOnAtmosLampSet(uint8_t brightness, uint8_t temp);
void TurnOffAtmosLampSet(uint8_t brightness);
void ResetAtmosLedBlinkTime(uint8_t num);
void AtRunLedLampSet(EN_LED_STATUS status, uint8_t brightness);
void LedProcess(void);
extern uint8_t gerrorled;
#endif
