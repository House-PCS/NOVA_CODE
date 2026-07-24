/*
 * ph3_calc_posneg.h
 *
 * 三相正负序分离（Clarke + 90° 带通 + 正负序分解 + Park）
 */
#ifndef ALGO_PH3_CALC_POSNEG_H_
#define ALGO_PH3_CALC_POSNEG_H_

#include "ph3_calc_def.h"

void Ph3Calc_PosNegInit(Ph3Calc_PosNegData_t* data);
void Ph3Calc_PosNegRun(Ph3Calc_PosNegData_t* data,
                       float32 va, float32 vb, float32 vc,
                       float32 theta);

#endif /* ALGO_PH3_CALC_POSNEG_H_ */
