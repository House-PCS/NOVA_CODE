/*
 * dev_ntc.h
 *
 *  Created on: 2025Äê5ÔÂ15ÈÕ
 *      Author: Nova006352
 */

#ifndef DEVICE_DEV_NTC_H_
#define DEVICE_DEV_NTC_H_

#include "F28x_Project.h"

typedef struct _ntc_temperature_stru_t
{
    float32 rediator_temperature;
    float32 chassis_temperature;
    float32 pv_temperature;
    float32 Inv_temperature;
} ntc_temperature_t;


float32 InvTemperatureGet(void);
float32 PvTemperatureGet(void);
float32 RadiatorTemperatureGet(void);
float32 ChassisTemperatureGet(void);
float GetNTCMaxTemp(void);
void TempDataFliterInit();
extern ntc_temperature_t gstNtcTemperature;
#endif /* DEVICE_DEV_NTC_H_ */
