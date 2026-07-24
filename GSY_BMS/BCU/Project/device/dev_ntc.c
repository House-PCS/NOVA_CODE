/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#include "dev_ntc.h"
#include "math.h"


#define LOG_TAG     "ntc"
#define RTEMP_B		3435    // 
#define RTEMP25		10//100
#define T25			(25 + 273.15)

#define R_UP        15

 #define LN			_appro_ln
//#define LN			log

#define TEMPERATURE_GET_PERIOD  100

#define NTC_CHANNEL     ADC_CHL_0

/** 
 * @brief    NTC当前阻值计算
 * @param    a:要计算ln的指数
 * @return   ln值
 * @note:    
 */
static double _appro_ln(double a)
{
	uint8_t N = 15;//我们取前15+1项来估算
	uint8_t k, nk;
	double x, x2, y;
	
	x = (a - 1) / (a + 1);
	x2 = x * x;
	nk = 2*N + 1;
	y = 1.0 / nk;
	
	for(k = N; k > 0; k--)
	{
		nk -= 2;
		y = 1.0 / nk + x2 * y;
	}
	return 2.0 * x * y;
}

//单位K
static float _RNTCCalc(uint16_t adc_volt, uint16_t volt_max, uint32_t r_up)
{
	float r_ntc;
    if(adc_volt >= volt_max)
    {
        adc_volt = volt_max - 1;
    }
    r_ntc = ((float)(adc_volt * r_up)) / (volt_max - adc_volt);
    return r_ntc;
}


static float GetTemperature(float Rntc)
{
	float tmp1;
	float res;
	
	tmp1 = LN(Rntc / RTEMP25) / RTEMP_B + 1 / T25; 
	res = 1 / tmp1 - 273.15L;
	return res;
//    float N1,N2,N3,N4;
//    
//    N1 = (log(Rntc) - log(RTEMP25)) / RTEMP_B;
//    N2 = 1/T25 + N1;
//    N3 = 1 / N2;
//    N4 = N3 - 273.15;
//    return N4;
    
} 












