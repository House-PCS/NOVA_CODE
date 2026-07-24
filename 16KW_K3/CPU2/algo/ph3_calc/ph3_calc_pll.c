/*
 * ph3_calc_pll.c
 *
 * 正序 SRF-PLL（CPU FPU 实现，对应 ThreePhasePLL / ThreePhasePLL1）
 */
#include "ph3_calc_pll.h"
#include "ph3_calc_math.h"

#pragma CODE_SECTION(Ph3Calc_WrapTheta, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_Clarke, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_PllRun, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_PllRunAbc, ".TI.ramfunc");

static void Ph3Calc_PllResetState(Ph3Calc_Pll_t* pll)
{
    pll->theta_temp = 0.0f;
    pll->theta_cos = 0.0f;
    pll->theta_sin = 0.0f;
    pll->integral = 0.0f;
    pll->vd = 0.0f;
    pll->vq = 0.0f;
    pll->error = 0.0f;
    pll->w = 0.0f;
    pll->freq = 0.0f;
}

static float32 Ph3Calc_WrapTheta(float32 theta)
{
    if(theta > (2.0f * PH3_CALC_PI))
    {
        theta -= 2.0f * PH3_CALC_PI;
    }
    if(theta < (-2.0f * PH3_CALC_PI))
    {
        theta += 2.0f * PH3_CALC_PI;
    }
    return theta;
}

static void Ph3Calc_Clarke(float32 va, float32 vb, float32 vc,
                           float32* alfa, float32* beta)
{
    *alfa = (va - 0.5f * vb - 0.5f * vc) * 0.66666667f;
    *beta = (0.866025f * vb - 0.866025f * vc) * 0.66666667f;
}

void Ph3Calc_PllInit(Ph3Calc_Pll_t* pll, float32 step)
{
    if(pll == NULL)
    {
        return;
    }

    pll->kp = PH3_CALC_PLL_KP;
    pll->ki = PH3_CALC_PLL_KI;
    pll->step = step;
    Ph3Calc_PllResetState(pll);
}

void Ph3Calc_PllRun(Ph3Calc_Pll_t* pll, float32 alfa, float32 beta)
{
    if(pll == NULL)
    {
        return;
    }

    pll->vd = alfa * Ph3Calc_Cos(pll->theta_temp) + beta * Ph3Calc_Sin(pll->theta_temp);
    pll->vq = (-1.0f * alfa * Ph3Calc_Sin(pll->theta_temp)) + beta * Ph3Calc_Cos(pll->theta_temp);

    pll->error = pll->vq;
    pll->integral += pll->step * pll->error;
    pll->integral = PH3_CALC_VALUE_LIMIT(pll->integral,
                                           PH3_CALC_PLL_W_LIMIT,
                                           -PH3_CALC_PLL_W_LIMIT);

    pll->w = pll->kp * pll->error + pll->ki * pll->integral;
    pll->w = PH3_CALC_VALUE_LIMIT(pll->w, PH3_CALC_PLL_W_LIMIT, -PH3_CALC_PLL_W_LIMIT);

    pll->theta_temp += (pll->w + PH3_CALC_PLL_W_NOMINAL) * pll->step;
    pll->theta_temp = Ph3Calc_WrapTheta(pll->theta_temp);

    pll->theta_cos = pll->theta_temp;
    pll->theta_sin = pll->theta_cos + 0.5f * PH3_CALC_PI;
}

void Ph3Calc_PllRunAbc(Ph3Calc_Pll_t* pll, float32 va, float32 vb, float32 vc)
{
    float32 alfa;
    float32 beta;

    if(pll == NULL)
    {
        return;
    }

    Ph3Calc_Clarke(va, vb, vc, &alfa, &beta);
    Ph3Calc_PllRun(pll, alfa, beta);
}
