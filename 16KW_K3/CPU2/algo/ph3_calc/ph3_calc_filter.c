/*
 * ph3_calc_filter.c
 *
 * 块累加平均滤波（满窗后输出均值并清零累加器）
 */
#include "ph3_calc_filter.h"
#include "ph3_calc_math.h"

void Ph3Calc_Filter_Init(Ph3Calc_Filter_t *filter, uint32_t window_size)
{
    if(filter == NULL)
    {
        return;
    }

    filter->sum = 0.0f;
    filter->count = 0U;
    filter->window_size = (window_size > 0U) ? window_size : PH3_CALC_FILTER_DEFAULT_WIN;
    filter->output = 0.0f;
}

void Ph3Calc_Filter_Reset(Ph3Calc_Filter_t *filter)
{
    if(filter == NULL)
    {
        return;
    }

    filter->sum = 0.0f;
    filter->count = 0U;
}

float32 Ph3Calc_Filter_Run(Ph3Calc_Filter_t *filter, float32 sample)
{
    if(filter == NULL)
    {
        return 0.0f;
    }

    filter->count++;
    filter->sum += sample;

    if(filter->count >= filter->window_size)
    {
        filter->output = Ph3Calc_Div(filter->sum, (float32)filter->count);
        filter->sum = 0.0f;
        filter->count = 0U;
    }

    return filter->output;
}

float32 Ph3Calc_Filter_GetOutput(const Ph3Calc_Filter_t *filter)
{
    if(filter == NULL)
    {
        return 0.0f;
    }

    return filter->output;
}
