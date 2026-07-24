
#ifndef __DEV_NTC_H_
#define __DEV_NTC_H_

#include "bsp.h"

float TemperatureGet(uint8_t channel);
float TemperatureDataChange(float adc_vol, uint8_t r_up);
float TemperatureDataChange4200(float adc_vol, uint8_t r_up);

//void TemperatureProcess(void);

#endif 













