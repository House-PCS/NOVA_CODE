/**
 * @file dev_ntc.c
 * @brief NTC thermistor parameters and temperature conversion
 */

#include "dev_ntc.h"

#define LN(a) (_ApproLn(a))

NtcParam_t _gNtcPara = {
    .b_value = 4050.0f,
    .r25_value = 10.0f,
    .t25_kelvin = 1.0f / (25.0f + 273.15f),
    .vol_up = NTC_VOL_UP,
    .r_up = NTC_R_UP,
    .r_down = NTC_R_DOWN,
};

/**
 * @brief Approximate natural logarithm
 * @param a Input value
 * @return Approximation of ln(a)
 */
static double _ApproLn(double a)
{
    uint16_t n = 15U; /* Use first n+1 terms */
    uint16_t k = 0U;
    uint16_t nk = 0U;
    double x = 0.0;
    double x2 = 0.0;
    double y = 0.0;

    x = (a - 1.0) / (a + 1.0);
    x2 = x * x;
    nk = 2U * n + 1U;
    y = 1.0 / nk;

    for(k = n; k > 0U; k--)
    {
        nk -= 2U;
        y = 1.0 / nk + x2 * y;
    }
    return 2.0 * x * y;
}

/**
 * @brief Convert ADC voltage to temperature
 * @param voltage ADC voltage (V)
 * @return Temperature (degC)
 */
float CalculateTemp(float voltage)
{
    float rtotal = 0.0f;
    float rntc = 0.0f;
    float rtemp = 0.0f;
    float temperature = 0.0f;

    if(voltage <= 0.0f || voltage >= _gNtcPara.vol_up)
    {
        return 0.0f;
    }

    rtotal = (_gNtcPara.r_up * voltage) / (_gNtcPara.vol_up - voltage);
    rntc = (_gNtcPara.r_down * rtotal) / (_gNtcPara.r_down - rtotal);
    rtemp = LN(rntc / _gNtcPara.r25_value) / _gNtcPara.b_value + _gNtcPara.t25_kelvin;
    temperature = 1.0f / rtemp - 273.15f;

    return temperature;
}
