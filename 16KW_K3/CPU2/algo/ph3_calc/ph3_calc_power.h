/*
 * ph3_calc_power.h
 *
 * 三相有功/无功/视在功率及功率因数
 */
#ifndef ALGO_PH3_CALC_POWER_H_
#define ALGO_PH3_CALC_POWER_H_

#include "ph3_calc_def.h"

void Ph3Calc_PowerUpdateFactor(Ph3Calc_Channel_t* ch);
void Ph3Calc_PowerUpdateReactive(Ph3Calc_Channel_t* ch);

#endif /* ALGO_PH3_CALC_POWER_H_ */
