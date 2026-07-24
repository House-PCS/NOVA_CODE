/**
 * @file mid_sliding_filter.c
 * @brief Sliding-window moving-average filter
 */

#include "mid_sliding_filter.h"

void Mid_SlidingFilter_Init(Mid_SlidingFilter_t* filter, float32* buffer, uint16_t win_size)
{
    uint16_t i;

    if ((filter == NULL) || (buffer == NULL) || (win_size == 0U))
    {
        return;
    }

    filter->buffer = buffer;
    filter->win_size = win_size;

    for (i = 0U; i < win_size; i++)
    {
        filter->buffer[i] = 0.0f;
    }

    Mid_SlidingFilter_Reset(filter);
}

void Mid_SlidingFilter_Reset(Mid_SlidingFilter_t* filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->index = 0U;
    filter->is_full = 0U;
    filter->sum = 0.0f;
}

float32 Mid_SlidingFilter_Update(Mid_SlidingFilter_t* filter, float32 sample)
{
    uint16_t count;

    if ((filter == NULL) || (filter->buffer == NULL) || (filter->win_size == 0U))
    {
        return sample;
    }

    if (filter->is_full != 0U)
    {
        filter->sum -= filter->buffer[filter->index];
    }

    filter->sum += sample;
    filter->buffer[filter->index] = sample;
    filter->index++;

    if (filter->index >= filter->win_size)
    {
        filter->index = 0U;
        filter->is_full = 1U;
    }

    count = (filter->is_full != 0U) ? filter->win_size : filter->index;
    if (count == 0U)
    {
        return sample;
    }

    return filter->sum / (float32)count;
}
