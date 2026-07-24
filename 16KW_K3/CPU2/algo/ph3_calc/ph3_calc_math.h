/*
 * ph3_calc_math.h
 *
 * CPU FPU 数学接口，替代 CLAmath（CLAcos / CLAsin / CLAsqrt / CLAdiv）
 */
#ifndef ALGO_PH3_CALC_MATH_H_
#define ALGO_PH3_CALC_MATH_H_

#include <math.h>

#define Ph3Calc_Cos(x)    cosf(x)
#define Ph3Calc_Sin(x)    sinf(x)
#define Ph3Calc_Sqrt(x)   sqrtf(x)
#define Ph3Calc_Div(n, d) ((n) / (d))

#endif /* ALGO_PH3_CALC_MATH_H_ */
