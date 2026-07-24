#ifndef PLL_H
#define PLL_H

#include "Solar_F.h"

typedef struct pll_stru_t
{
    NOTCH_COEFF_F notch_coeff;
    NOTCH_VARS_F notch_vars;
    CNTL_PI_F   cntl_pi;
    float ref;
    float fbk;
    float delta_wn;
} Pll_t;

float PllGetSignPhaseErr(float sign1, float sign2, NOTCH_VARS_F* notch_vars, NOTCH_COEFF_F* notch_coeff);
void PllInit(Pll_t* pll);
float PllHandler(Pll_t* pll, float ref, float fbk);
float PllOffsetHandler(Pll_t* pll, float ref, float offset, uint8_t en_inject);

#endif
