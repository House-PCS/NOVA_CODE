#ifndef __ARC_DETECT_THRESHOLD_H_
#define __ARC_DETECT_THRESHOLD_H_

#include "adc.h"
#include "arm_math.h"
#include "dev_cur_detect.h"
#include "math.h"



void CA_CFAR(float *signal,float *threshold);
//void CA_CFAR(float *signal,int len,float *threshold);
//void CA_CFAR1(float *signal,int len,float *threshold);
//int Arc_strength(float *signal,int len);
int ArcJudge(uint16_t value,uint16_t threshold);

void Constant_Value(float *signal,float threshold);
//int Arcjudgment(float *signal,int len,int Continuous_value);

//int Arcjudgment(float *signal,int set);
int Arcjudgment(float *signal,float *threshold,int set);
//int Arc_strength(float *signal);
int Arc_strength(float *signal,float *threshold);
//int Constant_Value(float *signal,int len);
int CountThershold(float *signal);
void Least_square_method(float *signal);

static int quick_sort(float *a,int low,int high);
float Absolute_difference(float *signal);

extern float32_t sum;
#endif