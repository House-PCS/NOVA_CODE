/*
 * ph3_calc_rms.h
 *
 * 三相有效值计算
 */
#ifndef ALGO_PH3_CALC_RMS_H_
#define ALGO_PH3_CALC_RMS_H_

#include "ph3_calc_def.h"

void Ph3Calc_RmsStateInit(Ph3Calc_RmsState_t* state);
void Ph3Calc_RmsStateRun(Ph3Calc_RmsState_t* rms_state,
                         Ph3Calc_Phase_t* phase);

#endif /* ALGO_PH3_CALC_RMS_H_ */
