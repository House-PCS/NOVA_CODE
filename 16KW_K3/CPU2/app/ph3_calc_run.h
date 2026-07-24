/*
 * ph3_calc_run.h
 *
 * CPU2 three-phase calculation runtime.
 */
#ifndef APP_PH3_CALC_RUN_H_
#define APP_PH3_CALC_RUN_H_

#include "ph3_calc/ph3_calc_channel.h"

extern Ph3Calc_Channel_t s_grid;
extern Ph3Calc_Channel_t s_gen;
extern Ph3Calc_Channel_t s_load;
extern Ph3Calc_Channel_t s_inv;
extern Ph3Calc_Channel_t s_ct;

void Ph3CalcRun_Init(void);
void Ph3CalcRun_Run(void);
void Ph3CalcRun_FlagUpdate(void);
void Ph3CalcRun_DataUpdate(void);

#endif /* APP_PH3_CALC_RUN_H_ */
