/*
 * ph3_calc_posneg.c
 *
 * 三相正负序分离（CPU FPU 实现，对应 ThreePhaseTransformPosNegOrder）
 */
#include "ph3_calc_posneg.h"
#include "ph3_calc_math.h"

#pragma CODE_SECTION(Ph3Calc_PosNegRun, ".TI.ramfunc");

void Ph3Calc_PosNegInit(Ph3Calc_PosNegData_t* data)
{
    if(data == NULL)
    {
        return;
    }

//    data->a_instant = 0.0f;
//    data->b_instant = 0.0f;
//    data->c_instant = 0.0f;
//    data->alfa = 0.0f;
//    data->beta = 0.0f;
    data->alfa_90 = 0.0f;
    data->beta_90 = 0.0f;
    data->alfa_90_1 = 0.0f;
    data->beta_90_1 = 0.0f;
    data->alfa_1 = 0.0f;
    data->beta_1 = 0.0f;
    data->alfa_pos = 0.0f;
    data->alfa_neg = 0.0f;
    data->beta_pos = 0.0f;
    data->beta_neg = 0.0f;
    data->d_pos = 0.0f;
    data->d_neg = 0.0f;
    data->q_pos = 0.0f;
    data->q_neg = 0.0f;
}

void Ph3Calc_PosNegRun(Ph3Calc_PosNegData_t* data,
                       float32 va, float32 vb, float32 vc,
                       float32 theta)
{
    float32 w;
    float32 a1;
    float32 b0;
    float32 b1;
    float32 cos_theta;
    float32 sin_theta;
    float32 cos_neg_theta;
    float32 sin_neg_theta;

    if(data == NULL)
    {
        return;
    }

    data->a_instant = va;
    data->b_instant = vb;
    data->c_instant = vc;

    w = 2.0f * PH3_CALC_PI * PH3_CALC_POSNEG_BP_FREQ;
    a1 = Ph3Calc_Div((w * PH3_CALC_TS - 2.0f), (w * PH3_CALC_TS + 2.0f));
    b0 = Ph3Calc_Div((-2.0f + w * PH3_CALC_TS), (w * PH3_CALC_TS + 2.0f));
    b1 = 1.0f;

    data->alfa = (va - 0.5f * vb - 0.5f * vc) * 0.66666667f;
    data->beta = (0.866025f * vb - 0.866025f * vc) * 0.66666667f;

    data->alfa_90 = (-1.0f * a1 * data->alfa_90_1) + (b0 * data->alfa) + (b1 * data->alfa_1);
    data->beta_90 = (-1.0f * a1 * data->beta_90_1) + (b0 * data->beta) + (b1 * data->beta_1);

    data->alfa_90_1 = data->alfa_90;
    data->beta_90_1 = data->beta_90;
    data->alfa_1 = data->alfa;
    data->beta_1 = data->beta;

    data->alfa_pos = 0.5f * data->alfa - 0.5f * data->beta_90;
    data->beta_pos = 0.5f * data->beta + 0.5f * data->alfa_90;
    data->alfa_neg = 0.5f * data->alfa + 0.5f * data->beta_90;
    data->beta_neg = 0.5f * data->beta - 0.5f * data->alfa_90;

    cos_theta = Ph3Calc_Cos(theta);
    sin_theta = Ph3Calc_Sin(theta);
    cos_neg_theta = Ph3Calc_Cos(-theta);
    sin_neg_theta = Ph3Calc_Sin(-theta);

    data->d_pos = data->alfa_pos * cos_theta + data->beta_pos * sin_theta;
    data->q_pos = (-1.0f * data->alfa_pos * sin_theta) + data->beta_pos * cos_theta;
    data->d_neg = data->alfa_neg * cos_neg_theta + data->beta_neg * sin_neg_theta;
    data->q_neg = (-1.0f * data->alfa_neg * sin_neg_theta) + data->beta_neg * cos_neg_theta;
}
