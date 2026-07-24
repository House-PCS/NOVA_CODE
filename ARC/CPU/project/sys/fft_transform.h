#ifndef _FFT_TRANSFORM_H_

#define _FFT_TRANSFORM_H_


#include  "arm_math.h"
#define TEST_LENGTH_SAMPLES 1024 

void arm_rfft_f32_app(void);
void PowerPhaseRadians_f32(float32_t *_ptr, float32_t *_phase, uint16_t _usFFTPoints, float32_t _uiCmpValue);

#endif
