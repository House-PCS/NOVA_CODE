/*
 * ph3_calc_pll.h
 *
 * SRF 锁相环（CPU FPU 实现）
 */
#ifndef ALGO_PH3_CALC_PLL_H_
#define ALGO_PH3_CALC_PLL_H_

#include "ph3_calc_def.h"

void Ph3Calc_PllInit(Ph3Calc_Pll_t* pll, float32 step);
void Ph3Calc_PllRun(Ph3Calc_Pll_t* pll, float32 alfa, float32 beta);
void Ph3Calc_PllRunAbc(Ph3Calc_Pll_t* pll, float32 va, float32 vb, float32 vc);

#endif /* ALGO_PH3_CALC_PLL_H_ */
