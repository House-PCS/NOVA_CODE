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
static float RtempCal(float u_rtemp)
{
	float rtemp; 
	
    if(u_rtemp >= 3.3L)
    {
        u_rtemp = 3.299999;
    }
	rtemp = R_UP * u_rtemp / (3.3L - u_rtemp);
	return rtemp;
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

static float GetTemperature4200(float Rntc)
{
	float tmp1;
	float res;
	
	tmp1 = LN(Rntc / 100) / 4200 + 1 / T25; 
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





static float TemperatureGet(uint8_t channel)
{
    float res;
    float    volt;
	float 	 rntc; 
/*    
//    adc_value = AdcGetData(channel);
    Adc_Handle.current_chl = channel;
    Adc_Handle.pRValtage = &volt;
    if(AdcVoltageGet(&Adc_Handle) == false)
    {
        return false;
    }
	rntc = RtempCal(volt);
	res  = GetTemperature(rntc);
 */   
    return res;
}
//int NTC_TempCalculate(uint16_t vol,uint16_t rtc_B,uint8_t rtc_K,uint8_t r_up)
//{
//    uint16_t volt = 0;
//    float rntc = 0,tmp	= 0;
//    
//    volt = vol;
//    if(volt >= 3300L)
//    {
//        volt = 3299;
//    }
//	rntc = r_up * volt / (3000 - volt);   //上拉电阻  
//    tmp = LN(rntc / rtc_K) / rtc_B + 1 / T25; 
//	return 1 / tmp - 273.15L;    
//}
uint32_t NTC_ResistorCalculate(uint16_t adc_vol,uint32_t r_up,uint16_t vol)
{
    uint16_t volt = 0;
    uint16_t rntc = 0,tmp	= 0;
    
    volt = adc_vol;
    if(volt >= 3300)
    {
        volt = 3299;
    }
	return r_up * volt / (vol - volt);   //上拉电阻
}
int NTC_TempCalculate(uint32_t rntc,uint16_t rtc_B,uint32_t rtc_K)
{
    float tmp	= 0;
    
    tmp = LN((double)rntc / rtc_K) / rtc_B + 1 / T25; 
	return 1 / tmp - 273.15L;    
}
float TemperatureDataChange(float adc_vol, uint8_t r_up)
{
    float res;
    float    volt;
	float 	 rntc; 
    
    volt = adc_vol;
    if(volt >= 3.3L)
    {
        volt = 3.299999;
    }
	rntc = r_up * volt / (3L - volt);   //上拉电阻100K
    
	res  = GetTemperature(rntc);
    
    return res;
}

float TemperatureDataChange4200(float adc_vol, uint8_t r_up)
{
    float res;
    float    volt;
	float 	 rntc; 
    
    volt = adc_vol;
    if(volt >= 3.3L)
    {
        volt = 3.299999;
    }
	rntc = r_up * volt / (3.3L - volt);   //上拉电阻100K
    
	res  = GetTemperature4200(rntc);
    
    return res;
}






