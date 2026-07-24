
#ifndef __INTERRUPT_H
#define __INTERRUPT_H
//#include "dev_sample.h"
//#include "bsp.h"
#include "mid_position_pid.h"
//#include "dev_high_low_side.h"
#include "mid_pv_logic_control.h"


extern SampleRawValue gSampleRawValue;

void ClearIrqElectricFlag (void);
bool GetIrqElectricFlag (void);
void SampleRawValueGet(void);
void LoopCtrlTimerInit(void);
void irq_param_init(void);
bool gout_over_voltage_get (void);
void gOutOVFlagClear (void);
void PidRunInit(void);

void gOutOcFlagClear (void);
bool gout_over_current_get (void);


void irq_mppt_output_limit(void);
void gInOVFlagClear (void);
bool gin_over_voltage_get (void);
#endif








