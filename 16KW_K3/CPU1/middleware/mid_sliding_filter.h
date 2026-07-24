/**
 * @file mid_sliding_filter.h
 * @brief Sliding-window moving-average filter (caller-owned buffer)
 */

#ifndef MID_SLIDING_FILTER_H
#define MID_SLIDING_FILTER_H

#include "F28x_Project.h"

typedef struct mid_sliding_filter_stru_t
{
    float32* buffer;
    uint16_t   win_size;
    uint16_t   index;
    uint16_t   is_full;
    float32  sum;
} Mid_SlidingFilter_t;

void Mid_SlidingFilter_Init(Mid_SlidingFilter_t* filter, float32* buffer, uint16_t win_size);
void Mid_SlidingFilter_Reset(Mid_SlidingFilter_t* filter);
float32 Mid_SlidingFilter_Update(Mid_SlidingFilter_t* filter, float32 sample);

#endif /* MID_SLIDING_FILTER_H */
