
#ifndef __DEV_NTC_H_
#define __DEV_NTC_H_

#include "stdint.h"

typedef enum
{
    EN_DEV_NTC_B_3450 = 3450,
    EN_DEV_NTC_B_3435 = 3435
}EN_NTC_B;


typedef enum
{
    EN_DEV_NTC_T25_5K = 5000,
    EN_DEV_NTC_T25_10K = 10000,
    EN_DEV_NTC_T25_50K = 50000,
    EN_DEV_NTC_T25_100K = 100000,
}EN_NTC_T25;


float TemperatureGet(uint8_t channel);
float TemperatureDataChange(float adc_vol, uint8_t r_up);
float TemperatureDataChange4200(float adc_vol, uint8_t r_up);

uint32_t NTC_ResistorCalculate(uint16_t adc_vol,uint32_t r_up,uint16_t vol);
int NTC_TempCalculate(uint32_t rntc,uint16_t rtc_B,uint32_t rtc_K);
//void TemperatureProcess(void);

#endif 













