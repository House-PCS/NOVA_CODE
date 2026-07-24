#ifndef __CA_CFAR_H_
#define __CA_CFAR_H_

#include "adc.h"
#include "arm_math.h"
#include "dev_cur_detect.h"
#include "math.h"


void CA_CFAR(float *signal,int len,float *threshold);
int Arc_strength(float *signal,int len);
int ArcJudge(uint16_t value,uint16_t threshold);

int Arcjudgment(float *signal,int len,int Continuous_value);
int Constant_Value(float *signal,int len);
void Least_square_method(float *signal);
#endif