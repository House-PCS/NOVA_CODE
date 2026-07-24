/*
 * ph3_calc_filter.h
 *
 * 块累加平均滤波（满窗后输出均值并清零累加器）
 */
#ifndef ALGO_PH3_CALC_FILTER_H_
#define ALGO_PH3_CALC_FILTER_H_

#include "ph3_calc_def.h"

#define PH3_CALC_FILTER_DEFAULT_WIN  400U

typedef struct ph3_calc_filter_stru_t
{
    float32 sum;
    uint32_t count;
    uint32_t window_size;
    float32 output;
} Ph3Calc_Filter_t;

void Ph3Calc_Filter_Init(Ph3Calc_Filter_t *filter, uint32_t window_size);
void Ph3Calc_Filter_Reset(Ph3Calc_Filter_t *filter);
float32 Ph3Calc_Filter_Run(Ph3Calc_Filter_t *filter, float32 sample);
float32 Ph3Calc_Filter_GetOutput(const Ph3Calc_Filter_t *filter);

#endif /* ALGO_PH3_CALC_FILTER_H_ */
