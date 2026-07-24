/*
 * ct.h
 *
 *  Created on: 2025-11-17
 *      Author: Nova006348
 */

#ifndef APP_CT_H_
#define APP_CT_H_

#include "F28x_Project.h"


#define CT_BREAK_THRESHOLD   (0.8f)

typedef struct _ct_out_info_stru_t
{
    float   a_phase_active;
    float   b_phase_active;
    float   c_phase_active;
    float   total_active;

    float   a_phase_reactive;
    float   b_phase_reactive;
    float   c_phase_reactive;
    float   total_reactive;

    float   a_phase_apperant;
    float   b_phase_apperant;
    float   c_phase_apperant;
    float   total_apperant;

} CtOutInfo_t;

typedef struct _ct_info_stru_t
{
    CtOutInfo_t   ct_power;
} CtDataInfo_t;

#define WINDOW_SIZE 30

typedef struct {
    float data[WINDOW_SIZE];  /* Window sample buffer */
    int index;                /* Current write index */
    int count;                /* Number of samples in window */
    float sum;                /* Sum of window samples */
    float square_sum;         /* Sum of squared window samples */
} CtSlideWindow_t;

void CtAutoCorrentPower(void);   /* CT auto polarity correction; use only without remote load or parallel units */
void CtReversePower(int ct_inverse);   /* CT polarity inversion; required for all CT active/reactive power outputs */
bool* CtbreakJudge(void);   /* CT wire-break detection */

extern CtDataInfo_t  gCtDataInfo;

#endif /* APP_CT_H_ */
