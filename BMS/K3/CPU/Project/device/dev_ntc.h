
#ifndef __DEV_NTC_H_
#define __DEV_NTC_H_

#include "bsp.h"

float TemperatureGet(uint8_t channel);
float TemperatureDataChange4200(float adc_vol, uint8_t r_up);
void TemperatureProcess(void);
void SetBMSTempS0(void);
void ResetBMSTempS0(void);
void SetBMSTempS1(void);
void ResetBMSTempS1(void);
void SetBMSTempS2(void);
void ResetBMSTempS2(void);

#endif 



