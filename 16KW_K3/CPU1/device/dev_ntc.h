/**
 * @file dev_ntc.h
 * @brief NTC thermistor parameters and temperature conversion
 */

#ifndef DEVICE_DEV_NTC_H_
#define DEVICE_DEV_NTC_H_

#include "F28x_Project.h"

typedef struct
{
    float b_value;      /**< B constant */
    float r25_value;    /**< Resistance at 25 degC (kOhm) */
    float t25_kelvin;   /**< Kelvin temperature at 25 degC */
    float vol_up;       /**< Pull-up supply voltage */
    float r_up;         /**< Pull-up resistor (kOhm) */
    float r_down;       /**< Pull-down resistor (kOhm) */
} NtcParam_t;

#define NTC_VOL_UP   (3.3f)   /**< Pull-up voltage (V) */
#define NTC_R_UP     (10.0f)  /**< Pull-up resistor (kOhm) */
#define NTC_R_DOWN   (75.0f)  /**< Pull-down resistor (kOhm) */

static double _ApproLn(double a);
float CalculateTemp(float voltage);

#endif /* DEVICE_DEV_NTC_H_ */
