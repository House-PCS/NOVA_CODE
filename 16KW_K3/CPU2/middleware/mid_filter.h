/**
 * @file mid_filter.h
 * @brief First/second-order Butterworth low-pass filters
 */

#ifndef MID_FILTER_H
#define MID_FILTER_H

#include "F28x_Project.h"

typedef struct mid_filter_1order_butter_stru_t
{
    float32 a0;
    float32 a1;
    float32 b1;
    float32 yn;
    float32 yn_last1;
    float32 xn_last1;
} Mid_Filter1OrderButter_t;

typedef struct mid_filter_2order_butter_stru_t
{
    float32 nb0;
    float32 nb1;
    float32 na1;
    float32 na2;
    float32 xn_last1;
    float32 xn_last2;
    float32 yn;
    float32 yn_last1;
    float32 yn_last2;
} Mid_Filter2OrderButter_t;

void Mid_Filter1OrderButter_Init(Mid_Filter1OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz);
void Mid_Filter1OrderButter_Reset(Mid_Filter1OrderButter_t* filter);
float32 Mid_Filter1OrderButter_Update(Mid_Filter1OrderButter_t* filter, float32 input);

void Mid_Filter2OrderButter_Init(Mid_Filter2OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz);
void Mid_Filter2OrderButter_Reset(Mid_Filter2OrderButter_t* filter);
float32 Mid_Filter2OrderButter_Update(Mid_Filter2OrderButter_t* filter, float32 input);

#endif /* MID_FILTER_H */
