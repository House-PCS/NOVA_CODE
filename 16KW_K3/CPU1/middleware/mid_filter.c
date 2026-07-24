/**
 * @file mid_filter.c
 * @brief First/second-order Butterworth low-pass filters
 */

#include "mid_filter.h"

#define MID_FILTER_PI           (3.1415926f)
#define MID_FILTER_BUTTER2_ZETA (0.707106781f)

void Mid_Filter1OrderButter_Init(Mid_Filter1OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz)
{
    float32 wt;
    float32 den;

    if ((filter == NULL) || (cutoff_hz <= 0.0f) || (sample_hz <= 0.0f))
    {
        return;
    }

    /* Bilinear transform: yn = a0*x(n) + a1*x(n-1) - b1*y(n-1) */
    wt = 2.0f * MID_FILTER_PI * cutoff_hz / sample_hz;
    den = wt + 2.0f;
    filter->a0 = wt / den;
    filter->a1 = filter->a0;
    filter->b1 = (wt - 2.0f) / den;

    Mid_Filter1OrderButter_Reset(filter);
}

void Mid_Filter1OrderButter_Reset(Mid_Filter1OrderButter_t* filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->yn = 0.0f;
    filter->yn_last1 = 0.0f;
    filter->xn_last1 = 0.0f;
}

float32 Mid_Filter1OrderButter_Update(Mid_Filter1OrderButter_t* filter, float32 input)
{
    if (filter == NULL)
    {
        return input;
    }

    filter->yn = input * filter->a0 + filter->a1 * filter->xn_last1 - filter->b1 * filter->yn_last1;
    filter->xn_last1 = input;
    filter->yn_last1 = filter->yn;

    return filter->yn;
}

void Mid_Filter2OrderButter_Init(Mid_Filter2OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz)
{
    float32 wc;
    float32 ts;
    float32 b0;
    float32 a0;
    float32 a1;
    float32 a2;

    if ((filter == NULL) || (cutoff_hz <= 0.0f) || (sample_hz <= 0.0f))
    {
        return;
    }

    /* Same coefficient design as Second_Order_ButterWorth in cla_control.cla */
    wc = 2.0f * MID_FILTER_PI * cutoff_hz;
    ts = 1.0f / sample_hz;
    b0 = wc * wc * ts * ts;
    a0 = b0 + 4.0f * MID_FILTER_BUTTER2_ZETA * wc * ts + 4.0f;
    a1 = 2.0f * b0 - 8.0f;
    a2 = b0 - 4.0f * MID_FILTER_BUTTER2_ZETA * wc * ts + 4.0f;

    filter->nb0 = b0 / a0;
    filter->nb1 = 2.0f * filter->nb0;
    filter->na1 = a1 / a0;
    filter->na2 = a2 / a0;

    Mid_Filter2OrderButter_Reset(filter);
}

void Mid_Filter2OrderButter_Reset(Mid_Filter2OrderButter_t* filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->xn_last1 = 0.0f;
    filter->xn_last2 = 0.0f;
    filter->yn = 0.0f;
    filter->yn_last1 = 0.0f;
    filter->yn_last2 = 0.0f;
}

float32 Mid_Filter2OrderButter_Update(Mid_Filter2OrderButter_t* filter, float32 input)
{
    if (filter == NULL)
    {
        return input;
    }

    filter->yn = filter->nb0 * (filter->xn_last2 + input)
               + filter->nb1 * filter->xn_last1
               - filter->na1 * filter->yn_last1
               - filter->na2 * filter->yn_last2;

    filter->xn_last2 = filter->xn_last1;
    filter->xn_last1 = input;
    filter->yn_last2 = filter->yn_last1;
    filter->yn_last1 = filter->yn;

    return filter->yn;
}
