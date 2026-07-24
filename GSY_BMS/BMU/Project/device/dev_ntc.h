
#ifndef __DEV_NTC_H_
#define __DEV_NTC_H_

#include "stdint.h"

float TemperatureGet(uint8_t channel);
float TemperatureDataChange(float adc_vol, uint8_t r_up);
float TemperatureDataChange4200(float adc_vol, uint8_t r_up);

uint32_t NTC_ResistorCalculate(uint16_t adc_vol,uint32_t r_up,uint16_t vol);
int NTC_TempCalculate(uint32_t rntc,uint16_t rtc_B,uint32_t rtc_K);
//void TemperatureProcess(void);

#endif 













