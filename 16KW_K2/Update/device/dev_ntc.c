/*
 * dev_ntc.c
 *
 *  Created on: 2025年5月15日
 *      Author: Nova006352
 */

#include "dev_ntc.h"
#include "adc.h"
#include "data_process.h"

#define LN(a)         (_ApproLn(a))
#define RTEMP_B     4050
#define RTEMP25     10
#define T25_1       (float)(1.0 / (25 + 273.15))
#define VOL_UP      3.3
#define R_UP        10
#define R_DOWN      75
#define TEMP_Filter_SIZE  30

#define TEMP_LowPass_Ts 0.00005f
#define TEMP_LowPass_W0 200.0f

#define Value_FOUR_MAX(x,y,z,w)      (((x > y ? x : y)>z ? (x > y ? x : y) : z)> w ?\
                                     ((x > y ? x : y)>z ? (x > y ? x : y) : z) :w)


ntc_temperature_t gstNtcTemperature;
// DataFilter INVTemp;
// DataFilter PvTemp;
// DataFilter RadiatorTemp;
// DataFilter ChassisTemp;

LowPassfilter_1order_t INVTemp_LowPass;
LowPassfilter_1order_t PvTemp_LowPass;
LowPassfilter_1order_t RadiatorTemp_LowPass;
LowPassfilter_1order_t ChassisTemp_LowPass;



static double _ApproLn(double a)//也可用math库中的exp函数
{
    uint16_t n = 15;//我们取前15+1项来估算
    uint16_t k = 0;
    uint16_t nk = 0;
    double x = 0;
    double x2 = 0;
    double y = 0;

    x = (a - 1) / (a + 1);
    x2 = x * x;
    nk = 2*n + 1;
    y = 1.0 / nk;

    for(k = n; k > 0; k--)
    {
        nk -= 2;
        y = 1.0 / nk + x2 * y;
    }
    return 2.0 * x * y;


}

// void TempDataFliterInit(void)
// {
//     static uint16_t TempData_initialized;
//     if (!TempData_initialized)
//     {
//         data_filter_init(&INVTemp , TEMP_Filter_SIZE);
//         data_filter_init(&PvTemp , TEMP_Filter_SIZE);
//         data_filter_init(&RadiatorTemp , TEMP_Filter_SIZE);
//         data_filter_init(&ChassisTemp , TEMP_Filter_SIZE);
//     }    
// }

/**
* @brief: INVTemperatureGet
* @param: float
* @note:
*
*/
float32 InvTemperatureGet(void)
{
    float32 v = 0;
    float32 rtotal = 0;
    float32 rntc = 0;
    float32 rtemp = 0;

    v = LowPass_1order_butterfilter(&INVTemp_LowPass ,TEMP_LowPass_W0 ,TEMP_LowPass_Ts, gstAdcResult.Temp_A1);

    rtotal = (R_UP * v) / (VOL_UP - v);
    rntc = (R_DOWN * rtotal) / (R_DOWN - rtotal);

    rtemp = LN(rntc / RTEMP25) / RTEMP_B + T25_1;
    rtemp = 1 / rtemp - 273.15f;

    gstNtcTemperature.Inv_temperature = rtemp;
    return rtemp;
}


/**
* @brief: PVTemperatureGet
* @param: float
* @note:
*/
float32 PvTemperatureGet(void)
{
    float32 v = 0;
    float32 rtotal = 0;
    float32 rntc = 0;
    float32 rtemp = 0;

    v = LowPass_1order_butterfilter(&PvTemp_LowPass,TEMP_LowPass_W0 ,TEMP_LowPass_Ts,gstAdcResult.Temp_A0);

    rtotal = (R_UP * v) / (VOL_UP - v);
    rntc = (R_DOWN * rtotal) / (R_DOWN - rtotal);

    rtemp = LN(rntc / RTEMP25) / RTEMP_B + T25_1;
    rtemp = 1 / rtemp - 273.15f;

    gstNtcTemperature.pv_temperature = rtemp;
    return rtemp;
}


/**
* @brief: RadiatorTemperatureGet
* @param: float
* @note:
*/
float32 RadiatorTemperatureGet(void)
{
    float32 v = 0;
    float32 rtotal = 0;
    float32 rntc = 0;
    float32 rtemp = 0;

    v = LowPass_1order_butterfilter(&RadiatorTemp_LowPass,TEMP_LowPass_W0 ,TEMP_LowPass_Ts,gstAdcResult.Temp_A2);

    rtotal = (R_UP * v) / (VOL_UP - v);
    rntc = (R_DOWN * rtotal) / (R_DOWN - rtotal);

    rtemp = LN(rntc / RTEMP25) / RTEMP_B + T25_1;
    rtemp = 1 / rtemp - 273.15f;

    gstNtcTemperature.rediator_temperature = rtemp;
    return rtemp;

}

/**
* @brief: ChassisTemperatureGet
* @param: float
* @note:
*
*/
float32 ChassisTemperatureGet(void)
{
    float32 v = 0;
    float32 rtotal = 0;
    float32 rntc = 0;
    float32 rtemp = 0;

    v = LowPass_1order_butterfilter(&ChassisTemp_LowPass,TEMP_LowPass_W0 ,TEMP_LowPass_Ts,gstAdcResult.Temp_A3);

    rtotal = (R_UP * v) / (VOL_UP - v);
    rntc = (R_DOWN * rtotal) / (R_DOWN - rtotal);

    rtemp = LN(rntc / RTEMP25) / RTEMP_B + T25_1;
    rtemp = 1 / rtemp - 273.15f;

    gstNtcTemperature.chassis_temperature =rtemp;
    return rtemp;


}

/**
  * @brief  获取NTC最大温度
  * @param  None
  * @retval None
  */
float GetNTCMaxTemp(void)
{
    return    Value_FOUR_MAX(gstNtcTemperature.Inv_temperature, gstNtcTemperature.chassis_temperature, \
                             gstNtcTemperature.pv_temperature,gstNtcTemperature.rediator_temperature);
}
